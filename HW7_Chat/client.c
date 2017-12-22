#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PID 1	// 현재 프로세스의 PID
#define MSGQ_KEY 5000	// 메시지 큐의 키 값
#define SHARED_KEY 6000 // 공유 메모리의 키 값

#define FROM_PID1 11
#define FROM_PID2 12	// 전송받을 메시지의 타입 
#define FROM_PID3 13

typedef struct {
	long mtype;			// 목적지 클라이언트의 ID값. Broadcast시 255.
	char mtext[1024]; 	// 데이터
} ChatMsg;

int main() {
	key_t que_key = MSGQ_KEY;
	int que_id = msgget(que_key, IPC_CREAT|0666);
	
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);

	int send_result, ret;
	char user_input;

	ssize_t rcv_bytes;
	
	printf("**CHAT_CLIENT**\n");
	printf("CURRENT ID: %d / QUEUE ID: %d\n\n", PID, que_id);
	printf("USAGE: \n");
	printf("  SEND: s <DEST_ID> <MESSAGE>\n");
	printf("  RECEIVE: r\n");
	printf("  VIEW: v\n");
	printf("  QUIT: q\n\n");
	
	printf("> ");
	while(1) {
		scanf("%c", &user_input);

		switch(user_input) {
		
		// 메시지 전송
		case 'S':
		case 's':
			scanf("%ld", &msg.mtype);
			scanf("%s", &msg.mtext);
			
			if(msgsnd(que_id, &msg, msg_size, IPC_NOWAIT) != 0) {
				printf("ERROR: message send failed.\n");
			}
			
			break;

		// 쪽지 읽기
		case 'R':
		case 'r':
			rcv_bytes = msgrcv(que_id, &msg, msg_size, 11, IPC_NOWAIT);
			if(rcv_bytes > 0) {
				printf("[PID %d] %s\n", 11 - 10, msg.mtext);
			}
			else
				printf("error_no %d\n", errno);

			break;

	/*
		// 전체 메시지 보기			
		case 'V':
		case 'v':
			read_chat_log();
			break;
	*/		
		case 'Q':
		case 'q':
			exit(0);

		default:
			printf("Wrong Input. Try again.\n");
		}
		
		while (getchar() != (int)'\n');
		printf("> ");
	}
	
	return 0;
}
