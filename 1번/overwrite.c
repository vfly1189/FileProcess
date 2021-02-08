#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFLEN 1024

int main(int argc, char* argv[])
{
	char input[BUFLEN];
	char buf[BUFLEN];
	int fd;
	int offset;

	if(argc != 4)
	{
		fprintf(stderr, "usage : %s <filename> <offset> <note>\n",argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_WRONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	offset = atoi(argv[2]);
	lseek(fd, offset, SEEK_CUR);

	int i=0;
	for(i=0; argv[3][i] != '\0'; i++)
		buf[i] = argv[3][i];
	buf[i] = 0;
	write(fd, buf, i);
}

