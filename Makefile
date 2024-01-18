
.PHONY: all clean

all:
	gcc main.c -o kronch -lraylib -lGL -lm -lpthread -ldl -lrt

clean:
	rm kronch


