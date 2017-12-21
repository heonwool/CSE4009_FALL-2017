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

typedef struct {
	long mtype;			// 목적지 클라이언트의 ID값. Broadcast시 255.
	char mtext[1024]; 	// 데이터
} ChatMsg;

int main() {
	key_t que_key = MSGQ_KEY;
	int que_id = msgget(que_key, IPC_CREAT|0666);
	
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);

	int send_result;
	char user_input;
	
	printf("**CHAT_CLIENT**\n");
	printf("CURRENT ID: %d\n\n", PID);
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
			while(msgrcv(que_id, &msg, msg_size, 0, IPC_NOWAIT) > 0) {
				printf("[PID %d] %s\n", msg.mtype, msg.mtext);
			}
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
