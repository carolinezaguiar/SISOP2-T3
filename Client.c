// UFRGS - INF01151 Sistemas Operacionais II - 2014/1
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

#define PORT 32000
#define TRUE 1
#define BUFFER_SIZE 256


char inputBuffer[BUFFER_SIZE];
int UUID;
int sockfd,n ;


pthread_t readerHandler;

void *reader(void *arg)
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
                    printf("\n%s > %s",name,inputBuffer);
                    printf( "Enter the message: ");
                    fflush(stdout);
                }
            }
            else
            {
                sscanf(inputBuffer,"S#%[^\t\n]",control);
                printf("\nSERVER > %s\n",control);
                printf( "Enter the message: ");
                fflush(stdout);

            }
                

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
        printf("ERROR connecting\n");
    
    else
    {
        
        if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)
        {
            printf("My ID is %s \n", inputBuffer);
            UUID = atoi(inputBuffer);

            if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)   //Welcome message
            {
                printf("%s \n",inputBuffer);

                pthread_create(&readerHandler,NULL,reader,NULL);
            }
        }

    while(TRUE)
    {
        printf("Enter the message: ");
        bzero(outputBuffer,BUFFER_SIZE);
        fgets(outputBuffer,BUFFER_SIZE,stdin);
        
        if(strlen(outputBuffer) > 1) //No empty messages :)
            write(sockfd,outputBuffer,BUFFER_SIZE);   

        if(outputBuffer[0] == '/' && outputBuffer[1] == 'q')
            break;
    }
    }
	close(sockfd);
    return 0;
}
