/*
 * Complete this file and run rpcgen -MN minifyjpeg.x
 */
typedef struct jpeg_out *minified_jpeg_t;

struct jpeg_out {
    u_int outfile_len;
    string outfile_val<>;
};

union minifyjpeg_res switch (int errno) {
    case 0:
        minified_jpeg_t minified_jpeg;
    default:
        void;
};

program MINIFY_JPEG_PROG {
  version MINIFY_JPEG_VERS {
    minifyjpeg_res MINIFY_JPEG_PROC() = 1;
  } = 1;
} = 0x20000001;
