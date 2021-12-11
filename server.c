#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define oops(msg) { perror(msg); exit(1); }

int main(int argc, char* argv[]) {    
    	int server_sock;
    	int client_sock;
    
    	struct sockaddr_in server_addr;
    	struct sockaddr_in client_addr;
	socklen_t client_addr_size;
   
    	// get a socket
    	server_sock = socket(PF_INET, SOCK_STREAM, 0); 
   	if (server_sock == -1)
		oops("socket");
    
    	// initialize the address and set IP address and port
    	memset(&server_addr, 0, sizeof(server_addr)); 
    	server_addr.sin_family = AF_INET; // IPv4
    	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // IP address
    	server_addr.sin_port = htons(atoi(argv[1])); // port
    
    	// bind socket and the server address
    	if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) != 0)
		oops("bind");
    
    	// allow incoming calls with Qsize = 1 on socket 
    	if (listen(server_sock, 1) != 0)
    		oops("listen");
    
   	// main loop
    	while (1) {
    		// wait for a call
		client_addr_size = sizeof(client_addr);
    		client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_addr_size);
    		if (client_sock == -1)
        		oops("accept");
	    	// write a message to client
		char msg[] = "hello this is server";
        	write(client_sock, msg, sizeof(msg));
	    	// close socket
	    	close(client_sock);
		return 0;
	}
}

