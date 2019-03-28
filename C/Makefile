all:
	gcc -g -c -o socket_library.o socket_library.c -lpthread
	gcc -g -o client.out client.c socket_library.o -lpthread
	gcc -g -o server.out server.c socket_library.o -lpthread
	rm *.o
clean: 
	rm *.out
