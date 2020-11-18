#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_NUM 6

void * decryption(void * num)
{
	int n = (int)num;
	printf("%d",n);
	pthread_exit(0);
}
int main(void)
{
	pthread_t th[THREAD_NUM];

	for(int i = 0;i<THREAD_NUM;i++)
	{
		pthread_create(&th[i],NULL,decryption,(void *)i);
	}

	
	for(int i = 0;i<THREAD_NUM;i++)
	{
	}

	pthread_exit(0);	
}
