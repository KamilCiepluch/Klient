
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include  <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ncurses.h>
#include "game_logic.h"
#include "game_enums.h"



struct listener
{
    enum MoveOptions *move;
    enum keyboard_option option;
    sem_t *sem;
    unsigned long long *server_round;
};
void print_window(const struct server_read_data data, int number_of_cols, int number_of_rows)
{
    int x=5,y=5;

    start_color();
    init_pair(1,COLOR_RED,COLOR_RED);
    init_pair(2,COLOR_BLACK,COLOR_YELLOW);
    init_pair(3,COLOR_BLACK,COLOR_GREEN);
    init_pair(4,COLOR_BLACK,COLOR_MAGENTA);
    init_pair(5,COLOR_BLACK,COLOR_WHITE);
    for(int i=0; i<number_of_rows; i++)
    {
        for(int j=0; j<number_of_cols; j++)
        {
            if(data.map[i][j] == '|')
            {
                attron(COLOR_PAIR(1));
                mvprintw(x+i,y+j,"%c",data.map[i][j]);
                attroff(COLOR_PAIR(1));

            }
            else if(data.map[i][j] == 'c' || data.map[i][j] == 't' || data.map[i][j] == 'T' || data.map[i][j] == 'D')
            {
                attron(COLOR_PAIR(2));
                mvprintw(x+i,y+j,"%c",data.map[i][j]);
                attroff(COLOR_PAIR(2));

            }
            else if(data.map[i][j] == 'A' )
            {
                attron(COLOR_PAIR(3));
                mvprintw(x+i,y+j,"%c",data.map[i][j]);
                attroff(COLOR_PAIR(3));

            }
            else if(data.map[i][j] >= '1' && data.map[i][j] <= '9')
            {
                attron(COLOR_PAIR(4));
                mvprintw(x+i,y+j,"%c",data.map[i][j]);
                attroff(COLOR_PAIR(4));

            }
            else
            {
                attron(COLOR_PAIR(5));
                mvprintw(x+i,y+j,"%c",data.map[i][j]);
                attroff(COLOR_PAIR(5));
            }
        }
    }





}
void print_server_info(pid_t PID,int start_x, int start_y)
{
    start_color();
    init_pair(8,COLOR_BLACK,COLOR_WHITE);
    attron(COLOR_PAIR(8));
    for(int i=0; i<2; i++)
    {
        for(int j=-2; j<50; j++)
        {
            mvprintw(start_y+1+i,start_x+1+j," ");
        }
    }
    mvprintw(start_y+1,start_x+1,"Server's PID: %d", PID);
    mvprintw(start_y+2,start_x+2,"Campsite X/Y: unknown" );
    attroff(COLOR_PAIR(8));
}
void print_player_info(const struct server_read_data *player, int x, int y)
{

    start_color();
    init_pair(6,COLOR_BLACK,COLOR_WHITE);
    attron(COLOR_PAIR(6));
    if(player->connectionStatus == Connected)
    {

        mvprintw(y+1,x,"%d",getpid());
        mvprintw(y+2,x,"HUMAN");
        mvprintw(y+3,x,"%02d/%02d",player->coordinates.x,player->coordinates.y);
        mvprintw(y+4,x,"%d",player->deaths);

        mvprintw(y+8,x,"%d",player->carried_coins);
        mvprintw(y+9,x,"%d",player->brought_coins);
    }
    else
    {
        mvprintw(y+1,x,"-");
        mvprintw(y+2,x,"-");
        mvprintw(y+3,x,"--/--");
        mvprintw(y+4,x,"-");
    }
    attroff(COLOR_PAIR(6));
}
void print_stats(const struct server_read_data *players, unsigned long long round, int start_x, int start_y)
{
    start_color();
    init_pair(8,COLOR_BLACK,COLOR_WHITE);
    attron(COLOR_PAIR(8));
    for(int i=0; i<23; i++)
    {
        for(int j=-2; j<50; j++)
        {
            mvprintw(start_y+3+i,start_x+1+j," ");
        }
    }


    mvprintw(start_y+3,start_x+2,"Round number: %llu",round );


    mvprintw(start_y+6,start_x+1,"Parameter:");
    mvprintw(start_y+7,start_x+2,"PID");
    mvprintw(start_y+8,start_x+2,"Type");
    mvprintw(start_y+9,start_x+2,"CURR X/Y");
    mvprintw(start_y+10,start_x+2,"Deaths");

    mvprintw(start_y+13,start_x+1,"Coins");
    mvprintw(start_y+14,start_x+5,"carried");
    mvprintw(start_y+15,start_x+5,"brought");




    mvprintw(start_y+18,start_x+1,"Legend:");
    mvprintw(start_y+19,start_x+7,"- players");
    mvprintw(start_y+20,start_x+7,"- wall");
    mvprintw(start_y+21,start_x+7,"- bushes (slow down)");
    mvprintw(start_y+22,start_x+7,"- enemy");
    mvprintw(start_y+23,start_x+7,"- one coin");
    mvprintw(start_y+23,start_x+31,"- dropped treasure");
    mvprintw(start_y+24,start_x+7,"- treasure (10 coin)");
    mvprintw(start_y+25,start_x+7,"- large treasure (50 coin)");
    mvprintw(start_y+22,start_x+31,"- campsite ");
    attroff(COLOR_PAIR(8));

    init_pair(9,COLOR_WHITE,COLOR_MAGENTA);
    attron(COLOR_PAIR(9));
    mvprintw(start_y+19,start_x+2,"1234");
    attroff(COLOR_PAIR(9));

    init_pair(10,COLOR_BLACK,COLOR_BLACK);
    attron(COLOR_PAIR(10));
    mvprintw(start_y+20,start_x+2,"|");
    attroff(COLOR_PAIR(10));

    init_pair(11,COLOR_BLACK,COLOR_WHITE);
    attron(COLOR_PAIR(11));
    mvprintw(start_y+21,start_x+2,"#");
    mvprintw(start_y+22,start_x+2,"*");
    attroff(COLOR_PAIR(11));

    init_pair(12,COLOR_BLACK,COLOR_YELLOW);
    attron(COLOR_PAIR(12));
    mvprintw(start_y+23,start_x+2,"c");
    mvprintw(start_y+24,start_x+2,"t");
    mvprintw(start_y+25,start_x+2,"T");
    mvprintw(start_y+23,start_x+29, "D");
    attroff(COLOR_PAIR(12));

    init_pair(13,COLOR_YELLOW,COLOR_GREEN);
    attron(COLOR_PAIR(13));
    mvprintw(start_y+22,start_x+29,"A");
    attroff(COLOR_PAIR(13));
    print_player_info(players,start_x+14,start_y+6);
    move(0,0);
}

void *keyborad_listener(void *arg)
{
    struct listener *ptr = (struct listener*) arg;
    while (ptr->option!= QUIT_GAME)
    {
        int option = getch();
        if(option == 'q') {
            ptr->option = QUIT_GAME;
            break;
        }
        sem_wait(ptr->sem);
        switch (option) {
            case  'w':
            {
                *ptr->move = UP;
                *ptr->server_round = AFK_TIME;
                break;
            }
            case 's':
            {
                *ptr->move  = DOWN;
                *ptr->server_round= AFK_TIME;
                break;
            }
            case 'a':
            {
                *ptr->move  = LEFT;
                *ptr->server_round= AFK_TIME;
                break;
            }
            case 'd':
            {
                *ptr->move  = RIGHT;
                *ptr->server_round= AFK_TIME;
                break;
            }
            default:
            {
                *ptr->move  = STAY;
                *ptr->server_round;
            }
        }
        sem_post(ptr->sem);

    }
    return NULL;
}
int main()
{


    int shm = shm_open("players_list", O_CREAT  | O_RDWR,0777);
    if(ftruncate(shm, sizeof(struct shared_memory_names))!=0) // przypisanie pamięci
    {
        printf("\nServer is not responding\n");
        return -1;
    }

    sem_t *sem= sem_open("my_sem", O_CREAT, 0777, 1);
    sem_wait(sem);
    struct shared_memory_names *names =mmap(NULL, sizeof(struct shared_memory_names), PROT_READ | PROT_WRITE, MAP_SHARED,shm,0);
    sem_post(sem);



    char *sem_name =NULL;
    char *shm_read=NULL;
    char *shm_write=NULL;
    int playerNumber = -1;
    for(int i=0; i<4; i++)
    {
        if(names->slots[i].status==NotConnected)
        {
            sem_name= names->slots[i].sem_name;
            shm_read = names->slots[i].read_data;
            shm_write = names->slots[i].write_data;
            names->slots[i].status = Connected;
            playerNumber= i;
            break;
        }
    }

    if(sem_name==NULL || shm_read==NULL || shm_write==NULL)
    {
        printf("\nCan not connected to server\n");
        return 0;
    }



    sem_t *psem =  sem_open(sem_name, O_CREAT, 0777, 1);
    int shm_r = shm_open(shm_read, O_CREAT  | O_RDWR,0777);
    if(ftruncate(shm_r, sizeof(const struct server_read_data))!=0) // przypisanie pamięci
    {
        printf("truncate failed");
        return -1;
    }

    const struct server_read_data *read_info =mmap(NULL, sizeof(const struct server_read_data ), PROT_READ | PROT_WRITE, MAP_SHARED,shm_r,0);



    int shm_w = shm_open(shm_write, O_CREAT  | O_RDWR,0777);
    if(ftruncate(shm_w, sizeof( struct server_write_data))!=0) // przypisanie pamięci
    {
        printf("truncate failed");
        return -1;
    }

     struct server_write_data *write_info =mmap(NULL, sizeof(struct server_write_data), PROT_READ | PROT_WRITE, MAP_SHARED,shm_w,0);

    sem_wait(sem);
        write_info->move =STAY;
        write_info->playerPID = getpid();
        write_info->status = WaitingForData;
    sem_post(sem);

  //  //todo poprawic
    initscr();
    cbreak();
    noecho();

    int HEIGHT = 80;
    int WIDTH = 120;
    WINDOW  *game_window  = newwin(HEIGHT,WIDTH,0,0);
    keypad(game_window,true);
    box(game_window, 0, 0);



    pthread_t listener;
    struct listener data;
    data.move = &write_info->move;
    data.option = DO_NOTHING;
    data.sem = psem;
    data.server_round = &write_info->rounds_to_exit;

    pthread_create(&listener,NULL,keyborad_listener, (void *) &data);
    unsigned long long current_round = read_info->round_number -1;

    write_info->rounds_to_exit = AFK_TIME;
    while (data.option !=QUIT_GAME)
    {

        if(read_info->connectionStatus == ServerQuit)   break;
        if(write_info->rounds_to_exit == 0)
        {
            printf("Player kicked from the server\n");
            break;
        }

        if(current_round!= read_info->round_number)
        {
            sem_wait(psem);
            current_round = read_info->round_number;
            print_window(*read_info, 5, 5);
            print_server_info(read_info->serverPID,15, 2);
            print_stats(read_info,read_info->round_number,15, 2);
            refresh();
            sem_post(psem);
        }



    }

    sem_wait(sem);
    write_info->status = NotConnected;
    write_info->playerPID = NotConnected;
    names->slots[playerNumber].status = NotConnected;
    sem_post(sem);


    endwin();
    printf("\n Client process finished\n");
    return 0;
}