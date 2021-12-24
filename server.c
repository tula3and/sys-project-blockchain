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
	sprintf(h, "%d", height);
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

void send_message(int s, char *m) {
	int n = write(s, m, BUFSIZ*5);
	if (n < 0)
		oops("writing socket", 1);
}

int main(int argc, char* argv[]) {    
    	int server_sock, client_sock, pid;
	FILE *fp;
    
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
		create_block("", 0, "genesis block", "unknown");
	}

	// main loop
    	while (1) {
		// wait for a call
		client_addr_size = sizeof(client_addr);
    		client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_addr_size);
    		if (client_sock == -1)
        		oops("accept", 1);
		// read option number as a string
		size_t n;
		char message[BUFSIZ], username[BUFSIZ];
		n = read(client_sock, message, BUFSIZ-1);
		if (n < 0)
			oops("reading socket", 1);
		// get a pipe
		int thepipe[2];
		if (pipe(thepipe) == -1)
			oops("pipe", 1);
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
			n = read(client_sock, username, BUFSIZ-1);
			if (n < 0)
				oops("reading socket", 1);
			printf("From %s: %s\n", username, message);
			// read recent block info
			fp = fopen("./blockchain/blockchain", "r");
			if (fp == NULL)
				oops("fp", 1);
			char prev[65], temp[2], height[32];
			fgets(prev, 65, fp);
			fgets(temp, 2, fp);
			fgets(height, 32, fp);
			int h = atoi(height);
			create_block(prev, h+1, message, username);
			char confirm_msg[] = "Block is added!\n";
			n = write(client_sock, confirm_msg, sizeof(confirm_msg));
			if (n < 0)
				oops("writing socket", 1);
	    		fclose(fp);
	    		close(client_sock);
			close(thepipe[0]);
			if (dup2(thepipe[1], 1) == -1)
				oops("redirect stdout", 4);
			close(thepipe[1]);
			execlp("cat", "cat", "./blockchain/blockchain", NULL);
			oops("cat", 5);
		}
		else if (pid == 0 && !strcmp(message, "2")) { // see all the blocks
			close(thepipe[0]);
			close(thepipe[1]);
			fp = fopen("./blockchain/blockchain", "r");
			char curr_hash[BUFSIZ];
			char *folder = "./blockchain/";
			int curr_height;
			fscanf(fp, "%s", curr_hash);
			fscanf(fp, "%d", &curr_height);
	    		fclose(fp);
			// make a full path for reading a block
			char full_path[512];
			bzero(full_path, sizeof(full_path));
			strcat(full_path, folder);
			strcat(full_path, curr_hash);
			char prev_hash[BUFSIZ], height_s[BUFSIZ], data[BUFSIZ], user[BUFSIZ], whole_data[BUFSIZ*5];
			int last_point = 0;
			while ((fp = fopen(full_path, "r")) != NULL) {
				bzero(whole_data, sizeof(whole_data));
				fscanf(fp, "%s", curr_hash);
				if (!last_point && curr_height)
					fscanf(fp, "%s", prev_hash);
				fscanf(fp, "%s", height_s);
				fscanf(fp, " %[^\n]", data);
				fscanf(fp, "%s", user);
	    			fclose(fp);
				if (!strcmp(curr_hash, prev_hash) || !curr_height) {
					bzero(prev_hash, sizeof(prev_hash));
					// make a message containing whole block data
					strcat(whole_data, curr_hash);
					strcat(whole_data, "|");
					strcat(whole_data, prev_hash);
					strcat(whole_data, "|");
					strcat(whole_data, height_s);
					strcat(whole_data, "|");
					strcat(whole_data, data);
					strcat(whole_data, "|");
					strcat(whole_data, user);
					send_message(client_sock, whole_data);
					break;
				}
				bzero(full_path, sizeof(full_path));
				strcat(full_path, folder);
				strcat(full_path, prev_hash);
				// make a message containing whole block data
				strcat(whole_data, curr_hash);
				strcat(whole_data, "|");
				strcat(whole_data, prev_hash);
				strcat(whole_data, "|");
				strcat(whole_data, height_s);
				strcat(whole_data, "|");
				strcat(whole_data, data);
				strcat(whole_data, "|");
				strcat(whole_data, user);
				send_message(client_sock, whole_data);
				curr_height = atoi(height_s);
				if (curr_height == 1)
					last_point = 1;
			}
	    		close(client_sock);
			return 0;		
		}
		else {
			int newpid;
			newpid = fork();
			if (newpid < 0) {
				oops("fork", 1);
			}	
			else if (newpid == 0 && !strcmp(message, "1")) { 
				close(thepipe[1]);
				if (dup2(thepipe[0], 0) == -1)
					oops("redirect stdout", 3);
				close(thepipe[0]);
				execlp("head", "head", "-1", NULL);
				oops("head", 4);			
			}
			else
				close(client_sock);
		}
	}
}

