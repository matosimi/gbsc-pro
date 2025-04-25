#include "OLEDMenuItem.h"
#include <string.h>
void OLEDMenuItem::calculate()
{
    if (!this->parent)
    {
        return; // root
    }
    if (this->str)
    {
        size_t strLength = strlen(this->str);
        this->imageWidth = pgm_read_byte(this->font) * strLength;
        this->imageHeight = pgm_read_byte(this->font + 1);
        // 让 OLED Display 处理文本对齐，以实现完美对齐。
        // 当然，如果我们能将文本视为图像（使用 imageWidth），效果会更好、
        // 但如果使用非单行字体，对齐方式就会有偏差。
        // 如果使用非行距字体，计算出的 imageWidth 总是会比较大，这样就会出现奇怪的情况、
        // 因此最好只使用单行距字体）。
        if (this->imageWidth <= OLED_MENU_WIDTH)
        {
            switch (this->alignment)
            {
            case OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_LEFT:
                this->alignOffset = 0;
                break;
            case OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER:
                this->alignOffset = OLED_MENU_WIDTH / 2;
                break;
            case OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_RIGHT:
                this->alignOffset = OLED_MENU_WIDTH;
                break;
            default:
                break;
            }
        }
    }
    else if (this->imageWidth <= OLED_MENU_WIDTH)
    {
        switch (this->alignment)
        {
        case OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER:
        default:
            this->alignOffset = (OLED_MENU_WIDTH - this->imageWidth) / 2;
            break;
        case OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_RIGHT:
            this->alignOffset = OLED_MENU_WIDTH - this->imageWidth;
            break;
        }
    }

    if (this->imageWidth > OLED_MENU_WIDTH)
    {
        // 滚动时，居中对齐没有意义。改为左对齐。
        this->alignment = OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_LEFT;
    }

    // 决定此项目属于哪个页面
    uint16_t lastLineAt = 0;
    for (int i = 0; i < parent->numSubItem - 1; ++i)
    {
        // 找到最后一页的所有项目，看看是否合适
        // 如果不合适，把它放到下一页。
        // parent->numSubItem 中的最后一个项目总是 "this"
        OLEDMenuItem *item = parent->subItems[i];
        if (item->pageInParent != parent->maxPageIndex)
        {
            continue;
        }
        lastLineAt += item->imageHeight;
    }
    lastLineAt += imageHeight; // self

    if (lastLineAt <= OLED_MENU_USABLE_AREA_HEIGHT)
    {
        this->pageInParent = parent->maxPageIndex;
    }
    else
    {
        this->pageInParent = ++parent->maxPageIndex;
    }
}