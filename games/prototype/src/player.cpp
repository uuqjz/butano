#include "player.h"
#include "bn_camera_ptr.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_sprite_item.h"
#include "bn_sprite_items_ninja.h"

Player::Player(int x, int y)             : sprite(bn::sprite_items::ninja.create_sprite(x, y)),
    palette(sprite.palette()),
    animate_action(bn::create_sprite_animate_action_forever(
        sprite, 16, bn::sprite_items::ninja.tiles_item(), 12, 13, 14, 15)),
    rect(sprite.position(),sprite.dimensions()) {
}

void Player::move(bool bounce, bn::camera_ptr& camera, BlockMap& blocks){
    if (bn::keypad::held(bn::keypad::key_type::LEFT)) {
        velocity_x = -Player::DISTANCE;
    }

    else if (bn::keypad::held(bn::keypad::key_type::RIGHT)) {
        velocity_x = DISTANCE;
    }

    else {
        velocity_x *= is_on_ground ? 0.0f : AIR_RESISTANCE;
    }

    if (bn::keypad::pressed(bn::keypad::key_type::A) && (is_on_ground || framesSinceGround < JUMP_GRACE_PERIOD)) {
        velocity_y = JUMP_VELOCITY;
        is_on_ground = false;
    }

    if (!is_on_ground) {
        velocity_y += GRAVITY;
        framesSinceGround++;
    }

    sprite.set_position(sprite.x() + velocity_x, sprite.y() + velocity_y);
    bn::fixed_rect prev_rect = rect;
    rect.set_position(sprite.position());

    bool standingOnBlock = false;

    for(auto& tile : getTiles()){
        if(blocks.contains(tile)){
            auto& block = blocks.at(tile);

            if(rect.intersects(block.rect)){
                bool fromAbove = prev_rect.bottom() <= block.rect.top();
                bool fromBelow = prev_rect.top() >= block.rect.bottom();

                if (fromBelow) {
                    sprite.set_y(block.rect.bottom() + sprite.dimensions().height() / 2);
                    velocity_y = 0;
                }
                else if (fromAbove) {
                    sprite.set_y(block.rect.top() - sprite.dimensions().height() / 2);
                    velocity_y = 0;
                    standingOnBlock = true;
                }

                bool fromLeft = prev_rect.right() <= block.rect.left();
                bool fromRight = prev_rect.left() >= block.rect.right();

                if (fromLeft) {
                    sprite.set_x(block.rect.left() - sprite.dimensions().width() / 2);
                    velocity_x = 0;
                }
                else if (fromRight) {
                    sprite.set_x(block.rect.right() + sprite.dimensions().width() / 2);
                    velocity_x = 0;
                }
            }

            rect.set_position(sprite.position());
        }
    }

    if(standingOnBlock){
        is_on_ground = true;
        framesSinceGround=0;
    } else if(sprite.y() < GROUND_LEVEL){
        is_on_ground = false;
    }

    bool rightFromCamera = sprite.x() > camera.x();
    if(bn::abs(sprite.x()-camera.x())>CAMERA_BORDER_X){
        int delta = (rightFromCamera ? -1 : 1) * CAMERA_BORDER_X;
        camera.set_x(sprite.x()+delta);
    }

    bool overCamera = sprite.y() < camera.y();
    if(bn::abs(sprite.y() - camera.y()) > CAMERA_BORDER_Y){
        int delta = (overCamera ? 1 : -1) * CAMERA_BORDER_Y;
        camera.set_y(bn::min(bn::fixed(0), sprite.y() + delta));
    }

    bool changedDirection = false;
    if(bn::abs(velocity_x) > 0){
        bool lookingRightNow = velocity_x > 0;
        if(lookingRight!=lookingRightNow){
            lookingRight=lookingRightNow;
            changedDirection = true;
        }
    }

    if(changedDirection){
        if(lookingRight)
        {
            animate_action = bn::create_sprite_animate_action_forever(
                sprite, 16, bn::sprite_items::ninja.tiles_item(), 12, 13, 14, 15);
        }
        else
        {
            animate_action = bn::create_sprite_animate_action_forever(
                sprite, 16, bn::sprite_items::ninja.tiles_item(), 8, 9, 10, 11);
        }
    }

    if (sprite.y() >= GROUND_LEVEL) {
        sprite.set_y(GROUND_LEVEL);

        if (bounce && bn::abs(velocity_y) > MIN_BOUNCE_VELOCITY) {
            velocity_y = -velocity_y * BOUNCE_FACTOR;
        } else {
            velocity_y = 0;
            is_on_ground = true;
            framesSinceGround=0;
        }
    }
}

bn::vector<bn::fixed_point, 9> Player::getTiles()
{
    int x0 = int(sprite.x()) / Block::BLOCK_SIZE;
    int y0 = int(sprite.y()) / Block::BLOCK_SIZE;

    bn::vector<bn::fixed_point, 9> tiles;

    for(int x = x0 -1 ; x<= x0+1; x++){
        for(int y = y0 -1 ; y<= y0+1; y++){
            tiles.push_back(bn::fixed_point(x,y));
        }
    }

    return tiles;
}
