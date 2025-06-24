#include "enemymanager.h"

using Utils::framesBeforeRespawn;
using Utils::framesSinceLastHit;
using Utils::random_spawn;
using Utils::INVINCIBILITY_FRAMES;

constexpr int RESPAWN_TIMER = 100;
constexpr int RESPAWN_RANGE = 100;

void EnemyManager::add(int x, int y, EnemyType type){
    enemies.push_back({x,y,type});
}

void EnemyManager::set_camera(const bn::camera_ptr& camera){
    for(auto& enemy:enemies){
        enemy.sprite.set_camera(camera);
    }
}

void EnemyManager::update(){
    for(auto& enemy : enemies){
        enemy.animate_action.update();
    }
}

void EnemyManager::respawn(Player& player, bn::camera_ptr& camera){
    framesBeforeRespawn++;
    if (enemies.size() < MAX_ENEMIES && framesBeforeRespawn > RESPAWN_TIMER) {
        bool dino = (random_spawn.get_int(2) == 0);
        Enemy new_enemy(0, GROUND_LEVEL, dino ? EnemyType::DINO : EnemyType::TURTLE);
        new_enemy.sprite.set_camera(camera);

        int enemy_x;
        do {
            enemy_x = random_spawn.get_int(-RESPAWN_RANGE, RESPAWN_RANGE);
            new_enemy.sprite.set_x(enemy_x+camera.x());
            new_enemy.rect.set_position(new_enemy.sprite.position());
        }
        while (new_enemy.isColliding(player, enemies) || new_enemy.collidesWithBlock());

        new_enemy.spawnX = new_enemy.sprite.x();
        enemies.push_back(new_enemy);
        framesBeforeRespawn = 0;
    }
}

void EnemyManager::removeDead(){
    for (int i = 0; i < enemies.size(); i++) {
        auto& enemy = enemies[i];
        if (enemy.hit_points == 0) {
            enemies.erase(enemies.begin() + i);
            i--;
            framesBeforeRespawn=0;
        }
    }
}

void EnemyManager::move(Player& player){
    framesSinceLastHit++;
    if(framesSinceLastHit > INVINCIBILITY_FRAMES){
        player.sprite.set_blending_enabled(false);
    }

    for (auto& enemy : enemies){
        enemy.move(player,enemies);
    }
}
