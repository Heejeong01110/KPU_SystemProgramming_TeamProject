#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <pthread.h>
#include <signal.h>
#include <time.h>


#define MSG_SIZE 80

key_t key_id, key_id2;
void* filesend(int n);
pthread_t thread[3];


typedef struct tTHREAD
{
	char ddmsg[MSG_SIZE];
	int cCnt;
}THREAD;

THREAD sendbuf[3];
THREAD recvbuf[3];
/*void open_Thread() {
	if ((filedesth = open(threadpathTemp, O_WRONLY)) < 0) {
		printf("fail to call fifo thread() %s\n", threadpathTemp);
		exit(1);
	}
}*/

/*void snd_Data() {
	if (msgsnd(key_id2, dmsg, MSG_SIZE) == -1) { //fifo rw
		printf("fail to call msgsnd()\n");
		exit(1);
	}
}*/
/*void read_Data() {
	if (msgrcv(key_id, msg, MSG_SIZE, getpid(), 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
}*/
//client
int main()
{
	char msg[MSG_SIZE]; //num count
	char msg2[MSG_SIZE];//after
	int filedesmf;
	int cnt;
	int nread;
	key_id = msgget((key_t)60041, IPC_CREAT | 0666); //recv queue
	key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //send queue

	if (key_id < 0 || key_id2 < 0 ) {
		perror("msgget error");
		exit(1);
	}
	// open managefifo file
	

	/*if ((nread = read(filedesmf, msg, MSG_SIZE)) < 0) { //cnt
		printf("fail to call read manage()\n");
		exit(1);
	}
	*/
	
	strcpy(sendbuf[0].ddmsg, "hello");
	strcpy(sendbuf[1].ddmsg, "two");
	strcpy(sendbuf[2].ddmsg, "three");

	//thread 3
	for (int i = 0; i < 3; i++) {
		sendbuf[i].cCnt = i + 1;//1,2,3 pass
		pthread_create(&thread[i], NULL, (void*)filesend, (int*)i/*(void*)& sendbuf[i]*/);
		printf("쓰레드 %d 생성 완료\n", i);


		//sleep(1);
	}

	for (int i = 0; i <= 3; i++)
	{
		pthread_join(thread[i], NULL);
	}
}

void* filesend(int n) { //n = file source


	if (msgsnd(key_id2, (void*)&sendbuf[n], sizeof(struct tTHREAD), 0) == -1) { //fifo rw
		printf("fail to call msgsnd()\n");
		exit(1);
	}
	printf("send success\n");
	printf("%d. %s\n", sendbuf[n].cCnt, sendbuf[n].ddmsg);

	if (msgrcv(key_id, (void*)& recvbuf[n], sizeof(struct tTHREAD), 0, 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
	else {
		printf("receive success\n");
		printf("%d. %s\n", recvbuf[n].cCnt, recvbuf[n].ddmsg);
	}
	//printf("%d. %s", pp->cCnt, pp->ddmsg);
	//printf("%s", dmsg);
}
	