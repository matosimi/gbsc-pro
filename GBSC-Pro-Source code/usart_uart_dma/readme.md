# Preparation steps for the environment
Download Keil uVision from the official web.

I'm using MDK542a.exe evaluation version:
https://armkeil.blob.core.windows.net/eval/MDK542a.exe

Install the toolset, I set up directories following way:

C:\Keil_v5

C:\Arm\Packs

# Open
Clone this github repository on your local machine.

Open Keil uVision5

Go Project->Open Project...

Browse for Uart_Dma.uvprojx inside the usb_dev_cdc\MDK directory of this repository.

Be sure to select Target: Uart_Dma_Release

# Options for target Uart_Dma_Release 

## Device
* Device: HC32F460JEUA

## Target

|default|off-chip|Start|Size|Startup/NoInit
|--|--|--|--|--|
|check|IROM1:|0x10000|0x60000|check|
||IROM2|0x3000C00|0x3FC||	
|check|IRAM1:|0x1FFF8000|0x2F000||
||IRAM2:|0x200F0000|0x1000||

Code Generation -> ARM Compiler -> Use default compiler version 6.

## User
After build run #1: 
```
fromelf --bin --output "$L@L.bin" "#L"
```
- this is to generate binary output for the GBSC_PRO_Programmer_COM_fix.exe

## C/C++ (AC6)
Update include paths for your env. (Windows11 in my case)
```
..\source;
..\..\drivers\cmsis\Device\HDSC\hc32f4xx\Include;
..\..\drivers\cmsis\Include;
..\..\drivers\hc32_ll_driver\inc;
..\..\drivers\bsp\ev_hc32f460_lqfp100_v2;
..\..\midwares\hc32\usb;
..\..\midwares\hc32\usb\usb_device_lib\device_core;
..\..\midwares\hc32\usb\usb_device_lib\device_class\single_cdc;
..\cm_backtrace
```

all these directories are in the repository in GBSC-Pro-Source code\usart_uart_dma\usb_dev_cdc

# Build
Run Project->Build Target (or F7) to build the project.

It is currently showing ~67 warnings regarding the debug messages printed using cmb_println. This can be ignored for now.

Build creates usb_dev_cdc.axf in **output\release** directory as well as usb_dev_cdc.hex file and in After Build the usb_dev_cdc.bin file.

The **usb_dev_cdc.bin** is actually the firmware binary that can be flashed to the AV module of GBSC-pro using the GBSC_PRO_Programmer_COM_fix.exe utility.

# GBSC_PRO_Programmer_COM_fix.exe

I have made an adjustment to original programmer which is distributed with the official firmware releases... by changing logic of selection of COM ports,
so this "COM_fix" version does not select proper COM port based on its name, but based on technical name instead. (This was an issue when device had name with special characters - non English Windows)

# How to flash

 1. Run GBSC_PRO_Programmer_COM_fix.exe
 2. Turn off the power switch of GBSC-Pro and unplug the DC connector
 3. Connect USB-C cable to your PC running the GBSC_PRO_Programmer_COM_fix.exe
 4. Press BOOT button on GBSC-Pro while connecting the USB-C connector to the USB-C socket of GBSC-Pro
 5. If done correctly, the GBSC-Pro logo of GBSC_PRO_Programmer_COM_fix.exe turns blue and GBSC-Pro device LEDs are slowly alternating between green and red color
 6. Browse for the firmware binary (usb_dev_cdc.bin) using the GBSC_PRO_Programmer_COM_fix.exe tool and click Send button
 7. Flashing process takes roughly 6 seconds, GBSC-pro LED stays red during the process
 8. Once done, GBSC-Pro LED turns green
 9. Disconnect the USB-C cable from the USB-C connector of GBSC-Pro
 10. Plug in the DC and push the power switch to run the GBSC-Pro with the newly flashed firmware 

# Disclaimer
MatoSimi is not responsible for any damage to GBSC-Pro device(s) caused by using firmware builds from this repository.
The code modifications applied in this repository are experimental and not intended for general use.
It is strongly advised to use official firmware releases from the original RetroScaler GBSC-Pro repository:
https://github.com/RetroScaler/gbsc-pro/releases
