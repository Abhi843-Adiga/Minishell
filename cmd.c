#include "mini.h"

char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help","jobs","fg","bg",NULL};

extern char *ext[160];  
extern int status,p;
extern slist *head;

char *get_command(char *input_string)
{
    int c=0;
    char *str=malloc(strlen(input_string)+1); 
    while((input_string[c])!=' ' && (input_string[c])!='\n' && input_string[c] != '\0') 
        c++;

    strncpy(str,input_string,c);
    str[c]='\0';
    return str;

}

int check_command_type(char *command)
{
    int i=0;
    while(builtins[i]!=NULL) 
    {
        if(strcmp(command,builtins[i++])==0)
            return BUILTIN;
    }
    
    i=0;
    while(ext[i]!=NULL)
    {
        if(strcmp(command,ext[i++])==0)
            return EXTERNAL;
    }

    return NO_COMMAND;
}

void extract_external_commands(char **external_commands)
{
    int fd=open("ext.txt",O_RDONLY);

    if(fd==-1)
    {
        perror("Open fail");
        return;
    }

    int size=5,r=0,c=0;char ch;

    char *str=malloc(size);

    if(str==NULL)
    {
        perror("Malloc failed");
        free(str);
        close(fd);
        return;
    }

    while(read(fd,&ch,1)>0)
    {
        if(ch!='\n')
        {
            str[c++]=ch;

            if(c>=size)
            {
                size+=5;
                char *temp=realloc(str,size);

                if(temp==NULL)
                {
                    perror("Realloc failed");
                    free(str);
                    close(fd);
                    return;
                }
                str=temp; 
            }
        }
        else if(ch=='\n')
        {
            str[c]='\0';

            external_commands[r]=malloc(c+1);

            if(external_commands[r]==NULL)
            {
                perror("Mealloc failed");
                close(fd);
                free(str);
                return;
            }

            strcpy(external_commands[r],str);

            r++;
            c=0;
        } 
    }

    if(c>0) //Last external command
    {
        str[c]='\0';

        external_commands[r]=malloc(c+1);

        if(external_commands[r]==NULL)
        {
            perror("Realloc failed");
            close(fd);
            free(str);
            return;
        }

        strcpy(external_commands[r],str);

        r++;
        c=0;
    }
    free(str);
    close(fd);

}

void execute_internal_commands(char *input_string)
{
    if(strcmp(input_string,"exit")==0)
        exit(0);
    
    else if(strncmp(input_string,"cd",2)==0)
    {
        if(chdir(input_string+3)==-1)
             printf("No such file or directory\n");
    }
    
    else if(strncmp(input_string,"pwd",3)==0)
    {
        char buf[250];
        getcwd(buf,250);
        printf("%s\n",buf);
    }
    
    else if(strcmp(input_string,"echo $$")==0)
        printf("%d\n",getpid());
    
    else if(strcmp(input_string,"echo $SHELL")==0)
        printf("%s\n",getenv("SHELL"));
    
    else if(strcmp(input_string,"echo $?")==0)
        printf("%d\n",status);
    
    else if(strcmp(input_string,"jobs")==0)
        print(JOBS);
    
    else if(strcmp(input_string,"fg")==0)
    {
        if(head!=NULL)
        {
            p=head->pid; //Notify parent tht child is running
            kill(p,SIGCONT);  //Continue stopped process
            print(FG);  //Print the job which resumed
            delete_first(); //Delet the entry
            int st;
            waitpid(p,&st,WUNTRACED);
            p=0; //Child done 
        }
    }
    else if(strcmp(input_string,"bg")==0)
    {
        if(head!=NULL)
        {
            p=head->pid; 
            kill(p,SIGCONT);
            print(BG);
            delete_first(); 
            signal(SIGCHLD,signal_handler); //Register sigchild in such a way tht it terminates automaticlaly ,while running in background
            p=0; 
        }
    }
}

char **twod(char *oned,int *rv)
{
    int r=5,c=5;
    char **arr=malloc(sizeof(char*)*(r+1));
    for(int i=0;i<r;i++)
        arr[i]=malloc(sizeof(char)*c);

    char *delim=" ";
    int ri=0;
    
    char *token=strtok(oned,delim);

    while(token!=NULL)
    {
        //grow particular row's coloumn, if token is longer
        if(strlen(token)+1>c) 
            arr[ri]=realloc(arr[ri],strlen(token)+1);
        
        strcpy(arr[ri++],token);

        if(ri>=r)
        {
            //grow rows, if the slots are more
            int or=r;
            r+=5;
            arr=realloc(arr,sizeof(char*)*(r+1));

            for(int i=or;i<r;i++)
                arr[i]=malloc(sizeof(char)*c);
        }

        token=strtok(NULL,delim);
    }
    arr[ri]=NULL; // NULL terminate for execvp
    *rv=ri;
    return arr;
}

void npipe(char **arr,int *np,int n)
{
    pid_t pid[n+1];
    int fd[2],prev=-1;  //prev holds read end of previous pipe,-1 for first command
    for(int i=0;i<=n;i++)
    {
        if(i<n)
            pipe(fd); //create pipe for all except last command

        pid_t pi=fork();

        if(pi==0)
        {
            // connect read end of previous pipe to stdin
            if(prev!=-1)
            {
                dup2(prev,0);
                close(prev);
            }
            //connect write end of current pipe to stdout
            if(i<n)
            {
                dup2(fd[1],1);
                close(fd[1]);
                close(fd[0]);
            }
            if(i==0)
                execvp(arr[i],arr);
            else
                execvp(arr[np[i-1]+1],arr+np[i-1]+1);
            perror("Execvp failed");
            exit(1);
        }

        pid[i]=pi;
        if(prev!=-1)
            close(prev);
        if(i<n)
        {
            close(fd[1]);
            prev=fd[0]; // save read end for next iteration
        }
            
    }

    for(int i=0;i<=n;i++)
    waitpid(pid[i],NULL,0);
}

void execute_external_commands(char *input_string)
{
    int rv;
    char **arr=twod(input_string,&rv);

    int in=5;
    int *np=malloc(in*sizeof(int));  //stores pipe indices 
    int cnt=0;

    for(int i=0;i<rv;i++)
    {
        if(strcmp(arr[i],"|")==0)
        {
            if(cnt>=in)
            {
                in+=5;
                np=realloc(np,in*sizeof(int));
            }
            np[cnt++]=i;
            arr[i]=NULL;
        }
    }

    if(cnt==0)
    {
        execvp(arr[0],arr);
        perror("Execvp failed");
        return;
    }
    else
        npipe(arr,np,cnt);
    
    for(int i=0;i<rv;i++)
        free(arr[i]);
    free(arr);
    free(np);

}



