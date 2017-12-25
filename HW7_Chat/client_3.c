#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include <time.h>

#define MQ_KEY 9002	// Client 2의 메시지 큐의 키 값
#define SHARED_KEY 6000 // 공유 메모리의 키 값

/* mtype = 1: Broadcast
 * mtype = PID * 2: Client -> Server
 * mtype = PID: Server -> Client
 *
 *
 */

typedef struct {
	long mtype;			// 목적지 클라이언트의 ID값.
	char mtext[1024]; 	// 데이터
} ChatMsg;

typedef struct client {
	int qid;
	int pid;
} ClientType;

typedef struct {
	int head;
	int count;
	int pid[100];
	char mtext[100][1024];
} ChatLog;
 
long max_pid = -1;
long pid_list[3];

int shm_id;
void *shm_addr;
ChatLog *shm_data;

void usage() {
	printf("Enter any of the following commands after the prompt > :\n"
	"    s <DEST_PID> <TEXT>  -- Send <TEXT> to <DEST_PID>. To broadcast\n"
	"    the message, <DEST_PID> must be 0.\n"
	"    r -- Receive private message between clients.\n"
	"    v -- Print Chat-log from Shared Memory.\n"
	"    h -- Print this help message.\n"
	"    q -- Quit.\n\n");
}

int init_connection(ClientType * c) {
	printf("Waiting for other clients...\n");

	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);

	int ret, init_count = 0;
	ssize_t nbytes;

	msg.mtype = c->pid;
	ret = msgsnd(c->qid, &msg, msg_size, IPC_NOWAIT);
	
	if(ret != 0) return -1;
	
	printf("Client PID List: ");
	while(init_count < 3) {
		nbytes = msgrcv(c->qid, &msg, msg_size, 0, IPC_NOWAIT);
		if(nbytes > 0) {
			pid_list[init_count] = msg.mtype;
			if(msg.mtype > max_pid) max_pid = msg.mtype;
			
			printf("%ld ", msg.mtype);
			init_count++;
		}
	}
	printf("\n");
	printf("Connection between clients and server established.\n");
	
	return 0;
}

int send_message(int qid) {
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);	
	long dest;

	scanf("%ld %[^\n]s", &dest, msg.mtext);
	
	if(dest == 0) msg.mtype = 1;
	else if(dest == pid_list[0] || dest == pid_list[1] || dest == pid_list[2]) msg.mtype = dest * 2;
	else return -100;
	
	return msgsnd(qid, &msg, msg_size, IPC_NOWAIT);
}

int receive_message(int qid) {
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);
	ssize_t nbytes;
	
	while(1) {
		nbytes = msgrcv(qid, &msg, msg_size, 0, IPC_NOWAIT);
		if(nbytes > 0) {
			printf("[PID %ld] : %s\n", msg.mtype, msg.mtext);
		}
		else {
			break;
		}
	}

	return 0;
}

int view_public_chat() {
	int i, total_count = 0;

	if(shm_data->count < 100) {
		for(i = 0; i < shm_data->count; i++) {
			printf("[PID %d] : %s\n", shm_data->pid[i], shm_data->mtext[i]);
		}	
	}
	
	else {
		for(i = shm_data->head; i < 100; i++) {
			printf("[PID %d] : %s\n", shm_data->pid[i], shm_data->mtext[i]);
		}
		
		for(i = 0; i < shm_data->head; i++) {
			printf("[PID %d] : %s\n", shm_data->pid[i], shm_data->mtext[i]);
		} 
	}

	return 0;
}

// 퍼블릭 메시지 테스트
void test_public(int qid) {
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);
	int verbose;

	static int count = 0;

	printf("TEST_PUBLIC > ");
	scanf("%d", &verbose);

	for(int i = 0; i < verbose; i++) {
		msg.mtype = 1;
		sprintf(msg.mtext, "%d_test", count);
	
		msgsnd(qid, &msg, msg_size, IPC_NOWAIT);
		count++;
	}

}

int main() {
	ClientType client;
	char user_input;
	int ret;

	client.qid = msgget((key_t) MQ_KEY, IPC_CREAT|0666);
	client.pid = getpid();

	shm_id = shmget((key_t)SHARED_KEY, sizeof(ChatLog), IPC_CREAT|0666);
	shm_addr = shmat(shm_id, (void *) 0, 0);
	shm_data = (ChatLog *) shm_addr;
	
	printf("CHAT CLIENT (PID: %d, QID: %d)\n", client.pid, client.qid);
	usage();

	init_connection(&client);
	
	printf("> ");
	while(1) {
		scanf("%c", &user_input);
		
		switch(user_input) {
		
		// 메시지 전송
		case 'S':
		case 's':
			if(send_message(client.qid) != 0) {
				printf("ERROR: send_message failed.\n");
			}
			break;

		// 쪽지 읽기
		case 'R':
		case 'r':
			ret = receive_message(client.qid);
			if(ret == -100) {
				printf("ERROR: PID does not exist.\n");
			}
			
			else if(ret != 0) {
				printf("ERROR: receive_message failed.\n");
			}
			
			break;

		// 전체 메시지 보기			
		case 'V':
		case 'v':
			if(view_public_chat() != 0) {
				printf("ERROR: view_public_chat failed.\n");
			}
			break;

		// 퍼블릭 메시지 테스트
		/*
		case 'T':
		case 't':
			test_public(client.qid);
			break;
		*/
		case 'H':
		case 'h':
			usage();
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

