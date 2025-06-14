#ifndef BLOCK_H
#define BLOCK_H

#include "bn_sprite_ptr.h"
#include "bn_fixed_rect.h"

struct Block {
    static constexpr int SIZE = 16;

    bn::sprite_ptr sprite;
    bn::fixed_rect rect;

    Block(int x, int y);
};

#endif // BLOCK_H
