#ifndef UTILS_H
#define UTILS_H

#include "bn_fixed_fwd.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_shape_size.h"

namespace Utils{
    constexpr int MAX_ENEMIES = 2;
    constexpr int GROUND_LEVEL = 64;
    constexpr int MAX_BLOCKS = 32;

    inline bool collision(bn::sprite_ptr& objectA, bn::sprite_ptr& objectB){
        bn::fixed objectA_radius = (objectA.shape_size().width() / 2) * objectA.horizontal_scale();
        bn::fixed objectB_radius = (objectB.shape_size().width() / 2) * objectB.horizontal_scale();
        bn::fixed max_distance = (objectA_radius + objectB_radius) * (objectA_radius + objectB_radius);

        bn::fixed distanceX = objectB.x() - objectA.x();
        bn::fixed distanceY = objectB.y() - objectA.y();
        bn::fixed distance = distanceX * distanceX + distanceY * distanceY;

        return (distance < max_distance);
    }
}

#endif // UTILS_H
