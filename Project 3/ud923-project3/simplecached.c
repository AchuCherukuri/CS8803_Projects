#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <mqueue.h>

#include "shm_channel.h"
#include "simplecache.h"

#define MAX_CACHE_REQUEST_LEN 256

static void _sig_handler(int signo){
	if (signo == SIGINT || signo == SIGTERM){
		/* Unlink IPC mechanisms here*/
		mq_unlink("mq_request");
		mq_unlink("mq_response");
		exit(signo);
	}
}

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  simplecached [options]\n"                                                  \
"options:\n"                                                                  \
"  -t [thread_count]   Num worker threads (Default: 1, Range: 1-1000)\n"      \
"  -c [cachedir]       Path to static files (Default: ./)\n"                  \
"  -h                  Show this help message\n"                              

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
  {"nthreads",           required_argument,      NULL,           't'},
  {"cachedir",           required_argument,      NULL,           'c'},
  {"help",               no_argument,            NULL,           'h'},
  {NULL,                 0,                      NULL,             0}
};

void Usage() {
  fprintf(stdout, "%s", USAGE);
}

int main(int argc, char **argv) {
	int nthreads = 1;
	int i;
	char *cachedir = "locals.txt";
	char option_char;

	mqd_t request_mq, response_mq;
	struct mq_attr attr;

	while ((option_char = getopt_long(argc, argv, "t:c:h", gLongOptions, NULL)) != -1) {
		switch (option_char) {
			case 't': // thread-count
				nthreads = atoi(optarg);
				break;   
			case 'c': //cache directory
				cachedir = optarg;
				break;
			case 'h': // help
				Usage();
				exit(0);
				break;    
			default:
				Usage();
				exit(1);
		}
	}

	if (signal(SIGINT, _sig_handler) == SIG_ERR){
		fprintf(stderr,"Can't catch SIGINT...exiting.\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGTERM, _sig_handler) == SIG_ERR){
		fprintf(stderr,"Can't catch SIGTERM...exiting.\n");
		exit(EXIT_FAILURE);
	}

	/* Initializing the cache */
	simplecache_init(cachedir);

	//Your code here...

	/* initialize the queue attributes */
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 10240;
	attr.mq_curmsgs = 0;
	//open a message queue for receiving request
	request_mq = mq_open("mq_request", O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG, &attr);
	if (request_mq < 0) {
		perror("In mq_open()");
		exit(1);
	}
	response_mq = mq_open("mq_response", O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG, &attr);
	if (response_mq < 0) {
		perror("In mq_open()");
		exit(1);
	}
}
