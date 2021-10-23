
/*****************************************************************************
*                                                                            *
*                            ImgUtil          								 *
*                                                                            *
*   Copyright (C) 2021 By 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : ImgUtilr.cppp
    Author(S)       : Me Eunkyung
    Created         : 23 Oct 2021

    Description     : ImgUtil.cpp
    Notes           : Utility related image.
*/

#include "ImgUtil.hpp"

using namespace std;
using namespace cv;

void ImgUtil::SaveImage(SCENE *sc, int type, SCENE* sc2, PARAM* p, int opt)
{
    if (type == 0)
        return;

    if (type == 1)
    {
        Mat img;
        char filename[50] = { 0, };
        drawKeypoints(sc->img, sc->ip, img);
        sprintf(filename, "saved/%d_keypoint.png", sc->id);
        imwrite(filename, img);        
    }
    else if (type == 2)
    {
        Mat img;
        char filename[50] = { 0, };
        img = sc->mask_img;
        sprintf(filename, "saved/%d_masking.png", sc->id);
        imwrite(filename, img);                
    }
    else if (type == 3)
    {
        Mat img;
        char filename[50] = { 0, };
        drawKeypoints(sc->img, sc->ip, img);
        for (int i = 0; i < 4; i++)
        {
            circle(img,
                    Point((int)sc2->four_fpt[i].x/p->p_scale, (int)sc2->four_fpt[i].y/p->p_scale), 6, Scalar(255), -1);

        }        
        sprintf(filename, "saved/%d_keypoint.png", sc->id);
        imwrite(filename, img);        
    }
    else if (type == 4) {
        Mat img;
        char filename[50] = { 0, };

        vector<Scalar> color;
        color.push_back(Scalar(0, 0, 255));
        color.push_back(Scalar(0, 255, 0));
        color.push_back(Scalar(255, 0, 0));
        color.push_back(Scalar(255, 255, 0));
        color.push_back(Scalar(255, 0, 255));
        color.push_back(Scalar(0, 255, 255));        

        drawKeypoints(sc->img, sc->ip, img);
        for (int i = 0; i < 4; i++)       {
            circle(img,
                    Point((int)sc2->four_fpt[i].x/p->p_scale, (int)sc2->four_fpt[i].y/p->p_scale), 6, Scalar(255), -1);
        }

        Point2f tp1 = Point2f(sc->projected_normal.at<Point2f>(0).x/p->p_scale, 
                sc->projected_normal.at<Point2f>(0).y/p->p_scale);

        for (int i = 1 ; i < sc->projected_normal.rows; i ++) {
            Logger("projected point %f %f ", sc->projected_normal.at<Point2f>(i).x,
                    sc->projected_normal.at<Point2f>(i).y);
            Point2f tp2 = Point2f(sc->projected_normal.at<Point2f>(i).x/p->p_scale, 
                    sc->projected_normal.at<Point2f>(i).y/p->p_scale);

            line(img, tp1, tp2, color[i], 2);
        }

        sprintf(filename, "saved/%d_keypoint+normal.png", sc->id);
        imwrite(filename, img);        
    }
    else if (type == 5) {
        Mat img;
        char filename[50] = { 0, };
        img = sc->pyramid[1];
        sprintf(filename, "saved/%d_pyramid_check1.png", sc->id);        
        imwrite(filename, img);        
        img = sc->pyramid[2];
        sprintf(filename, "saved/%d_pyramid_check2.png", sc->id);
        imwrite(filename, img);                

    }
    else if (type == 6) {
        Mat img;
        char filename[50] = { 0, };
        for(int step = 2 ; step >= 0; step --){ 
            int scl = p->pyramid_scale[step];
            int cnt = sc->pyramid_ip[step].size(); 
            img = sc->pyramid[step].clone();            
            for (int i = 0; i < cnt; i++) {
                circle(img,
                    Point((int)sc->pyramid_ip[step][i].pt.x, (int)sc->pyramid_ip[step][i].pt.y), 
                    1, Scalar(125, 125,0), -1);

            }
            sprintf(filename, "saved/%d_%d_pr_sel_point.png", sc->id, step);
            imwrite(filename, img);
        }

    } else if (type == 7) {
        Mat img;
        char filename[50] = { 0, };
        int step = opt;
        img = sc->pyramid[step].clone();
        int scl = p->pyramid_scale[step];
        for (int i = 0; i < 4; i++) {
/*             Logger(" pyramid pair %d %d " , (int)sc->pyramid_pair[step][i].query.x, 
                                    (int)sc->pyramid_pair[step][i].query.y);
 */            
            circle(img,
                Point((int)sc->pyramid_pair[step][i].query.x/scl, (int)sc->pyramid_pair[step][i].query.y/scl), 2, Scalar(255), -1);

            sprintf(filename, "saved/%d_%d_pr2_best_point.png", sc->id, step);
            imwrite(filename, img);
        }
    }
 
}

vector<Mat> ImgUtil::LoadImages(const string &path, vector<string>* dsc_id)
{
    const int FK = 3500;
    const int FHD = 1900;
    vector<string>image_paths;

    namespace fs = std::__fs::filesystem;

    for (const auto &entry : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(entry) &&
            entry.path().extension().string() == ".png")
        {
            image_paths.push_back(entry.path().string());
        }
    }

    sort(begin(image_paths), end(image_paths), less<string>());
    vector<Mat> images;
    int len = path.length();
    for (const string &ip : image_paths)
    {        
        images.push_back(imread(ip));
        string dsc = ip.substr(len, len + 6);
        dsc = dsc.substr(0, 7);
        dsc_id->push_back(dsc);
        Logger("Read image : %s , desc_id %s ", ip.c_str(), dsc.c_str());
    }
    return images;
}