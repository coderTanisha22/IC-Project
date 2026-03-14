#include <stdbool.h>
#include <stdlib.h>

#define cells 8

enum { khali, RED, BLACK, RedK, BlackK };

int board[cells][cells];
bool redTurn = true;

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
            return true;
        if (!isRed && (midPiece == RED || midPiece == RedK))
            return true;
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
        board[dr][dc] = RedK;

    if (piece == BLACK && dr == 7)
        board[dr][dc] = BlackK;

    //capture move ke liye
    if (abs(dr - sr) == 2 && abs(dc - sc) == 2) {
        int midrow = (sr + dr) / 2;
        int midcol = (sc + dc) / 2;
        board[midrow][midcol] = khali;
    }

    redTurn = !redTurn;
}