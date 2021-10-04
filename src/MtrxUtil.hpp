  
/*****************************************************************************
*                                                                            *
*                            MtrxUtil         								 *
*                                                                            *
*   Copyright (C) 2021 By 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : MtrxUtil.hpp
    Author(S)       : Me Eunkyung
    Created         : 04 Oct 2021

    Description     : MtrxUtil.hpp
    Notes           : Matrix calculate uility
*/

#include <cmath>
#include "../util/DefData.hpp"
#include "../util/Pip.hpp"


using namespace std;
using namespace cv;


class MtrxUtil {

public:

    Point2f GetRotatePoint(Point2f ptCenter, Point2f ptRot, double dbAngle);
    Mat GetRotationMatrix(float rad, float cx, float cy);
    Mat GetScaleMatrix(float scalex, float scaley, float cx, float cy);
    Mat GetScaleMatrix(float scalex, float scaley);
    Mat GetTranslationMatrix(float tx, float ty);
    Mat GetRotationMatrix(float rad);

    Point2f TransformPtbyHomography(FPt* in, Mat& homography);
};