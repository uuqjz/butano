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
    constexpr float JUMP_VELOCITY = -10;
    constexpr float GRAVITY = 0.5;
    constexpr float AIR_RESISTANCE = 0.9;
    constexpr int GROUND_LEVEL = 50;

}

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::bg_palettes::set_transparent_color(bn::color(16, 16, 16));
    
    bn::sprite_ptr sprite = bn::sprite_items::a_button.create_sprite(0, 50);

    float velocity_x = 0.0f;
    float velocity_y = 0.0f;
    bool is_on_ground = true;

    while (true) {
        if (bn::keypad::held(bn::keypad::key_type::LEFT)) {
            velocity_x = -DISTANCE;
        }

        else if (bn::keypad::held(bn::keypad::key_type::RIGHT)) {
            velocity_x = DISTANCE;
        }

        else {
            velocity_x *= is_on_ground ? 0.0f : AIR_RESISTANCE;
        }

        if (bn::keypad::pressed(bn::keypad::key_type::A) && is_on_ground) {
            velocity_y = JUMP_VELOCITY;
            is_on_ground = false;
        }

        if (!is_on_ground) {
            velocity_y += GRAVITY;
        }

        sprite.set_position(sprite.x() + velocity_x, sprite.y() + velocity_y);

        if (sprite.y() >= GROUND_LEVEL) {
            sprite.set_y(GROUND_LEVEL);
            is_on_ground = true;
            velocity_y = 0;
        }

        bn::core::update();
    }
}
