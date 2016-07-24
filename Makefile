CC 			= clang
CC_FLAGS 	= -std=gnu11 -Wall -Wpedantic -Wextra -O0 -g

objs = textfield.o textarea.o event_loop.o server_comms.o tui.o
main_objs = $(objs) main.o

tiny: $(main_objs)
	$(CC) $^ -o $@ -g -lncurses

test.o: test/main.c
	$(CC) -Iinclude $^ -c -o $@ $(CC_FLAGS)

tiny_test: server_comms.o test.o
	$(CC) $^ -o $@ -g

%.o: src/%.c
	$(CC) -Iinclude $^ -c -o $@ $(CC_FLAGS)

.PHONE: clean

clean:
	rm -f tiny
	rm -f tiny_test
	rm -f *.o

.PHONY: tags
tags:
	ctags-exuberant **/*.h **/*.c

.PHONY: test
test: tiny_test
	./tiny_test
