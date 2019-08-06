#include <stdio.h>
#include <unistd.h>
int main()
{
    int a=10;
    int pid = fork();
    if(pid == 0)
    {
        printf("child:%d\n",&a);
    }
    if(pid > 0)
    {
        printf("parent:%d\n",&a);
    }
    wait(NULL);
    return 0;
}
