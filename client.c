#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define oops(msg) { perror(msg); exit(1); }

void sig_handler(int sig) {
	signal(sig, SIG_IGN);
}

int main(int argc, char* argv[]) {
	int client_sock, choice;
	struct sockaddr_in server_addr;
	char message[BUFSIZ], choice_s[BUFSIZ];
	
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);
	signal(SIGQUIT, sig_handler);

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
        	printf("\n 1. Make a block");
        	printf("\n 2. See all blocks");
        	printf("\n 3. Exit");
        	printf("\n----------");
        	printf("\nInput number: ");
		scanf("%s", choice_s);
		if (write(client_sock, choice_s, BUFSIZ-1) < 0)
			oops("writing socket");
        	choice = atoi(choice_s);
		switch(choice) {
        	case 1:
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
			printf("Message from server: %s", message);
            		break;
        	case 2:
			// read a response from the server
			bzero(message, BUFSIZ);
			if (read(client_sock, message, BUFSIZ-1) < 0)
				oops("read");
			// print the message
			printf("Message from server: %s", message);
            		break;
   	     	case 3:
			exit(0);
        	 	break;
	        default:
			printf("Invalid input\n");
		}
		printf("**********\n");
	}
	return 0;
}
