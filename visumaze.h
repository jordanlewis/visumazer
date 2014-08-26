#ifndef __VISUMAZE_H
#define __VISUMAZE_H

typedef struct {
    float xpos, ypos, zpos;
    float xvel, yvel, zvel;
    float xrot, yrot, zrot;
    float xrotvel, yrotvel, zrotvel;
} obj_state;

float deg_to_rad(double deg);

void resize_gl_scene(int width, int height);
int init_gl(int width, int height);

void draw_light(void);
void draw_floor(void);
void draw_walls(void);

unsigned char check_collisions(float x, float y, float z);
void move(float x, float y, float z);

void display(void);

void read_keys(void);

void special_pressed(int key, int x, int y);
void special_released(int key, int x, int y);
void key_pressed(unsigned char key, int x, int y);
void key_released(unsigned char key, int x, int y);

int main(int argc, char **argv);

#endif
