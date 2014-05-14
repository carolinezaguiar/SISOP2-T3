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

#define PORT 32000
#define TRUE 1  //You know why...
#define BUFFER_SIZE 256


char inputBuffer[BUFFER_SIZE];
int UUID;
int sockfd,n;

static WINDOW *wInput, *wOutput;
char mesg[] = "Enter the message: ";

int lineIndex;
pthread_t readerHandler;
WINDOW *create_newwin(int height, int width, int starty, int startx);

void *reader(void)
{
	
    char name[25], control[BUFFER_SIZE];
    while(TRUE)
    {
        if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)
        {
            if(inputBuffer[0] == 'U')
            {
				sscanf(inputBuffer,"U#%[^\t\n]",name);
				if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)
                {
					if(lineIndex < LINES-7)
					{
						wattron(wOutput,COLOR_PAIR(1));
						mvwprintw(wOutput,lineIndex+1,1,"%s > %s",name,inputBuffer);
						wattroff(wOutput,COLOR_PAIR(1));
						lineIndex++;
					}
					else //Time to scroll :)
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
            else
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
    if (argc < 2) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }
	
	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
        printf("ERROR connecting\n");
		close(sockfd);
		exit(1);
		
	}
		
	//Lets create the windows shall we?
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
	
	mvwprintw(wInput,2,1,mesg);
	wrefresh(wInput);
	
	mvwprintw(wInput,2,1,mesg);
	wrefresh(wInput);
	
	if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)
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
		
		mvwprintw(wInput,2,1,mesg);
		wclrtoeol(wInput);
		wrefresh(wInput);
		        
        if(strlen(outputBuffer) > 1) //No empty messages :)
		{
			outputBuffer[strlen(outputBuffer)]='\n';
            write(sockfd,outputBuffer,BUFFER_SIZE);   
		}

        if(outputBuffer[0] == '/' && outputBuffer[1] == 'q')
            break;
    }
    endwin();
	close(sockfd);
    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}