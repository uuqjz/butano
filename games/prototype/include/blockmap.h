#ifndef BLOCKMAP_H
#define BLOCKMAP_H

#include "bn_unordered_map.h"
#include "bn_vector.h"
#include "block.h"
#include "utils.h"

using Utils::MAX_BLOCKS;

struct BlockMap {
    void insert(int x, int y) {
        map.insert(bn::fixed_point(x, y), {x, y});
    }

    void erase(int x, int y) {
        map.erase(bn::fixed_point(x, y));
    }

    void erase(bn::fixed_point& point) {
        map.erase(point);
    }

    bool contains(int x, int y) const {
        return map.contains(bn::fixed_point(x,y));
    }

    bool contains(bn::fixed_point& point) const {
        return map.contains(point);
    }

    const Block& at(int x, int y) const {
        return map.at(bn::fixed_point(x, y));
    }

    const Block& at(bn::fixed_point& point) const {
        return map.at(point);
    }

    bn::vector<Block,MAX_BLOCKS> getAllBlocks() const {
        bn::vector<Block,MAX_BLOCKS> blocks;

        for (const auto& [key, block] : map) {
            blocks.push_back(block);
        }

        return blocks;
    }

    bn::unordered_map<bn::fixed_point, Block, MAX_BLOCKS> map;
};

#endif // BLOCKMAP_H
