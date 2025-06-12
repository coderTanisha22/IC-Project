#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<sys/select.h>

#define PORT 8080 
#define cells 8
#define cell_size 80
#define window_size (cells * cell_size)

enum { khali, RED, BLACK, RedK, BlackK }; //zero one intialize

int board[cells][cells];
bool redTurn = true;
int sock = -1;
bool quit = false;
//circle banana, jidhar circle hai udhar fill hojaaha
void drawCirc(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}
//board banana overall
void drawBoard(SDL_Renderer* renderer) {
    for (int row = 0; row < cells; row++) {
        for (int col = 0; col < cells; col++) {
            SDL_Rect cell = { col * cell_size, row * cell_size, cell_size, cell_size };
            if ((row + col) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);
            else
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
            SDL_RenderFillRect(renderer, &cell);

            int piece = board[row][col];
            if (piece != khali) {
                int cx = (col * cell_size) + cell_size / 2;
                int cy = (row * cell_size) + cell_size / 2;
                int radius = cell_size / 2 - 10;

                if (piece == RED || piece == RedK)
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                drawCirc(renderer, cx, cy, radius);

                if (piece == RedK || piece == BlackK) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    drawCirc(renderer, cx, cy, radius / 2); //agar king hoga to draw karna
                }
            }
        }
    }
}
//board pe pieces daalna
void initialboard() {
    for (int i = 0; i < cells; i++) {
        for (int j = 0; j < cells; j++) {
            if ((i + j) % 2 == 1) {
                if (i < 3) board[i][j] = BLACK;
                else if (i > 4) board[i][j] = RED;
                else board[i][j] = khali;
            } else {
                board[i][j] = khali;
            }
        }
    }
}
//move valid hai check karne ke liye
bool isValidMove(int sr, int sc, int dr, int dc, bool isRed) {
    if (dr < 0 || dr >= cells || dc < 0 || dc >= cells) {
        return false;
    }
    if (board[sr][sc] == khali || board[dr][dc] != khali) {
        return false;
    }

    int piece = board[sr][sc];
    int direction = isRed ? -1 : 1;

    //diagonal move ke liye
    if ((piece == RED || piece == BLACK) && dr - sr == direction && abs(dc - sc) == 1) {
        return true;
    }

    //agar king hoga aage peeche done move hoga
    if ((piece == RedK || piece == BlackK) && abs(dr - sr) == 1 && abs(dc - sc) == 1) {
        return true;
    }
    //captur ke liye
    if (abs(dr - sr) == 2 && abs(dc - sc) == 2) {
        int midrow = (sr + dr) / 2;
        int midcol = (sc + dc) / 2;
        int midPiece = board[midrow][midcol];
        if (isRed && (midPiece == BLACK || midPiece == BlackK))
            {
                return true;
            }
        if (!isRed && (midPiece == RED || midPiece == RedK))
            {
                return true;
            }
    }

    return false;
}
//move karne ke liye
void makeMove(int sr, int sc, int dr, int dc) {
    int piece = board[sr][sc];
    board[dr][dc] = piece;
    board[sr][sc] = khali;

    // Promotion ke liye
    if (piece == RED && dr == 0)
        {
            board[dr][dc] = RedK;
        }
    if (piece == BLACK && dr == 7)
        {
            board[dr][dc] = BlackK;
        }
//capture move ke liye
    if (abs(dr - sr) == 2 && abs(dc - sc) == 2) {
        int midrow = (sr + dr) / 2;
        int midcol = (sc + dc) / 2;
        board[midrow][midcol] = khali;
    }
    
    redTurn = !redTurn;
}
//move send ke liye
void sendMove(int sr, int sc, int dr, int dc, int sock) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d %d %d %d", sr, sc, dr, dc);
    send(sock, buffer, strlen(buffer), 0);
}
//check for incoming move block kare bina
bool noblock(int* sr, int* sc, int* dr, int* dc, int sock) {
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

    if (activity > 0 && FD_ISSET(sock, &readfds)) {
        char buffer[20] = {0};
        int readval = read(sock, buffer, sizeof(buffer));
        if (readval > 0) {
            sscanf(buffer, "%d %d %d %d", sr, sc, dr, dc);
            return true;
        }
    }
    return false;
}

int main() {
    struct sockaddr_in serv_addr; //server address store ke liye
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET; //ipv4 ke liye
    serv_addr.sin_port = htons(PORT);//netwrok byte

    if (inet_pton(AF_INET, "//IP address", &serv_addr.sin_addr) <= 0) { // ip check karna
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Checkers Client - Red", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_size, window_size, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initialboard();

    int selectedRow = -1, selectedCol = -1;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;

            if (event.type == SDL_MOUSEBUTTONDOWN && redTurn) {
                int x = event.button.x;
                int y = event.button.y;
                int row = y / cell_size;
                int col = x / cell_size;

                int piece = board[row][col];

                if (selectedRow == -1) {
                    if ((piece == RED || piece == RedK)) {
                        selectedRow = row;
                        selectedCol = col;
                    }
                } 
                else {
                    if (isValidMove(selectedRow, selectedCol, row, col, true)) {
                        makeMove(selectedRow, selectedCol, row, col);
                        sendMove(selectedRow, selectedCol, row, col, sock);
                        redTurn = false;
                    }
                    selectedRow = -1;
                }
            }
        }

        if (!redTurn) {
            int sr, sc, dr, dc;
            if (noblock(&sr, &sc, &dr, &dc, sock)) {
                makeMove(sr, sc, dr, dc);
                redTurn = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawBoard(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    close(sock);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}