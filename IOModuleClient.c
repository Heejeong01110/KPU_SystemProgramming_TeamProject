#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

#define THREAD_NUM 3

void * reader(void * i)
{
	int n = (int)i;
	printf("%d",n);
	pthread_exit(0);
}
int main(void)
{
	pthread_t th[THREAD_NUM];
	
	for(int i = 0 ; i<THREAD_NUM; i++)
	{
		pthread_create(&th[i],NULL,reader,(void *)i);
	}

	pthread_exit(0);
}
