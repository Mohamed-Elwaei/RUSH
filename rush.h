//
//  rush.h
//  Project1
//
//  Created by Mohammed Elwaei on 2/3/24.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef rush_h
#define rush_h

#define DELIMITERS " \t\n"
#define min(a,b) ((a) > (b) ? (b) : (a))
size_t LINE_LENGTH = 1024;
char error_message[30] = "An error has occurred\n";
enum cmd_status{
    INVALID,
    VALID,
    EMPTY,
};


typedef struct Command{
    char** words;
    char** arguments;
    char* inputFile, *outputFile;
    int word_count,arg_count;
    enum cmd_status valid;
    
}Command;

char curr_dir[1024];

void delete_command(Command *c){
    for (int i = 0; i < c->word_count; i++)
        free(c->words[i]);
    free(c->words);
    for (int i = 0; i < c->arg_count; i++)
        free(c->arguments[i]);

    free(c);
}


char ** get_words(char* arg, int* word_count){
    char ** words = NULL;
    char * line = arg;
    char * duplicate = strdup(line);
    int count = 0;
    
    char* token;
    while ((token = strsep(&line, DELIMITERS))!=NULL){
        if (strlen(token)>0)
            count++;
    }
    words = malloc(sizeof(char*) * (count + 1));
    int i = 0;
    while ((token = strsep(&duplicate, DELIMITERS))) {
        if (strlen(token))
        {
            words[i] = strdup(token);
            i++;
        }
    }
    words[count] = NULL;
    *word_count = count+1;
    return words;
}

char ** get_args(char** words, int* arg_count){
    char ** arguments = NULL;
    int count = 0;
    for (; words[count] != NULL; count++) {
        if (words[count] && (strcmp(words[count], ">") == 0 || strcmp(words[count], "<") == 0)) {
            break;
        }
    }
    arguments = malloc(sizeof(count + 1) * sizeof(char*));
    for (int i = 0; i < count; i++) {
        arguments[i] = strdup(words[i]);
    }
    arguments[count] = NULL;
    *arg_count = count+1;
    return arguments;
}
char * get_inputFile(char** words,enum cmd_status* valid){
    char * ret = NULL;
    
    for (int i = 0; words[i] !=NULL; i++) {
        if(strcmp(words[i], "<") == 0)
        {
            if(words[i+1] == NULL)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else if (ret)
                *valid = INVALID;
            else
                ret = words[i+1];
        }
    }
    return ret;
}
char * get_outputFile(char** words, enum cmd_status* valid){
    char* ret = NULL;
    for (int i = 0; words[i] !=NULL; i++) {
        
        if(strcmp(words[i], ">") == 0)
        {
            if(words[i+1] == NULL)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else if (ret!=NULL)
                *valid = INVALID;
            else
                ret = words[i+1];
        }
    }
    return ret;
}
void execute(char **arguments, char* path){
    char* duplicate = strdup(path);
    char* token;
    while (( token = strsep(&duplicate, ":"))){
        snprintf(curr_dir, 1024, "%s/%s",token,arguments[0]);
        
        if (access(curr_dir, X_OK)!=-1) {
            execv(curr_dir, arguments);
        }
    }
    
    snprintf(curr_dir, 1024, "%s/%s",getcwd(curr_dir, 1024),arguments[0]);
    
    if (access(curr_dir, X_OK)!=-1) {
        execv(curr_dir, arguments);
    }
}

void set_path(char** arguments, int arg_count,char path[]){
    if (arg_count == 1) {
        strcpy(path, "");
        return ;
    }
    int path_size = 0;
    for (int i = 1; i < arg_count-1; i++) {
        path_size += strlen(arguments[i]) + 1;
    }
    
    strcpy(path, "");
    for (int i = 1; i<arg_count-1; i++) {
        strcat(path, arguments[i]);
        strcat(path, ":");
    }
    return;
}


char* format_line(char* original, char* delimiters){
    size_t start = 0,end = strlen(original);
    for (size_t i = 0; i<strlen(original); i++) {
        int valid = 0;
        for (int k = 0; k<strlen(delimiters); k++)
            if(delimiters[k] == original[i])
                valid = 1;
        if (valid)
            start++;
        else break;
        
    }
    for (size_t i = strlen(original) - 1; i>-1; i--) {
        int valid = 0;
        for (int k = 0; k<strlen(delimiters); k++)
            if(delimiters[k] == original[i])
                valid = 1;
            if (valid)
                end--;
            else break;
        
    }
    char* formatted = malloc(end - (start) + 1);
    strncpy(formatted, original + start, end - (start) + 1);
    formatted[strlen(formatted) - 1] = '\0';
    free(original);
    return formatted;

}

/*
 void command(char* arg) {
     char* token;
     char* line = arg;
     char* tmp = strdup(line);
     int count = 0;
     int exit_command = 0;
     while ((token = strsep(&line, DELIMITERS))) {
         if (strlen(token)) count++;
         if (strcmp(token, "exit") == 0){
             exit_command = 1;
             break;
             }
     }
     
     //Get all the arguments in an array
     char** words = malloc((count + 1) * sizeof(char*));
     words[count] = NULL;
     int i = 0;
     int ins = 0,outs = 0;
     while ((token = strsep(&tmp, DELIMITERS))) {
         if (strlen(token)) {
             if (strcmp(token, ">") == 0)
                 outs++;
             if (strcmp(token, "<") == 0)
                 ins++;
             words[i] = strdup(token);
             i++;
         }
     }
     
     int inputFile,outputFile;
     int arg_count = count;
     for (int i = 0; i < count; i++) {
         switch (*words[i]) {
             case '>':
                 
                 outputFile = open(words[i+1], O_TRUNC|O_CREAT|O_WRONLY,0777);
                 dup2(outputFile,STDOUT_FILENO);
                 close(outputFile);
                 arg_count = min(i,arg_count);
                 break;
                 
             case '<':
                 inputFile = open(words[i+1], O_RDONLY,0777);
                 dup2(inputFile,STDIN_FILENO);
                 close(inputFile);
                 arg_count = min(i,arg_count);
                 break;
                 
             default:
                 break;
         }
     }
     
     char ** arguments = malloc((arg_count+1) * sizeof(char*));
     arguments[i] = NULL;
     for (int i = 0 ; i<arg_count; i++){
         arguments[i] = strdup(words[i]);
     }
     
     
     
     if (fork() == 0) {
         execvp(arguments[0], arguments);
         write(STDERR_FILENO, error_message, strlen(error_message));
         exit(1);
     }

     wait(NULL);
     
     for(int i = 0; i<arg_count; i++)
         free(arguments[i]);
     for (i = 0; i < count; i++) {
         free(words[i]);
     }
     free(words);
     free(tmp);
     if (exit_command == 1) {
         exit(40);
     }
     else exit(0);
 }
 */


#endif /* rush_h */
