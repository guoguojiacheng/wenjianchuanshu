#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MYARGV 10
#define READBUFF 4096
#define PORT 6000
#define IPPORT "127.0.0.1"
#define BUFF 128
#define RECVBUFF 1024

void put_file(int sockfd,char *name,int fd)
{
    printf("put_file\n");
    char buff[BUFF] = {0};
    if(recv(sockfd,buff,BUFF-1,0) <= 0)
    {
        return;
    }
    if(strncmp(buff,"ok#",3) != 0)
    {
        printf("error\n");
        return;
    }
    /*int fd=open(name,O_RDONLY);
    if(fd == -1)
    {
        printf("open err\n");
        return;
    }*/
    int size = 0;
    sscanf(buff+3,"%d",&size);
    
    int file_size=lseek(fd,0,SEEK_END);
    lseek(fd,size,SEEK_SET);
    
    if(file_size == 0)
    {
        close(fd);
        return;
    }
    if(file_size == size)
    {
        close(fd);
        printf("finish\n");
        return;
    }

    char res_buff[BUFF] = {0};
    sprintf(res_buff,"ok%d",file_size);
    send(sockfd,res_buff,strlen(res_buff),0);

    char readbuff[READBUFF] = {0};
    int num =0;

    printf("put begin\n");
    while((num=read(fd,readbuff,READBUFF)) > 0)
    {
        send(sockfd,readbuff,num,0);
    }
    printf("put over\n");
    close(fd);
    return;
}
void recv_file(int sockfd,char *name)
{
    char buff[BUFF] = {0};
    if(recv(sockfd,buff,BUFF-1,0) <= 0)
    {
        return;
    }
    if(strncmp(buff,"ok#",3) != 0)
    {
        printf("error:%d\n",buff+3);
        return;
    }
    int size_num = 0;
    
    int size = 0;
    printf("file size:%s\n",buff+3);
    sscanf(buff+3,"%d",&size);
    printf("size=%d\n",size);
    if(size == 0)
    {
        send(sockfd,"err",3,0);
        return;
    }

    int fd = open(name,O_WRONLY|O_CREAT,0600);
    if(fd == -1)
    {
        printf("fd == -1\n");
        send(sockfd,"err",3,0);
        return;
    }
    
    size_num = lseek(fd,0,SEEK_END);
    if(size_num==size)
    {
        printf("finish\n");
        close(fd);
        return;
    }

    char res_buff[10]={0};
    sprintf(res_buff,"ok%d",size_num);
    send(sockfd,res_buff,strlen(res_buff),0);
    char recvbuff[RECVBUFF] = {0};

    int num = 0;
    int curr_size = size_num;
    //printf("\033[H");
    printf("get begin\n");
    while((num = recv(sockfd,recvbuff,RECVBUFF,0)) > 0)
    {
        write(fd,recvbuff,num);
        curr_size +=num;

        float f=curr_size*100.0/size;
        printf("进度:%.2f%%\r",f);
        fflush(stdout);
        if(curr_size >= size)
        {
            printf("\n");
            break;
        }
    }
    //printf("\033[?251");
    printf("下载完成\n");
    printf("下载大小:%d,文件大小%d\n",curr_size,size);
    close(fd);
}
int create_socket()
{
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1)
    {
        return -1;
    }
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = inet_addr(IPPORT);

    int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));

    if(res == -1)
    {return -1;}
    return sockfd;
}

int main()
{
    int sockfd = create_socket();
    if(sockfd == -1)
    {
        printf("connect to ser error!\n");
        return 0;
    }
    while(1)
    {
        char buff[128] = {0};
        printf("Connect>>");
        fflush(stdout);

        fgets(buff,128,stdin);
        buff[strlen(buff)-1] = 0;

        if(buff[0]==0)
        {
            continue;
        }

        char sendbuff[128] = {0};
        strcpy(sendbuff,buff);

        int i=0;
        char *myargv[MYARGV] = {0};
        char *s = strtok(buff," ");
        while(s != NULL)
        {
            myargv[i++] = s;
            s = strtok(NULL," ");
        }

        if(myargv[0] == NULL)
        {
            continue;
        }

        if(strcmp(myargv[0],"exit") == 0)
        {
            break;
        }
        else if(strcmp(myargv[0],"get")==0)
        {
            if(myargv[1] == NULL)
            {
                continue;
            }
            send(sockfd,sendbuff,strlen(sendbuff),0);
            recv_file(sockfd,myargv[1]);
        }
        else if(strcmp(myargv[0],"put") == 0)
        {
            if(myargv[1]==NULL)
            {
                continue;
            }

            int fd=open(myargv[1],O_RDONLY);
            if(fd == -1)
            {
                printf("open err\n");
                continue;
            }
            send(sockfd,sendbuff,strlen(sendbuff),0);
            put_file(sockfd,myargv[1],fd);
        }
        else
        {
            send(sockfd,sendbuff,strlen(sendbuff),0);

            //char readbuff[READBUFF] = {0};
            while(1)
            {
                char readbuff[READBUFF] = {0};
                recv(sockfd,readbuff,READBUFF-1,0);
                if(strncmp(readbuff,"over",4)==0)
                {
                    break;
                }
                if(strncmp(readbuff,"ok#",3) != 0)
                {
                    printf("err");
                    continue;
                }
                printf("%s\n",readbuff+3); 
            }
        }
    }
    close(sockfd);
}
