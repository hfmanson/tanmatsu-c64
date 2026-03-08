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
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "hal/color_types.h"
#include "hal/ppa_types.h"
#include "pax_types.h"
extern "C" {
#include <stddef.h>
#include "bsp/display.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "freertos/idf_additions.h"
}
#ifdef USE_GFXMCH22
#include "GfxMCH22.hpp"
#include "HardwareInitializationException.h"
// #include <FreeRTOS.h>
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include "hal/lcd_types.h"

// #include <task.h>
#include "bsp/display.h"
#include "pax_fonts.h"
#include "pax_gfx.h"
#include "pax_text.h"

static const char* TAG = "GfxMCH22";

// Global variables
// static esp_lcd_panel_handle_t       display_lcd_panel    = NULL;
// static esp_lcd_panel_io_handle_t    display_lcd_panel_io = NULL;
// static size_t                       display_h_res        = 0;
// static size_t                       display_v_res        = 0;
// static lcd_color_rgb_pixel_format_t display_color_format;
// static pax_buf_t                    fb                = {};
// static QueueHandle_t                input_event_queue = NULL;

// static uint32_t lu_pinbitmask[256];

void GfxMCH22::blit(void)
{
    esp_lcd_panel_draw_bitmap(this->display_lcd_panel, 0, 0, this->display_h_res, this->display_v_res,
                              pax_buf_get_pixels(&this->fb));
}

void GfxMCH22::writeCmd(uint8_t cmd)
{
    // Write a command to the display
}

void GfxMCH22::writeData(uint8_t data)
{
    // Write a byte of data to the display
}

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
    border_width  = (display_v_res - vic_h_width) / 2;
    border_height = (display_h_res - vic_v_height) / 2;

    ESP_LOGI(TAG, "display_h_res: %d, display_v_res: %d", display_h_res, display_v_res);

    // allocate raw framebuffer memory
    // raw_fb = (uint16_t*)calloc(display_h_res * display_v_res, sizeof(uint16_t));
    raw_fb = (uint16_t*)heap_caps_calloc(display_h_res * display_v_res, sizeof(uint16_t),
                                         MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "raw_fb: %p", raw_fb);
}

void GfxMCH22::copyinit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
    // Start copy on display?? not sure
    // uint16_t x1 = x0 + w - 1;
    // uint16_t y1 = y0 + h - 1;
}

void GfxMCH22::copycopy(uint16_t data, uint32_t clearMask)
{
    // Copy data on display ?? not sure
}

void GfxMCH22::copyend()
{
    // End copy on display?? not sure
}

void GfxMCH22::copyColor(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t data)
{
}

uint32_t GfxMCH22::rgb565ToRgb8888(uint16_t rgb565)
{

    uint8_t r5 = (rgb565 >> 11) & 0x1F;
    uint8_t g6 = (rgb565 >> 5) & 0x3F;
    uint8_t b5 = rgb565 & 0x1F;

    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g6 << 2) | (g6 >> 4);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);

    return (0xFF << 24) | (r8 << 16) | (g8 << 8) | b8;  // ARGB8888
}

void IRAM_ATTR GfxMCH22::drawFrame(uint16_t* frameColors)
{
}

void GfxMCH22::drawMenuOverlay()
{
}

void GfxMCH22::drawBitmap(uint16_t* bitmap)
{

    // Send the frame to the display over MIPI DSI.
    //ESP_LOGI(TAG, "raw_fb: %p", raw_fb);
	//memcpy(raw_fb, bitmap, display_h_res * display_v_res);
    //ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(display_lcd_panel, 0, 0, display_h_res, display_v_res, raw_fb));
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(display_lcd_panel, 0, 0, 320, 208, bitmap));
	//ESP_ERROR_CHECK(bsp_display_blit(0, 0, display_h_res, display_v_res, bitmap));
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
#endif
