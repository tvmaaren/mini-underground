default: main 

libs=-lSDL2 -lSDL2main -lSDL2_gfx -lm
CPPFLAGS=$(libs) -g -Wall

main: main.cpp transform.o misc.o lines.o types.hpp stations.o settings.hpp passengers.o
stations.o: stations.cpp stations.hpp transform.o settings.hpp types.hpp passengers.cpp lines.cpp
lines.o: lines.cpp transform.o misc.o lines.hpp types.hpp settings.hpp passengers.cpp
passengers.o: types.hpp misc.cpp settings.hpp transform.cpp stations.cpp 
transform.o: transform.cpp transform.hpp types.hpp
transform.h:
misc.o: misc.cpp misc.hpp types.hpp
