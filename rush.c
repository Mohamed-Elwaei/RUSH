
#include "rush.h"


int main(int argc, char* argv[]) {
    char PATH[1024];
    strcpy(PATH, getenv("PATH"));
    
    
    if (argc>1){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    int original = getpid();
    while (original == getpid()) {
        printf("rush> ");
        fflush(stdout);
        char* line = NULL;


        getline(&line, &LINE_LENGTH, stdin);
        fflush(stdin);
        int valid = 0;
        for (int i = 0; i <strlen(line); i++) {
            if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t')
                continue;
            else if(line[i] == '&' && !valid)
                break;
            else{
                valid = 1;
                break;
            }
        }
        if (!valid) {
            free(line);
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }
        valid = 0;
        for (int i = strlen(line) - 1; i > 0; i--) {
            if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t')
                continue;
            else if(line[i] == '&' && !valid)
                break;
            else{
                valid = 1;
                break;
            }
        }
        if (!valid) {
            free(line);
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }

        char* tmp = strdup(line);
        

        char* token;
        int cmd_count = 0;

        while ((token = strsep(&line, "&"))) {
            cmd_count++;
        }

        char** command_str = malloc(cmd_count * sizeof(char*));
        int i = 0;
        while ((token = strsep(&tmp, "&"))) {
            if (token != NULL && strlen(token) > 0) {
                command_str[i] = strdup(token);
                i++;
            }

        }
        Command **commands = malloc(sizeof(Command*) * cmd_count);
        
        for (int i = 0; i < cmd_count; i++) {
            char * curr = command_str[i];
            commands[i] = malloc(sizeof(Command));
            commands[i]->valid = VALID;
            commands[i]->words = get_words(curr, &commands[i]->word_count);
            commands[i]->arguments = get_args(commands[i]->words, &commands[i]->arg_count);
            commands[i]->inputFile = get_inputFile(commands[i]->words,&commands[i]->valid);
            commands[i]->outputFile = get_outputFile(commands[i]->words,&commands[i]->valid);
            if (commands[i]->word_count==1 || commands[i]->arg_count==1) //CHeck if path is empty
                commands[i]->valid = EMPTY;
        }
        
        
        
        pid_t* children =(pid_t*)  malloc(cmd_count * sizeof(pid_t));
        
        for (i = 0; i < cmd_count; i++) {
            int in, out;
            Command * curr = commands[i];
            if (curr->valid == INVALID) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
            else if(curr->valid == EMPTY)
                continue;
            else if((strcmp(curr->arguments[0],"exit") == 0)){
                if(curr->arguments[1]){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                }
                else
                    exit(0);
            }
            else if((strcmp(curr->arguments[0],"cd") == 0)){
                if (curr->arguments[1] == NULL) {// No directory to change to
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                else if(curr->arguments[2]){//Too many arguments
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                else{
                    chdir(curr->arguments[1]);
                }
            }
            else if((strcmp(curr->arguments[0], "path") == 0)){
                set_path(curr->arguments, curr->arg_count,PATH);
            }
            else if ((children[i] = fork()) == 0) {
                if (curr->inputFile!=NULL){
                    in = open(curr->inputFile, O_RDONLY,0777);
                    if (in<0) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        continue;
                    }
                    dup2(in,STDIN_FILENO);
                    close(in);
                }
                if (curr->outputFile!=NULL){
                    out = open(curr->outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0777);
                    if (out<0) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        continue;
                    }
                    dup2(out, STDOUT_FILENO);
                    close(out);
                }
                execute(curr->arguments, PATH);
                write(STDERR_FILENO, error_message, strlen(error_message));
            }

        }
        
        int status;
        int time_to_exit = 0;
        for (i = 0; i < cmd_count; i++) {
            waitpid(children[i],&status,0);
            if (WEXITSTATUS(status) == 40)
                time_to_exit = 1;
        }
        
        for (int i=0; i<cmd_count; i++)
            delete_command(commands[i]);
    
        for (i = 0; i< cmd_count; i++){
            free(command_str[i]);
            
        }
        free(command_str);

        free(children);
        if (time_to_exit)  exit(0);
    }


    return 0;
}


/*
 for (int i = 0; arguments[i] != NULL; i++) {
     printf("Argument %d: %s\n", i,arguments[i]);
 }
 if (inputFile==NULL) {
     printf("No input file\n");
 }
 else printf("Input File : %s\n",inputFile);
 if (outputFile==NULL) {
     printf("No output file\n");
 }
 else printf("Output File : %s\n",outputFile);
 */
