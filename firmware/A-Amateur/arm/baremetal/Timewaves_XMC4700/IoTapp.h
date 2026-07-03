#include "TempS.h"
#include "Pulse.h"

/* version */
#define FIRMWARE_VERSION	"v1.0.1"

/* ---version history---
 * v1.0.1	2025-10	firmware version added
 * v1.0.0	2021-03 stable version, distributed to #51 #58 #60 #61 #78 #84
*/


/* ---program settings--- */

/* device */
#define DEVICE_ID  			IP_ADDR3
#define NETWORK_IP 			IP_ADDR2	// Vierow: 42  MVP: 42  Greifswald: 42  Dambeck: 178  Lübeck: 42  Tuerkenfeld: 42  Munich: 178
#define TELEMETRY  			false
#define PING_TEST	  		false		// a ping will be sent unconditionally (without data acquisition running) every 30 sec.
#define LAN_TEST        	false       // client will connect to server on LAN address 192.168.42.121

/* automatic TEST extension after subject */
#if PING_TEST==true
	#define SUBJECT_TEST	"TEST"
#else
	#define SUBJECT_TEST	""
#endif

/* measurement range [fs] */
#define MEASURE_RANGE		1200000
#define MEASURE_EDGE		600000
#define MEASURE_STEPS		600

/* LED control */
#define LED2_PIN P5_8

/* signal that a message should be sent by netconn client */
#define SIGNAL_MESSAGE_FTL	0x0001
#define CYCLE_MESSAGE_FTL	96

// DELTA_T (D-T) temperature offset [0.1 deg]
//--------------------------------------------------------------------
// #ID	Location		D-T	Locator	SN			UURI
//--------------------------------------------------------------------
// #50	Tuerkenfeld		180	JN58nc  599007983	EKD@JN58nc_Tuerkenfeld
// #51	Tuerkenfeld		180	JN58nc  591101575	EKD@JN58nc_Tuerkenfeld
// #52	Tuerkenfeld		214	JN58nc  			EKD@JN58nc_Tuerkenfeld
// #56	Dambeck			170	JO63rx				EKD@JO63rx_Dambeck
// #58	Greifswald		170	JO64qb				EKD@JO64qb_Greifswald
// #59	MVP				210	JO63sx				EKD@JO63sx_MVP
// #61	MVP				143	JO63sx				EKD@JO63sx_MVP
// #78	Luebeck			205	JO53jv  591101435	SUG@JO53jv_Luebeck
// #79	Vierow			170	JO64sc				EKD@JO64sc_Vierow
// #80	Munich			170	JN58sd	599007971	EKD@JN58sd_Munich
// #84	Munich			208	JN58re				SHKG@JN58re_Munich
// #97	Karlsburg		225	JO63tx				EKD@JO63tx_Karlsburg
// #98	Karlsburg		160	JO63tx				EKD@JO63tx_Karlsburg

// admin
#define DATA_OP "EKD@JN58nc_Tuerkenfeld"

// operator
#if DEVICE_ID==50

#endif
#if DEVICE_ID==51
	#define OPERATOR "EKD@JN58nc_Tuerkenfeld"
	#define DELTA_T  170
#endif
#if DEVICE_ID==52
	#define DELTA_T  234
#endif
#if DEVICE_ID==56
	#define OPERATOR "EKD@JO63rx_Dambeck"
#endif
#if DEVICE_ID==58
	#define OPERATOR "EKD@JO64qb_Greifswald"
#endif
#if DEVICE_ID==60
	#define OPERATOR "EKD@JO63sx_MVP"
	#define DELTA_T  210
	#define TELEMETRY  	true
#endif
#if DEVICE_ID==61
	#define OPERATOR "EKD@JO63sx_MVP"
	#define DELTA_T  143
	#define CYCLES_SKIPPED	0
#endif
#if DEVICE_ID==78
	#define OPERATOR "SUG@JO53jv_Luebeck"
	#define DELTA_T  205
#endif
#if DEVICE_ID==79
	#define OPERATOR "EKD@JO64sc_Vierow"
#endif
#if (DEVICE_ID==80)
	#define OPERATOR "EKD@JN58sd_Munich"
#endif
#if (DEVICE_ID==84)
	#define OPERATOR "SHKG@JN58re_Munich"
	#define DELTA_T  208
#endif
#if DEVICE_ID==97
	#define OPERATOR "EKD@JO63tx_Karlsburg"
	#define DELTA_T  225
#endif
#if DEVICE_ID==98
	#define OPERATOR "EKD@JO63tx_Karlsburg"
	#define DELTA_T  160
#endif

// default defines
#ifndef OPERATOR
	#define OPERATOR "EKD@JN58nc_Tuerkenfeld"
#endif
#ifndef CYCLES_SKIPPED
	#define CYCLES_SKIPPED	0
#endif
#ifndef DELTA_T
	#define DELTA_T  170
#endif

// digit range for telemetry values
#define DIGIT_FULL_RANGE	4095
#define DIGIT_SOLAR			3912
#define DIGIT_BATTERY		3459
#define DIGIT_I_POE	 		474

// ratio for telemetry values [µV,µA/digits]
#define RATIO_SOLAR			(69360000 / DIGIT_SOLAR)
#define RATIO_BATTERY		(15650000 / DIGIT_BATTERY)
#define RATIO_SHUNT	 		(463000 / DIGIT_I_POE)
#define RATIO_I_POE 		(4630000 / DIGIT_I_POE)
#define RATIO_U_POE			(59900000 / DIGIT_FULL_RANGE)
#define RATIO_U_CYCLE		(6500000 / DIGIT_FULL_RANGE)
#define RATIO_U_DELAY		(4500000 / DIGIT_FULL_RANGE)
// ratio for supply voltage
#define RATIO_Ucc5V			(6683000 / DIGIT_FULL_RANGE)
#define RATIO_Ucc12V		(15736000 / DIGIT_FULL_RANGE)

/* global variables */
#ifdef IoTappVariables
#define VARIABLE
#else
#define VARIABLE extern
#endif
// thread
VARIABLE int32_t Threads_;
VARIABLE sys_thread_t NetconnServerThreadID_;
VARIABLE sys_thread_t NetconnClientThreadID_;
// operation
VARIABLE int32_t DeviceID_;
VARIABLE int32_t PingID_;
VARIABLE int32_t Temperature_;
VARIABLE int32_t DeltaT_;
VARIABLE int32_t Ucc_;
VARIABLE int32_t Ucc5V_;
VARIABLE int32_t Ucc12V_;
VARIABLE int32_t Samples_;
VARIABLE int32_t Cycles_;
VARIABLE int32_t TotalCycles_;
VARIABLE int32_t LostCycles_;
VARIABLE int32_t Duty_;
VARIABLE bool isDelayLocked_;
// delay
VARIABLE int32_t HighValueDelay_;
VARIABLE int32_t Delay_fs_;
VARIABLE int32_t Phase_deg_;
VARIABLE int32_t Tuning_ps_;
VARIABLE int32_t Delay_ps_;
VARIABLE int32_t Inherent_ps_;
VARIABLE int32_t DelayStep_;
VARIABLE int32_t LookupIndex_;
// statistics
VARIABLE int32_t HighValueAvgSmooth_;
VARIABLE int32_t HighValueAvg_;
VARIABLE int32_t HighValueMin_;
VARIABLE int32_t HighValueMax_;
VARIABLE int32_t HighValueAvgSmooth_px_;
VARIABLE int32_t HighValueBeg_px_;
VARIABLE int32_t HighValueLen_px_;
VARIABLE int32_t HighValueMinPos_ms_;
VARIABLE int32_t HighValueMaxPos_ms_;
VARIABLE int32_t SwitchingAvg_;
VARIABLE int32_t SwitchingMin_;
VARIABLE int32_t SwitchingMax_;
VARIABLE int32_t SwitchingMinPos_ms_;
VARIABLE int32_t SwitchingMaxPos_ms_;
VARIABLE int32_t MaxBurstLength_;
VARIABLE int32_t MaxPauseLength_;
VARIABLE int32_t MaxBurstPos_us_;
VARIABLE int32_t MaxPausePos_us_;
VARIABLE int32_t MaxBurstBeg_px_;
VARIABLE int32_t MaxPauseBeg_px_;
VARIABLE int32_t MaxBurstLen_px_;
VARIABLE int32_t MaxPauseLen_px_;
VARIABLE int32_t MaxBurstPos_px_;
VARIABLE int32_t MaxPausePos_px_;
VARIABLE int32_t MaxBurstTxt_px_;
VARIABLE int32_t MaxPauseTxt_px_;
// run
VARIABLE int32_t RunDelay_fs_;
VARIABLE int32_t RunHighValueAvg_;
VARIABLE int32_t RunHighValueMin_;
VARIABLE int32_t RunHighValueMax_;
VARIABLE int32_t RunHighValueMinPos_ms_;
VARIABLE int32_t RunHighValueMaxPos_ms_;
VARIABLE int32_t RunSwitchingAvg_;
VARIABLE int32_t RunSwitchingMin_;
VARIABLE int32_t RunSwitchingMax_;
VARIABLE int32_t RunSwitchingMinPos_ms_;
VARIABLE int32_t RunSwitchingMaxPos_ms_;
VARIABLE int32_t RunMaxBurstLength_;
VARIABLE int32_t RunMaxPauseLength_;
VARIABLE int32_t RunMaxBurstPos_us_;
VARIABLE int32_t RunMaxPausePos_us_;
// plot arrays
VARIABLE int32_t StatLength_;
VARIABLE int16_t *StatPlot_;
VARIABLE int32_t ResponseLength_;
VARIABLE int16_t *ResponsePlot_;
VARIABLE int32_t CycleLength_;
VARIABLE int16_t *CyclePlot_;
VARIABLE int32_t ZoomLength_;
VARIABLE int32_t ZoomLabelY_;
VARIABLE int16_t *ZoomAvgPlot_;
VARIABLE int16_t *ZoomMinPlot_;
VARIABLE int16_t *ZoomMaxPlot_;
VARIABLE int16_t *ZoomBurstPlot_;
VARIABLE int16_t *ZoomPausePlot_;
VARIABLE int32_t RunLength_;
VARIABLE int32_t RunLabelY_;
VARIABLE int16_t *RunAvgPlot_;
VARIABLE int16_t *RunMinPlot_;
VARIABLE int16_t *RunMaxPlot_;
VARIABLE int16_t *RunBurstPlot_;
VARIABLE int16_t *RunPausePlot_;
// calibration
VARIABLE int32_t CalHighValue_[21];
VARIABLE bool isCalibration;
// solar power
VARIABLE int32_t Solar_;
VARIABLE int32_t Battery_;
VARIABLE int32_t Power_;
VARIABLE int32_t PoE_;
VARIABLE int32_t U_cycle_;
VARIABLE int32_t U_delay_;

/* run a web server and start a ping client */
void runIoTapp();

