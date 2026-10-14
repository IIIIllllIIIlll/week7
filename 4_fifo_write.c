#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_REQ   "/tmp/sysprog_fifo"
#define FIFO_REPLY "/tmp/sysprog_fifo_reply"
#define BUF_SIZE   512

int main(int argc, char *argv[])
{
    char msg[BUF_SIZE];

    if (argc >= 2) {
        msg[0] = '\0';
        size_t used = 0;
        for (int i = 1; i < argc; i++) {
            int written = snprintf(msg + used, sizeof(msg) - used,
                                   "%s%s", (i > 1) ? " " : "", argv[i]);
            if (written < 0 || (size_t)written >= sizeof(msg) - used) {
                break;
            }
            used += (size_t)written;
        }
    } else {
        snprintf(msg, sizeof(msg), "안녕하세요! (보내는 쪽 PID:%d 의 기본 메시지)", getpid());
    }

    int fd_out = open(FIFO_REQ, O_WRONLY);
    if (fd_out == -1) {
        perror("open (받는 쪽 ./3_fifo_read 를 먼저 실행했는지 확인하세요)");
        return 1;
    }

    if (write(fd_out, msg, strlen(msg)) == -1) {
        perror("write");
        close(fd_out);
        return 1;
    }
    close(fd_out);

    printf("[보내는 쪽 PID:%d] 보냈습니다: %s\n", getpid(), msg);

    int fd_in = open(FIFO_REPLY, O_RDONLY);
    if (fd_in != -1) {
        char ack[BUF_SIZE];
        ssize_t n = read(fd_in, ack, sizeof(ack) - 1);
        if (n > 0) {
            ack[n] = '\0';
            printf("[보내는 쪽 PID:%d] 상대방 응답: %s\n", getpid(), ack);
        }
        close(fd_in);
    }

    return 0;
}
