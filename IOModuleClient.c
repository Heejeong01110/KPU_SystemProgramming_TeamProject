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
	4. client는 3개의 쓰레드를 할당하고 각 쓰레드는 순서에 따라 "{자신의 PID}FIFO1{n}2ser"와 "{자신의 PID}FIFO{n}2cli"를 open한다
	5. client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.
	6. server의 각 쓰레드는 "{요청자 PID}FIFO{n}2ser" 파일을 읽고 복호화하여 "{요청자 PID}FIFO{n}temp.txt"파일에 임시 저장한다.
	7. server의 각 쓰레드는 "{요청자 PID}FIFO{n}temp.txt" 읽고 "{요청자 PID}FIFO{n}2cli" 파일에 writelock를 건 후 write한다.
	8. client의 1번 쓰레드는 "{자신의 PID}FIFO12cli"의 내용을 읽고 출력한다.
	9. 8이 끝나면 client의 2번 쓰레드는 "{자신의 PID}FIFO22cli"의 내용을 읽고 출력한다.
	10. 9가 끝나면 client의 3번 쓰레드는 "{자신의 PID}FIFO32cli"의 내용을 읽고 출력한다.
*/
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
sem_t printer;
/* 이 함수를 사용하면 에러처리를 위해 프로그램을 exit() 시키기 전에 FIFO파일이나 link파일을 삭제시켜줄 수 있다.*/
void error_handler(char * FileName){
    	if(FileName!=null)
        	unlink(FileName);

	pthread_exit(0);
}

//client
int main()
{
    char buf[BUF_SIZE];
    int protocol;

	memset(buf,0x00,BUF_SIZE);
	sem_init(&printer, 0, 0);
    /*2. open managefifo file 만약 파일이 없다면 {T}시간 동안 스핀하며 대기한다.*/
    //{T} 시간 측정 시작
    while((protocol = open("./managefifo", O_APPEND | O_WRONLY)) < 0){
        //{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
    }
    
    /*2. managefifo 파일에 "request {mypid} {파일 크기}"를 write */
    /*code.txt의 상태 읽기*/
    if (stat("./code.txt", &sb) == -1){ 
        printf("fail to call stat(./code,txt)\n");
        exit(1);
    }   
    fileSize = (long long)sb.st_size;
    mypid = (long)getpid();
    sprintf(buf,"request %ld %lld \n",mypid,fileSize);
	printf("strlen(buf) = %d\n",strlen(buf));
    if(write(protocol, buf, strlen(buf)) < 0 ){ 
        printf("fail to call write(protocol,request)\n");
        exit(1);
    }
	write(1,buf,strlen(buf));
    close(protocol);
    /*4. 3개의 쓰레드를 할당*/
    for(int i=0;i<THREADNUM;i++){
	//실패 시 에러처리 추가 
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)(i+1));
    }

    for(int i=0;i<THREADNUM;i++){
	//실패 시 에러처리 추가
          pthread_join(thread[i],NULL);
    }
	sem_destroy(&printer);
    pthread_exit(1);
}

void* filesend(void* arg){ 
    	int n = (int)arg;
    	
    	char linkFileName[FILENAMESIZE];
    	int fifo2Ser;
	int fifo2Cli;
	char fifo2SerFileName[FILENAMESIZE];
	char fifo2CliFileName[FILENAMESIZE];
	int linkFd;
	char buf[BUF_SIZE];
	int readlen;
	int readFlag = 0 ;
	struct flock fifo_lock;

	fifo_lock.l_type = F_WRLCK;
	fifo_lock.l_whence = SEEK_SET;
	fifo_lock.l_start = 0;
	fifo_lock.l_len = fileSize;
    
    	long long rwpointer; /* read write pointer */
    	/*4. "{자신의 PID}FIFO1{n}2ser"와 "{자신 PID}FIFO{n}2cli"를 open한다*/
    	sprintf(fifo2SerFileName,"./%ldFIFO%d2ser",mypid,n);
    	sprintf(fifo2CliFileName,"./%ldFIFO%d2cli",mypid,n);
	//{T} 시간 측정 시작
    	while((fifo2Ser = open(fifo2SerFileName, O_WRONLY)) < 0){
        	//{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
	}
	while((fifo2Cli = open(fifo2CliFileName, O_RDONLY)) < 0){
        	//{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
    	}
	
    /*5client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.*/
    /* 파일을 병행적으로 읽는 방법 
	1. link()를 통해 링크파일을 만든다(링크파일은 read write pointer를 공유하지 않지만 원본 파일의 내용은 공유하기 때문에 파일을 병행적으로 읽는 것에 적절하다)
	2. 링크파일을 open한다.
	3. lseek()를 통해 적절한 위치로 read write pointer 이동시킨다.
    */
    sprintf(linkFileName,"codelink%ld_%d",mypid,n);
    if(link("code.txt",linkFileName)<0){
        printf("fail to call link(code.txt,%s)\n",linkFileName);
        error_handler(linkFileName);
    }
    if((linkFd=open(linkFileName,O_RDONLY))<0){
        printf("fail to call open(%s)\n",linkFileName);
        error_handler(linkFileName);
    }
    rwpointer = (fileSize/THREADNUM)*(n-1);
    if(lseek(linkFd,rwpointer,SEEK_SET)<0){
        printf("fail to call lseek(%s)\n",linkFileName);
        error_handler(linkFileName);
    }


	if (fcntl(fifo2Ser,F_SETLKW,&fifo_lock)==-1){
		printf("fail to call fcntl()\n");
		error_handler(linkFileName);
	}

    for(int i = 0 ;i < (fileSize/THREADNUM)/BUF_SIZE;i++){
        
        if(read(linkFd,buf,BUF_SIZE)<0){
            printf("fail to call read()\n");
            error_handler(linkFileName);
        }
	if(write(fifo2Ser,buf,BUF_SIZE)<0){
            printf("fail to call write()\n");
            error_handler(linkFileName);
        }
    }
    if(read(linkFd,buf,(fileSize/THREADNUM)%BUF_SIZE)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }
    if(write(fifo2Ser,buf,(fileSize/THREADNUM)%BUF_SIZE)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }
    
	fifo_lock.l_type = F_UNLCK;
	if(fcntl(fifo2Ser,F_SETLK,&fifo_lock)==-1){
		printf("fail to call write()\n");
           	error_handler(linkFileName);
	}
	

    	
	//8. client의 1번 쓰레드는 "{자신의 PID}FIFO12cli"의 내용을 읽고 출력한다.
	//9. 8이 끝나면 client의 2번 쓰레드는 "{자신의 PID}FIFO22cli"의 내용을 읽고 출력한다.
	//10. 9가 끝나면 client의 3번 쓰레드는 "{자신의 PID}FIFO32cli"의 내용을 읽고 출력한다.
	int c;
	sem_getvalue(&printer,&c);	
	while(c != (n-1)){
		sem_getvalue(&printer,&c);	
	}
	int readflag=0;
	while(readflag==0){
	 	while((readlen=read(fifo2Cli,buf,BUF_SIZE))>0){
			printf("%s",buf);
			fflush(stdout);
			readflag=1;
		}
	}
    	sem_post(&printer);
	unlink(linkFileName);
	    
}



