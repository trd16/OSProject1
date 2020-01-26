//COP4610
//Project 1 Starter Code
//example code for initial parsing

//*** if any problems are found with this code,
//*** please report them to the TA


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void printTokens(instruction* instr_ptr);

int isPath(char* token);
char* expandPath(char* path);
int isValidDir(char* path);
int isValidFile(char* path);

void execute(char** cmd);
void iRedirection(char* command, char* file);
void oRedirection(char* command, char* file);
void pipeImplementation(char* command1, char* command2);

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
			return -1;
		}

		if(strcmp(instr.tokens[instr.numTokens-1], "|") == 0 || strcmp(instr.tokens[instr.numTokens - 1], "<") == 0 || strcmp(instr.tokens[instr.numTokens - 1],">") == 0)
		{
			printf("Invalid Syntax\n");
			return -1;
		}	




		//loop through all tokens
		int i;
		for (i =0; i < instr.numTokens; ++i)
		{
			if (isPath(instr.tokens[i]))
				instr.tokens[i] = expandPath(instr.tokens[i]);

			printf("%s ", instr.tokens[i]);


		}
		printf("\n");

		//checking for I/O Redirection & Piping
		int j;
		for(j = 0; j < instr.numTokens; ++j)
		{
			if(strcmp(instr.tokens[j], "<") == 0)
			{
				char* command = instr.tokens[j - 1];
				char* file = instr.tokens[j + 1]; 
				iRedirection(command, file);
			}
			if(strcmp(instr.tokens[j], ">") == 0)
			{
				char* command = instr.tokens[j-1];
				char* file = instr.tokens[j+1];
				oRedirection(command, file);
			}
			if(strcmp(instr.tokens[j],"|") == 0)
			{
				char* command1 = instr.tokens[j-1];
				char* command2 = instr.tokens[j+1];
				pipeImplementation(command1, command2);
			}

		}


		
		

		//execution
		//execute(instr);


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
	if (path[0] == '/')
		return path;

	char PATHenv[256];
	strcpy(PATHenv, getenv("PATH"));

	//split PATH into array
	char* ptr = strtok(PATHenv, ":");
	while (ptr != NULL)
	{
		int size = (int)(strlen(PATHenv) + strlen(path)) + 2;
		char* newPath = (char*)malloc(size * sizeof(char));
		strcpy(newPath, ptr);
		strcat(newPath, "/");
		strcat(newPath, path);

		if (isValidFile)
			return newPath;
		free(newPath);
		char* ptr = strtok(NULL, ":");
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
		execv(cmd[0],cmd);
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
	}
	else
	{
		//parent
		waitpid(pid,&status, 0);
	}
}

void iRedirection(char* command, char* file)
{
	printf("Found <\n");
	/*	if(fork() == 0)
		{
		open(file, O_RDONLY);
		close(0);
		dup(3);
		close(3);
		execute(command);
		exit(1);
		}
		else
		close(3);*/
	return;
}

void oRedirection(char* command, char* file)
{
	printf("Found >\n");
	/*	if(fork() == 0)
		{
		open(file, O_RDWR | O_CREAT | O_TRUNC);
		close(1);
		dup(3);
		close(3);
		execute(command);
		exit(1);
		}

		else
		close(3);*/
	return;
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
