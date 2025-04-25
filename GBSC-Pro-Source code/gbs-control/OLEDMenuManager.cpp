/*
   See https://github.com/PSHarold/OLED-SSD1306-Menu
   for original code, license and documentation
*/
#include "OLEDMenuManager.h"
#include "OLEDMenuFonts.h"
unsigned long OledUpdataTime;
OLEDMenuManager::OLEDMenuManager(SSD1306Wire *display)
    : display(display), rootItem(registerItem(nullptr, 0, nullptr))
{
    pushItem(rootItem);
    randomSeed(millis()); // does this work?
}
OLEDMenuItem *OLEDMenuManager::allocItem()
{
    OLEDMenuItem *newItem = nullptr;
    for (int i = 0; i < OLED_MENU_MAX_ITEMS_NUM; ++i)
    {
        if (!this->allItems[i].used)
        {
            memset(&this->allItems[i], 0, sizeof(OLEDMenuItem));
            newItem = &this->allItems[i];
            newItem->used = true;
            break;
        }
    }
    if (!newItem)
    {
        char msg[40];
        sprintf(msg, "Maximum number of items reached: %d", OLED_MENU_MAX_ITEMS_NUM);
        panicAndDisable(msg);
    }
    return newItem;
}
// 注册项目
OLEDMenuItem *OLEDMenuManager::registerItem(OLEDMenuItem *parent,
                                            uint16_t tag,
                                            uint16_t imageWidth,
                                            uint16_t imageHeight,
                                            const uint8_t *xbmImage,
                                            MenuItemHandler handler,
                                            OLEDDISPLAY_TEXT_ALIGNMENT alignment)
{
    OLEDMenuItem *newItem = allocItem();

    if (alignment == OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER_BOTH)
    {
        alignment = OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER;
    }
    newItem->imageWidth = imageWidth;
    newItem->imageHeight = imageHeight;
    newItem->xbmImage = xbmImage;

    newItem->tag = tag;
    newItem->handler = handler;
    newItem->alignment = alignment;
    if (parent)
    {
        parent->addSubItem(newItem);
        if (parent == rootItem)
        {
            cursor = 0;
            itemUnderCursor = rootItem->subItems[0];
        }
    }
    return newItem;
}
// 注册项目
OLEDMenuItem *OLEDMenuManager::registerItem(
    OLEDMenuItem *parent,
    uint16_t tag,
    const char *string,
    MenuItemHandler handler,
    const uint8_t *font,
    OLEDDISPLAY_TEXT_ALIGNMENT alignment)
{
    OLEDMenuItem *newItem = allocItem();
    newItem->str = string;
    if (font == nullptr)
    {
        font = DejaVu_Sans_Mono_12;
    }
    newItem->font = font;
    newItem->tag = tag;
    newItem->handler = handler;
    newItem->alignment = alignment;
    if (parent)
    {
        if (parent->numSubItem == OLED_MENU_MAX_SUBITEMS_NUM)
        {
            char msg[50];
            sprintf(msg, "Maximum number of sub items reached: %d", OLED_MENU_MAX_SUBITEMS_NUM);
            panicAndDisable(msg);
        }
        parent->addSubItem(newItem);
        if (parent == rootItem)
        {
            cursor = 0;
            itemUnderCursor = rootItem->subItems[0];
        }
    }
    return newItem;
}
// 绘制一个项目
void OLEDMenuManager::drawOneItem(OLEDMenuItem *item, uint16_t yOffset, bool negative)
{
    int16_t curScrollOffset = 0;
    if (negative)
    {
        this->display->setColor(OLEDDISPLAY_COLOR::WHITE); // display in negative, so fill white first, then draw image in black
        this->display->fillRect(0, yOffset, OLED_MENU_WIDTH, item->imageHeight);
        this->display->setColor(OLEDDISPLAY_COLOR::BLACK);
    }
    else
    {
        this->display->setColor(OLEDDISPLAY_COLOR::WHITE);
    }

    uint16_t wrappingOffset = OLED_MENU_WIDTH;
    if (item->imageWidth < OLED_MENU_WIDTH)
    {
        // 项目宽度 <= 屏幕宽度，不滚动
        curScrollOffset = item->alignOffset;
    }
    else if (item->imageWidth > OLED_MENU_WIDTH && (item == itemUnderCursor || item->alwaysScrolls))
    {
#if OLED_MENU_RESET_ALWAYS_SCROLL_ON_SELECTION
        // 滚动项目将在帧中产生引导（在滚动前，它们会在某些帧中保持不动）
        if (leadInFramesLeft == 0)
        {
            curScrollOffset = item->scrollOffset--;
        }
        else
        {
            --leadInFramesLeft;
        }
#else
        if (item == itemUnderCursor && !item->alwaysScrolls)
        {
            // 非始终滚动的项目会在帧中产生引导（它们在滚动前会在某些帧中保持静止）。
            if (leadInFramesLeft == 0)
            {
                curScrollOffset = item->scrollOffset--;
            }
            else
            {
                --leadInFramesLeft;
            }
        }
        else
        {
            curScrollOffset = item->scrollOffset--;
        }
#endif
        int16_t left = item->imageWidth + item->scrollOffset + 1;
        wrappingOffset = left + OLED_MENU_WRAPPING_SPACE;
        if (left <= 0)
        {
            curScrollOffset = item->scrollOffset = wrappingOffset;
            wrappingOffset = OLED_MENU_WIDTH + 1;
        }
    }
    else
    {
        // 已归零，但为便于理解，再次分配
        curScrollOffset = 0;
    }
    if (item->str)
    {
        // 让 OLEDDisplay 处理文本对齐问题
        this->display->setTextAlignment(item->alignment);
        this->display->setFont(item->font);
        this->display->drawString(curScrollOffset, yOffset, item->str);
        if (wrappingOffset < OLED_MENU_WIDTH)
        {
            this->display->drawString(wrappingOffset, yOffset, item->str);
        }
    }
    else
    {
        this->display->drawXbm(curScrollOffset, yOffset, item->imageWidth, item->imageHeight, item->xbmImage);
        if (wrappingOffset < OLED_MENU_WIDTH)
        {
            this->display->drawXbm(wrappingOffset, yOffset, item->imageWidth, item->imageHeight, item->xbmImage);
        }
    }
}
// 绘制子项目
void OLEDMenuManager::drawSubItems(OLEDMenuItem *parent)
{
    display->clear();
    drawStatusBar(itemUnderCursor == nullptr);
    uint16_t yOffset = OLED_MENU_STATUS_BAR_HEIGHT;
    uint8_t targetPage = itemUnderCursor == nullptr ? 0 : itemUnderCursor->pageInParent;
    for (int i = 0; i < parent->numSubItem; ++i)
    {
        OLEDMenuItem *subItem = parent->subItems[i];
#if OLED_MENU_OVER_DRAW
        if (subItem->pageInParent >= targetPage)
        {
#else
        if (subItem->pageInParent == targetPage)
        {
#endif
            bool negative = subItem == itemUnderCursor;
            drawOneItem(subItem, yOffset, negative);
            yOffset += subItem->imageHeight;
            if (itemUnderCursor == parent->subItems[i])
            {
                cursor = i;
            }
            if (subItem->pageInParent > targetPage)
            {
                // 过度绘制属于下一页的一个项目，以防该项目较大，给当前页留下巨大空白
                break;
            }
        }
    }
    display->display();
}
// 返回
void OLEDMenuManager::goBack(bool preserveCursor)
{
    // go back one page
    if (peakItem() != rootItem)
    {
        // cannot pop root
        if (state == OLEDMenuState::FREEZING)
        {
            unfreeze();
        }
        popItem(preserveCursor);
    }
    resetScroll();
    state = OLEDMenuState::GOING_BACK;
}
// 转到主页
void OLEDMenuManager::goMain(bool preserveCursor)
{
    while (peakItem() != rootItem)
    {
        goBack(preserveCursor);
    }
}

// 下一个项目
void OLEDMenuManager::nextItem()
{
    resetScroll();
    OLEDMenuItem *parentItem = peakItem();
    if (!parentItem->numSubItem)
    {
        // shouldn't happen
        return;
    }
    if (itemUnderCursor == nullptr)
    {
        // 选择状态栏
        itemUnderCursor = parentItem->subItems[cursor = 0];
    }
    else if (cursor == parentItem->numSubItem - 1)
    {
        if (parentItem == rootItem)
        {
            // 无法选择主菜单上的状态栏，绕行
            itemUnderCursor = parentItem->subItems[cursor = 0];
        }
        else
        {
            // 下一步选择状态栏
            itemUnderCursor = nullptr;
        }
    }
    else
    {
        itemUnderCursor = parentItem->subItems[++cursor];
    }
}
// 上一个项目
void OLEDMenuManager::prevItem()
{
    resetScroll();
    OLEDMenuItem *parentItem = peakItem();
    if (!parentItem->numSubItem)
    {
        // 不该发生
        return;
    }
    if (itemUnderCursor == nullptr)
    {
        // 选择状态栏
        cursor = parentItem->numSubItem - 1;
        itemUnderCursor = parentItem->subItems[cursor];
    }
    else if (cursor == 0)
    {
        if (parentItem == rootItem)
        {
            // 无法选择主菜单上的状态栏，绕行
            cursor = parentItem->numSubItem - 1;
            itemUnderCursor = parentItem->subItems[cursor];
        }
        else
        {
            // 下一步选择状态栏
            itemUnderCursor = nullptr;
        }
    }
    else
    {
        itemUnderCursor = parentItem->subItems[--cursor];
    }
}
// 重置滚动
void OLEDMenuManager::resetScroll()
{
    leadInFramesLeft = OLED_MENU_SCROLL_LEAD_IN_FRAMES;
    if (itemUnderCursor && itemUnderCursor->parent)
    {
        OLEDMenuItem *parent = itemUnderCursor->parent;
        for (int i = 0; i < itemUnderCursor->parent->numSubItem; ++i)
        {
            if (!OLED_MENU_RESET_ALWAYS_SCROLL_ON_SELECTION && parent->subItems[i]->alwaysScrolls)
            {
                continue;
            }
            parent->subItems[i]->scrollOffset = 0;
        }
    }
    lastUpdateTime = 0;
}
// 输入项目
void OLEDMenuManager::enterItem(OLEDMenuItem *item, OLEDMenuNav btn, bool isFirstTime)
{
    bool willEnter = true;
    if (this->state == OLEDMenuState::IDLE)
    {
        pushItem(item);
        this->state = OLEDMenuState::ITEM_HANDLING;
    }
    if (item->handler)
    {
        // 通知处理程序我们即将进入子菜单并绘制此项目的子项目，或者我们现在冻结。
        // 可以动态编辑子项目，或清除所有项目并注册新项目。
        // 如果不满足某些条件，处理程序可以返回 "false "来阻止管理器进入子菜单。
        // 如果没有子项目，则返回值无关紧要。
        // 此外，处理程序还可以调用 freeze() 来阻止管理器并打印自定义内容。如果是这样、
        // 管理器将持续调用处理程序，直到调用 unfreeze()。
        // 在此之前，处理程序负责处理输入。
        //
        willEnter = item->handler(this, item, btn, isFirstTime);
        if (this->state == OLEDMenuState::FREEZING)
        {
            return;
        }
        if (this->state == OLEDMenuState::GOING_BACK)
        {
            state = OLEDMenuState::IDLE;
            return;
        }
    }
    this->state = OLEDMenuState::IDLE;
    if (!willEnter || !item->numSubItem)
    {
        popItem();
        return;
    }
    resetScroll(); // 重置滚动
    cursor = 0;
    this->itemUnderCursor = item->subItems[0];
}
// 绘制状态栏
void OLEDMenuManager::drawStatusBar(bool negative)
{
    if (negative)
    {
        this->display->setColor(OLEDDISPLAY_COLOR::WHITE); // 以负片显示，因此先填充白色，然后绘制黑色
        this->display->fillRect(0, 0, OLED_MENU_WIDTH, OLED_MENU_STATUS_BAR_HEIGHT);
        this->display->setColor(OLEDDISPLAY_COLOR::BLACK);
    }
    else
    {
        this->display->setColor(OLEDDISPLAY_COLOR::WHITE);
    }
    if (peakItem() != rootItem)
    {
        // 不在主菜单上，画出返回按钮
        this->display->drawXbm(0, 0, IMAGE_ITEM(OM_STATUS_BAR_BACK));
    }
    else
    {
        // 主菜单，绘制一些自定义信息
        this->display->drawXbm(0, 0, IMAGE_ITEM(OM_STATUS_CUSTOM)); // OM_STATUS_CUSTOM
    }
    static uint8_t totalItems = 0;
    uint8_t curIndex = 1;
    this->display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_RIGHT);
    if (itemUnderCursor)
    {
        curIndex = cursor + 1;
        // itemUnderCursor 必须有一个父节点
        totalItems = itemUnderCursor->parent->numSubItem;
        // TODO 调整为 OLED_MENU_STATUS_BAR_HEIGHT
        this->display->setFont(DejaVu_Sans_Mono_10);
        this->display->drawStringf(OLED_MENU_WIDTH, 1, statusBarBuffer, "%d/%d", curIndex, totalItems);
    }
    else
    {
        this->display->drawString(OLED_MENU_WIDTH, 0, "     ");
    }
    this->display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_LEFT);
}
// 弹出项目
OLEDMenuItem *OLEDMenuManager::popItem(bool preserveCursor)
{
    if (itemSP == 1)
    {
        return rootItem; // cannot pop root
    }
    OLEDMenuItem *top = itemStack[--itemSP];
    if (preserveCursor)
    {
        for (int i = 0; i < top->parent->numSubItem; ++i)
        {
            // 可能需要更好的方法。也许可以在 OLEDMenuItems 中存储索引？但这值得吗？
            if (top == top->parent->subItems[i])
            {
                cursor = i;
                itemUnderCursor = top;
                return top;
            }
        }
        // 无效条目：在父条目子条目中找不到条目
        panicAndDisable("invalid item: item cannot be found in its parent's subItems");
        return nullptr;
    }
    cursor = 0;
    itemUnderCursor = top->parent->subItems[0];
    return top;
}

void OLEDMenuManager::tick(OLEDMenuNav nav)
{
    if (this->disabled)
    {
        return;
    }
    if (state == OLEDMenuState::FREEZING)
    {
        // 处理程序不会受到 OLED_MENU_REFRESH_INTERVAL_IN_MS 的影响，并且会在每次勾选时被调用
        enterItem(peakItem(), nav, false); // 输入项目
        return;
    }

    unsigned long curMili = millis();
    if (nav == OLEDMenuNav::IDLE)  //空闲
    { 
        if(OledUpdataTime == 1)
        {
            lastKeyPressedTime = curMili;
            OledUpdataTime = 0;
        }
        if (curMili - lastKeyPressedTime > OLED_MENU_SCREEN_SAVER_KICK_IN_SECONDS * 1000)
        {
            if (curMili - screenSaverLastUpdateTime > OLED_MENU_SCREEN_SAVER_REFRESH_INTERVAL_IN_MS)
            {
                drawScreenSaver();
                screenSaverLastUpdateTime = curMili;
            }
            return;
        }
        else if (curMili - this->lastUpdateTime < OLED_MENU_REFRESH_INTERVAL_IN_MS)
        {
            return;
        }
        
    }
    else
    {
        lastKeyPressedTime = curMili;
    }
    switch (nav)
    {
    case OLEDMenuNav::UP:
        prevItem(); // 上一个项目
        break;
    case OLEDMenuNav::DOWN:
        nextItem(); // 下一个项目
        break;
    case OLEDMenuNav::ENTER:
        if (itemUnderCursor) // 项目游标下
        {
            enterItem(itemUnderCursor, OLEDMenuNav::IDLE, true);
        }
        else
        {
            // 选择状态栏
            goBack();
            this->state = OLEDMenuState::IDLE;
        }
        break;
    default:
        break;
    }
    if (this->state != OLEDMenuState::FREEZING)
    { // 绘制子项目
        drawSubItems(peakItem());
        this->lastUpdateTime = curMili;
    }
}