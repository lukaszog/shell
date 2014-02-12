#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
//includowanie potrzebnych bibliotek
// kompilowanie z flaga -lred

void signals(int signo)
{
    pid_t pid;
    kill(pid,signo);
}

int parsuj(char *input, char *agrv2[], char **supPtr, int *modPtr) //funkcja partujaca wejscie
{
	int arg = 0, zak = 0;
	char *spacjelne = input;

	while(*spacjelne != '\0' && zak == 0)
	{
		*agrv2 = spacjelne;
		arg++; // zwiekszanie obiegu

		while(*spacjelne != ' ' && *spacjelne != '\t' && *spacjelne != '\0' && *spacjelne != '\n' && zak == 0)  // znaki oddzielajace
		{
			switch(*spacjelne)
			{
                //funkcje pipe
				case '&':
					*modPtr = 44;
					break;
				case '|':
					*modPtr = 33;
					*agrv2 = '\0';
					spacjelne++;
					while(*spacjelne == ' ' || *spacjelne == '\t')
						spacjelne++;
					*supPtr = spacjelne;
					zak = 1;
					break;
			}
			spacjelne++;
		}
		while((*spacjelne == ' ' || *spacjelne == '\t' || *spacjelne == '\n') && zak == 0)
		{
			*spacjelne = '\0';
			spacjelne++;
		}
		agrv2++;
	}
	*agrv2 = '\0'; // zakonczenie tablicy
	return arg;
}

void tnij(char *spacjelne) // oddzielanie tekstu
{
	while(*spacjelne != ' ' && *spacjelne != '\t' && *spacjelne != '\n')
	{
		spacjelne++;
	}
	*spacjelne = '\0'; // zakonczenie tablicy
}

void wykonaj(char **agrv2, int mod, char **supPtr)
{
	pid_t pid, pid2;
	int mod2 = 0, arg, status1, status2;
	char *agrv22[80], *sup2 = NULL;
	int myPipe[2];

	if(mod == 33)
	{
		if(pipe(myPipe))
		{
			fprintf(stderr, "Pipe");
			exit(-1);
		}
		parsuj(*supPtr, agrv22, &sup2, &mod2);  // parsowanie
	}
	pid = fork();
	if( pid < 0)
	{
		printf("Error");
		exit(-1);
	}
	else if(pid == 0)
	{
		switch(mod)
		{
            case 33:
				close(myPipe[0]);		//zamkniecie wejscia dla pipe
				dup2(myPipe[1], fileno(stdout));
				close(myPipe[1]);
				break;
		}
		execvp(*agrv2, agrv2);
	}
	else
	{
		if(mod == 44) //przejscie w tlo
			;
		else if(mod == 33)
		{
			waitpid(pid, &status1, 0);		//czekanie az 1 proces zakonczy
			pid2 = fork();
			if(pid2 < 0)
			{
				printf("error");
				exit(-1);
			}
			else if(pid2 == 0)
			{
				close(myPipe[1]);		//zamkniecie wyjscia dla pipe
				dup2(myPipe[0], fileno(stdin));
				close(myPipe[0]);
				execvp(*agrv22, agrv22);
			}
			else
			{
				close(myPipe[0]);
				close(myPipe[1]);
			}
		}
		else
			waitpid(pid, &status1, 0);
	}
}

int main(int argc, char *argv[])
{
	int mod = 0,arg;
	char *input, *agrv2[80], *sup = NULL;

	input = (char*)malloc(sizeof(char)*80); //alokowanie pamieci dla wejsciowego napisu

	char curDir[100];

	while(1) //glowna petla calajaca
	{
		mod = 0;
		getcwd(curDir, 100);
		input = readline("$ "); //readline

		if(strcmp(input, "exit\n") == 0) // exit
			exit(0);

		arg = parsuj(input, agrv2, &sup, &mod); //partowanie wejscia

		if(strcmp(*agrv2, "cd") == 0) //uzycie cd - chdir
		{
            if(signal(SIGINT, signals)==SIG_ERR)
            {
                perror("blad");
            }
			chdir(agrv2[1]);
		}
		else
		{
            if(signal(SIGINT, signals)==SIG_ERR)
            {
                perror("blad");
            }
			wykonaj(agrv2, mod, &sup);  //w przeciwnym razie wykonywuj program
        }
	}
	return 0;
}

