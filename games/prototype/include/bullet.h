#ifndef BULLET_H
#define BULLET_H

#include "bn_sprite_ptr.h"
#include "bn_vector.h"
#include "enemy.h"
#include "utils.h"

using Utils::MAX_ENEMIES;
using Utils::MAX_BULLETS;

struct Bullet {
    bn::sprite_ptr sprite;
    bool active = false;
    bn::fixed start_x = 0.0f;
    bn::fixed velocity_x = 0.0f;

    void fire(bn::fixed x, bn::fixed y, bool right);

    void update();

    void hitDetection(bn::vector<Enemy,MAX_ENEMIES>& enemies, int& framesBeforeRespawn);

    static void spawnAndMove(bn::vector<Bullet, MAX_BULLETS>& bullets, Player& player);
};

#endif // BULLET_H
