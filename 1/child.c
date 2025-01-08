#include <unistd.h>
#include <string.h>
#include <ctype.h>

int main() {
    char string[100] = "";
    int index = 0;
    char symbol;

    while (1) {
        ssize_t result = read(STDIN_FILENO, &symbol, sizeof(char));
        if (result == -1) {
            break;
        } else if (result == 0) {
            if (index > 0) {
                if (isupper(string[0])) {
                    string[index] = '\n';
                    string[index + 1] = '\0';
                    write(STDOUT_FILENO, string, strlen(string));
                    write(STDERR_FILENO, "ok\n", strlen("ok\n"));
                } else {
                    write(STDERR_FILENO, "error\n", strlen("error\n"));
                }
            }
            break;
        } else {
            if (symbol == '\0') {
                if (index == 0) {
                    break;
                }
                if (isupper(string[0])) {
                    string[index] = '\n';
                    string[index + 1] = '\0';
                    write(STDOUT_FILENO, string, strlen(string));
                    write(STDERR_FILENO, "ok\n", strlen("ok\n"));
                } else {
                    write(STDERR_FILENO, "error\n", strlen("error\n"));
                }
                string[0] = '\0';
                index = 0;
            } else {
                string[index++] = symbol;
                string[index] = '\0';
            }
        }
    }
    write(STDERR_FILENO, "ok\n", strlen("ok\n"));
    return 0;
}