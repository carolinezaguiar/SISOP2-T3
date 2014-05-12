#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define TRUE 1 //Dam no TRUE on C...

#define PORT 5690
#define MAX_CLIENTS 100
#define BUFFER_SIZE 256


int connectedClients[MAX_CLIENTS], socks[MAX_CLIENTS];	//Define the client list and its sockets
pthread_t clientHandler[MAX_CLIENTS];					//Or lovely handler

void *socket_threads(void *UUID) //Handles the client itself :)
{
   
	char buffer[BUFFER_SIZE];
	int id = (int)UUID;
    int i;
	while(TRUE)
	{
	if( read(socks[id],buffer,BUFFER_SIZE) >0 )
	{	
	    buffer[BUFFER_SIZE-1] = '\0';
		printf("Message from %d: %s", id, buffer);	
        for(i = 0; i<MAX_CLIENTS; i++)
        {
            if(i != id && socks[i] != -1)
            {
                write(socks[i],buffer,BUFFER_SIZE);
            }
        }


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
        printf("ERROR opening socket\n");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding\n");
	int UUID = 0;
	char UUIDCHAR[BUFFER_SIZE];	

    int i;
    for (i = 0; i<MAX_CLIENTS;i++)
    {
        socks[i] = -1;
    }

    printf("SV OK\n" );

    listen(sockfd,5);

	while(TRUE)
	{
		
		if((socks[UUID] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
	            printf("ERROR on accept\n");
	            //TODO: Decrement i if not accepted
        
        else
        {
	    printf("Client connected, yeah!\n");
	    sprintf(UUIDCHAR,"%d",(int)UUID);
	    write(socks[UUID],UUIDCHAR,sizeof(int));	
	    pthread_create(&clientHandler[UUID],NULL,socket_threads,(void *)UUID); //FIX ME :)
	    UUID++;
        }
        
		
	}
	
	close(sockfd);


	return 0; 
	}
