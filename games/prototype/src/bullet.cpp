#include "bullet.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_sprite_items_rocket_scaled.h"

using Utils::blocks;

constexpr bn::fixed SPEED = 2.0f;
constexpr int MAX_DISTANCE = 100;

Bullet::Bullet(int x, int y) : sprite(bn::sprite_items::rocket_scaled.create_sprite(x, y)),
    rect(sprite.position(),sprite.dimensions()) {
    sprite.set_visible(false);
}

void Bullet::fire(bn::fixed x, bn::fixed y, bool right) {
    if (active) return;

    active = true;
    sprite.set_visible(true);
    sprite.set_position(x, y);
    rect.set_position(sprite.position());
    sprite.set_horizontal_flip(!right);
    start_x = x;
    velocity_x = right ? SPEED : -SPEED;
}

void Bullet::update() {
    if (!active) return;

    sprite.set_x(sprite.x() + velocity_x);
    rect.set_position(sprite.position());

    if (bn::abs(sprite.x() - start_x) > MAX_DISTANCE) {
        active = false;
        sprite.set_visible(false);
    }
}

void Bullet::hitDetection(bn::vector<Enemy,MAX_ENEMIES>& enemies){
    if (active) {
        for (auto& enemy : enemies){
            if (rect.intersects(enemy.rect)) {
                enemy.hit_points--;
                active = false;
                sprite.set_visible(false);
            }
        }
    }

    if(active){
        for(auto& tile : Utils::getTiles(sprite)){
            if(blocks.contains(tile)){
                auto& block = blocks.at(tile);

                if(rect.intersects(block.rect)){
                    active = false;
                    sprite.set_visible(false);
                }
            }
        }
    }
}

void Bullet::spawnAndMove(bn::vector<Bullet, MAX_BULLETS>& bullets, Player& player){
    if (bn::keypad::pressed(bn::keypad::key_type::B)) {
        for (auto& bullet : bullets) {
            if (!bullet.active) {
                bullet.fire(player.sprite.x(), player.sprite.y(), player.lookingRight);
                break;
            }
        }
    }

    for (auto& bullet : bullets) {
        bullet.update();
    }
}
