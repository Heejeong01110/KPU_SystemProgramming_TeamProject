#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void)
{
	struct stat sb;
	stat("./code.txt",&sb);
	printf("file size = %lld byte\n",(long long)sb.st_size);
}
