#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magick/MagickCore.h"

void magickminify_init(){
  MagickCoreGenesis("./", MagickTrue);
}

void* magickminify(void* src, ssize_t src_len, ssize_t* dst_len){
  Image *image, *resize;
  ImageInfo image_info;
  ExceptionInfo *exception;
  size_t len;
  void *ans;
  
  printf("variables passed into magickminify() function.\n");
  
  printf("size of the source jpeg is %d.\n", (int)src_len);
  printf("source jpeg content is %x.\n", src);

  GetImageInfo(&image_info);
  exception = AcquireExceptionInfo();
  
  printf("magickminify line25.\n");

  image = BlobToImage(&image_info, src, src_len, exception);
  
  printf("image content is %x.\n", image);

  if (exception->severity != UndefinedException)
    CatchException(exception);
  if (image == (Image *) NULL) {
    printf("image not obtained by BlobToImage.\n");
    exit(1);
  }
  
  printf("magickminify line37.\n");  
  printf("image loaded in magickminify() function.\n");

  resize = MinifyImage(image, exception);

  if (exception->severity != UndefinedException)
    CatchException(exception);
  if (resize == (Image *) NULL)
    exit(1);
    
  printf("picture resized by MinifyImage() function.\n");

  ans = ImageToBlob(&image_info, resize, &len, exception);
  
  printf("got return data of minified picture.\n");
  
  if(dst_len != NULL)
    *dst_len = len;

  DestroyImage(image);
  DestroyImage(resize);
  DestroyExceptionInfo(exception);

  return ans;
}

void magickminify_cleanup(){
  MagickCoreTerminus();
}

