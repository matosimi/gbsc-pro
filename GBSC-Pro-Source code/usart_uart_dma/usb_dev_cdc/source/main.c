/**
 *******************************************************************************
 * @file  usart/usart_uart_dma/source/main.c
 * @brief This example demonstrates UART data receive and transfer by DMA.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Delete the redundant code
                                    Read USART_DR.RDR when USART overrun error occur.
   2023-01-15       CDT             Update UART timeout function calculating formula for Timer0 CMP value
   2023-09-30       CDT             Split register USART_DR to USART_RDR and USART_TDR
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022-2023, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "main.h"

/**
 * @addtogroup HC32F460_DDL_Examples
 * @{
 */

/**
 * @addtogroup USART_UART_DMA
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                       LL_PERIPH_EFM | LL_PERIPH_SRAM)

extern uint16_t g_u16_sys_timer;
extern uint16_t g_u16_key_timer;
extern uint16_t g_u16_mis_timer;
extern uint16_t g_u16_osd_timer;

__IO uint8_t m_u8SpeedUpd = 0U;

// const uint8_t usFlashInitVal[4] __attribute__((at(0x00007FFC))) = {0x23, 0x01, 0x89, 0x67}; // 定位在flash中
// const uint8_t usFlashInitVal[4] __attribute__((at(IAP_BOOT_SIZE - 4))) = {0x23, 0x01, 0x89, 0x67}; // 定位在flash中

const uint8_t bright_atr[3] = {0x00, 0x7f, 0x80};
const uint8_t contrast_atr[3] = {0x00, 0x80, 0xff};
static uint8_t BrightCount = 0x00;
static uint8_t ContrastCount = 0xb0;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

static void vd_Button_Init(void)
{
    /* configuration structure initialization */ // Output control port
    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;         // Pull-up
    stcGpioInit.u16PinDir = PIN_DIR_OUT;       // Output direction
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // Digital
    //    stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;   //cmos strong output
    //    stcGpioInit.u16ExtInt = PIN_EXTINT_OFF;   //Do not use external interrupts
    //    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;   //High 

    stcGpioInit.u16PinState = PIN_STAT_RST; // 拉低

    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_01, &stcGpioInit); // OUTPUT_EN
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_00, &stcGpioInit); // POWER_DOWN_N
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_03, &stcGpioInit); // INPUT_RESET_N

    //////////////////Line
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;         // Pull-up
    stcGpioInit.u16PinDir = PIN_DIR_IN;        // Input direction
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // Digital
    stcGpioInit.u16PinState = PIN_STAT_SET;    // Set

    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_05, &stcGpioInit);

    ///////////////ASW/////////////////////
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_OFF;        // Pull-up
    stcGpioInit.u16PinDir = PIN_DIR_OUT;       // Output direction
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // Digital
    stcGpioInit.u16PinState = PIN_STAT_RST;    // Reset
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;

    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW1, &stcGpioInit); // ASW01
    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW3, &stcGpioInit); // ASW03
    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW4, &stcGpioInit); // ASW04

    asw_02 = Read_ASW2();
    //    stcGpioInit.u16PullUp = PIN_PU_OFF;         // 上拉
    if (asw_02)
    {
        stcGpioInit.u16PinState = PIN_STAT_SET; 
    }
    else
    {
        stcGpioInit.u16PinState = PIN_STAT_RST; 
    }
    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW2, &stcGpioInit); // ASW02

    Video_ReadNot2(1);

    AVsw = Read_AVSW();
    if (AVsw)
    {
        stcGpioInit.u16PullUp = PIN_PU_OFF;     
        stcGpioInit.u16PinState = PIN_STAT_RST; 
                                                //        led_state = LED_ALL;
    }
    else
    {
        stcGpioInit.u16PullUp = PIN_PU_ON;      
        stcGpioInit.u16PinState = PIN_STAT_SET; 
                                                //        led_state = LED_RED;
    }
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_08, &stcGpioInit);
}

static void Key_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    /* configuration structure initialization */

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON; // KeyMode
    stcGpioInit.u16PinDir = PIN_DIR_IN;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    stcGpioInit.u16ExtInt = PIN_EXTINT_OFF;

    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_06, &stcGpioInit);
}

static void Read_Adv_7391Sw(void)
{
    static uint8_t key_state = 0, key_state_last = 0;

    key_state = GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06);
    if (key_state_last != key_state)
    {
        //        Video_Sw(key_state);
    }
    key_state_last = key_state;
}

static uint8_t Key_Read(uint8_t mode)
{
    static uint8_t key_up = 1; //Key release flag
    if (mode == 1)
        key_up = 1; //Support continuous pressing

    if (key_up && (GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06) == 0))
    {
        DDL_DelayMS(10);
        key_up = 0;
        if (GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06) == 0)
            return 1;
    }
    else if (GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06) == 1)
        key_up = 1;
    return 0; // No button pressed
}

// Enable automatic detection of certain formats
void enable_auto_detection(uint8_t enable)
{
    static uint8_t art[2];
    if (enable)
    {
        // Enable automatic detection of PAL, NTSC and SECAM
        art[0] = AUTO_DETECT_REG;
        art[1] = AD_PAL_EN | AD_NTSC_EN | AD_SECAM_EN | 0x80;
        (void)I2C_Master_Transmit(DEVICE_ADDR, art, 2, TIMEOUT);
    }
    else
    {
        // Disable automatic detection
        art[0] = AUTO_DETECT_REG;
        art[1] = 0x00;
        (void)I2C_Master_Transmit(DEVICE_ADDR, art, 2, TIMEOUT);
    }
}
uint8_t adv7280_7391_240p_config[] = {
    // **Turn off I2P (keep interlaced input)**
    0x84, 0x55, 0x00, // Turn off I2P
    0x84, 0x5A, 0x02, // Make sure I2P is off

    // **ADV7391 240p output settings**
    0x56, 0x30, 0x14, // Set ITU-R BT.601 240p output
    0x56, 0x31, 0x01, // Enable pixel data output
    0x56, 0x80, 0x10, // Luma SSAF 1.3MHz filter
    0x56, 0x82, 0xC9, // SD active video edge control & SD pixel data valid
    0x56, 0x87, 0x20, // Enable SD auto-detection
    0x56, 0x88, 0x00, // 8-bit YCbCr input
};

void printBinary(unsigned char num)
{
    // Assume we want to output a 32-bit binary number
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--)
    {
        // Use bitwise operations to check each bit
        putchar((num & (1U << i)) ? '1' : '0');
    }
    putchar('\n'); // Output newline character
}
/**
 * @brief  Main function of UART DMA project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t buff_main[2];
    stc_usart_uart_init_t stcUartInit;
    stc_irq_signin_config_t stcIrqSigninConfig;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);
    __enable_irq();
    EFM_FWMC_Cmd(ENABLE);
    BSP_CLK_Init();
    BSP_LED_Init();
    vd_Button_Init();
    Key_Init();
#if (LL_TMR0_ENABLE == DDL_ON)
    TMR02_A_Config();
#endif

#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
#endif
    i2c_init();
    TMR0_Start(CM_TMR0_2, TMR0_CHA);
    uart_dma_init();
    Video_Sw(adv_sw);
    Signal_led(Input_signal);
    for (;;)
    {

        if (Key_Read(0))
        {
            // static uint8_t count;
            // static uint8_t BUFF[] = {
            //     0x42, 0x0E, 0x00, // Re-enter map
            //     0x42, 0x0a, 0xe0, // new brightness 00(00)  7F(+30)  80(-30)    e0
            //     0x42, 0x08, 0x58, // new contrast   00(00)  80(01)   FF(02)     58
            //     0x42, 0xe3, 0x60, // new saturation 00(00)  80(01)   FF(02)     80
            //     0x42, 0x0B, 0x00, // new hue        80(00)  00(-312) ff(+312)
            // };
            // static uint8_t BUFF1[] = {
            //     0x42, 0x0E, 0x00, // Re-enter map
            //     0x42, 0x0a, 0x00, // new brightness 00(00)  7F(+30)  80(-30)    e0
            //     0x42, 0x08, 0x80, // new contrast   00(00)  80(01)   FF(02)     58
            //     0x42, 0xe3, 0x80, // new saturation 00(00)  80(01)   FF(02)     80
            //     0x42, 0x0B, 0x00, // new hue        80(00)  00(-312) ff(+312)
            // };
            // // new brightness   00(00)  7F(+30)  80(-30)    e0
            // if (++count >= 2)
            // {
            //     count = 0;
            // }
            // if (count)
            // {
            //     (void)ADV_7280_Send_Buff(BUFF, sizeof(BUFF) / 3, TIMEOUT);
            //     printf("1\n");
            // }
            // else
            // {
            //     (void)ADV_7280_Send_Buff(BUFF1, sizeof(BUFF1) / 3, TIMEOUT);
            //     printf("0\n");
            // }

    
            // static uint8_t buff[] = {0x41,0x00};
            // if(buff[1] == 0x41)
            // {
            //     buff[1] =0x01;
            // }
            // else 
            // {
            //     buff[1] =0x41;
            // }

            // (void)I2C_Master_Transmit(0x42, buff, 2, TIMEOUT); 
            // c_state = 1;
        }

        if (g_u16_sys_timer >= SYS_TIMEOUT_100MS) // 100ms
        {
            detect_loop();
            g_u16_sys_timer = 0;
        }
        if (g_u16_key_timer >= SYS_TIMEOUT_50MS) // 50MS
        {
            signal_turn();
            g_u16_key_timer = 0;
        }
        if (g_u16_mis_timer >= SYS_TIMEOUT_100MS) // 100ms
        {
            static uint8_t Input_signal_last = 0;
            if (c_state == 1)
                C_LED_OK();
            else if (c_state == 2)
                C_LED_ERR_RED();
            else if (c_state == 3)
                C_LED_ERR_GREEN();
            else if (c_state == 4)
                C_LED_ERR_BLUE();
            c_state = 0;
            if (Input_signal != Input_signal_last)
            {
                Signal_led(Input_signal);
                Input_signal_last = Input_signal;
            }
            g_u16_mis_timer = 0;
        }
        if (g_u16_osd_timer >= SYS_TIMEOUT_500MS) // 500MS   OSD
        {
            static uint8_t count;
            err_flag = 1;

            //            count++;
            //            if (count >= 3)
            //            {
            //                uint8_t buff = 0x10 ,buff_Re;
            //                count = 0;
            //                Chip_Receive(DEVICE_ADDR, &buff, &buff_Re, 1, TIMEOUT);
            //                printf("buff_Re 0x%02x \n",buff_Re);
            //
            ////                printf("ASW %d %d %d %d \n",
            ////                       GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1), GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW2), GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3), GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4));
            ////                printf(" AVsw %d\n" ,AVsw);
            //            }

            g_u16_osd_timer = 0;
        }
    }
}

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
