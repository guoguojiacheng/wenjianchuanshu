#include "socket.h"
#include "thread.h"
int main()
{
    int sockfd = create_socket();
    if(sockfd==-1)
    {
        return 0;
    }
    while(1)
    {
        struct sockaddr_in caddr;
        int len=sizeof(caddr);

        int c=accept(sockfd,(struct sockaddr*)&caddr,&len);
        if(c<=0)
        {continue;}
        printf("accept c=%d\n",c);
        int res=thread_start(c);

        if(res==-1)
        {close(c);}
    }


}
