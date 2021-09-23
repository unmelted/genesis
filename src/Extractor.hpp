
/*****************************************************************************
*                                                                            *
*                            Extractor      								 *
*                                                                            *
*   Copyright (C) 2021 By 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : Extractor.Hpp
    Author(S)       : Me Eunkyung
    Created         : 17 Sep 2021

    Description     : Extractor.Hpp
    Notes           : Feature Extractor From Image.
*/
#include <filesystem>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "../util/DefData.hpp"
#include "../util/pnp.hpp"


using namespace std;
using namespace cv;

class Extractor {

public :
    Extractor(string& imgset, int cnt , int* roi);
    ~Extractor();
    int Execute();
    void DrawInfo();
    PARAM* p;

    vector<Mat> imgs;
    vector<SCENE> cal_group;


private :
    const int blur_ksize = 7;
    const float blur_sigma = 1.0;
    const int desc_byte = 32;
    const bool use_ori = true;
    int p_scale = 0;
    SCENE* cur_train = 0;

    void SaveImageSet(vector<Mat>& images);
    int MaskKeypointWithROI(vector<KeyPoint>* oip);
    void SetCurTrainScene(SCENE* sc) { cur_train = sc; };

    vector<Mat>LoadImages(const string& path);
    vector<Mat>BlurImages(const vector<Mat>& images, int ksize, double sigma);
    vector<KeyPoint>Fast(const Mat& image);
};
