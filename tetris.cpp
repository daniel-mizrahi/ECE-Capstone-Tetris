#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <unistd.h>
#include "tetris.h"
#include <chrono>
#include <pthread.h>
#include <ncurses.h>
#include <curses.h>
using namespace std;

extern const int GAME_WIDTH;
extern const int GAME_HEIGHT;

Piece::Piece(){
    piece_num = rand() % 7;
    state = 0;
    switch (piece_num){
        case 0: { // I
            list<list<int> > state0 = {{0,0}, {0,1}, {0,2}, {0,3}};
            list<list<int> > state1 = {{0,2}, {1,2}, {2,2}, {3,2}};
            states = {state0, state1};
            break;
        }
        case 1: { // O
            list<list<int> > state0 = {{0,1}, {0,2}, {1,1}, {1,2}};
            states = {state0};
            break;
        }
        case 2: { // T
            list<list<int> > state0 = {{0,0}, {0,1}, {0,2}, {1,1}};
            list<list<int> > state1 = {{0,1}, {1,1}, {1,2}, {2,2}};
            list<list<int> > state2 = {{1,1}, {2,0}, {2,1}, {2,2}};
            list<list<int> > state3 = {{0,1}, {1,1}, {2,1}, {2,2}};
            states = {state0, state1, state2, state3};
            break;
        }
        case 3: { // S
            list<list<int> > state0 = {{0,1}, {0,2}, {1,0}, {1,1}};
            list<list<int> > state1 = {{0,0}, {1,0}, {1,1}, {2,1}};
            states = {state0, state1};
            break;
        }
        case 4: { // Z
            list<list<int> > state0 = {{0,0}, {0,1}, {1,1}, {1,2}};
            list<list<int> > state1 = {{0,2}, {1,1}, {1,2}, {2,2}};
            states = {state0, state1};
            break;
        }
        case 5: { // L
            list<list<int> > state0 = {{0,0}, {0,1}, {0,2}, {1,0}};
            list<list<int> > state1 = {{0,0}, {0,1}, {1,1}, {2,1}};
            list<list<int> > state2 = {{0,2}, {1,0}, {1,1}, {1,2}};
            list<list<int> > state3 = {{0,1}, {1,1}, {2,1}, {2,2}};
            states = {state0, state1, state2, state3};
            break;
        }
        case 6: { // J
            list<list<int> > state0 = {{0,0}, {0,1}, {0,2}, {1,2}};
            list<list<int> > state1 = {{0,1}, {1,1}, {2,1}, {2,2}};
            list<list<int> > state2 = {{1,0}, {2,0}, {2,1}, {2,2}};
            list<list<int> > state3 = {{0,1}, {0,2}, {1,1}, {2,1}};
            states  = {state0, state1, state2, state3};
        }
    }
}

Piece::~Piece(){ }

bool Piece::rotate(const char * board[GAME_HEIGHT][GAME_WIDTH], int row, int col){
    int new_state = (state + 1) % states.size();
    list<list<int> > coords = states[new_state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
         if (board[(*it).front() + row][(*it).back()+ col] == "[]"){
            return false;
        }
    }
    state = new_state;
    return true;
}

Game::Game(){
    for (int r = 0; r < GAME_HEIGHT; r++){
        for (int c = 0; c < GAME_WIDTH; c++){
            board[r][c] = "__";
        }
    }
    score = 0;
    row = 0;
    col = 3;
    srand((unsigned int)time(NULL));
    current_piece = new Piece();
    end_game = false;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);
}

Game::~Game(){ }

bool Game::shift_down(){
    list<list<int> > coords = current_piece->states[current_piece->state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row + 1][(*it).back() + col] == "[]"){
            return false;
        }
        if ((*it).front() + row == GAME_HEIGHT - 1){
            return false;
        }
    }
    row += 1;
    return true;
}

bool Game::move_left(){
    list<list<int> > coords = current_piece->states[current_piece->state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row][(*it).back() + col - 1] == "[]"){
            return false;
        }
        if ((*it).back() + col == 0){
            return false;
        }
    }
    col -= 1;
    return true;

}
bool Game::move_right(){
    list<list<int> > coords = current_piece->states[current_piece->state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row][(*it).back() + col + 1] == "[]"){
            return false;
        }
        if ((*it).back() + col == GAME_HEIGHT - 1){
            return false;
        }
    }
    col += 1;
    return true;

}

void Game::place_piece(){
    list<list<int> > coords = current_piece->states[current_piece->state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        board[(*it).front() + row][(*it).back() + col] = "[]";
    }
    spawn_piece();
}

void Game::visualize(){
    list<list<int> > coords = current_piece->states[current_piece->state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        board[(*it).front() + row][(*it).back() + col] = "[]";
    }
    for (int i = 0; i < GAME_HEIGHT; i++){
        for (int j = 0; j < GAME_WIDTH; j++){
            printw(board[i][j]);
            printw(" ");
        }
        printw("\n");
    }
    printw("\n");
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        board[(*it).front() + row][(*it).back() + col] = "__";
    }
}

void Game::spawn_piece(){
    row = 0;
    col = 3;
    delete current_piece;
    current_piece = new Piece();
    list<list<int> > coords = current_piece->states[current_piece->state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row][(*it).back() + col] == "[]"){
            end_game = true;
            return;
        }
    }
}


static void* async_function(void* arg){
    Game * tetris = (Game *) arg;
    int ch;
    while (((ch = getch()) != '\n') && !(tetris->end_game)) {
        pthread_mutex_lock(&(tetris->lock));
        switch(ch) {
            case KEY_UP: 
                tetris->current_piece->rotate(tetris->board, tetris->row, tetris->col);
                break;
            case KEY_DOWN: 
                //printw("Down\n");
                break;
            case KEY_LEFT: 
                tetris->move_left();
                break;
            case KEY_RIGHT: 
                tetris->move_right();
                break;
            default: tetris->end_game = true;
        }
        refresh();
        pthread_mutex_unlock(&(tetris->lock));
    }
    return tetris;
}

static void* sync_function(void* arg){
    Game * tetris = (Game *) arg;
    char * buffer;
    while(!(tetris->end_game)) {
        pthread_mutex_lock(&(tetris->lock));
        tetris->visualize();
        pthread_mutex_unlock(&(tetris->lock));
        refresh();
        usleep(200000);
        pthread_mutex_lock(&(tetris->lock));
        if(!(tetris->shift_down())){
            tetris->visualize();
            tetris->place_piece();
        }
        pthread_mutex_unlock(&(tetris->lock));
        refresh();
        clear();
    }
    return tetris;
}

int main(){
    Game * tetris = new Game();
    tetris->spawn_piece();
    pthread_t async;
    pthread_t sync;
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    pthread_create(&async, NULL, async_function, (void *) tetris);
    pthread_create(&sync, NULL, sync_function, (void *) tetris);  
    pthread_join(async, NULL);
    pthread_join(sync, NULL);
    refresh();
    getch();
    endwin();
    delete tetris;
    return 0;
}