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
#define BUF_SIZE 4096
#define FILENAMESIZE 255
#define null NULL
#define THREADPERWORK 3
key_t key_id, key_id2;  // , key_id3
int clientCnt = 0;
void* filerecv(void * arg);
pthread_t thread[3];


/*typedef struct tTHREAD
{
	char ddmsg[MSG_SIZE];
	int cCnt;
}THREAD;
*/
struct threadArg{
	char fifoFileName[FILENAMESIZE];
	long long fileSize;
	int number;
};

void error_handler(char * linkFileName){
  //  unlink(linkFileName);
     perror("handler err\n");
    exit(1);
}

void requestPasing(char * request[3],char buf[])
{
    int cnt=0;
    request[0] = &buf[0];
    for(int i = 0;i<BUF_SIZE;i++){
        if(cnt>2)
	    break;
	if(buf[i]==' '){
		buf[i]='\0';
		cnt++;
		request[cnt]=&buf[i+1];
	}
    }
}
//THREAD recvbuf[3];
//THREAD sendbuf[3];
//int sender_id;
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
	char buf[BUF_SIZE];
    int protocol;
    char * request[3];
	//int readlen=0;
	struct threadArg * argument;

	key_id = msgget((key_t)60041, IPC_CREAT | 0666); //send queue
	key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //recv queue
	//key_id3= msgget((key_t)60043, IPC_CREAT|0666); //result queue

	printf("시작 전\n");

    requestPasing(request,buf);
	for (int i = 0; i < THREADPERWORK; i++) {
        argument = (struct threadArg *)malloc(sizeof(struct threadArg));
		sprintf(argument->fifoFileName,"./%sFIFO%d",request[1],i+1);
		argument->number=i+1;
		argument->fileSize=atoll(request[2]);
		pthread_create(&thread[i], NULL, (void*)filerecv, (void *)argument/*, (void*)& recvbuf[i]*/);
	}
    memset(buf,0x00,BUF_SIZE);

    pthread_exit(0);
	/*for (int i = 0; i <= THREADPERWORK; i++)
	{
		pthread_join(thread[i], NULL);
	}*/
	//sem_destroy(&threadrock);

	//unlink("./managefifo");

	if (msgctl(key_id2, IPC_RMID, NULL) == -1) {
		printf("msgctl failed\n");
		exit(1);
	}

	return 0;
}

void* filerecv(void * arg/*int threadnum*/) {

	//sender_id = getpid();
    int fd;
	char buf[BUF_SIZE];
	char tempFileName[FILENAMESIZE];
	int tempfd;
	/*매개변수 저장*/
	struct threadArg * argument = (struct threadArg*)arg;

	if (msgrcv(key_id2, (void*)& buf, BUF_SIZE, 0, 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
	else {
		printf("receive success\n");
		printf("%s\n", buf);

		/*sendbuf[i].cCnt = recvbuf[i].cCnt;
		//sendbuf[n].ddmsg = recvbuf[n].ddmsg
		strcpy(sendbuf[i].ddmsg, recvbuf[i].ddmsg);
		//sendbuf[i].ddmsg = sendbuf[i].ddmsg - 32;

		if (msgsnd(key_id, (void*)& sendbuf[i], sizeof(struct tTHREAD), 0) == -1) { //fifo rw
			printf("fail to call msgsnd()\n");
			exit(1);
		}
		printf("send success\n");
		printf("%d. %s\n", sendbuf[i].cCnt, sendbuf[i].ddmsg);
        */
	}

}