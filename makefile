all:
	gcc -Wall server.c -o server -lpthread 
	gcc -Wall client.c -o client -lpthread -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
clean: 
	rm client
	rm server