# Ioannis Kapetanakis - csd4641

clean: 
	rm -f *.o ex2

all: ex2

ex2: main.o transportation_system.o
	gcc main.o transportation_system.o -pthread -o ex2

main.o: main.c transportation_system.h
	gcc -c main.c -pthread

transportation_system.o: transportation_system.c transportation_system.h
	gcc -c transportation_system.c -pthread