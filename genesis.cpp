/*
 *  2020.02.11 created by Kelly @nepes
 *  SouthWrap main
 *  SouthWrap.cpp
 *  Copyright (c) 2020, nepes inc, All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include "SouthWrap.hpp"
#include "SouthWrapEx.hpp"

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

//int main(int argc, char *argv[])
int SouthWrap(char *argv)
{
    bool exit = false;
    int c;
    int result = -1;
    printf(" ::::: TRINITY SOUTHWRAPPER ::::: \n");

    SouthWrapper* sthwr = new SouthWrapper(argv, "connect.json");
    printf(" argv config %c \n", argv[1]);

    while(!exit)
    {
        c = getcha();
        printf("c %d \n", c);

        switch(c)
        {
        case 99: //c
            //sthwr->ConnectToMed("113.130.69.145", 8000);
            sthwr->ConnectToMed();
            break;

        case 105: //i
            sthwr->InitModule();
            break;

        case 113://q
            exit = true;
            break;
        case 115://s
            sthwr->SendStatusUpdate();
            break;
        default:
            break;

        }
    }

    printf("main While loop terminated. delete sthwr \n");
    delete sthwr;
    return 1;
}
