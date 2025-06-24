#include "enemy.h"
#include "bn_sprite_item.h"
#include "bn_sprite_items_monsters.h"

using Utils::INVINCIBILITY_FRAMES;
using Utils::GRAVITY;
using Utils::blocks;
using Utils::framesSinceLastHit;

constexpr int SPEED = 1;
constexpr int OSCILLATION_RANGE = 15;
constexpr int OFFSET = -8;

Enemy::Enemy(int x, int y, EnemyType t) : type(t),
    sprite(bn::sprite_items::monsters.create_sprite(x, y + OFFSET)),
    palette(sprite.palette()),
    animate_action(bn::create_sprite_animate_action_forever(
        sprite, 16, bn::sprite_items::monsters.tiles_item(),
        type == EnemyType::DINO ? 0 : 3,
        type == EnemyType::DINO ? 1 : 4,
        type == EnemyType::DINO ? 2 : 5)),
    rect(sprite.position(),sprite.dimensions())
{
    spawnX = sprite.x();
}

bool Enemy::isColliding(Player& player, bn::vector<Enemy, MAX_ENEMIES>& enemies) {
    if (rect.intersects(player.rect)) {
        return true;
    }

    for (auto& otherenemy : enemies) {
        if (this != &otherenemy && rect.intersects(otherenemy.rect)) {
            return true;
        }
    }

    return false;
}

bool Enemy::collidesWithBlock(){
    for(auto& tile : Utils::getTiles(sprite)){
        if(blocks.contains(tile)){
            auto& block = blocks.at(tile);

            if(rect.intersects(block.rect)){
                return true;
            }
        }
    }

    return false;
}

void Enemy::move(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies){
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
    sprite.set_y(sprite.y() + fallingSpeed);

    if (framesSinceLastHit > INVINCIBILITY_FRAMES && rect.intersects(player.rect)){
        framesSinceLastHit=0;
        player.hit();
        player.sprite.set_blending_enabled(true);
    }

    if(isColliding(player,enemies)){
        sprite.set_x(sprite.x() - steps);
    }

    sprite.set_horizontal_flip(!lookingRight);

    bn::fixed_rect prev_rect = rect;
    rect.set_position(sprite.position());

    bool standingOnBlock = false;

    for(auto& tile : Utils::getTiles(sprite)){
        if(blocks.contains(tile)){
            auto& block = blocks.at(tile);

            if(rect.intersects(block.rect)){
                bool fromAbove = prev_rect.bottom() <= block.rect.top();

                if (fromAbove) {
                    sprite.set_y(block.rect.top() - sprite.dimensions().height() / 2);
                    standingOnBlock = true;
                }

                bool fromLeft = prev_rect.right() <= block.rect.left();
                bool fromRight = prev_rect.left() >= block.rect.right();

                if (fromLeft) {
                    sprite.set_x(block.rect.left() - sprite.dimensions().width() / 2);
                }
                else if (fromRight) {
                    sprite.set_x(block.rect.right() + sprite.dimensions().width() / 2);
                }
            }

            rect.set_position(sprite.position());
        }
    }

    if (standingOnBlock) {
        fallingSpeed = 0;
    }
    else{
        fallingSpeed += GRAVITY;
    }

    if(sprite.y() > DEATH_PANE){
        hit_points=0;
    }
}
