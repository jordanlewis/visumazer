#include <stdlib.h>
#include <stdio.h>
#include "parse.h"
#include "string.h"

maze_type *create_maze(int rows, int cols) // {{{
{
    // creates a new maze with given rows and cols
    maze_type *maze = (maze_type *) malloc(sizeof(maze_type));
    maze->rows = rows;
    maze->cols = cols;
    return maze;
} // }}}
void delete_maze(maze_type *maze) // {{{
{
    // frees a maze structure
    free(maze->walls);
    free(maze);
} // }}}

int get_wall(maze_type *maze, int row, int col) // {{{
{
    // returns the wall at row, col in the maze
    // indexed from 0
    if (row >= maze->rows || col >= maze->cols) {
        fprintf(stderr, "trying to access an undefined wall\n");
    }
    return maze->walls[(row * maze->cols) + col];
} // }}}
void set_wall(maze_type *maze, int row, int col, int wall) // {{{
{
    // sets the wall at given coords to given state
    if (row >= maze->rows || col >= maze->cols) {
        fprintf(stderr, "trying to access an undefined wall\n");
    }
    maze->walls[(row * maze->cols) + col] = wall;
} // }}}

void print_maze(maze_type *maze) // {{{
{
    // prints out the maze to stdout
    for (int row = 0; row < maze->rows; ++row) {
        for (int col = 0; col < maze->cols; ++col) {
            switch (get_wall(maze, row, col)) {
                case 1: printf("#"); break;
                default: printf(" "); break;
            }
        }
        printf("\n");
    }
} // }}}

maze_type *parse_file(FILE *input)  // {{{
{
    // given a file pointer, parse as a maze and put into a maze structure
    int line = 0;
    int pos = 0;
    int cols = 0;
    char next_char;
    maze_type *maze;
    int *walls = (int *) malloc(sizeof(int) * 80);
    while ((next_char = getc(input)) != EOF) {
        switch (next_char) {
            case '#':
                walls[pos] = 1;
                break;
            case ' ':
                walls[pos] = 0;
                break;
            case '\n':
                if (cols == 0) {
                    cols = pos;
                }
                ++line;
                // don't change the pos since we dont care about newlines
                --pos;
                // hack - can't figure out why we're not allocing enough mem,
                // so just add a bunch on after
                walls = realloc(walls, sizeof(int) * (line * cols + 50));
                break;
        }
        ++pos;
    }
    maze = create_maze(line, cols);
    maze->walls = walls;
    return maze;
} // }}}
