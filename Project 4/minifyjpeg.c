#include "minifyjpeg.h"
#include "magickminify.h"
#include <errno.h>

extern int errno;

char *filepath;
struct svc_req *req;

/* Implement the needed server-side functions here */
minifyjpeg_res *minify_jpeg_proc_1(filepath, req){
    static minifyjpeg_res res;
    int fildes;
    
    if (0 > (fildes = open(filepath, O_RDONLY))) {
        res.errno = errno;
        return (&res);
    }
}
