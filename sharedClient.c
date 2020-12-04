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
#define  MEM_SIZE  4096
#define THREADNUM 3
#define BUF_SIZE 4096
#define MAX_SIZE 4096
#define T 5
#define FILENAMESIZE 255
#define BUF_SIZE 4096
#define null NULL

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
};

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

static int SharedMemoryManageInit(int kNum);
static int SharedMemoryInit(int kNum);
static int SharedMemoryWrite(int shmid,char *sMemory, int size,int i);
static int SharedMemoryManageWrite(int shmid,char *sMemory, int size);
static int SharedMemoryRead(int shmid,char *sMemory,int i);
static int SharedMemoryFree(int shmid);

union semun sem_union;
    int sem_num=1;
    int sem_id;
    struct sembuf mysem_open  = {0, -1, SEM_UNDO}; // 세마포어 얻기
struct sembuf mysem_close = {0, 1, SEM_UNDO};  // 세마포어 돌려주

void* filesend(void* th);
pthread_t thread[3];

int main() {

	char buf[BUF_SIZE];
    int protocol;

	memset(buf,0x00,BUF_SIZE);
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

    
    //int threadShmid = 10 + (int)arg;//10을 서버에서 받아오게 수정
    int n = (int)arg;
    char recvbuf[MEM_SIZE];
	char sendbuf[MEM_SIZE];
    int threadId = (int)mypid;
    int cCnt = (int)arg; //스레드
    int tshmId = threadId*100 + cCnt*10;
    char linkFileName[FILENAMESIZE];
    int linkFd;
	char buf[BUF_SIZE];
	int readlen;
	int readFlag = 0 ;
    long long rwpointer; /* read write pointer */
    fflush(stdout);
    
    /*3. 각 쓰레드는 1개의 shm 생성.*/
    int stoctid = SharedMemoryInit(tshmId+1);
    int ctostid = SharedMemoryInit(tshmId+2);
    printf("thread%d stoc %d\n",cCnt,stoctid);
    fflush(stdout);

        /*5client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.*/
    /* 파일을 병행적으로 읽는 방법 
	1. link()를 통해 링크파일을 만든다(링크파일은 read write pointer를 공유하지 않지만 원본 파일의 내용은 공유하기 때문에 파일을 병행적으로 읽는 것에 적절하다)
	2. 링크파일을 open한다.
	3. lseek()를 통해 적절한 위치로 read write pointer 이동시킨다.
    */
    
    sprintf(linkFileName,"codelink%ld_%d",mypid,n);
    if(link("code.txt",linkFileName)<0){
        printf("fail to call link(code.txt,%s)\n",linkFileName);
        //error_handler(linkFileName);
        exit(0);
    }
    
    
    if((linkFd=open(linkFileName,O_RDONLY))<0){
        printf("fail to call open(%s)\n",linkFileName);
        //error_handler(linkFileName);
        exit(0);
    }
    rwpointer = (fileSize/THREADNUM)*(n-1);
    if(lseek(linkFd,rwpointer,SEEK_SET)<0){
        printf("fail to call lseek(%s)\n",linkFileName);
        //error_handler(linkFileName);
        exit(0);
    }

	int totalfilereadlen = 0; //파일 사이즈 알 필요없음
    int i=0;
    if((readlen=read(linkFd,buf,fileSize/THREADNUM))<0){
            printf("fail to call read()\n");
            //error_handler(linkFileName);
            exit(0);
        }

    
    SharedMemoryWrite(ctostid,buf,fileSize/THREADNUM,i);
    printf("thread%d first write  버퍼 : %s",threadId,buf);
    fflush(stdout);
    /*
    for(i = 0 ;i < (fileSize/THREADNUM)/BUF_SIZE;i++){
        if((readlen=read(linkFd,buf,BUF_SIZE))<0){
            printf("fail to call read()\n");
            //error_handler(linkFileName);
            exit(0);
        }
        //message send
        SharedMemoryWrite(ctostid,buf,BUF_SIZE,i);
	    
	totalfilereadlen += readlen;
    }
    if((readlen=read(linkFd,buf,(fileSize/THREADNUM)%BUF_SIZE))<0){ 
        printf("fail to call read()\n");
        //error_handler(linkFileName);
        exit(0);
    }
    printf("버퍼 test2 : %s\n",buf);
    fflush(stdout);

	SharedMemoryWrite(ctostid,buf,fileSize/THREADNUM,i);
    */


    //8. client의 1번 쓰레드는 "{자신의 PID}FIFO12cli"의 내용을 읽고 출력한다.
	//9. 8이 끝나면 client의 2번 쓰레드는 "{자신의 PID}FIFO22cli"의 내용을 읽고 출력한다.
	//10. 9가 끝나면 client의 3번 쓰레드는 "{자신의 PID}FIFO32cli"의 내용을 읽고 출력한다.
    //sleep(10);
	pthread_mutex_lock(&printlock);
	if(n==1)
		printf("\n\nThread1 Print Start\n");
	if((n==2)){
		if(endNum!=1)
			pthread_cond_wait(&printer2,&printlock);
		printf("\n\nThread2 Print Start\n");
	}
	else if((n==3)){
		if((endNum!=2))
			pthread_cond_wait(&printer3,&printlock);
		printf("\n\nThread3 Print Start\n");
	}
	
	int totallen=0;
    i=0;

    /*
    for(i = 0 ;i < (fileSize/THREADNUM)/BUF_SIZE;i++){
        SharedMemoryRead(stoctid,buf,i);
        totallen+=write(1,buf,readlen);
    }*/
    
    while(1){
        if(SharedMemoryRead(stoctid,buf,i)){
            break;
        }
    }
    //SharedMemoryRead(stoctid,buf,i)
    
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
	printf("filesize = %d Thread %d, fileread: %d byte, rcv: %d byte\n",fileSize,n,totalfilereadlen, totallen);
	unlink(linkFileName);


}

static int SharedMemoryManageInit(int kNum)
{
    int shmid; 
    if((shmid = shmget((key_t)kNum, 0, 0))==-1)
    {
        //perror("write Shmat failed");
    }else{
        return shmid;
    }
    return 0;
}


static int SharedMemoryInit(int kNum)
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //새로 만드는 경우
        //서버 측에서 이미 공유메모리 생성
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
            return shmid;
        }
    }
    SharedMemoryWrite(shmid,"0",1,1);
    //클에서 만듦
    //printf("클 생성해서 연결 성공%d\n",kNum);  
    return shmid;
}

static int SharedMemoryManageWrite(int shmid, char *sMemory, int size)
{
    void *shmaddr;
    char temp[MEM_SIZE];
    char* recvM = temp;
    //printf("before write :%s",sMemory);
        if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
        else{

        if(semop(sem_id, &mysem_open, 1) == -1)
        {
        //perror("semop error ");
        exit(0);
        }

        sprintf(recvM,shmaddr);

        
        
        if(strcmp(recvM,"hello")==0){
            semop(sem_id, &mysem_close, 1);

            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
            }
            return 0;
           
        }else{
            sprintf((char*)shmaddr,sMemory);
            semop(sem_id, &mysem_close, 1);
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
            }
            return 1;
        }
        }
}

static int SharedMemoryWrite(int shmid, char *sMemory, int size,int i)
{
    void *shmaddr;
    //printf("before write :%s",sMemory);
        if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
        else{
     
            sprintf((char*)shmaddr,sMemory);

       
            //printf("after write :%s",(char*)shmaddr);
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
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
            //printf("받은버퍼 :%s\n",(char*)shmaddr);
            sprintf(sMemory,shmaddr);
            if(sMemory[0]=='0'){
                return 0;
            }
            //printf("받은 문자열 :%s\n",sMemory);
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
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
