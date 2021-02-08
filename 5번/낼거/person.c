#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "person.h"

#define BUF_SIZE 1024

//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{	
	fseek(fp, pagenum * PAGE_SIZE, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);
	return;
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, pagenum * PAGE_SIZE, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
	return;
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 그런 후 이 레코드를 저장할 페이지를 readPage()를 통해 프로그램 상에
// 읽어 온 후 pagebuf에 recordbuf에 저장되어 있는 레코드를 저장한다. 그 다음 writePage() 호출하여 pagebuf를 해당 페이지 번호에
// 저장한다. pack() 함수에서 readPage()와 writePage()를 호출하는 것이 아니라 pack()을 호출하는 측에서 pack() 함수 호출 후
// readPage()와 writePage()를 차례로 호출하여 레코드 쓰기를 완성한다는 의미이다.
// 
void pack(char *recordbuf, const Person *p)
{
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다. 이 함수가 언제 호출되는지는
// 위에서 설명한 pack()의 시나리오를 참조하면 된다.
//
void unpack(const char *recordbuf, Person *p)
{
	strcpy(p->sn, strtok((char *)recordbuf, "#"));
	strcpy(p->name, strtok(NULL, "#"));
	strcpy(p->age, strtok(NULL, "#"));
	strcpy(p->addr, strtok(NULL, "#"));
	strcpy(p->phone, strtok(NULL, "#"));
	strcpy(p->email, strtok(NULL, "#"));
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값을 구조체에 저장한 후 아래의 insert() 함수를 호출한다.
//
void insert(FILE *fp, const Person *p)
{
	char *pagebuf = (char *)calloc(PAGE_SIZE, sizeof(char));
	char *recordbuf = (char *)calloc(RECORD_SIZE, sizeof(char));
	memset(recordbuf, (char)0xff, RECORD_SIZE);
	int head_data[4] = {};
	readPage(fp, pagebuf, 0);
	memcpy((void *)head_data, (void *)pagebuf, 4 * sizeof(int));

	pack(recordbuf, p);

	if(head_data[2] == -1 && head_data[3] == -1)
	{
		memset(pagebuf, 0, PAGE_SIZE);
		readPage(fp, pagebuf, head_data[0] - 1);

		int i=0;
		for(i=0; i<PAGE_SIZE; i = i + RECORD_SIZE)
		{
			if(pagebuf[i] == -1)
				break;
		}
		if(PAGE_SIZE - i < RECORD_SIZE)
		{
			fseek(fp, 0, SEEK_END);
			long length = strlen(recordbuf);
			recordbuf[strlen(recordbuf)] = 0xff;
			memset(pagebuf, (char)0xff, PAGE_SIZE);
			memcpy((void *)pagebuf, (void *)recordbuf, length);
			writePage(fp, pagebuf, head_data[0]);

			head_data[0]++;
			head_data[1]++;
			memset(pagebuf, (char)0xff, PAGE_SIZE);
			memcpy((void *)pagebuf, (void *)head_data, 4 * sizeof(int));
			writePage(fp, pagebuf, 0);
		}
		else
		{
			long length = strlen(recordbuf);
			recordbuf[strlen(recordbuf)] = 0xff;
			memcpy((void *)(pagebuf + i), (void *)recordbuf, length);
			writePage(fp, pagebuf, head_data[0] - 1);

			head_data[1]++;
			memset(pagebuf, (char)0xff, PAGE_SIZE);
			memcpy((void *)pagebuf, (void *)head_data, 4 * sizeof(int));
			writePage(fp, pagebuf, 0);
		}
	}
	else
	{
		memset(pagebuf, (char)0xff, PAGE_SIZE);
		readPage(fp, pagebuf, head_data[2]);

		char *p = pagebuf + (head_data[3] * RECORD_SIZE);
		char mark;
		int new_inf[2] = {};

		memcpy((void *)new_inf, (void *)(p+1), 2 * sizeof(int));

		recordbuf[strlen(recordbuf)] = 0xff;
		memcpy((void *)p, (void *)recordbuf, RECORD_SIZE);
		writePage(fp, pagebuf, head_data[2]);
		memset(pagebuf, (char)0xff, PAGE_SIZE);
		head_data[1]++;
		head_data[2] = new_inf[0];
		head_data[3] = new_inf[1];
		memcpy((void *)pagebuf, (void *)head_data, 4 * sizeof(int));
		writePage(fp, pagebuf, 0);
	}
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *sn)
{
	int head_data[4] = {};
	char *pagebuf = (char *)calloc(PAGE_SIZE, sizeof(char));
	char *recordbuf = (char *)calloc(RECORD_SIZE, sizeof(char));
	const char *buf2 = (char *)calloc(BUF_SIZE, sizeof(char));
	char mark = '*';
	int buf[2] = {};
	readPage(fp, pagebuf, 0);
	memcpy((void *)head_data, (void *)pagebuf, 4 * sizeof(int));

	int i=0;
	Person tmp;
	for(i=1; i<head_data[0]; i++)
	{
		memset(pagebuf, 0, PAGE_SIZE);
		readPage(fp, pagebuf, i);
		for(int j=0; j<PAGE_SIZE; j = j+RECORD_SIZE)
		{
			if(PAGE_SIZE - j < RECORD_SIZE) break;
			memcpy((void *)recordbuf, (void *)(pagebuf + j), RECORD_SIZE);

			char tmp2;
			memcpy((void *)&tmp2, (void *)recordbuf, sizeof(char));
			if(tmp2 == '*') continue;

			memcpy((void *)buf2, (void *)recordbuf, RECORD_SIZE);
			unpack(buf2, &tmp);

			if(!strcmp(tmp.sn, sn))
			{
				buf[0] = head_data[2];
				buf[1] = head_data[3];

				memcpy((void *)recordbuf, (void *)&mark, sizeof(char));
				memcpy((void *)(recordbuf + 1), (void *)buf, 2 * sizeof(int));
				memcpy((void *)(pagebuf + j), (void *)recordbuf , RECORD_SIZE);
				writePage(fp, pagebuf, i);

				head_data[1]--;
				head_data[2] = i;
				head_data[3] = j / RECORD_SIZE;
				memset(pagebuf, (char)0xff, PAGE_SIZE);
				readPage(fp, pagebuf, 0);
				memcpy((void *)pagebuf, (void *)head_data, 4 * sizeof(int));
				writePage(fp, pagebuf, 0);
				return;
			}
		}
	}	

}

int main(int argc, char *argv[])
{
	char *record_fname = (char *)calloc(BUF_SIZE, sizeof(char));
	char *recordbuf = (char *)calloc(RECORD_SIZE, sizeof(char));
	char *pagebuf = (char *)calloc(PAGE_SIZE, sizeof(char));
	memset((void *)recordbuf, (char)0xff, RECORD_SIZE);
	memset((void *)pagebuf, (char)0xff, PAGE_SIZE);
	FILE *fp;  // 레코드 파일의 파일 포인터
	Person tmp;

	int head_data[4] = {};

	strcpy(record_fname, argv[2]);
	if(!strcmp(argv[1] , "i"))
	{
		if(access(record_fname, F_OK) < 0)
		{
			int first_data[4];
			first_data[0] = 2;
			first_data[1] = 0;
			first_data[2] = -1;
			first_data[3] = -1;

			memcpy((void *)pagebuf, (void *)first_data, 4 * sizeof(int));
			fp = fopen(record_fname, "a");
			writePage(fp, pagebuf, 0);
			memset(pagebuf, (char)0xff, PAGE_SIZE);
			writePage(fp, pagebuf, 1);
			fclose(fp);
		}

		strcpy(tmp.sn , argv[3]);
		strcpy(tmp.name , argv[4]);
		strcpy(tmp.age , argv[5]);
		strcpy(tmp.addr , argv[6]);
		strcpy(tmp.phone , argv[7]);
		strcpy(tmp.email , argv[8]);
		
		fp = fopen(record_fname, "r+");
		insert(fp, &tmp);

		fclose(fp);
	}
	else if(!strcmp(argv[1], "d"))
	{
		if(access(record_fname, F_OK) < 0)
		{
			fprintf(stderr, "%s doesn't exist\n", record_fname);
			return 0;
		}
		fp = fopen(record_fname, "r+");
		delete(fp, argv[3]);
		fclose(fp);
	}
	else
		fprintf(stderr, "argument error\n");


	return 1;
}
