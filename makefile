all:
	gcc Client.c -lpthread -o Client
	gcc Server.c -lpthread -o Server

clean:
	rm Client Server
