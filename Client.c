#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>

#define PORT 5690
#define TRUE 1  //You know why...
#define BUFFER_SIZE 256


char inputBuffer[BUFFER_SIZE];
int UUID;
int sockfd,n ;

pthread_t readerHandler;

void *reader(void)
{
    
    while(TRUE)
    {
        if(read(sockfd,inputBuffer,BUFFER_SIZE) >0)
        {
            printf("\nMessage from X > %s",inputBuffer);
            printf( "Enter the message: ");  //TODO: Why this no Work? o.O
            fflush(stdout);
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
    read(sockfd,inputBuffer,BUFFER_SIZE);
    printf("My ID is %s \n", inputBuffer);
    UUID = atoi(inputBuffer);
    pthread_create(&readerHandler,NULL,reader,NULL);

    while(TRUE)
    {
        printf("Enter the message: ");
        bzero(outputBuffer,BUFFER_SIZE);
        fgets(outputBuffer,BUFFER_SIZE,stdin);
        write(sockfd,outputBuffer,BUFFER_SIZE);   
    }
    }
	close(sockfd);
    return 0;
}
