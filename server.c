#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#define oops(msg,n) { perror(msg); exit(n); }

void create_block(char *prev_hash, int height, char *data, char *user) {
	char h[32], buf[512], command[512], position[512], curr_hash[64];
	FILE *fp;
	// make a buffer for making hash string
	bzero(buf, sizeof(buf));
	sprintf(h, "%d", height+1);
       	strcat(buf, prev_hash);
	strcat(buf, h);
	strcat(buf, data);
	strcat(buf, user);
	// make current hash
	bzero(command, sizeof(command));
	strcat(command, "echo -n ");
	strcat(command, buf);
	strcat(command, " | sha256sum");
	fp = popen(command, "r");
	if (fscanf(fp, "%s", curr_hash) != 1)
		oops("fscanf", 1);
	pclose(fp);
	// make a new block
	bzero(position, sizeof(position));
	strcat(position, "./blockchain/");
	strcat(position, curr_hash);
	fp = fopen(position, "w");
	fprintf(fp, "%s\n", curr_hash);
	fprintf(fp, "%s\n", prev_hash);
	fprintf(fp, "%s\n", h);
	fprintf(fp, "%s\n", data);
	fprintf(fp, "%s\n", user);
	fclose(fp);
	// update a recode of recent block
	fp = fopen("./blockchain/blockchain", "w");
	fprintf(fp, "%s\n", curr_hash);
	fprintf(fp, "%s\n", h);
	fclose(fp);
}

int main(int argc, char* argv[]) {    
    	int server_sock, client_sock, pid;
    
    	struct sockaddr_in server_addr;
    	struct sockaddr_in client_addr;
	socklen_t client_addr_size;
   
    	// get a socket
    	server_sock = socket(PF_INET, SOCK_STREAM, 0); 
   	if (server_sock == -1)
		oops("socket", 1);
    
    	// initialize the address and set IP address and port
    	memset(&server_addr, 0, sizeof(server_addr)); 
    	server_addr.sin_family = AF_INET; // IPv4
    	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // IP address
    	server_addr.sin_port = htons(atoi(argv[1])); // port
    
    	// bind socket and the server address
    	if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) != 0)
		oops("bind", 1);
    
    	// allow incoming calls with Qsize = 1 on socket 
    	if (listen(server_sock, 1) != 0)
    		oops("listen", 1);
    		
    	// check if there is a blockchain folder
    	char *cwd = (char *)malloc(sizeof(char) * 1024);
        memset(cwd, 0, 1024);

	DIR *dir = NULL;
        struct dirent *entry;
        struct stat buf;
	// current directory
	getcwd(cwd, 1024);
        if ((dir = opendir(cwd)) == NULL)
        	oops("opendir", 1);
        int checked = 0;
        while ((entry = readdir(dir)) != NULL) {
                lstat(entry->d_name, &buf);
                if (S_ISDIR(buf.st_mode) && !strcmp(entry->d_name, "blockchain")) {
                	checked = 1;
                	break;
		}
        }
        free(cwd);
        closedir(dir); 
	// make a folder if there is no folder called blockchain
	// create a genesis block
	if (!checked) {
		if (mkdir("blockchain", 755) == -1)
			oops("mkdir", 1);
		create_block(" ", 0, "genesis block", "unknown");
	}

	// main loop
    	while (1) {
		// wait for a call
		client_addr_size = sizeof(client_addr);
    		client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_addr_size);
    		if (client_sock == -1)
        		oops("accept", 1);
		// set username as ip address
		char username[32];
		sprintf(username, "%d", client_addr.sin_addr.s_addr);
		// read option number as a string
		size_t n;
		char message[BUFSIZ];
		n = read(client_sock, message, BUFSIZ-1);
		if (n < 0)
			oops("reading socket", 1);	
		// create a process
		pid = fork();
		if (pid < 0) {
			oops("fork", 1);
		}
		else if (pid == 0 && !strcmp(message, "1")) { // make a block
			close(server_sock);
			bzero(message, BUFSIZ);
			n = read(client_sock, message, BUFSIZ-1);
			if (n < 0)
				oops("reading socket", 1);
			printf("From client: %s\n", message);
			// read recent block info
			FILE *fp = fopen("./blockchain/blockchain", "r");
			if (fp == NULL)
				oops("fp", 1);
			char prev[65], temp[2], height[32];
			fgets(prev, 65, fp);
			fgets(temp, 2, fp);
			fgets(height, 32, fp);
			printf("%s,height: %s\n", prev, height);
			int h = atoi(height);
			create_block(prev, h, message, username);
			char confirm_msg[] = "Block is added!\n";
			n = write(client_sock, confirm_msg, sizeof(confirm_msg));
			if (n < 0)
				oops("writing socket", 1);
	    		close(client_sock);
			return 0;
		}
		else if (pid == 0 && !strcmp(message, "2")) { // see all the blocks
			char confirm_msg[] = "See all the blocks\n";
			n = write(client_sock, confirm_msg, sizeof(confirm_msg));
			if (n < 0)
				oops("writing socket", 1);
	    		close(client_sock);
			return 0;		
		}
		else
	    		close(client_sock);
	}
}

