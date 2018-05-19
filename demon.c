#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
/*
	use -D to start process like demon
	use -i (--intF) FileName    to set output file for <int>
	use -c (--charF) FileName    to set output file for <char[5]>
	use -s (--structF) FileName    to set output file for <mystruct>
	To terminate process use Ctrl+C from shell , 
	if proccess works like demon use kill -15 PID (SIGTERM)
	if you use kill -9 PID (SIGKILL) message queue will be not removed
	

*/
#define TYPES_COUNT 3


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
	int demonFlag;
	char *typeFileName[TYPES_COUNT];
	int qid;
	FILE *typeFile[TYPES_COUNT];
	int stopFlag;

} globalArgs;

static const char *optString = "i:c:s:D";

static const struct option longOpts[] = {
	{"intF", required_argument, NULL, 'i'},
	{"cahrF", required_argument, NULL, 'c'},
	{"structF", required_argument, NULL, 's'},
	{"demon", no_argument, NULL, 'D'}
};




void myhandle(int mysignal)
{
	msgctl(globalArgs.qid, IPC_RMID, 0);
	for(int i=0;i < TYPES_COUNT; i++)
		fclose(	globalArgs.typeFile[i]);
	exit(1);
}



void SetPidFile(char* Filename)
{
    FILE* f;

    f = fopen(Filename, "w+");
    if (f)
    {
        fprintf(f, "%u", getpid());
        fclose(f);
    }
}
void currtime(char *str)
{
	struct tm *u;
	time_t t = time(NULL);
	char s[30];
	u = localtime(&t);
	for (int i = 0;i<40;i++) s[i] = 0;
	strftime(s, 25,"<%d.%m.%Y,%H:%M:%S>: ", u);
	strcpy(str,s);
}

void DataToFile(mesgData *mdata)
{
	char time[30];
	int i = mdata->dtype;
	currtime(time);
	fwrite(time, sizeof(char),strlen(time),globalArgs.typeFile[i]);
	fwrite(&mdata->data, typeList[i].sizeOfType, 1, globalArgs.typeFile[i]);
	fputs("\n",globalArgs.typeFile[i]);
	fflush(globalArgs.typeFile[i]);
}

void WorkProc()
{	
	
	key_t msgkey;
	mesgData recieved;
	
	msgkey = ftok(".",5);
	if((globalArgs.qid = msgget(msgkey, IPC_EXCL | IPC_CREAT | 0666)) == -1)
		return;
	while (!globalArgs.stopFlag)
	{
		msgrcv(globalArgs.qid, &recieved,sizeof(mesgData)-sizeof(long),1, 0);
		DataToFile(&recieved);
	}
	msgctl(globalArgs.qid, IPC_RMID, 0);
}

int main(int argc, char* argv[])
{
	int status;
	int pid;
	signal(SIGTERM, myhandle);
	signal(SIGINT, myhandle);
	for(int i = 0;i < TYPES_COUNT; i++)  // default fileNames 
	{
		globalArgs.typeFileName[i] = (char*) malloc(strlen(typeList[i].typeName) + 5);
		strcpy(globalArgs.typeFileName[i], typeList[i].typeName);
		strcat(globalArgs.typeFileName[i], ".txt");

	}
	int opt = 0;
	int longIndex = 0;
	opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	while( opt != -1 )
	{
		switch( opt)
		{
			case 'i':
				globalArgs.typeFileName[0] = optarg;
				break;
			case 'c':
				globalArgs.typeFileName[1] = optarg;
				break;
			case 's':
				globalArgs.typeFileName[2] = optarg;
				break;
			case 'D':
				globalArgs.demonFlag = 1;
				break;
			default:
				break;
		}
		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}
	for(int i = 0;i < TYPES_COUNT; i++)  // open file to write for every type 
	{
		globalArgs.typeFile[i] = fopen( globalArgs.typeFileName[i], "w+b");
	}

	printf("demonFlag %d\n",globalArgs.demonFlag);
	if ( globalArgs.demonFlag == 1)
	{
		pid = fork();
		if ( pid == -1)
		{
			printf("fork");
			return -1;
		}
		else if(!pid) // child process
		{
			setsid();
			SetPidFile("mydemonPID");
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			WorkProc();
			return 0;

		}
		else
		{
			return 0;
		}
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	WorkProc();
	return 0;
}
