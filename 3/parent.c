// при комплиции нужно указывать флажок '-lrt', а то вдруг :)
// варик: 15

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHARED_MEM_NAME "/shared_memory_example"
#define SHARED_MEM_SIZE 4096
#define SEMAPHORE_NAME "/semaphore_example"
#define LENGTH_FILE_NAME 100
#define LENGTH_WORD 100
void error_message_and_exit(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int main() {
	char file_name[LENGTH_FILE_NAME];
	char start_message[] = "please, enter the file name to write: ";
	ssize_t result_writing = write(STDOUT_FILENO, start_message, strlen(start_message));
	if (result_writing == -1) {
		error_message_and_exit("error writing");
	}
	ssize_t result_reading = read(STDIN_FILENO, file_name, LENGTH_FILE_NAME);
	if (result_reading == -1) {
		error_message_and_exit("error reading");
	}
	file_name[result_reading - 1] = '\0';

	int result_opening = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (result_opening <= 0) {
		error_message_and_exit("an error while opening file");
	}
	int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		error_message_and_exit("an error while shm_open");
	}

	if (ftruncate(shm_fd, SHARED_MEM_SIZE) == -1) {
		error_message_and_exit("an error while ftruncate");
	}

	char *shared_mem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shared_mem == MAP_FAILED) {
		error_message_and_exit("an error while mmap");
	}
	sem_t *sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 0);
	if (sem == SEM_FAILED) {
		close(result_opening);
		munmap(shared_mem, SHARED_MEM_SIZE);
		shm_unlink(SHARED_MEM_NAME);
		error_message_and_exit("an error while opening semaphore");
	}
	pid_t pid = fork();
	if (pid == -1) {
		close(result_opening);
		munmap(shared_mem, SHARED_MEM_SIZE);
		shm_unlink(SHARED_MEM_NAME);
		sem_close(sem);
		sem_unlink(SEMAPHORE_NAME);
		error_message_and_exit("an error while forking");
	}
	if (pid == 0) {
		int result_dup = dup2(result_opening, STDOUT_FILENO);
		if (result_dup == -1) {
			close(result_opening);
			munmap(shared_mem, SHARED_MEM_SIZE);
			shm_unlink(SHARED_MEM_NAME);
			sem_close(sem);
			sem_unlink(SEMAPHORE_NAME);
			error_message_and_exit("error while dup2");
		}
		int result_ex = execlp("./child", "./child", NULL);
		if (result_ex == -1) {
			close(result_opening);
			munmap(shared_mem, SHARED_MEM_SIZE);
			shm_unlink(SHARED_MEM_NAME);
			sem_close(sem);
			sem_unlink(SEMAPHORE_NAME);
			error_message_and_exit("error while execlp");
		}
		error_message_and_exit("execlp");
	} else {
		char word[LENGTH_WORD];
		do {
			result_writing = write(STDOUT_FILENO, "\n---processing strings---\n\n", strlen("\n---processing strings---\n\n"));
			if (result_writing == -1) {
				close(result_opening);
				munmap(shared_mem, SHARED_MEM_SIZE);
				shm_unlink(SHARED_MEM_NAME);
				sem_close(sem);
				sem_unlink(SEMAPHORE_NAME);
				error_message_and_exit("error while writing word");
			}
			result_reading = read(STDIN_FILENO, word, LENGTH_WORD);
			if (result_reading == -1) {
				close(result_opening);
				munmap(shared_mem, SHARED_MEM_SIZE);
				shm_unlink(SHARED_MEM_NAME);
				sem_close(sem);
				sem_unlink(SEMAPHORE_NAME);
				error_message_and_exit("error while reading word");
			}
			word[result_reading - 1] = '\0';
			strncpy(shared_mem, word, SHARED_MEM_SIZE - 1);
			shared_mem[SHARED_MEM_SIZE - 1] = '\0';
			sem_post(sem);
		} while (strlen(word) != 0);
		if (wait(NULL) == -1) {
			error_message_and_exit("error while waiting for child process");
		}
		if (munmap(shared_mem, SHARED_MEM_SIZE) == -1) {
			error_message_and_exit("error while unmapping shared memory");
		}
		if (shm_unlink(SHARED_MEM_NAME) == -1) {
			error_message_and_exit("error while unlinking shared memory");
		}
		if (sem_close(sem) == -1) {
			error_message_and_exit("error while closing semaphore");
		}
		if (sem_unlink(SEMAPHORE_NAME) == -1) {
			error_message_and_exit("error while unlinking semaphore");
		}
		if (close(result_opening) == -1) {
			error_message_and_exit("error while closing file");
		}
		result_writing = write(STDOUT_FILENO, "shared memory unlinked and cleaned up\n",
		                       strlen("shared memory unlinked and cleaned up\n"));
		if (result_writing == -1) {
			error_message_and_exit("error while writing final message");
		}
	}
	return 0;
}