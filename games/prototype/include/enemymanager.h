#ifndef ENEMYMANAGER_H
#define ENEMYMANAGER_H

#include "bn_camera_ptr.h"
#include "enemy.h"

struct EnemyManager {

    bn::vector<Enemy,MAX_ENEMIES> enemies;

    void add(int x, int y, EnemyType type);

    void set_camera(const bn::camera_ptr& camera);

    void update();

    void respawn(Player& player, bn::camera_ptr& camera);

    void removeDead();

    void move(Player& player);
};

#endif // ENEMYMANAGER_H
