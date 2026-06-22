GBSC-Pro Atari XL/XE fix
---
GBSC-Pro converts retro console video signals to HDMI up to 1080p with low latency, advanced color processing, and extensive customization options.

This repository was originally forked from [RetroScaler/gbsc-pro](https://github.com/RetroScaler/gbsc-pro) , but later forcefully rebased to [Brisma](https://github.com/brisma/gbsc-pro)'s fork.

Version 2.4.1 of AV module firmware of Brisma is causing image crop at the very top of the visible screen area of Atari XL/XE.
I have made a modification to the firmware that fixes the crop.
The fix is related only to the AV firmware.

To make the Atari XL/XE picture displayed correctly flash the 2.4.1_matosimi AV firmware to your GBSC-Pro (https://github.com/matosimi/gbsc-pro/releases).

Be sure use this AV firmware in the combination with appropriate main firmware:

Either...
 * [GBSC_PRO_v2.4.1.bin](https://github.com/brisma/gbsc-pro/releases/tag/v2.4.1) main firmware by @brisma

or

 * [GBSC_PRO_v2.4.1-W.1RC3.zip](https://github.com/user-attachments/files/28007809/GBSC_PRO_v2.4.1-W.1RC3.zip) main firmware by @warheat1990 from [Here](https://github.com/warheat1990/gbsc-pro/issues/2) - this one has lighter and enhanced webgui with additional SV/AV settings.

<img width="778" height="1058" alt="atari-before-after-2_4_1" src="https://github.com/user-attachments/assets/f00832e6-efe4-4e4e-affd-3687609af568" />


