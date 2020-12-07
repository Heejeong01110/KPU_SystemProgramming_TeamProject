#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include<time.h>

#define BILLION 1000000000L

void error_handling(char * message)
{
	puts(message);
	exit(1);
}
int main(char argc, char * argv[])
{
	int i,num;
	pid_t pid;
	pid_t wpid;
	char filename[255];
	int status=0;

	struct timespec start, stop;
	double accum;
	if(argc != 3)
	{
		printf("Usage: %s <program> <num>",argv[0]);
		exit(1);
	}
	//생성할 프로세스의 수
	num = atoi(argv[2]);
	
	if( clock_gettime( CLOCK_MONOTONIC, &start) == -1 ) {
		perror( "clock gettime" );
		return EXIT_FAILURE;
	}
	
	//프로세스 생성
	for(i=0;i<num;i++)
	{
		pid = fork();
		if(pid == -1){
			i--;
			continue;
		}
		else if(pid == 0){//클라이언트 프로세스로 변경
			printf("%d Child process: %d\n",i,getpid());
			
			close(0);
			//close(1);
			
			execlp((const char *)argv[1],(const char *)argv[1],(char *)0);
			printf("Child execlp() faile\n");
			return 0;
		}
		else{
			continue;
		}

	}
	
	while ((wpid = wait(&status)) >= 0){}

	if( clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ) {
		perror( "clock gettime" );
		return EXIT_FAILURE;
	}
	accum = ( stop.tv_sec - start.tv_sec )
		+ (double)( stop.tv_nsec - start.tv_nsec )
		/ (double)BILLION;
	printf("%.9f sec\n", accum);

  	printf("return Parent\n");
	//system("rm ./channel/* -i"); //절대 조심 파일 다 삭제됨
	return 0;

}

