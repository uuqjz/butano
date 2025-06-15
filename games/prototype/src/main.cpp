#include "bn_core.h"
#include "bn_string.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_random.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_sprite_ptr.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_colors.h"
#include "bn_cameras.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_items_rocket.h"
#include "bn_sprite_items_monsters.h"
#include "bn_sprite_items_head.h"
#include "bn_blending.h"
#include "bn_fixed_rect.h"
#include <bn_fixed_point.h>
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_items_clouds.h"
#include "bn_regular_bg_items_ground.h"

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

namespace
{
    constexpr int PLAYER_HIT_POINTS = 5;
    constexpr int INVINCIBILITY_FRAMES = 100;

    constexpr int MAX_BULLETS = 5;

    void spawnAndMoveBullets(bn::vector<Bullet, MAX_BULLETS>& bullets, Player& player){
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

    constexpr int RESPAWN_TIMER = 100;
    constexpr int SPEED = 1;

    bool isColliding(Enemy& enemy, Player& player, bn::vector<Enemy, MAX_ENEMIES>& enemies) {
        if (Utils::collision(enemy.sprite, player.sprite)) {
            return true;
        }

        for (auto& otherenemy : enemies) {
            if (&enemy != &otherenemy && Utils::collision(enemy.sprite, otherenemy.sprite)) {
                return true;
            }
        }

        return false;
    }

    void respawnEnemies(bn::random& random, int& framesBeforeRespawn, Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::camera_ptr& camera){
        framesBeforeRespawn++;
        if (enemies.size() < MAX_ENEMIES && framesBeforeRespawn > RESPAWN_TIMER) {
            bool dino = (random.get_int(2) == 0);
            Enemy new_enemy(0, GROUND_LEVEL, dino ? EnemyType::DINO : EnemyType::TURTLE);
            new_enemy.sprite.set_camera(camera);

            int enemy_x;
            do {
                enemy_x = random.get_int(-100, 100);
                new_enemy.sprite.set_x(enemy_x+camera.x());
            } 
            while (isColliding(new_enemy, player, enemies));

            enemies.push_back(new_enemy);
            framesBeforeRespawn = 0;
        }
    }

    void moveEnemiesToPlayer(Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::vector<bn::sprite_ptr,PLAYER_HIT_POINTS>& hearts, int& framesSinceLastHit){
        framesSinceLastHit++;
        if(framesSinceLastHit > INVINCIBILITY_FRAMES){
            player.sprite.set_blending_enabled(false);
        }
        for (auto& enemy : enemies){
            bool direction = player.sprite.x() > enemy.sprite.x();
            int steps = SPEED * (direction ? 1 : -1);

            enemy.sprite.set_x(enemy.sprite.x() + steps);

            if (framesSinceLastHit > INVINCIBILITY_FRAMES && Utils::collision(enemy.sprite, player.sprite)){
                framesSinceLastHit=0;
                if(hearts.size()>0){
                    hearts.pop_back();
                }
                player.sprite.set_blending_enabled(true);
            }

            if(isColliding(enemy,player,enemies)){
                enemy.sprite.set_x(enemy.sprite.x() - steps);
            }

            enemy.sprite.set_horizontal_flip(!direction);
        }
    }
}

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

    bn::regular_bg_ptr ground_bg = bn::regular_bg_items::ground.create_bg(8, -152);
    ground_bg.set_camera(camera);
    
    bn::vector<Bullet, MAX_BULLETS> bullets;

    for(int i = 0; i < MAX_BULLETS; i++) {
        bullets.push_back({bn::sprite_items::rocket.create_sprite(0, GROUND_LEVEL)});
        bullets[i].sprite.set_scale(0.5);
        bullets[i].sprite.set_visible(false);
        bullets[i].sprite.set_camera(camera);
    }

    bn::vector<bn::sprite_ptr,PLAYER_HIT_POINTS> hearts;
    for(int i = 0; i < PLAYER_HIT_POINTS; i++){
        hearts.push_back({bn::sprite_items::head.create_sprite(-100+(i*20), -60)});
    }

    Player player = {0, GROUND_LEVEL};

    bn::vector<Enemy,MAX_ENEMIES> enemies;
    enemies.push_back({-100,GROUND_LEVEL,EnemyType::DINO});
    enemies.push_back({100,GROUND_LEVEL,EnemyType::TURTLE});

    BlockMap blocks;
    blocks.insert(-1,1);
    blocks.insert(1,1);
    blocks.insert(2,1);
    blocks.insert(3,1);
    blocks.insert(1,2);
    blocks.insert(2,2);
    blocks.insert(3,2);
    blocks.insert(5,-2);
    blocks.insert(6,-2);

    int framesBeforeRespawn=0;
    int framesSinceLastHit=INVINCIBILITY_FRAMES;
    bn::random random;

    player.sprite.set_camera(camera);
    for(auto& enemy:enemies){
        enemy.sprite.set_camera(camera);
    }
    for(auto& block:blocks.getAllBlocks()){
        block.sprite.set_camera(camera);
    }

    bool bounce = Save::read();
    bool enemiesActive = false;

    while (hearts.size()>0) {
        if(bn::keypad::start_pressed()){
            enemiesActive = !enemiesActive;
        }

        if(bn::keypad::select_pressed()){
            bounce = !bounce;
            Save::write(bounce);
        }

        player.move(bounce,camera,blocks);

        spawnAndMoveBullets(bullets,player);

        for (auto& bullet : bullets) {
            bullet.hitDetection(enemies, framesBeforeRespawn);
        }

        respawnEnemies(random,framesBeforeRespawn,player,enemies,camera);

        if(enemiesActive){
            moveEnemiesToPlayer(player,enemies,hearts,framesSinceLastHit);    
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
