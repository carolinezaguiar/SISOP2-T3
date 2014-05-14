all:
	gcc Client.c -lpthread -o Client
	gcc Server.c -lpthread -o Server
	gcc ClientGUI.c -lpthread -lncurses -o ClientGUI

clean:
	rm Client Server ClientGUI
