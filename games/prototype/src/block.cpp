#include "block.h"
#include "bn_sprite_items_block.h"

Block::Block(int x, int y): sprite(bn::sprite_items::block.create_sprite(x * BLOCK_SIZE, y * BLOCK_SIZE)),
              rect(sprite.x(), sprite.y(), BLOCK_SIZE, BLOCK_SIZE) {
        }
