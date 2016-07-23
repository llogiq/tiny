CC 			= clang
CC_FLAGS 	= -std=gnu11 -Wall -Wpedantic -Wextra -O0 -g

objs = main.o textfield.o textarea.o event_loop.o server_comms.o tui.o

tiny: $(objs)
	$(CC) $^ -o $@ -g -lncurses

%.o: src/%.c
	$(CC) -Iinclude $^ -c -o $@ $(CC_FLAGS)

.PHONE: clean

clean:
	rm -f tiny
	rm -f *.o
