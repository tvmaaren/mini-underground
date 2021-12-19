default: main 

libs=-lSDL2 -lSDL2main -lSDL2_gfx -lm
CPPFLAGS=$(libs) -g -Wall

main: main.cpp transform.o misc.o lines.o types.hpp stations.o settings.hpp
lines.o: lines.cpp transform.o misc.o lines.hpp types.hpp settings.hpp
stations.o: stations.cpp stations.hpp transform.o settings.hpp
transform.o: transform.cpp transform.hpp
transform.h:
misc.o: misc.cpp misc.hpp types.hpp
