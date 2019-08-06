#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    char buff[1024] = "./shell.sh a.c";

    system(buff);
    FILE *fr = fopen("a.c.md5","r");
    char ch = getc(fr);
    char out[128];
    while(ch != ' ')
    {
        printf("%c",ch);
        ch=getc(fr);
    }
    printf("\n");
    system("./delete.sh a.c");   
    return 0;
}
