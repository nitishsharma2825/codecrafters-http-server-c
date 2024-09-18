#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	
	int server_fd, client_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	// Create a new TCP socket for IPv4
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	// max 5 connections can be in queue, else would be rejected
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	
	client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	printf("Client connected\n");

	char readBuffer[1024];
	recv(client_fd, readBuffer, sizeof(readBuffer), 0);
	char *method = strdup(readBuffer);

	char *reqPath = strtok(readBuffer, " ");  // GET/POST...
	printf("Request Method: %s\n", reqPath);

	reqPath = strtok(NULL, " ");
	printf("Request URL: %s\n", reqPath);

	if (strcmp(reqPath, "/") == 0)
	{
		char *resp = "HTTP/1.1 200 OK\r\n\r\n";
		send(client_fd, resp, strlen(resp), 0);
	} else if (strncmp(reqPath, "/echo/", 6) == 0)
	{
		reqPath = strtok(reqPath, "/"); // echo
		reqPath = strtok(NULL, "/"); // 1234
		
		int contentLen = strlen(reqPath);
		char resp[512];

		sprintf(resp, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", contentLen, reqPath);
		send(client_fd, resp, sizeof(resp), 0);

	} else if (strcmp(reqPath, "/user-agent") == 0)
	{
		reqPath = strtok(NULL, "\r\n"); // request line
		reqPath = strtok(NULL, "\r\n"); // 1st header Host:..
		reqPath = strtok(NULL, "\r\n"); // 2nd header User Agent

		char *headerBody = strtok(reqPath, " "); // User-Agent
		headerBody = strtok(NULL, " "); // foobar/1.2.3

		int contentLen = strlen(headerBody);
		char resp[512];

		sprintf(resp, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", contentLen, headerBody);
		send(client_fd, resp, sizeof(resp), 0);
	} else {
		char *resp = "HTTP/1.1 404 Not Found\r\n\r\n";
		send(client_fd, resp, strlen(resp), 0);
	}
	
	close(server_fd);

	return 0;
}