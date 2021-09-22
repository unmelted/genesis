
/*****************************************************************************
*                                                                            *
*                            Extractor      								 *
*                                                                            *
*   Copyright (c) 2021 by 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : Extractor.hpp
    Author(s)       : Me Eunkyung
    Created         : 17 Sep 2021

    Description     : Extractor.hpp
    Notes           : Feature Extractor from image.
*/
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "../util/DefData.hpp"


using namespace std;
using namespace cv;

class Extractor {

public :
    Extractor(string& imgset);
    ~Extractor();
    int Execute();
    void DrawInfo();

    vector<Mat> imgs;
    vector<SCENE> cal_group;

private :
    const int blur_ksize = 7;
    const float blur_sigma = 1.0;
    const int desc_byte = 32;
    const bool use_ori = true;
    int p_scale = 0;

    void SaveImageSet(vector<Mat>& images);

    vector<Mat>LoadImages(const string& path);
    vector<Mat>BlurImages(const vector<Mat>& images, int ksize, double sigma);
    vector<KeyPoint>Fast(const Mat& image);
};
