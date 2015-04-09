/*
 * Complete this file and run rpcgen -MN minifyjpeg.x
 */
 
struct jpeg_in {
 char* arg1;
};
struct jpeg_out {
 char* res1;
};

program MINIFY_JPEG_PROG {
  version MINIFY_JPEG_VERS {
    jpeg_out MINIFY_JPEG_PROC(jpeg_in) = 1;
  } = 1;
} = 0x20000001;
