#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>                     
#include <dirent.h>                     
#include <sys/stat.h>                   
#include <grp.h>                        
#include <pwd.h>                       
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>                     
#include <fnmatch.h>                                      
#include "find.h"

#define LENGTH 20

//brief introduction to find command
void Help(void){
    FILE * fP;
    char ch;
    fP = fopen("help.txt", "r");
    if(fP == NULL) {
        exit(EXIT_FAILURE);
    }
    while(ch != EOF){
        ch = fgetc(fP);
        putchar(ch);
    } 
    fclose(fP);
    return;
}

/*Initialise the find command functionality
    Path : list of specified search paths/ locations
    option : linked list containing options
*/
void Find_Path(char **path, Foptions *option){  
    struct stat attribute;
    int ret = 0;
    int i = 0;

    //if path is not provided then set it to current working directory
    if(path[i] == NULL){
        path[i] = getcwd(0,0);
        path[i+1] = NULL;
    }
    while(path[i]){
        ret = lstat(path[i], &attribute);
        //if lstat failed
        if(ret == -1)
            fprintf(stderr, "find : (%s): %s\n", path[i], strerror(errno));
        else{
            Test_File(path[i], option, &attribute);

            if(S_ISDIR(attribute.st_mode))
                Test_Dir(path[i], option, attribute);
        }
        i++;
    }
    return;
}

//Displaying data of directory in ls format including modification time, size, user id, group, path, etc
void Display_LS(char* filename, struct stat *a){
    // group database
    struct group *grp;
    //user account
    struct passwd *person;
    char* UserName;
    char* GroupName;
    char file_type[20];

    //S_IFMT used for type of file
    switch(a->st_mode & S_IFMT){
        case S_IFREG:{
            strcpy(file_type, "REGULAR FILE ");
            break;
        }
        case S_IFDIR:{
            strcpy(file_type, "DIRECTORY ");
            break;
        }
        case S_IFBLK:{
            strcpy(file_type, "BLOCK SPECIAL ");
            break;
        }
        case S_IFCHR:{
            strcpy(file_type, "CHARACTER SPECIAL ");
            break;
        }
        case S_IFIFO:{
            strcpy(file_type, "FIFO SPECIAL ");
            break;
        }
        case S_IFLNK:{
            strcpy(file_type, "SYMBOLIC LINK ");
            break;
        }
        case S_IFSOCK:{
            strcpy(file_type, "SOCKET ");
            break;
        }
        default:
            strcpy(file_type,"unknown");
    }

    grp = getgrgid(a->st_gid);
    person = getpwuid(a->st_uid);
    if(person == NULL){
        UserName = (char*)malloc(sizeof(char)*LENGTH);
        snprintf(UserName, LENGTH, "%u", a->st_uid);
    }
    else{
        UserName = person->pw_name;
    }
    if(grp == NULL){
        GroupName = (char*)malloc(sizeof(char) * LENGTH);
        snprintf(GroupName, LENGTH, "%u", a->st_gid);
    }
    else
        GroupName = grp->gr_name;

    //getting last modified date and time
    char *Modtime = ctime(&a->st_mtim.tv_sec) + 4;
    Modtime[strlen(Modtime)-9] = '\0';
    printf("%7lu %18s %8s %8s %8lu  %12s  %s\n",
           a->st_ino, file_type,
           UserName, GroupName, a->st_size,
           Modtime, filename);
    return;
}

//mode of given file is taken from struct stat
/* Checks type of a file
    S_ISREG() - regular file
    S_ISDIR() - directory
    S_ISCHR() - character file
    S_ISBLK() - block file
    S_ISFIFO() - FIFO
    S_ISLINK() - symbolic link
    S_ISSOCK() - socket */
char FileType(mode_t m){
    if(S_ISBLK(m))
        return 'b';
    if (S_ISCHR(m))
        return 'c';
    if (S_ISDIR(m))
        return 'd';
    if (S_ISFIFO(m))
        return 'p';
    if (S_ISREG(m))
        return 'f';
    if (S_ISLNK(m))
        return 'l';
    if (S_ISSOCK(m))
        return 's';
    return '\0';
}

//Checking all options against the given filepath and if they matched then simply return the path
void Test_File(char *path, Foptions *p, struct stat *a){
    int flag = 0;
    do{
        if(p->File_Type){
            if(FileType(a->st_mode) != p->File_Type)
                return;
        }
        if (p->size && p->size_flag == 'l') {
            int b = atoi(p->size);
            if (a->st_size >= b)
                return;
        }
        if (p->size && p->size_flag == 'e') {
            int b = atoi(p->size);
            if (a->st_size > b || a->st_size < b)
                return;
        }
        if (p->size && p->size_flag == 'g') {
            int b = atoi(p->size);
            if (a->st_size <= b)
                return;
        }
        if(p->Name){
            char* file = basename(path);//basename() returns the last string of path
            if(fnmatch(p->Name, file, 0) != 0)
                return;
        }
        if(p->user){
            if(p->user_id != a->st_uid)
                return;
        }
        if(p->Display){
            printf("%s\n",path);
            flag = 1;
        }
        if(p->Ls){
            Display_LS(path,a);
            flag = 1;
        }
        p = p->next;
    }while(p != NULL);

    if(flag == 0)//flag not set
        printf("%s\n",path);
    return;
}

//Recursive traversal of directory to gather the information of directory and process it
void Test_Dir(char *path, Foptions *p, struct stat a){
    DIR *directory;
    struct dirent* entry;
    int value = 0;
    errno = 0;
    directory = opendir(path);
    if(directory == NULL){
        fprintf(stderr, "find : %s %s\n", path, strerror(errno));
        return;
    }
    errno = 0;
    while((entry = readdir(directory)) != NULL){
        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)
            continue;
        unsigned long act_path = strlen(path)+strlen(entry->d_name);
        char Path[act_path];
        if(path[strlen(path)-1] == '/')
            sprintf(Path, "%s%s", path, entry->d_name);
        else
            sprintf(Path, "%s/%s", path, entry->d_name);
        value = lstat(Path, &a);
        if(value == -1){
            fprintf(stderr,"find : (%s): %s\n", Path, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(S_ISDIR(a.st_mode)){
            Test_File(Path, p, &a);
            Test_Dir(Path, p, a);
        }   
        else
            Test_File(Path, p, &a);
    }
    if(closedir(directory) == -1){
        fprintf(stderr, "find : %s %s \n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }    
    return;
}

//free the linked list of options
void Free_List(Fptr *p) {
    Foptions *start = *p;
    while(start) {
        Foptions *temp = start->next;
        free(start);
        start = temp;
    }   
    return;
}