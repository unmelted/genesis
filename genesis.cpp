/*
 *  2021.09.15 created by Kelly @4DREPLAY KOREA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include "include/DefData.hpp"

char getcha()
{
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        printf("%d\n",buf);
        return (buf);
}


int main(int n, char **argv)
{
    bool exit = false;
    int c;
    int result = -1;
    printf(" ::::: GENESIS ::::: \n");
    printf(" argv config %c \n", argv[1]);

    while(!exit)
    {
    
        c = getcha();
        printf("c %d \n", c);

        switch(c)
        {
        case 99: //c

            break;

        case 105: //i
            break;

        case 113://q
            exit = true;
            break;
        case 115://s

            break;
        default:
            break;

        }
    }

    printf("main While loop terminated. \n");
    return 1;
}

void TestFeature(int n, wchar_t* path)
{
    printf("Enter count : %d ", n);
    //string imglist = path;
    printf("recieved message %ls \n", path);

    /* for(int i = 0 ; i < n ; i ++)
    {

    }*/
}