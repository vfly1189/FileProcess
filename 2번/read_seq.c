#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define R_NUM 10000
#define B_LEN 100

int main(int argc, char **argv)
{
	char buf[B_LEN];
	struct timeval startTime, endTime;
	long diffTime;
	
	int fd;
	
	gettimeofday(&startTime, NULL);

	if((fd = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s" ,argv[1]);
		exit(1);
	}

	for(int i=0; i< R_NUM; i++)
		read(fd, buf, 100);

	gettimeofday(&endTime, NULL);

	diffTime = endTime.tv_usec - startTime.tv_usec;
	printf("#records: %d timecost: %ld us\n", R_NUM, diffTime);
	return 0;
}
