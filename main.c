/**********************************************************************************************************************
 * \file main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are solely in the form of
 * machine-executable object code generated by a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "mtb_hal.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* Flag control */
#define ONE_SECOND            ((TCPWM_COUNTER_config.period + 1) / 10)

/*********************************************************************************************************************/
/*-------------------------------------------------Global Variables--------------------------------------------------*/
/*********************************************************************************************************************/
/* Interrupt configuration */
const cy_stc_sysint_t IRQ_CFG =
{
    .intrSrc = ((NvicMux3_IRQn << CY_SYSINT_INTRSRC_MUXIRQ_SHIFT) | TCPWM_COUNTER_IRQ),
    .intrPriority = 2UL
};

/* For the Retarget -IO (Debug UART) usage */
static cy_stc_scb_uart_context_t    UART_context;          /** UART context */
static mtb_hal_uart_t               UART_hal_obj;          /** Debug UART HAL object */

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/**********************************************************************************************************************
 * Function Name: handle_TCPWM_IRQ
 * Summary:
 *  TCPWM interrupt handler function. Checks interrupt status and prints the information out to terminal.
 * Parameters:
 *  none
 * Return:
 *  none
 **********************************************************************************************************************
 */
void handle_TCPWM_IRQ(void)
{
    /* Get interrupt source */
    uint32_t intrMask = Cy_TCPWM_GetInterruptStatusMasked(TCPWM_COUNTER_HW, TCPWM_COUNTER_NUM);

    /* Clear interrupt source */
    Cy_TCPWM_ClearInterrupt(TCPWM_COUNTER_HW, TCPWM_COUNTER_NUM, intrMask);

    /* Rising edge capture */
    if (0UL != (CY_TCPWM_INT_ON_CC0 & intrMask))
    {
    	uint32_t capturedCounter = Cy_TCPWM_Counter_GetCapture0Val(TCPWM_COUNTER_HW, TCPWM_COUNTER_NUM);
        uint32_t duration = TCPWM_COUNTER_config.period - capturedCounter;
        if (duration < ONE_SECOND)
        {
            printf("********************************************** Pushdown time = %ld ms  \r\n", duration);
        }
        else
        {
            printf("********************************************** Pushdown time = %ld.%ld s  \r\n"
            , (duration / 1000), (duration % 1000));
        }
    }
    /* Underflow */
    else if (0UL != (CY_TCPWM_INT_ON_TC & intrMask))
    {
        printf("############################################## Counter has been stopped.\r\n");
    }
}

/**********************************************************************************************************************
 * Function Name: main
 * Summary:
 *  This is the main function. TCPWM Capture and its interrupt configuration is implemented here.
 * Parameters:
 *  none
 * Return:
 *  int
 **********************************************************************************************************************
 */
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Debug UART init */
    result = (cy_rslt_t)Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);

    /* UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&UART_hal_obj, &UART_hal_config, &UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&UART_hal_obj);

    /* HAL retarget_io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }


    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("****************** "
           "TCPWM Counter Capture Functionality "
           "****************** \r\n");

    /*TCPWM Counter Mode initial*/
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_Counter_Init(TCPWM_COUNTER_HW, TCPWM_COUNTER_NUM, &TCPWM_COUNTER_config))
    {
        CY_ASSERT(0);
    }

    /* Interrupt settings */
    Cy_SysInt_Init(&IRQ_CFG, &handle_TCPWM_IRQ);
    NVIC_SetPriority((IRQn_Type) NvicMux3_IRQn, 2UL);
    NVIC_EnableIRQ((IRQn_Type) NvicMux3_IRQn);

    /* Enable the initialized counter */
    Cy_TCPWM_Counter_Enable(TCPWM_COUNTER_HW, TCPWM_COUNTER_NUM);

    printf(" The time it takes to press and release user button is displayed below. \r\n");
    printf(" 10 seconds after pressed, the measurement will stop. \r\n\n");

    for (;;)
    {
    }
}

/* [] END OF FILE */
