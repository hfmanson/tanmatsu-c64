#include "MenuController.hpp"
#include <cstring>
#include <string>
#include "Config.hpp"
#include "C64Emu.hpp"
#include "DisplayDriver.hpp"
#include "MainMenu.hpp"
// #include "esp_log.h"
#include "menuoverlay/MenuDataStore.hpp"
#include "menuoverlay/MenuTypes.hpp"
#include "pax_gfx.h"

#if defined(BOARD_MCH22)
#define TEXT_SIZE 12
#define LINE_SIZE 7
#define SCREENSIZE_X 320
#define SCREENSIZE_Y 200
#elif defined(BOARD_KONSOOL)
#define TEXT_SIZE 16
#define LINE_SIZE 10
#define SCREENSIZE_X 640
#define SCREENSIZE_Y 400
#else
#error no board configuration
#endif

#define BLACK 0xff000000
#define WHITE 0xffffffff
#define BLUE  0xff0000ff

__attribute__((unused)) static const char* TAG = "MenuController";

MenuController::MenuController()
{
    return;
}

void MenuController::init(C64Emu* c64emu)
{
    MenuController::c64emu = c64emu;
    // Setup the main menu
    rootMenu               = new MainMenu("Main Menu", nullptr, this);
    currentMenu            = rootMenu;
    // Initialize the menu overlay
    DisplayDriver* driver  = c64emu->cpu.vic->getDriver();
    fb                     = driver->getMenuFb();
    // HID Pax framebuffer
    pax_buf_init(fb, NULL, SCREENSIZE_X, SCREENSIZE_Y, PAX_BUF_16_565RGB);
    pax_buf_reversed(fb, true);
    pax_background(fb, BLACK);

    // Initialize the menus
    rootMenu->init();

    driver->enableMenuOverlay(visible);

    // Call render function to initialize the menu overlay
    render();
}

void MenuController::render()
{
    // Allow the current menu to update itself
    currentMenu->update();

    // Clear the screen
    pax_background(fb, BLACK);
    // Draw the menu items
    pax_draw_rect(fb, WHITE, 0, 0, SCREENSIZE_X, 4 * LINE_SIZE);
    pax_draw_text(fb, BLACK, pax_font_sky_mono, TEXT_SIZE, LINE_SIZE, LINE_SIZE, currentMenu->getTitle().c_str());

    const auto& items = currentMenu->getItems();
    // ESP_LOGI(TAG, "Menu items: %d", items.size());
    size_t      i     = 0;
    for (const auto& item : items) {
        uint32_t    color = currentMenu->getSelectedItemIndex() == i ? BLUE : WHITE;
        std::string title;
        switch (item.type) {
            case MenuItemType::TOGGLE: {
                bool checked = menuDataStore->getBool(item.value_name, false);
                title        = ("--> " + item.title + (checked ? "On" : "Off"));
                break;
            }
            case MenuItemType::SPACER: {
                title        = "--------------------";
                break;
            }
            default: {
                title = ("--> " + item.title);
                break;
            }
        }
        // ESP_LOGI(TAG, "Menu Item %d: %s", i, title.c_str());
        pax_draw_text(fb, color, pax_font_sky_mono, TEXT_SIZE, 3 * LINE_SIZE, 6 * LINE_SIZE + i * (2 * LINE_SIZE), title.c_str());
        ++i;
    }
}

void MenuController::show()
{
    visible = true;
}

void MenuController::hide()
{
    visible = false;
}

void MenuController::toggle()
{
    visible = !visible;
}

bool MenuController::getVisible() const
{
    return visible;
}

void MenuController::handleInput(menu_overlay_input_type_t input)
{
    // Handle user input for menu navigation and selection
    switch (input) {
        case MENU_OVERLAY_INPUT_TYPE_UP:
            currentMenu->navigateUp();
            break;
        case MENU_OVERLAY_INPUT_TYPE_DOWN:
            currentMenu->navigateDown();
            break;
        case MENU_OVERLAY_INPUT_TYPE_SELECT:
            currentMenu->activateItem(currentMenu->getSelectedItemIndex());
            break;
        case MENU_OVERLAY_INPUT_TYPE_LAST:
            if (currentMenu->getParentMenu() != nullptr) {
                currentMenu = currentMenu->getParentMenu();
            }
        default:
            break;
    }
    render();
}

// Implement other methods defined in MenuController.hpp