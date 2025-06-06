#include "OSDManager.h"
#include "options.h"
#include "stdio.h"

extern userOptions *uopt;
extern void saveUserPrefs();
extern void shiftHorizontalRight();
extern void shiftHorizontalLeft();
extern void shiftVerticalDownIF();
extern void shiftVerticalUpIF();
extern void scaleVertical(uint16_t, bool);
extern void scaleHorizontal(uint16_t, bool);
extern void disableScanlines();
extern OSDManager osdManager;
// osd亮度
bool osdBrightness(OSDMenuConfig &config)
{
    const int16_t STEP = 8;
    int8_t cur = GBS::VDS_Y_OFST::read();
    if (config.onChange)
    {
        if (config.inc)
        {
            cur = MIN(cur + STEP, 127);
        }
        else
        {
            cur = MAX(-128, cur - STEP);
        }
        GBS::VDS_Y_OFST::write(cur); // bri[-128:127]
        printf("bri %d\n",abs(GBS::VDS_Y_OFST::read()));
    }
    config.barLength = 256 / STEP;
    config.barActiveLength = (cur + 128 + 1) / STEP;
    if (cur < 0)
    {
        printf("bri: -0x%02x\n", abs(cur));
    }
    else
        printf("bri: 0x%02x\n", cur);
    return true;
}
// 自动增益
bool osdAutoGain(OSDMenuConfig &config)
{
    config.barLength = 2;
    if (config.onChange)
    {
        uopt->enableAutoGain = config.inc ? 1 : 0;
        saveUserPrefs();
    }
    if (uopt->enableAutoGain == 1)
    {
        config.barActiveLength = 2;
    }
    else
    {
        config.barActiveLength = 0;
    }
    return true;
}

// 扫描线
bool osdScanlines(OSDMenuConfig &config)
{
    if (config.onChange)
    {   
      
        if (config.inc)
        {
            if (uopt->scanlineStrength > 0)    //扫描线强度
            {
                uopt->scanlineStrength -= 0x10;
            }
        }
        else
        {
            uopt->scanlineStrength = MIN(uopt->scanlineStrength + 0x10, 0x50);
        }
// printf("uopt->scanlineStrength 0x%02x \n",uopt->scanlineStrength);
        if (uopt->scanlineStrength == 0x50)
        {
            uopt->wantScanlines = 0;
            disableScanlines();
            // printf("Off ScanfLines\n");
        }
        else
        {
            uopt->wantScanlines = 1;
            // printf("On ScanfLines\n");
        }
        GBS::MADPT_Y_MI_OFFSET::write(uopt->scanlineStrength);
        saveUserPrefs();
    }

    /*
    OSD 显示条
    */
    config.barLength = 128;   //条形长度
    if (uopt->scanlineStrength == 0)   //扫描线强度
    {
        config.barActiveLength = 128;
    }
    else
    {
        config.barActiveLength = (5 - uopt->scanlineStrength / 0x10) * 25;
    }
    return true;
}
// osdLineFilter（行过滤器
bool osdLineFilter(OSDMenuConfig &config)
{
    config.barLength = 2;
    if (config.onChange)
    {
        uopt->wantVdsLineFilter = config.inc ? 1 : 0;
        GBS::VDS_D_RAM_BYPS::write(!uopt->wantVdsLineFilter);
        saveUserPrefs();
    }
    if (uopt->wantVdsLineFilter == 1)
    {
        config.barActiveLength = 2;
    }
    else
    {
        config.barActiveLength = 0;
    }
    return true;
}
// X移动
bool osdMoveX(OSDMenuConfig &config)
{
    config.barLength = 2;
    if (!config.onChange)
    {
        config.barActiveLength = 1;
        return true;
    }
    if (config.inc)
    {
        shiftHorizontalRight();
        config.barActiveLength = 2;
    }
    else
    {
        shiftHorizontalLeft();
        config.barActiveLength = 0;
    }
    return false;
}
// Y移动
bool osdMoveY(OSDMenuConfig &config)
{
    config.barLength = 2;
    if (!config.onChange)
    {
        config.barActiveLength = 1;
        return true;
    }
    if (config.inc)
    {
        shiftVerticalDownIF();
        config.barActiveLength = 2;
    }
    else
    {
        shiftVerticalUpIF();
        config.barActiveLength = 0;
    }
    return false;
}

bool osdScaleY(OSDMenuConfig &config)
{
    config.barLength = 2;
    if (!config.onChange)
    {
        config.barActiveLength = 1;
        return true;
    }
    if (config.inc)
    {
        scaleVertical(2, true);
        config.barActiveLength = 2;
    }
    else
    {
        scaleVertical(2, false);
        config.barActiveLength = 0;
    }
    return false;
}

bool osdScaleX(OSDMenuConfig &config)
{
    config.barLength = 2;
    if (!config.onChange)
    {
        config.barActiveLength = 1;
        return true;
    }
    if (config.inc)
    {
        scaleHorizontal(2, true);
        config.barActiveLength = 2;
    }
    else
    {
        scaleHorizontal(2, false);
        config.barActiveLength = 0;
    }
    return false;
}
// 对比度
bool osdContrast(OSDMenuConfig &config)
{
    const uint8_t STEP = 8;
    int16_t cur = GBS::ADC_RGCTRL::read(); // [0:255]
    if (config.onChange)
    {
        if (uopt->enableAutoGain == 1)
        {
            uopt->enableAutoGain = 0;
            saveUserPrefs();
        }
        else
        {
            uopt->enableAutoGain = 0;
        }
        if (config.inc)
        {
            cur = MAX(0, cur - STEP);
        }
        else
        {
            cur = MIN(cur + STEP, 255);
        }
        GBS::ADC_RGCTRL::write(cur);
        GBS::ADC_GGCTRL::write(cur);
        GBS::ADC_BGCTRL::write(cur);
    }
    config.barLength = 256 / STEP;
    config.barActiveLength = (256 - cur) / STEP;
    // printf("Con: 0x%02x\n", cur);
    return true;
}

void initOSD()
{
    osdManager.registerIcon(OSDIcon::BRIGHTNESS, osdBrightness);
    osdManager.registerIcon(OSDIcon::CONTRAST, osdContrast);

    // consufuing, disabled for now
    // osdManager.registerIcon(OSDIcon::HUE, osdScanlines);
    // osdManager.registerIcon(OSDIcon::VOLUME, osdLineFilter);

    osdManager.registerIcon(OSDIcon::MOVE_Y, osdMoveY);
    osdManager.registerIcon(OSDIcon::MOVE_X, osdMoveX);
    osdManager.registerIcon(OSDIcon::SCALE_Y, osdScaleY);
    osdManager.registerIcon(OSDIcon::SCALE_X, osdScaleX);
}