#ifndef ENEMY_H
#define ENEMY_H

#include "bn_sprite_animate_actions.h"
#include "bn_sprite_palette_ptr.h"

#include "enemytype.h"

struct Enemy {
    EnemyType type;
    bn::sprite_ptr sprite;
    bn::sprite_palette_ptr palette;
    int hit_points = 3;
    bn::sprite_animate_action<3> animate_action;

    Enemy(int x, int y, EnemyType t);
};

#endif // ENEMY_H
