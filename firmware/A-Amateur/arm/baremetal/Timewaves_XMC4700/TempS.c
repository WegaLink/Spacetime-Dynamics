/*
 * TempS.c
 *
 *  Created on: Oct 21, 2015
 *      Author:
 */

#include "TempS.h"
#include "stdlib.h"

int32_t Temperature = 0;
int32_t Threshold = 20;
int32_t Factor = 10;
int32_t Repeat = 3;

/**
 * This Function disables the Die Temperature Sensor
 */
inline void TMPS_Disable(void)
{
   /*Disabling the DTS*/
	XMC_SCU_DisableTemperatureSensor();
}

/**
 * This Function enables the Die Temperature Sensor
 */
inline void TMPS_Enable(void)
{
   /*Enabling the DTS*/
	XMC_SCU_EnableTemperatureSensor();
}

/**
 * This Function starts the measurement of the Die by setting the DTSCON->START
 * bit. But before starting, the sensor state has to be checked.If the sensor is
 * Powered and in IDLE state then only measurement will start.
 */
inline XMC_SCU_STATUS_t  TMPS_StartMeasurement(void)
{
  return (XMC_SCU_StartTemperatureMeasurement());
}


inline bool TMPS_IsBusy(void)
{
	return(XMC_SCU_IsTemperatureSensorBusy());
}

/** TMPS_ReadTemp.
 * This Function will read the measured temperature of the Die from  DTSSTAT
 * register and returns the value by converting it into 0.1 Degrees Celsius.
 * Basic granularity is limited to about 0.5 Degrees.
 *
 * @Return - temperature [0.1 deg]
 */
uint32_t  TMPS_ReadTemp ()
{
   volatile uint32_t Temp;
   /*Initialize the value*/
   Temp = 0U;
   if (XMC_SCU_IsTemperatureSensorBusy())
    {
        Temp  = XMC_SCU_GetTemperatureMeasurement();
        Temp  = ((Temp - 605U)*1000U)/ 205U ;
    }
   return (Temp);
}

/** TMPS_UpdateTemp.
 * The temperature sensor will be sampled Count times and
 * an average value will be estimated and returned.
 *
 * @Count - number of measurements for averaging.
 * @Return - average temperature [0.1 deg]
 *
 */
uint32_t  TMPS_UpdateTemp ()
{
	if(XMC_SCU_IsTemperatureSensorReady())
	{
		int32_t Value = 0;
		int32_t Count = Repeat;
		while(Count--){
			TMPS_StartMeasurement();
			while (!XMC_SCU_IsTemperatureSensorReady()){};
			Value = TMPS_ReadTemp();
			if(abs(Temperature - Value * Factor) > Threshold * Factor){
				Temperature = Value * Factor;
			}else{
				Temperature += (Value * Factor - Temperature) / Factor;
			}
		}
	}
	return Temperature / Factor;
}


