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
#include <semaphore.h>

#define MSG_SIZE 80
#define THREADNUM 3
#define BUF_SIZE 4096
#define FILENAMESIZE 255
key_t key_id, key_id2;
void* filesend(void* arg);
pthread_t thread[3];
long mypid; /*main에서 한 번 write하고 쓰레드에서는 읽기만 함으로 동기화 필요없음 */
struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
long long fileSize;
sem_t printer;
//int codeFd; /*code.txt 파일의 파일디스크립터*/

/*typedef struct tTHREAD
{
	char ddmsg[MSG_SIZE];
	int cCnt;
}THREAD;
*/
void error_handler(char* linkFileName) {
	//  unlink(linkFileName);
	perror("handler err\n");
	exit(1);
}
//THREAD sendbuf[3];
//THREAD recvbuf[3];
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
	char buf[BUF_SIZE];
	//int filedesmf;
	//int cnt;
	//int nread;
	memset(buf, 0x00, BUF_SIZE);
	sem_init(&printer, 0, 0);

	key_id = msgget((key_t)60041, IPC_CREAT | 0666); //recv queue
	key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //send queue

	if (key_id < 0 || key_id2 < 0) {
		perror("msgget error");
		exit(1);
	}

	if (stat("./code.txt", &sb) == -1) {
		printf("fail to call stat(./code,txt)\n");
		exit(1);
	}
	fileSize = (long long)sb.st_size;
	mypid = (long)getpid();
	sprintf(buf, "request %ld %lld ", mypid, fileSize);
	// open managefifo file


	/*if ((nread = read(filedesmf, msg, MSG_SIZE)) < 0) { //cnt
		printf("fail to call read manage()\n");
		exit(1);
	}
	*/
	//thread 3
	for (int i = 0; i < THREADNUM; i++) {
		//	sendbuf[i].cCnt = i + 1;//1,2,3 pass
		pthread_create(&thread[i], NULL, (void*)filesend, (void*)(i + 1));
		printf("쓰레드 %d 생성 완료 \n", i);


		//sleep(1);
	}

	for (int i = 0; i <= THREADNUM; i++)
	{
		pthread_join(thread[i], NULL);
		sem_destroy(&printer);
	}
}

void* filesend(void* arg) { //n = file source
	int n = (int)arg;
	char fileName[FILENAMESIZE];
	char linkFileName[FILENAMESIZE];
	int fifoFd;
	int linkFd;
	char buf[BUF_SIZE];
	char buf2[BUF_SIZE];

	long long rwpointer; /* read write pointer */
	sprintf(linkFileName, "codelink%ld_%d", mypid, n);
	if (link("code.txt", linkFileName) < 0) {
		printf("fail to call link(code.txt,%s)\n", linkFileName);
		error_handler(linkFileName);
	}
	if ((linkFd = open(linkFileName, O_RDONLY)) < 0) {
		printf("fail to call open(%s)\n", linkFileName);
		error_handler(linkFileName);
	}
	rwpointer = (fileSize / THREADNUM) * (n - 1);
	if (lseek(linkFd, rwpointer, SEEK_SET) < 0) {
		printf("fail to call lseek(%s)\n", linkFileName);
		error_handler(linkFileName);
	}
	for (int i = 0; i < (fileSize / THREADNUM) / BUF_SIZE; i++) {

		if (read(linkFd, buf, BUF_SIZE) < 0) {
			printf("fail to call read()\n");
			error_handler(linkFileName);
		}
		if (msgsnd(key_id2, (void*)& buf, BUF_SIZE, 0) == -1) { //fifo rw
			printf("fail to call msgsnd()\n");
			exit(1);
		}
		printf("\nthread%d send success\n", n);
		printf("%s\n", buf);

		int c;
		sem_getvalue(&printer, &c);
		while (c != (n - 1)) {
			sem_getvalue(&printer, &c);
		}

		if (msgrcv(key_id, (void*)& buf2, BUF_SIZE, 0, 0) == -1) {
			perror("msgrcv error");
			exit(1);
		}
		printf("\nthread%d receive success\n", n);
		printf("%s\n", buf2);
		sem_post(&printer);
		unlink(linkFileName);
		/*if (msgrcv(key_id, (void*)& recvbuf[n], sizeof(struct tTHREAD), 0, 0) == -1) {
			perror("msgrcv error");
			exit(1);
		}
		else {
			printf("receive success\n");
			printf("%d. %s\n", recvbuf[n].cCnt, recvbuf[n].ddmsg);
		}*/
		//printf("%d. %s", pp->cCnt, pp->ddmsg);
		//printf("%s", dmsg);
	}
}