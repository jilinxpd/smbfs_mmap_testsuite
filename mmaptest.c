#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if !defined(MAP_FAILED)
#  define MAP_FAILED      ((void *) -1)
#endif

int main (int argc, char *argv[])
{
  char testfile[] = "mmap.tmp";
  int fd;
  off_t filelen;
  off_t index;
  size_t stride;
  unsigned char *mapped;
  size_t pagesize;

  if (argc != 2)
    {
      printf("Usage: mmaptest stride\n");
      exit(1);
    }

#if defined(_SC_PAGESIZE)
  pagesize = (size_t) sysconf(_SC_PAGESIZE);
#else
  pagesize = (size_t) sysconf(_SC_PAGE_SIZE);
#endif

  stride=pagesize*atoi(argv[1]);

  fd = open(testfile,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
  if (fd == -1)
    {
      perror("Failed to open file");
      exit(1);
    }

  filelen=2147483647ULL*3;
  printf("File length = %lld\n", (long long)filelen);
  /* filelen=8096; */
  index = 0;
  if ((ftruncate(fd,filelen)) == -1)
    {
      perror("ftruncate failed");
      exit(1);
    }
#if 1
  {
    /* stride = pagesize*16; */
    printf("Stride      = %lld\n", (long long)stride);
    
    for (index = 0; index+stride < filelen; index += stride)
      {
        mapped = (unsigned char *) mmap((caddr_t) 0,stride,PROT_READ|PROT_WRITE,
                                        MAP_SHARED,fd,index);
        if (mapped == MAP_FAILED)
          {
            perror("Failed to map file");
            exit(1);
          }

        /* madvise(mapped, stride, MADV_SEQUENTIAL); */

        /* Uncomment the following line for a display of progress. */
        /* printf("index       = %lld\r", (long long)index); */
        memset((void *) mapped, 64, stride);

        if ((msync(mapped,stride,MS_ASYNC)) == -1) /* MS_ASYNC or MS_SYNC */
          {
            perror("\nmsync failed");
            exit(1);
          }

        if((munmap(mapped,stride)) == -1)
          {
            perror("\nmunmap failed");
            exit(1);
          }
        mapped=0;
      }
  }
#else
  {
    off_t off;
    unsigned char *p;
    off = 0;
    stride = pagesize*16;
    printf("Stride      = %d\n", stride);
    mapped = (unsigned char *) mmap((caddr_t) 0,filelen,PROT_READ|PROT_WRITE,
                                    MAP_SHARED,fd,off);
    if (mapped == MAP_FAILED)
      {
        perror("Failed to map file");
        exit(1);
      }

    for (p = mapped, index=0; index+stride < filelen; index += stride, p += stride)
      {
        memset((void *) p, 64, stride);
      }

    if((munmap(mapped,filelen)) == -1)
      {
        perror("munmap failed");
        exit(1);
      }
  }
#endif

  printf("\n");
  close(fd);
  unlink(testfile);
  return 0;
}
