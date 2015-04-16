#include "minifyjpeg.h"
#include "magickminify.h"

/* Implement the needed server-side functions here */
minifyjpeg_res *minify_jpeg_proc_1_svc(jpeg_in src_jpeg, struct svc_req *req){
    static minifyjpeg_res res;
    void *src_val = src_jpeg.src_jpeg_val.src_jpeg_val_val;
    ssize_t src_len = src_jpeg.src_jpeg_val.src_jpeg_val_len;
    ssize_t *dst_len = (ssize_t*) src_jpeg.dst_len;
    void *dst_val;
    
    printf("Server started... waiting for request...\n");
    
    magickminify_init();
    
    printf("magicminify initialized.\n");
    
    dst_val = magickminify(src_val, src_len, dst_len);
    
    printf("picture minified.\n");
    
    res.minifyjpeg_res_u.minified_jpeg_val.minified_jpeg_val_val = (char*) dst_val;
    
    printf("dst_val assigned to result union struct.\n");
    
    magickminify_cleanup();
    
    return &res;
}

int minify_jpeg_prog_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result){
    (void) xdr_free(xdr_result, result);
}
