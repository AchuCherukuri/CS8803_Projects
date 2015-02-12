/* ============================================================================
 * CS 8803 Introduction to Operating Systems
 * Project 1 Multi-Threaded Web Server
 *
 * Implements the server Project 1.
 *Zhihao Zou
============================================================================ */

#include <stdio.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/* CONSTANTS ===========================================================*/
#define SERVER_PORT 8888
#define BUFFER_SIZE 2048
/*======================================================================*/

int main(int argc, char **argv) {

	int socket_fd = 0; //server socket file description
	int client_socket_fd = 0; //client socket file description

	char comm_buffer[BUFFER_SIZE]; //buffer for normal communication between server and client

	int num_bytes = 0; //number of bytes read from client

	int set_reuse_addr = 1;
	int max_pending_connections = 1;

	struct sockaddr_in server; //server socket address structure
	struct sockaddr_in client; //client socket address structure
	struct hostent *client_host_info; //client host information
	char *client_host_ip; //client host ip address
	socklen_t client_addr_len;
	char *get_file_request_token;

	// Create socket (IPv4, stream-based, protocol likely set to TCP)
	if (0 > (socket_fd = socket(AF_INET, SOCK_STREAM, 0))) {
		fprintf(stderr, "server failed to create the listening socket\n");
		exit(1);
	}

	// Set socket to use wildcards
	if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &set_reuse_addr, sizeof(set_reuse_addr))) {
		fprintf(stderr, "server failed to set SO_REUSEADDR socket option (not fatal)\n");
	}

    // Configure server socket address structure (init to zero, IPv4, network byte
    //order for port and address)
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (0 > bind(socket_fd, (struct sockaddr *)&server, sizeof(server))) {
        fprintf(stderr, "server failed to bind\n");
        exit(1);
    }

    // Listen on the socket for up to some maximum pending connections
    if (0 > listen(socket_fd, max_pending_connections)) {
        fprintf(stderr, "server failed to listen\n");
        exit(1);
    }
    else {
        fprintf(stdout, "server listening for a connection on port %d\n", SERVER_PORT);
    }

    // Get the size client's address structure
    client_addr_len = sizeof(client);

    while(1) {
    	printf("\nWaiting for a connection\n");

    	// Accept a new client
		if (0 > (client_socket_fd = accept(socket_fd, (struct sockaddr *)&client, &client_addr_len))) {
			fprintf(stderr, "server accept failed\n");
		}
		else {
			fprintf(stdout, "\nServer accepted a client!\n");
		}

		printf("\nGot a connection!\n");

		// Determine who sent the echo so that we can respond
		client_host_info = gethostbyaddr((const char *)&client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), AF_INET);
		if (client_host_info == NULL) {
			fprintf(stderr, "server could not determine client host address\n");
		}
		client_host_ip = inet_ntoa(client.sin_addr);
		if (client_host_ip == NULL) {
			fprintf(stderr, "server could not determine client host IP\n");
		}
		fprintf(stdout, "\nServer established connection with %s (%s)\n", client_host_info->h_name, client_host_ip);

		// Read request from the client and parse the request protocol
		bzero(comm_buffer, BUFFER_SIZE);
		num_bytes = read(client_socket_fd, comm_buffer, BUFFER_SIZE);
		if (num_bytes == 0) {
			fprintf(stderr, "server could not read from socket\n");
		} else {
			fprintf(stdout, "\nServer received %d bytes: %s\n", num_bytes, comm_buffer);

			get_file_request_token = strtok(comm_buffer, " ");
			if (strcmp(get_file_request_token, "GetFile") == 0) {
				get_file_request_token = strtok(NULL, " ");
				if (strcmp(get_file_request_token, "GET") == 0) {
					get_file_request_token = strtok(NULL, " ");
				}
			}
			 fprintf(stdout, "\nThe client is asking for this file: %s\n", get_file_request_token);
		}

//		char response[] = "\nHey, I received your request, and I am going to"
//				" send you the file.\n";
//		// Echo back to the client
//		if (0 > write(client_socket_fd, response, strlen(response))) {
//			fprintf(stderr, "server could not write back to socket\n");
//		} else {
//			fprintf(stdout, "\nServer sending message back to client\n");
//		}

		char protocol_header_buffer[50];

		//open the requested file
		FILE *file_p = fopen(get_file_request_token, "rb");
		if (file_p == NULL) {
			fprintf(stderr, "\nFile open failed with error: %s!\n", strerror(errno));
			strcpy(protocol_header_buffer, "GetFile FILE_NOT_FOUND 0 0");
			if (0 > write(client_socket_fd, protocol_header_buffer, strlen(protocol_header_buffer))) {
				fprintf(stderr, "server could not write back to socket with error: %s\n", strerror(errno));
			} else {
				fprintf(stdout, "\nServer sending message back to client with: %s\n", protocol_header_buffer);
			}
			continue;
		} else {
			printf("\nFile opened! Starting to read and send file...\n");

//			fseek(file_p, 0L, SEEK_END);
//			int file_size = ftell(file_p);
//			fseek(file_p, 0L, SEEK_SET);
//
//			strcpy(protocol_header_buffer, "GetFile OK");
//			char protocol_header[30];
//			sprintf(protocol_header, "%s %d ", protocol_header_buffer, file_size);
//
//			if (0 > write(client_socket_fd, protocol_header, strlen(protocol_header))) {
//				fprintf(stderr, "server could not write back to socket with error: %s\n", strerror(errno));
//			} else {
//				fprintf(stdout, "\nServer sending message back to client with: %s\n", protocol_header);
//			}

			char file_stream[1024];

			while(1) {
				int bytes_read = fread(file_stream, sizeof(char), sizeof(file_stream), file_p);
				if (bytes_read < 0) {
					fprintf(stderr, "\nCouldn't read from file, error: %s\n", strerror(errno));
					break;
				}
				if (bytes_read == 0) {
					break;
				} else {

					//keeps track of where in the buffer we are
					void *writing_position = file_stream;

					int total_to_write = bytes_read;

					while (total_to_write > 0) {
						int bytes_written = write(client_socket_fd, writing_position, bytes_read);
						if (bytes_written <= 0) {
							fprintf(stderr, "\nServer could not write back to socket with error: %s\n", strerror(errno));
						} else {
							total_to_write -= bytes_written;
							writing_position += bytes_written;
							fprintf(stdout, "\nServer sending message back to client with: %s\n", file_stream);
						}
					}
				}
			}
			//free(file_stream);
		}

		fclose(file_p);

		printf("\nClosing the socket\n");
		//close client socket
		if(0 > close(client_socket_fd)) {
			printf("\nCould not close socket\n");
			return 0;
		}
    }
    // Close the socket and return
    close(socket_fd);
    return 0;
}
