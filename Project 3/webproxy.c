#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "gfserver.h"
                                                                \
#define USAGE                                                                 \
"usage:\n"                                                                    \
"  webproxy [options]\n"                                                     \
"options:\n"                                                                  \
"  -n [seg_count]      Number of segments to use in communication with cache\n"\
"  -z [seg_size]       The size (in bytes) of the segments\n"                 \
"  -p [listen_port]    Listen port (Default: 8888)\n"                         \
"  -t [thread_count]   Num worker threads (Default: 1, Range: 1-1000)\n"      \
"  -s [server]         The server to connect to (Default: Udacity S3 instance)"\
"  -h                  Show this help message\n"                              \
"special options:\n"                                                          \
"  -d [drop_factor]    Drop connects if f*t pending requests (Default: 5).\n"


/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
  {"segment-count", required_argument,      NULL,           'n'},
  {"segment-size",  required_argument,      NULL,           'z'},
  {"port",          required_argument,      NULL,           'p'},
  {"thread-count",  required_argument,      NULL,           't'},
  {"server",        required_argument,      NULL,           's'},         
  {"help",          no_argument,            NULL,           'h'},
  {NULL,            0,                      NULL,             0}
};

struct ShmSem{
   int shmfd;
   sem_t *sem_id;
   int inUse;
};

extern ssize_t handle_with_cache(gfcontext_t *ctx, char *path, void* arg);

static gfserver_t gfs;
unsigned short seg_count = 1;
int j;
char *default_seg_name = "/segment";
char *default_sem_name = "/semaphore";

static void _sig_handler(int signo){
  if (signo == SIGINT || signo == SIGTERM){
	for(j = 0; j < seg_count; j++) {

		char pool_number[10], sem_name[20];

		sprintf(pool_number, "%d", j);

		strcpy(sem_name, default_sem_name);
		strcat(sem_name, pool_number);

		//delete existing semaphore
		if(sem_unlink(sem_name) == 0)
			printf("Semaphore %s deleted.\n", sem_name);

		char seg_name[20];

		strcpy(seg_name, default_seg_name);
		strcat(seg_name, pool_number);

		//deleted existing shared memory segment
		if(shm_unlink(seg_name) == 0)
			printf("Segment %s deleted.\n", seg_name);
	}
	gfserver_stop(&gfs);
    exit(signo);
  }
}

/* Main ========================================================= */
int main(int argc, char **argv) {
  int i, option_char;
  size_t seg_size = 102400;
  unsigned short port = 8888;
  unsigned short nworkerthreads = 1;

  char *server = "s3.amazonaws.com/content.udacity-data.com";
  char *default_seg_name = "/segment";
  char *default_sem_name = "/semaphore";

  if (signal(SIGINT, _sig_handler) == SIG_ERR){
    fprintf(stderr,"Can't catch SIGINT...exiting.\n");
    exit(EXIT_FAILURE);
  }

  if (signal(SIGTERM, _sig_handler) == SIG_ERR){
    fprintf(stderr,"Can't catch SIGTERM...exiting.\n");
    exit(EXIT_FAILURE);
  }

  // Parse and set command line arguments
  while ((option_char = getopt_long(argc, argv, "n:z:p:t:s:h", gLongOptions, NULL)) != -1) {
    switch (option_char) {
      case 'n': // segment-count
        seg_count = atoi(optarg);
        break;
      case 'z': // segment-size
        seg_size = atoi(optarg);
        break;
      case 'p': // listen-port
        port = atoi(optarg);
        break;
      case 't': // thread-count
        nworkerthreads = atoi(optarg);
        break;
      case 's': // file-path
        server = optarg;
        break;                                          
      case 'h': // help
        fprintf(stdout, "%s", USAGE);
        exit(0);
        break;       
      default:
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }
  }

  /* SHM initialization...*/
  struct ShmSem shmsem_pool[seg_count];

  for(j = 0; j < seg_count; j++) {

	//set a unique name for each segment
	char pool_number[10], seg_name[20];
    sprintf(pool_number, "%d", j);
    strcpy(seg_name, default_seg_name);
    strcat(seg_name, pool_number);

    printf("Segment %s name is created\n", seg_name);

    //create shared memory segment
    int shmfd = shm_open(seg_name, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);

    if (shmfd < 0) {
    	perror("In shm_open()");
    	exit(1);
    }

    shmsem_pool[j].shmfd = shmfd;

    printf("%s shared segment is created\n", seg_name);

    ftruncate(shmfd, seg_size);

    printf("%s segment is resized to %d\n", seg_name, (int) seg_size);

    char sem_name[20];
    strcpy(sem_name, default_sem_name);
    strcat(sem_name, pool_number);

    printf("Semaphore %s name is created\n", sem_name);

    sem_t *sem_id = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR, 1);

    shmsem_pool[j].sem_id = sem_id;

    printf("%s semaphore is created\n", sem_name);

    shmsem_pool[j].inUse = 0;
  }

  /*Initializing server*/
  gfserver_init(&gfs, nworkerthreads);

  /*Setting options*/
  gfserver_setopt(&gfs, GFS_PORT, port);
  gfserver_setopt(&gfs, GFS_MAXNPENDING, 10);
  gfserver_setopt(&gfs, GFS_WORKER_FUNC, handle_with_cache);
  for(i = 0; i < nworkerthreads; i++)
    gfserver_setopt(&gfs, GFS_WORKER_ARG, i, shmsem_pool);

  printf("I am running\n");

  /*Loops forever*/
  gfserver_serve(&gfs);
}
