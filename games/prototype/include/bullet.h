#ifndef BULLET_H
#define BULLET_H

#include <bn_sprite_ptr.h>

struct Bullet {
    bn::sprite_ptr sprite;
    bool active = false;
    bn::fixed start_x = 0.0f;
    bn::fixed velocity_x = 0.0f;

    void fire(bn::fixed x, bn::fixed y, bool right);

    void update();
};

#endif // BULLET_H
