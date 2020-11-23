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

key_t key_id, key_id2;  // , key_id3
int clientCnt = 0;
void* filerecv(int i/*int threadnum*/);
pthread_t thread[3];


typedef struct tTHREAD
{
	char ddmsg[MSG_SIZE];
	int cCnt;
}THREAD;

THREAD recvbuf[3];
THREAD sendbuf[3];
int sender_id;
//sem_t threadrock;

/*void make_Fifo() {

	if (mkfifo("./managefifo", 0666) == -1) { //fifo init
		printf("fail to make fifo manage()\n");
		exit(1);
	}

}*/
/*
void open_Fifo() {
	if ((filedesmf = open("./managefifo", O_WRONLY | O_TRUNC)) < 0) { //waiting in client
		printf("fail to call open manage()\n");
		exit(1);
	}
}
*/
/*void snd_Data() {
	if (msgsnd(key_id, dmsg, MSG_SIZE) == -1) { //fifo rw
		printf("fail to call msgsnd()\n");
		exit(1);
	}
}*/
/*
void rcv_Data() {
	if (msgrcv(key_id2, arriveMsg, MSG_SIZE, getpid(), 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
	printf("receive success\n");
}*/
//server
int main()
{
	char msg[MSG_SIZE];
	char cnt[MSG_SIZE];
	
	key_id = msgget((key_t)60041, IPC_CREAT | 0666); //send queue
	key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //recv queue
	//key_id3= msgget((key_t)60043, IPC_CREAT|0666); //result queue

	printf("시작 전\n");

	for (int i = 0; i < 3; i++) {
		pthread_create(&thread[i], NULL, (void*)filerecv,(int *)i/*, (void*)& recvbuf[i]*/);
	}

	for (int i = 0; i <= 3; i++)
	{
		pthread_join(thread[i], NULL);
	}
	//sem_destroy(&threadrock);

	//unlink("./managefifo");

	if (msgctl(key_id2, IPC_RMID, NULL) == -1) {
		printf("msgctl failed\n");
		exit(1);
	}
	
	return 0;
}

void* filerecv(int i/*int threadnum*/) {

	//sender_id = getpid();

	if (msgrcv(key_id2, (void*)&recvbuf[i], sizeof(struct tTHREAD), 0, 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
	else {
		printf("receive success\n");
		printf("%d. %s\n", recvbuf[i].cCnt, recvbuf[i].ddmsg);

		sendbuf[i].cCnt = recvbuf[i].cCnt;
		//sendbuf[n].ddmsg = recvbuf[n].ddmsg
		strcpy(sendbuf[i].ddmsg, recvbuf[i].ddmsg);
		//sendbuf[i].ddmsg = sendbuf[i].ddmsg - 32;
		
		if (msgsnd(key_id, (void*)& sendbuf[i], sizeof(struct tTHREAD), 0) == -1) { //fifo rw
			printf("fail to call msgsnd()\n");
			exit(1);
		}
		printf("send success\n");
		printf("%d. %s\n", sendbuf[i].cCnt, sendbuf[i].ddmsg);
	}
	
}