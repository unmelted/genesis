
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
#include "../util/Pip.hpp"


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
    const int blur_ksize = 9;
    const float blur_sigma = 0.9;
    const int desc_byte = 32;
    const bool use_ori = true;
    const int nms_k = 9;
    const int fast_k = 24;
    const int minx = 0;

    int p_scale = 0;
    SCENE* cur_train = 0;
    SCENE* cur_query = 0;

    vector<Mat>LoadImages(const string& path);
    void SaveImageSet(vector<Mat>& images);
    void InitializeData(int* roi);
    void NormalizePoint(Pt* fpt, int maxrange);
    int CalculateCenter(SCENE* sc1, SCENE* sc2);


    int GetPreCalibraitonData(SCENE* sc);
    Mat ProcessImages(Mat& img, int ksize, double sigma);
    int GetFeature(SCENE* sc);
    vector<KeyPoint> MaskKeypointWithROI(vector<KeyPoint>* oip);
    void SetCurTrainScene(SCENE* sc) { cur_train = sc; };
    void SetCurQueryScene(SCENE* sc) { cur_query = sc; };
    int MakeMatchPair();
    int PostProcess(SCENE* sc);
    int GetVirtualRod(SCENE* sc);
    int FirstSolvePnP(Pt* set1, Pt* set2);
};
