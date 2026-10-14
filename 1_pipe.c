#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    const char *msg = (argc >= 2) ? argv[1] : "hello world";

    int p2c[2];
    int c2p[2];

    if (pipe(p2c) == -1) {
        perror("pipe (p2c)");
        return 1;
    }
    if (pipe(c2p) == -1) {
        perror("pipe (c2p)");
        close(p2c[0]);
        close(p2c[1]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(p2c[0]); close(p2c[1]);
        close(c2p[0]); close(c2p[1]);
        return 1;
    }

    if (pid == 0) {
        close(p2c[1]);
        close(c2p[0]);

        char buf[BUF_SIZE];
        ssize_t total = 0;
        ssize_t n;

        while ((n = read(p2c[0], buf + total, sizeof(buf) - 1 - (size_t)total)) > 0) {
            total += n;
            if ((size_t)total >= sizeof(buf) - 1) {
                break;
            }
        }
        close(p2c[0]);

        if (n == -1) {
            perror("child read");
            close(c2p[1]);
            _exit(1);
        }

        buf[total] = '\0';
        printf("  [자식 PID:%d] 부모로부터 받은 문자열: \"%s\"\n", getpid(), buf);
        fflush(stdout);
        for (ssize_t i = 0; i < total; i++) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }
        if (write(c2p[1], buf, (size_t)total) == -1) {
            perror("child write");
            close(c2p[1]);
            _exit(1);
        }

        close(c2p[1]);
        _exit(0);
    }
    close(p2c[0]);
    close(c2p[1]);

    printf("[부모 PID:%d] 자식에게 보낼 문자열: \"%s\"\n", getpid(), msg);
    fflush(stdout);
    if (write(p2c[1], msg, strlen(msg)) == -1) {
        perror("parent write");
        close(p2c[1]);
        close(c2p[0]);
        wait(NULL);
        return 1;
    }
    close(p2c[1]);
    char reply[BUF_SIZE];
    ssize_t total = 0;
    ssize_t n;
    while ((n = read(c2p[0], reply + total, sizeof(reply) - 1 - (size_t)total)) > 0) {
        total += n;
        if ((size_t)total >= sizeof(reply) - 1) {
            break;
        }
    }
    close(c2p[0]);

    if (n == -1) {
        perror("parent read");
        wait(NULL);
        return 1;
    }

    reply[total] = '\0';
    wait(NULL);

    printf("[부모 PID:%d] 자식으로부터 돌려받은 문자열: \"%s\"\n", getpid(), reply);
    return 0;
}
