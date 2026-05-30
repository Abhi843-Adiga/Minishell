#include "mini.h"

char *ext[160]={NULL}; //stores external command names loaded from ext.txt
int p,status; // p=current child pid, 0 if none,status=last exit code
extern char prompt[],input_string[]; 
slist *head=NULL; // head of stopped jobs linked list

void scan_input()
{
    int wstatus;
    extract_external_commands(ext); //load external commands from ext.txt into ext[]

    //parent handles Ctrl+C and Ctrl+Z;child will reset to SIG_DFL before execvp
    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);

    while(1)
    {
        printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "$: ",prompt);
        
        // save current prompt in case PS1 validation fails and we need to restore
        char copy[strlen(prompt)+1];
        strcpy(copy,prompt);

        scanf(" %[^\n]",input_string);

        if(strncmp(input_string,"PS1=",4)==0)
        {
            if((*(input_string+4))==' ' || (*(input_string+4))=='\0')
            {
                perror("Enter valid prompt");
                strcpy(prompt,copy); //Restore previous prompt
                continue;
            }
            
            strcpy(prompt,input_string+4); //Update prompt
        }
        else
        {
            char *command=get_command(input_string); //Extract first word
            int ret=check_command_type(command);
            free(command);

            if(ret==BUILTIN)
                execute_internal_commands(input_string);
            
            else if(ret==EXTERNAL)
            {
                p=fork();

                if(p==0)
                {
                    // child resets signals to default before execvp, so Ctrl C/Z act normally on the child process
                    signal(SIGINT,SIG_DFL);
                    signal(SIGTSTP,SIG_DFL);
                    execute_external_commands(input_string);
                    exit(1);
                }
                waitpid(p,&wstatus,WUNTRACED); //Also return if chid stopped
                p=0; //Child done
                if(WIFEXITED(wstatus))
                    status=WEXITSTATUS(wstatus); // save exit code for echo $?
            }
            else
                printf("No command\n");
        }
    }
}

void signal_handler(int sig_num)
{
    if(sig_num==SIGINT)
    {
        if(p==0)
        {
            write(1,"\n",1); //Because printf includes the buffer bypass
            write(1,ANSI_COLOR_GREEN,strlen(ANSI_COLOR_GREEN));
            write(1,prompt,strlen(prompt));
            write(1,ANSI_COLOR_RESET,strlen(ANSI_COLOR_RESET));
            write(1,"$: ",3);
        }
    }
    if(sig_num==SIGTSTP)
    {
        if(p==0)
        {
            write(1,"\n",1); 
            write(1,ANSI_COLOR_GREEN,strlen(ANSI_COLOR_GREEN));
            write(1,prompt,strlen(prompt));
            write(1,ANSI_COLOR_RESET,strlen(ANSI_COLOR_RESET));
            write(1,"$: ",3);
        }
        else
            insert_first(p);

    }
    else if(sig_num==SIGCHLD)
    {
        int st;
        waitpid(-1,&st,WNOHANG); //Don't block the code, return immedietly, used in bg child
    }
}

void insert_first(pid_t cpid)
{
    slist *new=malloc(sizeof(slist));
    new->pid=cpid;
    strcpy(new->cmd,input_string);
    new->link=NULL;
    
    if(head==NULL)
        head=new;
    else
    {
        slist *temp=head;
        head=new;
        new->link=temp;
    }  
}

void print(int mac)
{
    if(head==NULL)
    {
        printf("No such job\n");
        return;
    }

    slist *temp=head;

    switch(mac)
    {
        case JOBS:
            while(temp!=NULL)
            {
                printf("[%d] %s STOPPED\n",temp->pid,temp->cmd);
                temp=temp->link;
            }
            break;
        case FG:
            printf("%s\n",head->cmd);
            break;
        
        case BG:
            printf("[%d] %s &\n",head->pid,head->cmd);
    }   
}

void delete_first()
{
    slist *temp=head;
    head=head->link;
    free(temp);
}

