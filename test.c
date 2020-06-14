#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
int main()
{
    int x=1;
    switch (x)
    {
    case 1:{
    char str[]="apple";
        printf("%s",str);}
        break;
    
    default:
        break;
    }
}