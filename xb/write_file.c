#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

// #define FILEPATH "/tmp/sxb/testfile"
// #define FILESIZE 1<<16
// #define FILESIZE 1<<20
#define FILESIZE 1<<12


// cd xb; gcc -O0 -g3 -o write_file write_file.c; chmod +x write_file; ls -alh /mnt/sxb
int main(int argc, char *argv[])
{
  char *file_path = "/mnt/sxb/testfile";
  if(argc == 2)
    file_path = argv[1];
  printf("file_path: %s\n", file_path); // -> vfprintf () from /lib64/libc.so.6 -> __GI__IO_file_xsputn () from /lib64/libc.so.6 -> __GI__IO_do_write () from /lib64/libc.so.6 -> new_do_write () from /lib64/libc.so.6 -> _IO_file_write@@GLIBC_2.2.5 () from /lib64/libc.so.6 -> write () from /lib64/libc.so.6
  FILE * f = fopen(file_path, "w");
  for(int i = 0; i < FILESIZE; i++){
      int temp = fputs("a", f);
      if(temp > -1){
          //Sucess!
      }
      else{
          printf("err\n");
      }
  }
  fclose(f); // -> ... write
  printf("ls -alh %s\n", file_path);
  return 0;
}