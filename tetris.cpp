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
#include <unordered_map>
#include <math.h>
using namespace std;

extern const int GAME_WIDTH;
extern const int GAME_HEIGHT;

std::unordered_map<int, std::string> id_to_piece_string ({{ 0, "I"},{ 1, "O"},{ 2, "T"},{ 3, "S"},{ 4, "Z"},{ 5, "L"},{ 6, "J"}});

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
            list<list<int> > state1 = {{0,1}, {1,0}, {1,1}, {2,1}};
            list<list<int> > state2 = {{1,1}, {2,0}, {2,1}, {2,2}};
            list<list<int> > state3 = {{0,1}, {1,1}, {2,1}, {1,2}};
            states = {state0, state1, state2, state3};
            break;
        }
        case 3: { // S
            list<list<int> > state0 = {{0,1}, {0,2}, {1,0}, {1,1}};
            list<list<int> > state1 = {{0,1}, {1,1}, {1,2}, {2,2}};
            states = {state0, state1};
            break;
        }
        case 4: { // Z
            list<list<int> > state0 = {{0,0}, {0,1}, {1,1}, {1,2}};
            list<list<int> > state1 = {{0,1}, {1,1}, {1,0}, {2,0}};
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
            list<list<int> > state1 = {{0,1}, {1,1}, {2,1}, {2,0}};
            list<list<int> > state2 = {{1,0}, {2,0}, {2,1}, {2,2}};
            list<list<int> > state3 = {{0,1}, {0,2}, {1,1}, {2,1}};
            states  = {state0, state1, state2, state3};
        }
    }
}

Piece::~Piece(){ }

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
    for (int i = 0; i < 6; i ++){
        piece_queue.push_back(Piece());
    }
    end_game = false;
    lines_cleared = 0;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);
}

Game::~Game(){ }

bool Piece::rotate(Game * tetris){
    int new_state = (state + 1) % states.size();
    if (!in_bounds(tetris, new_state)){ return false;}
    list<list<int> > coords = states[new_state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (tetris->board[(*it).front() + tetris->row][(*it).back() + tetris->col] == "[]"){
            return false;
        }
    }
    state = new_state;
    return true;
}

bool Piece::in_bounds(Game * tetris, int new_state){
    list<list<int> > coords = states[new_state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if ((*it).front() + tetris->row > GAME_HEIGHT - 1){
            return false;
        }
        else if (((*it).back() + tetris->col > GAME_WIDTH - 1) || ((*it).back() + tetris->col < 0)){
            return false;
        }
    }
    return true;
}

bool Game::shift_down(){
    list<list<int> > coords = piece_queue.front().states[piece_queue.front().state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row + 1][(*it).back() + col] == "[]"){
            place_piece();
            return false;
        }
        if ((*it).front() + row == GAME_HEIGHT - 1){
            place_piece();
            return false;
        }
    }
    row += 1;
    return true;
}

bool Game::move_left(){
    list<list<int> > coords = piece_queue.front().states[piece_queue.front().state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row][(*it).back() + col - 1] == "[]" || (*it).back() + col <= 0){
            return false;
        }
    }
    col -= 1;
    return true;
}

bool Game::move_right(){
    list<list<int> > coords = piece_queue.front().states[piece_queue.front().state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row][(*it).back() + col + 1] == "[]" || (*it).back() + col >= (GAME_WIDTH - 1)){
            return false;
        }
    }
    col += 1;
    return true;
}

void Game::place_piece(){
    list<list<int> > coords = piece_queue.front().states[piece_queue.front().state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        board[(*it).front() + row][(*it).back() + col] = "[]";
    }
    remove_lines_and_score();
    spawn_piece();
    return;
}

void Game::visualize(){
    list<list<int> > coords = piece_queue.front().states[piece_queue.front().state];
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
    printw("Score: %d\n", score);
    printw("Next Pieces: ");
    for (list<Piece> ::iterator it = piece_queue.begin(); it != piece_queue.end(); it++){
        if (!(it == piece_queue.begin())){
            int num = ((*it).piece_num);
            printw(id_to_piece_string[num].c_str());
            printw(" ");
        }
    }
    printw("\n");
    return;
}

void Game::spawn_piece(){
    row = 0;
    col = 3;
    piece_queue.pop_front();
    piece_queue.push_back(Piece());
    list<list<int> > coords = piece_queue.front().states[piece_queue.front().state];
    for (list<list<int> > ::iterator it = coords.begin(); it != coords.end(); it++){
        if (board[(*it).front() + row][(*it).back() + col] == "[]"){
            end_game = true;
            return;
        }
    }
    return;
}

void Game::shift_rows_down(int row_index){
    for (int r = row_index; r > 0; r--){
        for (int c = 0; c < GAME_WIDTH; c++){
            board[r][c] = board[r-1][c];
        }
    }
    for (int c = 0; c < GAME_WIDTH; c++){
        board[0][c] = "__";
    }
    return;
}

int Game::remove_lines_and_score(){
    int lines = 0;
    for (int r = 0; r < GAME_HEIGHT; r++){
        bool remove = true;
        for (int c = 0; c < GAME_WIDTH; c++){
            if (board[r][c] == "__"){
                remove = false;
                break;
            }
        }
        if (remove){
            shift_rows_down(r);
            lines++;
            r--;
        }
    }
    score += lines * lines;
    return lines;
}

static void* async_function(void* arg){
    Game * tetris = (Game *) arg;
    int ch;
    while (((ch = getch()) != '\n') && !(tetris->end_game)) {
        pthread_mutex_lock(&(tetris->lock));
        switch(ch) {
            case KEY_UP: 
                tetris->piece_queue.front().rotate(tetris);
                break;
            case ' ': // Space Bar
                while(tetris->shift_down()){
                    continue;
                }
                break;
            case KEY_LEFT: 
                tetris->move_left();
                break;
            case KEY_RIGHT: 
                tetris->move_right();
                break;
            //default: tetris->end_game = true;
        }
        clear();
        tetris->visualize();
        refresh();
        pthread_mutex_unlock(&(tetris->lock));
    }
    return tetris;
}

static void* sync_function(void* arg){
    Game * tetris = (Game *) arg;
    while(!(tetris->end_game)) {
        pthread_mutex_lock(&(tetris->lock));
        clear();
        tetris->visualize();
        refresh();
        pthread_mutex_unlock(&(tetris->lock));
        usleep(500000 * pow(0.95, int(tetris->lines_cleared/5)));
        pthread_mutex_lock(&(tetris->lock));
        tetris->shift_down();
        clear();
        tetris->visualize();
        refresh();
        pthread_mutex_unlock(&(tetris->lock));
    }
    return tetris;
}

int main(){
    Game tetris = Game();
    tetris.spawn_piece();
    pthread_t async;
    pthread_t sync;
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    pthread_create(&async, NULL, async_function, (void *) &tetris);
    pthread_create(&sync, NULL, sync_function, (void *) &tetris);  
    pthread_join(async, NULL);
    pthread_join(sync, NULL);
    refresh();
    getch();
    endwin();
    return 0;
}