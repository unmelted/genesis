
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
#include <cmath>
#include "../util/DefData.hpp"
#include "../util/Pip.hpp"
#include "MtrxUtil.hpp"

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
    MtrxUtil mtrx;

    vector<Mat> imgs;
    vector<SCENE> cal_group;


private :

    bool is_first;
    bool verify_mode = false;

    SCENE* cur_train = 0;
    SCENE* cur_query = 0;

    int UpdateConfig();    
    vector<Mat>LoadImages(const string& path);
    void SaveImageSet(vector<Mat>& images);
    void InitializeData(int cnt, int* roi);
    
    Mat ProcessImages(Mat& img);
    int GetFeature(SCENE* sc);
    vector<KeyPoint> KeypointMasking(vector<KeyPoint>* oip);
    
    void SetCurTrainScene(SCENE* sc) { cur_train = sc; };
    void SetCurQueryScene(SCENE* sc) { cur_query = sc; };
    int FindHomographyMatch();
    void NormalizePoint(SCENE* sc, int maxrange);
    int DecomposeHomography();
    int FindHomographyP2P(); 


    int PostProcess();
    int CalVirtualRod();
    int SolvePnP();
    
    ADJST CalAdjustData();
    int Warping();
    int WarpingStep1();
    int WarpingStep2();
    int AdjustImage(ADJST adj);

};
