
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
#include <opencv2/opencv.hpp>
#include "../util/DefData.hpp"


using namespace std;
using namespace cv;

class Extractor {

public :
    Extractor(string& imgset);
    vector<Mat> imgs;
    vector<KeyPoint> imgkeys;

private :
    vector<Mat> load_images(const string& path);
    vector<Mat>blur_images(const vector<Mat>& images, int ksize, double sigma);
    vector<KeyPoint>Fast(const Mat& image);
};
