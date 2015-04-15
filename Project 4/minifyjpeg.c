#include "minifyjpeg.h"
#include "magickminify.h"

/* Implement the needed server-side functions here */
minifyjpeg_res *minify_jpeg_proc_1_svc(char* src_val, int src_len, int dst_len, struct svc_req *req){
    static minifyjpeg_res res;
    void *dst_val;
    ssize_t source_len = src_len;
    ssize_t dest_len = dst_len;
    
    printf("Server started... waiting for request...\n");
    
    magickminify_init();
    
    printf("magicminify initialized.\n");
    
    dst_val = magickminify(src_val, source_len, &dest_len);
    
    printf("picture minified.\n");
    
    res.minifyjpeg_res_u.minified_jpeg_val = dst_val;
    
    magickminify_cleanup();
    
    return &res;
}

int minify_jpeg_prog_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result){
    (void) xdr_free(xdr_result, result);
}
