#include "minifyjpeg_xdr.c"
#include "minifyjpeg_clnt.c"

/*
* Use RPC to obtain a minified version of the jpeg image
* stored in the array src_val and having src_len bytes.
*/
void* minify_via_rpc(CLIENT* clnt, void* src_val, size_t src_len, size_t *dst_len){
    minifyjpeg_res *result = malloc(sizeof(struct minifyjpeg_res));
    result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val = calloc(src_len,sizeof(char));
    void *minified_val;    
    struct jpeg_in jpeg_src;
    jpeg_src.src_jpeg_val.src_jpeg_val_val = src_val;
    jpeg_src.src_jpeg_val.src_jpeg_val_len = (u_int) src_len;
    jpeg_src.dst_len = (int*) dst_len;
            
    result = minify_jpeg_proc_1(jpeg_src, clnt);
    
    *dst_len = (size_t) result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_len;
    
    printf("received file length is %d.\n", (int)result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_len);
    
    if (result == (minifyjpeg_res *)NULL) {
        printf("result object is NULL");
        exit(1);
    }
    
    printf("via rpc line26.\n");
    
    /*
    result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val = src_val;
    *dst_len = src_len;
    */
    
    /*if (result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val == (char*)NULL){
        printf("result_val is NULL, content is %x.\n", *result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val);
        exit(1);
    }*/
    
    printf("via rpc line34.\n");
    
    minified_val = (void*)result->minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val;
    
    if (minified_val == (void*)NULL){
        printf("minified_val is null.\n");
        exit(1);
    }
    
    printf("minify_via_rpc received minified value.\n");
    printf("received file length is %d.\n", (int)*dst_len);
    
    return minified_val;
}


/*
*Create a new CLIENT instance
*/
CLIENT* get_minify_client(char *server){
    CLIENT *clnt;
    char *server_name;
    server_name = server;
    clnt = clnt_create(server, MINIFY_JPEG_PROG, MINIFY_JPEG_VERS, "tcp");
    if (clnt == (CLIENT *)NULL) {
        clnt_pcreateerror(server_name);
        exit(1);
    }
    return clnt;
}
