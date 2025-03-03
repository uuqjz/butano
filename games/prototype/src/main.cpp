#include "bn_core.h"
#include "bn_sram.h"
#include "bn_string.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_log.h"
#include "bn_random.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_sprite_ptr.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_colors.h"
#include "bn_cameras.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_items_ninja.h"
#include "bn_sprite_items_block.h"
#include "bn_sprite_items_rocket.h"
#include "bn_sprite_items_monsters.h"
#include "bn_sprite_items_head.h"
#include "bn_blending.h"
#include "bn_fixed_rect.h"
#include "bn_unordered_map.h"
#include <bn_fixed_point.h>

namespace
{
    struct sram_data
    {
        bn::array<char, 32> format_tag;
        bool bounce = false;
    };

    bool readSram(){
        bn::array<bn::string_view, 4> info_text_lines;

        sram_data cart_sram_data;
        bn::sram::read(cart_sram_data);

        bn::array<char, 32> expected_format_tag;
        bn::istring_base expected_format_tag_istring(expected_format_tag._data);
        bn::ostringstream expected_format_tag_stream(expected_format_tag_istring);
        expected_format_tag_stream.append("PROTOTYPE");

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

    void writeSram(bool bounce){
        sram_data cart_sram_data;

        bn::array<char, 32> format_tag;
        bn::istring_base format_tag_istring(format_tag._data);
        bn::ostringstream format_tag_stream(format_tag_istring);
        format_tag_stream.append("PROTOTYPE");

        cart_sram_data.format_tag=format_tag;
        cart_sram_data.bounce=bounce;

        bn::sram::write(cart_sram_data);
    }

    constexpr int MAX_BLOCKS = 32;
    constexpr int BLOCK_SIZE = 16;
    struct Block {
        bn::sprite_ptr sprite;
        bn::fixed_rect rect;

        Block(int x, int y) 
            : sprite(bn::sprite_items::block.create_sprite(x * BLOCK_SIZE, y * BLOCK_SIZE)),
              rect(sprite.x(), sprite.y(), BLOCK_SIZE, BLOCK_SIZE) {
        }
    };

    constexpr int DISTANCE = 3;
    constexpr bn::fixed JUMP_VELOCITY = -10;
    constexpr bn::fixed GRAVITY = 0.5;
    constexpr bn::fixed AIR_RESISTANCE = 0.95;
    constexpr int GROUND_LEVEL = 60;
    constexpr bn::fixed BOUNCE_FACTOR = 0.75f;
    constexpr bn::fixed MIN_BOUNCE_VELOCITY = 1.0f;
    constexpr int CAMERA_BORDER_X = 100;
    constexpr int CAMERA_BORDER_Y = 60;
    constexpr int PLAYER_HIT_POINTS = 5;
    constexpr int INVINCIBILITY_FRAMES = 100;

    struct Player {
        bn::sprite_ptr sprite;
        bn::sprite_palette_ptr palette;
        bn::fixed velocity_x = 0.0f;
        bn::fixed velocity_y = 0.0f;
        bool is_on_ground = true;
        bool lookingRight = true;
        bn::sprite_animate_action<4> animate_action;
        bn::fixed_rect rect;

        Player(const bn::sprite_item& sprite_item, int x, int y) 
            : sprite(sprite_item.create_sprite(x, y)),
            palette(sprite.palette()),
            animate_action(bn::create_sprite_animate_action_forever(
                  sprite, 16, sprite_item.tiles_item(), 12, 13, 14, 15)),
            rect(sprite.position(),sprite.dimensions()) {
        }

        void move(bool bounce, bn::camera_ptr& camera, bn::unordered_map<bn::fixed_point, Block, MAX_BLOCKS>& blocks){
            if (bn::keypad::held(bn::keypad::key_type::LEFT)) {
                velocity_x = -DISTANCE;
            }

            else if (bn::keypad::held(bn::keypad::key_type::RIGHT)) {
                velocity_x = DISTANCE;
            }

            else {
                velocity_x *= is_on_ground ? 0.0f : AIR_RESISTANCE;
            }

            if (bn::keypad::pressed(bn::keypad::key_type::A) && is_on_ground) {
                velocity_y = JUMP_VELOCITY;
                is_on_ground = false;
            }

            if (!is_on_ground) {
                velocity_y += GRAVITY;
            }

            sprite.set_position(sprite.x() + velocity_x, sprite.y() + velocity_y);
            bn::fixed_rect prev_rect = rect;
            rect.set_position(sprite.position());

            bool standingOnBlock = false;

            for(auto& tile : getTiles()){
                if(blocks.contains(tile)){
                    auto& block = blocks.at(tile);

                    if(rect.touches(block.rect)){ 
                        bool fromAbove = prev_rect.bottom() <= block.rect.top();
                        bool fromBelow = prev_rect.top() >= block.rect.bottom();

                        if (fromBelow) {
                            sprite.set_y(block.rect.bottom() + sprite.dimensions().height() / 2);
                            velocity_y = 0;
                        }
                        else if (fromAbove) {
                            sprite.set_y(block.rect.top() - sprite.dimensions().height() / 2);
                            velocity_y = 0;
                            standingOnBlock = true;
                        }
                    }

                    if(rect.intersects(block.rect)){ 
                        bool fromLeft = prev_rect.right() <= block.rect.left();
                        bool fromRight = prev_rect.left() >= block.rect.right();

                        if (fromLeft) {
                            sprite.set_x(block.rect.left() - sprite.dimensions().width() / 2);
                            velocity_x = 0;
                        }
                        else if (fromRight) {
                            sprite.set_x(block.rect.right() + sprite.dimensions().width() / 2);
                            velocity_x = 0;
                        }
                    }

                    rect.set_position(sprite.position());   
                }
            }

            if(standingOnBlock){
                is_on_ground = true;
            } else if(sprite.y() < GROUND_LEVEL){
                is_on_ground = false;
            }

            bool rightFromCamera = sprite.x() > camera.x();
            if(bn::abs(sprite.x()-camera.x())>CAMERA_BORDER_X){
                int delta = (rightFromCamera ? -1 : 1) * CAMERA_BORDER_X;
                camera.set_x(sprite.x()+delta);
            }

            bool overCamera = sprite.y() < camera.y();
            if(bn::abs(sprite.y() - camera.y()) > CAMERA_BORDER_Y){
                int delta = (overCamera ? 1 : -1) * CAMERA_BORDER_Y;
                camera.set_y(bn::min(bn::fixed(0), sprite.y() + delta));
            }

            bool changedDirection = false;
            if(bn::abs(velocity_x) > 0){
                bool lookingRightNow = velocity_x > 0;
                if(lookingRight!=lookingRightNow){
                    lookingRight=lookingRightNow;
                    changedDirection = true;
                }
            }

            if(changedDirection){
                if(lookingRight)
                {
                    animate_action = bn::create_sprite_animate_action_forever(
                    sprite, 16, bn::sprite_items::ninja.tiles_item(), 12, 13, 14, 15);
                }
                else
                {   
                    animate_action = bn::create_sprite_animate_action_forever(
                    sprite, 16, bn::sprite_items::ninja.tiles_item(), 8, 9, 10, 11);
                }                
            }

            if (sprite.y() >= GROUND_LEVEL) {
                sprite.set_y(GROUND_LEVEL);

                if (bounce && bn::abs(velocity_y) > MIN_BOUNCE_VELOCITY) {
                    velocity_y = -velocity_y * BOUNCE_FACTOR;
                } else {
                    velocity_y = 0;
                    is_on_ground = true;
                }
            }
        }

        bn::vector<bn::fixed_point, 9> getTiles()
        {
            int x0 = int(sprite.x()) / BLOCK_SIZE;
            int y0 = int(sprite.y()) / BLOCK_SIZE;

            bn::vector<bn::fixed_point, 9> tiles;

            for(int x = x0 -1 ; x<= x0+1; x++){
                for(int y = y0 -1 ; y<= y0+1; y++){
                    tiles.push_back(bn::fixed_point(x,y));
                }
            }

            return tiles;
        }
    };

    constexpr int MAX_BULLETS = 5;
    constexpr bn::fixed BULLET_SPEED = 2.0f;
    constexpr int MAX_BULLET_DISTANCE = 100;

    struct Bullet {
        bn::sprite_ptr sprite;
        bool active = false;
        bn::fixed start_x = 0.0f;
        bn::fixed velocity_x = 0.0f;
    };

    void handleBullets(bn::vector<Bullet, MAX_BULLETS>& bullets, Player& player){
        if (bn::keypad::pressed(bn::keypad::key_type::B)) {
            for (auto& bullet : bullets) {
                if (!bullet.active) {
                    bullet.active = true;
                    bullet.sprite.set_visible(true);
                    bullet.sprite.set_rotation_angle(player.lookingRight ? 270 : 90);
                    bullet.start_x = player.sprite.x();
                    bullet.sprite.set_position(player.sprite.x(), player.sprite.y());
                    bullet.velocity_x = player.lookingRight ? BULLET_SPEED : -BULLET_SPEED;
                    break;
                }
            }
        }

        for (auto& bullet : bullets) {
            if (bullet.active) {
                bullet.sprite.set_x(bullet.sprite.x() + bullet.velocity_x);
                if (bn::abs(bullet.sprite.x() - bullet.start_x) > MAX_BULLET_DISTANCE) {
                    bullet.active = false;
                    bullet.sprite.set_visible(false);
                }
            }
        }
    }

    bool collision(bn::sprite_ptr& objectA, bn::sprite_ptr& objectB){
        bn::fixed objectA_radius = (objectA.shape_size().width() / 2) * objectA.horizontal_scale();
        bn::fixed objectB_radius = (objectB.shape_size().width() / 2) * objectB.horizontal_scale();
        bn::fixed max_distance = (objectA_radius + objectB_radius) * (objectA_radius + objectB_radius);

        bn::fixed distanceX = objectB.x() - objectA.x();
        bn::fixed distanceY = objectB.y() - objectA.y();
        bn::fixed distance = distanceX * distanceX + distanceY * distanceY;

        return (distance < max_distance); 
    }

    constexpr bn::fixed HIT_POINT_SCALE = 0.2f;
    constexpr int MAX_ENEMIES = 2;
    constexpr int RESPAWN_TIMER = 100;
    constexpr int SPEED = 1;

    struct Enemy {
        bn::sprite_ptr sprite;
        bn::sprite_palette_ptr palette;
        int hit_points = 3;
        bn::sprite_animate_action<3> animate_action;

        Enemy(const bn::sprite_item& sprite_item, int x, int y, int frame_index) 
            : sprite(sprite_item.create_sprite(x, y)), 
            palette(sprite.palette()), 
            animate_action(bn::create_sprite_animate_action_forever(
                  sprite, 16, sprite_item.tiles_item(),
                  frame_index, frame_index + 1, frame_index + 2)) 
        {
            sprite.set_scale(HIT_POINT_SCALE * hit_points);
        }
    };

    void bulletHitDetection(bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::vector<Bullet, MAX_BULLETS>& bullets, int& framesBeforeRespawn){
        for (auto& bullet : bullets) {
            if (bullet.active) {
                for (int i = 0; i < enemies.size(); i++) {
                    auto& enemy = enemies[i];

                    if (collision(bullet.sprite,enemy.sprite)) {
                        enemy.hit_points--;
                        bullet.active = false;
                        bullet.sprite.set_visible(false);

                        if (enemy.hit_points == 0) {
                            enemies.erase(enemies.begin() + i);
                            i--;
                            framesBeforeRespawn=0;
                        } else {
                            enemy.sprite.set_scale(HIT_POINT_SCALE * enemy.hit_points);
                        }
                    }
                }
            }
        }
    }

    bool isColliding(Enemy& enemy, Player& player, bn::vector<Enemy, MAX_ENEMIES>& enemies) {
        if (collision(enemy.sprite, player.sprite)) {
            return true;
        }

        for (auto& otherenemy : enemies) {
            if (&enemy != &otherenemy && collision(enemy.sprite, otherenemy.sprite)) {
                return true;
            }
        }

        return false;
    }

    void respawnEnemies(bn::random& random, int& framesBeforeRespawn, Player& player, bn::vector<Enemy,MAX_ENEMIES>& enemies, bn::camera_ptr& camera){
        framesBeforeRespawn++;
        if (enemies.size() < MAX_ENEMIES && framesBeforeRespawn > RESPAWN_TIMER) {
            int frame_index = (random.get_int(2) % 2) * 3;
            BN_LOG(frame_index);
            Enemy new_enemy = {bn::sprite_items::monsters, 0, GROUND_LEVEL,frame_index};
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

            if (framesSinceLastHit > INVINCIBILITY_FRAMES && collision(enemy.sprite, player.sprite)){
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
    bn::blending::set_transparency_alpha(0.5);

    bn::camera_ptr camera = bn::camera_ptr::create(0, 0);
    
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

    Player player = {bn::sprite_items::ninja, 0, GROUND_LEVEL};

    bn::vector<Enemy,MAX_ENEMIES> enemies;
    enemies.push_back({bn::sprite_items::monsters,-100, GROUND_LEVEL,0});
    enemies.push_back({bn::sprite_items::monsters,100, GROUND_LEVEL,3});

    bn::unordered_map<bn::fixed_point, Block, MAX_BLOCKS> blocks;
    blocks.insert(bn::fixed_point(-1,1),{-1,1});
    blocks.insert(bn::fixed_point(1,1),{1,1});
    blocks.insert(bn::fixed_point(2,1),{2,1});
    blocks.insert(bn::fixed_point(3,1),{3,1});
    blocks.insert(bn::fixed_point(1,2),{1,2});
    blocks.insert(bn::fixed_point(2,2),{2,2});
    blocks.insert(bn::fixed_point(3,2),{3,2});
    blocks.insert(bn::fixed_point(5,-2),{5,-2});
    blocks.insert(bn::fixed_point(6,-2),{6,-2});

    int framesBeforeRespawn=0;
    int framesSinceLastHit=INVINCIBILITY_FRAMES;
    bn::random random;

    player.sprite.set_camera(camera);
    for(auto& enemy:enemies){
        enemy.sprite.set_camera(camera);
    }
    for(auto& [position,block]:blocks){
        block.sprite.set_camera(camera);
    }

    bool bounce = readSram();

    while (hearts.size()>0) {
        if(bn::keypad::start_pressed()){
            bounce = !bounce;
            writeSram(bounce);
        }

        player.move(bounce,camera,blocks);

        handleBullets(bullets,player);

        bulletHitDetection(enemies,bullets,framesBeforeRespawn);

        respawnEnemies(random,framesBeforeRespawn,player,enemies,camera);

        //moveEnemiesToPlayer(player,enemies,hearts,framesSinceLastHit);

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
