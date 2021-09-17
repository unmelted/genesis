/*
 *  2021.09.15 created by Kelly @4DREPLAY KOREA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include "util/DefData.hpp"

#define VER "0.1.0"

int TestFeature(unsigned char* framedata);

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
    //printf(" argv config %c \n", argv[1]);
    c = 1;

    //while(!exit)
    {
    
        //c = getcha();
        //printf("c %d \n", c);

        switch(c)
        {
        case 1 : 
            printf("Start \n");
            break;
            
        case 99: //c
            TestFeature((unsigned char*)argv[1]);
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

int TestFeature(unsigned char* framedata)
{
    printf("Enter! \n");
    static int index = 0;    
    printf("Enter! 2\n");
    IMG* bframe = CreateImage(1024, 688, 0, framedata);
    //IMG* bframe = CreateImage(1024, 688, 0);    
    printf(" 2 \n");
    char fname[25] = {0, };
    sprintf(fname, "test/saveimage_%d.png", index);
    index++;
    printf(" 3 %s \n", fname);    
    SaveImagePNG(bframe, fname);
    printf(" 4 \n");
    DestroyImage(bframe);
}

int ExtractFeature()
{
    printf(" Extract Feature is called \n");

}

void GetVersion()
{
    cout<< "Cur Version : " << VER << endl;
}

extern "C" {
    void Feature(unsigned char* buffers) {
        TestFeature(buffers);
    }
    void GetVerion() {
        GetVersion();
    }
    int Extract(unsigned char* beffers) {
        printf("first step");
    }
}
