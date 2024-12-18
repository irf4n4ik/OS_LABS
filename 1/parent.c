#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef enum constants {
	LENGTH_FILE_NAME = 100,
	LENGTH_BUFFER = 100
} constants;

void ErrorMessage(const char* message) {
	const char* currentError = strerror(errno);
	write(STDERR_FILENO, message, strlen(message));
	write(STDERR_FILENO, currentError, strlen(currentError));
	write(STDERR_FILENO, "\n", 1);
}

int main() {
	char fileName[LENGTH_FILE_NAME];
	char buffer[LENGTH_BUFFER];
	char message[] = "Please, enter the file name to write.\n";
	ssize_t result = write(STDOUT_FILENO, message, strlen(message));
	ssize_t resultRead = read(STDIN_FILENO, fileName, LENGTH_FILE_NAME);
	fileName[resultRead - 1] = '\0';
	int resultOpen = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (resultOpen <= 0) {
		ErrorMessage("An error with file: ");
		exit(EXIT_FAILURE);
	}
	int pipe1[2];
	int pipe2[2];
	pid_t pid;
	if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
		ErrorMessage("An error with pipes: ");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if (pid == -1) {
		ErrorMessage("An error with fork: ");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		close(pipe1[1]);
		close(pipe2[0]);
		dup2(pipe1[0], STDIN_FILENO);
		close(pipe1[0]);
		dup2(pipe2[1], STDERR_FILENO);
		close(pipe2[1]);
		dup2(resultOpen, STDOUT_FILENO);
        close(resultOpen);
		execlp("./child", "./child", NULL);
		ErrorMessage("An error with execl: ");
		exit(EXIT_FAILURE);
	} else {
		close(pipe1[0]);
		close(pipe2[1]);
		while (1) {
			char message2[] = "Enter the string to check:\n";
			write(STDOUT_FILENO, message2, strlen(message2));
			ssize_t countRead = read(STDIN_FILENO, buffer, LENGTH_BUFFER - 1);
			if (countRead == -1) {
				ErrorMessage("An error while reading strings: ");
				exit(EXIT_FAILURE);
			}
			buffer[countRead - 1] = '\0';
			if (strlen(buffer) == 0) {
				break;
			}
			write(pipe1[1], buffer, strlen(buffer) + 1);
			countRead = read(pipe2[0], buffer, LENGTH_BUFFER - 1);
			if (countRead > 0) {
				write(STDOUT_FILENO, "Output from child: ", 20);
				write(STDOUT_FILENO, buffer, countRead);
			}
		}
		close(pipe1[1]);
		close(pipe2[0]);
		close(resultOpen);
        wait(NULL);
    }
	return 0;
}