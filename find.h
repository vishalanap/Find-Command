//structure for a node of linked list used to emulate find command
typedef struct node{
    int Ls;
    int Help;
    int Display;
    char File_Type;
    char *Name;
    char *size;
    struct node *next;
}Foptions;

typedef Foptions* Fptr;

void Help(void);
void Find_Path(char **path, Foptions *option);
char FileType(mode_t m);
void Test_File(char *path, Foptions *p, struct stat *a);
void Test_Dir(char *path, Foptions *p, struct stat a);
void Free_List(Fptr *p);
void Display_LS(char* filename, struct stat *a);