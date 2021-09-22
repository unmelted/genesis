#pragma once

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "TimeUtil.hpp"
#include "Image.hpp"

using namespace cv;
using namespace std;

typedef enum _err {
    ERR_NONE = 0,


};
typedef struct _maindata {
    Mat img;
    vector<KeyPoint> ip;
    Mat desc;
} SCENE;

typedef struct _PARAM {
    int ground;    
    int dim;
    int* region; //half size 
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