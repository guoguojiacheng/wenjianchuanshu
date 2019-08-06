#include <stdio.h>
int main()
{
    char cmd[128]= "./shell.sh a.c";
    system(cmd);
    return 0;
}
