CC = /mnt/c/msys64/mingw64/bin/g++.exe
CXXFLAGS =   -Iinclude/SDL2 -lmingw32 -lSDL2main -lSDL2 -mwindows -mconsole -Llib/ 

all: main.o
	$(CC) -o build/game.exe build/main.o $(CXXFLAGS)

main.o:  
	$(CC) -c -o build/main.o ./src/main.cpp $(CXXFLAGS) 

