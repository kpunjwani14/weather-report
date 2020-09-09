/*
Kashish Punjwani
COSC 3360
Due Date: 03/25/2020
Client program that connects with the server, sending it requests for the weather report for a given city
Sources:
https://www.youtube.com/watch?v=eVYsIolL2gE ; series of tutorials discussing socket programming
https://piazza.com/class_profile/get_resource/k4bjn3aw8qc2ov/k728tmpytds3ch ; article by Robert Ingalls 
https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input ; remove '\n' from buffer
https://www.geeksforgeeks.org/problem-with-scanf-when-there-is-fgetsgetsscanf-after-it/ ; how to read '\n' in the buffer left by scanf()
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

void error(char *msg)   //when a system call fails; program is aborted
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    //sockfd is a file descriptor that stores the values returned by the socket system call
    //portno stores the port number that client enters to connect to server
    //n contains the number of chars read or written; if n is less than 0, there is an error in reading or writing
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;   //serv_addr contains address of server to which we want to connect
    struct hostent *server;     //server is a pointer to a structure of type hostent, which defines a host computer on the internet
    char buffer[256];       //client writes city name into the buffer and then reads the data (max temp and sky condition or no data) sent by server

    char hostname[64];  //stores the server host name entered by user
    printf("Enter the server host name: ");     //prompts user for server host name - localhost
    scanf("%s", hostname);
    server = gethostbyname(hostname);   //returns host address structure associated with a given host name
    if (server == NULL)     //if null, system could not locate a host with this name
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    printf("Enter the server port number: ");   //prompts user for port number
    char portnum[10];    //port number should be between 2000 and 65535
    scanf("%s", portnum);   //enter portno on which the server is listening for connections
    portno = atoi(portnum); //portnum is a string, so convert to int
    getchar();  //scanf leaves '\n' in the buffer, so when fgets is used to take the city name input, it reads '\n' instead, so adding getchar() after scanf() takes care of reading '\n'

    sockfd = socket(AF_INET, SOCK_STREAM, 0);   //create socket
    if (sockfd < 0)     //sockfd is a file descriptor that will return -1 if the creation of socket was unsuccessful
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));  //sets all values in buffer to zero
    serv_addr.sin_family = AF_INET;     //sin_family contains a code for the address family; should be set to symbolic constant AF_INET
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);  //because server->h_length is a character string, we use bcopy, which copies length bytes from s1 to s2
    serv_addr.sin_port = htons(portno);     //sin_port contains port number; htons converts portno in host byte order to network byte order

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)    //establishes a connection to the server; returns -1 if unable to connect
        error("ERROR connecting");

    printf("Enter a city name: ");  //prompt user for city name to send to server
    bzero(buffer,256);      //reset buffer
    fgets(buffer,255,stdin);    //store city name in buffer for server to read
    buffer[strcspn(buffer, "\n")] = 0; //remove '\n' from the buffer which is added by fgets when city name is taken as input
    printf("%s", buffer);
    
    n = write(sockfd,buffer,strlen(buffer));   //write city name to socket; the city name written by the client will be read by the server 
    if (n < 0)  //n returns number of characters written; if less than 0, failure in writing to socket
        error("ERROR writing to socket");

    bzero(buffer,256); 
    n = read(sockfd,buffer,255);    //read the corresponding weather data sent by the server (either max temp and sky condition or no data)
    if (n < 0)  //n returns number of characters read; if less than 0, failure in reading from socket
        error("ERROR reading from socket");
    printf("%s\n",buffer);  //in client, print out weather data, which is stored in buffer
    
    close(sockfd);      //terminates TCP connection

    return 0;
}