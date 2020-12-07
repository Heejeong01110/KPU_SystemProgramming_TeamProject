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
	3. server는 managefifo에 있는 요청 메시지를 확인하면 3개의 FIFO 파일 "{요청자 PID}FIFO1","{요청자 PID}FIFO2","{요청자 PID}FIFO3"을 생성하고 3개의 쓰레드를 각각의 파일에 할당한다.
	4. client는 3개의 쓰레드를 할당하고 각 쓰레드는 순서에 따라 "{자신의 PID}FIFO1","{자신의 PID}FIFO2","{자신의 PID}FIFO3"를 open한다 만약 파일이 없다면 T시간 동안 스핀하며 대기한다.(T시간이 지나면 프로세스를 종료한다.)
	5. client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}" 파일에 write한다. 마지막에는 "request END"를 추가한다.(서버 쓰레드와 FIFO 클라이언트 쓰레드가 1대1대1 대응이므로 마지막 메시지에는 식별정보 필요없음, n = 쓰레드 번호)
	6. server의 각 쓰레드는 "request {요청자 PID} END"를 읽을 때까지 "{요청자 PID}FIFO{n}" 파일을 읽고 "{요청자 PID}FIFO{n}temp"파일에 임시 저장한다.
	7. server의 각 쓰레드는 "{요청자 PID}temp{n}" 읽고 복호화하여 "{요청자 PID}FIFO{n}" 파일에 write한다. 마지막에는 "response END"를 추가한다.(서버 쓰레드와 FIFO 클라이언트 쓰레드가 1대1대1 대응이므로 마지막 메시지에는 식별정보 필요없음)
	8. client의 1번 쓰레드는 "{자신의 PID}FIFO1"의 내용을 "response END"를 읽을 때까지 출력한다.
	9. client의 2번 쓰레드는 "{자신의 PID}FIFO2"의 내용을 "response END"를 읽을 때까지 "{자신의 PID}temp2" 파일에 저장하고 1번 쓰레드의 출력이 끝나면 "{자신의 PID}temp2"의 내용을 출력한다.
	10. client의 3번 쓰레드는 "{자신의 PID}FIFO3"의 내용을 "response END"를 읽을 때까지 "{자신의 PID}temp3" 파일에 저장하고 2번 쓰레드의 출력이 끝나면 "{자신의 PID}temp3"의 내용을 출력한다.
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

#define THREADNUM 3
#define BUF_SIZE 4096
#define T 5
#define FILENAMESIZE 255

void* filesend(void* th);
pthread_t thread[3];
long mypid; /*main에서 한 번 write하고 쓰레드에서는 읽기만 함으로 동기화 필요없음 */
struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
long long fileSize;
int codeFd; /*code.txt 파일의 파일디스크립터*/

/* 이 함수를 사용하면 에러처리를 위해 프로그램을 exit() 시키기 전에 FIFO파일이나 link파일을 삭제시켜줄 수 있다.*/
void error_handler(char * linkFileName){
    unlink(linkFileName);
    exit(1);
}

//client
int main()
{
    char buf[BUF_SIZE];
    int protocol;

	memset(buf,0x00,BUF_SIZE);
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

    pthread_exit(1);
}

void* filesend(void* arg){ 
    int n = (int)arg;
    char fileName[FILENAMESIZE];
    char linkFileName[FILENAMESIZE];
    int fifoFd;
    int linkFd;
    char buf[BUF_SIZE];
    
    long long rwpointer; /* read write pointer */
    /*4. "{mypid}FIFO{n}"를 open한다 만약 파일이 없다면 T시간 동안 스핀하며 대기한다.*/
    sprintf(fileName,"./%ldFIFO%d",mypid,n);
    //{T} 시간 측정 시작
    while((fifoFd = open(fileName, O_APPEND | O_RDWR)) < 0){
        //{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
    }
	printf("%s opened 파일 디스크립터 = %d\n",fileName,fifoFd);
    /*5. client의 각 쓰레드는 "code.txt"파일을 3분할{rwpointer = (filesize/3)*(n-1)}하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}" 파일에 write한다. 마지막에는 "request END"를 추가한다.*/
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

    for(int i = 0 ;i < (fileSize/THREADNUM)/BUF_SIZE;i++){
        
        if(read(linkFd,buf,BUF_SIZE)<0){
            printf("fail to call read()\n");
            error_handler(linkFileName);
        }
	if(write(fifoFd,buf,BUF_SIZE)<0){
            printf("fail to call write()\n");
            error_handler(linkFileName);
        }else{
		//printf("send to %s: %s\n",fileName,buf);
	}
    }
    if(read(linkFd,buf,(fileSize/THREADNUM)%BUF_SIZE)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }
    if(write(fifoFd,buf,(fileSize/THREADNUM)%BUF_SIZE)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }else{
	//printf("send to %s: %s\n",fileName,buf);
    }
    if(write(fifoFd,"request END",11)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }else{
	printf("send to %s: request END\n",fileName,buf);
    }

    //8. client의 1번 쓰레드는 "{자신의 PID}FIFO1"의 내용을 "response END"를 읽을 때까지 출력한다.
    //9. client의 2번 쓰레드는 "{자신의 PID}FIFO2"의 내용을 "response END"를 읽을 때까지 "{자신의 PID}temp2" 파일에 저장하고 1번 쓰레드의 출력이 끝나면 "{자신의 PID}temp2"의 내용을 출력한다.
    //10. client의 3번 쓰레드는 "{자신의 PID}FIFO3"의 내용을 "response END"를 읽을 때까지 "{자신의 PID}temp3" 파일에 저장하고 2번 쓰레드의 출력이 끝나면 "{자신의 PID}temp3"의 내용을 출력한다.

    if(unlink(linkFileName)<0){
        printf("fail to call unlink(%s)\n",linkFileName);
        error_handler(linkFileName);
    }
    
}



