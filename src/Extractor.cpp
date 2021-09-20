
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

}

vector<Mat> Extractor::load_images(const string& path) {
    namespace fs = std::__fs::filesystem;

    vector<string> image_paths;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry) &&
            entry.path().extension().string() == ".png") {
            image_paths.push_back(entry.path().string());
        }
    }

    sort(begin(image_paths), end(image_paths), less<string>());
    vector<cv::Mat> images;
    for (const auto& ip : image_paths) {
        images.push_back(cv::imread(ip));
    }
    return images;
}

vector<Mat> Extractor::blur_images(const vector<Mat>& images, int ksize, double sigma) 
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
    auto feature_detector = cv::FastFeatureDetector::create();
    vector<cv::KeyPoint> keypoints;
    feature_detector->detect(image, keypoints);
    return keypoints;
}