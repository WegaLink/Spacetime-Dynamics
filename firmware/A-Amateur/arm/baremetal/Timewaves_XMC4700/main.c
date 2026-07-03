/*
 * main.c
 *
 *  Created on: 2019 Sep 18 21:25:42
 *  Author: Eckhard
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <stdio.h>

#include "IoTapp.h"

/* LED1 blinking */
#define LED1_PIN P5_9
void led1_task(void const *args)
{
  while(1)
  {
	XMC_GPIO_ToggleOutput(LED1_PIN);
	osDelay(1000);
  }
}
osThreadDef(led1_task, osPriorityNormal, 1, 0);

/* user tasks initialization */
void main_task(void const *args)
{
  XMC_GPIO_SetMode(LED1_PIN, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
  XMC_GPIO_SetMode(LED2_PIN, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

  osThreadCreate(osThread(led1_task), NULL);

  runIoTapp();
}
osThreadDef(main_task, osPriorityNormal, 1, 0);


int main(void)
{
  DAVE_STATUS_t status;

  /* enable die temperature */
  TMPS_Enable();

  /* initialize pulse acquisition */
  initPulse();


  status = DAVE_Init();           /* Initialization of DAVE APPs  */

  if(status != DAVE_STATUS_SUCCESS)
  {
    /* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
    XMC_DEBUG("DAVE APPs initialization failed\n");

    while(1U)
    {

    }
  }

//  /* Configure A/D conversion with FIR or IIR filter */
//  ADC_MEASUREMENT_Load_pos_handle.res_handle->post_processing_mode = XMC_VADC_DMM_FILTERING_MODE;
//  ADC_MEASUREMENT_Load_pos_handle.res_handle->data_reduction_control = 15;

  /* Start A/D conversion */
  ADC_MEASUREMENT_StartConversion(&ADC_MEASUREMENT_0);

  /* force wait (min 40ms) to get LWIP online also after a reboot */
  int _Wait_ms = 50;
  for(int t = 0; t < _Wait_ms; t++){
	// wait 1 ms
	char Wait[32];
	for(int i=0;i<75;i++){
		sprintf(Wait,"wait: %d",_Wait_ms);
	}
  }

  osKernelInitialize();
  osThreadCreate(osThread(main_task), NULL);
  osKernelStart();
}

/* called every second */
void Time_Handler(void)
{
	/* update die temperature */
	Temperature_ = TMPS_UpdateTemp() - DELTA_T;

	// get time
	time_t Current;
	RTC_Time(&Current);

	/* every 96 seconds in test mode */
	if(PING_TEST && (0 == Current % 96)){
		// signal 'run values ready' for next message FTL cycle
		osSignalSet(NetconnClientThreadID_, SIGNAL_MESSAGE_FTL);
	}
	
//	// signal messages FTL at determined seconds in every minute
//	if(0 == ((Current - DEVICE_ID) % 60) % CYCLE_MESSAGE_FTL){
//		// signal next message FTL cycle
//		osSignalSet(NetconnClientThreadID_, SIGNAL_MESSAGE_FTL);
//	}
}

/*End of measurements interrupt*/
void Adc_Measurement_Handler(void)
{
  // telemetry
  static uint32_t result_counter;
  static uint32_t result_Solar;
  static uint32_t result_Battery;
  static uint32_t result_I_PoE;
  static uint32_t result_U_PoE;
  static uint32_t result_U_cycle;
  static uint32_t result_U_delay;
  // supply voltage
  static uint32_t result_Ucc5V;
  static uint32_t result_Ucc12V;

  /*Read out conversion results*/
  result_Solar += ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_Solar);
  result_Battery += ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_Battery);
  result_I_PoE += ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_I_PoE);
  result_U_PoE += ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_U_PoE);
  result_U_cycle += ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_U_cycle);
  result_U_delay += ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_U_delay);

  /* Re-trigger conversion sequence */
  ADC_MEASUREMENT_StartConversion(&ADC_MEASUREMENT_0);

  /* evaluate measure cycles */
#define MEASURE_CYCLES 50000
#define PRECISION      50
  if(++result_counter >= MEASURE_CYCLES){
	  // telemetry
	  if(TELEMETRY){
		  // post processing
		  result_Solar /= result_counter;
		  result_Battery /= result_counter;
		  result_U_PoE /= result_counter;
		  result_I_PoE *= PRECISION;
		  result_I_PoE /= result_counter;
		  result_U_cycle /= result_counter;
		  result_U_delay /= result_counter;

		  // forward conversion results
		  Solar_ = (result_Solar - DIGIT_SOLAR) * RATIO_SOLAR;
		  Battery_ = -result_Battery * RATIO_BATTERY;
		  // solar
		  Solar_ = Solar_ < 0 ? Solar_ + Battery_ : 0; // charge starts when solar cell exceeds battery voltage
		  // battery
		  Battery_ += ((result_I_PoE/PRECISION) - DIGIT_I_POE) * RATIO_SHUNT;
		  // Power
		  int32_t I_PoE = DIGIT_I_POE*PRECISION - result_I_PoE;
		  I_PoE < 0 ? I_PoE = 0 : 0;
		  Power_ = ((I_PoE * RATIO_I_POE)/(100*PRECISION)) * ((result_U_PoE * RATIO_U_POE)/10000);
		  // PoE
		  PoE_ = result_U_PoE * RATIO_U_POE;
		  // voltages 5V/3.3V
		  U_cycle_ = result_U_cycle; // * RATIO_U_CYCLE;
		  U_delay_ = result_U_delay; // * RATIO_U_DELAY;

		  // reset values
		  result_Solar = 0;
		  result_Battery = 0;
		  result_I_PoE = 0;
		  result_U_PoE = 0;
		  result_U_cycle = 0;
		  result_U_delay = 0;
	  }
	  else{
		  // supply voltage 5V
		  result_Ucc5V = result_I_PoE;
		  result_I_PoE = 0;
		  result_Ucc5V /= result_counter;
		  Ucc5V_ = result_Ucc5V * RATIO_Ucc5V;
		  Ucc5V_ < 3000000 ? Ucc5V_ = 0 : 0;
		  // PoE voltage 12V
		  result_Ucc12V = result_U_PoE;
		  result_U_PoE = 0;
		  result_Ucc12V /= result_counter;
		  Ucc12V_ = result_Ucc12V * RATIO_Ucc12V;
		  Ucc12V_ < 6000000 ? Ucc12V_ = 0 : 0;
		  // primary supply voltage
		  Ucc_ = Ucc5V_ < Ucc12V_ ? Ucc12V_ : Ucc5V_;
	  }
	  result_counter = 0;
  }
}

