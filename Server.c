#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define TRUE 1 //Dam no TRUE on C...

#define PORT 32000
#define MAX_CLIENTS 100
#define BUFFER_SIZE 256


int connectedClients[MAX_CLIENTS], socks[MAX_CLIENTS];	//Define the client list and its sockets
pthread_t clientHandler[MAX_CLIENTS];					//Or lovely handler

void *socket_threads(void *UUID) //Handles the client itself :)
{
   
	char buffer[BUFFER_SIZE], control[BUFFER_SIZE];
	int id = (int)UUID;
    int i;
    char name[25] = "unamed";
	while(TRUE)
	{
    	if( read(socks[id],buffer,BUFFER_SIZE) >0 )
	    {	
            buffer[BUFFER_SIZE-1] = '\0';

            if(buffer[0] != '/')    //Pure text message
            {

		        printf("Message from %d: %s", id, buffer);	
                sprintf(control,"U#%s",name); //User message# User Name

                for(i = 0; i<MAX_CLIENTS; i++)
                {
                    if(i != id && socks[i] != -1)
                    {
                        write(socks[i],control,BUFFER_SIZE);
                        write(socks[i],buffer,BUFFER_SIZE);
                    }
                }
            }
            else //Control Message
            {
                printf("Control Message\n");
                int new_room;
                switch( buffer[1])
                {
                    case 'n':
                        sscanf(buffer,"%*s %[^\t\n]",name);
                        printf("Name changed to %s\n",name );
                        break;
                    case 'j':   //TODO: Complete this
                        
                        sscanf(buffer,"%*s %d", &new_room);
                        printf("Room changed to %d\n",new_room);
                        break;
                    case 'l':   //TODO: Complete this
                        printf("Left to looby\n" );
                        break;
                    case 'q':
                        for(i = 0; i<MAX_CLIENTS; i++)
                        {
                            if(i != id && socks[i] != -1)
                            {
                                sprintf(control,"S#%s rage quited!",name);
                                write(socks[i],control,BUFFER_SIZE);
                            }
                        }
                        socks[id] = -1;
                        pthread_exit(0);
                        break;
                       
                    default:
                        printf("Not recognized\n" );
                        break;

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
    char welcome[255] = "Welcome, plese type /n <name> to change your name :)";
	
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
            write(socks[UUID],UUIDCHAR,sizeof(UUIDCHAR));
 	        
            
            write(socks[UUID],welcome,sizeof(welcome));
    	    
            pthread_create(&clientHandler[UUID],NULL,socket_threads,(void *)UUID); //FIX ME :)
	        UUID++;
        }
        
		
	}
	
	close(sockfd);


	return 0; 
	}
