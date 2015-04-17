#include <stdlib.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "gfserver.h"

#define BUFFER_SIZE	4096

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
	int chunk_idx;
	size_t bytes_transferred;
	double file_len;
	ssize_t write_len;
	char buffer[BUFFER_SIZE];
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
    
    /* request failure on HTTP response>=400*/
    curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if(res != CURLE_OK) {
        /* If the file just wasn't found, then send FILE_NOT_FOUND code*/ 
        return gfs_sendheader(ctx, GF_FILE_NOT_FOUND, 0);
    } else {
        file_len = 0;
        info = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, &file_len);
             
        if (info != CURLE_OK) {
            printf((char *)curl_easy_strerror(info));
            
            return EXIT_FAILURE;
        } else {           
            gfs_sendheader(ctx, GF_OK, file_len);
            
            /* Sending the file contents chunk by chunk. */
            bytes_transferred = 0;
            chunk_idx = 0;
            while(bytes_transferred < chunk.size){
                if (chunk.size-bytes_transferred >= BUFFER_SIZE) {
		            write_len = gfs_send(ctx, chunk.memory+chunk_idx, BUFFER_SIZE);
		        } else {
		            write_len = gfs_send(ctx, chunk.memory+chunk_idx, chunk.size-bytes_transferred);
		        }
		        if (write_len <= 0){
			        fprintf(stderr, "handle_with_curl write error");
			        return EXIT_FAILURE;
		        }
		        chunk_idx += write_len;
		        bytes_transferred += write_len;
	        }
        }
    }
    
    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    
    free(chunk.memory);
    
    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();

	return bytes_transferred;
}
