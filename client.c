#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
/*
	use  -f (--file) FileName      set reading from file ... example ./client --file file1

*/

#define TYPES_COUNT 3
#define MAX_STR_LENGTH 100 // max length string in file

struct data_type
{
	char *typeName;
	int sizeOfType;

};


struct mystruct { int a; int b; int c;};

static const struct data_type typeList[TYPES_COUNT] = {
	{"int", sizeof(int)},
	{"char", sizeof(char)*5},
	{"mystruct", sizeof(struct mystruct)}
};

typedef struct mesgData_t
{
	long msgtype;
	union {
	int i;
	char c[5];
	struct mystruct ms;
	} data;
	int dtype;
} mesgData;

struct globalArgs_t 
{
	int qid;	
	char *fileName;
	FILE *f;
	int fromFile;

} globalArgs;
static const char *optString = "f:";

static const struct option longOpts[] = {
	{"file", required_argument, NULL, 'f'}
};


void parseStr(char* str, char *type, char *data)
{
	char *token = strtok(str,":");
	strcpy(type,token);
	if ((token = strtok(NULL,":")) != NULL)
		strcpy(data,token);
	
}

int getDataFromStr(char* typeName, char* str, mesgData *mdata)
{
	int value;
	int count;
	for(count = 0 ;count < TYPES_COUNT; count++)
		if (strcmp(typeName,typeList[count].typeName) == 0)
		{
			mdata->dtype = count;
			break;
		}
	if (count == TYPES_COUNT)
		return 1;
	switch (count)
	{
		case 0:
			if(((value = atoi(str)) == 0) && (str[0] != '0'))
				return 2;
			else
				mdata->data.i = value;
				
			break;
		case 1:
			if((strlen(str)) < 5)
				return 2;
			else
				sscanf(str,"%s", mdata->data.c);	
			break;
		case 2:
			sscanf(str,"%d,%d,%d",&mdata->data.ms.a,&mdata->data.ms.b,&mdata->data.ms.c);
			break;
		default:
			break;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	
	int qid;
	mesgData send;
	int count;
	char type[10];
	char str[30];
	int opt = 0;
	int longIndex = 0;
	send.msgtype = 1;

	opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	while( opt != -1 )
	{
		switch( opt)
		{
			case 'f':
				globalArgs.fileName = optarg;
				globalArgs.fromFile = 1;
				break;
			default:
				break;
		}
		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}



	key_t msgkey;
	msgkey = ftok(".",5);
	if( (globalArgs.qid = msgget(msgkey, 0)) == -1)
	{
		printf("message queue doesnt exist\n");
		return 1;
	}



	if (globalArgs.fromFile)
	{
		char s[MAX_STR_LENGTH];
		FILE *f;
		if(!(f = fopen( globalArgs.fileName, "r")))
		{
			printf("file not exists\n");
			return 1;
		}
		while(fgets(s,MAX_STR_LENGTH,f) != NULL)
		{
			int err;
			parseStr(s,type,str);
			if (!(err = getDataFromStr(type, str, &send)))
				msgsnd(globalArgs.qid, &send, sizeof(mesgData)-sizeof(long), 0);
			else if (err == 1)
				printf("unlnown type in file\n");
			else if (err == 2)
				printf("incorect data format in file\n");
		}
		return 0;
	}
	while (1)
	{
		
		printf("choose type to enter\n");
		scanf("%s",type);
		printf("enter data\n");
		scanf("%s",str);
		int err;
		if (!(err = getDataFromStr(type, str, &send)))
				msgsnd(globalArgs.qid, &send, sizeof(mesgData)-sizeof(long), 0);
			else if (err == 1)
				printf("unlnown type\n");
			else if (err == 2)
				printf("incorect input\n");
		if (err)
		{
			printf("\nsupported types:\n");
			for(int i=0 ;i < TYPES_COUNT; i++)
				printf("%s " ,typeList[i].typeName);
			printf("\n");
		}
	}

	return 0;
}
