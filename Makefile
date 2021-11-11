default: main

libs=-lSDL2 -lSDL2main -lSDL2_gfx -lm
CPPFLAGS=$(libs) -g -Wall

main: main.cpp transform.o misc.o

transform.o: transform.cpp transform.h
transform.h:
misc.o: misc.cpp
