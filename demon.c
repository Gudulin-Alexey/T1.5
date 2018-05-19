#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
struct globalArgs_t 
{
	int demonFlag;
	char* intFileName;
	char* charFileName;
	char* structFileName;

} globalArgs;

static const char *optString = "i:c:s:D";

static const struct option longOpts[] = {
	{"intF", required_argument, NULL, 'i'},
	{"cahrF", required_argument, NULL, 'c'},
	{"structF", required_argument, NULL, 's'},
	{"demon", no_argument, NULL, 'D'}
};

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

int main(int argc, char* argv[])
{
	int status;
	int pid;
	int opt = 0;
	int longIndex = 0;
	opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	while( opt != -1 )
	{
		switch( opt)
		{
			case 'i':
				globalArgs.intFileName = optarg;
				break;
			case 'c':
				globalArgs.charFileName = optarg;
				break;
			case 's':
				globalArgs.structFileName = optarg;
				break;
			case 'D':
				globalArgs.demonFlag = 1;
				break;
			default:
				break;
		}
		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}
	printf("intFileName%s\n",globalArgs.intFileName);
	printf("charFileName%s\n",globalArgs.charFileName);
	printf("structFileName%s\n",globalArgs.structFileName);
	printf("demonFlag%d\n",globalArgs.demonFlag);
	if ( globalArgs.demonFlag == 1)
	{
		pid = fork();
		if ( pid == -1)
		{
			printf("fork");
			return -1;
		}
		else if(!pid) // parent process
		{
			setsid();
			SetPidFile("mydemonPID");
			return 0;

		}
		else
		{
			return 0;
		}
	}
}
