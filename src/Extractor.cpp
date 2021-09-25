
/*****************************************************************************
*                                                                            *
*                            Extractor      								 *
*                                                                            *
*   Copyright (C) 2021 By 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : Extractor.Cpp
    Author(S)       : Me Eunkyung
    Created         : 17 Sep 2021

    Description     : Extractor.Cpp
    Notes           : Feature Extractor From Image.
*/

#include "Extractor.hpp"    

using namespace std;
using namespace cv;

Extractor::Extractor(string& imgset, int cnt, int* roi)
{
    Logger("Param set start ..  count %d ", cnt);
    p = (PARAM*)g_os_malloc(sizeof(PARAM));
    p->count = cnt/2 - 1;
    p->region = (Pt *)g_os_malloc(sizeof(Pt)* p->count);

    imgs = LoadImages(imgset);
    RoiScale(roi);

#ifdef _IMGDEBUG    
        //SaveImageSet(imgs);
#endif
}
void Extractor::RoiScale(int* roi) {

    if(p_scale != 1 ){
        for(int i = 0; i < p->count; i++) {
            int j = (i*2) + 1;
            p->region[i].x = int(roi[j]/p_scale);
            p->region[i].y = int(roi[j+1]/p_scale);
            //Logger("insert %d %d ", roi[j], roi[j+1]);
        }
    }
}

Extractor::~Extractor()
{

}
void Extractor::DrawInfo() {

    int index = 0;
    for( const auto& each : cal_group) {
        Mat dst;
        char filename[30] = {0, };
        drawKeypoints(each.img, each.ip, dst);
        sprintf(filename, "saved/%2d_feature.png", index);
        imwrite(filename, dst);
        index++;
/* 
        for(int i = 0; i < each.ip.size(); i++) {
            Logger("%d, %d  oct %d class %d resp %lf size %lf angle %lf", int(each.ip[i].pt.x), int(each.ip[i].pt.y), each.ip[i].class_id,
            each.ip[i].class_id, each.ip[i].response, each.ip[i].size , each.ip[i].angle);
        }
 */    }

}

void Extractor::SaveImageSet(vector<Mat>& images) {

    Logger("Save Image Set is called ");
    char filename[30] = {0, };
    int index = 0;
    for (const auto& img : images) {
        sprintf(filename, "saved/%2d_saveimg.png", index);
        imwrite(filename, img);
        index++;
    }
}

vector<Mat> Extractor::LoadImages(const string& path) {
    const int FK = 3500;
    const int FHD = 1900;

    namespace fs = std::__fs::filesystem;

    vector<string> image_paths;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry) &&
            entry.path().extension().string() == ".png") {
            image_paths.push_back(entry.path().string());
        }
    }

    if (p_scale == 0 ) {
        Mat sample = imread(image_paths[0]);
        if (sample.cols > FK ) {
            p_scale = 2;
        } else if (sample.cols > FHD) {
            p_scale = 2;
        } else {
            p_scale = 1;
        }
    }

    sort(begin(image_paths), end(image_paths), less<string>());
    vector<Mat> images;
    for (const string& ip : image_paths) {
        Logger("Read image : %s ", ip.c_str());        
        images.push_back(imread(ip));
    }
    return images;
}

int Extractor::Execute()
{
    int index = 0;
    for (Mat& img : imgs) {
        SCENE sc;        
        sc.ori_img = img;        
        sc.img = ProcessImages(img, blur_ksize, blur_sigma);

        int ret = GetFeature(&sc);
        cal_group.push_back(sc);

        if(index > 0) {
            SetCurTrainScene(&cal_group[index - 1]);
            SetCurQueryScene(&cal_group[index]);
            int ret = MakeMatchPair();

            if( ret > 0) {
                PostProcess(&sc);
            }
        }

        index++;
#if _DEBUG
        if (index > 2)
        break;
#endif

    }

    return ERR_NONE;
}


Mat Extractor::ProcessImages(Mat& img, int ksize, double sigma) 
{
    Mat blur_img; Mat dst;
    if ( p_scale != 1) {
        resize(img, img, Size(int(img.cols/p_scale), int(img.rows/p_scale)), 0, 0, 1);
    }

    cvtColor(img, blur_img, cv::COLOR_RGBA2GRAY);
    normalize(blur_img, dst, 0, 255, NORM_MINMAX,-1, noArray());        
    GaussianBlur(dst, blur_img, {ksize, ksize}, sigma, sigma);

    return blur_img;
}

int Extractor::GetFeature(SCENE* sc) 
{
    // FAST + BRIEF
    //auto feature_detector = FastFeatureDetector::create(fast_k, true, FastFeatureDetector::TYPE_9_16);
    //auto feature_detector = AgastFeatureDetector::create(); //AGAST
    //Ptr<xfeatures2d::MSDDetector>feature_detector = xfeatures2d::MSDDetector::create(9,11,15); //MSDETECTOR
    //Ptr<xfeatures2d::BriefDescriptorExtractor> dscr;
    //dscr = xfeatures2d::BriefDescriptorExtractor::create(desc_byte, use_ori);
 
    //AKAZE
    auto feature_detector = KAZE::create(false, false, 0.001f, 4, 4, KAZE::DIFF_PM_G1);
    Ptr<KAZE>dscr = feature_detector;

    Mat desc;    
    vector<KeyPoint> kpt;
    vector<KeyPoint> f_kpt;

    feature_detector->detect(sc->img, kpt);
    Logger("extracted keypoints count : %d", kpt.size());
    f_kpt = MaskKeypointWithROI(&kpt);
    dscr->compute(sc->img, f_kpt, desc);

    sc->ip = f_kpt;
    sc->desc = desc;

#if _DEBUG
/*     for (int i = 0 ; i < sc->ip.size(); i ++) {
        Logger("Keypoint index %2d id %3d, x %f y %f ",i, sc->ip[i].class_id, 
                            sc->ip[i].pt.x, sc->ip[i].pt.y);
    }
 */#endif

    return ERR_NONE;
}

vector<KeyPoint> Extractor::MaskKeypointWithROI(vector<KeyPoint>* oip) {
    
    vector<KeyPoint> ip;
    //Logger("Before masking %d ", oip->size());
    int left = 0;
    int total = 0;
    int del = 0;

    for(auto it = oip->begin(); it != oip->end(); it++) {
        total ++;
        Pt cp(int(it->pt.x), int(it->pt.y));
        int ret = isInside(p->region, p->count, cp);

        if (ret == 0 && it != oip->end()) {
            del ++;
        }
/*         else if (it->pt.x > 900 && it->pt.x < 1040 &&
                it->pt.y > 365 && it->pt.y < 490) {
                    Logger("except special case .. ");
        }
 */        else {
            ip.push_back(*it);
            left ++;
        }
    }

    Logger("new vector  %d. left %d  del %d / total ip %d ", ip.size(), left, del, total);
    return ip;
}
int Extractor::MakeMatchPair() {

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);

    vector<DMatch> in;
    vector<DMatch> matches;    

    if( cur_train->desc.type() != CV_32F || cur_query->desc.type() != CV_32F) {
        cur_train->desc.convertTo(cur_train->desc, CV_32F);
        cur_query->desc.convertTo(cur_query->desc, CV_32F);
    }

    matcher->match(cur_query->desc, cur_train->desc, in);
    sort(in.begin(), in.end());
    Logger("matchees before cut %d  ", in.size());
    for (int i = 0 ; i < in.size(); i ++) {
        Logger("Distance %f imgidx %d trainidx %d queryidx %d", in[i].distance,
        in[i].imgIdx, in[i].trainIdx, in[i].queryIdx);
    }


    int min, max = 0;
    float max_score = in[0].distance * 3;
    Logger("max score %f ", max_score);
    if( cur_train->ip.size() >= cur_query->ip.size()) {
        max = cur_train->ip.size();
        min = cur_query->ip.size();        
    }
    else {
        max = cur_query->ip.size();        
        min = cur_train->ip.size();
    }

    int* t_hist = (int *)g_os_malloc(sizeof(int) * cur_train->ip.size());
    int* q_hist = (int *)g_os_malloc(sizeof(int) * cur_query->ip.size());
    for(int a = 0 ; a < cur_train->ip.size() ; a ++)
        t_hist[a] = 0;
    for(int b = 0 ; b < cur_query->ip.size() ; b ++)
        q_hist[b] = 0;

    //min = 3;
    int is = 0;
    for (int t = 0 ; t < min ; t ++) {
        if(t_hist[in[t].trainIdx] == 0 && q_hist[in[t].queryIdx] == 0) {
            matches.push_back(in[t]);

            t_hist[in[t].trainIdx]++;
            q_hist[in[t].queryIdx]++;
            is ++;
            if(is >= min)
                break;
        }
        else
            Logger("double check %d %d ", in[t].trainIdx, in[t].queryIdx );
    }

    g_os_free(t_hist);
    g_os_free(q_hist);

#if defined _DEBUG

    Logger("matchees after cut %d  ", matches.size());
    for (int i = 0 ; i < matches.size(); i ++) {
        Logger("Distance %f imgidx %d trainidx %d queryidx %d", matches[i].distance,
        matches[i].imgIdx, matches[i].trainIdx, matches[i].queryIdx);
    }

#endif

    vector<Point2f> train_pt, query_pt;
#if 0    
    for (vector<DMatch>::const_iterator it = matches.begin() ; it != matches.end(); it++ ) {
        float tx = cur_train->ip[it->trainIdx].pt.x;
        float ty = cur_train->ip[it->trainIdx].pt.y;

        float qx = cur_query->ip[it->queryIdx].pt.x;
        float qy = cur_query->ip[it->queryIdx].pt.y;
        
        Logger("_pt push %f %f %f %f ", tx, ty, qx, qy);

        if((tx > 0 && ty > 0) && (tx < cur_train->img.cols && ty < cur_train->img.rows) &&
            (qx > 0 && qy > 0 ) && (qx < cur_query->img.cols && qy < cur_query->img.rows )) {
            train_pt.push_back(Point2f(tx, ty));            
            query_pt.push_back(Point2f(qx, qy));
        }
    }
#else
            train_pt.push_back(Point2f(365, ));            
            query_pt.push_back(Point2f(qx, qy));

            train_pt.push_back(Point2f(tx, ty));            
            query_pt.push_back(Point2f(qx, qy));

            train_pt.push_back(Point2f(tx, ty));            
            query_pt.push_back(Point2f(qx, qy));

#endif
    //Mat _h = findHomography(train_pt, query_pt, FM_RANSAC);
    Mat _h = getAffineTransform(query_pt, train_pt);   
    //Mat _h = estimateAffine2D(query_pt, train_pt);
    //Mat _h = estimateRigidTransform(query_pt, train_pt, false);

    cur_query->matrix = _h;
    Logger(" h shape : %d %d ", _h.cols, _h.rows);
    

#if defined _DEBUG
/*         for (int i = 0 ; i < matches.size(); i ++) {
        Logger("Distance %f ", matches[i].distance);
    }
*/
    static int index = 0;
    Mat outputImg = cur_train->img.clone(); 
    drawMatches(cur_query->img, cur_query->ip, cur_train->img, cur_train->ip,
                matches, outputImg, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    char filename[30] = {0, };
    sprintf(filename, "saved/%2d_matchimg.png", index);
    imwrite(filename, outputImg);

    for (int i = 0 ; i < _h.rows ; i ++ ){
        for (int j = 0 ; j < _h.cols ; j ++) {
            Logger("[%d][%d] %lf ",i, j, _h.at<double>(i,j));
        }
    }

    Mat fin, fin2;
    //warpPerspective(cur_train->img, fin, _h, Size(cur_train->img.cols, cur_train->img.rows));
    warpAffine(cur_query->ori_img, fin, _h, Size(cur_query->img.cols*p_scale, cur_train->img.rows*p_scale));
    //cur_query->img = fin;
    sprintf(filename, "saved/%2d_perspective.png", index);
    imwrite(filename, fin);

    warpAffine(cur_query->img, fin2, _h, Size(cur_query->img.cols, cur_train->img.rows));
    sprintf(filename, "saved/%2d_perspective_pp.png", index);
    imwrite(filename, fin2);

    index ++;
#endif         
    return matches.size();
}

int Extractor::PostProcess(SCENE* sc) {

    return ERR_NONE;
}