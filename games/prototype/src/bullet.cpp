#include "bullet.h"
#include "bn_keypad.h"
#include "bn_math.h"

constexpr bn::fixed SPEED = 2.0f;
constexpr int MAX_DISTANCE = 100;

void Bullet::fire(bn::fixed x, bn::fixed y, bool right) {
    if (active) return;

    active = true;
    sprite.set_visible(true);
    sprite.set_position(x, y);
    sprite.set_rotation_angle(right ? 270 : 90);
    start_x = x;
    velocity_x = right ? SPEED : -SPEED;
}

void Bullet::update() {
    if (!active) return;

    sprite.set_x(sprite.x() + velocity_x);

    if (bn::abs(sprite.x() - start_x) > MAX_DISTANCE) {
        active = false;
        sprite.set_visible(false);
    }
}

void Bullet::hitDetection(bn::vector<Enemy,MAX_ENEMIES>& enemies){
    if (active) {
        for (auto& enemy : enemies){
            if (Utils::collision(sprite,enemy.sprite)) {
                enemy.hit_points--;
                active = false;
                sprite.set_visible(false);
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
