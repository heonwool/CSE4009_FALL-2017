#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

void sig_handler();
void sig_handler2();

//각각의 프로세스의 pid 저장 변수
int proc1;
int proc2;
int proc3;
//현재 프로세스가 signal을 보낼 다음 프로세스의 pid 저장 변수
int nextProc;
//콘솔에서 입력된 정수값 저장 변수
int iteration;
//콘솔에서 입력된 파일 이름 저장 변수
char *fileName;

int main(int argc, char *argv[])
{
    mode_t mode = S_IRUSR | S_IWUSR;
    int fd;
    iteration = atoi(argv[1]);
    fileName = argv[2];

    //양수가 입력됐는지 확인
    if(atoi(argv[1]) < 0) {
        printf("Negative Integer. Check your input.\n");
        return 0;
    }
    //파일 존재 유무 확인. 없으면 생성. 파일 TRUNC.
    if((fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, mode)) < 0) {
        printf("file open error.\n");
        return 0;
    }
    //increase operation & file writing 을 수행하는 시그널 핸들러 지정
    if(signal(SIGALRM, sig_handler) == SIG_ERR) {
        printf("signal error.\n");
        return 0;
    }
    //입력된 정수 값 만큼 연산을 진행했을 때 발생하는 시그널에 대한 핸들러 지정
    if(signal(SIGQUIT, sig_handler2) == SIG_ERR) {
        printf("signal error.\n");
        return 0;
    }

    proc1 = getpid();

    if((proc2 = fork()) < 0) {
        printf("fork error.\n");
        return 0;
    }
    if(proc2 == 0) {
        if((proc3 = fork()) < 0) {
            printf("fork error.\n");
            return 0;
        }
        //proc3. Send signal to proc1
        if(proc3 == 0) {
            nextProc = proc1;
            kill(nextProc, SIGALRM);
            while(1) {
                pause();
            }
        //proc2. Send signal to proc3
        } else {
            nextProc = proc3;
            while(1) {
                pause();
            }
        }
    //proc1. Send signal to proc2
    } else {
        nextProc = proc2;
        while(1) {
            pause();
        }
    }
}
void sig_handler()
{
    FILE *fw, *fr;
    int num;
    //현재 파일에 적힌 값 확인
    fr = fopen(fileName, "r");
    fscanf(fr, "%d", &num);
    fclose(fr);
    //현재 파일에 적혀있는 값이 목표 정수이면
    if(num >= iteration) {
    	//프로세스 종료
        kill(proc1, SIGQUIT);
        exit(1);
    }
    //기존의 파일에 적힌 값을 증가시켜 파일 writing
    fw = fopen(fileName, "w");
    fprintf(fw, "%d", ++num);
    fclose(fw);
    //다음 프로세스에게 시그널 전달
    kill(nextProc, SIGALRM);
}
void sig_handler2()
{
    exit(0);
}

