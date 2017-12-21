#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <time.h>

#define MQ1_KEY 5000	// PID 1의 메시지 큐
#define MQ2_KEY 5001	// PID 2의 메시지 큐
#define MQ3_KEY 5002	// PID 3의 메시지 큐
#define SHARED_KEY 6000 // 공유 메모리의 키 값

typedef struct {
	long mtype;			// 목적지 클라이언트의 ID값. Broadcast시 255.
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

	printf("**CHAT_SERVER**\n");
	
	while(1) {
		
		for(int i = 0; i < 3; i++) {
			rcv_bytes = msgrcv(que_id[i], &msg, msg_size, 0, IPC_NOWAIT);
			if(rcv_bytes > 0) {
				// Case: Broadcast
				if(msg.mtype == 255) {
					printf("Broadcast.\n");
				}
		
				// Case: Private message
				else if(1 <= msg.mtype && msg.mtype <= 3) {
					printf("Private. PID%d->PID%d\n", i+1, msg.mtype);
					
					/////// ERROR BELOW
					int dest = msg.mtype - 1;
					msg.mtype = i + 1;
					if(msgsnd(que_id[dest], &msg, msg_size, IPC_NOWAIT) != 0) {
						printf("ERROR: message send failed.\n");
					}
					/////// ERROR END
				}
			
				else {
					continue;
				}
			}
		}
	}
	
	return 0;
}
