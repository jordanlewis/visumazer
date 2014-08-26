CFLAGS = -std=c99 -Wall -pedantic -g
GLFLAGS = -framework OpenGL -framework GLUT

OBJS = visumaze.o parse.o

all: visumaze tags maze_gen

visumaze: $(OBJS)
	gcc $(CFLAGS) -lm $(GLFLAGS) $(OBJS) -o visumaze

gen_maze: parse.c

tags:
	ctags visumaze.c visumaze.h parse.c parse.h

clean:
	rm -f *.o
	rm -f visumaze maze_gen
	rm -rf *.dSYM
	rm -f tags

run: all
	./visumaze
