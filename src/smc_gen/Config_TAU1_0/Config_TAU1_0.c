/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2021, 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_TAU1_0.c
* Component Version: 1.6.0
* Device(s)        : R7F124FPJ5xFB
* Description      : This file implements device driver for Config_TAU1_0.
***********************************************************************************************************************/
/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_TAU1_0.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_TAU1_0_Create
* Description  : This function initializes the TAU1 channel0 module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU1_0_Create(void)
{
    TPS1 &= _FFF0_TAU_CKM0_CLEAR;
    TPS1 |= _0000_TAU_CKM0_FCLK_0;
    TT1 |= (_0002_TAU_CH1_STOP_TRG_ON | _0001_TAU_CH0_STOP_TRG_ON);
    TMMK10 = 1U;    /* disable INTTM10 interrupt */
    TMIF10 = 0U;    /* clear INTTM10 interrupt flag */
    TMMK11 = 1U;    /* disable INTTM11 interrupt */
    TMIF11 = 0U;    /* clear INTTM11 interrupt flag */
    /* Set INTTM10 low priority */
    TMPR110 = 1U;
    TMPR010 = 1U;
    /* Set INTTM11 low priority */
    TMPR111 = 1U;
    TMPR011 = 1U;
    /* Channel 0 is used as master channel for PWM output function */
    TMR10 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_TRIGGER_SOFTWARE | 
            _0001_TAU_MODE_PWM_MASTER;
    TDR10 = _0F9F_TAU_TDR10_VALUE;
    TO1 &= (uint16_t)~_0001_TAU_CH0_OUTPUT_VALUE_1;
    TOE1 &= (uint16_t)~_0001_TAU_CH0_OUTPUT_ENABLE;
    /* Channel 1 is used as slave channel for PWM output function */
    TMR11 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_COMBINATION_SLAVE | 
            _0400_TAU_TRIGGER_MASTER_INT | _0009_TAU_MODE_PWM_SLAVE;
    TDR11 = _0FA0_TAU_TDR11_VALUE;
    TOM1 |= _0002_TAU_CH1_SLAVE_OUTPUT;
    TOL1 &= (uint16_t)~_0002_TAU_CH1_OUTPUT_LEVEL_L;
    TO1 &= (uint16_t)~_0002_TAU_CH1_OUTPUT_VALUE_1;
    PWMDLY2 &= _FFF3_TAU_CH1_OUTPUT_CLEAR;
    PWMDLY2 |= _0000_TAU_CH1_OUTPUT_NO_DELAY;
    TOE1 |= _0002_TAU_CH1_OUTPUT_ENABLE;
    /* Set TO11 pin */
    PSRSEL &= 0xFDU;
    POM1 &= 0xFBU;
    P1 &= 0xFBU;
    PM1 &= 0xFBU;

    R_Config_TAU1_0_Create_UserInit();
}

/***********************************************************************************************************************
* Function Name: R_Config_TAU1_0_Start
* Description  : This function starts the TAU1 channel0 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU1_0_Start(void)
{
    TMIF10 = 0U;    /* clear INTTM10 interrupt flag */
    TMMK10 = 0U;    /* enable INTTM10 interrupt */
    TMIF11 = 0U;    /* clear INTTM11 interrupt flag */
    TMMK11 = 0U;    /* enable INTTM11 interrupt */
    TOE1 |= _0002_TAU_CH1_OUTPUT_ENABLE;
    TS1 |= (_0002_TAU_CH1_START_TRG_ON | _0001_TAU_CH0_START_TRG_ON);
}

/***********************************************************************************************************************
* Function Name: R_Config_TAU1_0_Stop
* Description  : This function stops the TAU1 channel0 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU1_0_Stop(void)
{
    TT1 |= (_0002_TAU_CH1_STOP_TRG_ON | _0001_TAU_CH0_STOP_TRG_ON);
    TOE1 &= (uint16_t)~_0002_TAU_CH1_OUTPUT_ENABLE;
    TMMK10 = 1U;    /* disable INTTM10 interrupt */
    TMIF10 = 0U;    /* clear INTTM10 interrupt flag */
    TMMK11 = 1U;    /* disable INTTM11 interrupt */
    TMIF11 = 0U;    /* clear INTTM11 interrupt flag */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

