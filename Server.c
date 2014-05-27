// UFRGS - INF01151 Sistemas Operacionais II - 2014/1
// Caroline de Aguiar and Juliano Franz

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "Server.h"

// Client list and its sockets
int socks[MAX_CLIENTS];
int rooms[MAX_ROOMS][MAX_CLIENTS];

pthread_t clientHandler[MAX_CLIENTS];
pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;


// Concurrent server: Handles each client separated
void *socket_threads(void *UUID)
{
   char buffer[BUFFER_SIZE];
   char control[BUFFER_SIZE];
   int id = (int) UUID;
	int i;
	int my_room = LOBBY;
   int new_room;
	rooms[my_room][id] = 1;
   char userName[25] = "unamed";
	
	while (TRUE)
	{
		if (read(socks[id],buffer,BUFFER_SIZE) > 0)
	    	{		
				buffer[BUFFER_SIZE-1] = '\0';

				if(buffer[0] != '/')    //Pure text message
            {
					if(strcmp(userName,"unamed") == 0) // Check if client has ID
					{
						sprintf(control,"S#Please change your name first with /n");
						write(socks[id],control,BUFFER_SIZE);
					}
					else
					{ 
						printf("Message from %d: %s", id, buffer);	
						sprintf(control,"U#%s",userName); //User message# User Name
					
						// Replicate message to all clients of the same room
						for(i = 0; i<MAX_CLIENTS; i++)
						{
							if(i != id && socks[i] != -1 && rooms[my_room][i] == 1 )
							{
								write(socks[i],control,BUFFER_SIZE);
								write(socks[i],buffer,BUFFER_SIZE);
							}
						}
					}
            }
            else
            {
                printf("Control Message\n");
                switch (buffer[1])
                {
                    case 'n': // Change name
                        sscanf(buffer,"%*s %[^\t\n]",userName);
                        printf("Name changed to %s\n",userName);
                        break;

                    case 'j': // Join a room
								sscanf(buffer,"%*s %d", &new_room);
								if ((new_room <= LOBBY ) || (new_room >= MAX_ROOMS) || (new_room == my_room))
								{
									sprintf(control,"S#Invalid room number#%d",new_room);
									write(socks[id],control,BUFFER_SIZE);
								}
								else
								{
									pthread_mutex_lock(&mutexLock);
									rooms[my_room][id] = -1;
									my_room = new_room;
		                     printf("Room changed to %d\n",my_room);
									rooms[my_room][id] = 1;					
									sprintf(control,"S#Welcome to room #%d",my_room);
									write(socks[id],control,BUFFER_SIZE);						
									pthread_mutex_unlock(&mutexLock);
								}
                        break;
						
                    case 'l': // Leave a room
								if (my_room == LOBBY)
								{
									sprintf(control,"S#Already in the lobby");
									write(socks[id],control,BUFFER_SIZE);
								}
								else 
								{
									pthread_mutex_lock(&mutexLock);
		                     printf("Left to looby\n" );
									rooms[my_room][id] = -1;
									my_room = 0;
									rooms[my_room][id] = 1;
									sprintf(control,"S#You are back in the lobby");
									write(socks[id],control,BUFFER_SIZE);
									pthread_mutex_unlock(&mutexLock);
								}
                        break;

                    case 'q': // Quit the chat
                        for(i = 0; i<MAX_CLIENTS; i++)
                        {
                            if(i != id && socks[i] != -1)
                            {
                                sprintf(control,"S#%s rage quited!",userName);
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
	struct sockaddr_in serv_addr, cli_addr; // Initially, only two sockets
   char welcome[255] = "Welcome, please type /n <name> to change your name or /h for help!";
	
	// TCP: CREATE SOCKET
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
        printf("ERROR opening socket\n");
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	// TCP: BIND
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR on binding\n");
	}

	int UUID = 0;
	char UUIDCHAR[BUFFER_SIZE];
   int i,j;

	// Initialize rooms
	for(i =0; i<MAX_ROOMS;i++)
	{
		for(j=0;j<MAX_CLIENTS;j++)
		{
			rooms[i][j] = -1;
		}
	}
	// Initialize clients
   for (i = 0; i<MAX_CLIENTS;i++)
   {
   	socks[i] = -1;
   }

   printf("SV OK\n" );

	// TCP: LISTEN
   listen(sockfd,MAX_PENDING_CONNECTIONS);

	while(TRUE)
	{
		// TCP: ACCEPT
		if((socks[UUID] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
		{
	            printf("ERROR on accept new client\n");
		}
      else
      {
			printf("Client connected!\n");
			sprintf(UUIDCHAR,"%d",(int)UUID);
			write(socks[UUID],UUIDCHAR,sizeof(UUIDCHAR));            
         write(socks[UUID],welcome,sizeof(welcome));
			// Concurrent server: Create one thread for each client
			pthread_create(&clientHandler[UUID],NULL,socket_threads,(void*)UUID);
	      UUID++;
		}
	}
	
	// TCP: CLOSE SOCKET
	close(sockfd);

	return 0; 
}
