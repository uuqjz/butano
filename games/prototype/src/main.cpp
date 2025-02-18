/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_sram.h"
#include "bn_string.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_keypad.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_sprite_ptr.h"
#include "bn_sprite_items_a_button.h"

namespace
{
    struct sram_data
    {
        bn::array<char, 32> format_tag;
        int reads_count = 0;
    };

    void sram(bn::sprite_text_generator& text_generator){
        bn::array<bn::string_view, 4> info_text_lines;

        sram_data cart_sram_data;
        bn::sram::read(cart_sram_data);

        bn::array<char, 32> expected_format_tag;
        bn::istring_base expected_format_tag_istring(expected_format_tag._data);
        bn::ostringstream expected_format_tag_stream(expected_format_tag_istring);
        expected_format_tag_stream.append("SRAM example");

        bn::string<32> sram_reads_count;
        bn::string<32> number_str;

        if(cart_sram_data.format_tag == expected_format_tag)
        {
            ++cart_sram_data.reads_count;

            sram_reads_count = bn::to_string<32>(cart_sram_data.reads_count);

            number_str = "SRAM is formatted!"+bn::to_string<32>(69);
            info_text_lines[0] = number_str;
            info_text_lines[1] = "";
            info_text_lines[2] = "SRAM reads count:";
            info_text_lines[3] = sram_reads_count;
        }
        else
        {
            cart_sram_data.format_tag = expected_format_tag;
            cart_sram_data.reads_count = 1;

            info_text_lines[0] = "SRAM is not formatted";
            info_text_lines[1] = "";
            info_text_lines[2] = "If you see this message again,";
            info_text_lines[3] = "SRAM is not working";

            bn::sram::clear(bn::sram::size());
        }

        bn::sram::write(cart_sram_data);

        common::info info("SRAM", bn::span<const bn::string_view>(info_text_lines), text_generator);

        info.set_show_always(true);

        while(! bn::keypad::start_pressed())
        {
            info.update();
            bn::core::update();
        }
    }

    constexpr int DISTANCE = 3;
    constexpr int HELD_THRESHOLD = 15;

    void move_sprite(bn::sprite_ptr& sprite, bn::keypad::key_type key, int dx, int dy, int& held) {
        if (bn::keypad::pressed(key)) {
            sprite.set_position(sprite.x() + dx, sprite.y() + dy);
            held = 0;
        } else if (bn::keypad::held(key)) {
            if (++held == HELD_THRESHOLD) {
                sprite.set_position(sprite.x() + dx, sprite.y() + dy);
                held = 0;
            }
        } else if (bn::keypad::released(key)) {
            held = 0;
        }
    }
}

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::bg_palettes::set_transparent_color(bn::color(16, 16, 16));

    int i = 0;
    while(i++<2){
        sram(text_generator);
        bn::core::update();
    }
    
    bn::sprite_ptr sprite = bn::sprite_items::a_button.create_sprite(0, 0);

    int held = 0;
    while (true) {
        move_sprite(sprite, bn::keypad::key_type::UP, 0, -DISTANCE, held);
        move_sprite(sprite, bn::keypad::key_type::DOWN, 0, DISTANCE, held);
        move_sprite(sprite, bn::keypad::key_type::LEFT, -DISTANCE, 0, held);
        move_sprite(sprite, bn::keypad::key_type::RIGHT, DISTANCE, 0, held);
        if(bn::keypad::start_pressed()){
            sprite.set_position(0,0);
        }
        bn::core::update();
    }
}
