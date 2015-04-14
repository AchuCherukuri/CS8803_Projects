#include "minifyjpeg_xdr.c"
#include "minifyjpeg_clnt.c"

/*
* Use RPC to obtain a minified version of the jpeg image
* stored in the array src_val and having src_len bytes.
*/
void* minify_via_rpc(CLIENT* clnt, void* src_val, size_t src_len, size_t *dst_len){
    minifyjpeg_res *result;
    result = minify_jpeg_proc_1(src_val, clnt);
    
}


/*
*Create a new CLIENT instance
*/
CLIENT* get_minify_client(char *server){
    CLIENT *clnt;
    clnt = clnt_create(server, MINIFY_JPEG_PROG, MINIFY_JPEG_VERS, "tcp");
    if (clnt == (CLENT *)NULL) {
        clnt_pcreateerror(server);
        exit(1);
    }
    return clnt;
}
