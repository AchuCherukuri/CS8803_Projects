#include "minifyjpeg_xdr.c"
#include "minifyjpeg_clnt.c"

/*
* Use RPC to obtain a minified version of the jpeg image
* stored in the array src_val and having src_len bytes.
*/
void* minify_via_rpc(CLIENT* clnt, void* src_val, size_t src_len, size_t *dst_len){
    minifyjpeg_res *result;
    void *minified_val;    
    struct jpeg_in jpeg_src;
    jpeg_src.src_jpeg_val.src_jpeg_val_val = src_val;
    jpeg_src.src_jpeg_val.src_jpeg_val_len = (u_int) src_len;
    jpeg_src.dst_len = (int*) dst_len;
            
    result = minify_jpeg_proc_1(jpeg_src, clnt);
    if (result == (minifyjpeg_res *)NULL){
        exit(1);
    }
    
    minified_val = (*result).minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val;
    
    printf("minify_via_rpc received minified value.\n");
    
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
