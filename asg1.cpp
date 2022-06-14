#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <signal.h>
#include <sstream>
#include <fstream>
#include <fcntl.h>

void sigchld_handler(int sig) {
	while(waitpid(-1, 0, WNOHANG) > 0);  
}

int main(int argc, char **argv)
{
	signal(SIGCHLD, sigchld_handler);
	char path[255];
	char *input = NULL,*prompt,*args[5];
	pid_t pid;
	int counter=1;
	pid_t *pidlist=new pid_t[1];

	while(1){
		getcwd(path,sizeof(path));
		prompt=new char[strlen("minshell:")+strlen(path)+strlen("$ ")+1];
		strcpy(prompt, "minshell:");
		strcat(prompt, path);
		strcat(prompt, "$ ");
		input = readline(prompt);
		char *p = strtok(input," \t");
		
		if (!p) {
			std::cout << "No command!\n";  //empty command    
		}
		
		else if(strcmp(p,"cd")==0){  //Changing the Current Directory
			if(chdir(strtok(NULL," \t"))==-1)
			std::cout<<"No such directory!"<<std::endl;  //user entered wrong path
		}
		
		else if(strcmp(p,"dir")==0){  //Listing the Content of the Current Directory
			DIR *dr;
			struct dirent *en;
			dr=opendir(path);
			if(dr){
				while((en=readdir(dr))!=NULL){
					std::cout<<en->d_name<<std::endl;  //read all directory
				}
				closedir(dr); //close all directory
			}
		}
		
		else if(strcmp(p,"run")==0){  //Background Execution of Programs
			args[0]=strtok(NULL," \t");
			args[1]=strtok(NULL," \t");
			args[2]=strtok(NULL," \t");
			args[3]=strtok(NULL," \t");
			args[4]=NULL;
			std::ifstream file(args[0]);
			if(args[0]==NULL||args[1]==NULL||args[2]==NULL||args[3]==NULL||atoi(args[2])==0||atoi(args[3])==0||file.fail()){
				std::cout<<"Invalid input!"<<std::endl;
			}
			else
			{
				pid = fork();
				if(pid>0){  // parent process
					pidlist[counter-1]=pid;
					pid_t *temp=new pid_t[++counter];
					for(int i=0;i<counter-1;i++){
						temp[i]=pidlist[i];
					}
					delete []pidlist;
					pidlist=temp;
					temp=NULL;    
				}
				else if(pid==0){  // child process          
					close(1);
					execvp(args[0],args);      
					exit (0);
				}
				else {  // fork failed
					printf("fork () failed!\n");
					exit (EXIT_FAILURE);
				}   
			}
			file.close();
		}
		
		else if(strcmp(p,"kill")==0){  //Terminating Background Processes
			if(kill(atoi(strtok(NULL," \t")),SIGTERM)!=0)
			std::cout<<"No such process is running"<<std::endl;
		}
		
		else if(strcmp(p,"ps")==0){  //Listing All Background Processes
			char filename[1000];
			for(int i=0;i<counter-1;i++){
				sprintf(filename, "/proc/%d/stat", pidlist[i]);
				FILE *f = fopen(filename, "r");
				if(f!=NULL){
					sprintf(filename, "/proc/%d/exe", pidlist[i]);
					printf("%d: %s\n", pidlist[i],realpath(filename, NULL));
					fclose(f);
				}
			}
		}
		
		else if(strcmp(p,"quit")==0){  //Exiting the Shell
			for(int i=0;i<counter-1;i++)
			kill(pidlist[i],SIGTERM);
			return 0;
		}
		
		else {  //wrong command
			std::cout << "No such command!\n";  
		}
	}
	return 0;
}