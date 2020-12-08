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
#define MSG_SIZE 80000
#define BUF_SIZE 4096
#define FILENAMESIZE 255
#define null NULL
#define THREADPERWORK 3
int clientCnt = 0;
void* filerecv(void* arg);
pthread_t thread[30000];
int workNum = 0;
struct msgbuf { 
    long mtype; /* message type, must be > 0 */ 
    char mtext[BUF_SIZE]; /* message data */
};
//sem_t printer;
/*typedef struct tTHREAD
{
   char ddmsg[MSG_SIZE];
   int cCnt;
}THREAD;
*/
struct threadArg {
   char fifoFileName[FILENAMESIZE];
   long long fileSize;
   int number;
};
void error_handler(char* linkFileName) {
   //  unlink(linkFileName);
   perror("handler err\n");
   exit(1);
}
void decoding(char code[], int len)
{
   for (int i = 0; i < len; i++)
   {
      code[i] = code[i] - 1;
   }
}
void requestPasing(char* request[3], char buf[])
{
   int cnt = 0;
   request[0] = &buf[0];
   for (int i = 0; i < BUF_SIZE; i++) {
      if (cnt > 2)
         break;
      if (buf[i] == ' ') {
         buf[i] = '\0';
         cnt++;
         request[cnt] = &buf[i + 1];
      }
   }
}
void signalhandler(int sig) {
//   error_handler("./managefifo",null,null,null);
   exit(0);
}
int readRequest(int fd, char buf[])
{
   int i = 0;
   int readlen;
   while((readlen=read(fd, &buf[i++], 1))>0){
      if(buf[i-1]=='\n'){
         return i-1;
      }
   }
   //printf("i = %d\n",i);
   return i-1;
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
   //char buf2[BUF_SIZE];
   int protocol;
   int readlen;
   char* request[3];
   //int readlen=0;
   struct threadArg* argument;
   memset(buf, 0x00, BUF_SIZE);
   unlink("./managefifo");
   if(mkfifo("./managefifo",0666) == -1){ //fifo init
        printf("fail to make fifo manage()\n");
      unlink("./managefifo");
        //error_handler("./managefifo",null,null,null);
    }
   signal(SIGINT, signalhandler);
   //sem_init(&printer, 0, 0);   
   /*key_id = msgget((key_t)60041, IPC_CREAT | 0666); //send queue
   key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //recv queue*/
   //key_id3= msgget((key_t)60043, IPC_CREAT|0666); //result queue
   printf("시작 전\n");
   if((protocol = open("./managefifo", O_RDONLY)) < 0){ //waiting in client
           printf("fail to call open manage()\n");
         unlink("./managefifo");
         exit(0);
           //error_handler("./managefifo",null,null,null);
       }
   while(1){
        /*3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고*/
       /*if((readlen=read(protocol, buf, BUF_SIZE)) < 0 ){ 
      printf("fail to call read()");
      //error_handler("./managefifo",null,null,null);
        }*/
        if((readlen=readRequest(protocol, buf)) < 0 ){ 
      printf("fail to call read()");
      //error_handler("./managefifo",null,null,null);
        }
   if(readlen<7) /*읽은 값이 "request"의 길이인 7보다 작으면 while문 재시작*/
      continue;
   requestPasing(request,buf);
   // requestPasing(request,buf2);
   for (int i = 0; i < THREADPERWORK; i++) {
      argument = (struct threadArg*)malloc(sizeof(struct threadArg));
      snprintf(argument->fifoFileName,FILENAMESIZE-1, "./%sFIFO%d", request[1], i + 1);
      argument->number = i + 1;
      argument->fileSize = atoll(request[2]);
      pthread_create(&thread[i+workNum*3], NULL, (void*)filerecv, (void*)argument/*, (void*)& recvbuf[i]*/);
   }
   workNum++;
   memset(buf, 0x00, BUF_SIZE);
   lseek(protocol, 0, SEEK_SET);
   // memset(buf2,0x00,BUF_SIZE);
   /*for (int i = 0; i <= THREADPERWORK; i++)
   {
      pthread_join(thread[i], NULL);
   }*/
   //sem_destroy(&threadrock);
   //unlink("./managefifo");
   /*if (msgctl(key_id2, IPC_RMID, NULL) == -1) {
      printf("msgctl failed\n");
      exit(1);
   }*/
   }
   close(protocol);
   unlink("./managefifo");
   pthread_exit(0);
}
void* filerecv(void* arg) {
   int fifo2Ser;
   int fifo2Cli;
   //sender_id = getpid();
   int fd;
   char buf[BUF_SIZE];
   //char buf2[BUF_SIZE];
   char tempFileName[FILENAMESIZE];
   char fifo2SerFileName[FILENAMESIZE];
   char fifo2CliFileName[FILENAMESIZE];
   int tempfd;
   int readlen = 0;
   key_t sendkey, recvkey;
   struct msgbuf mybuf;
   /*매개변수 저장*/
   struct threadArg* argument = (struct threadArg*)arg;
   recvkey = msgget((key_t)60040 + (key_t)argument->number, IPC_CREAT | 0666); //recv queue
   sendkey = msgget((key_t)60040 + (key_t)(argument->number + THREADPERWORK), IPC_CREAT | 0666); //recv queue
   long long fileSize = argument->fileSize;
   snprintf(fifo2SerFileName,FILENAMESIZE-1, "%s2ser", argument->fifoFileName);
   snprintf(fifo2CliFileName,FILENAMESIZE-1, "%s2cli", argument->fifoFileName);
   snprintf(tempFileName, FILENAMESIZE-1,"./%stemp.txt", argument->fifoFileName);
   
   if ((tempfd = open(tempFileName, O_RDWR | O_CREAT,0777)) < 0) {
      printf(" ");
      exit(0);
   }
   for (int i = 0; i < (fileSize / 3) / BUF_SIZE; i++) {
      if ((readlen=msgrcv(recvkey, (void*)&mybuf, BUF_SIZE, 0, MSG_NOERROR)) < 0) {
         printf("fafil to call read()\n");
         //error_handler(linkFileName);
   exit(0);
      }
         decoding(mybuf.mtext, readlen);
   mybuf.mtext[BUF_SIZE-1]='\0';
         write(tempfd, mybuf.mtext, readlen);      
   }
   
   readlen = (fileSize/3)%BUF_SIZE;
   //printf("readlen = %d",readlen);
   if (msgrcv(recvkey, (void*)&mybuf, readlen, 0, MSG_NOERROR) == -1) {
      perror("msgrcv error");
      exit(1);
   }
   mybuf.mtext[readlen-1]='\0';
   decoding(mybuf.mtext, readlen-1);
   write(tempfd, mybuf.mtext, readlen);

   lseek(tempfd, 0, SEEK_SET);
      mybuf.mtype = 1;
  while((readlen=read(tempfd, buf, BUF_SIZE)) > 0) {
      sprintf(mybuf.mtext,"%s",buf);
      //write(1, mybuf.mtext, readlen);
      if (msgsnd(sendkey, (void*)&mybuf, readlen, 0) == -1) { //fifo rw
         printf("fail to call msgsnd()\n");
         exit(1);
      }
     
   }



   msgctl(recvkey,IPC_RMID, 0);
   msgctl(sendkey,IPC_RMID, 0);
}