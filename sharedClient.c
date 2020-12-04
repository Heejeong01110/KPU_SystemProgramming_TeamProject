#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/stat.h>

#define  KEY_NUM   60043
#define THREADNUM 3
#define MAX_SIZE 4096
#define FILENAMESIZE 255
#define null NULL



void* filesend(void* th);
pthread_t thread[3];
long mypid; /*main에서 한 번 write하고 쓰레드에서는 읽기만 함으로 동기화 필요없음 */
struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
long long afileSize;
pthread_cond_t printer1;
pthread_cond_t printer2;
pthread_cond_t printer3;
pthread_mutex_t printlock;
int endNum;


static int SharedMemoryInit(int kNum,long long fileSize);
static int SharedMemoryWrite(int shmid,char *sMemory);
static int SharedMemoryRead(int shmid,char *sMemory);
static int SharedMemoryFree(int shmid);

void error_handler(char * FileName){
    if(FileName!=null)
        unlink(FileName);
	pthread_exit(0);
}
struct threadArg{
    long long fileSize; //파일 사이즈
	int cCnt; //클라이언트 수
};


int main() {
    struct threadArg * argument;
	char buf[MAX_SIZE];
    int protocol;

	memset(buf,0x00,MAX_SIZE);
	pthread_cond_init(&printer1,NULL);
	pthread_cond_init(&printer2,NULL);
	pthread_cond_init(&printer3,NULL);
	pthread_mutex_init(&printlock,NULL);
    /*2. open managefifo file 만약 파일이 없다면 {T}시간 동안 스핀하며 대기한다.*/
    //{T} 시간 측정 시작
    while((protocol = open("./managefifo", O_APPEND | O_WRONLY)) < 0){
        //{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
    }
    
    /*2. managefifo 파일에 "request {mypid} {파일 크기}"를 write */
    /*code.txt의 상태 읽기*/
    if (stat("./codeqq.txt", &sb) == -1){ 
        printf("fail to call stat(./code,txt)\n");
        exit(1);
    }

    afileSize = (long long)sb.st_size;
    mypid = (long)getpid();
    sprintf(buf,"request %ld %lld \n",mypid,afileSize);

    long long fileS = afileSize/3 + 1;
    


    if(write(protocol, buf, strlen(buf)) < 0 ){ 
        printf("fail to call write(protocol,request)\n");
        exit(1);
    }

	write(1,buf,strlen(buf));
    close(protocol);


    /*4. 3개의 쓰레드를 할당*/
    for(int i=0;i<THREADNUM;i++){
	    //실패 시 에러처리 추가 
        argument = (struct threadArg *)malloc(sizeof(struct threadArg));
        argument->fileSize = fileS;
        argument->cCnt=i+1;
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)argument);
    }


    for(int i=0;i<THREADNUM;i++){
	//실패 시 에러처리 추가
          pthread_join(thread[i],NULL);
    }

    pthread_exit(1);
}

    

void* filesend(void* arg){ 
    struct threadArg * argument = (struct threadArg*)arg;
	int n = argument->cCnt; //스레드
    long long fileSize = argument->fileSize;
    
    //int threadShmid = 10 + (int)arg;//10을 서버에서 받아오게 수정

    char recvbuf[fileSize];
	char sendbuf[fileSize];
    int threadId = (int)mypid;
    int cCnt = n; //스레드
    int tshmId = threadId*100 + cCnt*10;
    char linkFileName[FILENAMESIZE];
    int linkFd;
	char buf[fileSize];
	int readlen;
    long long rwpointer; /* read write pointer */
    fflush(stdout);
    
    /*3. 각 쓰레드는 1개의 shm 생성.*/
    int stoctid = SharedMemoryInit(tshmId+1,fileSize);
    int ctostid = SharedMemoryInit(tshmId+2,fileSize);

    /*5client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.*/
    /* 파일을 병행적으로 읽는 방법 
	1. link()를 통해 링크파일을 만든다(링크파일은 read write pointer를 공유하지 않지만 원본 파일의 내용은 공유하기 때문에 파일을 병행적으로 읽는 것에 적절하다)
	2. 링크파일을 open한다.
	3. lseek()를 통해 적절한 위치로 read write pointer 이동시킨다.
    */
    
    sprintf(linkFileName,"codelink%ld_%d",mypid,n);
    if(link("codeqq.txt",linkFileName)<0){
        printf("fail to call link(codeqq.txt,%s)\n",linkFileName);
        error_handler(linkFileName);
        exit(0);
    }
    
    
    if((linkFd=open(linkFileName,O_RDONLY))<0){
        printf("fail to call open(%s)\n",linkFileName);
        error_handler(linkFileName);
        exit(0);
    }

    rwpointer = (fileSize)*(n-1);
    if(lseek(linkFd,rwpointer,SEEK_SET)<0){
        printf("fail to call lseek(%s)\n",linkFileName);
        error_handler(linkFileName);
        exit(0);
    }

	int totalfilereadlen = 0; //파일 사이즈 알 필요없음
    if((readlen=read(linkFd,buf,fileSize))<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
        exit(0);
    }

    SharedMemoryWrite(ctostid,buf);
 

	pthread_mutex_lock(&printlock);
	if(n==1)
		printf("\n\nThread1 Print Start\n");
        fflush(stdout);
	if((n==2)){
		if(endNum!=1)
			pthread_cond_wait(&printer2,&printlock);
		printf("\n\nThread2 Print Start\n");
        fflush(stdout);
	}
	else if((n==3)){
		if((endNum!=2))
			pthread_cond_wait(&printer3,&printlock);
		printf("\n\nThread3 Print Start\n");
        fflush(stdout);
	}
	
    
	int totallen=0;
    printf("%d - %d test3\n",tshmId,cCnt);
    fflush(stdout);
    
    while(1){
        if(SharedMemoryRead(stoctid,buf)){
            break;
        }
    }
    
    totallen+=write(1,buf,readlen);
	fflush(stdout);
	if(n==1){
		endNum=1;
		pthread_cond_signal(&printer2);
	}
	if(n==2){
		endNum=2;
		pthread_cond_signal(&printer3);
	}
	pthread_mutex_unlock(&printlock);


	printf("\nfilesize = %lld Thread %d, fileread: %d byte, rcv: %d byte\n",afileSize,n,totalfilereadlen, totallen);
    SharedMemoryFree(stoctid);
    SharedMemoryFree(ctostid);
	unlink(linkFileName);
}



static int SharedMemoryInit(int kNum,long long fileSize)
{
    int shmid;
    if((shmid = shmget((key_t)kNum, fileSize, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //새로 만드는 경우
        //서버 측에서 이미 공유메모리 생성
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
            return shmid;
        }
    }
    SharedMemoryWrite(shmid,"0");
    //클에서 만듦
    return shmid;
}


static int SharedMemoryWrite(int shmid, char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){

    }else{
        sprintf((char*)shmaddr,sMemory);

        if(shmdt(shmaddr) == -1){
            perror("close write Shmdt failed");
            return 0;
        }
        return 1;

    }
    return 0;
}


static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
    }
    else{
        sprintf(sMemory,shmaddr);
        if(sMemory[0]=='0'){
            return 0;
        }else{
            sprintf(shmaddr,'\0');
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
                return 0;
            }
            return 1;
        }
    }
    return 0;
}


static int SharedMemoryFree(int shmid)
{
    if(shmctl(shmid, IPC_RMID, 0) == -1) 
    {
        perror("Shmctl failed");
        return 1;
    }
    //printf("Shared memory end\n");
    return 0;
}
