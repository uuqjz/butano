#include "enemy.h"
#include "bn_sprite_item.h"
#include "bn_sprite_items_monsters.h"

constexpr int DINO_OFFSET = -4;

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
