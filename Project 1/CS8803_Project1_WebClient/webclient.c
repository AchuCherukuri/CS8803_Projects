/* ============================================================================
 * CS 8803 Introduction to Operating Systems
 * Project 1 Multi-Threaded Web Server
 *
 * Implements the clent Project 1.
 *Zhihao Zou
============================================================================ */

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* CONSTANTS =============================================================== */
#define SERVER_ADDR "localhost"
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
#define FILE_NAME "/home/zhihao/Documents/GaTech/CS8803/Project1/skeleton/lastname-firstname-pr1/1kb-sample-file-2.png"
#define FILE_COPY_PATH "/home/zhihao/Downloads/the_file_copy.png"
//=============================================================================

int main(int argc, char **argv) {

    int socket_fd = 0;
    struct sockaddr_in server_socket_addr;
    char buffer[BUFFER_SIZE];

    // Converts localhost into 0.0.0.0
    struct hostent *he = gethostbyname(SERVER_ADDR);
    unsigned long server_addr_nbo = *(unsigned long *)(he->h_addr_list[0]);

    // Create socket (IPv4, stream-based, protocol likely set to TCP)
    if (0 > (socket_fd = socket(AF_INET, SOCK_STREAM, 0))) {
        fprintf(stderr, "client failed to create socket\n");
        exit(1);
    }

    // Configure server socket address structure (init to zero, IPv4, network byte
    //order for port and address)
    bzero(&server_socket_addr, sizeof(server_socket_addr));
    server_socket_addr.sin_family = AF_INET;
    server_socket_addr.sin_port = htons(SERVER_PORT);
    server_socket_addr.sin_addr.s_addr = server_addr_nbo;

    // Connect socket to server
    if (0 > connect(socket_fd, (struct sockaddr *)&server_socket_addr, sizeof(server_socket_addr))) {
        fprintf(stderr, "client failed to connect to %s:%d!\n", SERVER_ADDR, SERVER_PORT);
        close(socket_fd);
        exit(1);
    }
    else {
        fprintf(stdout, "client connected to to %s:%d!\n", SERVER_ADDR, SERVER_PORT);
    }

    char get_file_content[250] = "GetFile GET ";

    strcat(get_file_content, FILE_NAME);

    fprintf(stdout, "\nThe GetFile Request is: %s\n", get_file_content);

    // Send file request
    if (0 > send(socket_fd, get_file_content, strlen(get_file_content), 0)) {
        fprintf(stderr, "client failed to send echo message");
        close(socket_fd);
        exit(1);
    }

    // Process response from server
    bzero(buffer, BUFFER_SIZE);
    int fid = open(FILE_COPY_PATH, O_CREAT, S_IWUSR);

    int total_file_size = 0;

    while (1) {
    	int bytes_read = read(socket_fd, buffer, BUFFER_SIZE);
    	if (bytes_read < 0) {
    		fprintf(stderr, "\nCouldn't read file from socket with error: %s\n", strerror(errno));
    	}
    	if (bytes_read == 0) {
    		break;
    	}
    	else {
    		int total_buffer_len = strlen(buffer);

    		char *file_stream_token = strtok(buffer, " ");
    		if (strcmp(file_stream_token, "GetFile") == 0) {
    			file_stream_token = strtok(NULL, " ");
				if (strcmp(file_stream_token, "OK") == 0) {
					//initialize the length of header for file exists response except for the length
					//of the "size"
					int header_len = 12;

					file_stream_token = strtok(NULL, " ");

					//extract the size that server declared sent
					char *server_read_file_size_str = file_stream_token;
					//add the length of sent size string into protocol header length
					int server_sent_size_str_len = strlen(server_read_file_size_str);
					header_len += server_sent_size_str_len;
					int file_stream_len = total_buffer_len - header_len;

					//parse server declared file read size to integer
					int server_sent_size = atoi(server_read_file_size_str);
					total_file_size += server_sent_size;
					int file_size_to_write = total_file_size;

					file_stream_token = strtok(NULL, " ");
					void *writing_position = file_stream_token;
					while(file_size_to_write > 0) {
						int bytes_written = write(fid, writing_position, strlen(file_stream_token));
						if (bytes_written <= 0) {
							fprintf(stderr, "\Could not write to the file with error: %s\n", strerror(errno));
							break;
						} else {
							file_size_to_write -= bytes_written;
							writing_position += bytes_written;
							fprintf(stdout, "\nServer sending message back to client with: %s\n", writing_position);
						}
					}
				}
				else if (strcmp(file_stream_token, "FILE_NOT_FOUND")) {
					fprintf(stderr, "Error: FILE NOT FOUND");
					exit(0);
				}
    		} else {
    			int total_buffer_len = strlen(buffer);
    			int file_size_to_write = total_buffer_len;
    			void *writing_position = buffer;
    			while(file_size_to_write > 0) {
					int bytes_written = write(fid, writing_position, strlen(file_stream_token));
					if (bytes_written <= 0) {
						fprintf(stderr, "\Could not write to the file with error: %s\n", strerror(errno));
						break;
					} else {
						file_size_to_write -= bytes_written;
						writing_position += bytes_written;
						fprintf(stdout, "\nServer sending message back to client with: %s\n", writing_position);
					}
				}
    		}
    	}
    }

    close(fid);
    fprintf(stdout, "\nTotoal size of the file received: %d bytes\n", total_file_size);
    fprintf(stdout, "\nFile saved at: %\n", FILE_COPY_PATH);

    // Close the socket and return the response length (in bytes)
    close(socket_fd);
    return 0;
}
