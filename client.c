#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define oops(msg) { perror(msg); exit(1); }

int main(int argc, char* argv[]) {
	int client_sock, choice;
	struct sockaddr_in server_addr;
	char message[BUFSIZ];
	while(1) {
	// check the number of arguments
	if (argc != 3) {
        	fprintf(stderr, "usage %s address port\n", argv[0]);
        	exit(0);
    	}

    	// get a socket
	client_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (client_sock == -1)
		oops("socket");

    	// initialize the address and set IP address and port
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // IPv4             
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP address
	server_addr.sin_port = htons(atoi(argv[2])); // port

	// connect to the server
	if (connect(client_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
		oops("connect");

    	
        	printf("Choose one");
        	printf("\n----------");
        	printf("\n 0. Make a block");
        	printf("\n 1. See all blocks");
        	printf("\n 2. Exit");
        	printf("\n----------");
        	printf("\nInput number: ");
		scanf("%d", &choice);
        	switch(choice) {
        	case 0:
			// write a message to the server
        		printf("Input message: ");
			bzero(message, BUFSIZ);
			scanf(" %[^\n]", message);
			if (write(client_sock, message, BUFSIZ-1) < 0)
				oops("writing socket");
			// read a response from the server
			bzero(message, BUFSIZ);
			if (read(client_sock, message, BUFSIZ-1) < 0)
				oops("read");
			// print the message
			printf("Message from server: %s\n", message);
            		break;
        	case 1:
   	     	case 2:
			exit(0);
        	 	break;
	        default:
			printf("Invalid number\n");
		}	
	}
	return 0;
}
