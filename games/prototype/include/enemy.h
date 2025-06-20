#ifndef ENEMY_H
#define ENEMY_H

#include "bn_random.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_palette_ptr.h"
#include "player.h"
#include "utils.h"
#include "enemytype.h"
#include "bn_camera_ptr.h"

using Utils::MAX_ENEMIES;

struct Enemy {
    EnemyType type;
    bn::sprite_ptr sprite;
    bn::sprite_palette_ptr palette;
    int hit_points = 3;
    bn::sprite_animate_action<3> animate_action;
    bn::fixed spawnX;
    bool lookingRight = false;
    bn::fixed_rect rect;
    bn::fixed fallingSpeed = 0;

    Enemy(int x, int y, EnemyType t);

    bool isColliding(Player& player, bn::vector<Enemy, MAX_ENEMIES>& enemies);

    static void respawn(int& framesBeforeRespawn, Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::camera_ptr& camera, bn::random& random);

    void move(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, int& framesSinceLastHit, BlockMap& blocks);

    static void moveAll(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, int& framesSinceLastHit, BlockMap& blocks);

    static void removeDead(bn::vector<Enemy,MAX_ENEMIES>& enemies, int& framesBeforeRespawn);
};

#endif // ENEMY_H
