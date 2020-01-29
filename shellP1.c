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
	int cmd1, cmd2, cmd3;
} instruction;

typedef struct 
{
	pid_t pid;
	char name[100][100];
} queue;

void addToken(instruction* instr_ptr, char* tok);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void printTokens(instruction* instr_ptr);

int isPath(char* token);
char* expandPath(char* path);
char* resolvePath(char* path);
int isValidDir(char* path);
int isValidFile(char* path);
int isExecutable(char* path);

void execute(char** cmd);
void iRedirection(char** command, int location);
void oRedirection(char** command, int location);
void pipeImplementation(char* command1, char* command2);

int isBuiltIn(char* token);
void builtIns(instruction* instr);		//checks user input
char * checkEnv(char * tkn);
void echoToks(char** toks,int numToks);
void jobs(instruction* instr);

void insertQueue(pid_t pid,char **cmds, int numCmds);
void deleteJob(pid_t pid);
void backgroundProc(char ** cmd, int numToks);
void checkBg();

//globals
//globals
pid_t child_pids[1000], pid, pid1, pid2, pid3;  
int child_nb = 0,  typed = 0;
queue q[100];

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;
	instr.input = -1;
	instr.output = -1;
	instr.cmd1 = -1;
	instr.cmd2 = -1;
	instr.cmd3 = -1;
	int numCommands = 0;

	while (1) {
		
		checkBg();
		
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

		++typed;

		//checking for I/O Errors, Piping Errors at beginning of input
		if(strcmp(instr.tokens[0],"|") == 0 || strcmp(instr.tokens[0],"<") == 0 || strcmp(instr.tokens[0], ">") == 0)
		{
			printf("Invalid Syntax\n");
			addNull(&instr);
			clearInstruction(&instr);
			continue;
		}
		//checking for I/O Errors, Piping Errors at end of input
		if(strcmp(instr.tokens[instr.numTokens - 1], "|") == 0 || strcmp(instr.tokens[instr.numTokens - 1], "<") == 0 
			|| strcmp(instr.tokens[instr.numTokens - 1],">") == 0)
		{
			printf("Invalid Syntax\n");
			addNull(&instr);
			clearInstruction(&instr);
			continue;
		}

		//loop through all tokens to find errors
		int iredir = FALSE, oredir = FALSE, pipes = 0, foreground = FALSE, background = FALSE, error = FALSE;
		for (int i = 0; i < instr.numTokens; ++i)
		{
			//checking first token
			if (i == 0)
			{
				//foreground operation
				if (strcmp(instr.tokens[i], "&") == 0)
					foreground = TRUE;
				//error if not executable or 
				else if (!isBuiltIn(instr.tokens[i]) && !isExecutable(resolvePath(instr.tokens[i])))
				{
					if (strcmp(instr.tokens[i], "<") == 0 || strcmp(instr.tokens[i], ">") == 0 || strcmp(instr.tokens[i], "|") == 0)
						printf("Invalid syntax\n");
					else
						printf("%s: Command not found.\n", instr.tokens[i]);
					error = TRUE;
					break;
				}
			}
			//checking second token
			else if (i == 1 && foreground)
			{
				//foreground operation
				if (!isExecutable(resolvePath(instr.tokens[i])) && !isBuiltIn(instr.tokens[i]))
				{
					if (strcmp(instr.tokens[i], "<") == 0 || strcmp(instr.tokens[i], ">") == 0 || strcmp(instr.tokens[i], "|") == 0)
						printf("Invalid syntax\n");
					else 
						printf("%s: Command not found.\n");
					error = TRUE;
					break;
				}
			}
			
			//checking for input redirection
			if (strcmp(instr.tokens[i], "<") == 0)
			{
				if (iredir)
				{
					printf("Can only take input from one file\n");
					error = TRUE;
					break;
				}
				iredir = TRUE;
			}
			//output redirection
			else if (strcmp(instr.tokens[i], ">") == 0)
			{
				if (oredir)
				{
					printf("Can only output to one file\n");
					error = TRUE;
					break;
				}
				oredir = TRUE;
			}
			//piping
			else if (strcmp(instr.tokens[i], "|") == 0)
			{
				if (++pipes > 2)
				{
					error = TRUE;
					printf("No more than two pipes are allowed\n");
					break;
				}
			}
			//background processing
			else if (strcmp(instr.tokens[i], "&") == 0 && i != 0)
			{
				background = TRUE;
				if (i +1 < instr.numTokens)
				{
					error = TRUE;
					printf("Invalid syntax\n");
					break;
				}
			}
			else if (iredir && instr.input == -1)
				instr.input = i;
			else if (oredir && instr.output == -1)
				instr.output = i;
			else if (pipes)
			{
				if (pipes == 1 && instr.cmd2 == -1)
					instr.cmd2 = i;
				if (pipes == 2 && instr.cmd3 == -1)
					instr.cmd3 = i;
			}
			else if (instr.cmd1 == -1 && instr.tokens[i][0] != '&')
				instr.cmd1 = i;
			
			//pipes and redirection not allowed at same time
			if (pipes && (iredir || oredir))
			{
				printf("Piping and I/O redirection cannot occur together\n");
				error = TRUE;
				break;
			}
		}

		//redirection without file 
		if ((iredir && instr.input == -1) || (oredir && instr.output == -1))
		{
			printf("Invalid syntax.\n");
			error = TRUE;
		}
		
		else if (instr.cmd1 == -1)
		{
			printf("Invalid syntax.\n");
			error = TRUE;
		}

		if (strcmp(instr.tokens[instr.numTokens - 1], "&") == 0)
			background = TRUE;

		if (!error)
		{	
			//split instr into temp instructions with only the command
			instruction tempInstr;
			tempInstr.numTokens = 0;
			tempInstr.tokens = NULL;
			
			instruction tempInstr2;
			tempInstr2.numTokens = 0;
			tempInstr2.tokens = NULL;
			
			instruction tempInstr3;
			tempInstr3.numTokens = 0;
			tempInstr3.tokens = NULL;
			
			int i;
			for (i = instr.cmd1; i < instr.numTokens; ++i)
			{
				if (i == instr.cmd1)
				{
					if (isBuiltIn(instr.tokens[i]))
					{
						addToken(&tempInstr, instr.tokens[i]);
					}
					else 
					{
						char* temp = resolvePath(instr.tokens[i]);
						if (!isExecutable(temp))
						{
							error = TRUE;
							printf("%s: Command not found.\n", instr.tokens[i]);
							break;
						}
						addToken(&tempInstr, instr.tokens[i]);
					}
				}
				else
				{
					if (strcmp(instr.tokens[i], ">") == 0 || strcmp(instr.tokens[i], "<") == 0 
					|| strcmp(instr.tokens[i], "&") == 0 || strcmp(instr.tokens[i], "|") == 0)
						break;
					
					if (instr.tokens[i][0] == '-')
						addToken(&tempInstr, instr.tokens[i]);
					else if (instr.tokens[i][0] == '$')
					{
						char* temp = getenv(checkEnv(instr.tokens[i]));
						if (temp == NULL)
						{
							error = TRUE;
							printf("%s: Undefined variable.\n");
							break;
						}
						addToken(&tempInstr, temp);
					}
					
					//files and directories
					else 
						addToken(&tempInstr, expandPath(instr.tokens[i]));
				}
			}
			
			addNull(&tempInstr);
			
			//execute
			if (!error)
			{
				// if (iredir && oredir)
				// {
					
				// }
				
				if (iredir)
				{
					if(fork() == 0)
					{
						if (background)		//background proc
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
								setpgid(0, 0);
								open(instr.tokens[instr.output], O_RDONLY);
								close(0);
								dup(3);
								close(3);
								
								if (isBuiltIn(tempInstr.tokens[0]))
									builtIns(&tempInstr);
								else
									execute(tempInstr.tokens);

								exit(1);
							}
							else
							{
								//parent
								insertQueue(pid,instr.tokens,instr.numTokens);
								printf("\n[%d]	[%d]\n",child_nb-1,pid);

								waitpid(pid,&status,WNOHANG);
								
							}
						}
						else
						{
							int fd = open(instr.tokens[instr.input], O_RDONLY);
							close(0);
							dup(3);
							close(3);
							
							dup2(fd, 0);
							if (isBuiltIn(tempInstr.tokens[0]))
								builtIns(&tempInstr);
							else
								execute(tempInstr.tokens);
							
							exit(1);
						}
					}
					else
						close(3);
					continue;
				}
				else if (oredir)
				{
					if(fork() == 0)
					{
						if (background)
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
								setpgid(0, 0);
								open(instr.tokens[instr.output], O_RDWR | O_CREAT | O_TRUNC);
								close(0);
								dup(3);
								close(3);
								
								if (isBuiltIn(tempInstr.tokens[0]))
									builtIns(&tempInstr);
								else
									execute(tempInstr.tokens);

								exit(1);
							}
							else
							{
								//parent
								insertQueue(pid,instr.tokens,instr.numTokens);
								printf("\n[%d]	[%d]\n",child_nb-1,pid);

								waitpid(pid,&status,WNOHANG);
								
							}
						}
						else
						{
							open(instr.tokens[instr.output], O_RDWR | O_CREAT | O_TRUNC);
							close(1);
							dup(3);
							close(3);

							if (isBuiltIn(tempInstr.tokens[0]))
								builtIns(&tempInstr);
							else
								execute(tempInstr.tokens);

							exit(1);	
						}
					}
					else
						close(3);
				}
				
				else if (pipes)
				{
					//extract cmd2
					for (i = instr.cmd2; i < instr.numTokens; ++i)
					{
						if (instr.tokens[i][0] == '|' || instr.tokens[i][0] == '&')
							break;
						
						if (i == instr.cmd2)
						{
							if (isBuiltIn(instr.tokens[i]))
							{
								addToken(&tempInstr2, instr.tokens[i]);
							}
							else 
							{
								char* temp = resolvePath(instr.tokens[i]);
								if (!isExecutable(temp))
								{
									error = TRUE;
									printf("%s: Command not foun-d.\n", instr.tokens[i]);
									break;
								}
								addToken(&tempInstr2, instr.tokens[i]);
							}
						}
						else
						{
							if (instr.tokens[i][0] == '-')
								addToken(&tempInstr2, instr.tokens[i]);
							else if (instr.tokens[i][0] == '$')
							{
								char* temp = getenv(checkEnv(instr.tokens[i]));
								if (temp == NULL)
								{
									error = TRUE;
									printf("%s: Undefined variable.\n");
									break;
								}
								addToken(&tempInstr2, temp);
							}
							
							//files and directories
							else 
								addToken(&tempInstr2, expandPath(instr.tokens[i]));
						}
					}
					addNull(&tempInstr2);
					
					if (pipes == 2)
					{
						//extract cmd2
						for (i = instr.cmd2; i < instr.numTokens; ++i)
						{
							if (instr.tokens[i][0] == '|' || instr.tokens[i][0] == '&')
								break;
							
							if (i == instr.cmd2)
							{
								if (isBuiltIn(instr.tokens[i]))
								{
									addToken(&tempInstr3, instr.tokens[i]);
								}
								else 
								{
									char* temp = resolvePath(instr.tokens[i]);
									if (!isExecutable(temp))
									{
										error = TRUE;
										printf("%s: Command not foun-d.\n", instr.tokens[i]);
										break;
									}
									addToken(&tempInstr3, instr.tokens[i]);
								}
							}
							else
							{
								if (instr.tokens[i][0] == '-')
									addToken(&tempInstr3, instr.tokens[i]);
								else if (instr.tokens[i][0] == '$')
								{
									char* temp = getenv(checkEnv(instr.tokens[i]));
									if (temp == NULL)
									{
										error = TRUE;
										printf("%s: Undefined variable.\n");
										break;
									}
									addToken(&tempInstr3, temp);
								}
								
								//files and directories
								else 
									addToken(&tempInstr3, expandPath(instr.tokens[i]));
							}
						}
						addNull(&tempInstr3);
						
						//execute cmd2
						
						int status2;
						int fd2[3];
						if(fork() == 0)
						{
							pipe(fd2);
							if(fork() == 0)
							{
								if(fork() == 0)
								{
									//cmd1 writer
									close(1);
									dup(fd2[1]);
									close(fd2[0]);
									close(fd2[1]);
									//execute cmd1
									if (isBuiltIn(tempInstr.tokens[0]))
										builtIns(&tempInstr);
									else
										execute(tempInstr.tokens);
								}
								else
								{
									//cmd2 reader
									close(0);
									dup(fd2[0]);
									close(fd2[0]);
									close(fd2[1]);
									//execute
									
									//cmd2 writer
									close(2);
									dup(fd2[2]);
									close(fd2[1]);
									close(fd2[2]);
									//execute
									
									if (isBuiltIn(tempInstr3.tokens[0]))
										builtIns(&tempInstr2);
									else
										execute(tempInstr2.tokens);
								}
							}
							else
							{
								//cmd3 reader
								close(1);
								dup(fd2[1]);
								close(fd2[1]);
								close(fd2[2]);
								//execute
								if (isBuiltIn(tempInstr3.tokens[0]))
									builtIns(&tempInstr3);
								else
									execute(tempInstr3.tokens);
							}
						}
						else
						{
							//parent
							close(fd2[0]);
							close(fd2[1]);
							close(fd2[2]);
							waitpid(pid1, &status2,0);
							waitpid(pid2, &status2, 0);
							waitpid(pid3,&status2, 0);
						}
					}
					
					//1 pipe
					else
					{
						int status;
						int fd[2];
						if(fork() == 0)
						{
							pipe(fd);
							//child
							if(fork() == 0)
							{
								//cmd1 writer
								close(STDOUT_FILENO);
								dup(fd[1]);
								close(fd[0]);
								close(fd[1]);
								
								//execute command
								if (isBuiltIn(tempInstr.tokens[0]))
									builtIns(&tempInstr);
								else
									execute(tempInstr.tokens);
								
								// exit(1);
							}
							//parent
							else
							{
								//cmd2 reader
								close(STDIN_FILENO);
								dup(fd[0]);
								close(fd[0]);
								close(fd[1]);
								
								//execute command
								if (isBuiltIn(tempInstr2.tokens[0]))
									builtIns(&tempInstr2);
								else
									execute(tempInstr2.tokens);
								
								// exit(1);
							}
							
						}
						else
						{
							//parent shell
							close(fd[0]);
							close(fd[1]);
							waitpid(pid1, &status, 0);
							waitpid(pid2, &status, 0);
						}
					}
				}
				
				//no piping or redirection normal execution
				else if(background)
					backgroundProc(instr.tokens,instr.numTokens);
				else if (isBuiltIn(tempInstr.tokens[0]))
					builtIns(&tempInstr);
				else
					execute(tempInstr.tokens);
				
			}
			
			clearInstruction(&tempInstr);
			clearInstruction(&tempInstr2);
			clearInstruction(&tempInstr3);
		}
		

		
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
	instr_ptr->cmd1 = -1;
	instr_ptr->cmd2 = -1;
	instr_ptr->cmd3 = -1;
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

///expands path to absolute path
///use for file
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
	//from root dir
	else if (path[0] != '/')
	{
		int len = strlen(getenv("PWD")) + strlen(path);
		newPath = (char*)malloc((len + 2) * sizeof(char));
		strcpy(newPath, getenv("PWD"));
		strcat(newPath, "/");
		strcat(newPath, path);
	}
	//relative
	else
	{
		newPath = (char*)malloc((strlen(path) + 3) * sizeof(char));
		strcpy(newPath, "./");
		strcat(newPath, path);
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

int isExecutable(char* file)
{
	if (access(file, X_OK) != -1)
		return TRUE;
	return FALSE;
}

///expands path of executables to absolute path
///searches in PWD and all of PATH
///return null if not found
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
		execv(resolvePath(cmd[0]), cmd);
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
	|| strcmp(token, "echo") == 0 || strcmp(token, "jobs") == 0)
		return TRUE;
	return FALSE;
}

void builtIns(instruction* instr)
{
	char **toks = instr->tokens;

	int numToks = instr->numTokens;


	if(numToks > 0)
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
				if(chdir(resolvePath(toks[1])) == 0)
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

			printf("Exiting now\n");
			printf("%d instruction(s) entered\n");

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
char* checkEnv(char * tkn)     
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
		if (toks[i] == NULL)
			break;
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

void backgroundProc(char ** cmd, int numToks)
{	

if(strcmp(cmd[0],"ok") != 0){

	cmd[numToks-1] = NULL;
	int numCmds = numToks-1;
	
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
		setpgid(0, 0);
		//sleep(3);
		execvp(cmd[0],cmd);
		printf("Problem executing(in BG) %s\n", cmd[0]);
		exit(1);
	}
	else
	{
		//parent
		insertQueue(pid,cmd,numCmds);
		printf("\n[%d]	[%d]\n",child_nb-1,pid);

		waitpid(pid,&status,WNOHANG);
	}
}

}



void jobs(instruction* instr)
{
	int j = 0;
	for(int i = 1;i < child_nb;i++)
	{
		printf("[%d]    [%d]    [ \n",i,q[i].pid);
		while(strcmp(q[i].name[j],NULL) != 0)
		{
			printf("%s ",q[i].name[j]);
			j++;
		}
		printf("]\n");
	}
}

void insertQueue(pid_t pid,char **cmds, int numCmds)
{
	q[child_nb].pid = pid;

    int i = 0;

    while(i < numCmds)
    {
		strcpy(q[child_nb].name[i],cmds[i]);				//copying the commands into queue
		i++;
    }
    strcpy(q[child_nb].name[i],"&");
	child_nb++;

}


void deleteJob(pid_t pid)
{
     int i = 0,j;
     for(i = 0;i< child_nb ; i++)
	 {
		if(q[i].pid == pid)						//finding the particular pid element
	       break;
	 }
     for(j = i + 1;j <= child_nb; j++)
		q[j-1]=q[j];							//shifting the whole queue by one step backward

     child_nb--;
}


void checkBg()
{
	int status;		//for waitpid

	for(int i = 0, j = 0; i < child_nb; i++)	//loop to check for finished background processes
	{
		int cPid = waitpid(q[i].pid,&status,WNOHANG);
		if(cPid != 0)
		{
			printf("[%d]+	[",i);
			//printf("\n%s\n",q[i].name[0]);

			while(strcmp(q[i].name[j], "&") != 0)
			{
				printf("%s ",q[i].name[j]);
				j++;
			}
			printf("%s",q[i].name[j]);
			printf("]\n");

			deleteJob(q[i].pid);
		}
	}

}
