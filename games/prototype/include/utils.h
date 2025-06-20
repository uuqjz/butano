#ifndef UTILS_H
#define UTILS_H

#include "block.h"
#include "blockmap.h"
#include "bn_fixed_fwd.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_shape_size.h"
#include "bn_vector.h"

namespace Utils{
    constexpr int MAX_ENEMIES = 2;
    constexpr int GROUND_LEVEL = 64;
    constexpr int DEATH_PANE = 90;
    constexpr int MAX_BULLETS = 5;
    constexpr int INVINCIBILITY_FRAMES = 100;
    constexpr int MAX_NEIGHBORING_BLOCKS = 25;
    constexpr bn::fixed GRAVITY = 0.5;

    extern BlockMap blocks;

    inline bool collision(bn::sprite_ptr& objectA, bn::sprite_ptr& objectB){
        bn::fixed objectA_radius = (objectA.shape_size().width() / 2) * objectA.horizontal_scale();
        bn::fixed objectB_radius = (objectB.shape_size().width() / 2) * objectB.horizontal_scale();
        bn::fixed max_distance = (objectA_radius + objectB_radius) * (objectA_radius + objectB_radius);

        bn::fixed distanceX = objectB.x() - objectA.x();
        bn::fixed distanceY = objectB.y() - objectA.y();
        bn::fixed distance = distanceX * distanceX + distanceY * distanceY;

        return (distance < max_distance);
    }

    inline bn::vector<bn::fixed_point, MAX_NEIGHBORING_BLOCKS> getTiles(bn::sprite_ptr& sprite)
    {
        int x0 = int(sprite.x()) / Block::SIZE;
        int y0 = int(sprite.y()) / Block::SIZE;

        bn::vector<bn::fixed_point, MAX_NEIGHBORING_BLOCKS> tiles;

        int border = sprite.shape_size().width() / Block::SIZE;

        for(int x = x0 - border ; x<= x0 + border; x++){
            for(int y = y0 - border ; y<= y0 + border; y++){
                tiles.push_back(bn::fixed_point(x,y));
            }
        }

        return tiles;
    }
}

#endif // UTILS_H
