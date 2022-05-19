#include <stdio.h>
#include <stdlib.h>
#include "find.c"

/*
FIND COMMAND:
	A simplified implementation of the 'find' utility on Linux systems. 
	This utility is used to locate files on a Unix or Linux system, 
	searching any set of directories that are specified for files 
	that match the supplied search criteria.

Check all given command line arguments and point to correct option
    length : number of command line arguments
    arr: array of pointers to save list of paths
    argv : command line arguments
*/

int main(int argc, char *argv[]){

    char* arr[argc];
    memset(arr, 0, argc);
    
    int index = 0;
    int Memory_flag = 0;
    int Option_flag = 0;
    int length = argc;

    Foptions *op = (Foptions *)malloc(sizeof(Foptions));
    memset(op, 0, sizeof(*op));
    Foptions *pass = op;

    for(int i = 1; i < length; i++){

        if(Option_flag == 1){
            op->next = (Foptions*)calloc(1, sizeof(*op));

            if (op->next == NULL) {
                fprintf(stderr, "find: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            op = op->next;
            op->next = NULL;
        }

        if(strcmp(argv[i], "-name") == 0){
        
            if(argv[++i]){
                op->Name = argv[i];
                Option_flag = 1;
                Memory_flag = 1;
                continue;
            }
            
            else{
                fprintf(stderr,"Missing argument to %s\n",argv[i-1]);
                exit(EXIT_FAILURE);
            }
        }

        else if(strcmp(argv[i],"-help") == 0){
            op->Help = 1;
            Option_flag = 1;
            Memory_flag = 1;
            continue;
        }

        else if(strcmp(argv[i],"-display") == 0){
            op->Display = 1;
            Option_flag = 1;
            Memory_flag = 1;
            continue;
        }

        else if(strcmp(argv[i],"-ls") == 0){
            op->Ls = 1;
            Option_flag = 1;
            Memory_flag = 1;
            continue;
        }

        else if(strcmp(argv[i],"-type") == 0){
        
            if(argv[++i]){
                if(strlen(argv[i]) > 1){
                    fprintf(stderr,"find : Argument to -type can only be single letter\n");
                    exit(EXIT_FAILURE);
                }

                char x = *(argv[i]);
                if(x == 'f' || x== 'b' || x == 'c' || x == 's' || x == 'd' || x == 'l' || x == 'p'){
                    op->File_Type = x;
                    Option_flag = 1;
                    Memory_flag = 1;
                    continue;
                }
                
                else{
                    fprintf(stderr, "find : Unknown Argument\n");
                    exit(EXIT_FAILURE);
                }
            }
            
            else{
                fprintf(stderr, "find : Missing Arguments\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(argv[i],"-user") == 0){
            struct passwd *pd;
            if (argv[++i]){
                op->user = argv[i];
                pd = getpwnam(op->user);

                if(pd != NULL) {
                    op->user_id = pd->pw_uid;
                    Option_flag = 1;
                    Memory_flag = 1;
                    continue;
                } 
                else if(isdigit(argv[i][0])) {
                    sscanf(op->user, "%lu", &op->user_id);
                    Option_flag = 1;
                    Memory_flag = 1;
                    continue;
                }
                else if (pd == NULL) {
                    fprintf(stderr, "find: `%s` is not the name of a known user \n", argv[i]);
                    exit(EXIT_FAILURE);
                }

            }
            else {
                fprintf(stderr, "find: missing arguments");
                exit(EXIT_FAILURE);
            }
        }
	    else if(strcmp(argv[i], "-size") == 0){
            if (argv[++i]) {
                op->size = (argv[i]);

                if (op->size != NULL) {
                    Option_flag = 1;
                    Memory_flag = 1;
                    continue;
                }
                else {
                    fprintf(stderr, "find: Unknown argument\n");
                    exit(EXIT_FAILURE);
                }
            } 
            else {
                fprintf(stderr, "find: missing argument\n");
                exit(EXIT_FAILURE);
            }
        }

        else if(Memory_flag != 0){
            if(argv[i][0] == '-'){
                fprintf(stderr,"Unknown Predicate");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr,"Run Your Program Again\n Please enter -help for viewing how to use program\n");
            exit(EXIT_FAILURE);
        }

        else
            Option_flag = 0;

        //Final path is taken here
        if(!Memory_flag && argv[i][0] != '-'){
            arr[index] = argv[i];
            index++;
        }
        else{
            fprintf(stderr,"Unknown Predicate %s\n",argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    arr[index] = NULL;
    
    if(pass->Help){
        Help();
        Free_List(&pass);
        return 0;
    }
    
    Find_Path(arr, pass);
    Free_List(&pass);
    
    return 0;
}
