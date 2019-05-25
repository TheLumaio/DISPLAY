SRC = src/*.c
OUT = DISPLAY.EXE

all:
	gcc -o $(OUT) $(SRC) -std=c99 -static -lCello -lraylib -lopengl32 -lgdi32 -limagehlp -lwinmm

run: all
	./$(OUT)
