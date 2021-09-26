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
    int z;
    Pt(int a, int b ) {
        x = a; y = b; z = 0;
    };
    Pt() { x = 0, y = 0; z = 0;};
};

typedef struct _maindata {
    Mat img;
    Mat ori_img;
    
    Pt four_pt[4];
    Pt center;
    
    double norm;
    double degree;
    Mat rot_matrix;

    int dim;
    Pt* roi;
    vector<KeyPoint> ip;
    Mat desc;
    Mat matrix_fromimg;

} SCENE;

typedef struct _PARAM {
    int ground;     //Groud type
    int count;      // Region point count

    float sensor_size = 17.30 / 1.35;
    float focal = 3840;
    float normal[2][3] = { 0, };

    Pt world[4];    // World Coord 4 point
    Pt world_center;
    Pt* region;     // Point array

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