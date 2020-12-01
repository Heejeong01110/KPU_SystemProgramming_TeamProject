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
	3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고, 각 쓰레드는 3개의 "{요청자 PID}FIFO{n}" FIFO 파일을 생성한다.  
	4. client는 3개의 쓰레드를 할당하고 각 쓰레드는 순서에 따라 "{자신의 PID}FIFO1","{자신의 PID}FIFO2","{자신의 PID}FIFO3"를 open한다 만약 파일이 없다면 T시간 동안 스핀하며 대기한다.(T시간이 지나면 프로세스를 종료한다.)
	5. client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}" 파일에 write한다. 마지막에는 "request END"를 추가한다.(서버 쓰레드와 FIFO 클라이언트 쓰레드가 1대1대1 대응이므로 마지막 메시지에는 식별정보 필요없음, n = 쓰레드 번호)
	6. server의 각 쓰레드는 "request {요청자 PID} END"를 읽을 때까지 "{요청자 PID}FIFO{n}" 파일을 읽고 "{요청자 PID}FIFO{n}temp.txt"파일에 임시 저장한다.
	7. server의 각 쓰레드는 "{요청자 PID}temp{n}" 읽고 복호화하여 "{요청자 PID}FIFO{n}" 파일에 write한다. 마지막에는 "response END"를 추가한다.(서버 쓰레드와 FIFO 클라이언트 쓰레드가 1대1대1 대응이므로 마지막 메시지에는 식별정보 필요없음)
	8. client의 1번 쓰레드는 "{자신의 PID}FIFO1"의 내용을 "response END"를 읽을 때까지 출력한다.
	9. client의 2번 쓰레드는 "{자신의 PID}FIFO2"의 내용을 "response END"를 읽을 때까지 "{자신의 PID}temp2" 파일에 저장하고 1번 쓰레드의 출력이 끝나면 "{자신의 PID}temp2"의 내용을 출력한다.
	10. client의 3번 쓰레드는 "{자신의 PID}FIFO3"의 내용을 "response END"를 읽을 때까지 "{자신의 PID}temp3" 파일에 저장하고 2번 쓰레드의 출력이 끝나면 "{자신의 PID}temp3"의 내용을 출력한다.
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

#define BUF_SIZE 4096
#define FILENAMESIZE 255
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
void error_handler(char * linkFileName){
    unlink("./managefifo");
    if(linkFileName!=null)
        unlink(linkFileName);
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
        error_handler(null);
    }
    
    
   while(1){
        /*3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고*/
    	if((protocol = open("./managefifo", O_RDONLY)) < 0){ //waiting in client
        	printf("fail to call open manage()\n");
        	error_handler(null);
    	}
        if((readlen=read(protocol, buf, BUF_SIZE)) < 0 ){ 
		printf("fail to call read()");
		error_handler(null);
        }
	printf("%d\n",readlen);
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
	
	int fd;
	char buf[BUF_SIZE];
	char tempFileName[FILENAMESIZE];
	int tempfd;
	/*매개변수 저장*/
	struct threadArg * argument = (struct threadArg*)arg;

	/*3. 각 쓰레드는 3개의 "{요청자 PID}FIFO{n}" FIFO 파일을 생성한다.*/
    	if(mkfifo(argument->fifoFileName,0666) == -1){ //fifo init
    	    printf("fail to make fifo manage()\n");
    	    error_handler(argument->fifoFileName);
    	}
	if((fd = open(argument->fifoFileName,O_RDONLY))<0){ 
        	printf("fail to call open manage()\n");
        	error_handler(argument->fifoFileName);
    	}
	printf("%s opened 파일 디스크립터 = %d\n",argument->fifoFileName,fd);
	//6. server의 각 쓰레드는 "request {요청자 PID} END"를 읽을 때까지 "{요청자 PID}FIFO{n}" 파일을 읽고 "{요청자 PID}FIFO{n}temp.txt"파일에 임시 저장한다.
	sprintf(tempFileName,"./%stemp.txt",argument->fifoFileName);
	tempfd = open(tempFileName,O_RDWR|O_CREAT);
	for(int i = 0 ;i < (argument->fileSize/THREADPERWORK)/BUF_SIZE;i++){
        
        	if(read(fd,buf,BUF_SIZE)<0){
        		printf("fail to call read()\n");
        		error_handler(argument->fifoFileName);
    		}
    		
		write(tempfd,buf,strlen(buf)-1);
    	}
    	if(read(fd,buf,(argument->fileSize/THREADPERWORK)%BUF_SIZE)<0){
    	    printf("fail to call read()\n");
    	    error_handler(argument->fifoFileName);
    	}else{
		
		write(tempfd,buf,strlen(buf)-1);
	}
	
	
	//7. server의 각 쓰레드는 "{요청자 PID}temp{n}" 읽고 복호화하여 "{요청자 PID}FIFO{n}" 파일에 write한다. 마지막에는 "response END"를 추가한다.(서버 쓰레드와 FIFO 클라이언트 쓰레드가 1대1대1 대응이므로 마지막 메시지에는 식별정보 필요없음)
	unlink(argument->fifoFileName);
	unlink(tempFileName);
	
	printf("쓰레드 %d 종료\n",argument->number);
	close(fd);
	close(tempfd);
	pthread_exit(0);
}
