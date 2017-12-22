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

/* mtype 설명.
 * 1. 개인 메시지
 *   1) 클라이언트에서 서버로 데이터를 전송할 때는 11, 12, 13을 이용한다. 
 *      각 숫자는 목적지 클라이언트를 의미하며, 11은 1번 클라이언트, 12번은 2번 클라이언트,
 *      13번은 3번 클라이언트를 의미한다.
 *   2) 서버에서 클라이언트로 데이터를 전송할 때는 1, 2, 3을 이용한다.
 *      각 숫자는 출발지 클라이언트를 의미하며, 1은 1번 클라이언트, 2는 2번 클라이언트, 
 *      3번은 3번 클라이언트를 의미한다.
 * 2. 전체 메시지 
 *   1) 클라이언트가 전체 메시지를 작성할 때는 255를 이용한다.
 */

typedef struct {
	long mtype;			// 메시지 타입
	char mtext[1024]; 	// 데이터
} ChatMsg;

void print_man() {
	printf("The usage is as follows.\n\n");
	
	printf("1. Message transmission\n");
	printf("Send a message. Use the 'S' option. If you enter 255 for the destination client's ID, it will be created as a full message.\n");
	printf("Usage example: s <DEST_ID> <MESSAGE>\n\n");
	
	printf("2. Receive message\n");
	printf("Receive messages sent between clients. Use the 'R' option.\n");
	printf("Usage example: r\n\n");
	
	printf("3. Full message browsing\n");
	printf("View chat window. Use the 'V' option.\n");
	printf("Usage example: v\n");
}

int main() {
	key_t que_key = MSGQ_KEY;
	int que_id = msgget(que_key, IPC_CREAT|0666);
	
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);

	int send_result, ret;
	long destination;
	char user_input;

	ssize_t rcv_bytes;
	
	printf("** CHAT_CLIENT (ID: %d) **\n\n", PID);
	printf("Try 'h' for details.\n");
	
	printf("> ");
	while(1) {
		scanf("%c", &user_input);

		switch(user_input) {
		
		// 메시지 전송
		case 'S':
		case 's':
			scanf("%ld", &destination);
			scanf("%s", &msg.mtext);
			
			if(destination != 255)
				msg.mtype = 10 + destination;
			
			else
				msg.mtype = 255;
			
			if(msgsnd(que_id, &msg, msg_size, IPC_NOWAIT) != 0) {
				printf("ERROR: message send failed.\n");
			}
			
			break;

		// 쪽지 읽기
		case 'R':
		case 'r':
			rcv_bytes = msgrcv(que_id, &msg, msg_size, -4, IPC_NOWAIT);
			if(rcv_bytes > 0) {
				printf("[PID %ld] %s\n", msg.mtype, msg.mtext);
			}
			else
				printf("error_no %d\n", errno);

			break;

		// 전체 메시지 보기			
		case 'V':
		case 'v':
			/*
			 * 이 부분에 Shared Memory를 활용해서 채팅 데이터를 읽어야합니다.
			 *
			 */
			break;
			
		case 'H':
		case 'h':
			print_man();
			break;
	
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
