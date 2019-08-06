#define _LARGEFILE_SOURCE 
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include "thread.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ARGC 10
#define READ_BUFF 4096
#define RES_BUFF 128
#define CLI_STATUS 64
#define SENDBUFF 1024
#define RECV_BUFF 64

void get_file(int c,char *name)
{
    printf("get_file:%s\n",name);
    if(name == NULL)
    {
        printf("name:%s\n",name);
        send(c,"err",3,0);
        return ;
    }
    int fd=open(name,O_WRONLY|O_CREAT,0600);
    if(fd == -1)
    {
        send(c,"err",3,0);
        printf("fd %d\n",fd);
        return;
    }
    int size_file=lseek(fd,0,SEEK_END);
    printf("size_file:%d\n",size_file);

    char res_buff[RES_BUFF] = {0};
    sprintf(res_buff,"ok#%d",size_file);
    printf("resbuff:%s\n",res_buff);
    send(c,res_buff,strlen(res_buff),0);
    
    char recv_buff[RECV_BUFF] = {0};
    if(recv(c,recv_buff,RECV_BUFF-1,0)<=0)
    {
        close(fd);
        printf("recv err\n");
        return;
    }
    printf("recv_buff:%s\n",recv_buff);
    if(strncmp(recv_buff,"ok",2) != 0)
    {
        close(fd);
        return;
    }
    int size = 0;
    sscanf(recv_buff+2,"%d",&size);
    
    int num = 0;
    char read_buff[READ_BUFF] = {0};
    while((num = recv(c,read_buff,READ_BUFF,0)) > 0)
    {
        write(fd,read_buff,num);
        size_file+=num;
        if(size_file >= size)
        {
            break;
        }
    }
    printf("size:%d,size_file%d\n",size,size_file);
    close(fd);
}
void send_file(int c,char*name)
{
    if(name==NULL)
    {
        send(c,"err",3,0);
        printf("name = NULL");
        return ;
    }
    int fd=open(name,O_RDONLY|O_LARGEFILE);
    if(fd == -1)
    {
        send(c,"err",3,0);
        printf("fd == -1");
        return;
    }
    
    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    char res_buff[RES_BUFF] = {0};
    sprintf(res_buff,"ok#%d",size);

    send(c,res_buff,strlen(res_buff),0);//发送文件总大小
    printf("send res_buff");
    char cli_status[CLI_STATUS] = {0};
    if(recv(c,cli_status,CLI_STATUS,0) <= 0)
    {
        close(fd);
        printf("recv<=0\n");
        return;
    }
    if(strncmp(cli_status,"ok",2)!=0)
    {
        close(fd);
        return;
    }
    
    int size_num=0;
    sscanf(cli_status+2,"%d",&size_num);
    lseek(fd,size_num,SEEK_SET);

    printf("send begin\n"); 
    int num=0;
    char sendbuff[SENDBUFF] = {0};
    while((num = read(fd,sendbuff,1024)) > 0)
    {
        printf("send~\n");
        send(c,sendbuff,num,0);
        printf("send~~\n");
    }
    close(fd);
    return ;
}
void *work_thread(void*arg)
{
    int c = (int)arg;
    while(1)
    {
        char buff[256]={0};
        int n=recv(c,buff,255,0);
        if(n<=0)
        {
            printf("one cli over!!!\n");
            break;
        }
        int i=0;
        char *myargv[ARGC]={0};
        char *ptr = NULL;
        char *s = strtok_r(buff," ",&ptr);
        while(s != NULL)
        {
            myargv[i++] = s;
            s = strtok_r(NULL," ",&ptr);
        }
        char * cmd = myargv[0];
        printf("%s,%s",myargv[0],myargv[1]);
        char * name_cmd = myargv[1];
        if(cmd == NULL)
        {
            send(c,"error",5,0);
            printf("cmd == NULL");
            continue;
        }
        if(strcmp(cmd,"get")==0)
        {//get
            send_file(c,name_cmd);
        }

        else if(strcmp(cmd,"put") == 0)
        {//put
            printf("cmd=%s\n",name_cmd);
            get_file(c,name_cmd);
        }
        else//bash
        {
            printf("no send file\n");
            int pipefd[2];
            pipe(pipefd);
            pid_t pid = fork();

            if(pid == -1)
            {
                send(c,"fork error",10,0);
                continue;
            }
            if(pid == 0)
            {
                dup2(pipefd[1],1);
                dup2(pipefd[1],2);
                execvp(cmd,myargv);
                perror("cmd err");
                exit(0);
            }
            close(pipefd[1]);//关闭标准错误输出
            wait(NULL);
            while(1)
            {
                char readbuff[READ_BUFF]={"ok#"};
                if(read(pipefd[0],readbuff+3,READ_BUFF-4) <= 0)
                {
                    send(c,"over",4,0);
                    break;
                }
                send(c,readbuff,strlen(readbuff),0);
            }
            close(pipefd[0]);
        }
    }
    close(c);
}
int thread_start(int c)
{
    pthread_t id;
    int res=pthread_create(&id,NULL,work_thread,(void*)c);
    if(res != 0)
    {return -1;}
    return 0;
}
