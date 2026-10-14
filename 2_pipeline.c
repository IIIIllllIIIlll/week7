#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int sep_idx = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ":") == 0) {
            sep_idx = i;
            break;
        }
    }

    if (sep_idx <= 1 || sep_idx >= argc - 1) {
        fprintf(stderr, "사용법: %s <명령1> [인자...] : <명령2> [인자...]\n", argv[0]);
        fprintf(stderr, "예시  : %s ls -l : wc -l\n", argv[0]);
        return 1;
    }

    argv[sep_idx] = NULL;
    char **cmd1 = &argv[1];
    char **cmd2 = &argv[sep_idx + 1];

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork (cmd1)");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }
    if (pid1 == 0) {
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 (stdout)");
            close(fd[1]);
            _exit(1);
        }
        close(fd[1]);
        execvp(cmd1[0], cmd1);
        perror(cmd1[0]);
        _exit(127);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork (cmd2)");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        return 1;
    }
    if (pid2 == 0) {
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2 (stdin)");
            close(fd[0]);
            _exit(1);
        }
        close(fd[0]);
        execvp(cmd2[0], cmd2);
        perror(cmd2[0]);
        _exit(127);
    }

    close(fd[0]);
    close(fd[1]);

    int status1 = 0, status2 = 0;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    if (WIFEXITED(status2)) {
        return WEXITSTATUS(status2);
    }
    return 1;
}
