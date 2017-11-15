#include <stdio.h>
#include <signal.h>
#include <unistd.h>

struct sigaction act_new;

void sig_fn(int signo);

int main(void)
{
	act_new.sa_handler = sig_fn; //시그널에 새로운 시그널 핸들러 지정
	sigemptyset(&act_new.sa_mask);

	sigaction(SIGINT, &act_new, NULL);

	while(1) {
		printf("process is running..\n");
		sleep(1);
	}
}

void sig_fn(int signo)
{
	printf("CTRL-C is pressed. Try Again.\n");
}
