#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>
#include<io.h>

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILSESIZE  2048

#define REGULAR 1
#define SPECIAL 2

#define START 0 //for lseek 
#define CURRENT 1
#define END 2

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
}SUPERBLOCK , *PSUPERBLOCK;

typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferanceCount;
    int permission; // 1 23
    struct inode *next; // self referencial structure
} INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int Count;
    int mode; // 1 2 3 
    PINODE ptrinode;
}FILETABLE, *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;

void man(char *name)
{
    if(name == NULL)return;

    if(strcmp(name,"create")== 0)
    {
        printf("Description : used to create new regular file\n");
        printf("Usage : create file name permission\n");
    }
    else if(strcmp(name,"read")== 0)
    {
        printf("Description : used to read new regular file\n");
        printf("Usage : read file_name No_of_bytes_to_read\n");
    }
    else if(strcmp(name,"write")== 0)
    {
        printf("Description : used to write new regular file\n");
        printf("Usage : write file_name\n After this enter the data that you want to write\n");
    }
    else if(strcmp(name,"ls")== 0)
    {
        printf("Description : used to list all information file\n");   
        printf("Usage : ls\n");
    }
    else if(strcmp(name,"stat")== 0)
    {
        printf("Description : used to display information file\n");   
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name,"fstat")== 0)
    {
        printf("Description : used to all information file\n");   
        printf("Usage : fstat File_Descripted\n");
    }
    else if(strcmp(name,"truncate")== 0)
    {
        printf("Description : used to remove data from file\n");   
        printf("Usage : truncate File_name\n");
    }
    else if(strcmp(name,"open")== 0)
    {
        printf("Description : used to open existing file\n");   
        printf("Usage : open File_name mode\n");
    }
    else if(strcmp(name,"close")== 0)
    {
        printf("Description : used to closed opened file\n");   
        printf("Usage : close File_name\n");
    }
    else if(strcmp(name,"closeall")== 0)
    {
        printf("Description : used to closed all opened file\n");   
        printf("Usage : closeall");
    }
    else if(strcmp(name,"lseek")== 0)
    {
        printf("Description : used to change the offset\n");   
        printf("Usage : Iseek File_Name ChangeInOffset StartPoint \n");
    }
    else if(strcmp(name,"rm")== 0)
    {
        printf("Description : used to delete file\n");   
        printf("Usage : rm File_name\n");
    }
    else
    {
        printf("ERROR : nO Manual entry available\n");   
    }
}

void DisplayHelp()
{
    printf("ls : To list out all files\n");
    printf("clear : To clear console\n");
    printf("open : To open the file\n");
    printf("close : To close the file\n");
    printf("closeall : To close all the file\n");
    printf("read : To Read the content from file\n");
    printf("write : To write the content into file\n");
    printf("exit : To Terminate the file system\n");
    printf("stat : To Display the information of file using name\n");
    printf("fstat : To Display the information of file using descripter\n");
    printf("truncate : To remove all the data from file\n");
    printf("rm : To delete the file\n");
}

int GetFDFromName(char *name)
{
    int i = 0;

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
            break;
        i++;
    }

    if(i == 50) return-1;
    else return i;
}

PINODE Get_Inode(char * name)
{
    PINODE temp = head;
    int i =0;

    if(name == NULL)
    return NULL;

    while(temp != NULL)
    {
        if(strcmp(name,temp->FileName)== 0)
        break;
        temp = temp->next;
    }
    return temp;
}

void CreateDILB()
{
    int i =1;
    PINODE newn = NULL;
    PINODE temp = head;

    while(i<= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount =0;
        newn->ReferanceCount =0;
        newn->FileType =0;
        newn->FileSize =0;

            newn->Buffer = NULL;
            newn->next = NULL;

        newn->InodeNumber = i;

        if(temp==NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB created Successfully \n");
}

void InitialiseSuperBlock()
{
    int i =0;
    while(i< MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

int CreateFile(char *name, int permission)
{
    int i =0;
    PINODE temp = head;

    if((name == NULL) || (permission == 0)||(permission > 3))
    return -1;

    if(SUPERBLOCKobj.FreeInode == 0)
    return -2;
    
    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name)!= NULL)
    return -3;

    while(temp != NULL)
    {
        if(temp ->FileType == 0)
        break;
        temp = temp->next;
    }

    while (i<50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->Count =1;
    UFDTArr[i].ptrfiletable->mode =permission;
    UFDTArr[i].ptrfiletable->readoffset =0;
    UFDTArr[i].ptrfiletable->writeoffset =0;

    UFDTArr[i].ptrfiletable->ptrinode =temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType= REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount=1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount =1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILSESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize =0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILSESIZE);

    return i;
}

//rm_file("Demo.txt")
int rm_file(char * name)
{
    int fd =0;

    fd = GetFDFromName(name);
    if(fd == -1)
    return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        // free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;
}

int ReadFile(int fd, char *arr, int isize)
{
    int read_size = 0;
    
    if(UFDTArr[fd].ptrfiletable == NULL);    return-1;

    if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ+WRITE)
    return -2;

    if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
    return -3;

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType!= REGULAR) return -4;

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);//A
    if(read_size < isize)
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),read_size);

        UFDTArr[fd].ptrfiletable->readoffset= UFDTArr[fd].ptrfiletable->readoffset+read_size;
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);

        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset)+isize;
    }

    return isize;
}

int WriteFile(int fd, char *arr, int isize)
{
    if(((UFDTArr[fd].ptrfiletable->mode)!= WRITE)&&((UFDTArr[fd].ptrfiletable->mode)!= READ+WRITE))return -1;

    if(((UFDTArr[fd].ptrfiletable->ptrinode->permission)!= WRITE)&&((UFDTArr[fd].ptrfiletable->ptrinode->permission)!= READ+WRITE))return -1;

    if((UFDTArr[fd].ptrfiletable->writeoffset)== MAXFILSESIZE) return -2;

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!= REGULAR) return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+ (UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize;

    return isize;
}

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
    return -1;

    temp = Get_Inode(name);
    if(temp == NULL)
    return -2;

    if(temp->permission < mode)
    return -3;

    while (i<50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL) return -1;
    UFDTArr[i].ptrfiletable->Count =1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount)++;
    
    return -1;
}

void closeFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset =0;
    UFDTArr[fd].ptrfiletable->writeoffset =0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferanceCount)--;
}

int CloseFileByName(char *name)
{
    int i =0;
    i = GetFDFromName(name);
    if(i == -1)
    return -1;
    
    UFDTArr[i].ptrfiletable->readoffset =0;
    UFDTArr[i].ptrfiletable->writeoffset =0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount)--;

    return 0;
}

void CloseAllFile()
{
    int i =0;
    while (i<50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset =0;
            UFDTArr[i].ptrfiletable->writeoffset =0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferanceCount)--;
            break;
        }
        i++;
    }
}

int LseekFile(int fd, int size, int from)
{
    if((fd<0)||(from > 2)) return -1;
    if(UFDTArr[fd].ptrfiletable == NULL) return -1;

    if((UFDTArr[fd].ptrfiletable->mode == READ)|| (UFDTArr[fd].ptrfiletable->mode == READ+WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset)+ size)> UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)return -1;
            if(((UFDTArr[fd].ptrfiletable->readoffset)+ size)<0)return -1;
            (UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->readoffset)+size;
        }
        else if(from == START)
        {
            if(size> (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)) return -1;
            if(size < 0) return -1;
            (UFDTArr[fd].ptrfiletable->readoffset)= size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size> MAXFILSESIZE) return -1;
            if(((UFDTArr[fd].ptrfiletable->readoffset)+ size) < 0) return -1;
            (UFDTArr[fd].ptrfiletable->readoffset)= (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size;
        }
    }
    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset)+ size)> MAXFILSESIZE)return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset)+ size)< 0)return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset)+ size)> (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize));
            (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->writeoffset) + size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+ size;
        }
        else if(from == START)
        {
            if(size > MAXFILSESIZE) return -1;
            if(size < 0) return -1;
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)= size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = size;
        }
        else if (from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size > MAXFILSESIZE) return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)< 0) return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size;
        }
    }
}

void ls_file()
{
    int i =0;
    PINODE temp = head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error : There are no files\n");
        return;
    }

    printf("\nFile Name\tInodw number \t file Size \t Link Count\n");
    printf("-----------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp ->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("---------------------------------------------------------------\n");
}

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if(fd< 0) return -1;

    if(UFDTArr[fd].ptrfiletable == NULL) return -2;

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n----------Statical information about file----------\n");
    printf("File Name : %s\n", temp->FileName);
    printf("Inode Number %d\n", temp->InodeNumber);
    printf("File size : %d\n",temp->FileSize);
    printf("Actual file size: %d\n",temp->FileActualSize);
    printf("Link Count : %d\n",temp->LinkCount);
    printf("Referance count : %d\n",temp->ReferanceCount);

    if(temp ->permission == 1)
        printf("File Permission : Read Only\n");
    else if(temp ->permission == 2)
        printf("File Permission : write Only\n");
    else if(temp ->permission == 3)
        printf("File Permission : Read & write\n");
        printf("--------------------------------------------\n\n");

    return 0;
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL) return -1;

    while (temp != NULL)
    {
        if(strcmp(name,temp->FileName)== 0)
        break;
        temp = temp->next;
    }

    if (temp == NULL) return -2;

        printf("\n----------Statical information about file---------\n");
        printf("File Name : %s\n", temp->FileName); 
        printf("Inode Number %d\n", temp->InodeNumber);
        printf("File size : %d\n",temp->FileName);
        printf("Actual file size: %d\n",temp->FileActualSize);
        printf("Link Count : %d\n",temp->LinkCount);
        printf("Referance count : %d\n",temp->ReferanceCount);

        if(temp ->permission == 1)
            printf("File Permission : Read Only\n");
        else if(temp ->permission == 2)
        printf("File Permission : write Only\n");
        else if(temp ->permission == 3)
            printf("File Permission : Read & write\n");
        printf("--------------------------------------------\n\n");

        return 0;
}

int truncate_File(char *name)
{
    int fd = GetFDFromName(name);
    if(fd == -1)
    return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
    UFDTArr[fd].ptrfiletable->readoffset =0;
    UFDTArr[fd].ptrfiletable->writeoffset =0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

int main()
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80],str[80],arr[1024];

    InitialiseSuperBlock();
    CreateDILB();

    while (1)
    {
        fflush(stdin);
        strcpy(str,"");

        printf("\n Marvellous VFS : >");

        fgets(str,80,stdin);

        count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

        if(count == 1)
        {
            if(strcmp(command[0],"ls")== 0)
            {
                ls_file();
            }
            else if(strcmp(command[0],"closeall")== 0)
            {
                CloseAllFile();
                printf("All filse closed Successfully\n");
                continue;
            }
            else if(strcmp(command[0],"clear")== 0)
            {
                system("cls");
                continue;
            }
            else if(strcmp(command[0],"help")== 0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0],"exit")== 0)
            {
                printf("Terminate the virtual filse system\n");
                break;
            }
            else
            {
                printf("\nCommand Not Found !!!\n");
                continue;
            }
        }
        else if(count == 2)
        {
            if(strcmp(command[0],"stat") == 0)
            {
                ret = stat_file(command[1]);
                if(ret == -1)
                printf("ERROR : Incorrect Parameters\n");
                if(ret == -2)
                printf("ERROR : There is no such file\n");
                continue;
            }
            else if(strcmp(command[0],"fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));
                if(ret == -1)
                printf("ERROR : Incorrect Parameters\n");
                if(ret == -2)
                printf("ERROR : There is no such file\n");
                continue;
            }
            else if(strcmp(command[0],"close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if(ret == -1)
                printf("ERROR : There is no such file\n");
                continue;
            }
            else if(strcmp(command[0], "rm") == 0)
            {
                ret = rm_file(command[1]);
                if( ret == -1)
                printf("ERROR : There is no such file\n");
                continue;
            }
            else if(strcmp(command[0],"man") == 0)
            {
                man(command[1]);
            }
            else if (strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromName(command[1]);
                if(fd == -1)
                {
                    printf("ERROR : Incorrect Parameter \n");
                    continue;
                }
                printf("Enter the data : \n ");
                scanf("%[^\n]",arr);

                ret = strlen(arr);
                if(ret == 0)
                {
                    printf ("ERROR : Incorrect parameter \n");
                    continue;
                }
                ret = WriteFile(fd, arr, ret);
                if (ret == -1)
                printf("ERROR : permission Deined\n");
                if (ret == -2)
                printf("ERROR : there is no sufficient memory to write\n");
                if (ret == -3)
                printf("ERROR : it is  ot regular file\n");        
            }
            else if (strcmp (command[0], "truncate") == 0)
            {
                ret = truncate_File(command[1]);
                if (ret == -1)
                printf("ERROR : INcorrect parameter\n");
            }
            else
            {
                printf("\n ERROR : Command not found !!! \n");
                continue;
            }
        }
        else if(count == 3)
        {
            if(strcmp(command [0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2]));
                if(ret > 0)
                printf("File is successful created with discriptior : %d\n", ret);
                if(ret == -1)
                printf("ERROR : Incorre t parameter\n");
                if(ret == -2)
                printf("ERROR : There is no Inodes\n");
                if(ret == -3)
                printf("ERROR : Files is ready to Exist\n");
                if(ret == -4)
                printf("ERROR : Memory allocation failuer\n");
                continue;
            }
            else if (strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1],atoi(command[2]));
                if(ret >= 0)
                printf("File is successful created with discriptior : %d\n", ret);
                if(ret == -1)
                printf("ERROR : Incorret parameter\n");
                if(ret == -2)
                printf("ERROR : File Not present\n");
                if(ret == -3)
                printf("ERROR : Permission Denied\n");
                continue;
            }
            else if(strcmp(command[0],"read") == 0)
            {
                fd = GetFDFromName(command[1]);
                if(fd == -1)
                {
                    printf("ERROR : Incorrect Parameter\n");
                    continue;
                }
                ptr = (char*)malloc(sizeof(atoi(command[2]))+1);
                if(ptr == NULL)
                {
                    printf("ERROR : Memory Allocation Failuer\n");
                    continue;
                }
                ret = ReadFile(fd, ptr, atoi(command[2]));
                if(ret == -1) 
                printf("ERROR: File is not exist\n");
                if(ret == -2)
                printf("ERROR: permission Deined\n");
                if(ret == -3)
                printf("ERROR: Reached At End Of Files\n");
                if(ret == -4)
                printf("ERROR: It is not regular file\n");
                if(ret > 0)
                {
                    write(2, ptr,ret);
                }
                continue;
            }
            else
            {
                printf("\n ERROR : Command Not Found !!!\n");
                continue;
            }
        }
        else if(count == 4)
        {
            if(strcmp(command[0],"lseek")==0)
            {
                fd = GetFDFromName(command[1]);
                continue;
                if(fd == -1)
                {
                    printf("ERROR : Incorrect parameter\n");
                }
                ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));
                if(ret == -1)
                {
                    printf("ERROR : unable to perform lseek\n");
                }
            }
            else
            {
                printf("\nERROR : Command Not Found !!!\n");
                continue;
            }
        }
        else
        {
            printf("\n ERROR : command Not Found!!!\n");
            continue;
        }
    }
    return 0;
}