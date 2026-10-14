#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_REQ   "/tmp/sysprog_fifo"
#define FIFO_REPLY "/tmp/sysprog_fifo_reply"
#define BUF_SIZE   512

int main(void)
{
    if (mkfifo(FIFO_REQ, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo (req)");
        return 1;
    }
    if (mkfifo(FIFO_REPLY, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo (reply)");
        unlink(FIFO_REQ);
        return 1;
    }

    printf("[받는 쪽 PID:%d] 보내는 쪽을 기다립니다... (%s)\n", getpid(), FIFO_REQ);
    fflush(stdout);

    int fd_in = open(FIFO_REQ, O_RDONLY);
    if (fd_in == -1) {
        perror("open (FIFO_REQ)");
        unlink(FIFO_REQ);
        unlink(FIFO_REPLY);
        return 1;
    }

    char buf[BUF_SIZE];
    ssize_t total = 0;
    ssize_t n;
    while ((n = read(fd_in, buf + total, sizeof(buf) - 1 - (size_t)total)) > 0) {
        total += n;
        if ((size_t)total >= sizeof(buf) - 1) {
            break;
        }
    }
    close(fd_in);

    if (total > 0) {
        buf[total] = '\0';
        printf("[받는 쪽 PID:%d] 받은 메시지 (%zd바이트): %s\n", getpid(), total, buf);
        fflush(stdout);

        int fd_out = open(FIFO_REPLY, O_WRONLY);
        if (fd_out != -1) {
            char ack[BUF_SIZE];
            int ack_len = snprintf(ack, sizeof(ack),
                                   "수신 완료 (%zd바이트, 받는 쪽 PID:%d)",
                                   total, getpid());
            if (ack_len > 0) {
                ssize_t written = write(fd_out, ack, (size_t)ack_len);
                (void)written;
            }
            close(fd_out);
            printf("[받는 쪽 PID:%d] 수신 확인 응답을 보냈습니다.\n", getpid());
        }
    } else {
        printf("[받는 쪽 PID:%d] 받은 내용이 없습니다.\n", getpid());
    }

    unlink(FIFO_REQ);
    unlink(FIFO_REPLY);
    printf("[받는 쪽 PID:%d] FIFO 정리(unlink) 완료.\n", getpid());
    return 0;
}
