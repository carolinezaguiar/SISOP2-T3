// UFRGS - INF01151 Sistemas Operacionais II - Trabalho III - 2014/1
// Caroline de Aguiar and Juliano Franz

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <ncurses.h>
#include "Client.h"

char inputBuffer[BUFFER_SIZE];
int UUID;
int sockfd,n;

static WINDOW *wInput, *wOutput;
char msg[] = "Enter the message: ";
int lineIndex;

pthread_t readerHandler;
pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;

WINDOW *create_newwin(int height, int width, int starty, int startx);

void *reader(void *arg)
{
	char name[25];
	char control[BUFFER_SIZE];
	while(TRUE)
	{
		// TCP: READ new message from server
		if(read(sockfd,inputBuffer,BUFFER_SIZE) > 0)
		{
			if(inputBuffer[0] == 'U') // Message from another client of the same room
			{
				sscanf(inputBuffer,"U#%[^\t\n]",name);
				if(read(sockfd,inputBuffer,BUFFER_SIZE) > 0)
				{
					if(lineIndex < LINES-7)
 					{
 						wattron(wOutput,COLOR_PAIR(1));
 						mvwprintw(wOutput,lineIndex+1,1,"%s > %s",name,inputBuffer);
 						wattroff(wOutput,COLOR_PAIR(1));
 						lineIndex++;
 					}
 					else // Scroll
 					{
 						wattron(wOutput,COLOR_PAIR(1));
 						mvwprintw(wOutput,LINES-7,1,"%s > %s",name,inputBuffer);
 						wattroff(wOutput,COLOR_PAIR(1));
 						wclrtoeol(wOutput);
 						scroll(wOutput);
 						box(wOutput, 0 , 0);
 					}
            }
			}
         else // Message from the server itself
         {
         	sscanf(inputBuffer,"S#%[^\t\n]",control);
				if(lineIndex < LINES-7)
 				{
 					wattron(wOutput,COLOR_PAIR(2));
 					mvwprintw(wOutput,lineIndex+1,1,"SERVER > %s",control);
 					wattroff(wOutput,COLOR_PAIR(2));
 					lineIndex++;
 				}
 				else //Time to scroll :)
 				{
 					wattron(wOutput,COLOR_PAIR(2));
 					mvwprintw(wOutput,LINES-7,1,"SERVER > %s",control);
 					wattroff(wOutput,COLOR_PAIR(2));
 					wclrtoeol(wOutput);
 					scroll(wOutput);
 					box(wOutput, 0 , 0);
 				}
			}                
			wrefresh(wOutput);
		}
	}
}

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
   char outputBuffer[BUFFER_SIZE];
	char outputBufferErr[BUFFER_SIZE];
	strcpy(outputBufferErr, "ERROR: Message is too big");
	//outputBufferErr[strlen(outputBufferErr)]='\n';

   if (argc < 2)
	{
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
   }
	
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
   }
    
	// TCP: CREATE SOCKET
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ 
        printf("ERROR opening socket\n");
	}
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	

	// TCP: CONNECT
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
		printf("ERROR connecting\n");
		close(sockfd);
		exit(1);		
	}
		
	// Create windows
	initscr();	//Init nCurses
	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	wInput = create_newwin(4, COLS, LINES-5, 0);
	wOutput = create_newwin(LINES-5,COLS,0,0);

	start_color();	
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_WHITE);
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
   scrollok(wOutput,TRUE);
	
	lineIndex = 0;
	
	mvwprintw(wInput,2,1,msg);
	wrefresh(wInput);
	
	mvwprintw(wInput,2,1,msg);
	wrefresh(wInput);

	// TCP: READ initial messages from the server
	if(read(sockfd,inputBuffer,BUFFER_SIZE) > 0) // Welcome message
	{
		wattron(wOutput,COLOR_PAIR(2));
 		mvwprintw(wOutput,lineIndex+1,1,"My ID is %s",inputBuffer);
 		wattroff(wOutput,COLOR_PAIR(2));
 		lineIndex++;

 		UUID = atoi(inputBuffer);

 		if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)   //Welcome message
 		{
 			wattron(wOutput,COLOR_PAIR(2));
 			mvwprintw(wOutput,lineIndex+1,1,"%s",inputBuffer);
 			wattroff(wOutput,COLOR_PAIR(2));
 			lineIndex++;
 			wrefresh(wOutput);
 			pthread_create(&readerHandler,NULL,reader,NULL);
 		}
	}

   while(TRUE)
   {
		bzero(outputBuffer,BUFFER_SIZE);
		wgetstr(wInput,outputBuffer);
		
		mvwprintw(wInput,2,1,msg);
		wclrtoeol(wInput);
		wrefresh(wInput);
		        
		// New Message
 		if(strlen(outputBuffer) > 1)
		{
			outputBuffer[strlen(outputBuffer)]='\n';
			// TCP: WRITE message
			if (strlen(outputBuffer) > COLS-strlen(msg)-4) 
			{
				if(lineIndex < LINES-7)
				{
					wattron(wOutput,COLOR_PAIR(1));
					mvwprintw(wOutput,lineIndex+1,1,"SERVER > %s",outputBufferErr);
					wattroff(wOutput,COLOR_PAIR(1));
					lineIndex++;
				}
				else // Scroll
				{
					wattron(wOutput,COLOR_PAIR(1));
	        		mvwprintw(wOutput,LINES-7,1,"SERVER > %s",outputBufferErr);
					wattroff(wOutput,COLOR_PAIR(1));
					wclrtoeol(wOutput);
					scroll(wOutput);
					box(wOutput, 0 , 0);
				}
            wrefresh(wOutput);
			}	
			else
			{
				write(sockfd,outputBuffer,BUFFER_SIZE);
				pthread_mutex_lock(&mutexLock);
		     	if(lineIndex < LINES-7)
				{
					wattron(wOutput,COLOR_PAIR(1));
					mvwprintw(wOutput,lineIndex+1,1,"Me > %s",outputBuffer);
					wattroff(wOutput,COLOR_PAIR(1));
					lineIndex++;
				}
				else // Scroll
				{
					wattron(wOutput,COLOR_PAIR(1));
	        		mvwprintw(wOutput,LINES-7,1,"Me > %s",outputBuffer);
					wattroff(wOutput,COLOR_PAIR(1));
					wclrtoeol(wOutput);
					scroll(wOutput);
					box(wOutput, 0 , 0);
				}
				wrefresh(wOutput);
		      pthread_mutex_unlock(&mutexLock);
			}

		}

      if(outputBuffer[0] == '/' && outputBuffer[1] == 'q')
		{
      	break;
		}
	}

	endwin();

	// TCP: CLOSE SOCKET
	close(sockfd);
   
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win,0,0);	// 0,0: default chars for the vertical and horizontal lines
	wrefresh(local_win); // Shows the box
	return local_win;
}
