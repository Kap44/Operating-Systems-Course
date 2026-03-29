# Ioannis Kapetanakis - CSD4641

clean: 
	rm -f *.o shell

all: shell

shell: main.o basic_funcs.o handle_funcs.o
	gcc main.o basic_funcs.o handle_funcs.o -o shell

main.o: main.c basic_funcs.h
	gcc -c main.c

basic_funcs.o: basic_funcs.c basic_funcs.h
	gcc -c basic_funcs.c

handle_funcs.o: handle_funcs.c basic_funcs.h handle_funcs.h
	gcc -c handle_funcs.c
