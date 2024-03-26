#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

// define constants
#define MAX_CMD_ARGUMENTS 1024
#define MAX_CMD_TOKENS 100
#define MAX_HISTORY 100 
#define PROMPT "sish> "
#define BUFFER_SIZE 1024
#define MAX_ARGS 64

// define global variables
char history[MAX_HISTORY][MAX_CMD_ARGUMENTS];
int history_count = 0;
int front = 0; 
int rear = MAX_HISTORY - 1;

// this function adds the command to the history
void addtoHistory(const char *cmd){
  char trimmed_cmd[MAX_CMD_ARGUMENTS];
  strncpy(trimmed_cmd, cmd, MAX_CMD_ARGUMENTS);
  trimmed_cmd[strcspn(trimmed_cmd, "\n")] = '\0';
  // add the command to the circular history buffer
  if(history_count >= MAX_HISTORY){
    strcpy(history[rear], trimmed_cmd);
    rear = (rear + 1) % MAX_HISTORY;
    front = (front + 1) % MAX_HISTORY;
    history_count++;
    // if the history is not full, add the command to the history
  }else{
    strcpy(history[history_count], trimmed_cmd);
    history_count++;
  }
}
// this function prints the history
void printHistory(){
  int i = front;
  // if the history is not full, print the history
  if(history_count < MAX_HISTORY){
    for(i = 0; i < history_count; i++){
      printf("%d %s\n", i, history[i]);
    }
    return;
  }
  else if(history_count >= MAX_HISTORY){
    i = front;
    int counter = 0;
    do{
      printf("%d %s\n", counter, history[i]);
      i = (i + 1) % MAX_HISTORY;
      counter++;
    }while(i != front);
  }
  return;
}

// this function clears the history
void clearHistory(){
  // for loop which goes through the history and clears it
  for(int i = 0; i < MAX_HISTORY; i++){
    memset(history[i], 0, MAX_CMD_ARGUMENTS);
  }
  history_count = 0;
}

char* printHistoryOffset(int offset){
  int i = front;
  // if the history is not full, return the command at the offset
  if(history_count >= MAX_HISTORY){
    return history[i + offset % MAX_HISTORY];
  }else{
    return history[offset % history_count];
  }
}

// this function parses the command line and stores the arguments in argv_ptr
void parseCommand(char *token, char **argv_ptr){
  char *saveptr1;
  char *delimiter = " ";
  // tokenizes the command line
  token = strtok_r(token, delimiter, &saveptr1);
  int i = 0;
  // while loop which tokenizes the command line and stores the arguments in argv_ptr
  while(token != NULL && i < MAX_CMD_ARGUMENTS){
    argv_ptr[i] = token;
    token = strtok_r(NULL, delimiter, &saveptr1);
    i++;
  }
  argv_ptr[i] = NULL;
}


void parse(char *line, char **argv){
  // loop until the end of the line
  while(*line != '\0'){
    // skip white spaces
    while (*line == ' ' || *line == '\t' || *line == '\n'){
      *line++ = '\0';
    }
    // check if the end of the line is reached
  if(*line == '\0'){
    break;
    
  }else{
      *argv++ = line;
      // move to the end of the argument
      while(*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n'){
        line++; // move to the next character
      }
    }
  }
}

// this function executes the piped command
void executePipedCommand(char **commands[]){
  int pipefd[MAX_CMD_TOKENS - 1][2];
  pid_t pid;
  int status;
  int numCmds = 0;
  int i;

// while loop which counts the number of commands
  while(commands[numCmds] != NULL){
    numCmds++;
  }
  // for loop which creates the pipes
  for(i = 0; i < numCmds - 1; i++){
    if(pipe(pipefd[i]) == -1){
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }
  // for loop which forks the processes
  for(i = 0; i < numCmds; i++){
    pid = fork();
    if(pid == -1){
      perror("fork");
      exit(EXIT_FAILURE);
    } 
    // if the process is a child process
    else if(pid == 0){
      if(i != 0){
        dup2(pipefd[i - 1][0], STDIN_FILENO);
      }
      // if the process is not the last process
      if(i != numCmds - 1){
        dup2(pipefd[i][1], STDOUT_FILENO);
      }
      // for loop which closes the pipes
      for(int j = 0; j < numCmds - 1; j++){
          close(pipefd[j][0]);
          close(pipefd[j][1]);
      }
      // executes the command
        execvp(commands[i][0], commands[i]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
  }

  // for loop which closes the pipes
  for(i = 0; i < numCmds - 1; i++){
    close(pipefd[i][0]);
    close(pipefd[i][1]);
  }

// waits for child processes to finish
  while(wait(&status) != -1);
}

void executeCommand(char *input_token, char *saveptr1, char *delimiter, char *token){
    pid_t pid = fork();
    // if execution fails
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        char *args[MAX_CMD_ARGUMENTS];
        int i = 0;
        args[i++] = input_token;
        // Tokenize the command line, store the arguments in args
        while ((token = strtok_r(NULL, delimiter, &saveptr1)) != NULL) {
            args[i++] = token;
            }
        args[i] = NULL;
        // Execute the command
        execvp(args[0], args);
        // If execvp fails, print error message
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(NULL); // Wait for the child process to finish
    }
}

// this function checks if the command line has a pipe
int checkPipe(char *line){
  // if the command line has a pipe, return 1
  if(strchr(line, '|') != NULL){
    return 1;
  }else{
    return 0;
  }
}

int main(int argc, char *argv[]) {
  // Define the required variables
  char *token = NULL;
  char *delimiter = " ";
  char line[BUFFER_SIZE];
  char **argv_ptr = NULL;
  char **commands[MAX_CMD_ARGUMENTS];
  int i = 0;
  char *saveptr1;

while (1) {
  // display prompt when waiting for input
  printf("%s", PROMPT);
    if (fgets(line, BUFFER_SIZE, stdin) != 0){
      line[strcspn(line, "\n")] = '\0';
      addtoHistory(line);

// if the command line has a pipe
    if(checkPipe(line) == 1){
      // if user enters exit, the shell will exit
      if(strcmp(line, "exit") == 0){
        exit(0);
      }
    
      addtoHistory(line);
      char *lineCopy = strdup(line);
      if(lineCopy == NULL){
        perror("strdup");
        exit(EXIT_FAILURE);

      }
      // tokenizes the command line
      char *token = strtok(lineCopy, "|");
      while(token != NULL && i < MAX_CMD_TOKENS - 1){
        argv_ptr = (char **)malloc(MAX_ARGS * sizeof(char *));
        // if malloc fails, print error message
        if(argv_ptr == NULL){
          perror("malloc");
          exit(EXIT_FAILURE);
        }
        // parses the command line
        parse(token, argv_ptr);
        commands[i] = argv_ptr;
        token = strtok(NULL, "|");
        i++;
      }
    
        commands[i] = NULL;
        executePipedCommand(commands);

        free(lineCopy);
        
        // free the memory
        for(int k = 0; k < i; k++){
          free(commands[k]);
        }
      }else{
        char *input_token = strtok_r(line, delimiter, &saveptr1);
        // checks for exit, if user enters exit, the shell will exit
        if (strcmp(input_token, "exit") == 0) {
            printf("Exiting\n");
            break;
        }
        // check for cd
        if (strcmp(input_token, "cd") == 0) {
            // get the next token (directory to change to)
            char *subtoken = strtok_r(NULL, delimiter, &saveptr1);
            // if the token is not null, change to the directory provided by user
            if (subtoken != NULL) {
                if (chdir(subtoken) != 0) {
                    perror("cd");
                }
                // if chdir fails, print error message
            } else {
                fprintf(stderr, "cd failed: missing directory\n");
            }
            continue;
        } 
        // if the user enters history, print the history
        if(strcmp(input_token, "history") == 0){
          char *subtoken = strtok_r(NULL, delimiter, &saveptr1);
          if(subtoken == NULL){
              printHistory();
              continue;
          }
          // if the user enters -c, clear the history
          else if(strcmp(subtoken, "-c") == 0){
              clearHistory();
              continue;
          }
          // if the user enters a number after history, print the history offset
          else{
              char* offsetCmd = printHistoryOffset(atoi(subtoken));
              executeCommand(offsetCmd, saveptr1, delimiter, token);
              continue;
          }
          // if the user enters a command that is not in the history, print error message
            printf("Invalid history command\n");
        }else{
          // Shell should execute the commands
          executeCommand(input_token, saveptr1, delimiter, token);
        }
      }
    }
  }
  // free the memory
  if(argv_ptr != NULL){
    free(argv_ptr);
    argv_ptr = NULL;
  }
  // exit the shell
  return 0;
}
