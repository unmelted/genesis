
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
    int VerifyNumeric();
    void DrawInfo();
    PARAM* p;

    vector<Mat> imgs;
    vector<SCENE> cal_group;


private :

    bool is_first;
    bool verify_mode = false;

    SCENE* cur_train = 0;
    SCENE* cur_query = 0;

    vector<Mat>LoadImages(const string& path);
    void SaveImageSet(vector<Mat>& images);
    void InitializeData(int cnt, int* roi);
    int UpdateConfig();
    void NormalizePoint(SCENE* sc, int maxrange);
    int CalculateCenter(SCENE* sc1, SCENE* sc2);


    Mat ProcessImages(Mat& img);
    int GetFeature(SCENE* sc);
    vector<KeyPoint> MaskKeypointWithROI(vector<KeyPoint>* oip);
    void SetCurTrainScene(SCENE* sc) { cur_train = sc; };
    void SetCurQueryScene(SCENE* sc) { cur_query = sc; };
    int MakeMatchPair();
    int PostProcess();
    int CalVirtualRod();
    int SolvePnP();
    int SolveRnRbyH();
    int CalAdjustData();
    int Warping();
    int WarpingStep1();
    int WarpingStep2();
    
};
