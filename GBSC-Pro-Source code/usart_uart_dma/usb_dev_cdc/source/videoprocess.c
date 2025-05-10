#include "videoprocess.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"
#include "main.h"

static uint8_t status = 0;
uint8_t Adv_7391_sw = 0;
uint8_t adv_input;
uint8_t adv_double = true;
uint8_t adv_smooth = false;
uint8_t adv_ace = false;
uint8_t adv_sw = false;
uint8_t adv_tv = 0xff;
uint8_t err_flag = 0;
uint8_t c_state = 0;

uint8_t Bright = 69; //0x00;
uint8_t Contrast = 121; //0x80;
uint8_t Saturation = 114; //0x80;

bool asw_01, asw_02, asw_03, asw_04; //  01 :  02 : compatible switch 03 : 04

bool AVsw;
uint8_t Input_signal = 0;
uint8_t buff_send[APP_FRAME_LEN_MAX];

uint8_t I2C_COMMANDS_I2P_ON[] =
    {
        0x84,0x55,0x80, // Enable I2P deinterlacing
        0x84,0x5A,0x02, // Set I2P parameters smoothing 1A

        0x42,0x0E,0x40, // 7280 input user submap 2
        0x42,0xE0,0x05, //ms 01 Enable fast lock mode
        0x42,0x0E,0x00,
};

//  I2C_COMMANDS_I2P_OFF_p
//  I2C_COMMANDS_I2P_OFF_240p
const uint8_t I2C_COMMANDS_I2P_OFF_p[] = {
   0x42,0x0E,0x40,
   0x42,0xE0,0x01,
   0x42,0x0E,0x00,

   0x84,0x55,0x00,
   0x84,0x5A,0x02,
    
};

//  I2C_COMMANDS_I2P_OFF_p
//  I2C_COMMANDS_I2P_OFF_576i
uint8_t I2C_COMMANDS_I2P_OFF_576i[] =
    {
        0x84, 0x55, 0x00,
        0x84, 0x5A, 0x02,

        0x56, 0x17, 0x02, // 7391Software reset
        0xFF, 0x0A, 0x00,
        0x56, 0x00, 0x1C, // pll on  | DAC[1:3]on  |  SleepMode off
        0x56, 0x01, 0x00, // SDInput  |
        0x56, 0x80, 0x10, // Luma SSAF  | NTSC   | 1.3 MHz
        0x56, 0x82, 0xC9, // SD active video edge control | SD pixel data valid | SD base
        0x56, 0x87, 0x20, // SD input standard auto detection |
        0x56, 0x88, 0x00, // 8-bit YCbCr input.
        0x56, 0x30, 0x10,
        0xFF, 0x0A, 0x00,

        0x84, 0x55, 0x00,
        0x84, 0x5A, 0x02};

uint8_t I2C_COMMANDS_SMOOTH_OFF[] =
    {
        0x84, 0x55, 0x80,
        0x84, 0x5A, 0x02, // Smooth off
        0x42, 0x0E, 0x00};

uint8_t I2C_COMMANDS_SMOOTH_ON[] =
    {
        0x84, 0x55, 0x80, //
        0x84, 0x5A, 0x1A, // Smooth on
        0x42, 0x0E, 0x00  //
};
uint8_t I2C_COMMANDS_BCSH[] =
    {
        0x42, 0x0E, 0x00, // Re-enter map
        0x42, 0x0a, 0xe0, // new brightness 00(00)  7F(+30)  80(-30)    e0
        0x42, 0x08, 0x58, // new contrast   00(00)  80(01)   FF(02)     58
        0x42, 0xe3, 0x58, // new saturation 00(00)  80(01)   FF(02)     80
};

uint8_t I2C_COMMANDS_YC_INPUT[] =
    {
        /* =============== ADV7280 YC component input configuration =============== */
        // [Register page control (manual section 6.2.15)]
        0x42, 0x0E, 0x00, // [0x0E] Select the main register page
        // [Video input selection (manual table 15)]
        0x42, 0x00, 0x09, // [0x00] YC input mode:
                          // Y signal-AIN3, C signal-AIN4
				// [Video processing control (manual section 6.2.56-57)]
			0x42, 0x38, 0x80, //24, // [0x38] NTSC processing: tried all values: values over 0x20 every 0x40 cause hanover bars for 8 consequent values (0x20-0x28, 0x60-0x68..etc)
                          // BIT5=0 disable chroma comb filtering
													// BIT2=1 enable luma path pass-through
			0x42, 0x39, 0xc0, //0x24, // [0x39] PAL processing: tried all values: NO impact to output whatsoever!
                          // Same bit configuration for PAL compatibility
			// [Filter control (Section 6.2.23 of the manual)] - tried all values 0x01-super sharp
			//																																		0x02-super smooth --> 0x0f super sharp
			//																																		0x07-sharp edges, but neighboring pixels are blended to gray 50%
			//then it is repeating every 32 (0x20) values +/- the same effect
        0x42, 0x17, 0x01, //0x49// [0x17]
                          // BIT6=1 Enable SH1 filter
                          // BIT[3:0]=1001: SVHS mode 8 (enhanced sharpness)

//			0x42,0x31,0x00 tried all values - no effect whatsoever
//			0x42,0x32,0x00 tried all values - no effect whatsoever
//			0x42,0x51,0x00 tried all values - no effect whatsoever


};

uint8_t I2C_COMMANDS_CVBS_INPUT[] =
    {
        /* =============== ADV7280 CVBS input configuration =============== */
        // [Register page control (manual section 6.2.15)]
        0x42, 0x0E, 0x00, // [0x0E] Select the main register page
        // [Video input selection (manual table 15)]
        0x42, 0x00, 0x00, // [0x00] CVBS input mode:
                          // AIN1 input composite video signal
        // [Video processing control (manual section 6.2.56-57)]
        0x42, 0x38, 0x80, //24 // [0x38] NTSC processing:
                          // BIT5=0 disable chroma comb filtering
													// BIT2=1 enable luma path pass-through
        0x42, 0x39, 0xc0, //24// [0x39] PAL processing:
                          // Same bit configuration for PAL compatibility
				// [Filter control (Section 6.2.23 of the manual)]
        0x42, 0x17, 0x49, // [0x17]
                          // BIT6=1 Enable SH1 filter
                          // BIT[3:0]=0111: Standard CVBS mode 7
			
//0x42, 0x2C, 0x61, // Try lower than current value (e.g., from 0x8E)
//0x42, 0x31, 0x12,
//0x42, 0x32, 0x41,
//0x42, 0x33, 0xc4,
			
};
// RetroScaler_I2C_AUTO_COMMANDS
// I2C_AUTO_COMMANDS
void SoftwareReset_ADV7280A()
{
    uint8_t buff[2];
    buff[0] = 0x0F;
    buff[1] = 0x80;
    (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT); // Reset all registers of ADV7280
    DDL_DelayMS(10);

    buff[0] = 0x0F;
    buff[1] = 0x00;
    (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT); // Reset all registers of ADV7280
    DDL_DelayMS(10);
}

uint8_t /*RetroScaler_*/I2C_AUTO_COMMANDS[] =
    {
  0x42,0xf,0, 			//power on
			0x42, 0x31, 0x01,  // Push start of active video down by 1 line
  0x42,0xe,0x80, 	//ADI control, reserved
  0x42,0x9c,0, 		//readonly letterbox2
  0x42,0x9c,0xff, 	//readonly letterbox2
  0x42,0xe,0, 			//ADI control 1,default
  0x42,3,0xc, 			//output control - output drivers tristated,only active video region filtered
  0x42,4,7, 				//ext.output ctrl - Y/C extended range,SFL output,blank output color during VBI, HS-VS-DIELD force active
  0x42,0x13,0, 		//read only status3
  0x42,0x1d,0x42, 	//ADI control 2,LLC pin active
  0x42,0x52,0xcd, 	//undefined????
  0x42,0x80,0x51, 	//undefined????
  0x42,0x81,0x51, 	//undefined????
  0x42,0x82,0x68, 	//undefined????
  0x42,0xe,0x80, 	//ADI control 1, reserved
  0x42,0xd9,0x44, 	//undefined??
  0x42,0xe,0, 			//ADI control 1,default
  0x42,0xe,0x42, 
  0x42,0xe0,5, 		//FL_ENABLE fast lock control
  0x42,0xe,0, 			//ADI control 1,default
  0x42,0xfd,0x84, 
  0x84,0xa3,0, 
  0x84,0x5b,0, 
  0x84,0x55,0x80, 
 0x42,0x2c,0x8e, 
  0x42,0x2f,0x34, 
  0x42,0x30,0x46, 
  0x42,0x14,0x11, 
  0x42,0x4d,0xcc, 
	
	     0x56,0x17,0x02,  //7391Software reset
       0xFF,0x0A,0x00,
       0x56,0x00,0x9C,
       0x56,0x01,0x70,
       0x56,0x30,0x1C, 
	 
       0x56,0x31,0x01,

};

/* BACKUP
// RetroScaler_I2C_AUTO_COMMANDS
uint8_t RetroScaler_I2C_AUTO_COMMANDS[] =
    {
        // 7280
        // 0x42, 0x0F, 0x80, // Reset all registers of ADV7280
        // 0x42, 0x0F, 0x00, // Exit reset mode and resume normal operation

        0x42, 0x00, 0x00, // CVBS in on AIN1
        0x42, 0x0E, 0x80, // ADI Required Write
        0x42, 0x9C, 0x00, // ADI Required Write
        0x42, 0x9C, 0xFF, // ADI Required Write

        0x42, 0x0E, 0x00, // Re-enter map
        0x42, 0x03, 0x0C, // All lines filtered and scaled | Output drivers enabled
        0x42, 0x04, 0x37, // Extended Output Control 0x04
        0x42, 0x13, 0x00, // Enable INTRQ output driver
        0x42, 0x1D, 0x40, // LLC pin activation
        0x42, 0x54, 0xC0, // ADI Required Write
        0x42, 0x80, 0x51, // ADI Required Write
        0x42, 0x81, 0x51, // ADI Required Write
        0x42, 0x82, 0x68, // ADI Required Write

        0x42, 0x14, 0x15, // Current clamp enable | Set free-run mode, output 100% color bar test signal
        0x42, 0x02, 0x04, // Select video input format (VID_SEL = 0x04, automatic detection of PAL/NTSC/SECAM)
        0x42, 0x07, 0xFF, // new  auto tvmode

#if RES_CHANGED == false
        0x42, 0x0a, 0xe0, // new brightness 00(00)  7F(+30)  80(-30)    e0
        0x42, 0x08, 0x58, // new contrast   00(00)  80(01)   FF(02)     58
        0x42, 0xe3, 0x58, // new saturation 00(00)  80(01)   FF(02)     80
                          // 0x42, 0x0B, 0xff, // new tone 80(00)  00(-312) ff(+312)
#endif
        // 0x42, 0x6B, 0x11, // VS/Field/SFL Pin Output Data Enable Official script does not have

        0x42, 0xFD, 0x84, // Set the VPP register address to 0x84
        0x84, 0xA3, 0x00, // ADI Required Write
        0x84, 0x5B, 0x00, // Enable advanced timing mode Advanced timing mode must be enabled for the I2P converter to work properly

        //User Sub Map 2
        0x42, 0x0E, 0x40, // Access User Sub Map 2
        // 0x42, 0xD9, 0x44, // Selects the minimum threshold for the input Luma video signal.
        0x42, 0xE0, 0x01, // Enable fast lock mode
        0x42, 0x0E, 0x00, // Re-enter map
        //End User Sub Map 2

        //7391
        0x56, 0x17, 0x02, // 7391Software reset
        0x56, 0x00, 0x1C, // Open DAC[1:3]
        0x56, 0x01, 0x70, // ED (at 54 MHz) input.
        0x56, 0x30, 0x04, // Embedded EAV/SAV code | SMPTE 293M, ITU-BT.1358
        0x56, 0x31, 0x01, // Pixel Data Valid [Encoder Writes finished]

        // 0x42 ,0x0F ,0x00, // Exit Power Down Mode [ADV7280 writes begin]
        // 0x42 ,0x00 ,0x00, // CVBS in on AIN1
        // // 0x42 ,0x0E ,0x80, // ADI Required Write
        // // 0x42 ,0x9C ,0x00, // ADI Required Write
        // // 0x42 ,0x9C ,0xFF, // ADI Required Write
        // 0x42 ,0x0E ,0x00, // Enter User Sub Map
        // 0x42 ,0x03 ,0x0C, // Enable Pixel & Sync output drivers
        // 0x42 ,0x04 ,0x07, // Power-up INTRQ, HS & VS pads
        // 0x42 ,0x13 ,0x00, // Enable INTRQ output driver
        // 0x42 ,0x17 ,0x41, // Enable SH1
        // 0x42 ,0x1D ,0x40, // Enable LLC output driver
        // // 0x42 ,0x52 ,0xCD, // ADI Required Write
        // // 0x42 ,0x80 ,0x51, // ADI Required Write
        // // 0x42 ,0x81 ,0x51, // ADI Required Write
        // // 0x42 ,0x82 ,0x68, // ADI Required Write
        // 0x42 ,0xFD ,0x84, // Set VPP Map
        // 0x84 ,0xA3 ,0x00, // ADI Required Write [ADV7280 VPP writes begin]
        // 0x84 ,0x5B ,0x00, // Enable Advanced Timing Mode
        // 0x84 ,0x55 ,0x80, // Enable the Deinterlacer for I2P [All ADV7280 writes finished]
        // 0x56 ,0x17 ,0x02, // Software Reset [Encoder writes begin]
        // 0x56 ,0x00 ,0x9C, // Power up DAC's and PLL
        // 0x56 ,0x01 ,0x70, // ED at 54MHz input
        // 0x56 ,0x30 ,0x04, // 525p at 59.94 Hz with Embedded Timing
        // 0x56 ,0x31 ,0x01, // Pixel Data Valid [Encoder Writes finished]

};

*/

// I2C_AUTO_COMMANDS
uint8_t I2C_AUTO_COMMANDS2[] =
    {
       0x42,0x0F,0x80, // reset the ADV7280 regs
       0x56,0x17,0x02, // reset the ADV7193 regs
       0xFF,0x0A,0x00,
       0x42,0x0F,0x00,
       0x42,0x05,0x00,
       0x42,0x02,0x04, // 04-all auto Standard 0000 0100   th17
       0x42,0x07,0xff, // new  tvmode
       0x42,0x14,0x15, // set free-run pattern to 100% color bar     (  0x11  )
       0x42,0x00,0x00,
//RT config starts here
       0x42,0x0E,0x80,  
       0x42,0x9C,0x00,
       0x42,0x9C,0xFF,
       0x42,0x0E,0x00,
       0x42,0x03,0x0C,
       0x42,0x04,0x07,
       0x42,0x13,0x00,
       0x42,0x17,0x40, // 41  
       0x42,0x1D,0x40, // enable LLC output
       0x42,0x52,0xCD,
       0x42,0x80,0x51,
       0x42,0x81,0x51,
       0x42,0x82,0x68,
       0x42,0x0E,0x80,
       0x42,0xD9,0x44,
       0x42,0x0E,0x00,
       0x42,0x0E,0x40,  //7280 input user submap 2
       0x42,0xE0,0x01,  // 0x05, //0x01,
       0x42,0x0E,0x00,
       0x42,0xFD,0x84, // set the VPP address 0x84
       0x84,0xA3,0x00, // vpp write begin
       0x84,0x5B,0x00, // enable the timing mode
       0x84,0x55,0x80, // enable the Dinterlacer I2P
       // 0x85, 0x5A, 0x85, //read the reg 0x5A
       0x84,0x5A,0x02,
       0x42,0x6B,0x11, // the vs/field/sfl pin output data enable.
// end of RT config    
       0x56,0x17,0x02,  //7391Software reset
       0xFF,0x0A,0x00,
       0x56,0x00,0x9C,
       0x56,0x01,0x70,
       0x56,0x30,0x1C,  
       0x56,0x31,0x01,
       0x42,0x0E,0x00
};

uint8_t I2C_AUTO_COMMANDS_backup[] =
    {
       0x42,0x0F,0x80, // reset the ADV7280 regs
       0x56,0x17,0x02, // reset the ADV7193 regs
       0xFF,0x0A,0x00,
       0x42,0x0F,0x00,
       0x42,0x05,0x00,
       0x42,0x02,0x04, // Standard 0000 0100   th17
       0x42,0x07,0xff, // new  tvmode
       0x42,0x14,0x15, // set free-run pattern to 100% color bar     (  0x11  )
       0x42,0x00,0x00,
       0x42,0x0E,0x80,
       0x42,0x9C,0x00,
       0x42,0x9C,0xFF,
       0x42,0x0E,0x00,
       0x42,0x03,0x0C,
       0x42,0x04,0x07,
       0x42,0x13,0x00,
       0x42,0x17,0x40, // 41
       0x42,0x1D,0x40, // enable LLC output
       0x42,0x52,0xCD,
       0x42,0x80,0x51,
       0x42,0x81,0x51,
       0x42,0x82,0x68,
       0x42,0x0E,0x80,
       0x42,0xD9,0x44,
       0x42,0x0E,0x00,
       0x42,0x0E,0x40,  //7280 input user submap 2
       0x42,0xE0,0x01,
       0x42,0x0E,0x00,
       0x42,0xFD,0x84, // set the VPP address 0x84
       0x84,0xA3,0x00, // vpp write begin
       0x84,0x5B,0x00, // enable the timing mode
       0x84,0x55,0x80, // enable the Dinterlacer I2P
       // 0x85, 0x5A, 0x85 //read the reg 0x5A
       0x84,0x5A,0x02,
       0x42,0x6B,0x11, // the vs/field/sfl pin output data enable.
       0x56,0x17,0x02,  //7391Software reset
       0xFF,0x0A,0x00,
       0x56,0x00,0x9C,
       0x56,0x01,0x70,
       0x56,0x30,0x1C,  
       0x56,0x31,0x01,
       0x42,0x0E,0x00
};

uint8_t Ace_Code_ON[] =
    {
        0x42, 0x0E, 0x40,
        0x42, 0x80, 0x80,
        0x42, 0x0E, 0x00};

uint8_t Ace_Code_OFF[] =
    {
        0x42, 0x0E, 0x40,
        0x42, 0x80, 0x00,
        0x42, 0x0E, 0x00
    };

// void info(void)
// {
//     uint8_t count = 0;
//     uint8_t buff[2] = {0xe0, 0x00};

//     GPIO_SetPins(POWER_DOWN_PORT, GPIO_PIN_POWER_DOWN); // POWER_DOWN_N
//     DDL_DelayMS(5);
//     GPIO_SetPins(INPUT_RESET_PORT, GPIO_PIN_INPUT_RESET); // INPUT_RESET_N
//     DDL_DelayMS(10);
//     GPIO_SetPins(OUTPUT_PORT, GPIO_PIN_OUTPUT_EN); // OUTPUT_EN
//     DDL_DelayMS(10);

//     (void)Chip_Receive(DEVICE_ADDR, buff, NULL, 1, TIMEOUT);
//     DDL_DelayMS(15);
//     buff[0] = 0x0f;
//     buff[1] = 0x80;
//     (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
//     DDL_DelayMS(10);

//     buff[0] = 0x0f;
//     buff[1] = 0x00;
//     (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
//     DDL_DelayMS(15);
//     SoftwareReset_ADV7280A();
//     (void)ADV_7280_Send_Buff(I2C_AUTO_COMMANDS, sizeof(I2C_AUTO_COMMANDS) / 3, TIMEOUT); // AUTO_REGS

//     buff[0] = VID_SEL_REG;
//     buff[1] = adv_tv;
//     (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT); // mode

//     set_input(1); // read button
//     set_smooth(0);
//     set_double_line(0); // GPIO_ReadInputPins(GPIO_PORT_B,GPIO_PIN_05)
//     Adv_7391_sw = 1;
//     printf("ModuleOn\n");
//     //    led_state &= ~0x01;
// }
void video_init(void)
{
    uint8_t count = 0;
    uint8_t buff[2] = {0xe0, 0x00};

    GPIO_SetPins(POWER_DOWN_PORT, GPIO_PIN_POWER_DOWN); // POWER_DOWN_N
    DDL_DelayMS(5);
    GPIO_SetPins(INPUT_RESET_PORT, GPIO_PIN_INPUT_RESET); // INPUT_RESET_N
    DDL_DelayMS(10);
    GPIO_SetPins(OUTPUT_PORT, GPIO_PIN_OUTPUT_EN); // OUTPUT_EN
    DDL_DelayMS(10);

    (void)Chip_Receive(DEVICE_ADDR, buff, NULL, 1, TIMEOUT);
    DDL_DelayMS(15);

    // SoftwareReset_ADV7280A();
    (void)ADV_7280_Send_Buff(I2C_AUTO_COMMANDS, sizeof(I2C_AUTO_COMMANDS) / 3, TIMEOUT); // AUTO_REGS
    set_input(adv_input); // read button

    buff[0] = VID_SEL_REG;
    buff[1] = adv_tv; // adv_tv
    printf(" Init adv_tv: 0x%02x", adv_tv);
    (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT); // mode

    set_bcsh();
    if (adv_double)
    {
        set_double_line(adv_double); // GPIO_ReadInputPins(GPIO_PORT_B,GPIO_PIN_05)
        if (adv_smooth)
            set_smooth(adv_smooth);
    }
    else
    {
        set_double_line(true);
        set_smooth(true);
        adv_smooth = false;
        set_smooth(adv_smooth);
        set_double_line(adv_double);
    }

    DDL_DelayMS(200);

    Adv_7391_sw = 1;
    printf("ModuleOn\n");
    //    led_state &= ~0x01;
}

void video_Deinit(void)
{
    GPIO_ResetPins(POWER_DOWN_PORT, GPIO_PIN_POWER_DOWN);   // POWER_DOWN_N
    GPIO_ResetPins(INPUT_RESET_PORT, GPIO_PIN_INPUT_RESET); // INPUT_RESET_N
    GPIO_ResetPins(OUTPUT_PORT, GPIO_PIN_OUTPUT_EN);        // OUTPUT_EN
    Adv_7391_sw = 0;
    printf("ModuleOff\n");
    //    led_state = 0x01;
}

void Video_Sw(uint8_t sw)
{
    if (sw == true)
    {
        video_init();
    }
    //    else
    //    {
    //        video_Deinit();
    //    }
}

void set_input(uint8_t input)
{
    if (input)
    {
        (void)ADV_7280_Send_Buff(I2C_COMMANDS_YC_INPUT, sizeof(I2C_COMMANDS_YC_INPUT) / 3, TIMEOUT);
        printf("SvSignal\n");
        //        led_state &= ~(LED_GREEN);
        //        led_state |= LED_BLUE;
    }
    else
    {
        (void)ADV_7280_Send_Buff(I2C_COMMANDS_CVBS_INPUT, sizeof(I2C_COMMANDS_CVBS_INPUT) / 3, TIMEOUT);
        printf("AvSignal\n");
        //        led_state &= ~(LED_BLUE);
        //        led_state |= LED_GREEN;
    }
}

void set_double_line(uint8_t doubleline)
{
    if (doubleline)
    {
        (void)ADV_7280_Send_Buff(I2C_COMMANDS_I2P_ON, sizeof(I2C_COMMANDS_I2P_ON) / 3, TIMEOUT); // I2P ON
        printf("I2pOn\n");
    }
    else
    {
        (void)ADV_7280_Send_Buff(I2C_COMMANDS_I2P_OFF_p, sizeof(I2C_COMMANDS_I2P_OFF_p) / 3, TIMEOUT); //_240p _576i
        printf("I2pOff\n");
    }
}

void set_smooth(uint8_t smooth)
{
    if (smooth)
    {
        (void)ADV_7280_Send_Buff(I2C_COMMANDS_SMOOTH_ON, sizeof(I2C_COMMANDS_SMOOTH_ON) / 3, TIMEOUT);
        printf("SmoothOn\n");
    }
    else
    {
        (void)ADV_7280_Send_Buff(I2C_COMMANDS_SMOOTH_OFF, sizeof(I2C_COMMANDS_SMOOTH_OFF) / 3, TIMEOUT);
        printf("SmoothOff\n");
    }
}

void set_bcsh(void)
{

    I2C_COMMANDS_BCSH[5] = Bright;
    I2C_COMMANDS_BCSH[8] = Contrast;
    I2C_COMMANDS_BCSH[11] = Saturation;
    (void)ADV_7280_Send_Buff(I2C_COMMANDS_BCSH, sizeof(I2C_COMMANDS_BCSH) / 3, TIMEOUT);
    printf("Bright    : 0x%02x\n", Bright);
    printf("Contrast  : 0x%02x\n", Contrast);
    printf("Saturation: 0x%02x\n", Saturation);
};

void set_ace(uint8_t ace)
{
    if (ace)
    {
        (void)ADV_7280_Send_Buff(Ace_Code_ON, sizeof(Ace_Code_ON) / 3, TIMEOUT);
        printf("AceOn\n");
    }
    else
    {
        (void)ADV_7280_Send_Buff(Ace_Code_OFF, sizeof(Ace_Code_OFF) / 3, TIMEOUT);
        printf("AceOff\n");
    }
}

void Read_Video_key_change(void)
{
    static uint8_t key_state = 0, key_state_last = 0;

    key_state = GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_12);
    if (key_state_last != key_state)
    {
        adv_input = key_state;
        set_input(adv_input);
    }
    key_state_last = key_state;

    static uint8_t key_line_state = 0, key_state_line_last = 0;
    key_line_state = GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_05);
    if (key_state_line_last != key_line_state)
    {
        set_double_line(key_line_state);
    }
    key_state_line_last = key_line_state;
}

void detect_loop(void)
{
    if (Adv_7391_sw == 1)
    {

        uint8_t detect_result, ad_result;
        uint8_t buff[2];

        buff[0] = 0x0E;
        buff[1] = 0x00;
        (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
        DDL_DelayMS(10);

        buff[0] = 0x0E;
        buff[1] = 0x00;
        (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);

        buff[0] = 0x13;
        buff[1] = 0x10;
        Chip_Receive(DEVICE_ADDR, &buff[0], &detect_result, 1, TIMEOUT);
        Chip_Receive(DEVICE_ADDR, &buff[1], &ad_result, 1, TIMEOUT);
        //        printf("detect_result: 0x%02x\n",detect_result);
        //        printf("ad_result: 0x%02x\n",ad_result);
        if ((uint8_t)(detect_result & 0x81) == 0x80 && (err_flag))
        {
            if (adv_input == AV_INPUT)
            {
                //                video_init();
                set_input(SV_INPUT);
                set_input(AV_INPUT);
                err_flag = 0;
            }
            else if (adv_input == SV_INPUT)
            {
                //                video_init();
                set_input(AV_INPUT);
                set_input(SV_INPUT);
                err_flag = 0;
            }

            led_sw = 0x01 | LED_ERR_RED;
            //          led_state |= LED_RED;
            printf("err\n");
            err_flag = 0;
        }
        if (((uint8_t)(ad_result & 0x02) == 0x02) && !status && (err_flag))
        {
            status = 1;
            //            set_double_line(false);
            //            set_smooth(false);

            //            if(adv_double)
            //            {
            //                set_double_line(adv_double); // GPIO_ReadInputPins(GPIO_PORT_B,GPIO_PIN_05)
            //                if(adv_smooth)
            //                    set_smooth(adv_smooth);
            //            }
            //            else
            //            if(adv_double)
            //            {
            //                set_double_line(true);
            //                set_smooth(true);
            //                set_smooth(false);
            //                set_double_line(false);
            //            }

            //            Adv_7391_sw = 1;
            printf(" Run Free Mode \n");
            c_state = 2;
            err_flag = 0;
            led_state = LED_RED;
        }
        else if (status && ((uint8_t)(ad_result & 0x05) == 0x05) && (err_flag))
        {
            status = 0;
            err_flag = 0;

            //            if(adv_double)
            //            {
            //                set_double_line(adv_double);
            //                set_smooth(adv_smooth);
            //            }
            printf(" Close Free Mode \n");
            Signal_led(Input_signal);
        }

        //        if (((uint8_t)(ad_result & 0x25)==0x25))  //PAL 60
        //        {
        ////            buff[0] = 0x02;
        ////            buff[1] = 0xB4;    //PAL M
        //            buff[0] = 0x02;
        //            buff[1] = 0x64;  //PAL 60
        //        }   //                                                                             60    Bit2==0
        //        // Detected swing pulse train Bit7==1 Detected 50 Hz standard definition Bit2==1 Achieved horizontal lock Bit0==1
        //        else  if ( ((uint8_t)(ad_result & 0x15)==0x15) && ((uint8_t)(detect_result&0x85) == 0x81))   //NTSC443
        //        {
        ////            buff[0] = 0x02;
        ////            buff[1] = 0x64;  //PAL 60
        //            buff[0] = 0x02;
        //            buff[1] = 0xB4;    //PAL M
        //        }
        //        else
        //        {
        //            buff[0] = 0x02;
        //            buff[1] = 0x04;  //Auto
        //        }

        //        (void)I2C_Master_Transmit(DEVICE_ADDR, buff, 2 , TIMEOUT);
    }
    else
        return;
}

void ASC(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state)
{
    if ((sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_On();
    else if ((!sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_Off();

    if ((sw2) && ((sw2 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW2)) || state))
        ASW2_On();
    else if ((!sw2) && ((sw2 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW2)) || state))
        ASW2_Off();

    if ((sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_On();
    else if ((!sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_Off();

    if ((sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_On();
    else if ((!sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_Off();
    //    mem_settings();
}

void ASC_Not02(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state)
{
    if ((sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_On();
    else if ((!sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_Off();

    //    if  (sw2)
    //        ASW2_On();
    //    else
    //        ASW2_Off();

    if ((sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_On();
    else if ((!sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_Off();

    if ((sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_On();
    else if ((!sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_Off();
    //    mem_settings();
}

void AV_Connecte_Set(uint8_t sw)
{
    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;       // Output direction
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL; // Digital
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;
    if (sw)
    {
        //        AV_Connecte_On();
        stcGpioInit.u16PullUp = PIN_PU_OFF;
        stcGpioInit.u16PinState = PIN_STAT_RST;
        printf("Connecte_on");
        //        C_LED_OK();
        c_state = 1;
    }
    else
    {
        //        AV_Connecte_Off();
        stcGpioInit.u16PullUp = PIN_PU_ON;
        stcGpioInit.u16PinState = PIN_STAT_SET;
        printf("Connecte_off");
        //        C_LED_ERR_RED();
        c_state = 2;
    }

    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_08, &stcGpioInit);
}

// void SwRgbSync_On(void)
//{
//     asw_01=1;
//     asw_03=1;
// }

// void SwSyncOnGreen_On(void)
//{
//     asw_04=0;
// }

// void SwRgbSync_Off(void)
//{
//     asw_01=1;
//     asw_03=1;
// }

// void SwSyncOnGreen_Off(void)
//{
//     asw_04=0;
// }

void Signal_led(uint8_t signal)
{
    switch (signal)
    {
    case 1: // RGBs
    {
        led_state = LED_RED | LED_GREEN | LED_BLUE;
    }
    break;
    case 2: // RGsB
    {
        led_state = LED_GREEN;
    }
    break;
    case 3: // VGA
    {
        led_state = LED_BLUE;
    }
    break;
    case 4: // YUV
    {
        led_state = LED_RED | LED_GREEN;
    }
    break;
    case 5: // SV
    {
        led_state = LED_RED | LED_BLUE;
    }
    break;
    case 6: // AV
    {
        led_state = LED_GREEN | LED_BLUE;
    }
    break;
    }
}

void Reser_Status(void)
{
    status = 0;
}