#include "minifyjpeg_xdr.c"
#include "minifyjpeg_clnt.c"

/*
* Use RPC to obtain a minified version of the jpeg image
* stored in the array src_val and having src_len bytes.
*/
void* minify_via_rpc(CLIENT* clnt, void* src_val, size_t src_len, size_t *dst_len){
    minifyjpeg_res *result;
    char* source_val = (char*) src_val;
    result = minify_jpeg_proc_1(source_val, src_len, *dst_len, clnt);
    if (result == (minifyjpeg_res *)NULL){
        exit(1);
    }
    return result;
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
