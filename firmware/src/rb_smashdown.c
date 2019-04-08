#include "rb_game.h"
#include "rb_printf.h"

#define ARENA_MIN_X (0)
#define ARENA_MIN_Y (0)
#define ARENA_MAX_X (31)
#define ARENA_MAX_Y (31)

#define MAX_GOAL (7)
#define MAX_SCORE (7)

#define BALL_SPEED_MAX (6000)
#define BALL_SPEED_MIN (1500)

typedef enum gamestate
{
    STATE_INIT,
    STATE_TITLE,
    STATE_START,
    STATE_PLAY,
    STATE_SCORE,
    STATE_GAMEOVER
} rb_gamestate_t;

typedef struct player
{
    uint8_t score;
    rb_color_t color;
    uint8_t active;
} rb_player_t;

typedef struct ball
{
    rb_coord_t x, y;
    rb_coord_t vx, vy;
} rb_ball_t;

typedef struct paddle
{
    int16_t pos;
    uint8_t width;
    rb_point_t start, end;
    uint8_t goal;
} rb_paddle_t;

static rb_gamestate_t gamestate;
static rb_player_t player[4];
static rb_ball_t ball[4];
static rb_paddle_t paddle[4];

static uint8_t active_players = 2;
static int32_t sw_prev_pos[4];
static int32_t rtc_base = 0;

static uint32_t gameloop_speed = 0;
static uint32_t ball_speed = BALL_SPEED_MAX;

uint8_t rb_point_on_line(rb_point_t p, rb_point_t a, rb_point_t b)
{
    int8_t tester;
    // ensure line is non-zero
    if ((a.x + a.y + b.x + b.y) == 0)
        return FALSE;

    // ensure points are collinear
    tester = (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y);
    //printf("%d\r\n", tester);
    if (tester)
        return FALSE;
    
    //if ((b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y))
    //    return FALSE;

    // check if x-coordinates are equal
    if ((a.x - b.x) == 0)
    {
        //ensure p.y is between a.y & b.y
        if (a.y < b.y)
        {
            if (p.y <= b.y && p.y >= a.y)
                return TRUE;
        }
        else
        {
            if (p.y >= b.y && p.y <= a.y)
                return TRUE;
        }
    }
    else
    {
        // ensure p.x is between a.x & b.x
        if (a.x < b.x)
        {
            if (p.x <= b.x && p.x >= a.x)
                return TRUE;
        }
        else
        {
            if (p.x >= b.x && p.x <= a.x)
                return TRUE;
        }
    }
    return FALSE;
}

int16_t rb_get_random_value(int16_t min, int16_t max)
{
    if (min > max)
    {
        int16_t tmp = max;
        max = min;
        min = tmp;
    }

    return (rand()%(abs(max - min) + 1) + min);
}

void rb_game_ball_reset(uint8_t i)
{
    ball[i].x = rb_get_random_value(10, 22);
    ball[i].y = rb_get_random_value(10, 22);
    ball[i].vx = rb_get_random_value(-1, 1);
    ball[i].vy = rb_get_random_value(-1, 1);
    //ball[i].x = ((int32_t)rand() % 10) + 12;
    //ball[i].y = ((int32_t)rand() % 10) + 12;
    //ball[i].vx = ((int32_t)rand() % 5) - 2;
    //ball[i].vy = ((int32_t)rand() % 5) - 2;
    if (ball[i].vx == 0 && ball[i].vy == 0)
    {
        ball[i].vx = -1;
        ball[i].vy = 1;
    }
}

void rb_game_balls_init(void)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        rb_game_ball_reset(i);
    }
}

void rb_game_paddle_set(uint8_t i, int16_t p, uint8_t w)
{
    paddle[i].pos = p;
    paddle[i].width = w;

    if (w == 0)
    {
        paddle[i].start.x = 0;
        paddle[i].start.y = 0;
        paddle[i].end.x = 0;
        paddle[i].end.y = 0;
        return;
    }

    switch (i)
    {
    //rb_point_t a0 = {0, 15};    //(0, 0)
    //rb_point_t b0 = {0, 15};    //(0, 31)
    case 0:
        paddle[0].start.x = 0;
        paddle[0].start.y = (16 - (paddle[i].width / 2)) + paddle[0].pos;
        paddle[0].end.x = 0;
        paddle[0].end.y = (15 + (paddle[i].width / 2)) + paddle[0].pos;
        break;

    //rb_point_t a1 = {31, 12};   //(31, 0)
    //rb_point_t b1 = {31, 17};   //(31, 31)
    case 1:
        paddle[1].start.x = 31;
        paddle[1].start.y = (16 - (paddle[i].width / 2)) + paddle[1].pos;
        paddle[1].end.x = 31;
        paddle[1].end.y = (15 + (paddle[i].width / 2)) + paddle[1].pos;
        break;

    //rb_point_t a2 = {13, 0};    //(0, 0)
    //rb_point_t b2 = {18, 0};    //(31, 0)
    case 2:
        paddle[2].start.x = (16 - (paddle[i].width / 2)) + paddle[2].pos;
        paddle[2].start.y = 0;
        paddle[2].end.x = (15 + (paddle[i].width / 2)) + paddle[2].pos;
        paddle[2].end.y = 0;
        break;

    //rb_point_t a3 = {13, 31};   //(0, 31)
    //rb_point_t b3 = {18, 31};   //(31, 31)
    case 3:
        paddle[3].start.x = (16 - (paddle[i].width / 2)) + paddle[3].pos;
        paddle[3].start.y = 31;
        paddle[3].end.x = (15 + (paddle[i].width / 2)) + paddle[3].pos;
        paddle[3].end.y = 31;
        break;
    }
}

uint8_t rb_game_paddles_pressed(void)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (rb_encoder_get_switch(i) == 1)
        {
            return 1;
        }
    }

    return 0;
}

uint8_t rb_game_player_get_active_count()
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        if (player[i].active == TRUE)
            count++;
    }

    return count;
}

void rb_game_player_set_active(uint8_t i, uint8_t status)
{
    if (status == FALSE)
    {
        rb_game_paddle_set(i, 0, 32);
        rb_encoder_set_color(i, COLOR_BLACK);
        player[i].active = FALSE;
        paddle[i].goal = 0;
    }
    else
    {
        rb_game_paddle_set(i, 0, 6);
        rb_encoder_set_color(i, player[i].color);
        player[i].active = TRUE;
    }
}

void rb_game_players_reset()
{
    rb_game_player_set_active(0, FALSE);
    rb_game_player_set_active(1, FALSE);
    rb_game_player_set_active(2, FALSE);
    rb_game_player_set_active(3, FALSE);
}

void rb_game_draw_goal(void)
{
    rb_point_t p0 = {22, 24};

    rb_matrix_set_rotation(2);
    rb_draw_uint16(p0, &rb_font_dejavu_10, paddle[0].goal, player[0].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(4);
    rb_draw_uint16(p0, &rb_font_dejavu_10, paddle[1].goal, player[1].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(3);
    rb_draw_uint16(p0, &rb_font_dejavu_10, paddle[2].goal, player[2].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(1);
    rb_draw_uint16(p0, &rb_font_dejavu_10, paddle[3].goal, player[3].color);
    rb_matrix_set_rotation(0);
}

void rb_game_draw_score(void)
{
    rb_point_t p0 = {22, 24};

    rb_matrix_set_rotation(2);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[0].score, player[0].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(4);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[1].score, player[1].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(3);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[2].score, player[2].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(1);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[3].score, player[3].color);
    rb_matrix_set_rotation(0);
}

void rb_menustate_init(void)
{
    srand(rb_rtc_get_seconds());

    player[0].color = COLOR_RED;
    player[1].color = COLOR_GREEN;
    player[2].color = COLOR_BLUE;
    player[3].color = COLOR_YELLOW;

    gamestate = STATE_TITLE;
}

void rb_menustate_title(void)
{
    rb_point_t p0 = {1, 0};
    rb_point_t p1 = {9, 3};
    rb_point_t p2 = {20, 12};

    rb_game_players_reset();

    rb_game_paddle_set(0, 0, 0);
    rb_game_paddle_set(1, 0, 0);
    rb_game_paddle_set(2, 0, 0);
    rb_game_paddle_set(3, 0, 0);

    rb_encoder_set_color(0, player[0].color);
    rb_encoder_set_color(1, player[1].color);
    rb_encoder_set_color(2, player[2].color);
    rb_encoder_set_color(3, player[3].color);

    uint8_t w0[] = {"smash"};
    rb_draw_string(p0, &rb_font_dejavu_10, w0, COLOR_GREEN);

    uint8_t w1[] = {"down"};
    rb_draw_string(p1, &rb_font_dejavu_10, w1, COLOR_BLUE);

    rb_draw_uint16(p2, &rb_font_dejavu_10, active_players, COLOR_WHITE);

    //printf("sw-0: %d\r\n", rb_encoder_get_position(0));

    if (rb_game_paddles_pressed() == 1)
    {
        rb_game_balls_init();
        rtc_base = rb_rtc_get_seconds();
        gamestate = STATE_START;
    }
}

void rb_menustate_start()
{
    int32_t rtc_now;
    int32_t rtc_count;

    rtc_now = rb_rtc_get_seconds();
    rtc_count = rtc_now - rtc_base;

    switch (active_players)
    {
    case 1:
        rb_game_player_set_active(0, TRUE);
        rb_game_player_set_active(1, FALSE);
        rb_game_player_set_active(2, FALSE);
        rb_game_player_set_active(3, FALSE);
        break;

    case 2:
        rb_game_player_set_active(0, TRUE);
        rb_game_player_set_active(1, TRUE);
        rb_game_player_set_active(2, FALSE);
        rb_game_player_set_active(3, FALSE);
        break;

    case 3:
        rb_game_player_set_active(0, TRUE);
        rb_game_player_set_active(1, TRUE);
        rb_game_player_set_active(2, TRUE);
        rb_game_player_set_active(3, FALSE);
        break;

    case 4:
        rb_game_player_set_active(0, TRUE);
        rb_game_player_set_active(1, TRUE);
        rb_game_player_set_active(2, TRUE);
        rb_game_player_set_active(3, TRUE);
        break;
    }

    rb_matrix_clear();

    for (uint8_t i = 0; i < 4; i++)
    {
        rb_draw_line(paddle[i].start, paddle[i].end, player[i].color);
    }

    if (rtc_count < 30)
    {
        rb_point_t p0 = {5, 2};
        uint8_t w0[] = {"ready"};
        rb_draw_string(p0, &rb_font_dejavu_10, w0, COLOR_WHITE);
    }

    if (rtc_count >= 30 && rtc_count <= 60)
    {
        rb_point_t p0 = {5, 8};
        uint8_t w0[] = {"set"};
        rb_draw_string(p0, &rb_font_dejavu_10, w0, COLOR_WHITE);
    }

    if (rtc_count >= 60 && rtc_count <= 90)
    {
        rb_point_t p0 = {5, 10};
        uint8_t w0[] = {"go"};
        rb_draw_string(p0, &rb_font_dejavu_10, w0, COLOR_WHITE);
    }

    if (rtc_count > 90)
    {
        gamestate = STATE_PLAY;
    }
}

void rb_menustate_play()
{
    if (active_players > 1)
    {
        if (rb_game_player_get_active_count() == 1)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                if (player[i].active == TRUE)
                player[i].score++;
            }
            gamestate = STATE_SCORE;  
        }
    }
    else
    {
        if (rb_game_player_get_active_count() == 0)
        {
            gamestate = STATE_GAMEOVER;
        }
    }
}

void rb_menustate_score()
{
    rb_point_t p0 = {22, 24};
    rb_point_t p1 = {7, 3};

    uint8_t w0[] = {"score"};

    rb_matrix_clear();

    rb_matrix_set_rotation(2);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[0].score, player[0].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(4);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[1].score, player[1].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(3);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[2].score, player[2].color);
    rb_matrix_set_rotation(0);

    rb_matrix_set_rotation(1);
    rb_draw_uint16(p0, &rb_font_dejavu_10, player[3].score, player[3].color);
    rb_matrix_set_rotation(0);

    if (paddle[0].goal > 0)
    {
        rb_matrix_set_rotation(2);
        rb_draw_string(p1, &rb_font_dejavu_10, w0, player[0].color);
        rb_matrix_set_rotation(0);
    }
    else if (paddle[1].goal > 0)
    {
        rb_matrix_set_rotation(4);
        rb_draw_string(p1, &rb_font_dejavu_10, w0, player[1].color);
        rb_matrix_set_rotation(0);
    }
    else if (paddle[2].goal > 0)
    {
        rb_matrix_set_rotation(3);
        rb_draw_string(p1, &rb_font_dejavu_10, w0, player[2].color);
        rb_matrix_set_rotation(0);
    }
    else if (paddle[3].goal > 0)
    {
        rb_matrix_set_rotation(1);
        rb_draw_string(p1, &rb_font_dejavu_10, w0, player[3].color);
        rb_matrix_set_rotation(0);
    }

    if (rb_game_paddles_pressed() == 1)
    {
        gamestate = STATE_GAMEOVER;
    }
}

void rb_menustate_gameover()
{
    rb_point_t p1 = {7, 3};
    rb_point_t p2 = {16, 3};

    uint8_t w0[] = {"game"};
    uint8_t w1[] = {"over"};

    rb_matrix_clear();

    rb_matrix_set_rotation(2);
    rb_draw_string(p1, &rb_font_dejavu_10, w0, player[0].color);
    rb_draw_string(p2, &rb_font_dejavu_10, w1, player[0].color);
    rb_matrix_set_rotation(0);

    if (rb_game_paddles_pressed() == 1)
    {
        gamestate = STATE_TITLE;
    }
}

void rb_gameloop_players_update(void)
{
    int32_t sw_pos[4];

    for (uint8_t i = 0; i < 4; i++)
    {
        sw_pos[i] = rb_encoder_get_position(i) / 4;

        if (sw_prev_pos[i] < sw_pos[i])
        {
            sw_prev_pos[i] = sw_pos[i];
            if (active_players < 4)
                active_players++;
        }
        if (sw_prev_pos[i] > sw_pos[i])
        {
            sw_prev_pos[i] = sw_pos[i];
            if (active_players > 1)
                active_players--;
        }
    }
}

void rb_gameloop_paddles_update(void)
{
    int32_t sw_pos[4];

    for (uint8_t i = 0; i < 4; i++)
    {
        if (player[i].active == TRUE)
        {
            sw_pos[i] = rb_encoder_get_position(i) / 4;

            if (sw_prev_pos[i] < sw_pos[i])
            {
                //printf("sw-0: %d\r\n", paddle[i].pos);
                sw_prev_pos[i] = sw_pos[i];
                if (paddle[i].pos < 13)
                    rb_game_paddle_set(i, paddle[i].pos + 2, paddle[i].width);
            }
            if (sw_prev_pos[i] > sw_pos[i])
            {
                sw_prev_pos[i] = sw_pos[i];
                if (paddle[i].pos > -13)
                    rb_game_paddle_set(i, paddle[i].pos - 2, paddle[i].width);
            }
        }
    }
}

void rb_gameloop_update(void)
{
    rb_matrix_clear();

    for (uint8_t i = 0; i < 4; i++)
    {
        rb_point_t ball_pt;
        ball_pt.x = ball[i].x;
        ball_pt.y = ball[i].y;

        rb_draw_point(ball_pt, player[i].color);
        rb_draw_line(paddle[i].start, paddle[i].end, player[i].color);

        if (ball[i].x <= ARENA_MIN_X)
        {
            if (rb_point_on_line(ball_pt, paddle[0].start, paddle[0].end) == TRUE)
            {
                if (ball[i].vy == 0)
                    ball[i].vy = ((int32_t)rand() % 3) - 1;

                ball[i].vx = -ball[i].vx;
            }
            else
            {
                if (player[0].active == TRUE)
                    paddle[0].goal++;
                rb_game_ball_reset(i);
            }
        }

        if (ball[i].x >= ARENA_MAX_X)
        {
            if (rb_point_on_line(ball_pt, paddle[1].start, paddle[1].end) == TRUE)
            {
                if (ball[i].vy == 0)
                    ball[i].vy = ((int32_t)rand() % 3) - 1;

                ball[i].vx = -ball[i].vx;
            }
            else
            {
                if (player[1].active == TRUE)
                    paddle[1].goal++;
                rb_game_ball_reset(i);
            }
        }

        if (ball[i].y <= ARENA_MIN_Y)
        {
            if (rb_point_on_line(ball_pt, paddle[2].start, paddle[2].end) == TRUE)
            {
                if (ball[i].vx == 0)
                    ball[i].vx = ((int32_t)rand() % 3) - 1;

                ball[i].vy = -ball[i].vy;
            }
            else
            {
                if (player[2].active == TRUE)
                    paddle[2].goal++;
                rb_game_ball_reset(i);
            }
        }

        if (ball[i].y >= ARENA_MAX_Y)
        {
            if (rb_point_on_line(ball_pt, paddle[3].start, paddle[3].end) == TRUE)
            {
                if (ball[i].vx == 0)
                    ball[i].vx = ((int32_t)rand() % 3) - 1;

                ball[i].vy = -ball[i].vy;
            }
            else
            {
                if (player[3].active == TRUE)
                    paddle[3].goal++;
                rb_game_ball_reset(i);
            }
        }

        ball[i].x = ball[i].x + ball[i].vx;
        ball[i].y = ball[i].y + ball[i].vy;

        if (paddle[i].goal >= MAX_GOAL)
            rb_game_player_set_active(i, FALSE);
    }
}

void rb_gameloop(void)
{
    if (gameloop_speed > ball_speed)
    {
        switch (gamestate)
        {
        case STATE_INIT:
            rb_menustate_init();
            break;

        case STATE_TITLE:
            rb_gameloop_players_update();
            rb_gameloop_update();
            rb_menustate_title();
            break;

        case STATE_START:
            rb_menustate_start();
            break;

        case STATE_PLAY:
            rb_gameloop_paddles_update();
            rb_gameloop_update();
            rb_menustate_play();
            //rb_game_draw_goal();
            if (ball_speed > BALL_SPEED_MIN)
                ball_speed--;
            break;

        case STATE_SCORE:
            rb_menustate_score();
            break;

        case STATE_GAMEOVER:
            rb_menustate_gameover();
            ball_speed = BALL_SPEED_MAX;
            break;
        }
        gameloop_speed = 0;
    }
    gameloop_speed++;
}