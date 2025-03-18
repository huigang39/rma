#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEVICE "/dev/sharedmem"
#define SHARED_MEM_SIZE 4096

int main(int argc, char *argv[]) {
  int fd;
  char *map;

  /* 打开设备 */
  fd = open(DEVICE, O_RDWR);
  if (fd < 0) {
    perror("open");
    return EXIT_FAILURE;
  }

  /* 映射共享内存 */
  map = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return EXIT_FAILURE;
  }

  /* 写入数据到共享内存 */
  strcpy(map, "Hello, shared memory from user space!");

  /* 从共享内存读取数据 */
  printf("共享内存内容: %s\n", map);

  munmap(map, SHARED_MEM_SIZE);
  close(fd);
  return EXIT_SUCCESS;
}
