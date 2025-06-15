#ifndef ENEMY_H
#define ENEMY_H

#include "bn_sprite_animate_actions.h"
#include "bn_sprite_palette_ptr.h"
#include "player.h"
#include "utils.h"
#include "enemytype.h"
#include "bn_camera_ptr.h"

using Utils::MAX_ENEMIES;
using Utils::PLAYER_HIT_POINTS;

struct Enemy {
    EnemyType type;
    bn::sprite_ptr sprite;
    bn::sprite_palette_ptr palette;
    int hit_points = 3;
    bn::sprite_animate_action<3> animate_action;

    Enemy(int x, int y, EnemyType t);

    bool isColliding(Player& player, bn::vector<Enemy, MAX_ENEMIES>& enemies);

    static void respawn(int& framesBeforeRespawn, Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::camera_ptr& camera);

    static void moveToPlayer(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::vector<bn::sprite_ptr,PLAYER_HIT_POINTS>& hearts, int& framesSinceLastHit);
};

#endif // ENEMY_H
