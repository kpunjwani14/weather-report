/*
Kashish Punjwani
COSC 3360
Due Date: 03/25/2020
Server program that waits for connection requests: receives a city name from client and returns the max temperature and sky condition corresponding to that city
Sources:
https://www.youtube.com/watch?v=eVYsIolL2gE ; series of tutorials discussing socket programming
https://piazza.com/class_profile/get_resource/k4bjn3aw8qc2ov/k728tmpytds3ch ; article by Robert Ingalls 
https://stackoverflow.com/questions/15091284/read-comma-separated-input-with-scanf ; read comma-separated input
https://www.programiz.com/c-programming/c-file-input-output ; open/read a file in C
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>      
#include <sys/socket.h>     
#include <netinet/in.h>   
#include <unistd.h>  

void error(char *msg)   //when a system call fails; program is aborted
{
    perror(msg);
    exit(1);
}

struct node     //data in the input file will be stored in a linked list; each node of the linked list contains the following information: city name, max temp, and sky conditions
{
    char city[64];
    char maxTemp[5];
    char skyCondition[50];
    struct node* next;
};

int main(int argc, char *argv[])
{
    //sockfd and newsockfd are file descriptors that store the values returned by the socket system call
    //portno stores the port number on which server accepts connections from client
    //clilen stores the size of the address of the client
    //n contains the number of chars read or written; if n is less than 0, there is an error in reading or writing
    int sockfd, newsockfd, portno, clilen, n;   
    char buffer[256];   //server reads characters from the socket connection into the buffer, in this case, city name
    struct sockaddr_in serv_addr, cli_addr;     //serv_addr contains address of server, cli_addr contains address of client which seeks to connect to server

    //reading input from file to a linked list
    FILE* fptr;
    if ((fptr = fopen("weather20.txt","r")) == 0)   //open 'weather20.txt' to read city name, max temp, and sky condition
    {
        printf("Error! opening file");
        exit(1);
    }
    char line[50];  //for storing each line in input file (city, max condition, sky temperature)
    struct node* head=(struct node*)malloc(sizeof(struct node));    //head node is a dummy node, head->next contains the first city, maxTemp, skyCondition
    struct node* cu=head;
    while((fgets(line, 50, fptr))!=0)    //while there is a line in the input line
    {
        struct node* temp=(struct node*)malloc(sizeof(struct node));    //create a node for each city in input file
        //read at most (64,5,50) characters or until ',' or '\n' is encountered; to the left of the first ',' is the city name, store it in the node's city field; do the same for max temp and sky conditions
        sscanf(line,"%64[^,],%5[^,],%50[^\n]", temp->city, temp->maxTemp, temp->skyCondition);     
        temp->next=0;   //have the new node point to null
        cu->next=temp;  //make previous node point to this newly created node
        cu=temp;        //set cu equal to new node
    }
    fclose(fptr);

    //prints linked list containing the data in input file
    // cu=head->next;
	// while (cu)
	// {
	// 	printf("%s %s %s\n", cu->city, cu->maxTemp, cu->skyCondition);
	// 	cu=cu->next;
	// }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);   //create a TCP socket, which will create an endpoint for a network connection
    if (sockfd < 0)     //sockfd is a file descriptor that will return -1 if the creation of socket was unsuccessful
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));  //sets all values in buffer to zero

    printf("Enter server port number: ");   //user passes in the port number on which server will listen for connections from clients
    char portnum[10];    //port number should be between 2000 and 65535
    scanf("%s", portnum);
    portno = atoi(portnum);     //portnum is a string, so convert to int

    serv_addr.sin_family = AF_INET;     //sin_family contains a code for the address family; should be set to symbolic constant AF_INET
    serv_addr.sin_port = htons(portno); //sin_port contains port number; htons converts portno in host byte order to network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY; //s_addr contains IP address of host; INADDR_ANY gets IP address of the machine on which server is running

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)    //bind an address to socket: a server process calls bind to attach itself to a specified port number
        error("ERROR on binding");
    
    //server process calls listen to tell the kernel to initialize a wait queue of connections for this socket; have a queue of 5 clients at max
    //if more than 5 clients come in at once, the server is going to put only 5 of them in line, the others will have to reconnect at another time
    listen(sockfd,5);   

    while(1)    //keep the server running; continuously accepts client requests and subsequently reads and writes; use SIGINT (ctrl+c) to terminate server side
    {   
        clilen = sizeof(cli_addr);  //size of the client structure
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);    //accept new connections from clients trying to connect to server
        if (newsockfd < 0)
            error("ERROR on accept");
        
        //code below will execute iff client has successfully connected to the server; server and client can now exchange data
        bzero(buffer,256);  //initialize buffer
        n = read(newsockfd,buffer,255);     //read city name sent by the client in the buffer
        if (n < 0)  //n returns number of characters read; if less than 0, failure in reading from socket
            error("ERROR reading from socket");

        printf("Weather report for %s\n", buffer);  //print weather report for city in the buffer

        cu=head->next;  //head->next contains the contents of the first city in input file
        while(cu!=0)    //iterate through all the nodes of linked list
        {
            if (strcmp(buffer, cu->city) == 0)  //if city in the buffer is equal to the current node's city, print next day temp and sky condition on the screen
            {   
                printf("Tomorrow’s maximum temperature is %s F\n", cu->maxTemp);
                printf("Tomorrow’s sky condition is %s\n", cu->skyCondition);

                bzero(buffer, 256);     //reset the buffer
                sprintf(buffer, "Tomorrow’s maximum temperature is %s F\nTomorrow’s sky condition is %s", cu->maxTemp, cu->skyCondition);  //send to client a SINGLE MESSAGE with requested data (i.e. store into the buffer the max temp and sky condition for client to read)
                break;  //if match is found (i.e. if city exists in linked list), break out of while loop
            } 
            if(!cu->next)     //at this point, we're at the last node and a match still hasn't been found (i.e. we have finished traversing through all the nodes (city names) and the requested data for a city doesn't exist)
            {
                printf("No data\n");    //subsequently, print 'no data' on the screen
                bzero(buffer, 256);     //reset the buffer
                sprintf(buffer, "No data");     //store 'No data' into buffer for client to read
            }
            cu=cu->next;    //keep traversing through linked list until no more nodes (cu==0)
        }

        n = write(newsockfd,buffer,strlen(buffer));     //anything written by the server will be read by the client, so client will read the max temp and sky conditions if data for requested city exists
        if (n < 0)      //n returns number of characters written; if less than 0, failure in writing to socket
            error("ERROR writing to socket");
        
        close(newsockfd);   //terminates TCP connection
    }
    return 0; 
}