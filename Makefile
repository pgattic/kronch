
.PHONY: all clean install

all:
	gcc src/main.c -o kronch -lraylib -lGL -lm -lpthread -ldl -lrt
	chmod +x kronch

clean:
	rm kronch

install:
	mv kronch /usr/local/bin/

