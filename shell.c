/*** Yash Patel, 201301134
  CSE, IIIT-H	 ***/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/utsname.h>
#include<sys/types.h>

typedef struct {
	char starting_directory[200];
	char user_name[200];
	char system_name[200];
}Global;

Global instance;

//This function gets user name, system name and starting working directory.
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

//This function gets the differnce between starting directory and present directory, and generates the shell prompt.
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

//This function executes all the built in functions.
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

	while(1)
	{
		generateShellPrompt();

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
				for(t=0;t<specific_counter;t++)
				{
					arg[t] = specific_parsed[t];
				}

				if(pid < 0)
				{
					perror("ERROR: in forking new process\n");
					exit(1);
				}

				else if(pid==0)
				{
					int ret;
					ret = execvp(arg[0], arg);
					if(ret < 0)
					{
						perror("ERROR : in execlp()\n");
						exit(1);
					}
					exit(0);
				}

				wait();
			}
		}
	}
	return 0;
}
