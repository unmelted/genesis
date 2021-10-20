
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


Extractor::Extractor(string &imgset, int cnt, int *roi)
{
    mtrx = MtrxUtil();
    genutil = ExpUtil();
    t = new TIMER();    
    InitializeData(cnt, roi);
    imgs = LoadImages(imgset);


}

Extractor::~Extractor()
{
    g_os_free(p->region);
    g_os_free(p->moved_region);
    g_os_free(p->world);
    g_os_free(p->camera_matrix);
    g_os_free(p->skew_coeff);
    g_os_free(p);
    delete t;
}

void Extractor::InitializeData(int cnt, int *roi)
{
    p = (PARAM *)g_os_malloc(sizeof(PARAM));
    p->calibration_type = RECALIBRATION_3D;
    p->match_type = PYRAMID_MATCH;
    p->roi_type = RECTANGLE;

    //PRESET_NONE_3D setting value
    /*
    p->calibration_type = PRESET_NONE_3D;
    p->roi_type = CIRCLE;
    p->masking_type = FOUR_POINT_BASE;
    p->match_type = BEST_MATCH; */
    if(p->match_type == BEST_MATCH || p->match_type == KNN_MATCH) {
        p->p_scale = 2;
        if (p->roi_type == POLYGON)
        {
            p->roi_count = (cnt - 1) / 2;
            p->region = (Pt *)g_os_malloc(sizeof(Pt) * p->roi_count);
            for (int i = 0; i < p->roi_count; i++)
            {
                int j = (i * 2) + 1;
                p->region[i].x = int(roi[j] / p->p_scale);
                p->region[i].y = int(roi[j + 1] / p->p_scale);
            }
        }
        else if (p->roi_type == CIRCLE)
        {
            if (p->masking_type == FOUR_POINT_BASE) {
                p->roi_count = 4;
                if (p->match_type == BEST_MATCH ||p->match_type == KNN_MATCH )
                    p->circle_fixedpt_radius = 200;
                else if(p->match_type == SPLIT_MATCH)
                    p->circle_fixedpt_radius = 100;
                    p->circle_fixedpt_radius_2nd = 120;
            }
            else if (p->masking_type == USER_INPUT_CIRCLE)
                p->roi_count = cnt;

            Logger("Initialize circle roi_count %d %d ", cnt, p->roi_count);
            p->circles = (Cr *)g_os_malloc(sizeof(Cr) * p->roi_count);
            for (int i = 0; i < p->roi_count; i++)
            {
                int j = (i * 3) + 1;
                p->circles[i].center.x = int(roi[j] / p->p_scale);
                p->circles[i].center.y = int(roi[j + 1] / p->p_scale);
                p->circles[i].radius = int(roi[j + 2] / p->p_scale);
            }
        }
    }
    else if(p->match_type == PYRAMID_MATCH) {
        if(p->roi_type == RECTANGLE) {
            if(p->calibration_type == RECALIBRATION_3D) 
                p->masking_type = FOUR_POINT_BASE;
            else
                p->masking_type = INNER_2POINT_BASE;
            p->match_type = PYRAMID_MATCH;
            p->roi_count = 4;
            p->pyramid_step = 3;
            p->pyramid_scale[0] = 1;
            p->pyramid_scale[1] = 2;
            p->pyramid_scale[2] = 4;        
            p->pyramid_patch[0] = 101;
            p->pyramid_patch[1] = 51; 
            p->pyramid_patch[2] = 25;
            p->p_scale = 2;           
            p->stride[0] = 3;
            p->stride[1] = 3;            
            p->stride[2] = 3;            
            p->base_kernel = 25;

        }
    }

    if(p->p_scale == 1) { 
        p->blur_ksize = 15;
        p->blur_sigma = 0.9;
        p->desc_byte = 32;
        p->use_ori = true;
        p->nms_k = 17;
        p->fast_k = 21;
        p->minx = 0;

        p->pwidth = 3840;  //4K width
        p->pheight = 2160; //4K height
    } else {

        p->blur_ksize = 7;
        p->blur_sigma = 0.9;
        p->desc_byte = 32;
        p->use_ori = true;
        p->nms_k = 9;
        p->fast_k = 21;
        p->minx = 0;

        p->pwidth = 3840;
        p->pheight = 2160;
    }

    p->sensor_size = 17.30 / 1.35;
    p->focal = 3840;

    p->world = new SCENE();
    // nba 
    
    p->world->four_fpt[0].x = 210.0;
    p->world->four_fpt[0].y = 318.0;
    p->world->four_fpt[1].x = 593.0;
    p->world->four_fpt[1].y = 318.0;
    p->world->four_fpt[2].x = 593.0;
    p->world->four_fpt[2].y = 428.0;
    p->world->four_fpt[3].x = 210.0;
    p->world->four_fpt[3].y = 428.0;
    p->world->center.x = 210.0;
    p->world->center.y = 372.0;
    //football 
    /*
    p->world->four_fpt[0].x = 226.0;
    p->world->four_fpt[0].y = 695.0;
    p->world->four_fpt[1].x = 572.0;
    p->world->four_fpt[1].y = 695.0;
    p->world->four_fpt[2].x = 572.0;
    p->world->four_fpt[2].y = 581.0;
    p->world->four_fpt[3].x = 226.0;
    p->world->four_fpt[3].y = 581.0;
    p->world->center.x = 400.0;
    p->world->center.y = 674.0;  */

    p->world->rod_norm = 100;
    p->camera_matrix = (float *)g_os_malloc(sizeof(float) * 9);
    p->skew_coeff = (float *)g_os_malloc(sizeof(float) * 4);

    p->world->rod_norm = 0;
    //NormalizePoint(p->world, 100);
    p->camera_matrix[0] = p->focal;
    p->camera_matrix[1] = 0;
    p->camera_matrix[2] = p->pwidth / 2;

    p->camera_matrix[3] = 0;
    p->camera_matrix[4] = p->focal;
    p->camera_matrix[5] = p->pheight / 2;

    p->camera_matrix[6] = 0;
    p->camera_matrix[7] = 0;
    p->camera_matrix[8] = 1;

    p->skew_coeff[0] = 0;
    p->skew_coeff[1] = 0;
    p->skew_coeff[2] = 0;
    p->skew_coeff[3] = 0;

    Logger("Data Initalize complete .. ");
}

int Extractor::UpdateConfig()
{
    //read config file and update parameter

    //or Reset
}

void Extractor::NormalizePoint(SCENE *sc, int maxrange)
{

    float minx = sc->four_fpt[0].x;
    float miny = sc->four_fpt[0].y;
    float maxx = sc->four_fpt[0].x;
    float maxy = sc->four_fpt[0].y;

    for (int i = 0; i < 4; i++)
    {
        if (minx > sc->four_fpt[i].x)
            minx = sc->four_fpt[i].x;
        if (miny > sc->four_fpt[i].y)
            miny = sc->four_fpt[i].y;
        if (maxx < sc->four_fpt[i].x)
            maxx = sc->four_fpt[i].x;
        if (maxy < sc->four_fpt[i].y)
            maxy = sc->four_fpt[i].y;
    }

    Logger("min max %f %f %f %f ", minx, miny, maxx, maxy);

    float range = 0;
    float marginx = 0;
    float marginy = 0;

    if ((maxx - minx) > (maxy - miny))
    {
        range = maxrange / (maxx - minx);
        marginy = (maxrange - ((maxy - miny) * range)) / 2.0;
    }
    else
    {
        range = maxrange / (maxy - miny);
        marginx = (maxrange - ((maxx - minx) * range)) / 2.0;
    }

    Logger("range marginx  %f %f %f ", range, marginx, marginy);

    for (int i = 0; i < 4; i++)
    {
        sc->four_fpt[i].x = ((float)sc->four_fpt[i].x - minx) * range + marginx;
        sc->four_fpt[i].y = ((float)sc->four_fpt[i].y - miny) * range + marginy;
    }
    /* 
    sc->normal[0][0] = (400.0 - minx) * range + marginx;
    sc->normal[0][1] = (656.0 - miny) * range + marginy;
    sc->normal[0][2] = 0.0;
    sc->normal[1][0] = (400.0 - minx) * range + marginx;
    sc->normal[1][1] = (656.0 - miny) * range + marginy;
    sc->normal[1][2] = -100.0;
 */

    p->normal[0][0] = 50.0;
    p->normal[0][1] = 50.0;
    p->normal[0][2] = 0.0;
    p->normal[1][0] = 50.0;
    p->normal[1][1] = 50.0;
    p->normal[1][2] = -100.0;

#if _DEBUG
    Logger("Normalized point 4pt");
    for (int i = 0; i < 4; i++)
        Logger(" %f, %f ", sc->four_fpt[i].x, sc->four_fpt[i].y);

    Logger("Normalized point normal.");
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            Logger("[%d][%d]  %f", i, j, p->normal[i][j]);

#endif
}

void Extractor::SaveImage(SCENE *sc, int type)
{
    if (type == 0)
        return;

    Mat img;
    char filename[30] = { 0, };

    if (type == 1)
    {
        drawKeypoints(sc->img, sc->ip, img);
        sprintf(filename, "saved/%d_keypoint.png", sc->id);
    }
    else if (type == 2)
    {
        img = sc->mask_img;
        sprintf(filename, "saved/%d_masking.png", sc->id);
    }
    else if (type == 3)
    {
        drawKeypoints(sc->img, sc->ip, img);
        for (int i = 0; i < 4; i++)
        {
            circle(img,
                    Point((int)cal_group.back().four_fpt[i].x/p->p_scale, (int)cal_group.back().four_fpt[i].y/p->p_scale), 6, Scalar(255), -1);

        }        
        sprintf(filename, "saved/%d_keypoint.png", sc->id);
    }
    else if (type == 4) {
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
                    Point((int)cal_group.back().four_fpt[i].x/p->p_scale, (int)cal_group.back().four_fpt[i].y/p->p_scale), 6, Scalar(255), -1);
        }

        Point2f tp1 = Point2f(cur_query->projected_normal.at<Point2f>(0).x/p->p_scale, 
                cur_query->projected_normal.at<Point2f>(0).y/p->p_scale);

        for (int i = 1 ; i < cur_query->projected_normal.rows; i ++) {
            Logger("projected point %f %f ", cur_query->projected_normal.at<Point2f>(i).x,
                    cur_query->projected_normal.at<Point2f>(i).y);
            Point2f tp2 = Point2f(cur_query->projected_normal.at<Point2f>(i).x/p->p_scale, 
                    cur_query->projected_normal.at<Point2f>(i).y/p->p_scale);

            line(img, tp1, tp2, color[i], 2);
        }

        sprintf(filename, "saved/%d_keypoint+normal.png", sc->id);
    }
    else if (type == 5) {
        img = sc->pyramid[1];
        sprintf(filename, "saved/%d_pyramid_check1.png", sc->id);        
        imwrite(filename, img);        
        img = sc->pyramid[2];
        sprintf(filename, "saved/%d_pyramid_check2.png", sc->id);        

    }
    else if (type == 6) {
        drawKeypoints(sc->pyramid[0], sc->pyramid_ip[0], img);
        sprintf(filename, "saved/%d_pr0_keypoint.png", sc->id);
        imwrite(filename, img);
        drawKeypoints(sc->pyramid[1], sc->pyramid_ip[1], img);
        sprintf(filename, "saved/%d_pr1_keypoint.png", sc->id);
        imwrite(filename, img);
        drawKeypoints(sc->pyramid[2], sc->pyramid_ip[2], img);
        sprintf(filename, "saved/%d_pr2_keypoint.png", sc->id);
    }

    imwrite(filename, img);
}

vector<Mat> Extractor::LoadImages(const string &path)
{
    const int FK = 3500;
    const int FHD = 1900;

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
        dsc_id.push_back(dsc);
        Logger("Read image : %s , desc_id %s ", ip.c_str(), dsc.c_str());
    }
    return images;
}
int Extractor::Execute() {

    int index = 0;
    int ret = -1;
    int step = 1; int start = 0;
    TIMER* all;
    all = new TIMER();    
    StartTimer(all);
    
    if(p->match_type == PYRAMID_MATCH) {
        step = 2;
        start = 0;
    }

    for (int i = start ; i < imgs.size(); i += step)
    {
        StartTimer(t);
        SCENE sc;
        sc.id = index;
        sc.ori_img = imgs.front();
        ProcessImages(&sc);

        if (index == 0)
        {
            sc.id = 0;
            //nba 30096_10            
            sc.four_fpt[0].x = 1688.0;
            sc.four_fpt[0].y = 1058.0;
            sc.four_fpt[1].x = 2456.0;
            sc.four_fpt[1].y = 473.0;
            sc.four_fpt[2].x = 2831.0;
            sc.four_fpt[2].y = 510.0;
            sc.four_fpt[3].x = 2165.0;
            sc.four_fpt[3].y = 1128.0;
            sc.center.x = 1906.0;
            sc.center.y = 1092.0; 
            //football
            /*
            sc.four_fpt[0].x = 1275.0;
            sc.four_fpt[0].y = 165.0;
            sc.four_fpt[1].x = 1212.0;
            sc.four_fpt[1].y = 1560.0;
            sc.four_fpt[2].x = 3273.0;
            sc.four_fpt[2].y = 1461.0;
            sc.four_fpt[3].x = 2364.0;
            sc.four_fpt[3].y = 148.0;
            sc.center.x = 1470.0;
            sc.center.y = 630.0; */
        }

        if(p->match_type != PYRAMID_MATCH) {
            ImageMasking(&sc);
            ret = GetFeature(&sc);
            Logger("[%d] feature extracting  %f ", index, LapTimer(t));
        }
        else {
            if(sc.id == 0)
                CreateFeature(&sc, false, true);
            //else 
                //CreateFeature(&sc, false, true);
        }

        if (sc.id == 0) {
            cal_group.push_back(sc);
            SetCurTrainScene(p->world);
            SetCurQueryScene(&cal_group[index]);
        }
        else {
            cal_group.push_back(sc);
            SetCurTrainScene(&cal_group[index - 1]);
            SetCurQueryScene(&cal_group[index]);
        }
        continue;
        ret = Match();
        Logger("return from FindHomography------  %d", ret);
        Logger("[%d] match consuming %f ", index, LapTimer(t));        

        if (ret > 0) {
            PostProcess();
#ifdef _IMGDEBUG
            SaveImage(&sc, 4);
#endif            
        }
        else {
            Logger("Match Pair Fail. Can't keep going.");
        }

        Logger("------- [%d] end  consuming %f ", index, LapTimer(t));

        index++;
        if (index == 1)
            break;
    }

    //Export result to josn file
    //genutil.Export(dsc_id, cal_group, p);
    //genutil.ExportforApp(dsc_id, cal_group, p);
    Logger("All process time..  %f ", LapTimer(all));

    return ERR_NONE;
}

//Mat Extractor::ProcessImages(Mat &img)
int Extractor::ProcessImages(SCENE* sc) {
    if(p->match_type != PYRAMID_MATCH) {
        Mat blur_img;
        Mat dst;
        if (p->p_scale != 1)    {
            resize(sc->ori_img, dst, Size(int(sc->ori_img.cols / p->p_scale), 
                int(sc->ori_img.rows / p->p_scale)), 0, 0, 1);
        }
        cvtColor(dst, blur_img, cv::COLOR_RGBA2GRAY);
        normalize(blur_img, dst, 0, 255, NORM_MINMAX, -1, noArray());
        GaussianBlur(dst, blur_img, {p->blur_ksize, p->blur_ksize}, p->blur_sigma, p->blur_sigma);

        sc->img = blur_img;
    }
    else {
        Logger("Process Image start step %d ", p->pyramid_step);        
        for(int i = 0 ; i < p->pyramid_step; i ++) {
            Mat blur_img;
            Mat dst;
            if(p->pyramid_scale[i] != 1) 
                resize(sc->ori_img, dst, Size(int(sc->ori_img.cols / p->pyramid_scale[i]), 
                    int(sc->ori_img.rows / p->pyramid_scale[i])), 0, 0, 1);
            else dst = sc->ori_img;
            cvtColor(dst, blur_img, cv::COLOR_RGBA2GRAY);
            normalize(blur_img, dst, 0, 255, NORM_MINMAX, -1, noArray());
            GaussianBlur(dst, blur_img, {p->blur_ksize, p->blur_ksize}, p->blur_sigma, p->blur_sigma);
            sc->pyramid[i] = blur_img;
        }
        SaveImage(sc, 5);
    }

    return ERR_NONE;
}

int Extractor::ImageMasking(SCENE* sc)
{
    Logger("Image masking function start ");
    Mat mask = Mat::zeros(sc->img.rows, sc->img.cols, CV_8UC1);

    if (p->masking_type == FOUR_POINT_BASE)
    {
        for (int i = 0; i < 4; i++)
        {
            if (sc->id == 0) {
                Logger("masking point 1 %f %f ", sc->four_fpt[i].x, sc->four_fpt[i].y);
                circle(mask, Point((int)sc->four_fpt[i].x/p->p_scale, (int)sc->four_fpt[i].y/p->p_scale),
                       int(p->circle_fixedpt_radius), Scalar(255), -1);
            }
            else {
                Logger("masking point 2 %f %f ", cal_group.back().four_fpt[i].x, cal_group.back().four_fpt[i].y);
                circle(mask,
                       Point((int)cal_group.back().four_fpt[i].x/p->p_scale, (int)cal_group.back().four_fpt[i].y/p->p_scale),
                       int(p->circle_fixedpt_radius), Scalar(255), -1);
            }
        }
    }
    else if (p->masking_type == USER_INPUT_CIRCLE) {
        for (int i = 0; i < p->roi_count; i++) {
            Logger("masking point 3 %d %d %d ", p->circles[i].center.x, p->circles[i].center.y, p->circles[i].radius);
            circle(mask,
                   Point(p->circles[i].center.x, p->circles[i].center.y),
                   p->circles[i].radius, Scalar(255), -1);
        }
    }

    sc->mask_img = mask;

#if defined _IMGDEBUG
//    SaveImage(sc, 2);
#endif
    
}

int Extractor::GetFeature(SCENE *sc) {
    // FAST + BRIEF
    //auto feature_detector = FastFeatureDetector::create(p->fast_k, true, FastFeatureDetector::TYPE_9_16);

    //auto feature_detector = AgastFeatureDetector::create(); //AGAST
    //Ptr<xfeatures2d::MSDDetector>feature_detector = xfeatures2d::MSDDetector::create(9,11,15); //MSDETECTOR
    //Ptr<xfeatures2d::BriefDescriptorExtractor> dscr;
    //dscr = xfeatures2d::BriefDescriptorExtractor::create(p->desc_byte, p->use_ori);

    //AKAZE
    //auto feature_detector = AKAZE::create(false, false, 0.001f, 4, 1, KAZE::DIFF_PM_G1); //KAZE
    auto feature_detector = AKAZE::create();
    Ptr<AKAZE> dscr = feature_detector;

    Mat desc;
    vector<KeyPoint> kpt;
    vector<KeyPoint> f_kpt;

    feature_detector->detect(sc->img, kpt, sc->mask_img);
    Logger("extracted keypoints count : %d", kpt.size());
    //f_kpt = KeypointMasking(&kpt);
    dscr->compute(sc->img, kpt, desc);

    sc->ip = kpt;
    sc->desc = desc;

#if _DEBUG
/*     for (int i = 0 ; i < sc->ip.size(); i ++) {
        Logger("Keypoint index %2d id %3d, x %f y %f ",i, sc->ip[i].class_id, 
                            sc->ip[i].pt.x, sc->ip[i].pt.y);
    }
 */
#endif

    return ERR_NONE;
}

int Extractor::CreateFeature(SCENE* sc, bool train, bool query) {
    float rk = 0;
    Ptr<xfeatures2d::BriefDescriptorExtractor> dscr;
    dscr = xfeatures2d::BriefDescriptorExtractor::create(p->desc_byte, false);    
    Mat desc;
    vector<KeyPoint> kpt;

    if(train) {
        for(int i = p->pyramid_step -1; i >= 0; i --){
            int scl = p->pyramid_scale[i];            
            for(int j = 0 ; j < p->roi_count; j++) {
                float cen_x = float(sc->four_fpt[j].x)/scl;
                float cen_y = float(sc->four_fpt[j].y)/scl;
                Logger("step %d roicnt %d scl %d ", i, j, scl);                                
                if(i == 2) {
                    Logger("fpt %4.2f %4.2f cen %4.2f %4.2f ", sc->four_fpt[j].x, sc->four_fpt[j].y, 
                            cen_x, cen_y);
                    KeyPoint kp = KeyPoint(cen_x, cen_y, p->base_kernel, -1, 0, 0, j);
                    sc->pyramid_ip[i].push_back(kp);
                }
                else if( i == 1) {
                    rk = p->base_kernel / 2;
                    for(int a = cen_x - rk ; a <= cen_x + rk ; a += rk) {
                        for(int b = cen_y - rk ; b <= cen_y + rk ; b += rk) {
                            KeyPoint kp = KeyPoint(float(a), float(b), rk, -1, 0, 0, j);
                            sc->pyramid_ip[i].push_back(kp);
                            Logger("2nd push ip %4.2f %4.2f size %3.2f ", float(a), float(b), rk);
                        }
                    }
                }
                else if(i == 0) {
                    rk = p->base_kernel;                    
                    for(int a = cen_x - rk ; a <= cen_x + rk ; a += rk) {
                        for(int b = cen_y - rk ; b <= cen_y + rk ; b += rk) {
                            KeyPoint kp = KeyPoint(float(a), float(b), rk, -1, 0, 0, j);
                            sc->pyramid_ip[i].push_back(kp);                        
                            Logger("3rd push ip %4.2f %4.2f size %3.2f ", float(a), float(b), rk);
                        }
                    }
                }
            }
        }
        SaveImage(sc, 6);
        Logger("train kpt size %d %d %d ", sc->pyramid_ip[0].size(), sc->pyramid_ip[1].size(), sc->pyramid_ip[2].size());
    }
    else if (query) {
        for(int i = p->pyramid_step -1; i >= 0; i --){
            int scl = p->pyramid_scale[i];            
            for(int j = 0 ; j < p->roi_count; j++) {
                float cen_x = float(sc->four_fpt[j].x)/scl;
                float cen_y = float(sc->four_fpt[j].y)/scl;                
                Logger("step %d roicnt %d scl %d ", i, j, scl);                
                if(i == 2) {
                    rk = p->base_kernel / 2;
                    Logger("fpt %4.2f %4.2f cen %4.2f %4.2f ", sc->four_fpt[j].x, sc->four_fpt[j].y, 
                            cen_x, cen_y);
                    for(int a = cen_x - rk; a < cen_x + rk ; a += 3) {
                        for(int b = cen_y - rk; b < cen_y + rk; b += 3) {
                            KeyPoint kp = KeyPoint(float(a), float(b), p->base_kernel, -1, 0, 0, j);
                            sc->pyramid_ip[i].push_back(kp);
                        }
                    }
                }
                else if( i == 1) {
                    rk = p->base_kernel;
                    for(int a = cen_x - rk ; a <= cen_x + rk ; a += 5) {
                        for(int b = cen_y - rk ; b <= cen_y + rk ; b += 5) {
                            KeyPoint kp = KeyPoint(float(a), float(b), rk, -1, 0, 0, j);
                            sc->pyramid_ip[i].push_back(kp);
                            Logger("2nd push ip %4.2f %4.2f size %3.2f ", float(a), float(b), rk);
                        }
                    }
                }
                else if(i == 0) {
                    rk = p->base_kernel * 2;                    
                    for(int a = cen_x - rk ; a <= cen_x + rk ; a += 11) {
                        for(int b = cen_y - rk ; b <= cen_y + rk ; b += 9) {
                            KeyPoint kp = KeyPoint(float(a), float(b), rk, -1, 0, 0, j);
                            sc->pyramid_ip[i].push_back(kp);                        
                            Logger("3rd push ip %4.2f %4.2f size %3.2f ", float(a), float(b), rk);
                        }
                    }
                }
            }
        }
        SaveImage(sc, 6);
        Logger("query kpt size %d %d %d ", sc->pyramid_ip[0].size(), sc->pyramid_ip[1].size(), sc->pyramid_ip[2].size());        
    }


    //dscr->compute(sc->img, kpt, desc);

    //sc->ip = kpt;
    //sc->desc = desc;

}

vector<KeyPoint> Extractor::KeypointMasking(vector<KeyPoint> *oip)
{
    vector<KeyPoint> ip;
    //Logger("Before masking %d ", oip->size());
    int left = 0;
    int total = 0;
    int del = 0;

    for (int i = 0; i < p->roi_count; i++)
    {
        Logger("roi check %d %d ", p->moved_region[i].x, p->moved_region[i].y);
    }

    for (auto it = oip->begin(); it != oip->end(); it++)
    {
        total++;
        Pt cp(int(it->pt.x), int(it->pt.y));
        int ret = isInside(p->moved_region, p->roi_count, cp);

        if (ret == 0 && it != oip->end())
        {
            del++;
        }
        else if (it->pt.x >= 1760 && it->pt.y >= 930 &&
                 it->pt.x <= 2070 && it->pt.y <= 1190)
            del++;
        else
        {
            ip.push_back(*it);
            left++;
        }
    }

    Logger("new vector  %d. left %d  del %d / total ip %d ", ip.size(), left, del, total);
    return ip;
}

int Extractor::Match() {

    int ret = -1;

    if (cur_query->id == 0) {
        ret = FindBaseCoordfromWd();
    } else {
        ret = MatchPlain();
    }

    return ret;
}

int Extractor::MatchPlain() {

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    vector<DMatch> matches;
    vector<DMatch> good;

    if (cur_train->desc.type() != CV_32F || cur_query->desc.type() != CV_32F)
    {
        cur_train->desc.convertTo(cur_train->desc, CV_32F);
        cur_query->desc.convertTo(cur_query->desc, CV_32F);
    }
    Logger("Match start %d %d ", cur_train->ip.size(), cur_query->ip.size());

    if(p->match_type == KNN_MATCH) {
        vector<vector<DMatch>> in;        
        const float ratio_thresh = 0.90f;        
        matcher->knnMatch(cur_query->desc, cur_train->desc, in, 2); //knn matcher
        for( int i = 0 ; i < in.size(); i++) {
            if(in[i][0].distance < ratio_thresh * in[i][1].distance) {
                good.push_back(in[i][0]);
            }
        }

    } else if (p->match_type == BEST_MATCH || p->match_type == SPLIT_MATCH) {
        matcher->match(cur_query->desc, cur_train->desc, good); //normal mathcer
    }

    sort(good.begin(), good.end());
    Logger("First matche size %d ", good.size());

    matches = RefineMatch(good);
    
    if (matches.size() < 5 ) {
        return -1; 
    }

    vector<Point2f> train_pt, query_pt;
    vector<Point2f> scaled_train_pt, scaled_query_pt;

    for (vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); it++)
    {
        float tx = cur_train->ip[it->trainIdx].pt.x;
        float ty = cur_train->ip[it->trainIdx].pt.y;

        float qx = cur_query->ip[it->queryIdx].pt.x;
        float qy = cur_query->ip[it->queryIdx].pt.y;

        //        Logger("_pt push %f %f %f %f ", tx, ty, qx, qy);

        if ((tx > 0 && ty > 0) && (tx < cur_train->img.cols && ty < cur_train->img.rows) &&
            (qx > 0 && qy > 0) && (qx < cur_query->img.cols && qy < cur_query->img.rows))
        {
            if (p->p_scale != 1)
            {
                scaled_train_pt.push_back(Point2f(tx, ty));
                scaled_query_pt.push_back(Point2f(qx, qy));
                train_pt.push_back(Point2f(tx * p->p_scale, ty * p->p_scale));
                query_pt.push_back(Point2f(qx * p->p_scale, qy * p->p_scale));
            }
            else
            {
                train_pt.push_back(Point2f(tx, ty));
                query_pt.push_back(Point2f(qx, qy));
            }
        }
    }

    if(p->match_type == KNN_MATCH  || p->match_type == BEST_MATCH) {
        Mat _h = findHomography(train_pt, query_pt, FM_RANSAC);
        //Mat _h = getAffineTransform(query_pt, train_pt);
        //Mat _h = estimateAffine2D(train_pt, query_pt);
        //Mat _h = estimateRigidTransform(query_pt, train_pt, false);
        cur_query->matrix_fromimg = _h;

        if (p->p_scale != 1)
        {
            Mat sc_h = findHomography(scaled_train_pt, scaled_query_pt, FM_RANSAC);
            cur_query->matrix_scaledfromimg = sc_h;
        }
#if defined _DEBUG
        static int fi = 0;
        Mat outputImg = cur_train->img.clone();
        drawMatches(cur_query->img, cur_query->ip, cur_train->img, cur_train->ip,
                    matches, outputImg, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        char filename[30] = {
            0,
        };
        sprintf(filename, "saved/%2d_match.png", fi);
        imwrite(filename, outputImg);
        fi++;
        
#endif        
    } else if (p->match_type == SPLIT_MATCH) {

#if defined _DEBUG
        static int fi2 = 0;
        Mat outputImg = cur_train->img.clone();
        drawMatches(cur_query->img, cur_query->ip, cur_train->img, cur_train->ip,
                    matches, outputImg, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        char filename[40] = {
            0,
        };
        sprintf(filename, "saved/%2d_match_splitbefore.png", fi2);
        imwrite(filename, outputImg);
        fi2++;
        
#endif        
        MatchSplit(train_pt, query_pt);
    }


    return matches.size();
}

int Extractor::MatchSplit(vector<Point2f> m_train, vector<Point2f>m_query) {

    Logger(" Split Match Start! m_train size %d %d   ", m_train.size(), m_query.size());
    vector<Point2f> mr_train[4];
    vector<Point2f> mr_query[4];
    int count[4] = { 0, };    

    float range = (float)p->circle_fixedpt_radius * 1.2;

    for(int i = 0 ; i < m_train.size(); i ++) {
        int index = -1;
        for(int j = 0 ; j < 4 ; j ++) {
//            Logger("value check %f %f ", abs(cur_train->four_fpt[j].x - m_train[i].x), 
//                abs(cur_train->four_fpt[j].y - m_train[i].y));

            if ( abs(cur_train->four_fpt[j].x - m_train[i].x) < range &&
                    abs(cur_train->four_fpt[j].y - m_train[i].y) < range) {
                    index = j;
                    break;
            }
        }

        if(index >= 0) {
            mr_train[index].push_back(Point2f(m_train[i].x, m_train[i].y));
            mr_query[index].push_back(Point2f(m_query[i].x, m_query[i].y));            
            count[index]++;            
//            Logger("train %f %f query %f %f insert to index %d (%f %f) count %d", m_train[i].x, m_train[i].y,
//            m_query[i].x, m_query[i].y, index, cur_train->four_fpt[index].x, cur_train->four_fpt[index].y, //count[index]); 

        } /*else {
            Logger("Didn't belong to any range..%f %f ", m_train[i].x, m_train[i].y);
        } */
    }
    Logger("region match count %d %d %d %d ", count[0], count[1], count[2], count[3]);

    int max_index = -1;
    int max_val = 0;
    for(int i = 0; i < 4; i++) {
        if(count[i] > max_val) {
            max_index = i;
            max_val = count[i];
        }
    }

    if(max_val < 6) {
        Logger("Can't find any available answer..");
        return -1;
    } else 
        Logger("max value index %d, score %d", max_index, max_val);


    for(int k = 0 ; k < 4 ; k ++) {
        Mat _h;
        if(count[k] > 6) {
            //Mat _h = estimateRigidTransform(mr_train[k], mr_query[k], true);
            _h = estimateAffine2D(mr_train[k], mr_query[k]);            
            FPt newpt = mtrx.TransformPtbyAffine(cur_train->four_fpt[k], _h);
            Logger("Split match point move[%d] %f %f -> %f %f ", k, cur_train->four_fpt[k].x, 
                    cur_train->four_fpt[k].y, newpt.x, newpt.y);
            cur_query->four_fpt[k].x = newpt.x;
            cur_query->four_fpt[k].y = newpt.y;        
        }
        else {
            Logger("Point is not enough..- max_index value apply ");
            //Mat _h = estimateRigidTransform(mr_train[max_index], mr_query[max_index], true);                
            _h = estimateAffine2D(mr_train[max_index], mr_query[max_index]);
            FPt newpt = mtrx.TransformPtbyAffine(cur_train->four_fpt[k], _h);
            Logger("Split match point move[%d] %f %f -> %f %f ", k, cur_train->four_fpt[k].x, 
                    cur_train->four_fpt[k].y, newpt.x, newpt.y);
            cur_query->four_fpt[k].x = newpt.x;
            cur_query->four_fpt[k].y = newpt.y;        

        }

        float ncc_ = ncc(k, _h);
    }

    //Mat _h = estimateRigidTransform(mr_train[max_index], mr_query[max_index], true);
    /*
    Mat _h = estimateAffine2D(mr_train[max_index], mr_query[max_index]);    
    FPt newpt = mtrx.TransformPtbyAffine(cur_train->four_fpt[max_index], _h);
    Logger("Split match point move[%d] %f %f -> %f %f ", max_index, cur_train->four_fpt[max_index].x, 
            cur_train->four_fpt[max_index].y, newpt.x, newpt.y);
    cur_query->four_fpt[max_index].x = newpt.x;
    cur_query->four_fpt[max_index].y = newpt.y;        

    if( ncc_ >= 0.7) {
        Logger("Homography OK. apply another point.. ");
        for(int k = 0 ; k < 4 ; k ++) {
            if(k != max_index) {
                FPt newpt = mtrx.TransformPtbyAffine(cur_train->four_fpt[k], _h);
                cur_query->four_fpt[k].x = newpt.x;
                cur_query->four_fpt[k].y = newpt.y;        
            }
        }
    }
    else {
        Logger("Homography fail. Try to searching again");
    }
 */
}

int Extractor::MatchVerify() {


}

float Extractor::ncc(int max_index, Mat _h) {
    int minx_t, miny_t, maxx_t, maxy_t;
    int minx_q, miny_q, maxx_q, maxy_q;    
    int width_t, width_q, height_t, height_q;
    int width, height;
    int pindex = max_index;

    if (cur_train->four_fpt[pindex].x - p->circle_fixedpt_radius < 0 )
        minx_t = 0;
    else 
        minx_t = (cur_train->four_fpt[pindex].x - p->circle_fixedpt_radius) / p->p_scale;

    if (cur_train->four_fpt[pindex].x + p->circle_fixedpt_radius > p->pwidth) 
        maxx_t = p->pwidth / p->p_scale;
    else
        maxx_t = (cur_train->four_fpt[pindex].x + p->circle_fixedpt_radius) / p->p_scale;
    
    if (cur_train->four_fpt[pindex].y - p->circle_fixedpt_radius < 0 )
        miny_t = 0;
    else 
        miny_t = (cur_train->four_fpt[pindex].y - p->circle_fixedpt_radius)/p->p_scale;
    
    if (cur_train->four_fpt[pindex].y + p->circle_fixedpt_radius > p->pheight)
        maxy_t = p->pheight / p->p_scale;
    else
        maxy_t = (cur_train->four_fpt[pindex].y + p->circle_fixedpt_radius)/p->p_scale;


    if (cur_query->four_fpt[pindex].x - p->circle_fixedpt_radius < 0 )
        minx_q = 0;
    else 
        minx_q = (cur_query->four_fpt[pindex].x - p->circle_fixedpt_radius) / p->p_scale;

    if (cur_query->four_fpt[pindex].x + p->circle_fixedpt_radius > p->pwidth) 
        maxx_q = p->pwidth / p->p_scale;
    else
        maxx_q = (cur_query->four_fpt[pindex].x + p->circle_fixedpt_radius) / p->p_scale;
    
    if (cur_query->four_fpt[pindex].y - p->circle_fixedpt_radius < 0 )
        miny_q = 0;
    else 
        miny_q = (cur_query->four_fpt[pindex].y - p->circle_fixedpt_radius)/p->p_scale;
    
    if (cur_query->four_fpt[pindex].y + p->circle_fixedpt_radius > p->pheight)
        maxy_q = p->pheight / p->p_scale;
    else
        maxy_q = (cur_query->four_fpt[pindex].y + p->circle_fixedpt_radius)/p->p_scale;

    width_t = maxx_t - minx_t;
    width_q = maxx_q - minx_q;
    height_t = maxy_t - miny_t;
    height_q = maxy_q - miny_q;

    if( width_t == width_q)
        width = width_t;
    else
        width = min(width_t, width_q);

    if( height_t == height_q )    
        height = height_t;
    else
        height = min(height_t, height_q);


    Mat sc1 = cur_train->img(Rect(minx_t, miny_t, width, height));
    Mat sc2 = cur_query->img(Rect(minx_q, miny_q, width, height));


    //image debug
    static int s_index = 0;
    char filename[40];
    sprintf(filename, "saved/patch_%d_t.png", s_index);
    imwrite(filename, sc1);
    sprintf(filename, "saved/patch_%d_q.png", s_index);    
    imwrite(filename, sc2);
    s_index++;
    
    vector<uchar> patch1;
    patch1.assign(sc1.data, sc1.data + sc1.total()*sc1.channels());

    vector<uchar> patch2;
    patch2.assign(sc2.data, sc2.data + sc2.total()* sc2.channels());

    int tcnt = (p->circle_fixedpt_radius) * (p->circle_fixedpt_radius); //area of rectangle patch
    double m1, m2, s1, s2, s12, denom, r;
    m1 = 0; m2 = 0;
    for ( int i = 0; i < tcnt ; i ++) {
        m1 += patch1[i];
        m2 += patch2[i];
    }
    m1 /= tcnt;
    m2 /= tcnt;

    s1 = 0; s2 = 0;
    for(int i = 0; i < tcnt; i ++) {
        s1 += (patch1[i] - m1) * (patch1[i] - m1);
        s2 += (patch2[i] - m2) * (patch2[i] - m2);
    }
    denom = sqrt(s1 * s2);
    s12 = 0;
    for(int i = 0 ; i < tcnt; i ++) {
        s12 += (patch1[i] - m1) * (patch2[i] - m2);
    }
    r = s12 / denom;

    Logger("Cross correl .. m1 %f m2 %f s1 %f s2 %f s12 %f denom %f tcnt %d -> %f", m1, m2, s1, s2, s12, denom, tcnt, r);
    return r;

}

vector<DMatch> Extractor::RefineMatch(vector<DMatch> good) {

    vector<DMatch>matches;
    vector<DMatch>last;

    int *t_hist = (int *)g_os_malloc(sizeof(int) * cur_train->ip.size());
    int *q_hist = (int *)g_os_malloc(sizeof(int) * cur_query->ip.size());
    for (int a = 0; a < cur_train->ip.size(); a++)
        t_hist[a] = 0;
    for (int b = 0; b < cur_query->ip.size(); b++)
        q_hist[b] = 0;

    for (int t = 0; t < good.size(); t++)
    {
        if (t_hist[good[t].trainIdx] == 0 && q_hist[good[t].queryIdx] == 0)
        {
            matches.push_back(good[t]);
            t_hist[good[t].trainIdx]++;
            q_hist[good[t].queryIdx]++;
        }
//        else
//            Logger("double check %d %d ", good[t].trainIdx, good[t].queryIdx);
    }

    g_os_free(t_hist);
    g_os_free(q_hist);

    last = RemoveOutlier(matches);
    if(p->match_type != SPLIT_MATCH) {

        if (last.size() > 100) {
            while (last.size() >= 100) {
                last.pop_back();
            }
            Logger("matches->pop_back size %d ", last.size());
        }
    }
    return last;

};

vector<DMatch> Extractor::RemoveOutlier(vector<DMatch> matches) {

    vector<DMatch> result;
    Logger("Remove Outlier is called %d ", matches.size());
    double covar_deg = 0;
    double covar_dist = 0;
    double t_covar_deg = 0;        
    double t_covar_dist = 0;
    double degsum = 0;
    double dist_sum = 0;
    double degavg = 0;
    double distavg = 0;

    for( vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); it++) {
        float tx = cur_train->ip[it->trainIdx].pt.x;
        float ty = cur_train->ip[it->trainIdx].pt.y;
        float qx = cur_query->ip[it->queryIdx].pt.x;
        float qy = cur_query->ip[it->queryIdx].pt.y;
        float dx = qx - tx;
        float dy = qy - ty;

        dist_sum += sqrt( dx * dx + dy * dy );
        float orideg = fastAtan2( dx, dy);
        if (orideg > 180 )
            orideg -= 180;
        degsum += orideg;
//        Logger(" diff %f %f ", sqrt( dx * dx  + dy * dy ), orideg);
    }


    distavg = dist_sum / (float)matches.size();
    degavg = degsum / (float)matches.size();

    for(vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); it++) {
        float tx = cur_train->ip[it->trainIdx].pt.x;
        float ty = cur_train->ip[it->trainIdx].pt.y;
        float qx = cur_query->ip[it->queryIdx].pt.x;
        float qy = cur_query->ip[it->queryIdx].pt.y;
        float dx = qx - tx;
        float dy = qy - ty;

        t_covar_dist += (sqrt( dx * dx + dy * dy)  - distavg) * (sqrt( dx * dx + dy * dy ) - distavg);

        float orideg = fastAtan2( dx, dy);
        if (orideg > 180 )
            orideg -= 180;

        t_covar_deg += (orideg - degavg)*(orideg - degavg);
    }

    covar_dist = sqrt(t_covar_dist/(float)matches.size());
    covar_deg = sqrt(t_covar_deg/(float)matches.size());    

    Logger("covar %f %f threshold %f %f  ", covar_deg, covar_dist);

    for(vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); it++) {
        float tx = cur_train->ip[it->trainIdx].pt.x;
        float ty = cur_train->ip[it->trainIdx].pt.y;
        float qx = cur_query->ip[it->queryIdx].pt.x;
        float qy = cur_query->ip[it->queryIdx].pt.y;
        float dx = qx - tx;
        float dy = qy - ty;

        float mh_distance_dist = abs(sqrt(dx * dx + dy * dy) - distavg )/covar_dist;
        float orideg = fastAtan2( dx, dy);
        if (orideg > 180 )
            orideg -= 180;

        float mh_distance_deg = abs(orideg - degavg)/covar_deg;

        //Logger(" mh distance %f -> %f  angle %f -> %f ", sqrt(dx * dx + dy * dy), mh_distance_dist, orideg, mh_distance_deg);

        if( mh_distance_deg >= 1.0 || mh_distance_dist >= 1.0)
            //Logger("mh distance is over limit %f %f ", mh_distance_deg, mh_distance_dist);
            continue;
        else
           result.push_back(*it);
    }

    return result;
}

int Extractor::PostProcess() {

    if (cur_query->id == 0) {
        FindBaseCoordfromWd(NORMAL_VECTOR_CAL);
        return ERR_NONE;
    }
    Logger(" Post Process start.. ");

    if (p->match_type == SPLIT_MATCH) {
        FindBaseCoordfromWd(NORMAL_VECTOR_CAL);    
        return ERR_NONE;
    }

    float err = 0;
    //move centerpoint
    FPt newcen = mtrx.TransformPtbyHomography(cur_train->center, cur_query->matrix_fromimg);
    Logger("Query center answer(%f, %f) - (%f, %f)", cur_query->center.x, cur_query->center.y, newcen.x, newcen.y);
    cur_query->center = newcen;

    //move 4point
    for (int i = 0; i < p->roi_count; i++)
    {
        FPt newpt = mtrx.TransformPtbyHomography(cur_train->four_fpt[i], cur_query->matrix_fromimg);

        Logger(" four pt move [%d] answer (%f, %f) - (%f, %f) ", i,
               cur_query->four_fpt[i].x, cur_query->four_fpt[i].y,
               newpt.x, newpt.y);
        cur_query->four_fpt[i].x = newpt.x;
        cur_query->four_fpt[i].y = newpt.y;
    }

    FindBaseCoordfromWd(NORMAL_VECTOR_CAL);

    //move user circle input
    if (p->roi_type == CIRCLE && p->masking_type == USER_INPUT_CIRCLE)
    {
        Mat apply_homo;
        if (p->p_scale != 1)
            apply_homo = cur_query->matrix_scaledfromimg;
        else
            apply_homo = cur_query->matrix_fromimg;

        for (int i = 0; i < p->roi_count; i++)  {
            Pt newpt = mtrx.TransformPtbyHomography(&p->circles[i].center, apply_homo);
            p->circles[i].center = newpt;
        }
    }
}

int Extractor::FindBaseCoordfromWd(int mode)
{
    Logger("FindBaseCoordfromW start ");
    Mat cm(3, 3, CV_32F, p->camera_matrix);
    Mat sc(4, 1, CV_32F, p->skew_coeff);
    Mat ppset1(4, 3, CV_32F);
    Mat ppset2(4, 2, CV_32F);
    for (int i = 0; i < 4; i++)
    {
        ppset1.at<float>(i, 0) = p->world->four_fpt[i].x;
        ppset1.at<float>(i, 1) = p->world->four_fpt[i].y;
        ppset1.at<float>(i, 2) = p->world->four_fpt[i].z;

        ppset2.at<float>(i, 0) = (float)cur_query->four_fpt[i].x;
        ppset2.at<float>(i, 1) = (float)cur_query->four_fpt[i].y;

    }

    Mat ret1(3, 1, CV_32F);
    Mat ret2(3, 1, CV_32F);

    bool result = solvePnP(ppset1, ppset2,
                        cm, sc,
                        ret1, ret2,
                        false, SOLVEPNP_ITERATIVE);

    cur_query->rot_matrix = ret1;
    cur_query->trans_matrix = ret2;

    Logger("solve pnp is done result %d :  %d %d %d %d ", result,
        ret1.cols, ret1.rows, ret2.cols, ret2.rows);
    for (int i = 0; i < cur_query->rot_matrix.rows; i++)
        for (int j = 0; j < cur_query->rot_matrix.cols; j++)
            Logger("[%d][%d] %f ", i, j, cur_query->rot_matrix.at<float>(i, j));

    Logger(" ---- ");
    for (int i = 0; i < cur_query->trans_matrix.rows; i++)
        for (int j = 0; j < cur_query->trans_matrix.cols; j++)
            Logger("[%d][%d] %f ", i, j, cur_query->trans_matrix.at<float>(i, j));

    Mat projectedNormal;
    float v_normal[6][3];
    float loca_x = 50.0;
    float loca_y = 50.0;        
    float nlen = 10;
    v_normal[0][0] = loca_x;
    v_normal[0][1] = loca_y;
    v_normal[0][2] = 0.0;

    v_normal[1][0] = loca_x + nlen;
    v_normal[1][1] = loca_y;
    v_normal[1][2] = 0;

    v_normal[2][0] = loca_x;
    v_normal[2][1] = loca_y + nlen;
    v_normal[2][2] = 0;

    v_normal[3][0] = loca_x;
    v_normal[3][1] = loca_y;
    v_normal[3][2] = -nlen * 3;

    v_normal[4][0] = loca_x - nlen;
    v_normal[4][1] = loca_y;
    v_normal[4][2] = 0;

    v_normal[5][0] = loca_x;
    v_normal[5][1] = loca_y - nlen;
    v_normal[5][2] = 0;

    Mat tvec(6, 3, CV_32F, v_normal);
    projectPoints(tvec, cur_query->rot_matrix, cur_query->trans_matrix,
        cm, sc, projectedNormal);

    cur_query->projected_normal = projectedNormal.clone();

    Point2f tp1 = Point2f(cur_query->projected_normal.at<Point2f>(0).x, 
                cur_query->projected_normal.at<Point2f>(0).y);

    Point2f tp2 = Point2f(cur_query->projected_normal.at<Point2f>(3).x, 
            cur_query->projected_normal.at<Point2f>(3).y);

    double distance = norm(tp2 - tp1);
    double degree = fastAtan2(tp1.y - tp2.y, tp1.x - tp2.x);
    degree = 90 - degree; // skew ratio of vector basd 90deg normal vector
    degree = 360 - degree; // inverse rotation for image
    Logger(" tp1 %f %f tp2 %f %f dx %f dy %f ", tp1.x, tp1.y, tp2.x, tp2.y, tp1.x-tp2.x, tp1.y - tp2.y);
    cur_query->rod_norm = distance;
    cur_query->rod_degree = degree;
    double scale = cur_train->rod_norm / cur_query->rod_norm;    
    Logger("normal vector norm %f degree %f scale %f ", distance, degree, scale);


    if (cur_query->id == 0)
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, 1);
    else
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, scale);

#if 0
    Point2f angle_vec = Point2f(cur_query->normal_vec[1].x - cur_query->normal_vec[0].x,
                                cur_query->normal_vec[1].y - cur_query->normal_vec[0].y);

    double degree = fastAtan2(angle_vec.x, angle_vec.y);
    double dnorm = norm(cur_query->normal_vec[0] - cur_query->normal_vec[1]);

    if (cur_query->normal_vec[1].y > cur_query->normal_vec[0].y)
    {
        degree = 360.0 - degree;
        if (degree >= 360)
            degree -= 360;
    }
    else
    {
        degree = 180.0 - degree;
    }

    cur_query->rod_norm = dnorm;
    cur_query->rod_degree = degree;

    float diffnorm = 0;
    if (cur_query->id == 0)
        diffnorm = 1;
    else
    {
        diffnorm = cal_group[0].rod_norm / cur_query->rod_norm;
        Logger("second train norm %f diff norm %f ", cal_group[0].rod_norm, diffnorm);
    }

    Logger("dnorm %f degree %f", cur_query->rod_norm, cur_query->rod_degree);
    Logger(" ---- ");
    if (cur_query->id == 0)
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, 1);
    else
    {
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, diffnorm);
    }

    for (int i = 0; i < cur_query->rod_rotation_matrix.rows; i++)
        for (int j = 0; j < cur_query->rod_rotation_matrix.cols; j++)
            Logger("[%d][%d] %f ", i, j, cur_query->rod_rotation_matrix.at<double>(i, j));

#endif
    return 1;
}

void Extractor::ApplyImage() {

    Mat dst;
    char filename[30] = { 0, };    
    warpAffine(cur_query->ori_img, dst, cur_query->rod_rotation_matrix, Size(cur_query->ori_img.cols, cur_query->ori_img.rows));
    sprintf(filename, "saved/%d_applyadjust.png", cur_query->id);
    imwrite(filename, dst);

}

ADJST Extractor::CalAdjustData()
{
    ADJST newadj;

    double interval = cur_query->rod_norm - cur_train->rod_norm;
    /*     double agvx = (cur_train->center.x + cur_query->center.x)/2;
    double agvy = (cur_train->center.y + cur_query->center.y)/2;
 */
    double angle = -cur_query->rod_degree;
    if (angle >= -180)
        angle += -90;
    else
        angle += 270;

    double scale = cur_train->rod_norm / cur_query->rod_norm;
    double adjustx = cur_train->center.x - cur_query->center.x;
    double adjusty = cur_train->center.y - cur_query->center.y;
    double rotatex = cur_query->center.x;
    double rotatey = cur_query->center.y;

    double angleadjust = -1 * (angle + 90);
    double radian = angleadjust * M_PI / 180;
    double width = cur_query->ori_img.cols;
    double height = cur_query->ori_img.rows;

    Point2f pt1, pt2, pt3, pt4, ptR1, ptR2, ptR3, ptR4;
    pt1.x = (float)(cur_query->center.x * (1 - scale));
    pt1.y = (float)(cur_query->center.y * (1 - scale));

    pt2.x = (float)(pt1.x + width * scale);
    pt2.y = pt1.y;

    pt3.x = pt2.x;
    pt3.y = (float)(pt1.y + height * scale);

    pt4.x = pt1.x;
    pt4.y = pt3.y;

    Point2f ptcenter(rotatex, rotatey);
    ptR1 = mtrx.GetRotatePoint(ptcenter, pt1, radian);
    ptR2 = mtrx.GetRotatePoint(ptcenter, pt2, radian);
    ptR3 = mtrx.GetRotatePoint(ptcenter, pt3, radian);
    ptR4 = mtrx.GetRotatePoint(ptcenter, pt4, radian);

    int margin_l = 0;
    int margin_t = 0;
    int margin_r = width;
    int margin_b = height;

    if (ptR1.x + adjustx > margin_l)
        margin_l = (int)(ptR1.x + adjustx);
    if (ptR1.y + adjusty > margin_t)
        margin_t = (int)(ptR1.y + adjusty);

    if (ptR2.x + adjustx < margin_r)
        margin_r = (int)(ptR2.x + adjustx);
    if (ptR2.y + adjusty > margin_t)
        margin_t = (int)(ptR2.y + adjusty);

    if (ptR3.x + adjustx < margin_r)
        margin_r = (int)(ptR3.x + adjustx);
    if (ptR3.y + adjusty < margin_b)
        margin_b = (int)(ptR3.y + adjusty);

    if (ptR4.x + adjustx > margin_l)
        margin_l = (int)(ptR4.x + adjustx);
    if (ptR4.y + adjusty < margin_b)
        margin_b = (int)(ptR4.y + adjusty);

    if (margin_l > margin_t * width / height)
        margin_t = margin_l * height / width;
    else
        margin_l = margin_t * width / height;

    if (margin_r < margin_b * width / height)
        margin_b = margin_r * height / width;
    else
        margin_r = margin_b * width / height;

    // Margin 뒤집힘 현상 임시 방지
    if (margin_l > margin_r)
    {
        int nTemp = margin_l;
        margin_l = margin_r;
        margin_r = nTemp;
    }
    if (margin_t > margin_b)
    {
        int nTemp = margin_t;
        margin_t = margin_b;
        margin_b = nTemp;
    }

    newadj.angle = angle;
    newadj.rotate_centerx = rotatex;
    newadj.rotate_centery = rotatey;
    newadj.scale = scale;
    newadj.trans_x = adjustx;
    newadj.trans_x = adjusty;
    newadj.rect = Rect(margin_l, margin_t, margin_r - margin_l, margin_b - margin_t);

    Logger("Adjust data ..");
    Logger("angle  %f  centerx %f  centery %f scale %f ", newadj.angle, newadj.rotate_centerx, newadj.rotate_centery,
           newadj.scale);
    Logger(" trans x %f y %f rect %f %f %f %f ", newadj.trans_x, newadj.trans_y,
           newadj.rect.x, newadj.rect.y, newadj.rect.width, newadj.rect.height);

    return newadj;
}

int Extractor::WarpingStep1()
{
    //Test
    //Image Warping
    vector<Point2f> t_pset;
    vector<Point2f> q_pset;

    for (int i = 0; i < 4; i++)
    {
        t_pset.push_back(Point2f(cur_train->four_fpt[i].x, cur_train->four_fpt[i].y));
        q_pset.push_back(Point2f(cur_query->four_fpt[i].x, cur_query->four_fpt[i].y));

        Logger("t_pset %f %f  -- t_qset %f %f", t_pset[i].x, t_pset[i].y,
               q_pset[i].x, q_pset[i].y);
    }
    Logger(" Start find homography ");

    Mat _h = findHomography(q_pset, t_pset, 0);
    Logger(" Get homography ");

    for (int i = 0; i < _h.rows; i++)
        for (int j = 0; j < _h.cols; j++)
            Logger("[%d][%d] %lf ", i, j, _h.at<double>(i, j));

    Logger("ori image size %d %d ", cur_query->ori_img.cols, cur_query->ori_img.rows);

    Mat final;
    warpPerspective(cur_query->ori_img, final, _h, Size(cur_query->ori_img.cols, cur_query->ori_img.rows));

    static int index = 0;
    char filename[30] = {
        0,
    };
    sprintf(filename, "saved/%2d_perspective.png", index);
    imwrite(filename, final);

    Mat mcenter(3, 1, CV_64F);
    mcenter.at<double>(0) = cur_train->center.x;
    mcenter.at<double>(1) = cur_train->center.y;
    mcenter.at<double>(2) = 1;

    Mat mresult = _h * mcenter;

    Logger("estimated center point %f %f ", mresult.at<double>(0), mresult.at<double>(1));
    Logger("error  %f %f ", cur_query->center.x - mresult.at<double>(0), cur_query->center.y - mresult.at<double>(1));
    //ROI Warping
}

int Extractor::DecomposeHomography()
{

    Mat _h = cur_query->matrix_fromimg;
    Mat cm(3, 3, CV_32F, p->camera_matrix);
    Logger("_h from img is cols %d rows %d ", _h.cols, _h.rows);

    vector<Mat> Rs_decomp, ts_decomp, normals_decomp;

    int solutions = decomposeHomographyMat(_h, cm, Rs_decomp, ts_decomp, normals_decomp);

    for (int i = 0; i < solutions; i++)
    {
        Mat rvec_decomp;
        Rodrigues(Rs_decomp[i], rvec_decomp);
        Logger("Solution %d : ", i);
        Mat rvec_t = rvec_decomp.t();
        Logger("rvec from homography decomposition: %f %f %f ", rvec_t.at<double>(0), rvec_t.at<double>(1), rvec_t.at<double>(2));
        Mat ts_decom_t = ts_decomp[i].t();
        Mat normal_decom_t = normals_decomp[i].t();
        Logger("tvec from homography decomposition: %f %f %f ", ts_decom_t.at<double>(0), ts_decom_t.at<double>(1),
               ts_decom_t.at<double>(2));
        Logger("plane normal from homography decomposition: %f %f %f ", normal_decom_t.at<double>(0),
               normal_decom_t.at<double>(1), normal_decom_t.at<double>(2));
    }
}

int Extractor::FindHomographyP2P()
{

    Mat ppset1(4, 2, CV_32F);
    Mat ppset2(4, 2, CV_32F);
    for (int i = 0; i < 4; i++)
    {
        ppset1.at<float>(i, 0) = cur_train->four_fpt[i].x;
        ppset1.at<float>(i, 1) = cur_train->four_fpt[i].y;

        ppset2.at<float>(i, 0) = cur_query->four_fpt[i].x;
        ppset2.at<float>(i, 1) = cur_query->four_fpt[i].y;

        Logger("ppset %f %f %f -- %f %f", ppset1.at<float>(i, 0), ppset1.at<float>(i, 1),
               ppset2.at<float>(i, 0), ppset2.at<float>(i, 1));
    }

    Mat h = findHomography(ppset1, ppset2);
    for (int i = 0; i < h.rows; i++)
        for (int j = 0; j < h.cols; j++)
            Logger("[%d][%d] %lf ", i, j, h.at<double>(i, j));

    Mat mcenter(3, 1, CV_64F);
    mcenter.at<double>(0) = cur_train->center.x;
    mcenter.at<double>(1) = cur_train->center.y;
    mcenter.at<double>(2) = 1;
    Mat mret = h * mcenter;

    double newx = mret.at<double>(0) / mret.at<double>(2);
    double newy = mret.at<double>(1) / mret.at<double>(2);
    Logger("transformed cetner by P2P : %f %f ", newx, newy);
}

int Extractor::AdjustImage(ADJST adj)
{
    Size sz = Size(cur_query->ori_img.cols, cur_query->ori_img.rows);
    double angle = adj.angle + 90;
    double rad = angle * M_PI / 180.0;
    Logger("Adjust Image angle %f rad %f ", angle, rad);

    //Mat flipm = Mat::eye(3, 3, CV_32FC1);
    Mat mrot = mtrx.GetRotationMatrix(rad, adj.rotate_centerx, adj.rotate_centery);
    Mat mscale = mtrx.GetScaleMatrix(adj.scale, adj.scale, adj.rotate_centerx, adj.rotate_centery);
    Mat mtran = mtrx.GetTranslationMatrix(adj.trans_x, adj.trans_y);
    Mat mscaleout = mtrx.GetScaleMatrix(1, 1);

    Mat mfm = mscaleout * mtran * mscale * mrot;
    Logger("5 assembed mfm matrix ");
    for (int i = 0; i < mfm.rows; i++)
        for (int j = 0; j < mfm.cols; j++)
            Logger("[%d][%d] %f ", i, j, mfm.at<float>(i, j));

    Mat mret = mfm(Rect(0, 0, 3, 2));
    Logger("6 mret = submatrix of mfm");
    for (int i = 0; i < mret.rows; i++)
        for (int j = 0; j < mret.cols; j++)
            Logger("[%d][%d] %f ", i, j, mret.at<float>(i, j));

    Mat final;
    warpAffine(cur_query->ori_img, final, mret, Size(cur_query->ori_img.cols, cur_query->ori_img.rows));

    static int index = 0;
    char filename[30] = {
        0,
    };
    sprintf(filename, "saved/%2d_perspective.png", index);
    imwrite(filename, final);
}
