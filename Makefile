
.PHONY: all clean install

all:
	gcc src/main.c -o kronch -lraylib -lGL -lm -lpthread -ldl -lrt
	chmod +x kronch

clean:
	rm kronch

install:
	mkdir "${HOME}/bin" -p
	mv kronch "${HOME}/bin/"

