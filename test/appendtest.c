#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

int main(void)
{
	if(open("aa.txt",O_APPEND)<0)
	{
		printf("파일이 없으면 APPEND옵션으로 안열림~");
	}
}
