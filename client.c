#Tic-Tac-Toe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <sys/socket.h> //socket specific saari files
#include <arpa/inet.h> //for ip address family
#include <unistd.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#define windowsize 600
#define cellsize (windowsize / 3) //each small box 200 size ka hai
#define PORT 8080

char board[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
int count = 0;
int status = 0;
bool meri = false;//Client-O,server-X

int checkingforwin(){
    int combis[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8}, {0,4,8},{2,4,6}};//all combis for winning
    for (int i=0;i<8;i++){
        if (board[combis[i][0]] == board[combis[i][1]] && board[combis[i][1]] == board[combis[i][2]] && board[combis[i][0]] != ' ') {
            return (board[combis[i][0]] == 'X') ? 1 : 2;
        }
    }
    return (count == 9) ? 3 : 0;
}

void drawboard(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    for (int i=1;i<3;i++) {
        SDL_RenderDrawLine(renderer,i*cellsize,0,(i*cellsize),windowsize);
        SDL_RenderDrawLine(renderer, 0, (i*cellsize), windowsize,(i*cellsize));
    }

    for (int i=0;i<9;i++) {
        int row=i/3, col=i%3;
        int x= (col*cellsize )+(cellsize/2);
        int y= (row*cellsize)+(cellsize/2);

        if (board[i]=='X') {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawLine(renderer, x-40, y-40, x+40, y+40);
            SDL_RenderDrawLine(renderer, x+40, y-40, x-40, y+40);
        } else if (board[i]=='O') {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            for (int angle = 0; angle <= 360; angle++) {
                float x1 = x + 40 * cos(angle * M_PI / 180.0);
                float y1 = y + 40 * sin(angle * M_PI / 180.0);
                SDL_RenderDrawPoint(renderer, (int)x1, (int)y1);
            }
        }
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "//IP address", &serv_addr.sin_addr); 

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    printf("Connected to server.\n");

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Tic Tac Toe - Client (O)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowsize,windowsize ,0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event event;

    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type==SDL_QUIT) {
                running= false;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && meri && status == 0) {
                int x =event.button.x;
                int y =event.button.y;
                int row = y/cellsize, col=x/ cellsize;
                int index = row * 3 + col;

                if (board[index]==' ') {
                    board[index]='O';
                    count++;
                    status=checkingforwin();
                    meri=false;

                    char msg[2]={index+ '0','\0' };
                    send(sock, msg, sizeof(msg), 0);
                }
            }
        }

        if (!meri && status == 0) {
            char buffer[2];
            int val=recv(sock, buffer, sizeof(buffer), MSG_DONTWAIT);
            if (val>0) {
                int index=buffer[0] - '0';
                if (board[index]==' ') {
                    board[index]= 'X';
                    count++;
                    status=checkingforwin();
                    meri=true;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);
        drawboard(renderer);
        SDL_RenderPresent(renderer);
    }

    close(sock);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
