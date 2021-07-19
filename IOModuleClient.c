#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>
#define THREADNUM 3
#define BUF_SIZE 4096
#define T 5
#define FILENAMESIZE 255
#define null NULL

void* filesend(void* th);
pthread_t thread[3];
long mypid; /*main에서 한 번 write하고 쓰레드에서는 읽기만 함으로 동기화 필요없음 */
struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
long long fileSize;
int codeFd; /*code.txt 파일의 파일디스크립터*/
pthread_cond_t printer1;
pthread_cond_t printer2;
pthread_cond_t printer3;
pthread_mutex_t printlock;
int endNum;
/* 이 함수를 사용하면 에러처리를 위해 프로그램을 exit() 시키기 전에 FIFO파일이나 link파일을 삭제시켜줄 수 있다.*/
void file_remove(void) {
	char fileName[FILENAMESIZE];
	for (int n = 1; n <= 3; n++) {
		snprintf(fileName,FILENAMESIZE-1, "./channel/%ldFIFO%d2ser", getpid(), n);
		remove(fileName);
		snprintf(fileName,FILENAMESIZE-1, "./channel/%ldFIFO%d2cli", getpid(), n);
		remove(fileName);
		snprintf(fileName,FILENAMESIZE-1, "./channel/%ldFIFO%d2temp.txt", getpid(), n);
		remove(fileName);
		snprintf(fileName,FILENAMESIZE-1, "./channel/codelink%d_%d", getpid(), n);
		unlink(fileName);
		
	}
	
}

void signalhandler(int sig) {
	file_remove();
	exit(0);
}

//client
int main()
{
	char buf[BUF_SIZE];
	int protocol;

	struct flock fifo_lock;

	fifo_lock.l_type = F_WRLCK;
	fifo_lock.l_whence = SEEK_SET;
	fifo_lock.l_start = 0;
	fifo_lock.l_len = 4096;
	memset(buf, 0x00, BUF_SIZE);
	pthread_cond_init(&printer1, NULL);
	pthread_cond_init(&printer2, NULL);
	pthread_cond_init(&printer3, NULL);
	pthread_mutex_init(&printlock, NULL);
	/*2. open managefifo file 만약 파일이 없다면 {T}시간 동안 스핀하며 대기한다.*/
	//{T} 시간 측정 시작
	while ((protocol = open("./managefifo", O_APPEND | O_WRONLY)) < 0) {
		//{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료
	}
	signal(SIGINT, signalhandler);
	/*2. managefifo 파일에 "request {mypid} {파일 크기}"를 write */
	/*code.txt의 상태 읽기*/
	if (stat("./code.txt", &sb) == -1) {
		printf("fail to call stat(./code.txt)\n");
		exit(1);
	}
	fileSize = (long long)sb.st_size;
	mypid = (long)getpid();
	snprintf(buf,FILENAMESIZE-1, "request %ld %lld \n", mypid, fileSize);
	printf("strlen(buf) = %d\n", strlen(buf));
	if (fcntl(protocol, F_SETLKW, &fifo_lock) == -1) {
		printf("fail to call fcntl()\n");
		exit(0);
	}
	if (write(protocol, buf, strlen(buf)) < 0) {
		printf("fail to call write(protocol,request)\n");
		exit(1);
	}
	fifo_lock.l_type = F_UNLCK;
	if (fcntl(protocol, F_SETLK, &fifo_lock) == -1) {
		printf("fail to call write()\n");
		exit(0);
	}
	write(1, buf, strlen(buf));
	close(protocol);
	/*4. 3개의 쓰레드를 할당*/
	for (int i = 0; i < THREADNUM; i++) {
		//실패 시 에러처리 추가 
		pthread_create(&thread[i], NULL, (void*)filesend, (void*)(i + 1));
	}

	for (int i = 0; i < THREADNUM; i++) {
		//실패 시 에러처리 추가
		pthread_join(thread[i], NULL);
	}
	file_remove();
	return 0;
}

void* filesend(void* arg) {
	int n = (int)arg;

	char linkFileName[FILENAMESIZE];
	int fifo2Ser;
	int fifo2Cli;
	char fifo2SerFileName[FILENAMESIZE];
	char fifo2CliFileName[FILENAMESIZE];
	int linkFd;
	char buf[BUF_SIZE];
	int readlen;
	int readFlag = 0;
	struct flock fifo_lock;

	fifo_lock.l_type = F_WRLCK;
	fifo_lock.l_whence = SEEK_SET;
	fifo_lock.l_start = 0;
	fifo_lock.l_len = fileSize;

	long long rwpointer; /* read write pointer */
	/*4. "{자신의 PID}FIFO1{n}2ser"와 "{자신 PID}FIFO{n}2cli"를 open한다*/
	snprintf(fifo2SerFileName,FILENAMESIZE-1, "./channel/%ldFIFO%d2ser", mypid, n);
	snprintf(fifo2CliFileName,FILENAMESIZE-1, "./channel/%ldFIFO%d2cli", mypid, n);
	//{T} 시간 측정 시작
	while ((fifo2Ser = open(fifo2SerFileName, O_WRONLY)) < 0) {
		//{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
	}
	while ((fifo2Cli = open(fifo2CliFileName, O_RDONLY|O_NONBLOCK)) < 0) {
		//{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
	}

	/*5client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.*/
	/* 파일을 병행적으로 읽는 방법
	1. link()를 통해 링크파일을 만든다(링크파일은 read write pointer를 공유하지 않지만 원본 파일의 내용은 공유하기 때문에 파일을 병행적으로 읽는 것에 적절하다)
	2. 링크파일을 open한다.
	3. lseek()를 통해 적절한 위치로 read write pointer 이동시킨다.
	*/
	snprintf(linkFileName,FILENAMESIZE-1, "./channel/codelink%ld_%d", mypid, n);
	if (link("code.txt", linkFileName) < 0) {
		printf("fail to call link(code.txt,%s)\n", linkFileName);
		exit(0);
	}
	if ((linkFd = open(linkFileName, O_RDONLY)) < 0) {
		printf("fail to call open(%s)\n", linkFileName);
		exit(0);
	}
	rwpointer = (fileSize / THREADNUM) * (n - 1);
	if (lseek(linkFd, rwpointer, SEEK_SET) < 0) {
		printf("fail to call lseek(%s)\n", linkFileName);
		exit(0);
	}


	if (fcntl(fifo2Ser, F_SETLKW, &fifo_lock) == -1) {
		printf("fail to call fcntl()\n");
		exit(0);
	}
	int totalfilereadlen = 0;
	for (int i = 0; i < (fileSize / THREADNUM) / BUF_SIZE; i++) {

		if ((readlen = read(linkFd, buf, BUF_SIZE)) < 0) {
			printf("fail to call read()\n");
			exit(0);
		}
		if (write(fifo2Ser, buf, BUF_SIZE) < 0) {
			printf("fail to call write()\n");
			exit(0);
		}
		totalfilereadlen += readlen;
	}

	if ((readlen = read(linkFd, buf, (fileSize / THREADNUM) % BUF_SIZE)) < 0) {
		printf("fail to call read()\n");
		exit(0);
	}

	if (write(fifo2Ser, buf, (fileSize / THREADNUM) % BUF_SIZE) < 0) {
		printf("fail to call read()\n");
		exit(0);
	}

	totalfilereadlen += readlen;
	printf("Thread %d, send to server to decoding: %d byte\n", n,totalfilereadlen);
	fifo_lock.l_type = F_UNLCK;
	if (fcntl(fifo2Ser, F_SETLK, &fifo_lock) == -1) {
		printf("fail to call write()\n");
		exit(0);
	}

	//8. client의 1번 쓰레드는 "{자신의 PID}FIFO12cli"의 내용을 읽고 출력한다.
	//9. 8이 끝나면 client의 2번 쓰레드는 "{자신의 PID}FIFO22cli"의 내용을 읽고 출력한다.
	//10. 9가 끝나면 client의 3번 쓰레드는 "{자신의 PID}FIFO32cli"의 내용을 읽고 출력한다.

	pthread_mutex_lock(&printlock);
	if (n == 1)
		printf("\n\nThread1 Print Start\n\n");
	if ((n == 2)) {
		if (endNum != 1)
			pthread_cond_wait(&printer2, &printlock);
		printf("\n\nThread2 Print Start\n\n");
	}
	else if ((n == 3)) {
		if ((endNum != 2))
			pthread_cond_wait(&printer3, &printlock);
		printf("\n\nThread3 Print Start\n\n");
	}

	int totallen = 0;
	int readflag = 0;
	while (readflag == 0) {
		while ((readlen = read(fifo2Cli, buf, BUF_SIZE)) > 0) {
			totallen += write(1, buf, readlen);
			readflag=1;

		}
	}
	fflush(stdout);
	if (n == 1) {
		endNum = 1;
		pthread_cond_signal(&printer2);
	}
	if (n == 2) {
		endNum = 2;
		pthread_cond_signal(&printer3);
	}
	pthread_mutex_unlock(&printlock);
	printf("filesize = %d Thread %d, fileread: %d byte, rcv: %d byte\n", fileSize, n, totalfilereadlen, totallen);
}



