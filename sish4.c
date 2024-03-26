#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define MAX_CMD_ARGUMENTS 1024
#define MAX_CMD_TOKENS 100
#define MAX_HISTORY 100 
#define PROMPT "sish> "
#define BUFFER_SIZE 1024
#define MAX_ARGS 64

char history[MAX_HISTORY][MAX_CMD_ARGUMENTS];
int history_count = 0;
int front = 0; 
int rear = MAX_HISTORY - 1;

void addtoHistory(const char *cmd){
  char trimmed_cmd[MAX_CMD_ARGUMENTS];
  strncpy(trimmed_cmd, cmd, MAX_CMD_ARGUMENTS);
  trimmed_cmd[strcspn(trimmed_cmd, "\n")] = '\0';
  if(history_count >= MAX_HISTORY){
    strcpy(history[rear], trimmed_cmd);
    rear = (rear + 1) % MAX_HISTORY;
    front = (front + 1) % MAX_HISTORY;
    history_count++;
  }else{
    strcpy(history[history_count], trimmed_cmd);
    history_count++;
  }
}
void printHistory(){
  int i = front;
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


void clearHistory(){
  for(int i = 0; i < MAX_HISTORY; i++){
    memset(history[i], 0, MAX_CMD_ARGUMENTS);
  }
  history_count = 0;
}

char* printHistoryOffset(int offset){
  int i = front;
  if(history_count >= MAX_HISTORY){
    return history[i + offset % MAX_HISTORY];
  }else{
    return history[offset % history_count];
  }
}

void parse(char *line, char **argv){
  while(*line != '\0'){
    while (*line == ' ' || *line == '\t' || *line == '\n'){
      *line++ = '\0';
    }
  if(*line == '\0'){
    break;
  }else{
      *argv++ = line;
      while(*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n'){
        line++;
      }
    }
  }
}

void executePipedCommand(char **commands[]){
  int pipefd[MAX_CMD_TOKENS - 1][2];
  pid_t pid;
  int status;
  int numCmds = 0;
  int i;

  while(commands[numCmds] != NULL){
    numCmds++;
  }
  for(i = 0; i < numCmds - 1; i++){
    if(pipe(pipefd[i]) == -1){
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }
  for(i = 0; i < numCmds; i++){
    pid = fork();
    if(pid == -1){
      perror("fork");
      exit(EXIT_FAILURE);
    } else if(pid == 0){
      if(i != 0){
        dup2(pipefd[i - 1][0], STDIN_FILENO);
      }
      if(i != numCmds - 1){
        dup2(pipefd[i][1], STDOUT_FILENO);
      }
      for(int j = 0; j < numCmds - 1; j++){
          close(pipefd[j][0]);
          close(pipefd[j][1]);
      }
        execvp(commands[i][0], commands[i]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
  }

  for(i = 0; i < numCmds - 1; i++){
    close(pipefd[i][0]);
    close(pipefd[i][1]);
  }

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
        while ((token = strtok_r(NULL, delimiter, &saveptr1)) != NULL) {
            args[i++] = token;
            }
        args[i] = NULL;
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(NULL); // Wait for the child process to finish
    }
}

int checkPipe(char *line){
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

    if(checkPipe(line) == 1){
      if(strcmp(line, "exit") == 0){
        exit(0);
      }

      char *lineCopy = strdup(line);
      if(lineCopy == NULL){
        perror("strdup");
        exit(EXIT_FAILURE);
      }

      char *token = strtok(lineCopy, "|");
      while(token != NULL && i < MAX_CMD_TOKENS - 1){
        argv_ptr = (char **)malloc(MAX_ARGS * sizeof(char *));
        if(argv_ptr == NULL){
          perror("malloc");
          exit(EXIT_FAILURE);
        }
        parse(token, argv_ptr);
        commands[i] = argv_ptr;
        token = strtok(NULL, "|");
        i++;
      }
        commands[i] = NULL;


        executePipedCommand(commands);
        free(lineCopy);


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
        if(strcmp(input_token, "history") == 0){
          char *subtoken = strtok_r(NULL, delimiter, &saveptr1);
          if(subtoken == NULL){
              printHistory();
              continue;
          }
          else if(strcmp(subtoken, "-c") == 0){
              clearHistory();
              continue;
          }else{
              char* offsetCmd = printHistoryOffset(atoi(subtoken));
              if(checkPipe(offsetCmd) == 1){
                parse(offsetCmd, argv_ptr);
                commands[0] = argv_ptr;
                executePipedCommand(commands);
                continue;
              }else{
                executeCommand(offsetCmd, saveptr1, delimiter, token);
                continue;
              }
            }
            printf("Invalid history command\n");
        }else{
          // Shell should execute the commands
          executeCommand(input_token, saveptr1, delimiter, token);
        }
      }
    }
  }

  for(int k = 0; k < i; k++){
    if(commands[k] != NULL){
      free(commands[k]);
      commands[k] = NULL;
    }
  }

  if(argv_ptr != NULL){
    free(argv_ptr);
    argv_ptr = NULL;
  }
  return 0;
}

