
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
#include "Util.hpp"

using namespace std;
using namespace cv;

class Extractor {

public :
    Extractor(string& imgset, int cnt , int* roi);
    ~Extractor();
    int Execute();
    int VerifyNumeric();

    PARAM* p;
    MtrxUtil mtrx;
    Util genutil;

    vector<string>image_paths;
    vector<string>dsc_id;
    vector<Mat>imgs;
    vector<SCENE>cal_group;


private :
    TIMER* t;
    bool verify_mode = false;

    SCENE* cur_train = 0;
    SCENE* cur_query = 0;

    int UpdateConfig();    
    vector<Mat>LoadImages(const string& path);
    void SaveImage(SCENE* sc, int type = 0);
    void InitializeData(int cnt, int* roi);
    
    Mat ProcessImages(Mat& img);
    int ImageMasking(SCENE* sc);
    int GetFeature(SCENE* sc);
    vector<KeyPoint> KeypointMasking(vector<KeyPoint>* oip);
    int Match();    
    int MatchSplit(vector<Point2f> m_train, vector<Point2f>m_query);
    int PostProcess();
    void MakingLog();


    float GetDistance(float x1, float y1, float x2, float y2) {
        float dx = x1 - x2;
        float dy = y2 - y2;
        float distance = sqrt( dx * dx + dy * dy);
        return distance;
    };

    void SetCurTrainScene(SCENE* sc) { cur_train = sc; };
    void SetCurQueryScene(SCENE* sc) { cur_query = sc; };
    int FindBaseCoordfromWd(int mode = 0);
    int FindHomographyMatch();
    int FindHomographyP2P(); 
    vector<DMatch> RefineMatch(vector<DMatch> good);
    vector<DMatch> RemoveOutlier(vector<DMatch> matches);

    void NormalizePoint(SCENE* sc, int maxrange);
    int DecomposeHomography();
    ADJST CalAdjustData();
    void DrawNormal();
    void ApplyImage();

    int Warping();
    int WarpingStep1();
    int WarpingStep2();
    int AdjustImage(ADJST adj);

};
