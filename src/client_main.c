#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080 
#define cells 8
#define cell_size 80
#define window_size (cells * cell_size)


enum { khali, RED, BLACK, RedK, BlackK };

//from game.c
extern int board[cells][cells];
extern bool redTurn;

void initialboard();
bool isValidMove(int sr, int sc, int dr, int dc, bool isRed);
void makeMove(int sr, int sc, int dr, int dc);

//from ui.c
void drawBoard(SDL_Renderer* renderer);

int sock = -1;
bool quit = false;
bool isRedPlayer = false;

static const char* side_name(bool is_red) {
    return is_red ? "RED" : "BLACK";
}

static bool is_own_piece(int piece, bool is_red_player) {
    if (is_red_player) {
        return piece == RED || piece == RedK;
    }
    return piece == BLACK || piece == BlackK;
}

//move send ke liye
void sendMove(int sr, int sc, int dr, int dc, int sock) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d %d %d %d\n", sr, sc, dr, dc);
    printf("[client-%s] sending move: %d %d -> %d %d\n", side_name(isRedPlayer), sr, sc, dr, dc);
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
        int readval = read(sock, buffer, sizeof(buffer) - 1);
        if (readval > 0) {
            buffer[readval] = '\0';
            if (sscanf(buffer, "%d %d %d %d", sr, sc, dr, dc) == 4) {
                printf("[client-%s] received move: %d %d -> %d %d\n", side_name(isRedPlayer), *sr, *sc, *dr, *dc);
                return true;
            }
            printf("[client-%s] received malformed payload: %s\n", side_name(isRedPlayer), buffer);
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

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { // ip check karna
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    char roleByte = 0;
    int roleRead = read(sock, &roleByte, 1);
    if (roleRead != 1) {
        printf("Failed to receive role from server\n");
        close(sock);
        return -1;
    }

    if (roleByte == 'R') {
        isRedPlayer = true;
    }

    printf("[client] connected to server on 127.0.0.1:%d\n", PORT);
    printf("[client] assigned role: %s\n", side_name(isRedPlayer));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window;

    if(isRedPlayer){
        window = SDL_CreateWindow("Checkers Client - Red",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                window_size, window_size, 0);
    }
    else{
        window = SDL_CreateWindow("Checkers Client - Black",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            window_size, window_size, 0);
        }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initialboard();

    int selectedRow = -1, selectedCol = -1;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x;
                int y = event.button.y;
                int row = y / cell_size;
                int col = x / cell_size;

                if (row < 0 || row >= cells || col < 0 || col >= cells) {
                    continue;
                }

                printf("[client-%s] click at row=%d col=%d (turn=%s)\n",
                       side_name(isRedPlayer), row, col, side_name(redTurn));

                if (redTurn != isRedPlayer) {
                    printf("[client-%s] ignored click: not your turn\n", side_name(isRedPlayer));
                    continue;
                }

                int piece = board[row][col];

                if (selectedRow == -1) {
                    if (is_own_piece(piece, isRedPlayer)) {
                        selectedRow = row;
                        selectedCol = col;
                        printf("[client-%s] selected piece at %d %d\n", side_name(isRedPlayer), row, col);
                    } else {
                        printf("[client-%s] selection rejected: choose your own piece\n", side_name(isRedPlayer));
                    }
                } 
                else {
                    if (selectedRow == row && selectedCol == col) {
                        printf("[client-%s] deselected piece at %d %d\n", side_name(isRedPlayer), row, col);
                    } else if (isValidMove(selectedRow, selectedCol, row, col, isRedPlayer )) {
                        makeMove(selectedRow, selectedCol, row, col);
                        sendMove(selectedRow, selectedCol, row, col, sock);
                        printf("[client-%s] move applied locally\n", side_name(isRedPlayer));
                    } else {
                        printf("[client-%s] invalid move from %d %d to %d %d\n",
                               side_name(isRedPlayer), selectedRow, selectedCol, row, col);
                    }
                    selectedRow = -1;
                    selectedCol = -1;
                }
            }
        }

        if (redTurn != isRedPlayer) {
            int sr, sc, dr, dc;
            if (noblock(&sr, &sc, &dr, &dc, sock)) {
                makeMove(sr, sc, dr, dc);
                // redTurn = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawBoard(renderer);

        if (selectedRow != -1 && selectedCol != -1) {
            SDL_Rect highlight = {
                selectedCol * cell_size,
                selectedRow * cell_size,
                cell_size,
                cell_size
            };
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &highlight);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    close(sock);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
