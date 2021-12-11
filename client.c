#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define oops(msg) { perror(msg); exit(1); }

int main(int argc, char* argv[]) {
	int client_sock;
	struct sockaddr_in server_addr;
	char message[BUFSIZ];

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

	// read a message from the server
	if (read(client_sock, message, sizeof(message)-1) == -1)
		oops("read");
	
	// print the message
	printf("Message from server: %s\n", message);

	// close socket
	close(client_sock);
	
	return 0;
}
