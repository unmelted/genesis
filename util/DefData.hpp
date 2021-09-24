#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "TimeUtil.hpp"
#include "Image.hpp"

using namespace cv;
using namespace std;

typedef enum _err {
    ERR_NONE = 0,


};


typedef struct Pt
{    
    int x;
    int y;
    Pt(int a, int b ) {
        x = a; y = b;
    };
};

typedef struct _maindata {
    Mat img;
    int dim;
    Pt* roi;
    vector<KeyPoint> ip;
    Mat desc;
} SCENE;

typedef struct _PARAM {
    int ground;    
    int count;
    Pt* region;
} PARAM;

typedef enum groundtype
{
    BaseballHome,
    BaseballGround,
    BasketballHalf,
    BasketballGround,
    Boxing,
    IceLinkHalf,
    IceLink,
    SoccerHalf,
    Soccer,
    Taekwondo,
    TennisHalf,
    Tennis,
    Ufc,
    VolleyballHalf,
    VolleyballGround,
    Football
};