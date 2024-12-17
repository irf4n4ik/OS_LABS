// при комплиции нужно указывать флажок '-lrt', а то вдруг :)
// варик: 15

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#define SHARED_MEM_NAME "/shared_memory_example"
#define SHARED_MEM_SIZE 4096
#define SEMAPHORE_NAME "/semaphore_example"

void error_message_and_exit(const char *message) {
  perror(message);
  exit(EXIT_FAILURE);
}

int main() {
  int shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
  if (shm_fd == -1) {
    error_message_and_exit("an error while shm_open");
  }

  char *shared_mem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared_mem == MAP_FAILED) {
    error_message_and_exit("an error while mmap");
  }

  sem_t *sem = sem_open(SEMAPHORE_NAME, 0);
  if (sem == SEM_FAILED) {
    error_message_and_exit("an error while sem_open");
  }
  int result_writing = 1;
  while (1) {
    sem_wait(sem);
    if (strlen(shared_mem) == 0) break;

    if (isupper(shared_mem[0])) {
      result_writing = write(STDERR_FILENO, "ok\n", strlen("ok\n"));
      if (result_writing == -1) {
        munmap(shared_mem, SHARED_MEM_SIZE);
        sem_close(sem);
        error_message_and_exit("an error while writing by child");
      }

      result_writing = write(STDOUT_FILENO, shared_mem, strlen(shared_mem));
      if (result_writing == -1) {
        munmap(shared_mem, SHARED_MEM_SIZE);
        sem_close(sem);
        error_message_and_exit("an error while writing by child");
      }
      result_writing = write(STDOUT_FILENO, "\n", strlen("\n"));
      if (result_writing == -1) {
        munmap(shared_mem, SHARED_MEM_SIZE);
        sem_close(sem);
        error_message_and_exit("an error while writing by child");
      }
    } else {
      result_writing = write(STDERR_FILENO, "error\n", strlen("error\n"));
      if (result_writing == -1) {
        munmap(shared_mem, SHARED_MEM_SIZE);
        sem_close(sem);
        error_message_and_exit("an error while writing by child");
      }
    }
  }

  int result_munmap = munmap(shared_mem, SHARED_MEM_SIZE);
  if (result_munmap == -1) {
    error_message_and_exit("an error while munmap");
  }
  int result_closing = sem_close(sem);
  if (result_closing == -1) {
    error_message_and_exit("an error while sem_close");
  }
  return 0;
}