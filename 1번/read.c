#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFLEN 1024

int main(int argc, char* argv[])
{
	char tmp;
	char buf[BUFLEN];
	int fd;
	int offset, read_byte;
	int index = 0;

	if(argc != 4)
	{
		fprintf(stderr, "usage : %s <file> <offset> <read_Byte>\n",argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	offset = atoi(argv[2]);
	read_byte = atoi(argv[3]);

	lseek(fd, offset, SEEK_CUR);
	
	while(read(fd, &tmp, 1) > 0 && read_byte > 0)
	{
		buf[index++] = tmp;
		read_byte--;
	}
	buf[index] = 0;
	printf("%s",buf);
}
