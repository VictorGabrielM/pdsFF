#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define NAVIGATION 0
#define BATTLE 1
#define SIZE_OBJ 60
#define NUM_ENEMIES 20
#define RAIO_DETECTION 25
#define WIN 1
#define LOST 1

int game_setup = NAVIGATION;
int playing = 1;
int special_count = 0; 
int special = 0;
int game_over = 0;
int winner = 0;
int score = 0;
int block = 0; //block escape
int enemy_escape = 0;
float mat_enemy_escape[NUM_ENEMIES][3];
int tecla_naveg = 0;
int new_high_score = 0;
int tam_pow = 20;

typedef struct color
{
    int r, g, b;
} color;

typedef struct hero
{
    int tl_x, tl_y;
    int br_x, br_y;
    int size;
    ALLEGRO_COLOR color;
    int life;
    int walk;
} hero;

typedef struct enemy
{
    float x, y;
    int life;
} enemy;

typedef struct battle_selector
{
    int tl_x, tl_y, br_x, br_y;
    ALLEGRO_COLOR color;
} battle_selector;

const float FPS = 20;

const int SCREEN_W = 960;
const int SCREEN_H = 540;
int arrive_objective(hero h)
{
    if (h.tl_x >= SCREEN_W - SIZE_OBJ && h.br_y <= SIZE_OBJ)
    {
        return 1;
    }
    return 0;
}

void make_navigation_scenery(ALLEGRO_BITMAP *scenery_nav)
{
    al_draw_bitmap(scenery_nav,0,0,0);
}

void init_hero(hero *h)
{
    (*h).size = 30;
    (*h).walk = 30;
    (*h).tl_y = SCREEN_H - (*h).size;
    (*h).tl_x = 0;
    (*h).br_x = (*h).size;
    (*h).br_y = SCREEN_H;
    (*h).color = al_map_rgb(255, 0, 0);
    (*h).life = 200;
}

void create_hero_naveg(hero h)
{
    al_draw_filled_rectangle(h.tl_x, h.tl_y, h.br_x, h.br_y, h.color);
    if(tecla_naveg == 1)// A esquerda
    {
        al_draw_filled_triangle(h.tl_x,h.tl_y,h.tl_x,h.br_y,h.tl_x - h.size /2,h.tl_y+h.size/2,al_map_rgb(0,0,0));
    }else if(tecla_naveg == 2)
    {
        al_draw_filled_triangle(h.br_x,h.tl_y,h.br_x,h.br_y,h.br_x + h.size/2,h.tl_y+h.size/2,al_map_rgb(0,0,0));
    }else if(tecla_naveg == 3)
    {
        al_draw_filled_triangle(h.tl_x,h.tl_y,h.br_x,h.tl_y,h.tl_x+h.size/2,h.tl_y-h.size/2,al_map_rgb(0,0,0));
    }else if(tecla_naveg == 4)
    {
        al_draw_filled_triangle(h.tl_x,h.br_y,h.br_x,h.br_y,h.tl_x + h.size/2,h.br_y + h.size/2,al_map_rgb(0,0,0));
    }    
}

int ultrapassatela(hero h, char k)
{
    switch (k)
    {

    case 'A':
        if (h.tl_x - h.walk < 0)
            return 1;
        break;

    case 'D':
        if (h.br_x + h.walk > SCREEN_W)
            return 1;
        break;

    case 'W':
        if (h.tl_y - h.walk < 0)
            return 1;
        break;

    case 'S':
        if (h.br_y + h.walk > SCREEN_H)
            return 1;
        break;
    }
    return 0;
}

void movimentation_mechanical(hero *h, int k)
{
    if (k == ALLEGRO_KEY_A && !ultrapassatela(*h, 'A'))
    {
        (*h).tl_x -= (*h).walk;
        (*h).br_x -= (*h).walk;
        tecla_naveg = 1;
    }
    else if (k == ALLEGRO_KEY_D && !ultrapassatela(*h, 'D'))
    {
        (*h).tl_x += (*h).walk;
        (*h).br_x += (*h).walk;
        tecla_naveg = 2;
    }
    else if (k == ALLEGRO_KEY_W && !ultrapassatela(*h, 'W'))
    {
        (*h).tl_y -= (*h).walk;
        (*h).br_y -= (*h).walk;
        tecla_naveg = 3;
    }
    else if (k == ALLEGRO_KEY_S && !ultrapassatela(*h, 'S'))
    {
        (*h).tl_y += (*h).walk;
        (*h).br_y += (*h).walk;
        tecla_naveg = 4;
    }
}

float dist_hero_enemies(float ch_x, float ch_y, float mat_enemies[NUM_ENEMIES][3])
{
    int i = 0, enemymp = 0;
    float smaller_dist = sqrt(pow(SCREEN_H, 2) + pow(SCREEN_W, 2));
    float vet_dists[NUM_ENEMIES];
    for (i = 0; i < NUM_ENEMIES; i++)
    { // X of enemy                //Y of enemy
        vet_dists[i] = sqrt(pow(mat_enemies[i][0] - ch_x, 2) + pow(mat_enemies[i][1] - ch_y, 2));
        if (vet_dists[i] < smaller_dist)
        {
            smaller_dist = vet_dists[i];
            enemymp = i;
        }
    }
    return smaller_dist;
}

int what_enemy_mp(hero h, float mat_enemies[NUM_ENEMIES][3])
{
    int i = 0, enemymp = 0;
    float ch_x = h.tl_x + h.size / 2.0;
    float ch_y = h.tl_y + h.size / 2.0;
    float smaller_dist = sqrt(pow(SCREEN_H, 2) + pow(SCREEN_W, 2));
    float vet_dists[NUM_ENEMIES];
    for (i = 0; i < NUM_ENEMIES; i++)
    { // X of enemy                //Y of enemy
        vet_dists[i] = sqrt(pow(mat_enemies[i][0] - ch_x, 2) + pow(mat_enemies[i][1] - ch_y, 2));
        if (vet_dists[i] < smaller_dist)
        {
            smaller_dist = vet_dists[i];
            enemymp = i;
        }
    }
    return enemymp;
}

int enemy_detected(hero h, float mat_enemies[NUM_ENEMIES][3])
{
    float centerh_x = h.tl_x + h.size / 2.0;
    float centerh_y = h.tl_y + h.size / 2.0;
    if (dist_hero_enemies(centerh_x, centerh_y, mat_enemies) <= RAIO_DETECTION)
    {
        return 1;
    }
    else
        return 0;
}

void print_matrix(float M[NUM_ENEMIES][3])
{
    int i = 0, j = 0;
    for (i = 0; i < NUM_ENEMIES; i++)
    {
        for (j = 0; j < 3; j++)
        {
            printf("%.1f ", M[i][j]);
        }
        printf("\n");
    }
}

void make_battle_scenery(ALLEGRO_BITMAP *background_battle,battle_selector bs)
{   
    al_draw_bitmap(background_battle,0,0,0);
    /*al_clear_to_color(al_map_rgb(255, 255, 255));
    al_draw_bitmap(monster, 700, 200, 0);                                   //draw enemy
    al_draw_bitmap(hero_sprite, 50, 252, 0);*/                                //draw hero
    al_draw_filled_rectangle(bs.tl_x, bs.tl_y, bs.br_x, bs.br_y, bs.color); //draw battle selector
}

void make_attack_selector(battle_selector *bs, int k)
{
    al_draw_filled_rectangle((*bs).tl_x, (*bs).tl_y, (*bs).br_x, (*bs).br_y, (*bs).color); //Make rectangle for selection of atack
    if (k == ALLEGRO_KEY_S && (*bs).tl_y != 135)
    { //Moviment battle selector for down
        (*bs).tl_y += 35;
        (*bs).br_y += 35;
    }
    else if (k == ALLEGRO_KEY_W && (*bs).tl_y != 30)
    { //Moviment battle selector for up
        (*bs).tl_y -= 35;
        (*bs).br_y -= 35;
    }
}

//number of mp //tecla //coordinates of bs
void hero_attack_mechanical(hero *h, float M[NUM_ENEMIES][3], int n, int k, battle_selector *bs, int *animation)
{
    int rand_damage = 0;
    int role_of_escape = 0;
    if (k == ALLEGRO_KEY_ENTER && (*bs).tl_y == 30 && *animation == 0)
    {
        M[n][2] -= 50; //standard attack
        *animation = 1;
        special_count++;
        score += 50;
        block = 0;
    }
    else if (k == ALLEGRO_KEY_ENTER && (*bs).tl_y == 65 && *animation == 0)
    {
        rand_damage = rand() % 90;
        M[n][2] -= rand_damage; //random attack
        *animation = 1;
        special_count++;
        score += rand_damage;
        block = 0;
    }
    else if (k == ALLEGRO_KEY_ENTER && (*bs).tl_y == 100 && special_count >= 3 && *animation == 0)
    {
        M[n][2] -= 100; //special attack
        *animation = 1;
        special = 1;
        special_count = 0;
        block = 0;

        score += 100; //para fazer sistema de pontuação
    }
    else if (k == ALLEGRO_KEY_ENTER && (*bs).tl_y == 135 && *animation == 0)
    {
        role_of_escape = 1 + rand() % 100; //probability of escape // probability: 40 per cent
        block++;
        if (role_of_escape <= 40 && block == 1 && (*h).tl_x < SCREEN_W - RAIO_DETECTION - 10 - (*h).size) //se o monstro esta a esquerda
        {
            game_setup = NAVIGATION;
            (*h).tl_x += RAIO_DETECTION + 10;
            (*h).br_x += RAIO_DETECTION + 10;
            mat_enemy_escape[enemy_escape][0] = M[n][0];
            mat_enemy_escape[enemy_escape][1] = M[n][1];
            enemy_escape++;
        }
        else if (role_of_escape <= 40 && block == 1 && (*h).tl_x > RAIO_DETECTION + 10) //se o monstro esta a direita
        {
            game_setup = NAVIGATION;
            (*h).tl_x -= RAIO_DETECTION + 10;
            (*h).br_x -= RAIO_DETECTION + 10;
            mat_enemy_escape[enemy_escape][0] = M[n][0];
            mat_enemy_escape[enemy_escape][1] = M[n][1];
            enemy_escape++;
        }
    }
}

void fill_mat_escape()
{
    int i=0, j=0;
    for(i=0;i<NUM_ENEMIES;i++)
    {
        for(j=0;j<3;j++)
        {
            mat_enemy_escape[i][j] = -400;
        }
    }
}

void enemy_attack_mechanical(hero *h,float mat_enemies[NUM_ENEMIES][3], int mp)
{
    if(mat_enemies[mp][2] < 130 )
    {
    (*h).life -= 20 + rand() % 10;
    }else if(mat_enemies[mp][2] >= 130 && mat_enemies[mp][2] < 170)
    {
     (*h).life -= 35 + rand() % 10;   
    }else if(mat_enemies[mp][2] >= 170)
    {
      (*h).life -= 45 + rand() % 10;  
    }
}

void init_battle_selector(battle_selector *bs)
{
    (*bs).tl_x = 30;
    (*bs).tl_y = 30;
    (*bs).br_x = 60;
    (*bs).br_y = 60;
    (*bs).color = al_map_rgb(0, 0, 255);
}

void show_life_display(hero h, float M[NUM_ENEMIES][3], int mp, ALLEGRO_FONT *arial)
{
    char life_hero[5];
    al_draw_filled_rectangle(40, 500, 40 + h.life, 520, al_map_rgb(255, 0, 0)); //life of hero
    sprintf(life_hero, "%d", h.life);
    al_draw_text(arial, al_map_rgb(0, 0, 0), 45, 505, 0, life_hero);

    char life_enemy[5];
    al_draw_filled_rectangle(700, 500, 700 + M[mp][2], 520, al_map_rgb(255, 0, 0)); //life of enemy
    sprintf(life_enemy, "%.0f", M[mp][2]);
    al_draw_text(arial, al_map_rgb(0, 0, 0), 705, 505, 0, life_enemy);
}

void draw_screen_go(ALLEGRO_FONT *size32)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(size32,al_map_rgb(255,255,255),SCREEN_W/3 + 70,SCREEN_H/2-50,0,"GAME OVER");
}

void draw_screen_win(ALLEGRO_FONT *size32,int high_score)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(size32,al_map_rgb(0,0,255),SCREEN_W/3 + 70,SCREEN_H/2-50,0,"VOCÊ GANHOU");
    al_draw_text(size32,al_map_rgb(255,255,255),SCREEN_W/3,SCREEN_H/2,0,"Score");
    char sre[30];
    sprintf(sre,"%d",score);
    al_draw_text(size32,al_map_rgb(255,255,255),SCREEN_W/3 + 100,SCREEN_H/2,0,sre);

    if(score > high_score)
    {
        new_high_score = 1;
        al_draw_text(size32,al_map_rgb(0,0,255),SCREEN_W/3 + 170,SCREEN_H/2,0,"NOVO RECORDE");    
    }else if(new_high_score)
    {
        al_draw_text(size32,al_map_rgb(0,0,255),SCREEN_W/3 + 170,SCREEN_H/2,0,"NOVO RECORDE");
    }
    al_draw_text(size32,al_map_rgb(255,255,255),SCREEN_W/3,SCREEN_H/2+50,0,"pressione enter para sair");    
}

void show_score(int scr, ALLEGRO_FONT *size32)
{
    char sc[30];
    sprintf(sc, "%d", scr);
    al_draw_text(size32, al_map_rgb(0, 0, 0), 25, 25, 0, sc);
}

void show_enemies()
{
    int i = 0;
    for (i = 0; i < NUM_ENEMIES; i++)
    {
        al_draw_filled_circle(mat_enemy_escape[i][0], mat_enemy_escape[i][1], 5, al_map_rgb(0, 0, 255));
    }
}

int main(int argc, char **argv)
{

    ALLEGRO_DISPLAY *display = NULL;         //display
    ALLEGRO_EVENT_QUEUE *event_queue = NULL; //fila de eventos
    ALLEGRO_TIMER *timer = NULL;             //timer do allegro5

    //----------------------- rotinas de inicializacao ---------------------------------------

    //inicializa o Allegro
    if (!al_init())
    {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    //inicializa o módulo de primitivas do Allegro
    if (!al_init_primitives_addon())
    {
        fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }

    //inicializa o modulo que permite carregar imagens no jogo
    if (!al_init_image_addon())
    {
        fprintf(stderr, "failed to initialize image module!\n");
        return -1;
    }

    //cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if (!timer)
    {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    //cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display)
    {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    //instala o teclado
    if (!al_install_keyboard())
    {
        fprintf(stderr, "failed to install keyboard!\n");
        return -1;
    }

    //inicializa o modulo allegro que carrega as fontes
    al_init_font_addon();

    //inicializa o modulo allegro que entende arquivos tff de fontes
    if (!al_init_ttf_addon())
    {
        fprintf(stderr, "failed to load tff font module!\n");
        return -1;
    }

    //carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);
    if (size_32 == NULL)
    {
        fprintf(stderr, "font file does not exist or cannot be accessed!\n");
    }

    ALLEGRO_FONT *size_14 = al_load_font("arial.ttf", 14, 1);
    if (size_14 == NULL)
    {
        fprintf(stderr, "font file does not exist or cannot be accessed!\n");
    }

    //cria a fila de eventos
    event_queue = al_create_event_queue();
    if (!event_queue)
    {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        return -1;
    }

    //registra na fila os eventos de tela (ex: clicar no X na janela)
    al_register_event_source(event_queue, al_get_display_event_source(display));
    //registra na fila os eventos de tempo: quando o tempo altera de t para t+1
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    //registra na fila os eventos de teclado (ex: pressionar uma tecla)
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    //inicia o temporizador
    al_start_timer(timer);
    int i = 0, j = 0, mp = 0, a = 0, power = 0;
    int animation = 0;
    int bola_x = 100, bola_dx = 20;
    int bola_y = SCREEN_H / 2, bola_dy = 0;
    int redraw = 1;
    int high_score = 0;
    srand(time(NULL));
    ALLEGRO_BITMAP *monster = al_load_bitmap("bowser.png");
    ALLEGRO_BITMAP *hero_sprite = al_load_bitmap("luigi.png");
    ALLEGRO_BITMAP *background_battle = al_load_bitmap("background_battle.png");
    ALLEGRO_BITMAP *scenery_nav = al_load_bitmap("background_navigation.png");
    
    FILE *hs_r = fopen("high_score.txt","r");
    fscanf(hs_r,"%d",&high_score);
    fclose(hs_r);

    hero hero;
    init_hero(&hero);

    battle_selector bs;
    init_battle_selector(&bs); //inicialize battle selector

    float mat_enemies[NUM_ENEMIES][3]; // Matrix of enemies
    //make random matrix of enemies
    for (i = 0; i < NUM_ENEMIES; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (j == 0)
            {
                mat_enemies[i][j] = (float)(rand() % SCREEN_W); //X   of enemy
            }
            if (j == 1)
            {
                mat_enemies[i][j] = (float)(rand() % SCREEN_H); //Y of enemy
            }
            if (j == 2)
            {
                mat_enemies[i][j] = (float)(101 + rand() % 100); //initial life enemy
            }                                                    //entre 101 e 200
        }
    }
    print_matrix(mat_enemies);
    fill_mat_escape();
    while (playing)
    {
        ALLEGRO_EVENT ev;
        //espera por um evento e o armazena na variavel de evento ev
        al_wait_for_event(event_queue, &ev);

        //se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            if (game_setup == NAVIGATION)
            {
                make_navigation_scenery(scenery_nav);
                create_hero_naveg(hero);
                show_score(score, size_32);
                if (arrive_objective(hero))
                {
                    winner = WIN;
                    draw_screen_win(size_32, high_score); //draw screen win
                }
                if (enemy_detected(hero, mat_enemies))
                {
                    game_setup = BATTLE;
                }
                if(enemy_escape && !winner) //show enemies when escape
                {
                    show_enemies();
                }
            }

            else // EVENT TIMER:GAME SETUP == BATTLE
            {
                mp = what_enemy_mp(hero, mat_enemies);
                make_battle_scenery(background_battle, bs);
                show_life_display(hero, mat_enemies, mp, size_14);
                show_score(score, size_32);
                if (animation == 1 && bola_x <= SCREEN_W - 120 && power == 0) //BATLLE animation
                {
                    bola_x += bola_dx;
                    bola_y += bola_dy;
                    redraw = 1;
                }
                else if (animation == 1 && bola_x >= 100 && power == 1)
                {
                    bola_x -= bola_dx;
                    bola_y -= bola_dy;
                    redraw = 1;
                }
                else if (game_over)
                {
                    draw_screen_go(size_32); //draw screen game over
                }
            }
            al_flip_display();
        }
        if (redraw && al_is_event_queue_empty(event_queue))
        {
            //nao preciso redesenhar
            redraw = 0;
            //limpo a tela
            al_clear_to_color(al_map_rgb(255, 255, 255));
            //desenho o passaro na nova posicao
            if (special)
            {
                tam_pow = 50;
                //al_draw_filled_circle(bola_x, bola_y, 70, al_map_rgb(255, 0, 0));
            }
            al_draw_bitmap(background_battle,0,0,0);
            al_draw_filled_circle(bola_x, bola_y, tam_pow, al_map_rgb(255, 0, 0)); //power
            /*al_draw_bitmap(monster, 700, 200, 0);                             //draw enemy
            al_draw_bitmap(hero_sprite, 50, 252, 0);*/
            al_draw_filled_rectangle(bs.tl_x, bs.tl_y, bs.br_x, bs.br_y, bs.color); //battle selector
            show_life_display(hero, mat_enemies, mp, size_14);
            //dou um refresh na tela
            al_flip_display();
            if (bola_x == SCREEN_W - 100) //860
            {
                power = 1;
                special = 0;
                tam_pow = 20;
                if (hero.life > 0 && mat_enemies[mp][2] <= 0)
                {
                    game_setup = NAVIGATION;
                    mat_enemies[mp][0] = -500;
                    mat_enemies[mp][1] = -500;
                    animation = 0;
                    bola_x = 100;
                    power = 0;
                }
            }
            else if (bola_x == 80) //onde se aplica o dano do inimigo
            {
                animation = 0;
                power = 0;
                bola_x = 100;
                enemy_attack_mechanical(&hero,mat_enemies,mp);
                if (hero.life <= 0 && mat_enemies[mp][2] >= 0)
                {
                    game_over = 1;
                }
            }
        }

        //se o tipo de evento for o fechamento da tela (clique no x da janela)
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            playing = 0;
        }
        //se o tipo de evento for um pressionar de uma tecla
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (game_setup == NAVIGATION)
            {
                movimentation_mechanical(&hero, ev.keyboard.keycode);
                if (winner && ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    playing = 0;
                }
            }
            else
            {
                mp = what_enemy_mp(hero, mat_enemies);
                make_attack_selector(&bs, ev.keyboard.keycode);
                hero_attack_mechanical(&hero, mat_enemies, mp, ev.keyboard.keycode, &bs, &animation);
                if (game_over && ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    playing = 0;
                }
            }
        }
    } //fim do while
    printf(" %d \n", score);
    if(score > high_score)
    {
    FILE *hs_w = fopen("high_score.txt","w");
    fprintf(hs_w,"%d",score);
    fclose(hs_w);
    }
    //procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}