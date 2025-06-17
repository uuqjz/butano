#include "enemy.h"
#include "bn_sprite_item.h"
#include "bn_sprite_items_monsters_grounded.h"
#include "bn_random.h"

using Utils::INVINCIBILITY_FRAMES;

constexpr int RESPAWN_TIMER = 100;
constexpr int SPEED = 1;
constexpr int RESPAWN_RANGE = 100;
constexpr int OSCILLATION_RANGE = 15;
constexpr int OFFSET = -8;

Enemy::Enemy(int x, int y, EnemyType t) : type(t),
    sprite(bn::sprite_items::monsters_grounded.create_sprite(x, y + OFFSET)),
    palette(sprite.palette()),
    animate_action(bn::create_sprite_animate_action_forever(
        sprite, 16, bn::sprite_items::monsters_grounded.tiles_item(),
        type == EnemyType::DINO ? 0 : 3,
        type == EnemyType::DINO ? 1 : 4,
        type == EnemyType::DINO ? 2 : 5))
{
    sprite.set_scale(0.5f);
    spawnX = sprite.x();
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
            enemy_x = random.get_int(-RESPAWN_RANGE, RESPAWN_RANGE);
            new_enemy.sprite.set_x(enemy_x+camera.x());
        }
        while (new_enemy.isColliding(player, enemies));

        new_enemy.spawnX = new_enemy.sprite.x();
        enemies.push_back(new_enemy);
        framesBeforeRespawn = 0;
    }
}

void Enemy::move(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, int& framesSinceLastHit){
    int steps = 0;

    if(type==EnemyType::DINO){
        if(bn::abs(player.sprite.x() - sprite.x()) > SPEED){
            lookingRight = player.sprite.x() > sprite.x();
            steps = SPEED * (lookingRight ? 1 : -1);
        }
    }

    else if(type==EnemyType::TURTLE){
        if(lookingRight){
            if(sprite.x()+SPEED<spawnX+OSCILLATION_RANGE){
                steps = SPEED;
            }
            else{
                lookingRight = false;
            }
        }
        else{
            if(sprite.x()-SPEED>spawnX-OSCILLATION_RANGE){
                steps = -SPEED;
            }
            else{
                lookingRight = true;
            }

        }
    }

    sprite.set_x(sprite.x() + steps);

    if (framesSinceLastHit > INVINCIBILITY_FRAMES && Utils::collision(sprite, player.sprite)){
        framesSinceLastHit=0;
        player.hit();
        player.sprite.set_blending_enabled(true);
    }

    if(isColliding(player,enemies)){
        sprite.set_x(sprite.x() - steps);
    }

    sprite.set_horizontal_flip(!lookingRight);
}

void Enemy::moveAll(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, int& framesSinceLastHit){
    framesSinceLastHit++;
    if(framesSinceLastHit > INVINCIBILITY_FRAMES){
        player.sprite.set_blending_enabled(false);
    }

    for (auto& enemy : enemies){
        enemy.move(player,enemies,framesSinceLastHit);
    }
}
