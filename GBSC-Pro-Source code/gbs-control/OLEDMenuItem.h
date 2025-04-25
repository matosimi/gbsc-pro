/*
   See https://github.com/PSHarold/OLED-SSD1306-Menu
   for original code, license and documentation
*/
#ifndef OLED_MENU_ITEM_H_
#define OLED_MENU_ITEM_H_
#include <string.h>
#include "SSD1306Wire.h"
#include "OLEDMenuConfig.h"

class OLEDMenuItem;
class OLEDMenuManager;
enum class OLEDMenuNav;
typedef bool (*MenuItemHandler)(OLEDMenuManager *, OLEDMenuItem *, OLEDMenuNav, bool);

class OLEDMenuItem
{
    friend class OLEDMenuManager;

private:
    OLEDDISPLAY_TEXT_ALIGNMENT alignment;
    void addSubItem(OLEDMenuItem *item)
    {
        if (item)
        {
            item->parent = this;
            this->subItems[this->numSubItem++] = item;
            item->calculate();
        }
    }

    //  }
    void clearSubItems()
    {
        for (int i = 0; i < this->numSubItem; ++i)
        {
            subItems[i]->used = false;
            subItems[i] = nullptr;
        }
        this->numSubItem = 0;
        this->maxPageIndex = 0;
    }
    void calculate();

public:
    // static uint
    bool used;
    uint16_t tag;
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint16_t alignOffset; // used if imageWidth < OLED_MENU_WIDTH
    uint8_t pageInParent;
    uint8_t maxPageIndex;
    uint8_t numSubItem = 0; // 子项目数
    const uint8_t *xbmImage;
    const char *str;
    const uint8_t *font;
    MenuItemHandler handler;
    OLEDMenuItem *parent;
    bool alwaysScrolls;
    int16_t scrollOffset;
    OLEDMenuItem *subItems[OLED_MENU_MAX_SUBITEMS_NUM];
};
#endif