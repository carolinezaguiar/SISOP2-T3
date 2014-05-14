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
#define MAX_ROOMS 4


int connectedClients[MAX_CLIENTS], socks[MAX_CLIENTS], rooms[MAX_ROOMS][MAX_CLIENTS];	//Define the client list and its sockets
pthread_t clientHandler[MAX_CLIENTS];					//Or lovely handler
pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;


void *socket_threads(void *UUID) //Handles the client itself :)
{
   
	char buffer[BUFFER_SIZE], control[BUFFER_SIZE];
	int id = (int)UUID;
    int i;
	int my_room = 0;	//lobby
	rooms[my_room][id] = 1;
    char name[25] = "unamed";
	
	while(TRUE)
	{
    	if( read(socks[id],buffer,BUFFER_SIZE) >0 )
	    {	
            buffer[BUFFER_SIZE-1] = '\0';

            if(buffer[0] != '/')    //Pure text message
            {
				if(strcmp(name,"unamed") == 0) //Ensures that the client is IDed
				{
					sprintf(control,"S#Please change your name first with /n");
					write(socks[id],control,BUFFER_SIZE);
				}
				else
				{
					printf("Message from %d: %s", id, buffer);	
					sprintf(control,"U#%s",name); //User message# User Name
					
					for(i = 0; i<MAX_CLIENTS; i++)
					{
						if(i != id && socks[i] != -1 && rooms[my_room][i] ==1 )
						{
							write(socks[i],control,BUFFER_SIZE);
							write(socks[i],buffer,BUFFER_SIZE);
							
						}
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
                    case 'j': 
                        
						pthread_mutex_lock(&mutexLock);
						rooms[my_room][id] = -1;
                        sscanf(buffer,"%*s %d", &my_room);
                        printf("Room changed to %d\n",my_room);
						rooms[my_room][id] = 1;					
						sprintf(control,"S#Welcome to room #%d",my_room);
						write(socks[id],control,BUFFER_SIZE);						
						pthread_mutex_unlock(&mutexLock);
                        break;
						
                    case 'l': 
						pthread_mutex_lock(&mutexLock);
                        printf("Left to looby\n" );
						rooms[my_room][id] = -1;
						my_room = 0;
						rooms[my_room][id] = 1;
						sprintf(control,"S#You are back in the lobby");
						write(socks[id],control,BUFFER_SIZE);
						pthread_mutex_unlock(&mutexLock);
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
						pthread_mutex_lock(&mutexLock);	//Ensures that no one else is reading/writing
                        socks[id] = -1;
						rooms[my_room][id] = -1;
						pthread_mutex_unlock(&mutexLock);
                        pthread_exit((void*) 0);
                        break;
                    case 'h':
						sprintf(control,"S#Commands: /n <name> /j <room_number> /l back to lobby /q quit");
						write(socks[id],control,BUFFER_SIZE);
						break;
                    default:
                        printf("Not recognized\n" );
						sprintf(control,"S#Command not recognized, type /h for help");
						write(socks[id],control,BUFFER_SIZE);
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
    char welcome[255] = "Welcome, please type /n <name> to change your name or /h for help!";
	
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

    int i,j;
	for(i =0; i<MAX_ROOMS;i++)
		for(j=0;j<MAX_CLIENTS;j++)
			rooms[i][j] = -1;
	
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
