#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "flash.h"
#include "fdevicedriver.c"
// 필요한 경우 헤더파일을 추가한다

FILE *flashfp;	// fdevicedriver.c에서 사용

//
// 이 함수는 FTL의 역할 중 일부분을 수행하는데 물리적인 저장장치 flash memory에 Flash device driver를 이용하여 데이터를
// 읽고 쓰거나 블록을 소거하는 일을 한다 (동영상 강의를 참조).
// flash memory에 데이터를 읽고 쓰거나 소거하기 위해서 fdevicedriver.c에서 제공하는 인터페이스를
// 호출하면 된다. 이때 해당되는 인터페이스를 호출할 때 연산의 단위를 정확히 사용해야 한다.
// 읽기와 쓰기는 페이지 단위이며 소거는 블록 단위이다.
// 

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

int main(int argc, char *argv[])
{	
	char sectorbuf[SECTOR_SIZE];
	char pagebuf[PAGE_SIZE];
	char sparebuf[SPARE_SIZE];
	char *blockbuf;

	int num_of_blocks;
	char file_name[1000];

	int ppn;
	int pbn;
	
	if(!strcmp(argv[1], "c"))
	{
		strcpy(file_name, argv[2]);
		num_of_blocks = atoi(argv[3]);

		FILE *fp;
		fp = fopen(file_name, "w");

		blockbuf = (char *)malloc(BLOCK_SIZE);
		memset((void *)blockbuf, (char)0xff, BLOCK_SIZE);
		for(int i=0; i<num_of_blocks; i++)
			fwrite(blockbuf, BLOCK_SIZE, 1, fp);
		fclose(fp);
	}
	else if(!strcmp(argv[1], "w"))
	{
		char buf[4][PAGE_SIZE];

		for(int i=0; i<4; i++)
			memset(buf[i], (char)0xff, PAGE_SIZE);

		strcpy(file_name, argv[2]);
		ppn = atoi(argv[3]);

		memset((void *)pagebuf, (char)0xff, PAGE_SIZE);
		memset((void *)sparebuf, (char)0xff, SPARE_SIZE);
		memset((void *)sectorbuf, (char)0xff, SECTOR_SIZE);


		strncpy(sectorbuf, argv[4], strlen(argv[4]));
		strncpy(sparebuf, argv[5], strlen(argv[5]));

		memcpy(pagebuf, sectorbuf, sizeof(sectorbuf));
		char *p = pagebuf;
		p = p + SECTOR_SIZE;
		memcpy(p, sparebuf, sizeof(sparebuf));

		flashfp = fopen(file_name, "r+");


		fseek(flashfp, (ppn/4) * BLOCK_SIZE, SEEK_SET);

		for(int i=0; i<4; i++)
		{
			if(i == (ppn%4)) continue;
			else
			{
				fread(buf[i], PAGE_SIZE, 1, flashfp);
			}
		}

		dd_erase(ppn/4);

		for(int i=0; i<4; i++)
			dd_write((ppn/4) * 4 + i, buf[i]);

		dd_write(ppn, pagebuf);
	}
	else if(!strcmp(argv[1], "r"))
	{
		strcpy(file_name, argv[2]);
		ppn = atoi(argv[3]);

		flashfp = fopen(file_name, "r");
		dd_read(ppn, pagebuf);


		int flag = 0;
		for(int i=0; i<PAGE_SIZE; i++)
		{
			if(i == SECTOR_SIZE - 1 && flag == 1)
				printf(" ");

			if(pagebuf[i] == -1)
				continue;
			else
			{
				if(flag == 0) flag = 1;
				printf("%c", pagebuf[i]);
			}
		}
		if(flag == 1) printf("\n");
	}
	else if(!strcmp(argv[1], "e"))
	{
		strcpy(file_name, argv[2]);
		pbn = atoi(argv[3]);

		flashfp = fopen(file_name, "r+");
		dd_erase(pbn);
	}


	// flash memory 파일 생성: 위에서 선언한 flashfp를 사용하여 flash 파일을 생성한다. 그 이유는 fdevicedriver.c에서 
	//                 flashfp 파일포인터를 extern으로 선언하여 사용하기 때문이다.
	// 페이지 쓰기: pagebuf의 섹터와 스페어에 각각 입력된 데이터를 정확히 저장하고 난 후 해당 인터페이스를 호출한다
	// 페이지 읽기: pagebuf를 인자로 사용하여 해당 인터페이스를 호출하여 페이지를 읽어 온 후 여기서 섹터 데이터와
	//                  스페어 데이터를 분리해 낸다
	// memset(), memcpy() 등의 함수를 이용하면 편리하다. 물론, 다른 방법으로 해결해도 무방하다.

	return 0;
}
