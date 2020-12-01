/*
	작성자: 김은란
	주석 규칙:
		1. /* 로 시작하는 주석은 아래혹은 왼쪽에 있는 코드 설명 주석이다
		2. /* 이후 번호가 있으면 (ex /*3. 내용) 아래 통신 규칙에 해당하는 번호이다. 
		3. //로 시작하는 주석은 다음 내용에 해당하는 코드 추가가 필요하다는 의미의 주석이다.
		4. 주석안에 {val}은 변수 val을 의미한다.
	통신 규칙:
	1. server가 managefifo file을 생성한다.
	2. client는 managefifo file이 있으면 open 후 "request {자신의 PID} {파일 크기}"를 write하고, 없으면 T시간 동안 스핀하며 대기한다.(T시간이 지나면 프로세스를 종료한다.)
	3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고, 각 쓰레드는 3개의 "{요청자 PID}FIFO{n}2ser"와 "{요청자 PID}FIFO{n}2cli" FIFO 파일을 생성한다.  
	4. client는 3개의 쓰레드를 할당하고 각 쓰레드는 순서에 따라 "{자신의 PID}FIFO1{n}ser"를 open한다
	5. client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.
	6. server의 각 쓰레드는 "{요청자 PID}FIFO{n}2ser" 파일을 읽고 복호화하여 "{요청자 PID}FIFO{n}temp.txt"파일에 임시 저장한다.
	7. server의 각 쓰레드는 "{요청자 PID}FIFO{n}temp.txt" 읽고 "{요청자 PID}FIFO{n}2cli" 파일에 writelock를 건 후 write한다.
	8. client의 1번 쓰레드는 "{자신의 PID}FIFO12cli"의 내용을 읽고 출력한다.
	9. 8이 끝나면 client의 2번 쓰레드는 "{자신의 PID}FIFO22cli"의 내용을 읽고 출력한다.
	10. 9가 끝나면 client의 3번 쓰레드는 "{자신의 PID}FIFO32cli"의 내용을 읽고 출력한다.
*/
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define BUF_SIZE 4096
#define FILENAMESIZE 200
#define null NULL
#define THREADPERWORK 3
pthread_t thread[3]; 

void* filerecv(void * arg);

struct threadArg{
	char fifoFileName[FILENAMESIZE];
	long long fileSize;
	int number;
};


/* 이 함수를 사용하면 에러처리를 위해 프로그램을 exit() 시키기 전에 FIFO파일이나 link파일을 삭제시켜줄 수 있다.*/
void error_handler(char * FileName1, char * FileName2, char * FileName3, char * FileName4){
    	if(FileName1!=null)
        	unlink(FileName1);
	if(FileName2!=null)
        	unlink(FileName2);
	if(FileName3!=null)
        	unlink(FileName3);
	if(FileName4!=null)
        	unlink(FileName4);
	pthread_exit(0);
}
void signalhandler(int sig) {
	error_handler("./managefifo",null,null,null);
	exit(0);
}

void decoding(char code[],int len)
{
	for(int i = 0 ; i<len;i++)
	{
		code[i] = code[i]-1;
	}
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
int main()
{
    char buf[BUF_SIZE];
    int protocol;
    char * request[3];
	int readlen=0;
	struct threadArg * argument;
    printf("시작 전\n");
    /*1. server가 managefifo file을 생성한다.*/
    if(mkfifo("./managefifo",0666) == -1){ //fifo init
        printf("fail to make fifo manage()\n");
        error_handler("./managefifo",null,null,null);
    }
    signal(SIGINT, signalhandler);
    
   while(1){
        /*3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고*/
    	if((protocol = open("./managefifo", O_RDONLY)) < 0){ //waiting in client
        	printf("fail to call open manage()\n");
        	error_handler("./managefifo",null,null,null);
    	}
        if((readlen=read(protocol, buf, BUF_SIZE)) < 0 ){ 
		printf("fail to call read()");
		error_handler("./managefifo",null,null,null);
        }
	if(readlen<7) /*읽은 값이 "request"의 길이인 7보다 작으면 while문 재시작*/
		continue;
	requestPasing(request,buf);
	
	printf("%s %s %s 처리 시작\n",request[0],request[1],request[2]);
	
	for(int i = 0 ;i<THREADPERWORK;i++){
		argument = (struct threadArg *)malloc(sizeof(struct threadArg));
		sprintf(argument->fifoFileName,"./%sFIFO%d",request[1],i+1);
		argument->number=i+1;
		argument->fileSize=atoll(request[2]);
		pthread_create(&thread[i],NULL,filerecv, (void *)argument); //실패 시 에러처리 추가
		
	}
	memset(buf,0x00,BUF_SIZE);
	close(protocol);
    }
    unlink("./managefifo");
    pthread_exit(0);
}

void* filerecv(void * arg){
	
	int fifo2Ser;
	int fifo2Cli;
	char buf[BUF_SIZE];
	char fifo2SerFileName[FILENAMESIZE];
	char fifo2CliFileName[FILENAMESIZE];
	char tempFileName[FILENAMESIZE];
	int tempfd;
	int readlen;
	struct flock fifo_lock;
	/*매개변수 저장*/
	struct threadArg * argument = (struct threadArg*)arg;
	sprintf(fifo2SerFileName,"%s2ser",argument->fifoFileName);
	sprintf(fifo2CliFileName,"%s2cli",argument->fifoFileName);
	sprintf(tempFileName,"./%stemp.txt",argument->fifoFileName);
	

	/*3. 각 쓰레드는 3개의 "{요청자 PID}FIFO{n}2ser"와 "{요청자 PID}FIFO{n}2cli" FIFO 파일을 생성한다.  */
    	if(mkfifo(fifo2SerFileName,0666) == -1){ //fifo init
    	    	printf("fail to make fifo manage()\n");
		error_handler("./managefifo",fifo2SerFileName,null,null);
    	}
	if((fifo2Ser = open(fifo2SerFileName,O_RDONLY|O_NONBLOCK))<0){ 
        	printf("fail to call open manage()\n");
		error_handler("./managefifo",fifo2SerFileName,null,null);
    	}

	if(mkfifo(fifo2CliFileName,0666) == -1){ //fifo init
    	    	printf("fail to make fifo manage()\n");
		error_handler("./managefifo", fifo2SerFileName, fifo2CliFileName,null);
    	}
	if((fifo2Cli = open(fifo2CliFileName,O_WRONLY))<0){ 
        	printf("fail to call open manage()\n");
		error_handler("./managefifo", fifo2SerFileName, fifo2CliFileName,null);
    	}
	/*6. server의 각 쓰레드는 "{요청자 PID}FIFO{n}" 파일을 읽고 복호화하여 "{요청자 PID}FIFO{n}temp.txt"파일에 임시 저장한다.*/
	
	if((tempfd = open(tempFileName,O_RDWR|O_CREAT))<0){ 
        	printf("fail to call open manage()\n");
		error_handler("./managefifo",fifo2SerFileName,fifo2CliFileName,tempFileName);
    	}
	
	int readflag=0;
	while(readflag==0){
		 while((readlen=read(fifo2Ser,buf,BUF_SIZE))>0){
			decoding(buf,readlen);
			write(tempfd,buf,readlen);
			readflag=1;
		}
	}
	/* 7. server의 각 쓰레드는 "{요청자 PID}FIFO{n}temp.txt" 읽고 "{요청자 PID}FIFO{n}" 파일에 writelock를 건 후 write한다.*/
	fifo_lock.l_type = F_WRLCK;
	fifo_lock.l_whence = SEEK_SET;
	fifo_lock.l_start = 0;
	fifo_lock.l_len = argument->fileSize;
	
	if (fcntl(fifo2Cli,F_SETLKW,&fifo_lock)==-1){
		printf("fail to call fcntl(lock)\n");
		write(fifo2Cli,"server error\n",14);
		error_handler("./managefifo",fifo2SerFileName,fifo2CliFileName,tempFileName);
	}
	
	lseek(tempfd,0,SEEK_SET);
	int totallen=0;
	while((readlen=read(tempfd,buf,BUF_SIZE))>0){
		write(fifo2Cli,buf,readlen);
		totallen+=readlen;
		
	}
	
	//fflush(fifo2Cli);
	fifo_lock.l_type = F_UNLCK;
	if(fcntl(fifo2Cli,F_SETLK,&fifo_lock)==-1){
		printf("fail to call fcntl(unlock)\n");
           	error_handler("./managefifo",fifo2SerFileName,fifo2CliFileName,tempFileName);
	}
	
	
	printf("Thread %d, send: %d byte\n",argument->number,totallen);
	fflush(stdout);
	unlink(fifo2SerFileName);
	unlink(fifo2CliFileName);
	unlink(tempFileName);
	
	close(fifo2Ser);
	close(fifo2Cli);
	close(tempfd);
	pthread_exit(0);
}
