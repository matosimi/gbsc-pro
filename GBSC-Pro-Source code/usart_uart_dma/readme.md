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
* Device: HC32F460JEUA

|default|off-chip|Start|Size|Startup/NoInit
|--|--|--|--|--|
|check|IROM1:|0x10000|0x60000|check|
||IROM2|0x3000C00|0x3FC||	
|check|IRAM1:|0x1FFF8000|0x2F000||
||IRAM2:|0x200F0000|0x1000||

# Build
Run Project->Build Target (or F7) to build the project.

It is currently showing ~67 warnings regarding the debug messages printed using cmb_println. This can be ignored for now.

Build creates usb_dev_cdc.axf in **output\release** directory as well as usb_dev_cdc.hex file and in After Build the usb_dev_cdc.bin file.

The **usb_dev_cdc.bin** is actually the firmware binary that can be flashed to the AV module of GBSC-pro using the GBSC_PRO_Programmer.exe utility which is distributed with the official firmware releases.

# How to flash

 1. Run GBSC_PRO_Programmer.exe
 2. Turn off the power switch of GBSC-Pro and unplug the DC connector
 3. Connect USB-C cable to your PC running the GBSC_PRO_Programmer.exe
 4. Press BOOT button on GBSC-Pro while connecting the USB-C connector to the USB-C socket of GBSC-Pro
 5. If done correctly, the GBSC-Pro logo of GBSC_PRO_Programmer.exe turns blue and GBSC-Pro device LEDs are slowly alternating between green and red color
 6. Browse for the firmware binary (usb_dev_cdc.bin) using the GBSC_PRO_Programmer.exe tool and click Send button
 7. Flashing process takes roughly 6 seconds, GBSC-pro LED keeps red during the process
 8. Once done, GBSC-Pro LED turn green
 9. Disconnect the USB-C cable from the USB-C connector of GBSC-Pro
 10.Plug in the DC and push the power switch to run the GBSC-Pro with the newly flashed firmware 

# Disclaimer
MatoSimi is not responsible for any damage to GBSC-Pro device(s) caused by using firmware builds from this repository.
The code modifications applied in this repository are experimental and not intended for general use.
It is strongly advised to use official firmware releases from the original RetroScaler GBSC-Pro repository:
https://github.com/RetroScaler/gbsc-pro/releases
