#pragma once
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
#include "DisplayDriver.hpp"
#include "esp_lcd_types.h"
#include "hal/lcd_types.h"
#include "pax_types.h"

static constexpr uint16_t swap16(uint16_t v) {
    return (uint16_t)((v >> 8) | (v << 8));
}

class GfxMCH22 : public DisplayDriver {
   private:
    static const uint16_t c64_black      = swap16(0x0000);
    static const uint16_t c64_white      = swap16(0xffff);
    static const uint16_t c64_red        = swap16(0x8000);
    static const uint16_t c64_turquoise  = swap16(0xa7fc);
    static const uint16_t c64_purple     = swap16(0xc218);
    static const uint16_t c64_green      = swap16(0x064a);
    static const uint16_t c64_blue       = swap16(0x0014);
    static const uint16_t c64_yellow     = swap16(0xe74e);
    static const uint16_t c64_orange     = swap16(0xd42a);
    static const uint16_t c64_brown      = swap16(0x6200);
    static const uint16_t c64_lightred   = swap16(0xfbae);
    static const uint16_t c64_grey1      = swap16(0x3186);
    static const uint16_t c64_grey2      = swap16(0x73ae);
    static const uint16_t c64_lightgreen = swap16(0xa7ec);
    static const uint16_t c64_lightblue  = swap16(0x043f);
    static const uint16_t c64_grey3      = swap16(0xb5d6);

    static const uint16_t vic_h_width  = 320;
    static const uint16_t vic_v_height = 200;

    pax_buf_t                    fb;
    uint16_t*                    raw_fb;
    esp_lcd_panel_handle_t       display_lcd_panel;
    esp_lcd_panel_io_handle_t    display_lcd_panel_io;
    lcd_color_rgb_pixel_format_t display_color_format;
    size_t                       border_width;
    size_t                       border_height;
    size_t                       display_h_res;
    size_t                       display_v_res;
    uint16_t                     frame_mem_size;

    const uint16_t c64Colors[16] = {
        c64_black, c64_white,      c64_red,       c64_turquoise, c64_purple,   c64_green,
        c64_blue,  c64_yellow,     c64_orange,    c64_brown,     c64_lightred, c64_grey1,
        c64_grey2, c64_lightgreen, c64_lightblue, c64_grey3
    };

    bool menu_overlay_enabled = true;

   public:
    void               init() override;
    void               drawFrame(uint16_t* frameColors) override;
    void               drawBitmap(uint16_t* bitmap) override;
    const uint16_t*    getC64Colors() const override;
    void               enableMenuOverlay(bool enable) override;
    virtual pax_buf_t* getMenuFb() override;
};
