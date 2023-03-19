#include "snake2.hpp"
#include "assets.hpp"

using namespace blit;

Surface* background = nullptr;
Surface* sr_title = nullptr;

#define START_LEVEL 0 // 0 = first level
#define MAX_LEVEL 9

#define BLOCK 4

#ifdef PICO_BOARD
#define SCREEN_WIDTH 124 / BLOCK
#define SCREEN_HEIGHT 124 / BLOCK
#else
#define SCREEN_WIDTH 164 / BLOCK //41
#define SCREEN_HEIGHT 124 / BLOCK //31
#endif

#define MAX_APPLE 8
#define BONUS 250



// game.status:
// 0 = game
// 1 = start screen
// 2 = h.o.f. screen
// 3 = game over
// 4 = enter name

struct GAME
{
    short status;
    short timer;
    short level;
    short speed;
    int level_timer;
    short field[SCREEN_WIDTH][SCREEN_HEIGHT];
    int score;
    short apple;
    short bg;
    short menu;
};

struct PLAYER
{
    short life;
    int score;
    int bonus;
    short status;
    short head_x;
    short head_y;
    short tail_x;
    short tail_y;
    short direction;
    short grow;
    int alpha;
};

struct APPLE
{
    short x;
    short y;
};

struct HOF
{
    short rank = 8;
    short pos;
    bool cursor;
};

struct BEST
{
    int highscore[8];
    char name[8][11];
};


GAME game;
PLAYER p;
PLAYER s;
APPLE a[10];
HOF hof;
BEST best;

Timer title;
Timer timer;
Timer cursor;
Timer bonus_snd;
Tween level_exit;


void title_update(Timer &t)
{
    if (game.status == 3 && hof.rank < 8)
    {
        game.status = 4;
        title.stop();
    }     
    else if(game.status == 2)
    {
        game.status = 1;
        title.start();
    }
    else
    {
        game.status = 2;
        title.start();
    }
}

void cursor_update(Timer &t)
{
    hof.cursor? hof.cursor = false: hof.cursor = true;
}

void bonus_snd_update(Timer &t)
{
    channels[3].trigger_attack();
}

void start_position(short x, short y, short z)
{
    p.head_x = x;
    p.head_y = y;
    p.direction = z;
    p.tail_x = x;
    p.tail_y = y;
    game.field[x][y] = z;
    p.grow = 15;

    s.head_x = (SCREEN_WIDTH - 1) - x;
    s.head_y = (SCREEN_HEIGHT - 1) - y;
    s.direction = z + 2;
    if (s.direction > 4)
        s.direction -= 4;
    s.tail_x = s.head_x;
    s.tail_y = s.head_y;
    game.field[s.head_x][s.head_y] = s.direction;
    s.grow = 15;
}

void new_apple(short apples)
{
    short a_x;
    short a_y;

    for (short i = 0; i < apples; i++)
    {
        do
        {
            a_x = std::rand() %(SCREEN_WIDTH - 2) + 1;
            a_y = std::rand() %(SCREEN_HEIGHT - 2) + 1;
        }
        while (game.field[a_x][a_y] != 0);

        for (short n = 0; n < 10; n++)
        {
            if (a[n].x == 0 && a[n].y == 0)
            {
                a[n].x = a_x;
                a[n].y = a_y;
                game.field[a_x][a_y] = 11;
                game.apple++;
                break;
            }
        }
    }

    //reset timer
    game.level_timer = (SCREEN_WIDTH - 1) * BLOCK;
}

void del_apple(short x, short y)
{
    for (short n = 0; n < 10; n++)
    {
        if (a[n].x == x && a[n].y == y)
        {
            a[n].x = 0;
            a[n].y = 0;
            game.apple--;
            if (game.apple == 0)
            {
                game.field[SCREEN_WIDTH / 2][0] = 15;
                channels[1].frequency = 1400;
            }
            else
            {
                channels[1].frequency = 1200;
            }
            channels[1].trigger_attack();
            break;
        }
    }
}

void timer_update(Timer &t)
{
    game.level_timer--;
    if (game.level_timer < 0)
        new_apple(4);
}

void score(int point)
{
    p.score += point;
    p.bonus += point;
    if (p.bonus >= BONUS)
    {
        bonus_snd.start();
        p.bonus -= BONUS;
        p.life < 6? p.life++: p.score += 100;
    }
}

void wall(short x1, short y1, short x2, short y2)
{
    for (short y = y1; y <= y2; y++)
    {
        for (short x = x1; x <= x2; x++)
        {
            game.field[x][y] = 20;
        }
    }
}

void new_level()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            (x == 0 || x == SCREEN_WIDTH - 1|| y == 0 || y == SCREEN_HEIGHT - 1)? game.field[x][y] = 20: game.field[x][y] = 0;
        }
    }

    switch(game.level)
    {
        case 0:
            start_position((SCREEN_WIDTH - 1) / 2, SCREEN_HEIGHT - 2, 1);
            break;
        case 1:
            wall(10, (SCREEN_HEIGHT - 1) / 2, SCREEN_WIDTH - 11, (SCREEN_HEIGHT - 1) / 2);
            start_position(1, SCREEN_HEIGHT - 8, 2);
            break;
        case 2:
            wall(9, 6, 9, SCREEN_HEIGHT - 7);
            wall(SCREEN_WIDTH - 10, 6, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 7);
            start_position((SCREEN_WIDTH - 1) / 2, SCREEN_HEIGHT - 2, 1);
            break;
        case 3:
            wall(1, (SCREEN_HEIGHT - 1) / 2, (SCREEN_WIDTH / 2) - 3, (SCREEN_HEIGHT - 1) / 2);
            wall((SCREEN_WIDTH / 2) + 3, (SCREEN_HEIGHT - 1) / 2, SCREEN_WIDTH - 1, (SCREEN_HEIGHT - 1) / 2);
            start_position((SCREEN_WIDTH - 1) / 2, SCREEN_HEIGHT - 2, 1);
            break;
        case 4:
            wall(9, 7, SCREEN_WIDTH / 2, 7);
            wall(SCREEN_WIDTH / 2, 9, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10);
            wall(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 8, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 8);
            start_position(1, (SCREEN_HEIGHT - 1) / 2, 2);
            break;
        case 5:
            wall(SCREEN_WIDTH / 3, 1, SCREEN_WIDTH / 3, SCREEN_HEIGHT - (SCREEN_HEIGHT / 3));
            wall(SCREEN_WIDTH - (SCREEN_WIDTH / 3) - 1, SCREEN_HEIGHT / 3, SCREEN_WIDTH - (SCREEN_WIDTH / 3) - 1, SCREEN_HEIGHT - 1);
            start_position((SCREEN_WIDTH - 1) / 2, SCREEN_HEIGHT - 2, 1);
            break;
        case 6:
            wall(7, (SCREEN_HEIGHT - 1) / 2 - 1, 11, (SCREEN_HEIGHT - 1) / 2 + 1);
            wall((SCREEN_WIDTH - 1) / 2 - 1, 6, (SCREEN_WIDTH - 1) / 2 + 1, 10);
            wall((SCREEN_WIDTH - 1) / 2 - 1, SCREEN_HEIGHT - 11, (SCREEN_WIDTH - 1) / 2 + 1, SCREEN_HEIGHT - 7);
            wall(SCREEN_WIDTH - 12, (SCREEN_HEIGHT - 1) / 2 - 1, SCREEN_WIDTH - 8, (SCREEN_HEIGHT - 1) / 2 + 1);
            start_position((SCREEN_WIDTH - 1) / 3 * 2, SCREEN_HEIGHT - 2, 1);
            break;
        case 7:
            wall(7, 7, SCREEN_WIDTH - 15, 7);
            wall(SCREEN_WIDTH - 8, 7, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 15);
            wall(14, SCREEN_HEIGHT - 8, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8);
            wall(7, 14, 7, SCREEN_HEIGHT - 8);
            start_position(10, SCREEN_HEIGHT - 2, 1);
            break;
        case 8:
            for (short i = 0; i < 24; i+=6)
            {
                for (short t = 0; t < 24; t+=6)
                {
                    game.field[i + ((SCREEN_WIDTH - 1) / 2 - 9)][t + 6] = 20;
                }
            }
            start_position((SCREEN_WIDTH - 1) / 2, SCREEN_HEIGHT - 2, 1);
            break;
        case 9:
            for (short i = 6; i < 30; i+=6)
            {
                wall(8, i, SCREEN_WIDTH - 9, i);
            }
            start_position(1, (SCREEN_HEIGHT - 1) / 2, 2);
            break;
    }

    p.status = 0;
    p.alpha = 255;
    hof.pos = 0;
    hof.rank = 8;


    //reset apple
    for (short i = 0; i < 10; i++)
    {
        a[i].x = 0;
        a[i].y = 0;
    }
    game.apple = 0;
    new_apple(8);

    game.status = 0;

    timer.start();
}

void cpu_control()
{
    short direction[5] = {-1,0,1,0,-1};

    short s_dir[3];
    s_dir[0] = s.direction; // strait ahead
    s_dir[1] = s.direction + 1; // turn cw
    if (s_dir[1] > 4) 
        s_dir[1] = 1;
    s_dir[2] = s.direction - 1; // turn ccw
    if (s_dir[2] < 1)
        s_dir[2] = 4;

    short s_steps[3]; 
    for (short i = 0; i < 3; i++)
    {
        short new_sx = s.head_x + direction[s_dir[i]];
        short new_sy = s.head_y + direction[s_dir[i] - 1];
        if (game.field[new_sx][new_sy] > 0) // next step is an apple or an obstacle
        {
            game.field[new_sx][new_sy] == 11? s_steps[i] = 0: s_steps[i] = 100;
        }
        else if (game.apple > 0) // next apple in array
        {
            for (short n = 0; n < 10; n++)
            {
                if (a[n].x > 0)
                {
                    s_steps[i] = abs(a[n].x - new_sx) + abs(a[n].y - new_sy); // steps to next apple
                    break;
                }
            }
        }
        else // no apple, no obstacle
        {
            s_steps[i] = 1; 
        }
    }
    if (s_steps[1] == s_steps[2] && s_steps[1] < s_steps[0]) // turn cw or ccw
        std::rand() %2 == 0? s.direction = s_dir[1]: s.direction = s_dir[2];
    else if (s_steps[1] < s_steps[0]) // turn cw
        s.direction = s_dir[1];
    else if (s_steps[2] < s_steps[0]) // turn ccw
        s.direction = s_dir[2];
    else
        s.direction = s_dir[0]; // turn strait
}

// init()
void init() 
{
    set_screen_mode(ScreenMode::lores);

    background = Surface::load(asset_background);
    sr_title = Surface::load(asset_image);

        if (!read_save(best))
        {
            for (short i=0; i<8; i++)
            {
                best.name[i][0] = 0;
                best.highscore[i] = 0;
            }
        }

// Highscore Tabelle löschen
/*
        best.highscore[i] = 0;
        best.name[i][0] = 0;
        write_save(best.highscore[i]);
        write_save(best.name[i], 10);
*/


    channels[0].waveforms = Waveform::SINE;
    channels[0].volume = 3000;
    channels[0].frequency = 500;
    channels[0].attack_ms   = 10;
    channels[0].decay_ms = 10;
    channels[0].sustain = 10;
    channels[0].release_ms = 5;

    channels[1].waveforms = Waveform::SINE; // Eat apple
    channels[1].volume = 2000;
    channels[1].attack_ms = 5;
    channels[1].decay_ms = 120;
    channels[1].sustain = 10;
    channels[1].release_ms = 5;

    channels[2].waveforms = Waveform::SQUARE; // Snake Collision
    channels[2].volume = 2000;
    channels[2].frequency = 200;
    channels[2].attack_ms = 5;
    channels[2].decay_ms = 500;
    channels[2].sustain = 5;
    channels[2].release_ms = 5;

    channels[3].waveforms = Waveform::SINE; //Bonus
    channels[3].volume = 3000;
    channels[3].frequency = 1500;
    channels[3].attack_ms = 10;
    channels[3].decay_ms = 50;
    channels[3].sustain = 50;
    channels[3].release_ms = 10; 

    title.init(title_update, 4000, 1);
    title.start();
    timer.init(timer_update, 20000 / ((SCREEN_WIDTH - 1) * BLOCK), -1);
    cursor.init(cursor_update, 200, -1);
    cursor.start();
    bonus_snd.init(bonus_snd_update, 120, 3);

    level_exit.init(tween_sine, 0, 255, 500, -1);
    level_exit.start();

    game.status = 1;

// Test
//    game.status = 3;
    
}

// render(time)
void render(uint32_t time) 
{
    screen.blit(background, Rect(0, 0, (SCREEN_WIDTH - 1) * BLOCK, (SCREEN_HEIGHT - 1) * BLOCK), Point(0, 0)); // Background

    if (game.status == 1) // Title
    {
        screen.blit(sr_title, Rect(14, 18, 100, 59), Point(SCREEN_WIDTH / 2 * BLOCK - 50, 30));
        int color = level_exit.value;
        screen.pen = Pen(255, color, color);  
        screen.text("PRESS A TO START", minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK, 110), true, TextAlign::center_center);
    }

    else if (game.status == 2) // Hall of fame
    {
        screen.pen = Pen(150, 50, 150);
        screen.rectangle(Rect(SCREEN_WIDTH / 2 * BLOCK - 43, 15, 85, 9));
        screen.pen = Pen(255, 255, 255);
        screen.text("HALL OF FAME", minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK, 20), true, TextAlign::center_center);
        for (short i=0; i<8; i++)
        {
            hof.rank == i? screen.pen = Pen(0, 255, 0): screen.pen = Pen(255, 255, 255);
            screen.text(std::string_view(best.name[i], 10), minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK - 40, 32 + (i * 8)), true, TextAlign::top_left);
            screen.text(std::to_string(best.highscore[i]), minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK + 40, 32 + (i * 8)), true, TextAlign::top_right);
        }
        int color = level_exit.value;
        screen.pen = Pen(255, color, color);  
        screen.text("PRESS A TO START", minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK, 110), true, TextAlign::center_center);
    }

    else if (game.status == 4)
    {
        short x = 0;
        short y = 0;

        screen.pen = Pen(255, 255, 255);
        screen.text("enter your name", minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK, 24), true, TextAlign::center_center);
        screen.text(std::string_view(best.name[hof.rank], hof.pos + 1), minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK - 26, SCREEN_HEIGHT * BLOCK - 32));
        for (short i = 0; i < 30; i++)
        {
            screen.pen = Pen(255, 255, 255);
            int x1 = (SCREEN_WIDTH / 2 * BLOCK) - 42 + (x * 11);
            int y1 = 40 + (y * 10);
            if (i == game.menu)
            {
                short size = 7;
                if (i > 27)
                    size = 18;
                screen.rectangle(Rect(x1 - 1, y1, size, 7));
                screen.pen = Pen(0, 0, 0);
            }
            if (i == 26)
                screen.text(".", minimal_font, Point(x1, y1), true, TextAlign::top_left);
            else if (i == 27)
                screen.text(" ", minimal_font, Point(x1, y1), true, TextAlign::top_left);
            else if (i == 28)
            {
                screen.text("del", minimal_font, Point(x1, y1), true, TextAlign::top_left);
                x++;
            }
            else if (i == 29)
                screen.text("end", minimal_font, Point(x1, y1), true, TextAlign::top_left);
            else
            {
                char c = 65 + i;
                screen.text(std::string_view(&c, 1), minimal_font, Point(x1, y1), true, TextAlign::top_left);
            }
            x++;
            if (x == 8)
            {
                x = 0;
                y++;
            }
        }
    }

    else // game + game over
    {
        for (int y = 1; y < SCREEN_HEIGHT - 1; y++)
        {
            for (int x = 1; x < SCREEN_WIDTH - 1; x++)
            {
                switch(game.field[x][y])
                {
                    case 1: case 2: case 3: case 4: // Player Tail
                        screen.alpha = p.alpha;
                        screen.pen = Pen(0, 155, 0);
                        screen.rectangle(Rect(x * BLOCK - 2, y * BLOCK - 2, BLOCK, BLOCK));
                        screen.alpha = 255;
                        break;
                    case 5: case 6: case 7: case 8: // CPU Tail
                        screen.pen = Pen(255, 192, 0);
                        screen.rectangle(Rect(x * BLOCK - 2, y * BLOCK - 2, BLOCK, BLOCK));
                        break;
                    case 9: // Player Head
                        screen.alpha = p.alpha;
                        screen.pen = Pen(0, 255, 0);
                        screen.rectangle(Rect(p.head_x * BLOCK - 2, p.head_y * BLOCK - 2, BLOCK, BLOCK));
                        screen.alpha = 255;
                        break;
                    case 10: // CPU Head
                        screen.pen = Pen(255, 255, 64); //
                        screen.rectangle(Rect(s.head_x * BLOCK - 2, s.head_y * BLOCK - 2, BLOCK, BLOCK));
                        break;
                    case 11: // Apple
                        screen.pen = Pen(255, 0, 0);  
                        screen.rectangle(Rect(x * BLOCK - 1, y * BLOCK - 1, 2, 2));
                        break;
                    case 20: // Wall
                        screen.pen = Pen(128, 128, 128);
                        screen.rectangle(Rect(x * BLOCK - 2, y * BLOCK - 2, BLOCK, BLOCK));
                        break;
                }
            }
        }  

        if (game.apple == 0) // Exit
        {
            int color = level_exit.value;
            screen.pen = Pen(255, color, color);  
            screen.rectangle(Rect(SCREEN_WIDTH / 2 * BLOCK - 3, 0, 6, 2));
        }   

        screen.pen = Pen(150, 50, 150); // timer
        screen.rectangle(Rect(0, (SCREEN_HEIGHT - 1) * BLOCK - 2, game.level_timer, 2));;

        screen.pen = Pen(255, 255, 255); 
        screen.text(std::to_string(p.score), minimal_font, Point(4, SCREEN_HEIGHT * BLOCK - 11), true, TextAlign::top_left);

        for (short i = 0; i < p.life; i++)
        {
            screen.pen = Pen(96, 255, 0);
            screen.pixel(Point(4 + (i * 2), SCREEN_HEIGHT * BLOCK - 16));
            screen.pen = Pen(0, 205, 0);
            screen.line(Point(4 + (i * 2), SCREEN_HEIGHT * BLOCK - 15), Point(4 + (i * 2), SCREEN_HEIGHT * BLOCK - 13));
        }

        if (game.status == 3) // game over
        {
            screen.alpha = 128;
            screen.pen = Pen(0, 0, 0);
            if (hof.rank == 8)
            {
                screen.rectangle(Rect(SCREEN_WIDTH / 2 * BLOCK - 30, 55, 60, 9));
                screen.alpha = 255;
                screen.pen = Pen(0, 255, 0);
                screen.text("GAME OVER", minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK, SCREEN_HEIGHT / 2 * BLOCK), true, TextAlign::center_center);
            }
            else
            {
                screen.rectangle(Rect(SCREEN_WIDTH / 2 * BLOCK - 44, 50, 88, 18));
                screen.alpha = 255;
                screen.pen = Pen(0, 255, 0);
                screen.text("WELCOME TO THE\nHALL OF FAME", minimal_font, Point(SCREEN_WIDTH / 2 * BLOCK, SCREEN_HEIGHT / 2 * BLOCK), true, TextAlign::center_center);
            }
        }
    }
}

// update(time)
void update(uint32_t time) 
{
    if (game.status == 0)
    {

        game.timer--;
        if (game.timer < game.speed)
        {
            game.timer = 7;
            short direction[5] = {-1,0,1,0,-1};

            if (buttons & Button::DPAD_UP && p.direction != 3)
                p.direction = 1;
            else if (buttons & Button::DPAD_RIGHT && p.direction != 4)
                p.direction = 2;
            else if (buttons & Button::DPAD_DOWN && p.direction != 1)
                p.direction = 3;
            else if (buttons & Button::DPAD_LEFT && p.direction != 2)
                p.direction = 4;


            // player snake
            if (p.grow == 0)
            {
                short old_tail_dir = game.field[p.tail_x][p.tail_y];
                game.field[p.tail_x][p.tail_y] = 0;
                p.tail_x += direction[old_tail_dir];
                p.tail_y += direction[old_tail_dir - 1];
                if (p.status == 1)
                {
                    score(1);
                    channels[0].trigger_attack();
                    if (p.tail_x == p.head_x && p.tail_y == p.head_y)
                    {
                        game.level++;
                        if (game.level > MAX_LEVEL)
                        {
                            game.level = 0;
                            if (game.speed < 2)
                                game.speed++;
                        }
                        new_level();
                        p.status = 0;
                    }
                }
            }
            else
            {
                p.grow--;
            }

            if (p.status == 0)
            {  
                game.field[p.head_x][p.head_y] = p.direction;

                p.head_x += direction[p.direction];
                p.head_y += direction[p.direction - 1];
                
                if (game.field[p.head_x][p.head_y] > 0)
                {
                    if (game.field[p.head_x][p.head_y] == 11)
                    {
                        score(5);
                        del_apple(p.head_x, p.head_y);
                        p.grow += 10;
                    }
                    else if (game.apple == 0 && p.head_x == (SCREEN_WIDTH - 1) / 2 && p.head_y == 0)
                    {
                        timer.stop();
                        p.status = 1;
                    }  
                    else 
                    {
                        p.head_x -= direction[p.direction];
                        p.head_y -= direction[p.direction - 1];

                        channels[2].trigger_attack();                        
                        timer.stop();
                        p.status = 2;
                    }
                }
                game.field[p.head_x][p.head_y] = 9;
            }
            else if (p.status == 2)
            {
                p.alpha -= 15;
                if (p.alpha <= 0)
                {
                    p.alpha = 0;
                    p.life--;
                    if (p.life < 0)
                    {
                        for (short i=0; i<8; i++)
                        {
                            if (p.score > best.highscore[i])
                            {
                                hof.rank = i;
                                for (short t=7; t>i; t--)
                                {
                                    for (short k=0; k<11; k++)
                                    {
                                         best.name[t][k] = best.name[t - 1][k];
                                    }
                                    best.highscore[t] = best.highscore[t -1];
                                }
                                for (short t=0; t<11; t++)
                                    best.name[i][t] = 0;
                                break;
                            }    
                        }
                        game.status = 3;
                        title.start();
                    }
                    else
                    {
                        new_level();
                    }
                }
                else
                {
                    p.grow = 1;
                }
            }

            // cpu snake
            if (s.grow == 0) // new cpu tail
            {
                if (game.field[s.tail_x][s.tail_y] < 9 && game.field[s.tail_x][s.tail_y] > 4) // is there a tail?
                {
                    short old_tail_dir = game.field[s.tail_x][s.tail_y] - 4;
                    game.field[s.tail_x][s.tail_y] = 0;
                    s.tail_x += direction[old_tail_dir]; // set new tail
                    s.tail_y += direction[old_tail_dir - 1];
                }
            }
            else // cpu snake grow
            {
                s.grow--;
            }  

            cpu_control(); // new head direction

            game.field[s.head_x][s.head_y] = s.direction + 4; // store old head direction for the tail

            s.head_x += direction[s.direction];
            s.head_y += direction[s.direction - 1];

            if (game.field[s.head_x][s.head_y] > 0)
            {
                if (game.field[s.head_x][s.head_y] == 11) // eat apple
                {
                    del_apple(s.head_x, s.head_y);
                    s.grow += 10;
                }  
                else // obstacle -> stop moving
                {
                    s.head_x -= direction[s.direction];
                    s.head_y -= direction[s.direction - 1];
                }
            }

            game.field[s.head_x][s.head_y] = 10;
        }
    }

    else if (game.status == 1 || game.status == 2) 
    {
        if (buttons.pressed & Button::A)
        {
            title.stop();
            p.life = 3;
            p.score = 0;
            p.bonus = 0;
            game.level = START_LEVEL;
            game.speed = 0;
            new_level();
        }
    }

    else if (game.status == 4)
    {
        (hof.pos < 10 && hof.cursor)? best.name[hof.rank][hof.pos] = 223: best.name[hof.rank][hof.pos] = 0;

        if (buttons.pressed & Button::DPAD_UP && game.menu > 7)
            game.menu < 29? game.menu -= 8: game.menu = 22;
        else if (buttons.pressed & Button::DPAD_RIGHT && game.menu < 29)
            game.menu++;
        else if (buttons.pressed & Button::DPAD_DOWN && game.menu < 24)
        {
            if (game.menu == 21)
                game.menu = 28;
            else if (game.menu > 21)
                game.menu = 29;
            else
                game.menu += 8;
        }
        else if (buttons.pressed & Button::DPAD_LEFT && game.menu > 0)
            game.menu--;

        if (buttons.pressed & Button::A)
        {
            if (hof.pos < 10)
            {
                if (game.menu < 26)
                {
                    best.name[hof.rank][hof.pos] = 65 + game.menu; 
                    hof.pos++;
                }
                else if (game.menu == 26) // "."
                {
                    best.name[hof.rank][hof.pos] = 174;
                    hof.pos++;
                }
                else if (game.menu == 27) // " "
                {
                    best.name[hof.rank][hof.pos] = 0;
                    hof.pos++;
                }
            }

            if (game.menu == 28) // delete
            {
                if (hof.pos > 0)
                {
                    hof.pos--;
                    best.name[hof.rank][hof.pos] = 0;
                }
            }
            else if (game.menu == 29) // ende eingabe
            {
                for (short i = hof.pos; i < 11; i++)
                    best.name[hof.rank][i] = 0;
                best.highscore[hof.rank] = p.score;

                for (short i=0; i<8; i++)
                {
                    write_save(best);
                }

                game.status = 2;
                game.menu = 0;
                title.start();
            }
        }
    }
}
