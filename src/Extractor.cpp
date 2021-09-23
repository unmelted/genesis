
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
    imgs = LoadImages(imgset);
    imgs = BlurImages(imgs, blur_ksize, blur_sigma);

    Logger("Param set start ..  count %d ", cnt);
    p = (PARAM*)g_os_malloc(sizeof(PARAM));
    p->count = cnt/2;
    p->region = (Pt *)g_os_malloc(sizeof(Pt)* p->count);

    for(int i = 0; i < p->count; i++) {
        int j = (i*2) + 1;
        p->region[i].x = roi[j];
        p->region[i].y = roi[j+1];
        //Logger("insert %d %d ", roi[j], roi[j+1]);
    }

#ifdef _IMGDEBUG    
        SaveImageSet(imgs);
#endif
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
    }

    for(int i = 0; i < p->count; i++) {
        Logger("roi pt : %d %d ", p->region[i].x, p->region[i].y );
    }
}

int Extractor::Execute()
{
    Ptr<xfeatures2d::BriefDescriptorExtractor> dscr;
    dscr = xfeatures2d::BriefDescriptorExtractor::create(desc_byte, use_ori);

    for (const auto& img : imgs) {
        Mat desc;
        SCENE sc;        
        sc.img = img;        
        vector<KeyPoint>ip = Fast(img);
        dscr->compute(img, ip, desc);
        int r = MaskKeypointWithROI(&ip);        
        Logger("desc..? width : %d height : %d ", desc.size().width, desc.size().height);

        sc.ip = ip;
        sc.desc = desc;
        cal_group.push_back(sc);
    }

    return ERR_NONE;
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
        if (sample.cols > 3600 ) {
            p_scale = 4;
        } else if (sample.cols > 1900) {
            p_scale = 2;
        } else {
            p_scale = 1;
        }
    }

    sort(begin(image_paths), end(image_paths), less<string>());
    vector<Mat> images;
    for (const string& ip : image_paths) {
        Logger("Read image : %s ", ip.c_str());        
        if ( p_scale == 1) {
            images.push_back(imread(ip));
        } else {
            Mat t = imread(ip);
            resize(t, t, Size(int(t.cols/p_scale), int(t.rows/p_scale)), 0, 0, 1);
            images.push_back(t);
        }
    }
    return images;
}

vector<Mat> Extractor::BlurImages(const vector<Mat>& images, int ksize, double sigma) 
{
    vector<Mat> blurred_images;
    for (const auto& img : images) {
        Mat blur_img;
        cvtColor(img, blur_img, cv::COLOR_RGBA2GRAY);
        GaussianBlur(blur_img, blur_img, {ksize, ksize}, sigma, sigma);
        blurred_images.push_back(blur_img);
    }
    return blurred_images;
}

vector<KeyPoint> Extractor::Fast(const Mat& image) 
{
    auto feature_detector = FastFeatureDetector::create();
    vector<KeyPoint> keypoints;
    feature_detector->detect(image, keypoints);
    Logger("extracted keypoints count : %d", keypoints.size());
    
    return keypoints;
}

int Extractor::MaskKeypointWithROI(vector<KeyPoint>* oip) {

    Logger("Before masking %d ", oip->size());

    for(auto it = oip->begin(); it != oip->end(); it++) {
        Pt cp(int(it->pt.x), int(it->pt.y));
        bool ret = isInside(p->region, p->count, cp);
        //Logger(" %f, %f inside ? %d ", it->pt.x, it->pt.y, ret);

        if (ret == false) {
            oip->erase(it);
            //Logger("Erase!! ");
        }
    }
    Logger("After masking %d ", oip->size());
}