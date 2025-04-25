/* 
   See https://github.com/PSHarold/OLED-SSD1306-Menu 
   for original code, license and documentation
*/
#ifndef OLED_MENU_CONFIG_H_
#define OLED_MENU_CONFIG_H_
// include your translations here
#include "OLEDMenuTranslations.h"
#define OLED_MENU_WIDTH 128
#define OLED_MENU_HEIGHT 64
#define OLED_MENU_MAX_SUBITEMS_NUM 16 // should be less than 256
#define OLED_MENU_MAX_ITEMS_NUM 64    // should be less than 1024
#define OLED_MENU_MAX_DEPTH 5 // 子菜单的最高级别
#define OLED_MENU_REFRESH_INTERVAL_IN_MS 50 // not precise
#define OLED_MENU_SCREEN_SAVER_REFRESH_INTERVAL_IN_MS 5000 // not precise
#define OLED_MENU_SCROLL_LEAD_IN_TIME_IN_MS 600 // 项目被选中后开始滚动前的毫秒数
#define OLED_MENU_SCREEN_SAVER_KICK_IN_SECONDS 180 // OLED_MENU_SCREEN_SAVE_KICK_IN_SECONDS "秒后，屏幕保护程序将显示，直到按下任何按键为止
#define OLED_MENU_OVER_DRAW 0 // 如果设置为 0，则页面的最后一个菜单项在部分超出屏幕的情况下根本不会绘制，您需要向下滚动才能看到它们
#define OLED_MENU_RESET_ALWAYS_SCROLL_ON_SELECTION 0 // 如果设置为 1，滚动项目将在选择时重置到原始位置
#define OLED_MENU_WRAPPING_SPACE (OLED_MENU_WIDTH / 3)
#define REVERSE_ROTARY_ENCODER_FOR_OLED_MENU 1 // 如果设置为 1，旋转编码器将反向用于菜单导航
#define REVERSE_ROTARY_ENCODER_FOR_OSD 0 // 如果设置为 1，旋转编码器将反向用于 OSD 导航
#define OSD_TIMEOUT 6000 // OSD_TIMEOUT 毫秒后，OSD 将在无输入的情况下消失

// 请勿编辑这些内容
#define OLED_MENU_STATUS_BAR_HEIGHT (OLED_MENU_HEIGHT / 4) // status bar uses 1/4 of the screen
#define OLED_MENU_USABLE_AREA_HEIGHT (OLED_MENU_HEIGHT - OLED_MENU_STATUS_BAR_HEIGHT)
#define OLED_MENU_SCROLL_LEAD_IN_FRAMES (OLED_MENU_SCROLL_LEAD_IN_TIME_IN_MS / OLED_MENU_REFRESH_INTERVAL_IN_MS)
#endif