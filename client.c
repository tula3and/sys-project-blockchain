#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define oops(msg) { perror(msg); exit(1); }

void sig_handler(int sig) {
	signal(sig, SIG_IGN);
}

int set_ticker(int n_msecs) { // milliseconds
	struct itimerval new_timeset;
	long n_sec, n_usecs;

	n_sec = n_msecs / 1000; // seconds
	n_usecs = (n_msecs % 1000) * 1000L; // microseconds

	new_timeset.it_interval.tv_sec = n_sec;
	new_timeset.it_interval.tv_usec = n_usecs;
	
	// store values
	new_timeset.it_value.tv_sec = n_sec;
	new_timeset.it_value.tv_usec = n_usecs;

	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

int sec;
void count_time(int sig) {
	sec++;
}

int main(int argc, char* argv[]) {
	int client_sock, choice, cnt;
	struct sockaddr_in server_addr;
	char message[BUFSIZ], choice_s[BUFSIZ];
	struct passwd *pwd;
	char whole_data[BUFSIZ*5];
	char *labels[5] = {"Hash", "PrevHash", "Height", "Data", "User"};
	char *username, *token, *lastB;
	
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGALRM, count_time);

	// save for connected time
	if (set_ticker(1000) == -1) {
		oops("set_ticker");
	}

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
		bzero(choice_s, BUFSIZ);
		scanf("%s", choice_s);
		while (1) {
			if (strlen(choice_s)) {
				if ((strlen(choice_s) == 1)) {
					choice = atoi(choice_s);
					if (choice > 0 && choice < 4)
						break;
				}
				printf("Invalid input. Only 1, 2, or 3 is accepted.\nInput number: ");
			}
			else
				printf("\nInput has nothing. Input a number.\nInput number: ");
			bzero(choice_s, BUFSIZ);
			scanf("%s", choice_s);
		}
		printf("**********\n");
		printf("Option selected\n");
		printf("**********\n");
		if (write(client_sock, choice_s, BUFSIZ-1) < 0)
			oops("writing socket");
		switch(choice) {
        	case 1:
			// write a message to the server
        		printf("Input message: ");
			bzero(message, BUFSIZ);
			scanf(" %[^\n]", message);
			while (strstr(message, "|") || (strlen(message) == 0)) {
				if (strlen(message))
					printf("'|' is not allowed. Write again.\nInput message: ");
				else
					printf("\nInput has nothing. Write a message.\nInput message: ");
				bzero(message, BUFSIZ);
				scanf(" %[^\n]", message);	
			}
			if (write(client_sock, message, BUFSIZ-1) < 0)
				oops("writing socket");
			// Save username
			pwd = getpwuid(geteuid());
			username = pwd->pw_name;
			if (write(client_sock, username, strlen(username)) < 0)
				oops("writing socket");
			// read a response from the server
			bzero(message, BUFSIZ);
			if (read(client_sock, message, BUFSIZ-1) < 0)
				oops("read");
			// print the message
			printf("Message from server: %s", message);
            		break;
        	case 2:
			// get info of blocks
			while (1) {
				printf("##########\n");
				if (read(client_sock, whole_data, sizeof(whole_data)-1) < 0)
					oops("read");
				lastB = strstr(whole_data, "||");
				if (lastB != NULL)
					break;
				token = strtok(whole_data, "|");
				cnt = 0;
			        while (token != NULL) {
					printf("%s: %s\n", labels[cnt++], token);
					token = strtok(NULL, "|");	
				}
			}
			cnt = 0;
			token = strtok(whole_data, "|");
			printf("%s: %s\n", labels[cnt++], token);
			printf("%s: %s\n", labels[cnt++], "");
			token = strtok(NULL, "|");	
			while (token != NULL) {
				printf("%s: %s\n", labels[cnt++], token);
				token = strtok(NULL, "|");
			}
			printf("##########\n");
            		break;
   	     	case 3:
			printf("Disconnected: total access time is %d seconds\n", sec);
			exit(0);
        	 	break;
	        default:
			printf("Invalid input\n");
		}
		printf("**********\n");
	}
	return 0;
}
