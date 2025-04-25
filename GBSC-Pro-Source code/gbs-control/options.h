#ifndef _USER_H_
using Ascii8 = uint8_t;
/// Output resolution requested by user, *given to* applyPresets().
enum PresetPreference : uint8_t {
    Output960P = 0,
    Output480P = 1,
    OutputCustomized = 2,
    Output720P = 3,
    Output1024P = 4,
    Output1080P = 5,
    OutputDownscale = 6,
    OutputBypass = 10,
};

enum INPUT_PresetPreference : uint8_t {
  MT_RGBs   ,
  MT_RGsB   ,
  MT_VGA    ,
  MT_YPBPR    ,
  MT_SV     ,
  MT_AV     ,
};

enum SETTING_PresetPreference : uint8_t {
    // MT_7391_OFF   ,
    // MT_7391_ON    ,
    MT_7391_1X    ,
    MT_7391_2X    ,
    MT_SMOOTH_OFF ,
    MT_SMOOTH_ON  ,
    MT_COMPATIBILITY_OFF   ,
    MT_COMPATIBILITY_ON    ,
    MT_ACE_OFF   ,
    MT_ACE_ON    ,
    // MT_7391_AV    ,
    // MT_7391_SV    ,
};

enum TVMODE_PresetPreference : uint8_t {
    MT_MODE_AUTO=0,
    MT_MODE_PAL,
    MT_MODE_NTSCM,
    MT_MODE_PAL60,
    MT_MODE_NTSC443,
    MT_MODE_NTSCJ,
    MT_MODE_PALNwp,
    MT_MODE_PALMwop,
    MT_MODE_PALM,
    MT_MODE_PALCmbN,
    MT_MODE_PALCmbNwp,
    MT_MODE_SECAM,
    
};

// userOptions holds user preferences / customizations
// userOptions 保存用户偏好/自定义设置
struct userOptions
{
    // 0 - normal, 1 - x480/x576, 2 - customized, 3 - 1280x720, 4 - 1280x1024, 5 - 1920x1080,
    // 6 - downscale, 10 - bypass
    PresetPreference presetPreference;
    INPUT_PresetPreference   INPUT_presetPreference;
    SETTING_PresetPreference  SETTING_presetPreference;
    TVMODE_PresetPreference  TVMODE_presetPreference;

    Ascii8 presetSlot;
    uint8_t enableFrameTimeLock;   //启用帧时间锁定
    uint8_t frameTimeLockMethod;  //帧时间锁定方法
    uint8_t enableAutoGain;   //启用自动增益
    uint8_t wantScanlines;    //要扫描线
    uint8_t wantOutputComponent;  //要输出组件
    uint8_t deintMode;        //非int模式
    uint8_t wantVdsLineFilter;  //想要 VdsLine过滤器
    uint8_t wantPeaking;   //峰值
    uint8_t wantTap6;
    uint8_t preferScalingRgbhv;
    uint8_t PalForce60;
    uint8_t disableExternalClockGenerator;  //禁用外部时钟生成器
    uint8_t matchPresetSource;  //匹配预置源
    uint8_t wantStepResponse;
    uint8_t wantFullHeight;  //想要全高
    uint8_t enableCalibrationADC;  //启用校准 ADC
    uint8_t scanlineStrength;   //扫描线强度
};


// runTimeOptions holds system variables
struct runTimeOptions
{
    uint32_t freqExtClockGen;
    uint16_t noSyncCounter; // is always at least 1 when checking value in syncwatcher
    uint8_t presetVlineShift;
    uint8_t videoStandardInput; // 0 - unknown, 1 - NTSC like, 2 - PAL like, 3 480p NTSC, 4 576p PAL
    uint8_t phaseSP;
    uint8_t phaseADC;
    uint8_t currentLevelSOG;
    uint8_t thisSourceMaxLevelSOG;
    uint8_t syncLockFailIgnore;
    uint8_t applyPresetDoneStage;//应用预置完成阶段
    uint8_t continousStableCounter;
    uint8_t failRetryAttempts;
    uint8_t presetID;  // PresetID
    bool isCustomPreset;
    uint8_t HPLLState;
    uint8_t medResLineCount;
    uint8_t osr;
    uint8_t notRecognizedCounter;
    bool isInLowPowerMode;
    bool clampPositionIsSet;
    bool coastPositionIsSet;
    bool phaseIsSet;
    bool inputIsYpBpR;
    bool syncWatcherEnabled;
    bool outModeHdBypass;
    bool printInfos;
    bool sourceDisconnected;   //源断开
    bool webServerEnabled;
    bool webServerStarted;
    bool allowUpdatesOTA;
    bool enableDebugPings;
    bool autoBestHtotalEnabled;
    bool videoIsFrozen;
    bool forceRetime;
    bool motionAdaptiveDeinterlaceActive;
    bool deinterlaceAutoEnabled;
    bool scanlinesEnabled;
    bool boardHasPower;
    bool presetIsPalForce60;
    bool syncTypeCsync;
    bool isValidForScalingRGBHV;
    bool useHdmiSyncFix;
    bool extClockGenDetected;
    bool HdmiHoldDetection;
};
// remember adc options across presets
struct adcOptions
{

    uint8_t r_gain;
    uint8_t g_gain;
    uint8_t b_gain;
    uint8_t r_off;
    uint8_t g_off;
    uint8_t b_off;
};
#endif