#include "block.h"
#include "bn_sprite_items_block.h"

Block::Block(int x, int y) : sprite(bn::sprite_items::block.create_sprite(x * SIZE, y * SIZE)),
              rect(sprite.x(), sprite.y(), SIZE, SIZE) {
        }
