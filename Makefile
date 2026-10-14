CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGETS = 1_pipe 2_pipeline 3_fifo_read 4_fifo_write

.PHONY: all clean test

all: $(TARGETS)

1_pipe: 1_pipe.c
	$(CC) $(CFLAGS) -o $@ $<

2_pipeline: 2_pipeline.c
	$(CC) $(CFLAGS) -o $@ $<

3_fifo_read: 3_fifo_read.c
	$(CC) $(CFLAGS) -o $@ $<

4_fifo_write: 4_fifo_write.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	@echo "=== [1] 파이프로 왕복 (1_pipe) 테스트 ==="
	./1_pipe "hello world"
	@echo ""
	@echo "=== [2] dup2로 명령 이어 붙이기 (2_pipeline) 테스트 ==="
	./2_pipeline ls -l : wc -l
	@echo ""
	@echo "=== [3] FIFO로 메시지 주고받기 (3_fifo_read / 4_fifo_write) 테스트 ==="
	./3_fifo_read & \
	READER_PID=$$!; \
	sleep 0.2; \
	./4_fifo_write "안녕하세요, FIFO 양방향 통신 테스트입니다!"; \
	wait $$READER_PID

clean:
	rm -f $(TARGETS) /tmp/sysprog_fifo /tmp/sysprog_fifo_reply
