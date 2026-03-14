#include <SDL2/SDL.h>

#define cells 8
#define cell_size 80

enum { khali, RED, BLACK, RedK, BlackK };
extern int board[cells][cells];

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