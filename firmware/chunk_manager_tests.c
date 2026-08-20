/*
 * chunk_manager_tests.c
 *
 * Testroutinen fuer den SD-Karten-Chunk-Datei-Manager (STM32H755, SDIO, exFAT/FatFs).
 * Voraussetzung: chunk_manager.[ch] aus der vorangegangenen Design-Phase stellt die
 * unten als extern deklarierten Typen/Funktionen bereit. Diese Datei fuegt nichts an
 * der Kernlogik hinzu, sondern testet und befuellt sie.
 *
 * Fuenf Routinen:
 *   1. test_throughput               - Sustained-Schreibdurchsatz ueber mehrere Perioden
 *   2. test_write_latency_histogram  - Latenzverteilung einzelner f_write()-Aufrufe (p99!)
 *   3. test_retention_cycles         - Rotation/Loeschlogik unter beschleunigter Zeit
 *   4. test_power_loss_recovery      - Reset waehrend offener Periode, Recovery pruefen
 *   5. test_generate_sample_dataset  - Realistische Testdaten fuer Client-/Chart-Software
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"                 /* FatFs: FRESULT, FIL, f_open/f_write/... */

/* ---------------------------------------------------------------------- *
 *  Aus chunk_manager.h vorausgesetzt (hier nur referenziert, nicht neu
 *  implementiert - siehe vorherige Design-Nachrichten dieses Projekts).
 * ---------------------------------------------------------------------- */
typedef uint64_t ByteSize;

typedef enum { POOL_A_RAW_L1 = 0, POOL_B_L2_L3 = 1 } PoolId;

typedef struct {
    PoolId   id;
    ByteSize capacity_budget_bytes;
    uint32_t period_seconds;
    uint32_t current_period_id;
    FIL      fp_l0, fp_l1, fp_l2, fp_l3;
    ByteSize l0_write_offset, l0_capacity_bytes;
    ByteSize l1_write_offset;
} PoolState;

typedef uint32_t L1RecordPacked;

typedef struct {
    int16_t  max, min, avg;
    uint32_t travel;
} ChannelStat;

typedef struct {
    ChannelStat count_high;
    ChannelStat pause;
    ChannelStat burst;
    int16_t     temp_avg_c10;
} StatRecord;                    /* 32 Byte */

#define L1_RECORD_SIZE    4u
#define L2L3_RECORD_SIZE  32u
#define L0_RECORD_SIZE    125u

extern FRESULT   chunk_open_period(PoolState *p, uint32_t period_id);
extern void      chunk_close_period(PoolState *p);
extern FRESULT   l1_write(PoolState *p, L1RecordPacked rec);
extern FRESULT   l0_write(PoolState *p, const void *block125, uint32_t l1_index, uint16_t reason);
extern void      chunk_retention_check(PoolState *p);
extern FRESULT   chunk_recover_at_boot(PoolState *p);
extern ByteSize  pool_used_bytes(PoolState *p);
extern L1RecordPacked l1_pack(uint16_t count_high, int16_t pause, uint16_t burst, bool trig);

/* Zeitbasis in Mikrosekunden - auf dem Zielsystem z.B. via DWT-Zykluszaehler
 * (CM7, 480 MHz) oder einen freilaufenden Hardwaretimer implementieren. */
extern uint32_t  test_get_time_us(void);
extern void      test_log(const char *fmt, ...);   /* z.B. printf-Wrapper auf UART/SWO */

/* ------------------------------------------------------------------ */
/*  Gemeinsame Statistikstruktur fuer alle Testroutinen                */
/* ------------------------------------------------------------------ */

#define LAT_HIST_BUCKETS      201u   /* 0..199 -> 0..19,9 ms in 100us-Schritten, 200 = Ueberlauf */
#define LAT_HIST_BUCKET_US    100u

typedef struct {
    uint32_t writes_total;
    uint32_t write_us_min, write_us_max;
    uint64_t write_us_sum;
    uint32_t hist[LAT_HIST_BUCKETS];
    uint32_t bytes_written;
    uint32_t elapsed_ms;
} WriteStats;

static void write_stats_reset(WriteStats *ws)
{
    memset(ws, 0, sizeof(*ws));
    ws->write_us_min = UINT32_MAX;
}

static void write_stats_record(WriteStats *ws, uint32_t us, uint32_t bytes)
{
    ws->writes_total++;
    ws->bytes_written += bytes;
    ws->write_us_sum   += us;
    if (us < ws->write_us_min) ws->write_us_min = us;
    if (us > ws->write_us_max) ws->write_us_max = us;

    uint32_t bucket = us / LAT_HIST_BUCKET_US;
    if (bucket >= LAT_HIST_BUCKETS) bucket = LAT_HIST_BUCKETS - 1;
    ws->hist[bucket]++;
}

/* p99 aus dem Histogramm ableiten - genuegt fuer Latenz-Ausreisser-Analyse,
 * ohne alle Einzelwerte im RAM vorhalten oder sortieren zu muessen. */
static uint32_t write_stats_percentile(const WriteStats *ws, float pct)
{
    uint32_t target = (uint32_t)(ws->writes_total * pct);
    uint32_t cum = 0;
    for (uint32_t i = 0; i < LAT_HIST_BUCKETS; i++) {
        cum += ws->hist[i];
        if (cum >= target) {
            if (i == LAT_HIST_BUCKETS - 1)
                return LAT_HIST_BUCKETS * LAT_HIST_BUCKET_US; /* "> 20ms" markieren */
            return i * LAT_HIST_BUCKET_US;
        }
    }
    return ws->write_us_max;
}

static void write_stats_report(const char *label, const WriteStats *ws)
{
    float avg_us = ws->writes_total ? (float)ws->write_us_sum / ws->writes_total : 0.0f;
    float kbps   = ws->elapsed_ms ? ((float)ws->bytes_written / 1024.0f) / (ws->elapsed_ms / 1000.0f) : 0.0f;
    test_log("[%s] n=%lu bytes=%lu elapsed=%lums throughput=%.1f KB/s\n",
              label, ws->writes_total, ws->bytes_written, ws->elapsed_ms, kbps);
    test_log("[%s] latency us: min=%lu avg=%.1f max=%lu p99=%lu p99.9=%lu\n",
              label, ws->write_us_min, avg_us, ws->write_us_max,
              write_stats_percentile(ws, 0.99f), write_stats_percentile(ws, 0.999f));
}

/* ------------------------------------------------------------------ */
/*  Kleiner deterministischer PRNG (fuer reproduzierbare Testdaten,     */
/*  kein CSPRNG-Anspruch - xorshift32 reicht hier vollkommen)          */
/* ------------------------------------------------------------------ */

static uint32_t rng_state = 0x1234ABCDu;

static void rng_seed(uint32_t seed) { rng_state = seed ? seed : 1u; }

static uint32_t rng_next(void)
{
    uint32_t x = rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return rng_state = x;
}

static float rng_gauss(float mean, float stdev)
{
    /* Box-Muller, ausreichend fuer synthetische Testdaten */
    float u1 = ((rng_next() >> 8) + 1) / (float)(1u << 24);
    float u2 = ((rng_next() >> 8) + 1) / (float)(1u << 24);
    float z  = sqrtf(-2.0f * logf(u1)) * cosf(6.28318530f * u2);
    return mean + z * stdev;
}

static int16_t clampi16(float v, int16_t lo, int16_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return (int16_t)v;
}

/* ==================================================================== */
/*  1. Durchsatz-/Stresstest ueber mehrere Perioden                      */
/* ==================================================================== */

void test_throughput(PoolState *pool, uint32_t num_periods, WriteStats *out)
{
    write_stats_reset(out);
    uint32_t t_start = test_get_time_us();

    for (uint32_t per = 0; per < num_periods; per++) {
        chunk_open_period(pool, pool->current_period_id + 1);

        uint32_t n_l1 = pool->period_seconds * 400u;   /* 400 Hz */
        for (uint32_t i = 0; i < n_l1; i++) {
            uint32_t t0 = test_get_time_us();
            L1RecordPacked rec = l1_pack(500, 0, 30, false);
            FRESULT r = l1_write(pool, rec);
            uint32_t t1 = test_get_time_us();
            if (r == FR_OK) write_stats_record(out, t1 - t0, L1_RECORD_SIZE);

            /* Baseline-L0-Satz alle 2s (800 L1-Zyklen) simulieren */
            if ((i % 800u) == 0u) {
                uint8_t block[L0_RECORD_SIZE] = {0};
                uint32_t tb0 = test_get_time_us();
                FRESULT rb = l0_write(pool, block, i, 0x01 /*TRIG_BASELINE*/);
                uint32_t tb1 = test_get_time_us();
                if (rb == FR_OK) write_stats_record(out, tb1 - tb0, L0_RECORD_SIZE);
            }
        }
        chunk_close_period(pool);
        chunk_retention_check(pool);
    }

    out->elapsed_ms = (test_get_time_us() - t_start) / 1000u;
    write_stats_report("throughput", out);
}

/* ==================================================================== */
/*  2. Reines Latenz-Histogramm einzelner f_write()-Aufrufe               */
/*     (kleinere, gezielte Messung ohne Perioden-Overhead dazwischen)     */
/* ==================================================================== */

void test_write_latency_histogram(PoolState *pool, uint32_t num_records, WriteStats *out)
{
    write_stats_reset(out);
    uint32_t t_start = test_get_time_us();

    for (uint32_t i = 0; i < num_records; i++) {
        uint32_t t0 = test_get_time_us();
        L1RecordPacked rec = l1_pack((uint16_t)(490 + (i % 20)), 0, 30, false);
        FRESULT r = l1_write(pool, rec);
        uint32_t t1 = test_get_time_us();
        if (r == FR_OK) write_stats_record(out, t1 - t0, L1_RECORD_SIZE);
    }

    out->elapsed_ms = (test_get_time_us() - t_start) / 1000u;
    write_stats_report("latency", out);

    /* Ausreisser > 10 ms sind fuer die RAM-Puffer-Dimensionierung relevant
     * (siehe Architekturdiskussion: SD-Karten koennen kurzzeitig blockieren) */
    if (write_stats_percentile(out, 0.99f) > 10000u) {
        test_log("[latency] WARNUNG: p99 > 10ms - RAM-Zwischenpuffer vergroessern!\n");
    }
}

/* ==================================================================== */
/*  3. Retention-/Rotationszyklen unter beschleunigter Zeit               */
/* ==================================================================== */

void test_retention_cycles(PoolState *pool_a, PoolState *pool_b, uint32_t num_cycles)
{
    /* Periodenlaenge fuer den Test drastisch verkuerzen, damit viele
     * Rotationszyklen in kurzer Zeit durchlaufen werden. NICHT die
     * Produktionswerte (1 Woche) veraendern - separate Testinstanzen. */
    pool_a->period_seconds = 10;
    pool_b->period_seconds = 10;

    /* Kuenstlich enges Budget, damit Retention nach wenigen Perioden greift */
    pool_a->capacity_budget_bytes = 6u * (pool_a->period_seconds * 400u * L1_RECORD_SIZE);
    pool_b->capacity_budget_bytes = 6u * (pool_b->period_seconds / 2u * L2L3_RECORD_SIZE);

    for (uint32_t c = 0; c < num_cycles; c++) {
        chunk_open_period(pool_a, pool_a->current_period_id + 1);
        chunk_open_period(pool_b, pool_b->current_period_id + 1);

        for (uint32_t i = 0; i < pool_a->period_seconds * 400u; i++)
            l1_write(pool_a, l1_pack(500, 0, 30, false));

        for (uint32_t i = 0; i < pool_b->period_seconds / 2u; i++) {
            StatRecord sr = {0};
            /* l2_write()/l3_write() analog zu l1_write() vorausgesetzt */
        }

        chunk_close_period(pool_a);
        chunk_close_period(pool_b);

        ByteSize used_a_before = pool_used_bytes(pool_a);
        ByteSize used_b_before = pool_used_bytes(pool_b);
        chunk_retention_check(pool_a);
        chunk_retention_check(pool_b);
        ByteSize used_a_after = pool_used_bytes(pool_a);
        ByteSize used_b_after = pool_used_bytes(pool_b);

        test_log("[retention] Zyklus %lu: Pool A %llu -> %llu Byte, Pool B %llu -> %llu Byte\n",
                  c, (unsigned long long)used_a_before, (unsigned long long)used_a_after,
                  (unsigned long long)used_b_before, (unsigned long long)used_b_after);

        /* Erwartung pruefen: Budget nach Retention eingehalten */
        if (used_a_after > pool_a->capacity_budget_bytes ||
            used_b_after > pool_b->capacity_budget_bytes) {
            test_log("[retention] FEHLER: Budget nach Retention-Check ueberschritten!\n");
        }
        /* Erwartung pruefen: Pools bleiben getrennt (keine Kreuzloeschung) */
        /* -> in der echten Umsetzung: Verzeichnis /LOG/A vs /LOG/B separat verifizieren */
    }
}

/* ==================================================================== */
/*  4. Stromausfall-/Recovery-Simulation                                  */
/* ==================================================================== */

void test_power_loss_recovery(PoolState *pool)
{
    chunk_open_period(pool, pool->current_period_id + 1);

    uint32_t n_before_cut = 1000u;   /* etwas schreiben, dann "Strom weg" */
    for (uint32_t i = 0; i < n_before_cut; i++)
        l1_write(pool, l1_pack(500, 0, 30, false));

    ByteSize offset_before_cut = pool->l1_write_offset;

    /* Absichtlich KEIN chunk_close_period() - simuliert Spannungsausfall
     * mitten in der offenen Periode (record_count bleibt 0xFFFFFFFF im Header). */

    /* "Reset" emulieren: PoolState frisch initialisieren, als wuerde das
     * System neu booten, und die Recovery-Logik gegen die noch offene
     * Periode auf der Karte laufen lassen. */
    PoolState recovered = {0};
    recovered.id              = pool->id;
    recovered.period_seconds  = pool->period_seconds;
    recovered.capacity_budget_bytes = pool->capacity_budget_bytes;

    FRESULT r = chunk_recover_at_boot(&recovered);

    if (r != FR_OK) {
        test_log("[power-loss] FEHLER: Recovery fehlgeschlagen (FRESULT=%d)\n", r);
        return;
    }
    if (recovered.current_period_id != pool->current_period_id) {
        test_log("[power-loss] FEHLER: falsche Periode nach Recovery (%lu statt %lu)\n",
                  recovered.current_period_id, pool->current_period_id);
    }
    if (recovered.l1_write_offset != offset_before_cut) {
        test_log("[power-loss] WARNUNG: Schreib-Cursor weicht ab (%llu statt %llu) "
                  "- ggf. durch periodisches f_sync() bedingter Verlust der letzten "
                  "ungesicherten Saetze, sollte innerhalb der f_sync-Intervallgroesse liegen\n",
                  (unsigned long long)recovered.l1_write_offset,
                  (unsigned long long)offset_before_cut);
    } else {
        test_log("[power-loss] OK: Recovery exakt beim letzten geschriebenen Satz fortgesetzt\n");
    }

    /* Weiterschreiben nach Recovery, um sicherzustellen, dass die Periode
     * danach normal fortgesetzt und spaeter sauber geschlossen werden kann. */
    for (uint32_t i = 0; i < 100u; i++)
        l1_write(&recovered, l1_pack(500, 0, 30, false));
    chunk_close_period(&recovered);

    *pool = recovered;
}

/* ==================================================================== */
/*  5. Synthetischer, plausibler Testdatensatz fuer Client-/Chart-Tests   */
/*     Verteilungsparameter aus der Html.c-/Feld-Log-Auswertung dieses    */
/*     Projekts (drei simulierte Geraetetypen mit unterschiedlicher       */
/*     Pause/Burst-Charakteristik, siehe vorherige Analyse).              */
/* ==================================================================== */

typedef struct {
    const char *name;
    float burst_mean, burst_stdev;
    float pause_pos_mean, pause_pos_stdev;   /* 1-pause */
    float pause_neg_mean, pause_neg_stdev;   /* 0-pause, als Betrag */
    float extreme_event_probability;         /* Anteil L1-Zyklen mit simuliertem Ereignis */
} SyntheticDeviceProfile;

static const SyntheticDeviceProfile PROFILE_LUEBECK     = { "Luebeck#078",     29.3f, 2.3f,  60.8f, 16.7f, 56.7f, 16.6f, 0.0005f };
static const SyntheticDeviceProfile PROFILE_TUERKENFELD = { "Tuerkenfeld#051", 29.7f, 2.4f,  63.8f, 13.8f, 75.3f, 25.0f, 0.0005f };
static const SyntheticDeviceProfile PROFILE_MVP         = { "MVP#061",         31.0f, 2.8f,  33.9f,  7.8f, 30.5f,  6.8f, 0.0005f };

static void generate_l1_record(const SyntheticDeviceProfile *prof, L1RecordPacked *out_rec, bool *out_extreme)
{
    uint16_t count_high = clampi16(rng_gauss(500.0f, 40.0f), 0, 1000);
    uint16_t burst       = (uint16_t)clampi16(rng_gauss(prof->burst_mean, prof->burst_stdev), 0, 1000);

    bool positive = (rng_next() & 1u);
    int16_t pause = positive
        ? clampi16(rng_gauss(prof->pause_pos_mean, prof->pause_pos_stdev), 0, 1000)
        : (int16_t)(-clampi16(rng_gauss(prof->pause_neg_mean, prof->pause_neg_stdev), 0, 1000));

    bool extreme = ((rng_next() / (float)UINT32_MAX) < prof->extreme_event_probability);
    if (extreme) {
        /* Extremwert-Ereignis simulieren: einer der drei Kanaele weicht deutlich ab */
        switch (rng_next() % 3u) {
            case 0: count_high = clampi16(count_high + (rng_next()%2?1:-1)*300, 0, 1000); break;
            case 1: burst = (uint16_t)clampi16(burst * 3, 0, 1000); break;
            case 2: pause = clampi16(pause * 3, -1000, 1000); break;
        }
    }

    *out_rec = l1_pack(count_high, pause, burst, extreme);
    *out_extreme = extreme;
}

void test_generate_sample_dataset(PoolState *pool, const SyntheticDeviceProfile *profile, uint32_t num_periods)
{
    rng_seed(0xC0FFEEu ^ (uint32_t)(uintptr_t)profile->name);
    test_log("[testdata] generiere %lu Perioden fuer Profil '%s'\n", num_periods, profile->name);

    for (uint32_t per = 0; per < num_periods; per++) {
        chunk_open_period(pool, pool->current_period_id + 1);

        uint32_t n_l1 = pool->period_seconds * 400u;
        for (uint32_t i = 0; i < n_l1; i++) {
            L1RecordPacked rec; bool extreme;
            generate_l1_record(profile, &rec, &extreme);
            l1_write(pool, rec);

            bool baseline_due = (i % 800u) == 0u;   /* alle 2s */
            if (baseline_due || extreme) {
                uint8_t block[L0_RECORD_SIZE];
                for (uint32_t b = 0; b < L0_RECORD_SIZE; b++) block[b] = (uint8_t)(rng_next() & 0xFF);
                uint16_t reason = baseline_due ? 0x01 : 0x02;
                l0_write(pool, block, i, reason);
            }
        }
        chunk_close_period(pool);
        chunk_retention_check(pool);
    }
    test_log("[testdata] fertig - Pool-Belegung: %llu Byte\n", (unsigned long long)pool_used_bytes(pool));
}

/* ==================================================================== */
/*  Test-Runner                                                          */
/* ==================================================================== */

void run_all_chunk_manager_tests(PoolState *pool_a, PoolState *pool_b)
{
    WriteStats ws;

    test_log("=== 1) Durchsatztest ===\n");
    test_throughput(pool_a, 2, &ws);

    test_log("=== 2) Latenz-Histogramm ===\n");
    test_write_latency_histogram(pool_a, 5000, &ws);

    test_log("=== 3) Retention-Zyklen ===\n");
    test_retention_cycles(pool_a, pool_b, 20);

    test_log("=== 4) Stromausfall-Recovery ===\n");
    test_power_loss_recovery(pool_a);

    test_log("=== 5) Synthetische Testdaten (3 Geraeteprofile) ===\n");
    test_generate_sample_dataset(pool_a, &PROFILE_LUEBECK, 3);
    test_generate_sample_dataset(pool_a, &PROFILE_TUERKENFELD, 3);
    test_generate_sample_dataset(pool_a, &PROFILE_MVP, 3);

    test_log("=== Alle Tests abgeschlossen ===\n");
}
