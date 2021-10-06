#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

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


}ERR;


typedef struct _Pt
{    
    int x;
    int y;
    int z;
    _Pt(int a, int b ) {
        x = a; y = b; z = 0;
    };
    _Pt() { x = 0, y = 0; z = 0;};
} Pt;

typedef struct _FPt
{    
    float x;
    float y;
    float z;
    _FPt(float a, float b ) {
        x = a; y = b; z = 0.0;
    };
    _FPt() { x = 0.0, y = 0.0; z = 0.0;};
} FPt;

typedef struct _maindata {
    int id = 0;
    Mat img;
    Mat ori_img;
    Mat mask_img;
    
    //Pt four_pt[4];
    FPt four_fpt[4];    
    FPt center;

    Mat rot_matrix;
    Mat trans_matrix;
    Point2f normal_vec[2];

    double rod_norm;
    double rod_degree;
    Mat rod_rotation_matrix;

    vector<KeyPoint> ip;
    Mat desc;
    Mat matrix_fromimg;

} SCENE;

typedef struct _adj { 
    double angle;
    double scale;
    double trans_x;
    double trans_y;
    double rotate_centerx;
    double rotate_centery;
    Rect rect;

} ADJST;

typedef struct _PARAM {
    int ground;     //Groud type
    int count;      // Region point count

    int blur_ksize;
    float blur_sigma;
    int desc_byte;
    bool use_ori;
    int nms_k;
    int fast_k;
    int minx;
    int p_scale;

    int pwidth;
    int pheight;
    float sensor_size;
    float focal;
    float* camera_matrix;
    float* skew_coeff;
    float normal[2][3];
    
    SCENE* world;    // World Coord 4 point
    Pt* region;     // Point array for polygon (ROI)
    Pt* moved_region;

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
} GDT;