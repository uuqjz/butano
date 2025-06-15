#include "enemy.h"
#include "bn_sprite_item.h"
#include "bn_sprite_items_monsters.h"
#include <bn_random.h>

using Utils::INVINCIBILITY_FRAMES;

constexpr int DINO_OFFSET = -4;
constexpr int RESPAWN_TIMER = 100;
constexpr int SPEED = 1;

Enemy::Enemy(int x, int y, EnemyType t) : type(t),
    sprite(bn::sprite_items::monsters.create_sprite(x, y + (type == EnemyType::DINO ? DINO_OFFSET : 0))),
    palette(sprite.palette()),
    animate_action(bn::create_sprite_animate_action_forever(
        sprite, 16, bn::sprite_items::monsters.tiles_item(),
        type == EnemyType::DINO ? 0 : 3,
        type == EnemyType::DINO ? 1 : 4,
        type == EnemyType::DINO ? 2 : 5))
{
    sprite.set_scale(0.5f);
}

bool Enemy::isColliding(Player& player, bn::vector<Enemy, MAX_ENEMIES>& enemies) {
    if (Utils::collision(sprite, player.sprite)) {
        return true;
    }

    for (auto& otherenemy : enemies) {
        if (this != &otherenemy && Utils::collision(sprite, otherenemy.sprite)) {
            return true;
        }
    }

    return false;
}

void Enemy::respawn(int& framesBeforeRespawn, Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::camera_ptr& camera){
    bn::random random;
    framesBeforeRespawn++;
    if (enemies.size() < MAX_ENEMIES && framesBeforeRespawn > RESPAWN_TIMER) {
        bool dino = (random.get_int(2) == 0);
        Enemy new_enemy(0, GROUND_LEVEL, dino ? EnemyType::DINO : EnemyType::TURTLE);
        new_enemy.sprite.set_camera(camera);

        int enemy_x;
        do {
            enemy_x = random.get_int(-100, 100);
            new_enemy.sprite.set_x(enemy_x+camera.x());
        }
        while (new_enemy.isColliding(player, enemies));

        enemies.push_back(new_enemy);
        framesBeforeRespawn = 0;
    }
}

void Enemy::moveToPlayer(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::vector<bn::sprite_ptr,PLAYER_HIT_POINTS>& hearts, int& framesSinceLastHit){
    framesSinceLastHit++;
    if(framesSinceLastHit > INVINCIBILITY_FRAMES){
        player.sprite.set_blending_enabled(false);
    }
    for (auto& enemy : enemies){
        bool direction = player.sprite.x() > enemy.sprite.x();
        int steps = SPEED * (direction ? 1 : -1);

        enemy.sprite.set_x(enemy.sprite.x() + steps);

        if (framesSinceLastHit > INVINCIBILITY_FRAMES && Utils::collision(enemy.sprite, player.sprite)){
            framesSinceLastHit=0;
            if(hearts.size()>0){
                hearts.pop_back();
            }
            player.sprite.set_blending_enabled(true);
        }

        if(enemy.isColliding(player,enemies)){
            enemy.sprite.set_x(enemy.sprite.x() - steps);
        }

        enemy.sprite.set_horizontal_flip(!direction);
    }
}
