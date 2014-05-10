#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define TRUE 1 //Dam no TRUE on C...

#define PORT 4000
#define MAX_CLIENTS 100
#define BUFFER_SIZE 256


int connectedClients[MAX_CLIENTS], socks[MAX_CLIENTS];	//Define the client list and its sockets
pthread_t clientHandler[MAX_CLIENTS];					//Or lovely handler

void *socket_threads(void *UUID) //Handles the client itself :)
{
	
	char buffer[BUFFER_SIZE];
	
	while(TRUE)
	{
	if( read(socks[0],buffer,BUFFER_SIZE) >0 )
	{	
		buffer[BUFFER_SIZE-1] = '\0';
		printf("Message from %d: %s", (int)UUID, buffer);	
		//write(socks[0], buffer,BUFFER_SIZE);//TODO fix distribution :)
	}
	}
	

}


int main(int argc, char *argv[])
{
	int sockfd;
	socklen_t clilen;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	int UUID = 0;
	char UUIDCHAR[BUFFER_SIZE];	
	

	while(TRUE)
	{
		
		listen(sockfd, 5); 
		if((socks[0] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
	            printf("ERROR on accept");
	            //TODO: Decrement i if not accepted
	    printf("Client connected, yeah!\n");
	    
	    
	    sprintf(UUIDCHAR,"%d",(int)UUID);
	    write(socks[0],UUIDCHAR,sizeof(int));	
	    //pthread_create(&clientHandler[UUID],NULL,socket_threads,(void *)UUID); //FIX ME :)
	    UUID++;
		
	}
	
	close(sockfd);


	return 0; 
	}
