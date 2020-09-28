#ifndef TETRIS_H
#define TETRIS_H
#include <string>
#include <iostream>
#include <list>
#include <vector>
using namespace std;

class Piece;
class Game;

static const int GAME_WIDTH = 10;
static const int GAME_HEIGHT = 20;

class Piece{
     public:
        Piece();
        ~Piece();
        int piece_num;
        int state;
        bool rotate(Game * tetris);
        bool in_bounds(Game * tetris, int new_state);
        void visualize();
        vector<list<list<int> > > states;
};

class Game {
    public:
        Game();
        ~Game();
        bool shift_down();
        bool move_left();
        bool move_right();
        void place_piece();
        void visualize();
        void spawn_piece();
        int remove_lines_and_score();
        void shift_rows_down(int row_index);
        const char * board[GAME_HEIGHT][GAME_WIDTH];
        int score;
        int row;
        int col;
        bool end_game;
        int lines_cleared;
        list<Piece> piece_queue;
        pthread_mutex_t lock;
        pthread_cond_t condition;
};
#endif
