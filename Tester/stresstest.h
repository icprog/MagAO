static void help();

struct Var 
	{
	char name[ VAR_NAME_LEN+1];
	int type;
	int size;
	};


int main(int argc, char **argv);

void CreateRandomVar(char *);

void SetRandomVar(void);

void WaitAnswer( char *message, int seqnum);

int FindVar( char *name);

void MyLog (char *fmt, ...);
