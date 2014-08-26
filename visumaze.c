#include <stdlib.h>
#include <stdio.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <math.h>

#include "visumaze.h"
#include "parse.h"

#define WIDTH 800
#define HEIGHT 600

int keys[256];
int special_keys[256];
float speed = .02;

float lightx, lighty, lightz=-5;
obj_state player_state;

maze_type *maze;

float deg_to_rad(double deg) { // {{{
    return deg * M_PI / 180;
} //}}}

void resize_gl_scene(int width, int height) // {{{
{
    // called by glut if we resize. needed for glut.
    if (height == 0) {
        height = 1;
    }
    
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat) width / (GLfloat) height, 0.1f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
} // }}}

int init_gl(int width, int height) //{{{
{
    // random gl init functions live here.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // rgb alpha, for bg
    glClearDepth(1.0f);

    glShadeModel(GL_SMOOTH); // smooth shading
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // lighting and color on
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glCullFace(GL_FRONT_AND_BACK);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    // we do key release checking ourselves
    glutIgnoreKeyRepeat(1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // reset modelview matrix

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // best persp. corr.
    return 1;
} // }}}

void draw_light(void) // {{{
{
    // draws our light.
    float light_position[] = {lightx, lighty, lightz, 1};
    glPushMatrix();
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glTranslatef(lightx,lighty,lightz);

    //cube should be drawn without the lighting on so its ok looking
    glDisable(GL_LIGHTING);
    glColor3f(.4,.6,.3);
    glutSolidCube(.1);
    glEnable(GL_LIGHTING);
    glPopMatrix();
} // }}}

void draw_floor(void) // {{{
{
    // draws a floor. just a plane -5 below.
    glPushMatrix();
    glTranslatef(0,-5,0);
    glScalef(10000,1,1000);
    glColor3f(0,1,0);
    glutSolidCube(1);
    glPopMatrix();
} // }}}

unsigned char check_collisions(float x, float y, float z) //{{{
{
    // checks for collisions between the given position and the maze walls.
    // returns a char with bits set depending on what is colliding:
    // x = 1, y = 2, z = 4
    unsigned char result = 0;
    int cols = maze->cols;
    x /= 10;
    z /= 10;
    float xf = floor(x);
    float zf = floor(z);
    // add some larger boundaries so we can't see inside of the walls
    x += ((x - xf >= .5) ? 1 : -1) * .03;
    z += ((z - zf >= .5) ? 1 : -1) * .03;
    xf = floor(x);
    zf = floor(z);
    // set the bits depending on where we're colliding
    if (maze->walls[(int)((floor(player_state.zpos/10)*cols)+xf)]){
        result |= 1;
    }
    if (maze->walls[(int)((zf*cols)+floor(player_state.xpos/10))]){
        result |= 4;
    }
    
    return result;
} //}}}

void draw_walls(void) //{{{
{
    // draws the walls in maze->walls.
    glPushMatrix();
    glColor3f(0,0,1);
    for (int row = 0; row < maze->rows; ++row) {
        for (int col = 0; col < maze->cols; ++col) {
            // visual threshold - don't draw if too far away
            if (sqrt(pow(row-player_state.zpos/10,2)+
                     pow(col-player_state.xpos/10,2)) < 10) {
                glPushMatrix();
                // go to the middle of the square to draw the cube since
                // it is built around the origin
                glTranslatef((10*col)+5,0,(10*row)+5);
                if (maze->walls[(row * maze->cols) + col] == 1) {
                    // little bigger so we don't get ugly lines
                    glutSolidCube(10.2);
                }
                glPopMatrix();
            }
        }
    }
    glPopMatrix();
}//}}}

void move(float x, float y, float z) // {{{
{
    // moves the player, checking and handling collisions and wins
    unsigned char collisions = check_collisions(x,y,z);
    if (~collisions & 1) {
        player_state.xpos = x;
    }
    if (~collisions & 2) {
        player_state.ypos = y;
    }
    if (~collisions & 4) {
        player_state.zpos = z;
    }
    // have we gotten to the end square?
    if ((int) floor(z/10) * maze->cols + (int) floor(x/10) == maze->cols * maze->rows - 1) {
        printf("You have found the exit!\n");
        exit(0);
    }
} // }}}

void display(void) // {{{
{
    // main display function.
    read_keys();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen+depth buf

    glPushMatrix(); // camera 'position'
    // we move and rotate the entire scene in the opposite way from the acutal
    // location of the player. this gives the illusion of the camera moving;
    // its much easier to move the scene than to move the camera.
    glRotatef(player_state.xrot, 1, 0, 0);
    glRotatef(player_state.yrot, 0, 1, 0);
    glRotatef(player_state.zrot, 0, 0, 1);

    glTranslatef(-player_state.xpos, -player_state.ypos, -player_state.zpos);

    // draw all components
    draw_light();
    draw_floor();
    draw_walls();

    // pop the camera position off
    glPopMatrix();

    //glFlush();
    glutSwapBuffers();

    // do gravity, and make sure we aren't going through the floor
    if (player_state.ypos > -1.9) {
        player_state.yvel -= .0001;
    }
    else {
        if (player_state.yvel < 0) {
            player_state.yvel = 0;
        }
    }

    // update all of the velocities! we don't actually use all of these, but
    // for pedagogy..
    player_state.xpos = player_state.xpos + player_state.xvel;
    player_state.zpos = player_state.zpos + player_state.zvel;
    player_state.ypos = player_state.ypos + player_state.yvel;
    player_state.xrot = player_state.xrot + player_state.xrotvel;
    player_state.zrot = player_state.zrot + player_state.zrotvel;
    player_state.yrot = player_state.yrot + player_state.yrotvel;
    player_state.xrot -= 360*floor(player_state.xrot/360);
    player_state.yrot -= 360*floor(player_state.yrot/360);
    player_state.zrot -= 360*floor(player_state.zrot/360);
    return;
} // }}}

void read_keys(void) // {{{
{
    // check the keys arrays and act depending on what they're set to.
    if (keys[27] || keys['q']) {
        exit(0);
    }
    if (keys['j']) {
        lightx -= .1;
    }
    if (keys['l']) {
        lightx += .1;
    }
    if (keys['k']) {
        lighty -= .1;
    }
    if (keys['i']) {
        lighty += .1;
    }
    if (keys['w']) {
        float z = player_state.zpos - speed *cos(deg_to_rad(player_state.yrot));
        float x = player_state.xpos + speed *sin(deg_to_rad(player_state.yrot));
        move(x,player_state.ypos,z);
    }
    if (keys['s']) {
        float z = player_state.zpos + speed *cos(deg_to_rad(player_state.yrot));
        float x = player_state.xpos - speed *sin(deg_to_rad(player_state.yrot));
        move(x,player_state.ypos,z);
    }
    if (keys['a']) {
        float x = player_state.xpos - speed *cos(deg_to_rad(player_state.yrot));
        float z = player_state.zpos - speed *sin(deg_to_rad(player_state.yrot));
        move(x,player_state.ypos,z);
    }
    if (keys['d']) {
        float x = player_state.xpos + speed *cos(deg_to_rad(player_state.yrot));
        float z = player_state.zpos + speed *sin(deg_to_rad(player_state.yrot));
        move(x,player_state.ypos,z);
    }
    if (keys[' ']) {
        player_state.yvel = .05;
    }
    if (special_keys[GLUT_KEY_UP]) {
        player_state.xrot -= .1;
    }
    if (special_keys[GLUT_KEY_DOWN]) {
        player_state.xrot += .1;
    }
    if (special_keys[GLUT_KEY_RIGHT]) {
        player_state.yrot += .1;
    }
    if (special_keys[GLUT_KEY_LEFT]) {
        player_state.yrot -= .1;
    }
} // }}}

void special_pressed(int key, int x, int y) // {{{
{
    // this maintains state about just pressed versus long-time pressed
    // for jumping and acceleration stuff
    if (special_keys[key] == 0) {
        special_keys[key] = 1;
    }
    else if (special_keys[key] == -1) {
        special_keys[key] = 2;
    }
} // }}}
void special_released(int key, int x, int y) // {{{
{
    // same as above
    if (special_keys[key] == 2) {
        special_keys[key] = -1;
    }
    else if (special_keys[key] == 1) {
        special_keys[key] = 0;
    }
} // }}}
void key_pressed(unsigned char key, int x, int y) // {{{
{
    keys[key] = 1;
} // }}}
void key_released(unsigned char key, int x, int y) // {{{
{
    keys[key] = 0;
} // }}}

int main(int argc, char **argv) // {{{
{
    FILE *input;
    if (argc != 2) {
       input = stdin;
    }
    else {
        input = fopen(argv[1], "r");
    }
    maze = parse_file(input);
    fclose(input);

    // print out the maze to stdout to see how it looks
    print_maze(maze);
    glutInit(&argc, argv);

    // display mode: double buffer, rgba color, depth buffer
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (WIDTH, HEIGHT);
    glutInitWindowPosition (100, 100);
    glutCreateWindow("ogl");  

    // give all the callbacks functions to glut
    glutDisplayFunc(&display);
    //glutFullScreen();
    glutIdleFunc(&display);
    glutReshapeFunc(&resize_gl_scene);
    glutKeyboardFunc(&key_pressed);
    glutKeyboardUpFunc(&key_released);
    glutSpecialFunc(&special_pressed);
    glutSpecialUpFunc(&special_released);

    init_gl(WIDTH, HEIGHT);
    
    // initialize our dude's position
    player_state.xpos = 1;
    player_state.zpos = 15;
    player_state.yrot = 90;

    glutMainLoop();

    return 0;
} // }}}
