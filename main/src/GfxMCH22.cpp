/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 for more details.

 For the complete text of the GNU General Public License see
 http://www.gnu.org/licenses/.
*/
#include <cstdint>
#include "Config.hpp"

#ifdef USE_GFXMCH22

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "hal/color_types.h"
#include "pax_types.h"
extern "C" {
#include <stddef.h>
#include "bsp/display.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "freertos/idf_additions.h"
}

#include "GfxMCH22.hpp"
#include "HardwareInitializationException.h"
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include "hal/lcd_types.h"

#include "bsp/display.h"
#include "pax_fonts.h"
#include "pax_gfx.h"
#include "pax_text.h"

static const char* TAG = "GfxMCH22";

void GfxMCH22::init()
{
    // Enable backlight
    esp_err_t res = bsp_display_get_panel(&display_lcd_panel);
    ESP_ERROR_CHECK(res);                             // Check that the display handle has been initialized
    bsp_display_get_panel_io(&display_lcd_panel_io);  // Do not check result of panel IO handle: not all types of
                                                      // display expose a panel IO handle
    res = bsp_display_get_parameters(&display_h_res, &display_v_res, &display_color_format, NULL);
    ESP_ERROR_CHECK(res);  // Check that the display parameters have been initialized

    // Setup variables
    border_height  = (display_v_res - vic_v_height) / 2;
    
    raw_fb = (uint16_t*)heap_caps_calloc(display_h_res * border_height, sizeof(uint16_t),
                                         MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    for (int i = 0; i < display_h_res * border_height; i++) {
        raw_fb[i] = c64_lightblue;
    }   
    esp_lcd_panel_draw_bitmap(display_lcd_panel, 0, 0, display_h_res, border_height, raw_fb);
    esp_lcd_panel_draw_bitmap(display_lcd_panel, 0, display_v_res - border_height, display_h_res, display_v_res, raw_fb);
    heap_caps_free(raw_fb);
    
    ESP_LOGI(TAG, "display_h_res: %d, display_v_res: %d", display_h_res, display_v_res);
}

void IRAM_ATTR GfxMCH22::drawFrame(uint16_t* frameColors)
{
}

void GfxMCH22::drawBitmap(uint16_t* bitmap)
{
    uint16_t* frame_buffer = menu_overlay_enabled ? fb.buf_16bpp : bitmap;
    static int y_offset = 0;
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(display_lcd_panel, 0, y_offset + border_height, display_h_res, vic_v_height / 2 + y_offset + border_height, frame_buffer + y_offset * display_h_res));
    y_offset += vic_v_height / 2;
    if (y_offset == vic_v_height) {
        y_offset = 0;
    }
}

void GfxMCH22::enableMenuOverlay(bool enable)
{
    menu_overlay_enabled = enable;
}

pax_buf_t* GfxMCH22::getMenuFb()
{
    return &fb;
}

const uint16_t* GfxMCH22::getC64Colors() const
{
    return c64Colors;
}
#endif /* USE_GFXMCH22 */
