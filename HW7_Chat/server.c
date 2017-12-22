#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <time.h>

#define MQ1_KEY 5000	// PID 1의 메시지 큐의 키 값
#define MQ2_KEY 5001	// PID 2의 메시지 큐의 키 값
#define MQ3_KEY 5002	// PID 3의 메시지 큐의 키 값
#define SHARED_KEY 6000 // 공유 메모리의 키 값

typedef struct {
	long mtype;			// 목적지 클라이언트의 ID값.
	char mtext[1024]; 	// 데이터
} ChatMsg;

int main() {
	int que_id[3];
	que_id[0] = msgget(MQ1_KEY, IPC_CREAT|0666);
	que_id[1] = msgget(MQ2_KEY, IPC_CREAT|0666);
	que_id[2] = msgget(MQ3_KEY, IPC_CREAT|0666);

	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);
	ssize_t rcv_bytes;
	
	int dest;

	printf("** CHAT_SERVER **\n");
	
	while(1) {
		
		for(int i = 0; i < 3; i++) {
			// Case 1: Broadcast
			rcv_bytes = msgrcv(que_id[i], &msg, msg_size, 255, IPC_NOWAIT);
			if(rcv_bytes > 0) {
				printf("[PID %d -> Broadcast] %s\n", i + 1, msg.mtext);
				
				/*
				 * 이 부분에 Shared Memory를 활용해서 채팅 데이터를 저장해야합니다.
				 *
				 */
				
			}
			
			// Case 2: PID i to PID 1, 2, 3
			for(int j = 1; j <= 3; j++) {
				rcv_bytes = msgrcv(que_id[i], &msg, msg_size, 10 + j, IPC_NOWAIT);
				if(rcv_bytes > 0) {
					printf("[PID %d -> PID %d] %s\n", i + 1, j, msg.mtext);
				
					dest = (msg.mtype - 10) - 1;
					msg.mtype = i + 1;
					msg.mtext[0] = '#';
					if(msgsnd(que_id[dest], &msg, msg_size, IPC_NOWAIT) != 0) {
						printf("ERROR: message send failed.\n");
					}
				}
			}
		}
	}
	
	return 0;
}
