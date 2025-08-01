#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
// ./test $(python3 -c 'import sys; sys.stdout.buffer.write(b"A"*1024)')
void bof(char *str)
{
    char buffer[1024];
    strcpy(buffer,str);
    for(int i =0; i<10; i++)
    {
        printf("%c", buffer[i]);
    }
    printf(" middle value: %c \n", buffer[500]);
    printf(" last value: %c \n", buffer[1023]);
    return;
}
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("input string needed");
        return -1;
    }
    bof(argv[1]);
    return 0;
}