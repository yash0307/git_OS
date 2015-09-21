/*** Yash Patel, 201301134
  CSE, IIIT-H	 ***/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/utsname.h>
#include<sys/types.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/wait.h>

/////DECLARE STRUCTS HERE/////
typedef struct {
	char starting_directory[200];
	char user_name[200];
	char system_name[200];
}Global;

typedef struct {
	char command[200];
	bool r_in;
	bool r_out;
	bool r_in_append;
	bool r_out_append;
	char file_input[200];
	char file_output[200];
}Redirect;

typedef struct {
	int counter;
	int indexes[200];
	bool exists_in_command;
}Background;
/////DECLARE GLOBAL VARIABLES/////
Global instance;
Redirect re;
Background bg;
#define TRUE 1
#define FALSE 0
/////Need to check for background process.
/////If there are background processes
/////Maintian them in indexes.
/////**** DONOT RESET, INITITATE FOR AN INSTANCE OF SHELL /////
///// Functions specific to background processes goes herer////
///// Just Reset exists each time /////
void initializeBg()
{
	bg.counter = 0;
	bg.exists_in_command = FALSE;
	int i;
	for(i=0;i<200;i++)
	{
		bg.indexes[i] = 0;
	}
}
void checkBg(char command_parsed[100][100], int command_counter)
{
	int i;
	for(i=0;i<command_counter;i++)
	{
		if(strcmp(command_parsed[i], "&")==0)
		{
			bg.exists_in_command = TRUE;
		}
	}
}
void resetExistsBg()
{
	bg.exists_in_command = FALSE;
}
/////Ending, Background process/////
/////Need to check for redirection in command given.
/////***** RESET THESE AFTER EACH COMMAND /////
///// Functions specific to redirections goes here
void initializeRe()
{
	strcpy(re.command,"");
	re.r_in = FALSE;
	re.r_out = FALSE;
	re.r_in_append = FALSE;
	re.r_out_append = FALSE;
	strcpy(re.file_input,"");
	strcpy(re.file_output,"");
}
void resetRe()
{
	strcpy(re.command,"");
	re.r_in = FALSE;
	re.r_out = FALSE;
	re.r_in_append = FALSE;
	re.r_out_append = FALSE;
	strcpy(re.file_input,"");
	strcpy(re.file_output,"");
}
void checkReIn(char command_parsed[100][100], int command_counter)
{
	int i;
	for(i=0;i<command_counter;i++)
	{
		int cmd_len = strlen(command_parsed[i]);
		int j;
		for(j=0;j<cmd_len;j++)
		{
			if(command_parsed[i][j]=='<' || strcmp(command_parsed[i],"<<")==0)
			{
				if(i < command_counter-1)
				{
					re.r_in = TRUE;
					strcpy(re.file_input, command_parsed[i+1]);
					return;
				}
				else
				{
					printf("Enter Valid Input File\n");
					return;
				}
			}
		}
	}
}

void checkReOut(char command_parsed[100][100], int command_counter)
{
	int i;
	for(i=0;i<command_counter;i++)
	{
		int cmd_len = strlen(command_parsed[i]);
		int j;
		for(j=0;j<cmd_len;j++)
		{
			if(command_parsed[i][j]=='>' || strcmp(command_parsed[i],">>")==0)
			{
				if(i < command_counter-1)
				{
					re.r_out = TRUE;
					strcpy(re.file_output, command_parsed[i+1]);
					return;
				}
				else
				{
					printf("Enter Valid Output file\n");
					return;
				}
			}
		}
	}
}

void checkAppend(char command_parsed[100][100], int command_counter)
{
	int i;
	for(i=0;i<command_counter;i++)
	{
		if(strcmp(command_parsed[i],">>")==0)
		{
			re.r_out_append = TRUE;
		}
		if(strcmp(command_parsed[i], "<<")==0)
		{
			re.r_in_append = TRUE;
		}
	}
}
/////END OF REDIRECTION FUNCTIONS /////
/////This function gets user name, system name and starting working directory.
void getInstanceDetails()
{
	struct utsname unameData;
	if(uname(&unameData)!=0)
	{
		perror("Error: ");
		return;
	}

	char pwd[1024];
	if (getcwd(pwd, sizeof(pwd)) == NULL)
		perror("getcwd() error");
	else
	{
		strcpy(instance.starting_directory, pwd);
		strcpy(instance.user_name, unameData.nodename);
		strcpy(instance.system_name, unameData.sysname);
	}
}

/////This function gets the differnce between 
/////starting directory and present directory, 
/////and generates the shell prompt.
void generateShellPrompt()
{
	char pwd[1024];
	char dict[1024]={'\0'};
	if(getcwd(pwd, sizeof(pwd)) == NULL)
		perror("getcwd() error");
	else
	{
		char st_dict[200];
		strcpy(st_dict, instance.starting_directory);
		char *temp=&pwd[0];
		char *temp_1=&st_dict[0];
		while(*temp == *temp_1)
		{
			temp = temp + sizeof(char);
			temp_1 = temp_1 + sizeof(char);
		}
		strcpy(dict, "~ ");
		strcpy(dict, temp);
	}

	if(strcmp(pwd, instance.starting_directory)==0)
	{
		strcpy(dict, "~ ");
	}

	if(strcmp(dict, "")==0)
	{
		strcpy(dict, pwd);
	}

	printf("%s@%s:%s>", instance.user_name, instance.system_name, dict);

}

/////This function executes all the built in functions.
void executeBuiltInCommand(char command_input[100][100])
{

	//If command is pwd, then simply print the present working directory and return.
	if(strcmp(command_input[0],"pwd")==0)
	{
		char pwd[1024];
		if(getcwd(pwd, sizeof(pwd)) == NULL)
			perror("getcwd() error");
		else
		{
			printf("%s\n", pwd);
			return;
		}
	}

	//change the directory using chdir.
	else if(strcmp(command_input[0],"cd")==0)
	{
		char dict[1024];
		if(getcwd(dict, sizeof(dict)) == NULL)
			perror("getcwd() error");
		else
		{
			strcat(dict, "/");
			strcat(dict, command_input[1]);
			chdir(dict);
		}
	}

	//Echo out the second variable.
	else if(strcmp(command_input[0], "echo")==0)
	{
		//When the variable is environment variable, detecting "$" sign over here.
		if((command_input[1][0]+'0')==36 + '0')
		{
			//Extract the env variable out.
			char env_var[20]={'\0'};
			char *temp = &command_input[1][1];
			strcpy(env_var, temp);
			printf("%s\n", getenv(env_var));
		}

		//When variable is not environment variable.
		else
		{
			int i=1;
			while(strcmp(command_input[i], "")!=0)
			{
				printf("%s ", command_input[i]);
				i++;
			}
			printf("\n");
		}
	}
}

int main(int argc, char *argv[])
{
	char command[200];

	getInstanceDetails();

	initializeRe();

	while(1)
	{
		generateShellPrompt();

		initializeBg();

		gets(command);

		const char token_parm[2] = ";";
		char *token;
		char command_parsed[100][100];
		int token_counter = 0;

		token = strtok(command, token_parm);

		while(token != NULL)
		{	
			strcpy(command_parsed[token_counter], token);
			token_counter++;
			token = strtok(NULL, token_parm);
		}

		int i;

		for(i=0; i<token_counter; i++)
		{
			char *specific_token;
			const char specific_parm[2] = " ";
			char specific_parsed[100][100];
			int specific_counter = 0;

			specific_token = strtok(command_parsed[i], specific_parm);

			while(specific_token != NULL)
			{
				strcpy(specific_parsed[specific_counter], specific_token);
				specific_counter++;
				specific_token = strtok(NULL, specific_parm);
			}

			//Check if there is redirection in command.
			checkReIn(specific_parsed, specific_counter);
			checkReOut(specific_parsed, specific_counter);
			checkAppend(specific_parsed, specific_counter);

			//Check Background process.
			checkBg(specific_parsed, specific_counter);

			if(strcmp(specific_parsed[0], "cd")==0 || strcmp(specific_parsed[0], "echo")==0 || strcmp(specific_parsed[0],"pwd")==0)
			{
				executeBuiltInCommand(specific_parsed);
			}

			else if(strcmp(specific_parsed[0],"exit")==0)
			{
				return 0;
			}

			else
			{

				pid_t pid;
				int status;

				pid = fork();

				int t;
				char *arg[100];

				int in_cnt=1000;
				int out_cnt=1000;

				//If re.r_in is False and re.r_out is false then
				//give values to arg here itself.
				if(re.r_in==FALSE && re.r_out==FALSE && bg.exists_in_command==FALSE)
				{
					for(t=0;t<specific_counter;t++)
					{
						arg[t] = specific_parsed[t];
					}
				}

				if(pid < 0)
				{
					perror("ERROR: in forking new process\n");
					exit(1);
				}


				else if(pid==0)
				{
					//check for redirection input//
					if(re.r_in)
					{
						int in_i;
						for(in_i=0 ; in_i<specific_counter ; in_i++)
						{
							if(strcmp(specific_parsed[in_i],"<")==0 || strcmp(specific_parsed[in_i],"<<")==0)
							{
								in_cnt = in_i;
								break;
							}
						}
						int fd_in = open(re.file_input, O_RDONLY, 0);
						if(fd_in < 0)
						{
							perror("ERROR : can't open input file\n");
						}
						dup2(fd_in, STDIN_FILENO);
						close(fd_in);
					}

					//check for redirection output//
					if(re.r_out)
					{
						int out_i;
						for(out_i=0 ; out_i<specific_counter ; out_i++)
						{
							if(strcmp(specific_parsed[out_i],">")==0 || strcmp(specific_parsed[out_i],">>")==0)
							{
								out_cnt = out_i;
								break;
							}
						}

						int fd_out;
						if(re.r_out_append==FALSE)
						{
							fd_out = creat(re.file_output, 0644);
							if(fd_out<0)
								perror("ERROR : can't append in Output File\n");
						}
						else if(re.r_out_append==TRUE)
						{
							fd_out = open(re.file_output, O_WRONLY | O_APPEND);
							if(fd_out <0)
								perror("ERROR : can't create Output file\n");
						}
						dup2(fd_out, STDOUT_FILENO);
						close(fd_out);
					}
					//If both re.r_in and re.r_out are true.
					if(re.r_in==TRUE || re.r_out==TRUE)
					{
						if(in_cnt > out_cnt)
						{
							for(t=0;t<out_cnt;t++)
							{
								arg[t] = specific_parsed[t];
							}
						}
						else if(in_cnt < out_cnt)
						{
							for(t=0;t<in_cnt;t++)
							{
								arg[t] = specific_parsed[t];
							}
						}
					}

					//Check if there is background process.
					if(bg.exists_in_command==TRUE)
					{
						for(t=0;t<specific_counter;t++)
						{
							if(strcmp(specific_parsed[t],"&")==0)
							{
								break;
							}
							else
							{
								arg[t] = specific_parsed[t];
							}
						}
					}
					int ret;
					if(bg.exists_in_command==TRUE)
					{
						setpgid(0, 0);
					}
					ret = execvp(arg[0], arg);
					if(ret < 0)
					{
						perror("ERROR : in execvp() -- ");
						exit(1);
					}
					exit(0);
				}

				//Need to reset arg vars each time.
				for(t=0;t<specific_counter;t++)
				{
					arg[t] = '\0';
				}

				//If command to be execute is !bg process
				if(bg.exists_in_command==FALSE)
				{
					wait(NULL);
				}
				//If command to be executed is bg process
				else if(bg.exists_in_command==TRUE)
				{
					printf("BG Process pid:%d\n", pid);
				}
			}
			//Reset bg.exists_in_command.
			resetExistsBg();
			//Reset redirection instance everytime.
			resetRe();
		}
	}
	return 0;
}
