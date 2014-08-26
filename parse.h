#ifndef __PARSE_H
#define __PARSE_H

typedef struct {
    int rows, cols;
    int *walls;
} maze_type;

maze_type *create_maze(int rows, int cols);
void delete_maze(maze_type *maze);

int get_wall(maze_type *maze, int row, int col);
void set_wall(maze_type *maze, int row, int col, int wall);

void print_maze(maze_type *maze);
maze_type *parse_file(FILE *input);

#endif
