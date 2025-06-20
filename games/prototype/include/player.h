#ifndef PLAYER_H
#define PLAYER_H

#include "blockmap.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "utils.h"

using Utils::GROUND_LEVEL;
using Utils::DEATH_PANE;

struct Player {
    static constexpr int DISTANCE = 3;
    static constexpr bn::fixed JUMP_VELOCITY = -10;
    static constexpr bn::fixed AIR_RESISTANCE = 0.95;
    static constexpr bn::fixed BOUNCE_FACTOR = 0.75f;
    static constexpr int CAMERA_BORDER_X = 60;
    static constexpr int CAMERA_BORDER_Y = GROUND_LEVEL;
    static constexpr int JUMP_GRACE_PERIOD = 3;
    static constexpr int HIT_POINTS = 5;

    bn::sprite_ptr sprite;
    bn::sprite_palette_ptr palette;
    bn::fixed velocity_x = 0.0f;
    bn::fixed velocity_y = 0.0f;
    bool is_on_ground = true;
    bool lookingRight = true;
    bn::sprite_animate_action<4> animate_action;
    bn::fixed_rect rect;
    int framesSinceGround = 0;
    bn::vector<bn::sprite_ptr,HIT_POINTS> hearts;

    Player(int x, int y);

    void move(bool bounce, bn::camera_ptr& camera, BlockMap& blocks);

    int hitPoints();

    void hit();
};

#endif // PLAYER_H
