#include "bn_core.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_keypad.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_sprite_ptr.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_items_rocket_scaled.h"
#include "bn_blending.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_items_clouds.h"
#include "bn_random.h"

#include "utils.h"
#include "block.h"
#include "blockmap.h"
#include "player.h"
#include "save.h"
#include "bullet.h"
#include "enemytype.h"
#include "enemy.h"

using Utils::GROUND_LEVEL;
using Utils::MAX_ENEMIES;
using Utils::MAX_BULLETS;
using Utils::INVINCIBILITY_FRAMES;

namespace Utils {
    BlockMap blocks;
    int framesBeforeRespawn=0;
    int framesSinceLastHit=INVINCIBILITY_FRAMES;
    bn::random random_spawn;
}

using Utils::blocks;

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::bg_palettes::set_transparent_color(bn::color(16, 16, 16));

    bn::camera_ptr camera = bn::camera_ptr::create(0, 0);

    bn::regular_bg_ptr clouds_bg = bn::regular_bg_items::clouds.create_bg(0, 0);
    bn::blending::set_transparency_alpha(0.5);
    clouds_bg.set_blending_enabled(true);
    clouds_bg.set_camera(camera);
    
    bn::vector<Bullet, MAX_BULLETS> bullets;

    for(int i = 0; i < MAX_BULLETS; i++) {
        bullets.push_back({0, GROUND_LEVEL});
        bullets[i].sprite.set_camera(camera);
    }

    Player player = {0, GROUND_LEVEL};

    bn::vector<Enemy,MAX_ENEMIES> enemies;
    enemies.push_back({-100,GROUND_LEVEL,EnemyType::DINO});
    enemies.push_back({100,GROUND_LEVEL,EnemyType::TURTLE});

    blocks.insert(-1,1);
    blocks.insert(1,1);
    blocks.insert(2,1);
    blocks.insert(3,1);
    blocks.insert(1,2);
    blocks.insert(2,2);
    blocks.insert(3,2);
    blocks.insert(5,-2);
    blocks.insert(6,-2);
    blocks.insertLine(-20,20,5);
    blocks.insert(-2,4);
    blocks.insert(-23,5);
    blocks.insert(-24,5);

    player.sprite.set_camera(camera);
    for(auto& enemy:enemies){
        enemy.sprite.set_camera(camera);
    }
    for(auto& block:blocks.getAllBlocks()){
        block.sprite.set_camera(camera);
    }

    bool bounce = Save::read();
    bool enemiesActive = false;

    while (player.hitPoints()>0) {
        if(bn::keypad::start_pressed()){
            enemiesActive = !enemiesActive;
        }

        if(bn::keypad::select_pressed()){
            bounce = !bounce;
            Save::write(bounce);
        }

        player.move(bounce,camera);

        Bullet::spawnAndMove(bullets,player);

        for (auto& bullet : bullets) {
            bullet.hitDetection(enemies);
        }

        Enemy::removeDead(enemies);

        Enemy::respawn(player,enemies,camera);

        if(enemiesActive){
            Enemy::moveAll(player,enemies);
        }

        for(auto& enemy : enemies){
            enemy.animate_action.update();
        }

        player.animate_action.update();
        bn::core::update();
    }

    text_generator.set_center_alignment();

    bn::vector<bn::sprite_ptr, 32> text_sprites;
    text_generator.generate(0, -10, "GAME OVER", text_sprites);
    text_generator.generate(0, 10, "PRESS START", text_sprites);

    while(!bn::keypad::start_pressed()){
        bn::core::update();
    }
}
