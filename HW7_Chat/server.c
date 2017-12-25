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

#define MQ0_KEY 9000	// Client 0의 메시지 큐의 키 값
#define MQ1_KEY 9001	// Client 1의 메시지 큐의 키 값
#define MQ2_KEY 9002	// Client 2의 메시지 큐의 키 값
#define SHARED_KEY 6000 // 공유 메모리의 키 값

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

int shm_id;
void *shm_addr;
ChatLog *shm_data;

void init_mq(ClientType c[3]) {
	c[0].qid = msgget((key_t) MQ0_KEY, IPC_CREAT|0666);
	c[1].qid = msgget((key_t) MQ1_KEY, IPC_CREAT|0666);
	c[2].qid = msgget((key_t) MQ2_KEY, IPC_CREAT|0666);
}

void init_shm() {
	shm_id = shmget((key_t)SHARED_KEY, sizeof(ChatLog), IPC_CREAT|0666);
	shm_addr = shmat(shm_id, (void *) 0, 0);
	
	shm_data = (ChatLog *) shm_addr;
	shm_data->head = 0;
	shm_data->count = 0;
}

int init_connection_s(ClientType c[3]) {
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);

	ssize_t nbytes;
	int init_count = 0, i, j, flag[3] = {0, };

	while(init_count < 3) {
		for(i = 0; i < 3; i++) {
			nbytes = msgrcv(c[i].qid, &msg, msg_size, 0, IPC_NOWAIT);
			if(nbytes > 0 && flag[i] == 0) {
				printf("PID of Client %d is %ld\n", i, msg.mtype);
				c[i].pid = msg.mtype;
				
				flag[i] = 1;
				init_count++;
			}
		}
	}
	
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			msg.mtype = c[j].pid;
			msgsnd(c[i].qid, &msg, msg_size, IPC_NOWAIT);
		}
	}
	
	printf("Connection between clients and server established.\n");
}

int util_getqid(ClientType c[3], int pid) {
	int i;
	
	for(i = 0; i < 3; i++) {
		if(c[i].pid == pid) {
			return c[i].qid;
		}
	}
	
	return -1;
}

int util_getpid(ClientType c[3], int qid) {
	int i;
	
	for(i = 0; i < 3; i++) {
		if(c[i].qid == qid) {
			return c[i].pid;
		}
	}
	
	return -1;
}

int save_public_chat(char msg[1024], int origin) {
	void *ret;

	if(shm_data->count < 100) {
		shm_data->pid[shm_data->head + shm_data->count] = origin;
		ret = memcpy(shm_data->mtext[shm_data->head + shm_data->count], msg, strlen(msg) + 1);
		
		shm_data->count++;
	}
	
	else {
		shm_data->pid[shm_data->head] = origin;
		ret = memcpy(shm_data->mtext[shm_data->head], msg, strlen(msg) + 1);
		
		if(shm_data->head + 1 == 100) shm_data->head = 0;
		else shm_data->head++;
	}
	
	if(ret == NULL) return -1;
	else return 0;
}

int check_message(ClientType c[3]) {
	ChatMsg msg;
	const int msg_size = sizeof(msg) - sizeof(msg.mtype);

	ssize_t nbytes;
	int i, j, dest_qid;
	int ret = -1;

	for(i = 0; i < 3; i++) {
	
		// Case 1: Broadcast
		nbytes = msgrcv(c[i].qid, &msg, msg_size, 1, IPC_NOWAIT);
		if(nbytes > 0) {
			ret = save_public_chat(msg.mtext, util_getpid(c, c[i].qid));
		}
		
		// Case 2: Private
		for(j = 0; j < 3; j++) {
			nbytes = msgrcv(c[i].qid, &msg, msg_size, c[j].pid * 2, IPC_NOWAIT);
			if(nbytes > 0) {
				dest_qid = util_getqid(c, msg.mtype / 2);
				msg.mtype = util_getpid(c, c[i].qid);
				
				ret = msgsnd(dest_qid, &msg, msg_size, IPC_NOWAIT);
			}
		}
	}

	return ret;
}

int main() {
	ClientType client[3];
	
	init_mq(client);
	init_shm();
	
	printf("CHAT SERVER\n\n");
	
	init_connection_s(client);
	
	while(1) {
		check_message(client);
	}
	
	return 0;
}
