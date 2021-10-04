  
/*****************************************************************************
*                                                                            *
*                            MtrxUtil         								 *
*                                                                            *
*   Copyright (C) 2021 By 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : MtrxUtil.cpp
    Author(S)       : Me Eunkyung
    Created         : 04 Oct 2021

    Description     : MtrxUtil.Cpp
    Notes           : Matrix calculate uility
*/

#include "MtrxUtil.hpp"

Point2f MtrxUtil::GetRotatePoint(Point2f ptCenter, Point2f ptRot, double dbAngle)
{
    Point2f ptResult =Point2f();

    // 회전 중심좌표와의 상대좌표
    ptRot.x = ptRot.x - ptCenter.x;
    ptRot.y = -(ptRot.y - ptCenter.y);

    double cosx = cos(dbAngle);
    double sinx = sin(dbAngle);

    ptResult.x = (float)(ptRot.x * cosx - ptRot.y * sinx);
    ptResult.y = (float)(ptRot.x * sinx + ptRot.y * cosx);

    ptResult.x = ptResult.x + ptCenter.x;
    ptResult.y = -(ptResult.y - ptCenter.y);

    return ptResult;

}

Mat MtrxUtil::GetRotationMatrix(float rad, float cx, float cy) {

    Mat m = Mat::eye(3, 3, CV_32FC1);
    if( rad == 0)
        return m;
    
    Mat mtrana, mtranb, mrot;
    mtrana = GetTranslationMatrix(-cx, -cy);
    mrot = GetRotationMatrix(rad);
    mtranb = GetTranslationMatrix(cx, cy);

    m = mtranb * mrot * mtrana;
    return m;

}

Mat MtrxUtil::GetScaleMatrix(float scalex, float scaley, float cx, float cy) {

    Mat m = Mat::eye(3, 3, CV_32FC1);

    if( scalex == 0 && scaley == 0)
        return m;

    Mat mtrana, mtranb, mscale;

    mtrana = GetTranslationMatrix(-cx, -cy);
    mscale = GetScaleMatrix(scalex, scaley);
    mtranb = GetTranslationMatrix(cx, cy);

    m = mtranb * mscale * mtrana;
    return m;
}

Mat MtrxUtil::GetScaleMatrix(float scalex, float scaley) {

    Mat m = Mat::eye(3, 3, CV_32FC1);
    if (scalex == 0 && scaley == 0)
        return m;
    
    m.at<float>(0, 0) = scalex;
    m.at<float>(1, 1) = scaley;
    return m;

}

Mat MtrxUtil::GetTranslationMatrix(float tx, float ty) {

    Mat m = Mat::eye(3, 3, CV_32FC1);
    if (tx == 0 && ty == 0)
        return m;
    
    m.at<float>(0, 2) = tx;
    m.at<float>(1, 2) = ty;
    return m;
}

Mat MtrxUtil::GetRotationMatrix(float rad) {

    Mat m = Mat::eye(3, 3, CV_32FC1);
    if (rad == 0)
        return m;
    
    m.at<float>(0, 0) = (float)cos(rad);
    m.at<float>(0, 1) = (float)-sin(rad);
    m.at<float>(1, 0) = (float)sin(rad);
    m.at<float>(1, 1) = (float)cos(rad);

    return m;

}

Point2f MtrxUtil::TransformPtbyHomography(FPt* in, Mat& homography) {

    Mat mcenter(3, 1, CV_64F);
    mcenter.at<double>(0) = in->x;
    mcenter.at<double>(1) = in->y;    
    mcenter.at<double>(2) = 1;
    Mat mret = homography * mcenter;

    double newx = mret.at<double>(0) / mret.at<double>(2);
    double newy = mret.at<double>(1) / mret.at<double>(2);    
    Point2f newpt = Point2f((float)newx, (float)newy);
    return newpt;
}