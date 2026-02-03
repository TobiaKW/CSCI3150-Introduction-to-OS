//Group member: Wong Cheuk Yin 1155192671
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define WRITE_END 1
#define READ_END 0

int shell_execute(char ** args, int argc)//args >> the line of command, argc >> the number of elements (include the \0 at last kamosirenn)
{
	int pipepos[3] = {-1, -1, -1}; //need to handle up to 3 pipe
	int j = 0;
	//printf("argc=%d\n", argc);
	for(int i = 0; i < argc - 1; i++){ //to ask: why need argc-1, not argc
		//printf("i%d\n", i);
		if(strcmp(args[i], "|") == 0){
			pipepos[j] = i;
			j = j + 1;
			//printf("j%d\n\n", j);
			if(j == 3){
				break;
			}
		}
	}
	//printf("pipe positions are %d %d %d || ", pipepos[0], pipepos[1], pipepos[2]); //for checking

	int child_pid, wait_return, status;

	if(pipepos[0] == -1){ //if no pipe
		if (strcmp(args[0], "EXIT") == 0) // if argument == EXIT , return -1(for break loop, error/exit)
			return -1; 
		
		if((child_pid = fork()) < 0) // fork return a value to child_pid, if it is <0, that means fork error. 
		{
			printf("fork() error \n");
		}
		else if (child_pid == 0) // if the returned child_pid == 0, it means child process running
		{//attempt to run the external commands, execvp() is used for replacing the code(which is same as parent originally) with the command entered i.e. ls, ps
			if ( execvp(args[0], args) < 0) //use IF to execute may be better!
			{ 
				printf("execvp() error \n");
				exit(-1);	//exit if command not found in bin
			}



		}
		else // If positive, it should be the child's pid. then the parent waits
		{
			if ((wait_return = wait(&status)) < 0){
				printf("wait() error \n"); 
			}
		}
			
		return 0;
	}
	else{
		//there is pipe (2+ command)
		char ** argv1, **argv2, **argv3 = NULL, **argv4 = NULL;
		int pipeFD1[2], pipeFD2[2], pipeFD3[2];
		argv1 = malloc(sizeof (char *) * (pipepos[0] + 1)); // +1 >> include the NULL pointer

		//create pipes
		if(pipepos[0] != -1 && pipe(pipeFD1) < 0){
			perror("pipe1");
			exit(-1);
		}
		if(pipepos[1] != -1 && pipe(pipeFD2) < 0){
			perror("pipe2");
			exit(-1);
		}
		if(pipepos[2] != -1 && pipe(pipeFD3) < 0){
			perror("pipe3");
			exit(-1);
		}

		for(int i = 0; i < pipepos[0]; i++){//extract first command
			argv1[i] = args[i];
			//printf("%d", i);
		}
		argv1[pipepos[0]] = NULL;
		if(pipepos[1] == -1){//extract second command 
			argv2 = malloc(sizeof(char *) * (argc - pipepos[0])); 
			for(int i = pipepos[0]+1; i < argc - 1; i++){
				argv2[i-pipepos[0]-1] = args[i];
				//printf("%d", i);
			}
			argv2[argc-pipepos[0]-2] = NULL;
		}
		else{
			argv2 = malloc(sizeof (char *) * (pipepos[1] - pipepos[0]) ); 
			for(int i = pipepos[0]+1; i < pipepos[1]; i++){
				argv2[i-pipepos[0]-1] = args[i];
				//printf("%d", i);
			}
			argv2[pipepos[1]-pipepos[0]-1] = NULL;
		}
		
		

		int pid1 = fork(); //first child
		if(pid1 < 0){ //fork not created properly
			perror("fork 1");
		}
		else if(pid1 == 0){ //there exists 1 pipe, thus use dup2() to duplicate the file descripter i.e. pipeFD1 
			//dup() use the lowest available file descriptor
			//dup2() can specific the new file descriptor
			close(pipeFD1[READ_END]);
			dup2(pipeFD1[WRITE_END], STDOUT_FILENO);
			close(pipeFD1[WRITE_END]);
			if(pipepos[1] != -1){ //close all unused file descriptor
				close(pipeFD2[WRITE_END]);
				close(pipeFD2[READ_END]);
			}
			if(pipepos[2] != -1){
				close(pipeFD3[WRITE_END]);
				close(pipeFD3[READ_END]);
			}
			if(execvp(argv1[0], argv1) < 0){ //execute first command
				perror("execvp cmd1");
				exit(-1); 
			}
		}

		int pid2 = fork();//second child
		if(pid2 < 0){ 
			perror("fork 2");
		}
		else if(pid2 == 0){ 
			close(pipeFD1[READ_END]);
			dup2(pipeFD1[READ_END], STDIN_FILENO); //read from pipe1
			close(pipeFD1[WRITE_END]);
			if(pipepos[1] != -1){
				dup2(pipeFD2[WRITE_END], STDOUT_FILENO);
				close(pipeFD2[WRITE_END]);
				close(pipeFD2[READ_END]);
			}
			if(pipepos[2] != -1){
				close(pipeFD3[WRITE_END]);
				close(pipeFD3[READ_END]);
			}
			if(execvp(argv2[0], argv2) < 0){ // execute
				perror("execvp cmd2");
				exit(-1); 
			}
		}

		int pid3 = -1;
		int pid4 = -1;

		if(pipepos[1] != -1){//there is the third command

			if(pipepos[2] != -1){//extract the third command
				argv3 = malloc(sizeof (char *) * (pipepos[2] - pipepos[1]) );
				for(int i = pipepos[1]+1; i < pipepos[2]; i++){
					argv3[i-pipepos[1]-1] = args[i];
					//printf("%d", i);
				}
				argv3[pipepos[2]-pipepos[1]-1] = NULL;
			}
			else{
				argv3 = malloc(sizeof (char *) * (argc - pipepos[1]));
				for(int i = pipepos[1]+1; i < argc - 1; i++){
					argv3[i-pipepos[1]-1] = args[i];
					//printf("%d", i);
				}
				argv3[argc-pipepos[1]-2] = NULL;
			}
			int pid3 = fork();
			if(pid3 < 0){ // third child
				perror("fork 3");
			}
			else if(pid3 == 0){ 
				close(pipeFD1[0]);
				close(pipeFD1[1]);
				close(pipeFD2[0]);
				dup2(pipeFD2[0], STDIN_FILENO); 
				close(pipeFD2[1]);
				if(pipepos[2] != -1){
					dup2(pipeFD3[WRITE_END], STDOUT_FILENO);
					close(pipeFD3[0]);
					close(pipeFD3[1]);
				}
				if(execvp(argv3[0], argv3) < 0){
					perror("execvp cmd3");
					exit(-1); 
				}
			}


			if(pipepos[2] != -1){//there is the fourth command
				char ** argv4;
				argv4 = malloc(sizeof (char *) * (argc - pipepos[2]) );
				for(int i = pipepos[2]+1; i < argc - 1; i++){ //extract fourth command
					argv4[i-pipepos[2]-1] = args[i];
					//printf("%d", i);
				}
				argv4[argc-pipepos[2]-2] = NULL;
				int pid4 = fork();//fourth child
				if(pid4 < 0){ 
					perror("fork 4");
				}
				else if(pid4 == 0){ 
					close(pipeFD1[WRITE_END]);
					close(pipeFD1[READ_END]);
					close(pipeFD2[WRITE_END]);
					close(pipeFD2[READ_END]);
					close(pipeFD3[WRITE_END]);
					dup2(pipeFD3[READ_END], STDIN_FILENO); 
					close(pipeFD2[READ_END]);
					if(execvp(argv4[0], argv4) < 0){
						perror("execvp cmd4");
						exit(-1);
					}
				}
			}


		}

		//parent close pipes
		close(pipeFD1[READ_END]);
		close(pipeFD1[WRITE_END]);
		if(pipepos[1] != -1){
			close(pipeFD2[READ_END]);
			close(pipeFD2[WRITE_END]);
		}
		if(pipepos[2] != -1){
			close(pipeFD3[READ_END]);
			close(pipeFD3[WRITE_END]);
		}

		//wait for all child process
		waitpid(pid1, &status, 0);
		waitpid(pid2, &status, 0);
		if(pipepos[1] != -1){
			waitpid(pid3, &status, 0);
		}
		if(pipepos[2] != -1){
			waitpid(pid4, &status, 0);
		}
		
		//free all allocated memory
		free(argv1);
		free(argv2);
		if(pipepos[1] != -1){
			free(argv3);
		}
		if(pipepos[2] != -1){
			free(argv4);
		}
	
	}
	return 0;
}
