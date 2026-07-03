/*
 * TempS.h
 *
 *  Created on: Oct 21, 2015
 *      Author:
 */

#ifndef MODULES_XMC_TEMPSENSOR_TEMPS_H_
#define MODULES_XMC_TEMPSENSOR_TEMPS_H_

#include <xmc_scu.h>

void TMPS_Disable(void);
void TMPS_Enable(void);
XMC_SCU_STATUS_t  TMPS_StartMeasurement(void);
bool TMPS_IsBusy(void);
uint32_t  TMPS_ReadTemp (void);
uint32_t  TMPS_UpdateTemp ();


#endif /* MODULES_XMC_TEMPSENSOR_TEMPS_H_ */
