#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
 /*
  for use when using '>' to write command to file
  open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
  */

int outputfile;
char *outputfile_name;
size_t byteSize = 512;
int number_of_args = 0;
    //space for the tokenized string
   char **tokens ;
static char* args[512];
char error_message[30] = "An error has occurred\n";
char delimit[]=" \t\r\n\v\f" ;
int background_job = 0;
int batch_mode = 0; //0 for false 1 for true

void line_to_arguments(char * line, char ** toks ){

    char * p = strtok(line, delimit);

    char * p_copy = p;
    int i = 0;
    //allocate some memory for each token array element
    toks[i] = malloc(512 * sizeof(char));
   
    //flag of whether p has a special character in it or not 0 for no special char, 1 otherwise
    int no_special_char=1;

    while(p_copy){
//    printf("p is %s  tok[i] is %s\n",p,toks[i]);

        //loop through the string and store each token separately
      while(strlen(p_copy)!=0 ) {
        //if a special char is found, store it separetely as it is a token on its own.Also add terminating '\0'
        if(p_copy[0]=='&' || p_copy[0]=='>'){
          no_special_char = 0;
          //if special character is the first characer of the new argument token,
          //concat the special character to toks[i] and prepare the next toks[i+1]
          if(strlen(toks[i])==0){
            //check whether the token has two consecutive ampsersand or a single ampersand and lex them accordingly
            if(p_copy[0]=='&' && p_copy[1]=='&'){
            strcpy(toks[i],"&&");
            p_copy++;//pointer is being moved 2 places(increment once here and once at the end)
            }
            else{
            strncat(toks[i],&p_copy[0],1);//concat just one special char
            }

            strncat(toks[i], "",1);//add null terminator 
            toks[++i] = malloc(512 * sizeof(char));
          }
          //p_copy[0] is a special char but it's not the first char in the string
          else{
            strcat(toks[i++], "");
            toks[i] = malloc(512 * sizeof(char));
            //check whether the token has two consecutive ampsersand  or a single ampersand and lex them accordingly
            if(p_copy[0]=='&' && p_copy[1]=='&'){
            strcat(toks[i],"&&");
             p_copy++;//pointer is being moved 2 places(increment once here and once at the end)

            }
            else{
            strncat(toks[i],&p_copy[0],1);
            }
            strncat(toks[i++], "",1);
            toks[i] = malloc(512 * sizeof(char));
          }
          p_copy++;//pointer p points to the next char in the string
        }

        //if it's just a normal char, add it
        else {
          no_special_char= 1;
          strncat(toks[i], &p_copy[0],1);
          p_copy++;
        }
       }

      //if the last char processed was not a special char, add '\0'
      if(no_special_char==1){
        strcat(toks[i++], "");
      }

      p = strtok(NULL, delimit);
      p_copy = p;
      if(p){
        toks[i] = malloc(512 * sizeof(char));
      }
    }
    free( toks[i]);

    toks[i] = NULL;



}

int process_command(char *f){
 
//printf("background_job1: %d \n",background_job);
  /* setting up pipeline process with file descriptor (fd) */
  int fd[2]; // fd[0] is for reading, fd[1] is for writing
  pid_t child_pid;
  pipe(fd); // syscall to add two new file descriptors to be used for the pipeline
  if ((child_pid = fork()) < 0){
    perror("fork");
    exit(1);
  } 
//      printf("pid: %d\n", getpid());

 if (child_pid == 0) { // child process
  //  printf("in child: %d\n", getpid());
    dup2(0, fd[0]); // child closes input side and duplicated input descriptor onto stdin
  //  close(fd[1]);//close unneeded pipe
    if (outputfile > 0) {
      close(STDOUT_FILENO);
      open(f, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
      args[outputfile] = NULL;
    }
    if (execvp( args[0], args) == -1)
      write(STDERR_FILENO, error_message, strlen(error_message));
  }
  else { // parent process
    dup2(1, fd[1]); // parent closes output side and duplicated output descriptor onto stdout
    //close(fd[0]);//close unneeded pipe
    //nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    int status;
   if(background_job == 0)  status = waitpid(child_pid,&status,WUNTRACED);//only wait if you don't want to run background jobs

   //printf("in parent: %d status = %d\n", getpid(), status);

//   if(background_job == 0){ 
  //  printf("ga\n");
  // printf("child wait: %d\n",wait(NULL));//only wait if you don't want to run background jobs
 // }

  }
  close(fd[1]);
  close(fd[0]);
  return fd[0];
}

int execute(){
//check if the last argument is '&', set the flag accordingly
  //and discard the last argument
  if(strcmp(args[number_of_args-1],"&") == 0) {
    background_job = 1;
    free(tokens[number_of_args-1]);
    tokens[number_of_args - 1] = NULL;
    args[number_of_args-1] = NULL;
    number_of_args--;
  }
  else background_job = 0;

  //check for the first argument's last three characters, used for .py extension
  char * last_three_chars = (args[0]+(strlen(args[0])-3));
  
  if (strcmp(args[0], "exit") == 0){
      exit(0) ;
    } 
    else if (strcmp(args[0], "cd") == 0){
           if (chdir(args[1]) == -1)
        chdir(getenv("HOME"));
    } 
    else if (strcmp(args[0], "pwd") == 0){
      char* cwd;
      char buff[sizeof(args)];
      if (getcwd( buff, 512 - sizeof(args) ) == NULL )
        write(STDERR_FILENO, error_message, strlen(error_message));
      else
        printf("%s\n", getcwd( buff, 512 - sizeof(args)));
    } 
    else if (strcmp(args[0], "wait") == 0){
      wait(NULL);
    }
    else if(strcmp(last_three_chars,".py")==0){
      //we want to add "python" to args[0], so move each array element down one spot
      args[number_of_args+1]=NULL;//element after last token is NULL
      args[number_of_args]=malloc(512*sizeof(char));//we are adding a new element in the array so malloc
      for(int i = number_of_args; i > 0;i--){
        strcpy(args[i],args[i-1]);
      }
      strcpy(args[0],"python");
      number_of_args++;
      process_command(outputfile_name);   
    }
     else
        return process_command(outputfile_name);
    return 1;
}

int main(int argc, char *argv[]) {
  FILE* file;
  //get line will return the number of chars read or -1 if failed
  int result = 1;
  if(argv[1] != NULL){
   file = fopen(argv[1], "r");//open file read only
  }
  if(argv[1] != NULL && file == NULL){
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  else if(file != NULL)
    batch_mode=1;//set batch mode token to true



  char * buffer = (char *) malloc(byteSize+1);// add one for terminating \n
  while(result != -1){
    
    ///get this block so check for result this is for last error in batchmode!!!!!
    if (batch_mode==1 ) 
      result = getline(&buffer, &byteSize, file);
    else if(background_job == 1){
     result = getline(&buffer, &byteSize, stdin);
    }
    else{
       printf("mysh> ");
      result = getline(&buffer, &byteSize, stdin);
    }
///////////////////////////////////////////////////////////////////////////


//printf("result %d \n", result);
     if(batch_mode==1 && result != -1) {
      write(STDERR_FILENO, "mysh> ", 6);
      write(STDERR_FILENO, buffer, strlen(buffer));
  //    write(STDERR_FILENO, "\n", 1);
  }
    //allocate memory for tokens
      tokens = malloc(512 * sizeof(char*));
    //if sucessfully ran getline() function tokenize the string
    if(result != -1){
//       background_job = 0;
      line_to_arguments(buffer, tokens);
   
  

      int i = 0;
      outputfile = 0;
      while(tokens[i] != NULL){
        args[i] = tokens[i];
        number_of_args++;
        //check for '>' in the args array
        if (strcmp(args[i], ">") == 0) { 
          //check if the argument after '>' is not NULL and one after that is NULL
          if (tokens[i+2] == NULL && tokens[i+1] != NULL) {
            outputfile = i;
            outputfile_name = tokens[i+1];
          } else {
            outputfile = -1;
            write(STDERR_FILENO, error_message, strlen(error_message));
          }
        }
//        printf("I am a token: %s with length: %lu bj= %d\n", tokens[i], strlen(tokens[i]),background_job);
         i++;

      }

      if (outputfile == 0){
            if (number_of_args>0)
               execute();
      }
      else if (outputfile > 0) {
        int pc = process_command(outputfile_name);
      }
    }
    else{
                 // printf("result is %d, buffer is %s\n",result, buffer);
    if(batch_mode != 1)   write(STDERR_FILENO, error_message, strlen(error_message));
    }

    while(number_of_args!=0){
      free(tokens[number_of_args-1]);
      tokens[number_of_args]=NULL;
      args[number_of_args-1]=NULL;
      number_of_args--;
    }
     /*
   printf(" arg0 = %s\n", args[0]);
      printf(" arg1 = %s\n", args[1]);
      printf("arg2 = %s\n", args[2]);
      printf("argc= %d\n",number_of_args);
      */
  
  }

  free(buffer);

  return 0;
}
