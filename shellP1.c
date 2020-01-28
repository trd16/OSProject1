//COP4610
//Project 1 Starter Code
//example code for initial parsing

//*** if any problems are found with this code,
//*** please report them to the TA


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define _POSIX_C_SOURCE
#define _GNU_SOURCE
#define _BSD_SOURCE
#define _DEFAULT_SOURCE

typedef struct
{
	char** tokens;
	int numTokens, input, output;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void printTokens(instruction* instr_ptr);

int isPath(char* token);
char* expandPath(char* path);
char* resolvePath(char* path);
int isValidDir(char* path);
int isValidFile(char* path);

void execute(char** cmd);
void iRedirection(char** command, int location);
void oRedirection(char** command, int location);
void pipeImplementation(char* command1, char* command2);

int isBuiltIn(char* token);
void builtIns(instruction* instr);		//checks user input
char * checkEnv(char * tkn);
void echoToks(char** toks,int numToks);

//globals
pid_t child_pids[1000], pid;      //child_pids[child_nb++] = f;   //use after forking where f = fork()
int child_nb=0;

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	while (1) {
		printf("%s",getenv("USER"));
		printf("@");
		printf("%s",getenv("MACHINE"));
		printf(":%s>",getenv("PWD"));

		// loop reads character sequences separated by whitespace
		do {
			//scans for next token and allocates token var to size of scanned token
			scanf("%ms", &token);
			temp = (char*)malloc((strlen(token) + 1) * sizeof(char));

			int i;
			int start = 0;
			for (i = 0; i < strlen(token); i++) {
				//pull out special characters and make them into a separate token in the instruction
				if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&') {
					if (i-start > 0) {
						memcpy(temp, token + start, i - start);
						temp[i-start] = '\0';
						addToken(&instr, temp);
					}

					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					addToken(&instr,specialChar);

					start = i + 1;
				}
			}

			if (start < strlen(token)) {
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				addToken(&instr, temp);
			}

			//free and reset variables
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;
		} while ('\n' != getchar());    //until end of line is reached

		//checking for I/O Errors, Piping Errors
		if(strcmp(instr.tokens[0],"|")==0 || strcmp(instr.tokens[0],"<") == 0 || strcmp(instr.tokens[0], ">") == 0)
		{
			printf("Invalid Syntax\n");
			addNull(&instr);
			clearInstruction(&instr);
			continue;
		}
		if(strcmp(instr.tokens[instr.numTokens-1], "|") == 0 || strcmp(instr.tokens[instr.numTokens - 1], "<") == 0 
			|| strcmp(instr.tokens[instr.numTokens - 1],">") == 0)
		{
			printf("Invalid Syntax\n");
			addNull(&instr);
			clearInstruction(&instr);
			continue;
		}

		//loop through all tokens to find errors
		int iredir = FALSE, oredir, pipes = 0, background = FALSE;
		for (int i = 0; i < instr.numTokens; ++i)
		{
			if (isBuiltIn(instr.tokens[i]))
			{
				builtIns(&instr);
				break;
			}
			//output redirection
			else if (strcmp(instr.tokens[i], ">") == 0) 
			{
				if (oredir)
				{
					printf("ERROR: Multiple redirections of the same type are not allowed\n");
					break;
				}
				oredir = TRUE;
			}
			//input redirection
			else if (strcmp(instr.tokens[i], "<") == 0)
			{
				if (iredir)
				{
					printf("ERROR: Multiple redirections of the same type are not allowed\n");
					break;
				}
				iredir = TRUE;
			}
			//Pipes
			else if (strcmp(instr.tokens[i], "|") == 0)
			{
				if (++pipes > 2)
				{
					printf("ERROR: No more than two pipes are allowed at once");
					break;
				}
			}
			//background processing
			else if (strcmp(instr.tokens[i], "&"))
			{
				background = TRUE;
			}
			else 
			{
				instr.tokens[i] = resolvePath(instr.tokens[i]);
				if (instr.tokens[i] == NULL){
					if (i == 0 || i == 1 && strcmp(instr.tokens[i], "&") == 0)
						printf("Unknown command\n");
					else
						printf("No such file or path");
				}
				//if (stat(file, &sb) == 0 && sb.st_mode & S_IXUSR) 
			}
			
			//pipes and redirection not allowed at same time
			if (pipes && (iredir || oredir))
			{
				printf("ERROR: Pipes and I/O redirection cannot occur together\n");
				break;
			}
		}


		if (iredir || oredir)
		{
			//checking for I/O Redirection & Piping
			int j;
			for(j = 0; j < instr.numTokens; ++j)
			{
				if(strcmp(instr.tokens[j], "<") == 0)
				{
					instr.input = j + 1; 
				}
				if(strcmp(instr.tokens[j], ">") == 0)
				{
					instr.output = j + 1;
				}
				
				if (instr.input != -1 && instr.output != -1)
					break;
			}
			
			//invalid syntax CMD > & FILE and CMD > & FILE
			if (instr.input != -1 && strcmp(instr.tokens[instr.input], "&") == 0
			|| instr.output != -1 && strcmp(instr.tokens[instr.output], "&") == 0)
			{
				printf("Invalid syntax\n");
				continue;
			}
		}
		else if (pipes)
		{
			int j;
			for(j = 0; j < instr.numTokens; ++j)
			{ 
				if(strcmp(instr.tokens[j],"|") == 0)
				{
					char* command1 = instr.tokens[j-1];
					char* command2 = instr.tokens[j+1];
					pipeImplementation(command1, command2);
				}
			}
		}
		//normal execution
		else 
			execute(instr.tokens);

		addNull(&instr);
		clearInstruction(&instr);
	}

	return 0;
}

//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr_ptr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
	instr_ptr->input = -1;
	instr_ptr->output = -1;
}

int isPath(char* token)
{
	int i;
	for (i = 0; i < strlen(token); ++i)
	{
		if (token[i] == '/' || token[i] == '~')
			return TRUE;
	}
	return FALSE;
}

int numberOfDirs(char* path)
{
	if (path == NULL || strlen(path) <= 1)
		return 0;

	int num = 0, i;
	for (i = 0; i < strlen(path); ++i)
	{
		if (path[i] == '/')
			++num;
	}
	return num;
}

char* expandPath(char* path)
{
	char* newPath;

	//set ~ to home directory
	if (path[0] == '~')
	{
		//set the lenght of the newPath string
		int len = strlen(newPath) + strlen(getenv("HOME"));
		newPath = (char*)malloc((len) * sizeof(char));
		strcpy(newPath, getenv("HOME"));

		//add the path after '~' to the end of HOME
		int i, j = strlen(newPath);
		for (i = 1; i < strlen(path); ++i)
			newPath[j++] = path[i];
		newPath[j] = '\0';
	}
	//relative path
	else if (path[0] != '/')
	{
		int len = strlen(getenv("PWD")) + strlen(path);
		newPath = (char*)malloc((len + 2) * sizeof(char));
		strcpy(newPath, getenv("PWD"));
		strcat(newPath, "/");
		strcat(newPath, path);
	}
	//from root dir
	else
	{
		newPath = (char*)malloc((strlen(path) + 1) * sizeof(char));
		strcpy(newPath, path);
	}

	//return newPath if no further directories to expand. ie at root
	int numDirs = numberOfDirs(newPath);
	if (numDirs == 0)
		return newPath;

	char** pathArray = (char**)malloc(numDirs * sizeof(char*));
	int i, j;
	for (i = 0; i < numDirs; ++i)
		pathArray[i] = (char*)malloc(strlen(newPath) * sizeof(char*));

	i = 0;
	//split path into array
	char* ptr = strtok(newPath, "/");
	while (ptr != NULL)
	{
		//decrement i on ..
		if (strcmp(ptr, "..") == 0)
		{
			if (i == 0)
				return NULL;
			--i;
		}
		//only add directory to array if not . or ..
		else if (strcmp(ptr, ".") != 0)
			strcpy(pathArray[i++], ptr);

		ptr = strtok(NULL, "/");
	}

	//root
	strcpy(newPath, "/");
	for (j = 0; j < numDirs; ++j)
	{
		//add directory to path
		if (j < i)
		{
			if (j != 0)
				strcat(newPath, "/");
			strcat(newPath, pathArray[j]);
		}
		free(pathArray[j]);
	}
	free(pathArray);

	return newPath;
}


int isValidDir(char* path)
{
	DIR* dir = opendir(path);
	if (dir)
	{
		closedir(dir);
		return TRUE;
	}
	return FALSE;
}

int isValidFile(char* path)
{
	if (access(path, F_OK) != -1)
		return TRUE;
	return FALSE;
}

char* resolvePath(char* path)
{
	//absolute path
	if (path[0] == '/')
	{
		if (isValidFile(path))
			return path;
		return NULL;
	}
	
	//test for ./path first
	char* newPath = (char*)malloc(256 * sizeof(char));
	strcpy(newPath, "./");
	strcat(newPath, path);
	strcpy(newPath, expandPath(newPath));
	if (isValidFile(newPath))
		return newPath;
	
	char PATHenv[256];
	strcpy(PATHenv, getenv("PATH"));
	
	//split PATH and test each PATH var
	char* ptr = strtok(PATHenv, ":");
	while (ptr != NULL)
	{
		strcpy(newPath, ptr);
		strcat(newPath, "/");
		strcat(newPath, path);
		
		if (isValidFile(newPath))
			return newPath;
		
		ptr = strtok(NULL, ":");
	}
	
	//not found
	return NULL;
}

void execute(char** cmd)
{
	int status;
	pid_t pid = fork();
	if(pid == -1)
	{
		//error
		exit(1);
	}
	else if(pid == 0)
	{
		//child
		execvp(cmd[0],cmd);
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
	}
	else
	{
		//parent
		child_pids[child_nb++] = pid;
		waitpid(pid,&status, 0);
	}
}

void iRedirection(char** command, int location)
{
	printf("Found <\n");
	if(fork() == 0)
	{
		open(command[location+1], O_RDONLY);
		close(0);
		dup(3);
		close(3);
		execute(command);
		exit(1);
	}
	else
		close(3);
	
	return;
}

void oRedirection(char** command, int location)
{
	printf("Found >\n");
	if(fork() == 0)
	{
		open(command[location+1], O_RDWR | O_CREAT | O_TRUNC);
		close(1);
		dup(3);
		close(3);
		execute(command);
		exit(1);
		
	}
	else
		close(3);
	
	return;
	
}

void bothRedirection(char** command, int ilocation, int olocation)
{
	printf("Found > & <\n");

	if(ilocation < olocation)
	{
		//do iredirection first
		iRedirection(command, ilocation);
		//do oredirection with command[0]
		if(fork() == 0)
		{
			open(command[olocation+1], O_RDWR | O_CREAT | O_TRUNC);
			close(1);
			dup(3);
			close(3);
			execute(command[0]);
			exit(1);
		}
		else
			close(3);
		
	}
	
	if(olocation < ilocation)
	{
		//do oredirection first
		oRedirection(command, olocation);
		//do iredirection with command[0]
		if(fork() == 0)
		{
			open(command[ilocation+1], O_RDONLY);
			close(0);
			dup(3);
			close(3);
			execute(command[0]);
			exit(1);
		}
		else
			close(3);
		return;
	}
	
}




void pipeImplementation(char* command1, char* command2)
{
	printf("Found |\n");
	/*
	int fd[2];
	if(fork() == 0)
	{
		pipe(fd);
		if(fork() == 0)
		{
			//cmd1 writer
			close(STDOUT_FILENO);
			dup(fd[1]);
			close(fd[0]);
			close(fd[1]);
			//execute command
			exit(1);
		}
		else
		{
			//cmd2 reader
			close(STDIN_FILENO);
			dup(fd[0]);
			close(fd[0]);
			close(fd[1]);
			//execute command
			exit(1);
		}
	}
	else
	{
		//parent shell
		close(fd[0]);
		close(fd[1]);
	}
	*/
}

int isBuiltIn(char* token)
{
	if (strcmp(token, "exit") == 0 || strcmp(token, "cd") == 0 
	|| strcmp(token, "echo") == 0 || strcmp(token, "jobs"))
		return TRUE;
	return FALSE;
}

void builtIns(instruction* instr)
{
	char **toks = instr->tokens;

	int numToks = instr->numTokens;


	if(numToks-1 > 0)
	{
		if(strcmp(toks[0],"echo") == 0)
        {	
            echoToks(toks,numToks);
        }
		else if(strcmp(toks[0],"cd") == 0)	
		{			
            char buffer[200];

            //if no specified directory
			if(toks[1] == NULL)
            {
				if(chdir(getenv("HOME")) == 0)
                    setenv("PWD",getcwd(buffer, sizeof buffer),1);
            }
			else
            {
				if(chdir(toks[1]) == 0)
                    setenv("PWD",getcwd(buffer, sizeof buffer),1);
            }
		}
		else if(strcmp(toks[0],"jobs") == 0)
		{
            for(int i = 0;i < child_nb;i++)
            {
                printf("%d    %d    %d\n",i,child_pids[i],child_pids[i]);
            }
			
		}
		else if(strcmp(toks[0],"exit") == 0)
		{
            int status;
            waitpid(-1,&status,0);



			clearInstruction(&(*instr));
			exit(0);
		}
		else
		{
            execute(toks);
			//printf("%s: Command not found\n",toks[0]);
		}
		
	}
}


//if env variable, remove '$' and make everything uppercase
char * checkEnv(char * tkn)     
{
	memmove(tkn, tkn+1, strlen (tkn+1) + 1);
	for(int i = 0;i < strlen(tkn);i++)
		toupper(tkn[i]);
	return tkn;
}

//echo function
void echoToks(char** toks,int numToks)
{
			for(int i = 1;i < numToks; i++)		//loops thru all input
			{				
				if(toks[i][0] == '$')
                {
                    checkEnv(toks[i]);
					printf("%s ",getenv(toks[i]));
                }
				else
					printf("%s ",toks[i]);
			}
			printf("\n");
}

