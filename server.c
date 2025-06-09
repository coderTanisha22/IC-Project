#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include<sys/select.h>

#define PORT 8080
#define cells 8
#define cell_size 80
#define window_size (cells * cell_size)

enum { khali, RED, BLACK, RED_K, BLACK_K };

int board[cells][cells];
bool redTurn = true; // Red = client, Black = server used to define whose turn it is
int server_socket = -1, client_socket = -1;
bool quit = false;
bool isRed=true;
//function to draw circle
void drawCirc(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            //make a square but only printf if inside circle
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void drawBoard(SDL_Renderer* renderer) {
    for (int row = 0; row < cells; row++) {
        for (int col = 0; col < cells; col++) {
            SDL_Rect cell = { col * cell_size, row * cell_size, cell_size, cell_size };
            if ((row + col) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);
                //color for light sqaure
            else
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
                //color for dark sqaure
            SDL_RenderFillRect(renderer, &cell);

            int piece = board[row][col];
            if (piece != khali) {
                int cx = col * cell_size + cell_size / 2;
                int cy = row * cell_size + cell_size / 2;
                int radius = (cell_size / 2) - 10;

                if (piece == RED || piece == RED_K)
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    //color for red
                else
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    //color for black

                drawCirc(renderer, cx, cy, radius);

                if (piece == RED_K || piece == BLACK_K) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    drawCirc(renderer, cx, cy, radius / 2);
                }
            }
        }
    }
}

void initialboard() {
    for (int i = 0; i < cells; i++) {
        for (int j = 0; j < cells; j++) {
            if ((i + j) % 2 == 1) {
                if (i < 3) board[i][j] = BLACK;
                //till 3 make black
                else if (i > 4) board[i][j] = RED;
                // after 4 make red
                else board[i][j] = khali;
            } else {
                board[i][j] = khali;
            }
        }
    }
}

bool isValidMove(int sr, int sc, int dr, int dc, bool isRed) {
    if (dr < 0 || dr >= cells || dc < 0 || dc >= cells){
    return false;
    }
    if (board[sr][sc] == khali || board[dr][dc] != khali) {
        return false;
    }
    //check if outside the board or if selected empty
    int piece = board[sr][sc];
    int direction = isRed ? -1 : 1;

    // Simple diagonal move
    if ((piece == RED || piece == BLACK) && dr - sr == direction && abs(dc - sc) == 1) {
        return true;
    }

    // condition for king moving downward
    if ((piece == RED_K || piece == BLACK_K) && abs(dr - sr) == 1 && abs(dc - sc) == 1) {
        return true;
    }
    //check for capturing the piece
    if (abs(dr - sr) == 2 && abs(dc - sc) == 2) {
        int midrow = (sr + dr) / 2;
        int midcol = (sc + dc) / 2;
        int midPiece = board[midrow][midcol];
        if (isRed && (midPiece == BLACK || midPiece == BLACK_K)){
            return true;
        }
        if (!isRed && (midPiece == RED || midPiece == RED_K)){
            return true;
        }
    }

    return false;
}

void makeMove(int sr, int sc, int dr, int dc) {
    int piece = board[sr][sc];
    board[dr][dc] = piece;
    board[sr][sc] = khali;

    // Promotion
    if (piece == RED && dr == 0){
        board[dr][dc] = RED_K;
    }
    if (piece == BLACK && dr == 7){
        board[dr][dc] = BLACK_K;
    }
    //Capture
    if (abs(dr - sr) == 2 && abs(dc - sc) == 2) {
        int midrow = (sr + dr) / 2;
        int midcol = (sc + dc) / 2;
        board[midrow][midcol] = khali;
    }
    
    redTurn = !redTurn;

}

void sendMove(int sr, int sc, int dr, int dc, int sock) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d %d %d %d", sr, sc, dr, dc);
    send(sock, buffer, strlen(buffer), 0);
    //networking thing :)
}

bool NoBlock(int* sr, int* sc, int* dr, int* dc, int sock) {
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

    if (activity > 0 && FD_ISSET(sock, &readfds)) {
        char buffer[20] = {0};
        int valread = read(sock, buffer, sizeof(buffer));
        if (valread > 0) {
            sscanf(buffer, "%d %d %d %d", sr, sc, dr, dc);
            return true;
        }
    }
    return false;
}

int main() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr*)&address, sizeof(address));
    listen(server_socket, 1);
    printf("Waiting for a client...\n");
    client_socket = accept(server_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    // accepting the request from client
    if (client_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Checkers Server - Black", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_size, window_size, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initialboard();

    int selectedRow = -1, selectedCol = -1;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;
            
            if (event.type == SDL_MOUSEBUTTONDOWN && !redTurn) {
                int x = event.button.x;
                int y = event.button.y;
                int row = y / cell_size;
                int col = x / cell_size;

                int piece = board[row][col];

                if (selectedRow == -1) {
                    if ((piece == BLACK || piece == BLACK_K)) {
                        selectedRow = row;
                        selectedCol = col;
                    }
                } 
                else {
                    if (isValidMove(selectedRow, selectedCol, row, col, false)) {
                        makeMove(selectedRow, selectedCol, row, col);
                        sendMove(selectedRow, selectedCol, row, col, client_socket);
                        redTurn = true;
                    }
                    selectedRow = -1;
                }
            }
        }

        if (redTurn) {
            int sr, sc, dr, dc;
            if (NoBlock(&sr, &sc, &dr, &dc, client_socket)) {
                makeMove(sr, sc, dr, dc);
                redTurn = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawBoard(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    close(client_socket);
    close(server_socket);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
