#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "parse.h"

int neighbors_pos[8] = {1, 0, -1, 0, 0, 1, 0, -1};

void shuffle(int *array, int array_len) //{{{
{
    // randomly permutes array.
    for (int i = array_len - 1; i >= 1; --i) {
        int k = (random() % i) + 1; // since random gives from 0 to i-1
        int temp = array[0];
        array[0] = array[k];
        array[k] = temp;
    }
} // }}}

int *get_walls(unsigned int *cells, int cols, int rows) // {{{
{
    // from a cell-with-walls bitfield returns a normal walls array suitable 
    // for a maze.walls.
    int *blocks = (int *) malloc(sizeof(int) * cols * rows * 4);
    for (int i = 0; i < cols*rows*4; ++i) {
        // all normal cells (non-walls) are clearly open
        blocks[i] = 1;
    }
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            blocks[(row * 2 * cols * 2) + (col * 2)] = 0;
            // and check down and right neighbors for open corridors
            if (col < cols - 1 && ~cells[(row * cols) + col] & 4 &&
                ~cells[(row * cols) + col + 1] & 1) {
                blocks[(row * 2 * cols * 2) + (col * 2) + 1] = 0;
            }
            if (row < rows - 1 && ~cells[(row * cols) + col] & 8 &&
                ~cells[((row + 1) * cols) + col] & 2) {
                blocks[((row*2 + 1) * cols * 2) + (col * 2)] = 0;
            }
        }
    }
    return blocks;
} // }}}

void knock_walls(int x, int y, int cols, int rows, unsigned int *cells) // {{{
{
    // the maze generation algorithm. behaves like a random depth-first search.
    // 1. mark cell as visited
    // 2. randomize list of neighbors
    // 3. go to each neighbor that hasn't been visited, remove wall between
    //    current cell and that cell, and recurse from there.
    
    int n_neighbor[4] = {0,1,2,3};
    int cur_x = x, cur_y = y;
    int new_x, new_y;
    // mark the current cell as visited
    cells[(cur_y * cols) + cur_x] |= 16;
    shuffle(n_neighbor, 4);
    for (int i = 0; i < 4; ++i) {
        new_x = x + neighbors_pos[2*n_neighbor[i]];
        new_y = y + neighbors_pos[2*n_neighbor[i] + 1];
        if (new_x < 0 || new_x >= cols || new_y < 0 || new_y >= cols ||
            cells[(new_y * cols) + new_x] & 16) {
            continue;
        }
        // the neighbor hasn't been visited
        // cut down the wall between us and the new cell
        switch (neighbors_pos[2*i]) {
            case -1:
                cells[(y * cols) + x] &= ~1;
                cells[(new_y * cols) + new_x] &= ~4;
                break;
            case 1:
                cells[(y * cols) + x] &= ~4;
                cells[(new_y * cols) + new_x] &= ~1;
                break;
            default:
                break;
        }
        switch (neighbors_pos[2*i + 1]) {
            case -1:
                cells[(y * cols) + x] &= ~2;
                cells[(new_y * cols) + new_x] &= ~8;
                break;
            case 1:
                cells[(y * cols) + x] &= ~8;
                cells[(new_y * cols) + new_x] &= ~2;
                break;
            default:
                break;
        }
        // and recurse with the new cell as a starting point
        knock_walls(new_x, new_y, cols, rows, cells);
    }
} // }}}

int *gen_walls(int cols, int rows) // {{{
{
    // returns a random map with cols and rows.
    // here each cell is an unsigned int representing the walls around it:
    // 1 is left, 2 is top, 4 is right, 8 is down, 16 is visited
    int num_cells = cols * rows;
    int x = 0, y = 0;
    unsigned int *cells = (unsigned int *) malloc(sizeof(unsigned int) * num_cells);
    for (int i = 0; i < num_cells; ++i) {
        cells[i] = 15;
    }
    knock_walls(x, y, cols, rows, cells);
    return get_walls(cells, cols, rows);
} // }}}

int main (int argc, char **argv) // {{{
{
    if (argc != 3) {
        printf("usage: maze_gen [width] [height]\n");
        exit(1);
    }
    int width = (int) strtol(argv[1], NULL, 10);
    int height = (int) strtol(argv[2], NULL, 10);
    srandom(time(NULL));
    int *walls = gen_walls(width, height);
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            switch (walls[row*width + col]) {
                case 1: printf("#"); break;
                default: printf(" "); break;
            }
        }
        printf("\n");
    }
    //print_walls(walls);
    //maze_type *maze = create_maze(10,10);
    //maze->walls = walls;
    //print_maze(maze);
} // }}}
