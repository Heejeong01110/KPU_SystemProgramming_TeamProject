#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/stat.h>

#define  KEY_NUM   60043 /*manage 공유메모리 키값 */
#define  MEM_SIZE  4096
#define THREADNUM 3 /*스레드 수*/
#define THREADPERWORK 3
#define BUF_SIZE 4096
#define FILENAMESIZE 200
#define null NULL

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
};

struct threadArg{
	int tshmid; //스레드 순서
    long long fileSize; //파일 사이즈

	int cCnt; //클라이언트 수
};

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
void signalhandler(int sig) {
	//error_handler("./managefifo",null,null,null);
	exit(0);
}

void decoding(char code[],int len)
{
	for(int i = 0 ; i<len;i++)
	{
		code[i] = code[i]-1;
	}
}
static int SharedMemoryManageCreate(int kNum);
static int SharedMemoryCreate(int kNum);
static int SharedMemoryWrite(int shmid, char *sMemory, int size,int i);
static int SharedMemoryRead(int shmid,char *sMemory,int i);
static int SharedMemoryFree(int shmid);
static int SharedMemoryManageRead(int shmid,char *sMemory);

int clientCnt=0;
void* filerecv(void* arg);
pthread_t thread[3];

int main(void){
    char buf[BUF_SIZE];
    int protocol;
    char * request[3];
	int readlen=0;
	struct threadArg * argument;
    //unlink("./managefifo");
    printf("시작 전\n");
    /*1. server가 managefifo file을 생성한다.*/
    if(mkfifo("./managefifo",0666) == -1){ //fifo init
        printf("fail to make fifo manage()\n");
        exit(0);
        //error_handler("./managefifo",null,null,null);
    }
    signal(SIGINT, signalhandler);
    
   while(1){
        /*3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고*/
    	if((protocol = open("./managefifo", O_RDONLY)) < 0){ //waiting in client
        	printf("fail to call open manage()\n");
            exit(0);
        	//error_handler("./managefifo",null,null,null);
    	}
        if((readlen=read(protocol, buf, BUF_SIZE)) < 0 ){ 
		printf("fail to call read()");
        exit(0);
		//error_handler("./managefifo",null,null,null);
        }
	if(readlen<7) /*읽은 값이 "request"의 길이인 7보다 작으면 while문 재시작*/
		continue;
	requestPasing(request,buf);
	
	printf("%s %s %s 처리 시작\n",request[0],request[1],request[2]);
	
	for(int i = 0 ;i<THREADPERWORK;i++){
		argument = (struct threadArg *)malloc(sizeof(struct threadArg));
		argument->tshmid=atoi(request[1]);
		argument->fileSize=atoll(request[2]);
        argument->cCnt= i+1;
		pthread_create(&thread[i],NULL,filerecv, (void *)argument); //실패 시 에러처리 추가
		
	}
	memset(buf,0x00,BUF_SIZE);
	close(protocol);
    }
    unlink("./managefifo");
    pthread_exit(0);
}
/*
struct threadArg{
	int tshmid; //스레드 순서
    long long fileSize; //파일 사이즈
	int cCnt; //클라이언트 수
};
*/

void* filerecv(void * arg){
    struct threadArg * argument = (struct threadArg*)arg;
	int threadId = argument->cCnt; //스레드
    long long fileSize = argument->fileSize;
    int cCnt = argument->tshmid; //프로세스

    int tshmId = cCnt*100 + threadId*10;
    char recvbuf[MEM_SIZE];
	char sendbuf[MEM_SIZE];

    char tempFileName[FILENAMESIZE];
	int tempfd;
    char buf[BUF_SIZE];
	int readlen;

    sprintf(tempFileName,"./%dtemp.txt",tshmId);

	/*3. 각 쓰레드는 1개의 shm 생성.*/
    int stoctid = SharedMemoryCreate(tshmId+1);
    int ctostid = SharedMemoryCreate(tshmId+2);
    printf("thread%d stoc %d\n",threadId,stoctid);
    fflush(stdout);

    /*6. server의 각 쓰레드는 "{요청자 PID}FIFO{n}" 파일을 읽고 복호화하여 "{요청자 PID}FIFO{n}temp.txt"파일에 임시 저장한다.*/
	
	if((tempfd = open(tempFileName,O_RDWR|O_CREAT))<0){ 
        	printf("fail to call open manage()\n");
            exit(0);
		    //error_handler("./managefifo",fifo2SerFileName,fifo2CliFileName,tempFileName);
    	}
	
    int i=0;
    
    //sleep(1);
    while(1){
        if(SharedMemoryRead(ctostid,buf,i)){
            break;
        }
        
    }
    
    
    
    printf("thread%d first read  버퍼 : %s",threadId,buf);
    fflush(stdout);

    decoding(buf,fileSize/THREADNUM);
    write(tempfd,buf,fileSize/THREADNUM);


    /*
	for(i = 0 ;i < (fileSize/THREADNUM)/BUF_SIZE;i++){
        SharedMemoryRead(ctostid,buf,i);
        printf("%s 성공\n",buf);
        decoding(buf,BUF_SIZE);
        write(tempfd,buf,BUF_SIZE);
    }*/
    
    /* 7. server의 각 쓰레드는 "{요청자 PID}FIFO{n}temp.txt" 읽고 "{요청자 PID}FIFO{n}" 파일에 writelock를 건 후 write한다.*/
	lseek(tempfd,0,SEEK_SET);
	int totallen=0;
    i=0;

    read(tempfd,buf,fileSize/THREADNUM);
	

	/*
    while((readlen=read(tempfd,buf,BUF_SIZE))>0){
		SharedMemoryWrite(stoctid,buf,BUF_SIZE,i++); 
		totallen+=readlen;
		
	}*/
    printf("read temp file thread%d 버퍼: %s\n",threadId,buf);
    fflush(stdout);

    SharedMemoryWrite(stoctid,buf,fileSize/THREADNUM,i++); 

    //SharedMemoryFree(tid);
    pthread_exit(0);

}

static int SharedMemoryManageCreate(int kNum) //manage는 무조건 서버에서 새로생성할 필요가 있는거같다
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환 
        //클라이언트 측에서 이미 생성
        shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| 0666);
        if(shmid == -1)
        {
            printf("error1");
            return -1;
        }
        else
        {   
            SharedMemoryFree(shmid);
            shmid = shmget(kNum, MEM_SIZE, IPC_CREAT| 0666);
            if(shmid == -1)
            {
                printf("error2");
                return 1;
            }
            return shmid;
        }
    }
    return shmid;
}





static int SharedMemoryCreate(int kNum) //성공시 shmid 리턴
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환 
        //클라이언트 측에서 이미 생성
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
        }else{
            //printf("클측 생성 연결 성공%d\n",kNum);
            

            return shmid;
        }
    }
    SharedMemoryWrite(shmid,"0",1,1);
    //서버측에서 정상적으로 만든경우
    //printf("서버측 생성 연결 성공%d\n",kNum);
    return shmid;
}
 
static int SharedMemoryWrite(int shmid, char *sMemory, int size,int i)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        //연결 될때까지 무한루프
    }else{
        //sprintf((char*)&shmaddr[BUF_SIZE*i],sMemory);
        sprintf((char*)shmaddr,sMemory);
        //printf("보내진거 :%s\n",shmaddr);
        if(shmdt(shmaddr) == -1)
        {
            printf("error3");
        }
        return 0;
    }
    return 0;
}

 
static int SharedMemoryRead(int shmid,char *sMemory,int i)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
    else{
        //sprintf(sMemory,&shmaddr[BUF_SIZE*i]);
        
        sprintf(sMemory,shmaddr);
        if(sMemory[0]=='0'){
            return 0;
        }
        if(shmdt(shmaddr) == -1)
        {
            printf("error4");
        }
        return 1;
    }
    return 0;
}

 
static int SharedMemoryFree(int shmid)
{
    if(shmctl(shmid, IPC_RMID, 0) == -1) 
    {
        //perror("Shmctl failed");
        return 1;
    }
    //printf("Shared memory end\n");
    return 0;
}


static int SharedMemoryManageRead(int shmid,char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
    else{
        sprintf(sMemory,shmaddr);
        sprintf(shmaddr,"\0");
        if(shmdt(shmaddr) == -1)
        {
            printf("error4");
        }
        return 0;
    }
    return 0;
}
