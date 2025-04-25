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
    /* configuration structure initialization */ // 输出控制端口
    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;         // 上拉
    stcGpioInit.u16PinDir = PIN_DIR_OUT;       // 输出方向
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // 数字
    //    stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;   //coms强输出
    //    stcGpioInit.u16ExtInt = PIN_EXTINT_OFF;   //不使用外部中断
    //    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;   //高速

    stcGpioInit.u16PinState = PIN_STAT_RST; // 拉低

    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_01, &stcGpioInit); // OUTPUT_EN
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_00, &stcGpioInit); // POWER_DOWN_N
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_03, &stcGpioInit); // INPUT_RESET_N

    //////////////////Line
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;         // 上拉
    stcGpioInit.u16PinDir = PIN_DIR_IN;        // 输入方向
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // 数字
    stcGpioInit.u16PinState = PIN_STAT_SET;    // 拉低

    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_05, &stcGpioInit);

    ///////////////ASW/////////////////////
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_OFF;        // 上拉
    stcGpioInit.u16PinDir = PIN_DIR_OUT;       // 输出方向
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // 数字
    stcGpioInit.u16PinState = PIN_STAT_RST;    // 拉低
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;

    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW1, &stcGpioInit); // ASW01
    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW3, &stcGpioInit); // ASW03
    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW4, &stcGpioInit); // ASW04

    asw_02 = Read_ASW2();
    //    stcGpioInit.u16PullUp = PIN_PU_OFF;         // 上拉
    if (asw_02)
    {
        stcGpioInit.u16PinState = PIN_STAT_SET; // 拉高
    }
    else
    {
        stcGpioInit.u16PinState = PIN_STAT_RST; // 拉低
    }
    (void)GPIO_Init(GPIO_PORT_ASW, GPIO_PIN_ASW2, &stcGpioInit); // ASW02

    Video_ReadNot2(1);

    AVsw = Read_AVSW();
    if (AVsw)
    {
        stcGpioInit.u16PullUp = PIN_PU_OFF;     // 不上拉
        stcGpioInit.u16PinState = PIN_STAT_RST; // 拉低
                                                //        led_state = LED_ALL;
    }
    else
    {
        stcGpioInit.u16PullUp = PIN_PU_ON;      // 上拉
        stcGpioInit.u16PinState = PIN_STAT_SET; // 拉高
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
    static uint8_t key_up = 1; // 按键松开标志
    if (mode == 1)
        key_up = 1; // 支持连按

    if (key_up && (GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06) == 0))
    {
        DDL_DelayMS(10);
        key_up = 0;
        if (GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06) == 0)
            return 1;
    }
    else if (GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_06) == 1)
        key_up = 1;
    return 0; // 无按键按下
}

// 启用自动检测某些制式
void enable_auto_detection(uint8_t enable)
{
    static uint8_t art[2];
    if (enable)
    {
        // 启用PAL、NTSC和SECAM的自动检测
        art[0] = AUTO_DETECT_REG;
        art[1] = AD_PAL_EN | AD_NTSC_EN | AD_SECAM_EN | 0x80;
        (void)I2C_Master_Transmit(DEVICE_ADDR, art, 2, TIMEOUT);
    }
    else
    {
        // 禁用自动检测
        art[0] = AUTO_DETECT_REG;
        art[1] = 0x00;
        (void)I2C_Master_Transmit(DEVICE_ADDR, art, 2, TIMEOUT);
    }
}
uint8_t adv7280_7391_240p_config[] = {
    // **关闭 I2P（保持隔行输入）**
    0x84, 0x55, 0x00, // 关闭 I2P
    0x84, 0x5A, 0x02, // 确保 I2P 关闭

    // **ADV7391 240p 输出设置**
    0x56, 0x30, 0x14, // 设定 ITU-R BT.601 240p 输出
    0x56, 0x31, 0x01, // 使能像素数据输出
    0x56, 0x80, 0x10, // Luma SSAF 1.3MHz 滤波
    0x56, 0x82, 0xC9, // 标清主动视频边缘控制 & SD 像素数据有效
    0x56, 0x87, 0x20, // 启用 SD 自动检测
    0x56, 0x88, 0x00, // 8-bit YCbCr 输入
};

void printBinary(unsigned char num)
{
    // 假设我们要输出32位的二进制数
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--)
    {
        // 使用位运算检查每一位
        putchar((num & (1U << i)) ? '1' : '0');
    }
    putchar('\n'); // 输出换行符
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
            //     0x42, 0x0a, 0xe0, // new 亮度   00(00)  7F(+30)  80(-30)    e0
            //     0x42, 0x08, 0x58, // new 对比度 00(00)  80(01)   FF(02)     58
            //     0x42, 0xe3, 0x60, // new 饱和度 00(00)  80(01)   FF(02)     80
            //     0x42, 0x0B, 0x00, // new 色调   80(00)  00(-312) ff(+312)
            // };
            // static uint8_t BUFF1[] = {
            //     0x42, 0x0E, 0x00, // Re-enter map
            //     0x42, 0x0a, 0x00, // new 亮度   00(00)  7F(+30)  80(-30)    e0
            //     0x42, 0x08, 0x80, // new 对比度 00(00)  80(01)   FF(02)     58
            //     0x42, 0xe3, 0x80, // new 饱和度 00(00)  80(01)   FF(02)     80
            //     0x42, 0x0B, 0x00, // new 色调   80(00)  00(-312) ff(+312)
            // };
            // // new 亮度   00(00)  7F(+30)  80(-30)    e0
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
