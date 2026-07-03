#include "DAVE.h"

#include "IoTapp.h"
#include "Html.h"

/** Pulse recording
 *  The ticks (~6.94ns) will be measured between a PWM signal and the
 *  rising edge of a signal that was sent along a propagation line.
 *  It is assumed that the delay toggles between a high and a low
 *  value if the delay line is tuned appropriately.
 */

/** ToDo (- open, + done)
 *
 *  - tuning resource settings
 *  - 2-way communication with a peer (PC)
 *  - save 2-sec. statistics to SD card, add temperature and delay_ps
 *  - toggle display between high value/switching
 *
 *  + sent lookup table after re-calibration
 *  + determine Delay_fs for Run cycle 96 sec.
 *  + add positions for min/max of high value/switching
 *  + replace travel by max burst/pause in Zoom and Run
 *  + sent Ucc
 *  + sent Delay_ps
 *  + sent max burst/pause and position
 *  + show max burst/pause in Stat and Cycle
 *  + determine max burst/pause and position
 *  + signal edge stability (differential?) -> A2 pad
 *  + fix reboot
 */

/* delay line serial control pins */
#define AE_PIN P1_9
#define SC_PIN P1_1
#define SI_PIN P1_0

/* delay line characteristics */
#define DELAY_INHERENT_FS	63250000
#define DELAY_STEP_FS		250000
#define DELAY_STEPS			256
#define DELAY_STEPS_MIN		9
#define DELAY_FORERUN		8

/* delay adjust */
#define HIGH_VALUE_AVG_LO	250
#define HIGH_VALUE_AVG_HI	750
#define SWITCHING_MIN		150
#define SMOOTHING_FACTOR	10

/* display params */
#define ZOOM_THRESHOLD      40
#define ZOOM_OFFSET	    	54
#define BURST_OFFSET       -14
#define PAUSE_OFFSET        88
#define BURST_GAIN         100
#define PAUSE_GAIN          15

/* acquisition cycle length [ns] */
#define ACQ_CYCLE    		2500

/* Number of buffers for decoupling acquisition from further processing */
#define ACQ_COUNT			3

/* Maximal value count in acquisition buffers */
#define ACQ_LENGTH			1000

/* number of buffers for decoupling statistics from data storage */
#define STAT_COUNT			2

/* Maximal value count in statistics buffers */
#define STAT_LENGTH			800

/* maximal values */
#define MAX_HIGH_VALUE		0x7FFF
#define MAX_SWITCHING		0x7FFFFFFF


/* acquisition buffers */
struct _Acquisition{
	/* delay of rising edge */
	int16_t Delay[ACQ_LENGTH];		// delay against PWM signal
	int16_t HighValueDelay;			// basic delay of High values against PWM signal
	/* high value and switching */
	int16_t HighValue;				// high value count (= info about timeflow level)
	int16_t Switching;				// switching number between high/low values (= info about timeflow spectrum)
	/* burst extremes */
	int16_t BurstLength;			// number of subsequent switching at tick frequency
	int16_t PauseLength;			// number of subsequent same values (+ high, - low)
	int16_t MaxBurstLength;			// max number of subsequent switching at tick frequency
	int16_t MaxPauseLength;			// max number of subsequent same values (+ high, - low)
	int16_t MaxBurstPos;			// position of subsequent switching at tick frequency
	int16_t MaxPausePos;			// position of subsequent same values without switching
	/* buffer control */
	int16_t Position;				// current buffer position
	bool isProcessing;				// request for processing (from acq interrupt)
	bool isRecording;				// request for saving to disk  (from acq cycle interrupt)

} Acquisition[ACQ_COUNT];

/* statistics buffers */
struct _Statistics{
	/* high value/switching and max burst/pause in 2.5 ms acquisition cycles */
	int16_t HighValue[STAT_LENGTH];
	int16_t Switching[STAT_LENGTH];
	int16_t MaxBurst[STAT_LENGTH];
	int16_t MaxPause[STAT_LENGTH];
	/* high value statistics */
	int64_t HighValueSum;			// sum of high value numbers
	int64_t HighValueAvg;			// average of high value numbers
	int16_t HighValueMin;			// minimal number of high values
	int16_t HighValueMax;			// maximal number of high values
	int16_t HighValueMinPos_ms;		// position of minimal number of high values
	int16_t HighValueMaxPos_ms;		// position of maximal number of high values
	/* switching statistics */
	int64_t SwitchingSum;			// sum of switching number between high/low values
	int64_t SwitchingAvg;			// average of switching number between high/low values (x10)
	int32_t SwitchingMin;			// minimal switching number between high/low values
	int32_t SwitchingMax;			// maximal switching number between high/low values
	int32_t SwitchingMinPos_ms;		// position of minimal switching number between high/low values
	int32_t SwitchingMaxPos_ms;		// position of maximal switching number between high/low values
	/* burst/pause extremes */
	int32_t MaxBurstLength;			// number of subsequent switching at tick frequency
	int32_t MaxPauseLength;			// number of same subsequent values without switching
	int32_t MaxBurstPos_ns;			// position of subsequent switching at tick frequency
	int32_t MaxPausePos_ns;			// position of same subsequent values without switching

	/* operation params */
	int32_t Temperature_mK;			// ambient temperature [mK]
	int32_t DelayLine_ps;			// programmed nominal delay [ps] in the delay circuit

	/* measurement */
	int64_t DelayAvg_fs;			// estimated signal delay [fs] in the network cable

	/* stat control */
	int32_t Position;				// position index in the high value and switching buffers

} Statistics[STAT_COUNT];

/* zoom workspace/buffers, operates on 2-sec. intervals*/
int32_t ZoomPos = 0;
int16_t ZoomAvg[ZOOM_LENGTH];
int16_t ZoomMin[ZOOM_LENGTH];
int16_t ZoomMax[ZOOM_LENGTH];
int16_t ZoomBurst[ZOOM_LENGTH];
int16_t ZoomPause[ZOOM_LENGTH];

/* run workspace/buffers, operates on 96-sec. intervals*/
int32_t RunCount = 0;
int32_t RunHighValueAvg = 0;
int32_t RunHighValueMin = MAX_HIGH_VALUE;
int32_t RunHighValueMax = 0;
int32_t RunHighValueMinPos_ms = 0;
int32_t RunHighValueMaxPos_ms = 0;
int32_t RunSwitchingAvg = 0;
int32_t RunSwitchingMin = MAX_SWITCHING;
int32_t RunSwitchingMax = 0;
int32_t RunSwitchingMinPos_ms = 0;
int32_t RunSwitchingMaxPos_ms = 0;
int32_t RunMaxBurstLength = 0;
int32_t RunMaxPauseLength = 0;
int32_t RunMaxBurstPos_us = 0;
int32_t RunMaxPausePos_us = 0;
int32_t RunPos = 0;
int16_t RunAvg[RUN_LENGTH];
int16_t RunMin[RUN_LENGTH];
int16_t RunMax[RUN_LENGTH];
int16_t RunBurst[RUN_LENGTH];
int16_t RunPause[RUN_LENGTH];

/* conversion from High count to delay time */
int64_t ClockCycle_fs;
int32_t Lookup_fs[ACQ_LENGTH+1];

/* delay line setup */
int32_t DelayLineStep[DELAY_STEPS];
int32_t DelayLineResponse_fs[DELAY_STEPS];
int32_t HighValueResponse[DELAY_STEPS];
int32_t StepsOutOfRange = 0;
int32_t StepIncrement = 10;
int32_t LookupIndexMin = -1;
int32_t LookupIndexMax = -1;
int32_t LookupIndexSet = -1;
int32_t LookupIndexHighs = 0;
int32_t ValidSignalCount = 0;

/* acquisition/processing control */
int AcquisitionIdx;						// acquisition buffer index
int ProcessingIdx;						// processing buffer index
int SavingIdx;							// saving buffer index

/* workspace Acq IRQ handler */
struct _Acquisition *Acq;				// pointer to acquisition buffer
int16_t Position;						// position in Delay buffers
int16_t Delay;							// edge delay
int16_t PrevDelay;						// previous edge delay
bool isHighValue;						// whether current delay is a high value
bool isPrevHighValue;					// whether previous delay was a high value

/* workspace Acq Cycle IRQ handler */
struct _Acquisition *Proc;				// pointer to processing buffer
struct _Statistics *Stat;				// pointer to statistics buffer
bool isEdge = false;					// whether any edge was detected
int32_t ProcessedCycles;				// processed acquisition cycles
int32_t Incomplete;						// cycles below acq buffer length values
int32_t RunRangeSmoothed;				// smoothed range for zoom estimation

/* called every 2.5 µs */
void AcqIRQHandler(void)
{
	/* query delay */
	Delay = XMC_CCU4_SLICE_GetCaptureRegisterValue(CCU4_SLICE_CONFIG_DIRECT.slice_ptr, 1U) & CCU4_CC4_CV_CAPTV_Msk;

	/* process delay values */
	Acq = &Acquisition[AcquisitionIdx];
	if(false == Acq->isProcessing){
		/* write delay value to buffer */
		Acq->Delay[Acq->Position] = Delay;
		/* count high value and switching */
		Delay > PrevDelay ? isHighValue = true : Delay < PrevDelay ? isHighValue = false : 0;
		PrevDelay = Delay;
		isHighValue ? Acq->HighValue++ : 0;
		/* switching vs. pause */
		if(isPrevHighValue ^ isHighValue){
			/* switching */
			Acq->BurstLength++;
			if(Acq->PauseLength > 0){
				if(Acq->PauseLength > abs(Acq->MaxPauseLength)){
					Acq->MaxPauseLength = isPrevHighValue ? Acq->PauseLength : -Acq->PauseLength;
					Acq->MaxPausePos = Acq->Position;
				}
				Acq->PauseLength = 0;
			}
		}else{
			/* pause */
			Acq->PauseLength++;
			if(Acq->BurstLength > 0){
				Acq->Switching += Acq->BurstLength;
				if(Acq->BurstLength > Acq->MaxBurstLength){
					Acq->MaxBurstLength = Acq->BurstLength;
					Acq->MaxBurstPos = Acq->Position;
				}
				Acq->BurstLength = 0;
			}
		}
		isPrevHighValue = isHighValue;
		/* increment buffer position */
		if(++(Acq->Position) >= ACQ_LENGTH){
			/* switch acquisition buffer forward */
			if(++AcquisitionIdx >= ACQ_COUNT){
				AcquisitionIdx = 0;
			}
			/* mark recent buffer for processing */
			TotalCycles_++;
			Acq->isProcessing = true;
		}
	}
}

void EdgeIRQHandler(void)
{
	isEdge = true;
}

/* called every ~2.5 ms */
void AcqCycleIRQHandler(void)
{
	/* processing  */
	Proc = &Acquisition[ProcessingIdx];
	while(Proc->isProcessing){

		/* determine delay of high values */
		Proc->HighValueDelay = Proc->Position > 0 ? Proc->Delay[0] : 0;
		if(Proc->Switching > 0){
			/* find first switching */
			for(int i=0; i<ACQ_LENGTH; i++){
				if(Proc->Delay[i] < Proc->HighValueDelay) break;
				if(Proc->Delay[i] > Proc->HighValueDelay){
					Proc->HighValueDelay = Proc->Delay[i];
					break;
				}
			}
		}
		HighValueDelay_ = Proc->HighValueDelay;

		/* finalize burst/pause */
		if(Proc->BurstLength > 0){
			Proc->Switching += Proc->BurstLength;
			if(Proc->BurstLength > Proc->MaxBurstLength){
				Proc->MaxBurstLength = Proc->BurstLength;
				Proc->MaxBurstPos = Proc->Position;
			}
		}
		if(Proc->PauseLength > 0){
			if(Proc->PauseLength > abs(Proc->MaxPauseLength)){
				Proc->MaxPauseLength = Proc->Delay[ACQ_LENGTH-1] >= Proc->HighValueDelay ? Proc->PauseLength : -Proc->PauseLength;
				Proc->MaxPausePos = Proc->Position;
			}
		}

		/* statistics */
		Stat = &Statistics[SavingIdx];

		/* check acq buffer completeness, true every ~2.5 ms */
		if(Proc->Position >= ACQ_LENGTH){
			ProcessedCycles++;

			/* save high value/switching and max burst/pause for every 2.5 ms acquisition cycle */
			Stat->HighValue[Stat->Position] = Proc->HighValue;
			Stat->Switching[Stat->Position] = Proc->Switching;
			Stat->MaxBurst[Stat->Position] = Proc->MaxBurstLength;
			Stat->MaxPause[Stat->Position] = Proc->MaxPauseLength;

			/* update statistics, multiply HighAvg by 1000 to get three digits after comma */
			int Cycles = Stat->Position + 1;
			if(Cycles > 1){
				/* high value */
				Stat->HighValueSum += Proc->HighValue;
				Stat->HighValueAvg = (1000 * Stat->HighValueSum) / Cycles;
				if(Stat->HighValueMin > Proc->HighValue){
					Stat->HighValueMin = Proc->HighValue;
					Stat->HighValueMinPos_ms = (Stat->Position * ACQ_CYCLE) / 1000;
				}
				if(Stat->HighValueMax < Proc->HighValue){
					Stat->HighValueMax = Proc->HighValue;
					Stat->HighValueMaxPos_ms = (Stat->Position * ACQ_CYCLE) / 1000;
				}
				/* switching */
				Stat->SwitchingSum += Proc->Switching;
				Stat->SwitchingAvg = Stat->SwitchingSum / Cycles;
				if(Stat->SwitchingMin > Proc->Switching){
					Stat->SwitchingMin = Proc->Switching;
					Stat->SwitchingMinPos_ms = (Stat->Position * ACQ_CYCLE) / 1000;
				}
				if(Stat->SwitchingMax < Proc->Switching){
					Stat->SwitchingMax = Proc->Switching;
					Stat->SwitchingMaxPos_ms = (Stat->Position * ACQ_CYCLE) / 1000;
				}
				/* burst/pause extremes */
				if(Proc->MaxBurstLength > Stat->MaxBurstLength){
					Stat->MaxBurstLength = Proc->MaxBurstLength;
					Stat->MaxBurstPos_ns = (Proc->MaxBurstPos + Stat->Position * ACQ_LENGTH) * ACQ_CYCLE;
				}
				if(abs(Proc->MaxPauseLength) > abs(Stat->MaxPauseLength)){
					Stat->MaxPauseLength = Proc->MaxPauseLength;
					Stat->MaxPausePos_ns = (Proc->MaxPausePos + Stat->Position * ACQ_LENGTH) * ACQ_CYCLE;
				}
			}else
			{ /* init new cycle */
				/* high value */
				Stat->HighValueSum = Proc->HighValue;
				Stat->HighValueAvg = 1000 * Proc->HighValue;
				Stat->HighValueMin = Proc->HighValue;
				Stat->HighValueMax = Proc->HighValue;
				Stat->HighValueMinPos_ms = 0;
				Stat->HighValueMaxPos_ms = 0;
				/* switching */
				Stat->SwitchingSum = Proc->Switching;
				Stat->SwitchingAvg = Proc->Switching;
				Stat->SwitchingMin = Proc->Switching;
				Stat->SwitchingMax = Proc->Switching;
				Stat->SwitchingMinPos_ms = 0;
				Stat->SwitchingMaxPos_ms = 0;
				/* burst/pause */
				Stat->MaxBurstLength = Proc->MaxBurstLength;
				Stat->MaxBurstPos_ns = Proc->MaxBurstPos * ACQ_CYCLE;
				Stat->MaxPauseLength = Proc->MaxPauseLength;
				Stat->MaxPausePos_ns = Proc->MaxPausePos * ACQ_CYCLE;
			}

			/* check end of statistics cycle, usually every ~2 sec. */
			if(++(Stat->Position) >= STAT_LENGTH){

				/* publish cycle buffer */
				for (int i=0; i<ACQ_LENGTH; i++ ){
					CyclePlot_[i] = Proc->Delay[i];
				}

				/* publish high value statistics buffer */
				for (int i=0; i<STAT_LENGTH; i++ ){
					StatPlot_[i] = Stat->HighValue[i];
				}

				/* publish Run buffers */
				for (int i=0; i<RUN_LENGTH; i++ ){
					// run index
					int RunIndex = RunPos + i - (RunPos + i < RUN_LENGTH ? 0 : RUN_LENGTH);
					// avg
					RunAvgPlot_[i] = RunAvg[RunIndex];
					RunAvgPlot_[i] < RUN_RANGE_LO ? RunAvgPlot_[i] = RUN_RANGE_LO : RunAvgPlot_[i] >= RUN_RANGE_HI ? RunAvgPlot_[i] = RUN_RANGE_HI : 0;
					// min
					RunMinPlot_[i] = RunMin[RunIndex];
					RunMinPlot_[i] < RUN_RANGE_LO ? RunMinPlot_[i] = RUN_RANGE_LO : RunMinPlot_[i] >= RUN_RANGE_HI ? RunMinPlot_[i] = RUN_RANGE_HI : 0;
					// max
					RunMaxPlot_[i] = RunMax[RunIndex];
					RunMaxPlot_[i] < RUN_RANGE_LO ? RunMaxPlot_[i] = RUN_RANGE_LO : RunMaxPlot_[i] >= RUN_RANGE_HI ? RunMaxPlot_[i] = RUN_RANGE_HI : 0;
					// burst
					RunBurstPlot_[i] = (RUN_RANGE * (BURST_OFFSET + (BURST_GAIN*RunBurst[RunIndex])/100))/100;
					RunBurstPlot_[i] < RUN_RANGE_LO ? RunBurstPlot_[i] = RUN_RANGE_LO : RunBurstPlot_[i] >= RUN_RANGE_HI ? RunBurstPlot_[i] = RUN_RANGE_HI : 0;
					// pause
					RunPausePlot_[i] = (RUN_RANGE * (PAUSE_OFFSET + (PAUSE_GAIN*RunPause[RunIndex])/100))/100;
					RunPausePlot_[i] < RUN_RANGE_LO ? RunPausePlot_[i] = RUN_RANGE_LO : RunPausePlot_[i] >= RUN_RANGE_HI ? RunPausePlot_[i] = RUN_RANGE_HI : 0;
				}

				/* estimate appropriate zoom factor */
				int ZoomGain = RunLabelY_ / ZoomLabelY_;
				if(isDelayLocked_){
					int32_t RunRange = ZOOM_THRESHOLD*(HighValueMax_ - HighValueMin_)/10;
					(RunRangeSmoothed < RunRange/2) || (RunRangeSmoothed > RunRange*2) ? RunRangeSmoothed = RunRange : 0;
					RunRangeSmoothed += (RunRange - RunRangeSmoothed) / ZOOM_SMOOTH;
					if(RunRangeSmoothed > 0){
						RunRangeSmoothed > ZoomLabelY_ ? ZoomGain = RunLabelY_ / RunRangeSmoothed : 0;
						RunRangeSmoothed < RunLabelY_ / (ZoomGain+1) ? ZoomGain = ZoomGain+1 : 0;
						ZoomGain < 1 ? ZoomGain = 1 : 0;
						ZoomLabelY_ = RunLabelY_ / ZoomGain;
					}
				}else{
					ZoomLabelY_ = RunLabelY_;
					ZoomGain = 1;
				}

				/* publish Zoom buffer */
				int Precision = 100;
				int RunRangePrecision = RUN_RANGE * Precision;
				int ZoomOffset = (ZOOM_OFFSET*ZOOM_RANGE)/100;
				int ZoomIndex = ZoomPos-1 < 0 ? ZOOM_LENGTH : ZoomPos-1;
				int ZoomAvgSmooth = Precision * (ZoomAvg[ZoomIndex] - ZOOM_RANGE/2);
				for (int i=ZOOM_LENGTH-2; i>=0; i-- ){
					// zoom index
					ZoomIndex = ZoomPos + i - (ZoomPos + i < ZOOM_LENGTH ? 0 : ZOOM_LENGTH);
					// avg
					int ZoomAvgValue = Precision * (ZoomAvg[ZoomIndex] - ZOOM_RANGE/2);
					ZoomAvgPlot_[i] = ZoomOffset + ZoomGain * (ZoomAvgValue - ZoomAvgSmooth) * ZOOM_RANGE / RunRangePrecision;
					ZoomAvgPlot_[i] < 0 ? ZoomAvgPlot_[i] = 0 : ZoomAvgPlot_[i] > ZOOM_RANGE ? ZoomAvgPlot_[i] = ZOOM_RANGE : 0;
					// min
					int ZoomMinValue = Precision * (ZoomMin[ZoomIndex] - ZOOM_RANGE/2);
					ZoomMinPlot_[i] = ZoomOffset + ZoomGain * (ZoomMinValue - ZoomAvgSmooth) * ZOOM_RANGE / RunRangePrecision;
					ZoomMinPlot_[i] < 0 ? ZoomMinPlot_[i] = 0 : ZoomMinPlot_[i] > ZOOM_RANGE ? ZoomMinPlot_[i] = ZOOM_RANGE : 0;
					// max
					int ZoomMaxValue = Precision * (ZoomMax[ZoomIndex] - ZOOM_RANGE/2);
					ZoomMaxPlot_[i] = ZoomOffset + ZoomGain * (ZoomMaxValue - ZoomAvgSmooth) * ZOOM_RANGE / RunRangePrecision;
					ZoomMaxPlot_[i] < 0 ? ZoomMaxPlot_[i] = 0 : ZoomMaxPlot_[i] > ZOOM_RANGE ? ZoomMaxPlot_[i] = ZOOM_RANGE : 0;
					// burst
					ZoomBurstPlot_[i] = (ZOOM_RANGE * (BURST_OFFSET + (BURST_GAIN * ZoomBurst[ZoomIndex])/100))/100;
					ZoomBurstPlot_[i] <= 0 ? ZoomBurstPlot_[i] = 1 : ZoomBurstPlot_[i] >= ZOOM_RANGE ? ZoomBurstPlot_[i] = ZOOM_RANGE-1 : 0;
					// pause
					ZoomPausePlot_[i] = (ZOOM_RANGE * (PAUSE_OFFSET + (PAUSE_GAIN * ZoomPause[ZoomIndex])/100))/100;
					ZoomPausePlot_[i] <= 0 ? ZoomPausePlot_[i] = 1 : ZoomPausePlot_[i] >= ZOOM_RANGE ? ZoomPausePlot_[i] = ZOOM_RANGE-1 : 0;
					// smoothed reference
					ZoomAvgSmooth = ZoomAvgSmooth + (ZoomAvgValue - ZoomAvgSmooth)/ZOOM_SMOOTH;
				}

				/* publish counters */
				Cycles_ = TotalCycles_;
				LostCycles_ = TotalCycles_ > ProcessedCycles + 2 ? TotalCycles_ - ProcessedCycles : 0;

				/* publish statistics if any edge was detected */
				if(isEdge){
					Stat = &Statistics[SavingIdx];
					/* show cycle statistics */
					HighValueAvg_ = Stat->HighValueAvg;
					HighValueMin_ = Stat->HighValueMin == MAX_HIGH_VALUE ? 0 : Stat->HighValueMin;
					HighValueMax_ = Stat->HighValueMax;
					HighValueMinPos_ms_ = Stat->HighValueMinPos_ms;
					HighValueMaxPos_ms_ = Stat->HighValueMaxPos_ms;
					SwitchingAvg_ = Stat->SwitchingAvg;
					SwitchingMin_ = Stat->SwitchingMin == MAX_SWITCHING ? 0 : Stat->SwitchingMin;
					SwitchingMax_ = Stat->SwitchingMax;
					SwitchingMinPos_ms_ = Stat->SwitchingMinPos_ms;
					SwitchingMaxPos_ms_ = Stat->SwitchingMaxPos_ms;
					MaxBurstLength_ = Stat->MaxBurstLength;
					MaxPauseLength_ = Stat->MaxPauseLength;
					MaxBurstPos_us_ = Stat->MaxBurstPos_ns/1000;
					MaxPausePos_us_ = Stat->MaxPausePos_ns/1000;
					// high avg/min/max pixel position on statistics/cycle display
					HighValueAvgSmooth_px_ = 40 + HighValueAvgSmooth_/1000 - 100;
					HighValueAvgSmooth_px_ < 40 ? HighValueAvgSmooth_px_ = 40 : HighValueAvgSmooth_px_ > 840 ? HighValueAvgSmooth_px_ = 840 : 0;
					HighValueBeg_px_ = 40 + HighValueMin_ - 100;
					HighValueBeg_px_ < 40 ? HighValueBeg_px_ = 40 : HighValueBeg_px_ > 840 ? HighValueBeg_px_ = 840 : 0;
					HighValueLen_px_ = 40 + HighValueMax_ - HighValueBeg_px_ - 100;
					HighValueLen_px_ < 0 ? HighValueLen_px_ = 0 : HighValueBeg_px_ + HighValueLen_px_ > 840 ? HighValueLen_px_ = 840 - HighValueBeg_px_ : 0;
					// burst/pause pixel position on statistics/cycle display
					MaxBurstPos_px_ = 40 + MaxBurstPos_us_ / ACQ_CYCLE;
					MaxPausePos_px_ = 40 + MaxPausePos_us_ / ACQ_CYCLE;
					MaxBurstLen_px_ = MaxBurstLength_ < 0 ? 0 : MaxBurstLength_ > 1000 ? 1000 : MaxBurstLength_;
					MaxPauseLen_px_ = abs(MaxPauseLength_) > 1000 ? 1000 : abs(MaxPauseLength_);
					MaxBurstBeg_px_ = 40 + 2 * (MaxBurstPos_us_ % ACQ_CYCLE) / 5 - MaxBurstLen_px_;
					MaxBurstBeg_px_ < 40 ? MaxBurstBeg_px_ = 40 : MaxBurstBeg_px_ > 1040 - MaxBurstLength_ ? MaxBurstBeg_px_ = 1040 - MaxBurstLength_ : 0;
					MaxPauseBeg_px_ = 40 + 2 * (MaxPausePos_us_ % ACQ_CYCLE) / 5 - MaxPauseLen_px_;
					MaxPauseBeg_px_ < 40 ? MaxPauseBeg_px_ = 40 : MaxPauseBeg_px_ > 1040 - MaxPauseLen_px_ ? MaxPauseBeg_px_ = 1040 - MaxPauseLen_px_ : 0;
					MaxBurstTxt_px_ = MaxBurstBeg_px_ < 42 ? 42 : MaxBurstBeg_px_ > 960 ? 960 : MaxBurstBeg_px_;
					MaxPauseTxt_px_ = MaxPauseBeg_px_ < 42 ? 42 : MaxPauseBeg_px_ > 990 ? 990 : MaxPauseBeg_px_;

					// maintain delay line
					isDelayLocked_ ? verifyDelayLine() : setupDelayLine();

				}else{
					/* suppress cycle statistics */
					Delay_fs_ = 0;
					HighValueAvgSmooth_ = 0;
					HighValueAvgSmooth_px_ = 40;
					HighValueAvg_ = 0;
					HighValueMin_ = 0;
					HighValueMax_ = 0;
					HighValueMinPos_ms_ = 0;
					HighValueMaxPos_ms_ = 0;
					SwitchingAvg_ = 0;
					SwitchingMin_ = 0;
					SwitchingMax_ = 0;
					SwitchingMinPos_ms_ = 0;
					SwitchingMaxPos_ms_ = 0;
					MaxBurstLength_ = 0;
					MaxPauseLength_ = 0;
					MaxBurstPos_us_ = 0;
					MaxPausePos_us_ = 0;
					MaxBurstBeg_px_ = 40;
					MaxPauseBeg_px_ = 40;
					MaxBurstLen_px_ = 0;
					MaxPauseLen_px_ = 0;
					MaxBurstPos_px_ = 40;
					MaxPausePos_px_ = 40;
					MaxBurstTxt_px_ = 42;
					MaxPauseTxt_px_ = 42;
				}
				isEdge = false;

				/* reset cycle position */
				Statistics[SavingIdx].Position = 0;

				/* cumulate zoom values */
				// buffers high value
				ZoomAvg[ZoomPos] = isDelayLocked_ ? HighValueAvg_/1000 - RUN_RANGE/2 : 0;
				ZoomMin[ZoomPos] = isDelayLocked_ ? HighValueMin_ - RUN_RANGE/2 : 0;
				ZoomMax[ZoomPos] = isDelayLocked_ ? HighValueMax_ - RUN_RANGE/2 : 0;
				// buffers burst/pause
				ZoomBurst[ZoomPos] = isDelayLocked_ ? MaxBurstLength_ : 0;
				ZoomPause[ZoomPos] = isDelayLocked_ ? MaxPauseLength_ : 0;
				// switch zoom buffer forward
				if(++ZoomPos >= ZOOM_LENGTH){
					ZoomPos = 0;
				}

				/* cumulate run values */
				// high value
				RunHighValueAvg += HighValueAvg_;
				if(RunHighValueMin > HighValueMin_){
					RunHighValueMin = HighValueMin_;
					RunHighValueMinPos_ms = HighValueMinPos_ms_ + RunCount * (STAT_LENGTH * ACQ_CYCLE / ACQ_LENGTH);
				}
				if(RunHighValueMax < HighValueMax_){
					RunHighValueMax = HighValueMax_;
					RunHighValueMaxPos_ms = HighValueMaxPos_ms_ + RunCount * (STAT_LENGTH * ACQ_CYCLE / ACQ_LENGTH);
				}
				// switching
				RunSwitchingAvg += SwitchingAvg_;
				if(RunSwitchingMin > SwitchingMin_){
					RunSwitchingMin = SwitchingMin_;
					RunSwitchingMinPos_ms = SwitchingMinPos_ms_ + RunCount * (STAT_LENGTH * ACQ_CYCLE / ACQ_LENGTH);
				}
				if(RunSwitchingMax < SwitchingMax_){
					RunSwitchingMax = SwitchingMax_;
					RunSwitchingMaxPos_ms = SwitchingMaxPos_ms_ + RunCount * (STAT_LENGTH * ACQ_CYCLE / ACQ_LENGTH);
				}
				/* max burst/pause */
				if(MaxBurstLength_ > RunMaxBurstLength){
					 RunMaxBurstLength = MaxBurstLength_;
					 RunMaxBurstPos_us = MaxBurstPos_us_ + RunCount * STAT_LENGTH * ACQ_CYCLE;
				}
				if(abs(MaxPauseLength_) > abs(RunMaxPauseLength)){
					RunMaxPauseLength = MaxPauseLength_;
					RunMaxPausePos_us = MaxPausePos_us_ + RunCount * STAT_LENGTH * ACQ_CYCLE;
				}
				// next Run cycle
				RunCount = isDelayLocked_ ? RunCount + 1 : 0;

				/* publish run values every ~96 sec. */
				if(RunCount >= CYCLE_MESSAGE_FTL/2){

					// high
					RunHighValueAvg_ = RunHighValueAvg / (1000 * RunCount);
					RunHighValueMin_ = RunHighValueMin;
					RunHighValueMax_ = RunHighValueMax;
					RunHighValueMinPos_ms_ = RunHighValueMinPos_ms;
					RunHighValueMaxPos_ms_ = RunHighValueMaxPos_ms;
					// switching
					RunSwitchingAvg_ = RunSwitchingAvg / (1000 * RunCount);
					RunSwitchingMin_ = RunSwitchingMin;
					RunSwitchingMax_ = RunSwitchingMax;
					RunSwitchingMinPos_ms_ = RunSwitchingMinPos_ms;
					RunSwitchingMaxPos_ms_ = RunSwitchingMaxPos_ms;
					// burst/pause
					RunMaxBurstLength_ = RunMaxBurstLength;
					RunMaxPauseLength_ = RunMaxPauseLength;
					RunMaxBurstPos_us_ = RunMaxBurstPos_us;
					RunMaxPausePos_us_ = RunMaxPausePos_us;
					// buffers high value
					RunAvg[RunPos] = RunHighValueAvg_;
					RunMin[RunPos] = RunHighValueMin_;
					RunMax[RunPos] = RunHighValueMax_;
					// buffers burst/pause
					RunBurst[RunPos] = RunMaxBurstLength_;
					RunPause[RunPos] = RunMaxPauseLength_;

					// estimate RunDelay_fs for a ~96-sec. cycle
					RunDelay_fs_ = (HighValueDelay_ + CYCLES_SKIPPED) * ClockCycle_fs - 1000 * Tuning_ps_;
					int32_t RunHighValueDelay = RunHighValueAvg / RunCount;
					int32_t Index = RunHighValueDelay/1000;
					int32_t Fraction = RunHighValueDelay%1000;
					RunDelay_fs_ += RunSwitchingMin_ > 0 ? Lookup_fs[Index] + (Index < ACQ_LENGTH ? ((Lookup_fs[Index+1] - Lookup_fs[Index]) * Fraction) / 1000 : 0) : 0;

					/* reset run workspace */
					RunCount = 0;
					// high value
					RunHighValueAvg = 0;
					RunHighValueMin = MAX_HIGH_VALUE;
					RunHighValueMax = 0;
					RunHighValueMinPos_ms = 0;
					RunHighValueMaxPos_ms = 0;
					// switching
					RunSwitchingAvg = 0;
					RunSwitchingMin = MAX_SWITCHING;
					RunSwitchingMax = 0;
					RunSwitchingMinPos_ms = 0;
					RunSwitchingMaxPos_ms = 0;
					// burst/pause
					RunMaxBurstLength = 0;
					RunMaxPauseLength = 0;
					RunMaxBurstPos_us = 0;
					RunMaxPausePos_us = 0;

					/* switch run buffer forward */
					if(++RunPos >= RUN_LENGTH){
						RunPos = 0;
					}

					// signal 'run values ready' for next message FTL cycle
					osSignalSet(NetconnClientThreadID_, SIGNAL_MESSAGE_FTL);
				}

				// TEST: travel delay values systematically
//				if(ZoomPos %3 == 0){
//					// increment lookup index
//					LookupIndex_ = (LookupIndex_ + 1) % DELAY_STEPS ;
//					// set new delay
//					setDelayStep(DelayLineStep[LookupIndex_]);
//					Tuning_ps_ = (DELAY_INHERENT_FS + DelayLineStep[LookupIndex_] * DELAY_STEP_FS)/1000;
//					// activate measurement
//					isDelayLocked_ = true;
//				}
			}
		}

		/* reset buffer for next acquisition */
		Proc->HighValueDelay = 0;
		Proc->HighValue = 0;
		Proc->Switching = 0;
		Proc->BurstLength = 0;
		Proc->PauseLength = 0;
		Proc->MaxBurstLength = 0;
		Proc->MaxPauseLength = 0;
		Proc->MaxBurstPos = 0;
		Proc->MaxPausePos = 0;
		Proc->Position = 0;
		Proc->isProcessing = false;

		/* switch processing buffer forward */
		if(++ProcessingIdx >= ACQ_COUNT){
			ProcessingIdx = 0;
		}
		Proc = &Acquisition[ProcessingIdx];
	}
}

void verifyDelayLine()
{
	/* estimate a smoothed HighAvg */
	HighValueAvgSmooth_ += HighValueAvgSmooth_ > 0 ? (HighValueAvg_ - HighValueAvgSmooth_) / SMOOTHING_FACTOR : (HIGH_VALUE_AVG_HI + HIGH_VALUE_AVG_LO) * 1000 / 2;

	/* HighAvg range check */
	if(HighValueAvgSmooth_ >= 1000*HIGH_VALUE_AVG_LO && HighValueAvgSmooth_ <= 1000*HIGH_VALUE_AVG_HI){
		// inside range: refine delay [fs]
		Delay_fs_ = (Proc->HighValueDelay + CYCLES_SKIPPED) * ClockCycle_fs - 1000*Tuning_ps_;
		int32_t Index = Stat->HighValueAvg/1000;
		int32_t Fraction = Stat->HighValueAvg%1000;
		Delay_fs_ += Proc->Switching > 0 ? Lookup_fs[Index] + (Index < ACQ_LENGTH ? ((Lookup_fs[Index+1] - Lookup_fs[Index]) * Fraction) / 1000 : 0) : 0;
	}else{
		/* out-of-range: trigger new delay line setup */
		Delay_fs_ = 0;
		LookupIndex_ = (LookupIndexMin - DELAY_FORERUN + DELAY_STEPS) % DELAY_STEPS ;
		LookupIndexMin = -1;
		LookupIndexMax = -1;
		LookupIndexSet = -1;
		LookupIndexHighs = 0;
		ValidSignalCount = 0;
		isDelayLocked_ = false;
		/*  clear high value response */
		for(int i=0; i<DELAY_STEPS; i++){
			HighValueResponse[i] = 0;
		}
	}
}

void setupDelayLine()
{
	bool isSignalValid = SwitchingMax_ > SWITCHING_MIN;

	// save current high value response
	isSignalValid ? HighValueResponse[LookupIndex_] = HighValueAvg_ : 0;

	// set start of delay step range
	isSignalValid && LookupIndexMin < 0 ? LookupIndexMin = LookupIndex_ : 0;
	// reset start of delay step range
	if((StepsOutOfRange > DELAY_FORERUN) && ValidSignalCount < DELAY_STEPS_MIN){
		LookupIndexMin = -1;
		LookupIndexMax = -1;
		LookupIndexSet = -1;
		LookupIndexHighs = 0;
		ValidSignalCount = 0;
	}
	// check for end of delay line setup
	if((LookupIndexSet >= 0) && (ValidSignalCount >= DELAY_STEPS_MIN) && (StepsOutOfRange > DELAY_FORERUN)){
		// end-of-setup: lock best available delay step
		LookupIndex_ = LookupIndexSet;
		HighValueAvgSmooth_ = 0;
		isDelayLocked_ = true;
		// fill calibration array
		int SizeCalHighValue = sizeof(CalHighValue_) / sizeof(int32_t);
		for(int i = 0; i < SizeCalHighValue; i++){
			int LookupIndex = LookupIndex_ - SizeCalHighValue/2 + i;
			LookupIndex = LookupIndex < 0 ? LookupIndex + DELAY_STEPS : LookupIndex >= DELAY_STEPS ? LookupIndex - DELAY_STEPS : LookupIndex;
			CalHighValue_[i] = HighValueResponse[LookupIndex];
		}
		isCalibration = true;
		// signal 'calibration ready' for sending a calibration message
		osSignalSet(NetconnClientThreadID_, SIGNAL_MESSAGE_FTL);
	}else{
		// continue looking for optimal delay line step
		if(isSignalValid){
			ValidSignalCount++;
			// find optimal delay step
			if(abs(HighValueAvg_ - 1000*ACQ_LENGTH/2) < abs(LookupIndexHighs - 1000*ACQ_LENGTH/2)){
				LookupIndexHighs = HighValueAvg_;
				LookupIndexSet = LookupIndex_;
			}
			// reduce step increment
			if(StepIncrement > 1){
				LookupIndex_ -= (StepIncrement + 1);
				StepIncrement = 1;
			}
		}else{
			// increase step increment
			if(StepIncrement == 1 && StepsOutOfRange > DELAY_FORERUN){
					StepIncrement = 10;
					LookupIndexMin = -1;
					LookupIndexMax = -1;
					LookupIndexSet = -1;
					LookupIndexHighs = 0;
					ValidSignalCount = 0;
			}
		}
		// set end of delay step range
		isSignalValid && LookupIndexMin >= 0 ? LookupIndexMax = LookupIndex_ : 0;
		// advance lookup index
		LookupIndex_ = (LookupIndex_ + StepIncrement) % DELAY_STEPS;
		// count out-of-range steps
		StepsOutOfRange = isSignalValid ? 0 : StepIncrement > 1 ? 0 : StepsOutOfRange + 1;

	}
	setDelayStep(DelayLineStep[LookupIndex_]);
	Inherent_ps_ = DELAY_INHERENT_FS / 1000;
	Delay_ps_ = (DelayLineStep[LookupIndex_] * DELAY_STEP_FS)/1000;
	Tuning_ps_ = Inherent_ps_ + Delay_ps_;
	Phase_deg_ =  (3600*((Tuning_ps_*1000) % (ClockCycle_fs))) / ClockCycle_fs;
}

void setDelayStep(int _DelayStep)
{
	// publish new delay step
	DelayStep_ = _DelayStep;

	// enable serial input
	XMC_GPIO_SetOutputLow(SC_PIN);
	XMC_GPIO_SetOutputHigh(AE_PIN);

	// transfer 8 bits (~20µs)
	for(int i=0; i<8; i++){
		// clock low
		XMC_GPIO_SetOutputLow(SC_PIN);
		// set output bit
		_DelayStep *= 2;
		_DelayStep & 0x100 ? XMC_GPIO_SetOutputHigh(SI_PIN) : XMC_GPIO_SetOutputLow(SI_PIN);
		// clock high
		XMC_GPIO_SetOutputHigh(SC_PIN);
	}

	// activate new delay
	XMC_GPIO_SetOutputLow(AE_PIN);
}

void updateResponse(int16_t *_ResponsePlot, int _ResponseLength, int _ResponseRange)
{
	/* center response */
	int Offset = _ResponseLength >= ACQ_LENGTH ? 0 : (ACQ_LENGTH - _ResponseLength)/2;

	/* return centered values from lookup table */
	for(int i=0; i<ACQ_LENGTH-2*Offset; i++){
		_ResponsePlot[i] = _ResponseRange/2 + (Lookup_fs[i+Offset] * _ResponseRange) / MEASURE_EDGE;
	}
}

void initPulse(void)
{
	/* acquisition control */
	AcquisitionIdx = 0;
	ProcessingIdx = 0;
	SavingIdx = 0;

	/* acquisition buffers */
	for(int b=0; b<ACQ_COUNT; b++){
		Proc = &Acquisition[b];
		/* buffer statistics */
		Proc->HighValue = 0;
		Proc->Switching = 0;
		Proc->HighValueDelay = 0;
		/* buffer control */
		Proc->Position = 0;
		Proc->isProcessing = false;
		Proc->isRecording = false;
	}

	/* reset control values */
	isCalibration = false;
	isDelayLocked_ = false;
	StepIncrement = 10;
	LookupIndexMin = -1;
	LookupIndexMax = -1;
	LookupIndexSet = -1;
	LookupIndexHighs = 0;
	ValidSignalCount = 0;
	RunLabelY_ = 1200;
	ZoomLabelY_ = RunLabelY_;

	/* reset analog */
	Solar_ = DIGIT_SOLAR;
	Battery_ = 0;
	Power_ = 0;
	PoE_ = 0;
	U_cycle_ = 0;
	U_delay_ = 0;

	/* reset statistics */
	TotalCycles_ = 0;
	ProcessedCycles = 0;
	Delay = 0;
	PrevDelay = 0;
	isHighValue = false;
	isPrevHighValue = false;
	HighValueAvgSmooth_ = 0;

	/* cycle statistics */
	Stat = &Statistics[SavingIdx];
	Stat->Position = 0;
	Stat->HighValueSum = 0;
	Stat->HighValueAvg = 0;
	Stat->HighValueMin = MAX_HIGH_VALUE;
	Stat->HighValueMax = 0;
	Stat->SwitchingSum = 0;
	Stat->SwitchingAvg = 0;
	Stat->SwitchingMin = MAX_SWITCHING;
	Stat->SwitchingMax = 0;

	/* run buffers */
	for(int i=0; i<ACQ_LENGTH; i++){
		RunAvg[i] = RUN_RANGE/2;
		RunMin[i] = RUN_RANGE/2;
		RunMax[i] = RUN_RANGE/2;
		RunBurst[i] = RUN_RANGE/2;
		RunPause[i] = RUN_RANGE/2;
	}

	/* determine delay conversion factor based on nominal clock frequency */
	ClockCycle_fs = 1000000000 / 144;
	LookupIndex_ = 0;

	/* fill lookup table according to edge discrimination characteristics */
	int32_t EdgeStep_fs = MEASURE_EDGE / MEASURE_STEPS;
	int32_t MarginStep_fs = (MEASURE_RANGE - MEASURE_EDGE)/(ACQ_LENGTH - MEASURE_STEPS);
	int32_t Center = ACQ_LENGTH/2;
	int32_t MarginLo = (ACQ_LENGTH - MEASURE_STEPS)/2;
	int32_t MarginHi = ACQ_LENGTH - MarginLo;
	for(int i=0; i<=MEASURE_STEPS/2; i++){
		Lookup_fs[MarginLo + i] = -(Center - MarginLo - i)*EdgeStep_fs;
		Lookup_fs[MarginHi - i] = (MarginHi - Center - i)*EdgeStep_fs;
	}
	for(int i=0; i<MarginLo; i++){
		Lookup_fs[i] = Lookup_fs[MarginLo] - (MarginLo - i)*MarginStep_fs;
		Lookup_fs[ACQ_LENGTH-i] = Lookup_fs[MarginHi] + (ACQ_LENGTH - MarginHi - i)*MarginStep_fs;
	}

	/* initialize nominal delay line response */
	DelayLineResponse_fs[0] = ClockCycle_fs;
	for(int i=1; i<DELAY_STEPS; i++){
		DelayLineStep[i] = 0;
		DelayLineResponse_fs[i] = (DELAY_INHERENT_FS + i * DELAY_STEP_FS) % ClockCycle_fs;
	}
	/* sorting delay line response */
	int DelayResponse = 0;
	for(int i=1; i<DELAY_STEPS; i++){
		for( int j=DELAY_STEPS-1; j>0; j--){
			if((DelayLineResponse_fs[j] >= DelayResponse) && (DelayLineResponse_fs[j] <= DelayLineResponse_fs[DelayLineStep[i]])){
				DelayLineStep[i] = j;
			}
		}
		DelayResponse = DelayLineResponse_fs[DelayLineStep[i]];
		DelayLineResponse_fs[DelayLineStep[i]] = ClockCycle_fs; // mark as used
	}
	/* update delay response, clear high value response */
	for(int i=0; i<DELAY_STEPS; i++){
		DelayLineResponse_fs[i] = (DELAY_INHERENT_FS + DelayLineStep[i] * DELAY_STEP_FS) % ClockCycle_fs;
		HighValueResponse[i] = 0;
	}
}

