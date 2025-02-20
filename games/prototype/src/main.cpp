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
#include "bn_math.h"


#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_sprite_ptr.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_items_a_button.h"
#include "bn_sprite_items_down_button.h"
#include "bn_colors.h"

namespace
{
    struct sram_data
    {
        bn::array<char, 32> format_tag;
        bool bounce = false;
    };

    bool readSram(){
        bn::array<bn::string_view, 4> info_text_lines;

        sram_data cart_sram_data;
        bn::sram::read(cart_sram_data);

        bn::array<char, 32> expected_format_tag;
        bn::istring_base expected_format_tag_istring(expected_format_tag._data);
        bn::ostringstream expected_format_tag_stream(expected_format_tag_istring);
        expected_format_tag_stream.append("PROTOTYPE");

        if(cart_sram_data.format_tag == expected_format_tag)
        {
            return cart_sram_data.bounce;
        }
        else
        {
            cart_sram_data.format_tag = expected_format_tag;
            cart_sram_data.bounce = false;

            bn::sram::clear(bn::sram::size());
            bn::sram::write(cart_sram_data);

            return false;
        }
    }

    void writeSram(bool bounce){
        sram_data cart_sram_data;

        bn::array<char, 32> format_tag;
        bn::istring_base format_tag_istring(format_tag._data);
        bn::ostringstream format_tag_stream(format_tag_istring);
        format_tag_stream.append("PROTOTYPE");

        cart_sram_data.format_tag=format_tag;
        cart_sram_data.bounce=bounce;

        bn::sram::write(cart_sram_data);
    }

    constexpr int DISTANCE = 3;
    constexpr float JUMP_VELOCITY = -10;
    constexpr float GRAVITY = 0.5;
    constexpr float AIR_RESISTANCE = 0.95;
    constexpr int GROUND_LEVEL = 50;
    constexpr float BOUNCE_FACTOR = 0.75f;
    constexpr float MIN_BOUNCE_VELOCITY = 1.0f;

    constexpr int MAX_BULLETS = 5;
    constexpr float BULLET_SPEED = 2.0f;
    constexpr int MAX_BULLET_DISTANCE = 100;

    struct Bullet {
        bn::sprite_ptr sprite;
        bool active = false;
        bn::fixed start_x = 0.0f;
        float velocity_x = 0.0f;
    };

}

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::bg_palettes::set_transparent_color(bn::color(16, 16, 16));
    
    bn::sprite_ptr sprite = bn::sprite_items::down_button.create_sprite(0, GROUND_LEVEL);
    bn::sprite_palette_ptr sprite_palette = sprite.palette();

    bn::vector<Bullet, MAX_BULLETS> bullets;

    for(int i = 0; i < MAX_BULLETS; i++) {
        bullets.push_back({bn::sprite_items::a_button.create_sprite(0, GROUND_LEVEL), false, 0.0f, 0.0f});
        bullets[i].sprite.set_scale(0.25);
        bullets[i].sprite.set_visible(false);
    }


    float velocity_x = 0.0f;
    float velocity_y = 0.0f;
    bool is_on_ground = true;
    bool bounce = readSram();

    bool lookingRight=true;

    while (true) {
        if(bn::keypad::start_pressed()){
            bounce = !bounce;
            writeSram(bounce);
        }

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

        if(abs(velocity_x) > 0){
            lookingRight = velocity_x > 0;
            sprite.set_horizontal_flip(!lookingRight);
        }

        if(lookingRight)
        {
            sprite_palette.set_fade(bn::colors::red, 0.5);
        }
        else
        {   
            sprite_palette.set_fade(bn::colors::orange, 0.5);
        }

        if (sprite.y() >= GROUND_LEVEL) {
            sprite.set_y(GROUND_LEVEL);

            if (bounce && abs(velocity_y) > MIN_BOUNCE_VELOCITY) {
                velocity_y = -velocity_y * BOUNCE_FACTOR;
            } else {
                velocity_y = 0;
                is_on_ground = true;
            }
        }

        if (bn::keypad::pressed(bn::keypad::key_type::B)) {
            for (auto& bullet : bullets) {
                if (!bullet.active) {
                    bullet.active = true;
                    bullet.sprite.set_visible(true);
                    bullet.start_x = sprite.x();
                    bullet.sprite.set_position(sprite.x(), sprite.y());
                    bullet.velocity_x = lookingRight ? BULLET_SPEED : -BULLET_SPEED;
                    break;
                }
            }
        }

        for (auto& bullet : bullets) {
            if (bullet.active) {
                bullet.sprite.set_x(bullet.sprite.x() + bullet.velocity_x);
                if (bn::abs(bullet.sprite.x() - bullet.start_x) > MAX_BULLET_DISTANCE) {
                    bullet.active = false;
                    bullet.sprite.set_visible(false);
                }
            }
        }

        bn::core::update();
    }
}
