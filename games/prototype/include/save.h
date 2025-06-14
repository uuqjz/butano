#ifndef SAVE_H
#define SAVE_H

#include "bn_sram.h"

struct sram_data
{
    int format_tag;
    bool bounce = false;
};

constexpr int expected_format_tag = 721;

namespace Save
{
    inline bool read(){
        sram_data cart_sram_data;
        bn::sram::read(cart_sram_data);

        if(cart_sram_data.format_tag == expected_format_tag)
        {
            return cart_sram_data.bounce;
        }
        else
        {
            cart_sram_data.format_tag = expected_format_tag;
            cart_sram_data.bounce = false;

            bn::sram::clear(bn::sram::size());
            bn::sram::write(cart_sram_data);

            return false;
        }
    }

    inline void write(bool bounce){
        sram_data cart_sram_data;
        bn::sram::read(cart_sram_data);

        if(cart_sram_data.format_tag != expected_format_tag)
        {
            cart_sram_data.format_tag = expected_format_tag;
        }

        cart_sram_data.bounce=bounce;
        bn::sram::write(cart_sram_data);
    }
};

#endif // SAVE_H
