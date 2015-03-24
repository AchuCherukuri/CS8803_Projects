#include <stdlib.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "gfserver.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}


ssize_t handle_with_curl(gfcontext_t *ctx, char *path, void* arg){
	int fildes;
	size_t file_len, bytes_transferred, chunk_idx;
	ssize_t read_len, write_len;
	char buffer[4096];
	char *data_dir = arg;

	strcpy(buffer,data_dir);
	strcat(buffer,path);
	
	CURL *curl_handle;
    CURLcode res, info;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);
    
    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, buffer);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
    field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if(res != CURLE_OK) {
        /* If the file just wasn't found, then send FILE_NOT_FOUND code*/ 
        return gfs_sendheader(ctx, GF_FILE_NOT_FOUND, 0);
    } else {
        info = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, file_len);
        if (info != CURLE_OK) {
            printf(curl_easy_strerror(info);
        } else {
            gfs_sendheader(ctx, GF_OK, file_len);
            
            /* Sending the file contents chunk by chunk. */
            bytes_transferred = 0;
            while(bytes_transferred < chunk.size){
		        write_len = gfs_send(ctx, chunk.memory+bytes_transferred, chunk.size);
		        if (write_len <= 0){
			        fprintf(stderr, "handle_with_file write error");
			        return EXIT_FAILURE;
		        }
		        bytes_transferred += write_len;
	        }
        }
    }
	return bytes_transferred;
}
