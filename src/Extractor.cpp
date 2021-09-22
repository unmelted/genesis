
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

Extractor::Extractor(string& imgset)
{
    imgs = LoadImages(imgset);
    imgs = BlurImages(imgs, blur_ksize, blur_sigma);

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
        sprintf(filename, "saved/feature__%d.png", index);
        imwrite(filename, dst);
        index++;
    }
}

int Extractor::Execute()
{
    Ptr<xfeatures2d::BriefDescriptorExtractor> dscr;
    dscr = xfeatures2d::BriefDescriptorExtractor::create(desc_byte, use_ori);

    for (const auto& img : imgs) {
        Mat desc;
        SCENE sc;        
        vector<KeyPoint>ip = Fast(img);
        dscr->compute(img, ip, desc);
        Logger("desc..? width : %d height : %d ", desc.size().width, desc.size().height);
        sc.img = img;
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
        sprintf(filename, "saved/saveimg__%d.png", index);
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
    for (const auto& ip : image_paths) {
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