
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
#include "nlohmann/json.hpp"

using namespace std;
using namespace cv;
using json = nlohmann::json;

Extractor::Extractor(string &imgset, int cnt, int *roi)
{
    mtrx = MtrxUtil();
    InitializeData(cnt, roi);
    imgs = LoadImages(imgset);

    t = new TIMER();
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
    p->p_scale = 2;
    p->roi_type = CIRCLE;
    //p->circle_masking_type = FOUR_POINT_BASE;
    p->circle_masking_type = USER_INPUT_CIRCLE;

    if (p->roi_type == POLYGON)
    {
        p->count = (cnt - 1) / 2;
        p->region = (Pt *)g_os_malloc(sizeof(Pt) * p->count);
        for (int i = 0; i < p->count; i++)
        {
            int j = (i * 2) + 1;
            p->region[i].x = int(roi[j] / p->p_scale);
            p->region[i].y = int(roi[j + 1] / p->p_scale);
        }
    }
    else if (p->roi_type == CIRCLE)
    {
        if (p->circle_masking_type == FOUR_POINT_BASE) {
            p->count = 4;
            p->circle_fixedpt_radius = 200;
        }
        else if (p->circle_masking_type == USER_INPUT_CIRCLE)
            p->count = cnt;

        Logger("Initialize circle count %d %d ", cnt, p->count);
        p->circles = (Cr *)g_os_malloc(sizeof(Cr) * p->count);
        for (int i = 0; i < p->count; i++)
        {
            int j = (i * 3) + 1;
            p->circles[i].center.x = int(roi[j] / p->p_scale);
            p->circles[i].center.y = int(roi[j + 1] / p->p_scale);
            p->circles[i].radius = int(roi[j + 2] / p->p_scale);
        }
    }
    p->match_type = BEST_MATCH;

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

        p->blur_ksize = 5;
        p->blur_sigma = 0.6;
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
    //soccer
    /*
    p->world->four_fpt[0].x = 202.0;
    p->world->four_fpt[0].y = 601.0;
    p->world->four_fpt[1].x = 599.0;
    p->world->four_fpt[1].y = 601.0;
    p->world->four_fpt[2].x = 599.0;
    p->world->four_fpt[2].y = 762.0;
    p->world->four_fpt[3].x = 202.0;
    p->world->four_fpt[3].y = 761.0;
    p->world->center.x = 400.0;
    p->world->center.y = 656.0; */
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
    //ufc
    /*
    p->world->four_fpt[0].x = 271.0;
    p->world->four_fpt[0].y = 88.0;
    p->world->four_fpt[1].x = 528.0;
    p->world->four_fpt[1].y = 88.0;
    p->world->four_fpt[2].x = 528.0;
    p->world->four_fpt[2].y = 711.0;
    p->world->four_fpt[3].x = 271.0;
    p->world->four_fpt[3].y = 711.0;
    p->world->center.x = 400.0;
    p->world->center.y = 400.0;*/


    p->world->rod_norm = 100;

    /*     p->moved_region = (Pt *)g_os_malloc(sizeof(Pt) * p->count);
    memcpy(&p->moved_region, &p->region, sizeof(Pt) * p->count);
 */
    p->camera_matrix = (float *)g_os_malloc(sizeof(float) * 9);
    p->skew_coeff = (float *)g_os_malloc(sizeof(float) * 4);

    p->world->rod_norm = 0;
    NormalizePoint(p->world, 100);
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
    for (const string &ip : image_paths)
    {
        Logger("Read image : %s ", ip.c_str());
        images.push_back(imread(ip));
    }
    return images;
}

int Extractor::Execute()
{
    int index = 0;
    int ret = -1;
    for (Mat &img : imgs)
    {
        StartTimer(t);
        SCENE sc;
        sc.id = index;
        sc.ori_img = img;
        sc.img = ProcessImages(img);

        if (index == 0)
        {
            sc.id = 0;
            //soccer 1
            /*
            sc.four_fpt[0].x = 156.8897;
            sc.four_fpt[0].y = 1803.5595;
            sc.four_fpt[1].x = 1941.0336;
            sc.four_fpt[1].y = 542.6697;
            sc.four_fpt[2].x = 3731.3256;
            sc.four_fpt[2].y = 664.0180;
            sc.four_fpt[3].x = 2920.8808;
            sc.four_fpt[3].y = 2068.9079;
            sc.center.x = 2250.0673;
            sc.center.y = 1179.5054; */
            //soccer 3
            /*
            sc.four_fpt[0].x = 2421.0;
            sc.four_fpt[0].y = 1064.0;
            sc.four_fpt[1].x = 1792.0;
            sc.four_fpt[1].y = 1249.0;
            sc.four_fpt[2].x = 809.0;
            sc.four_fpt[2].y = 1180.0;
            sc.four_fpt[3].x = 1738.0;
            sc.four_fpt[3].y = 990.0;
            sc.center.x = 1720.0;
            sc.center.y = 1110.0; */
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
            //ufc
            /*
            sc.four_fpt[0].x = 1052.0;
            sc.four_fpt[0].y = 852.0;
            sc.four_fpt[1].x = 2056.0;
            sc.four_fpt[1].y = 772.0;
            sc.four_fpt[2].x = 2934.0;
            sc.four_fpt[2].y = 1720.0;
            sc.four_fpt[3].x = 1708.0;
            sc.four_fpt[3].y = 1852.0;            
            sc.center.x = 1092.0;
            sc.center.y = 1080.0; */
        }

        ImageMasking(&sc);
        ret = GetFeature(&sc);
        Logger("[%d] feature extracting  %f ", index, LapTimer(t));

#ifdef _IMGDEBUG
//        SaveImage(&sc, 1);
#endif

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
        if (index == 11)
            break;
    }

    //Export result to josn file
    Export();
    
    return ERR_NONE;
}

Mat Extractor::ProcessImages(Mat &img)
{

    Mat blur_img;
    Mat dst;
    if (p->p_scale != 1)
    {
        resize(img, img, Size(int(img.cols / p->p_scale), int(img.rows / p->p_scale)), 0, 0, 1);
    }

    cvtColor(img, blur_img, cv::COLOR_RGBA2GRAY);
    normalize(blur_img, dst, 0, 255, NORM_MINMAX, -1, noArray());
    GaussianBlur(dst, blur_img, {p->blur_ksize, p->blur_ksize}, p->blur_sigma, p->blur_sigma);

    return blur_img;
}

int Extractor::ImageMasking(SCENE *sc)
{
    Logger("Image masking function start ");
    Mat mask = Mat::zeros(sc->img.rows, sc->img.cols, CV_8UC1);

    if (p->circle_masking_type == FOUR_POINT_BASE)
    {
        for (int i = 0; i < 4; i++)
        {
            if (sc->id == 0) {
                Logger("masking point 1 %f %f ", sc->four_fpt[i].x, sc->four_fpt[i].y);
                circle(mask, Point((int)sc->four_fpt[i].x/p->p_scale, (int)sc->four_fpt[i].y/p->p_scale),
                       int(p->circle_fixedpt_radius/p->p_scale), Scalar(255), -1);
            }
            else {
                Logger("masking point 2 %f %f ", cal_group.back().four_fpt[i].x, cal_group.back().four_fpt[i].y);
                circle(mask,
                       Point((int)cal_group.back().four_fpt[i].x/p->p_scale, (int)cal_group.back().four_fpt[i].y/p->p_scale),
                       int(p->circle_fixedpt_radius/p->p_scale), Scalar(255), -1);
            }
        }
    }
    else if (p->circle_masking_type == USER_INPUT_CIRCLE) {
        for (int i = 0; i < p->count; i++) {
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

vector<KeyPoint> Extractor::KeypointMasking(vector<KeyPoint> *oip)
{
    vector<KeyPoint> ip;
    //Logger("Before masking %d ", oip->size());
    int left = 0;
    int total = 0;
    int del = 0;

    for (int i = 0; i < p->count; i++)
    {
        Logger("roi check %d %d ", p->moved_region[i].x, p->moved_region[i].y);
    }

    for (auto it = oip->begin(); it != oip->end(); it++)
    {
        total++;
        Pt cp(int(it->pt.x), int(it->pt.y));
        int ret = isInside(p->moved_region, p->count, cp);

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
        ret = FindHomographyMatch();
    }

    return ret;
}

int Extractor::FindHomographyMatch() {

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

    } else if (p->match_type == BEST_MATCH ) {
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

/*     for (int i = 0; i < _h.rows; i++)
        for (int j = 0; j < _h.cols; j++)
            Logger("[%d][%d] %lf ", i, j, _h.at<double>(i, j));
 */
#endif

    return matches.size();
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

    if (last.size() > 100) {
        while (last.size() >= 100) {
            last.pop_back();
        }
        Logger("matches->pop_back size %d ", last.size());
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
            Logger("mh distance is over limit %f %f ", mh_distance_deg, mh_distance_dist);
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

    if (cur_query->id == 1)
    {
        cur_query->four_fpt[0].x = 952.9364;
        cur_query->four_fpt[0].y = 1288.5572;
        cur_query->four_fpt[1].x = 1440.4313;
        cur_query->four_fpt[1].y = 840.5394;
        cur_query->four_fpt[2].x = 2800.3415;
        cur_query->four_fpt[2].y = 855.5865;
        cur_query->four_fpt[3].x = 2479.0112;
        cur_query->four_fpt[3].y = 1454.5618;
        cur_query->center.x = 1914.4328;
        cur_query->center.y = 1073.7937;
    }
    else if (cur_query->id == 2)
    {
        cur_query->four_fpt[0].x = 998.7236;
        cur_query->four_fpt[0].y = 1304.2517;
        cur_query->four_fpt[1].x = 1371.0202;
        cur_query->four_fpt[1].y = 836.4944;
        cur_query->four_fpt[2].x = 2736.2697;
        cur_query->four_fpt[2].y = 838.9214;
        cur_query->four_fpt[3].x = 2573.9860;
        cur_query->four_fpt[3].y = 1451.1641;
        cur_query->center.x = 1912.02;
        cur_query->center.y = 1073.25;
    }
    else if (cur_query->id == 3)
    {
        cur_query->four_fpt[0].x = 1052.4403;
        cur_query->four_fpt[0].y = 1324.8000;
        cur_query->four_fpt[1].x = 1305.9772;
        cur_query->four_fpt[1].y = 849.2764;
        cur_query->four_fpt[2].x = 2662.8134;
        cur_query->four_fpt[2].y = 833.5820;
        cur_query->four_fpt[3].x = 2672.0358;
        cur_query->four_fpt[3].y = 1450.5169;
        cur_query->center.x = 1906.8071;
        cur_query->center.y = 1076.7603;
    }
    else if (cur_query->id == 4)
    {
        cur_query->four_fpt[0].x = 1126.7055;
        cur_query->four_fpt[0].y = 1358.9393;
        cur_query->four_fpt[1].x = 1240.7729;
        cur_query->four_fpt[1].y = 872.2517;
        cur_query->four_fpt[2].x = 2579.9729;
        cur_query->four_fpt[2].y = 800.5753;
        cur_query->four_fpt[3].x = 2761.6718;
        cur_query->four_fpt[3].y = 1411.5235;
        cur_query->center.x = 1903.7908;
        cur_query->center.y = 1074.2812;
    }
    else if (cur_query->id == 5)
    {
        cur_query->four_fpt[0].x = 1207.7661;
        cur_query->four_fpt[0].y = 1379.4875;
        cur_query->four_fpt[1].x = 1195.3077;
        cur_query->four_fpt[1].y = 891.6674;
        cur_query->four_fpt[2].x = 2526.2563;
        cur_query->four_fpt[2].y = 807.5326;
        cur_query->four_fpt[3].x = 2864.8989;
        cur_query->four_fpt[3].y = 1411.3619;
        cur_query->center.x = 1918.5872;
        cur_query->center.y = 1086.5610;
    }

    float err = 0;
    //move centerpoint
    FPt newcen = mtrx.TransformPtbyHomography(cur_train->center, cur_query->matrix_fromimg);

    //err += abs(cur_query->center.x - newcen.x);
    //err += abs(cur_query->center.y - newcen.y);
    Logger("Query center answer(%f, %f) - (%f, %f)", cur_query->center.x, cur_query->center.y, newcen.x, newcen.y);
    cur_query->center = newcen;

    //move 4point
    for (int i = 0; i < p->count; i++)
    {
        FPt newpt = mtrx.TransformPtbyHomography(cur_train->four_fpt[i], cur_query->matrix_fromimg);

        Logger(" four pt move [%d] answer (%f, %f) - (%f, %f) ", i,
               cur_query->four_fpt[i].x, cur_query->four_fpt[i].y,
               newpt.x, newpt.y);
        //err += abs(cur_query->four_fpt[i].x - newpt.x);
        //err += abs(cur_query->four_fpt[i].y - newpt.y);
        cur_query->four_fpt[i].x = newpt.x;
        cur_query->four_fpt[i].y = newpt.y;
    }

    Logger("err : %f ", err);
    FindBaseCoordfromWd(NORMAL_VECTOR_CAL);
    ApplyImage();    
    //MakingLog();

    //move user circle input
    if (p->roi_type == CIRCLE && p->circle_masking_type == USER_INPUT_CIRCLE)
    {
        Mat apply_homo;
        if (p->p_scale != 1)
            apply_homo = cur_query->matrix_scaledfromimg;
        else
            apply_homo = cur_query->matrix_fromimg;

        for (int i = 0; i < p->count; i++)
        {
            //Logger("mas %d %d %d ", p->circles[i].center.x, p->circles[i].center.y, p->circles[i].radius);
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

int Extractor::VerifyNumeric()
{

    verify_mode = true;

    int index = 0;
    for (Mat &img : imgs)
    {
        SCENE sc;
        sc.id = index;
        sc.ori_img = img;

        if (index == 0)
        {
            sc.id = 0;
            sc.four_fpt[0].x = 916.3685;
            sc.four_fpt[0].y = 1266.8764;
            sc.four_fpt[1].x = 1535.5683;
            sc.four_fpt[1].y = 836.3326;
            sc.four_fpt[2].x = 2905.2381;
            sc.four_fpt[2].y = 881.4742;
            sc.four_fpt[3].x = 2403.9367;
            sc.four_fpt[3].y = 1468.9617;
            sc.center.x = 1944.2695;
            sc.center.y = 1077.5728;
        }
        else if (index == 1)
        {
            sc.id = 1;
            sc.four_fpt[0].x = 952.9364;
            sc.four_fpt[0].y = 1288.5572;
            sc.four_fpt[1].x = 1440.4313;
            sc.four_fpt[1].y = 840.5394;
            sc.four_fpt[2].x = 2800.3415;
            sc.four_fpt[2].y = 855.5865;
            sc.four_fpt[3].x = 2479.0112;
            sc.four_fpt[3].y = 1454.5618;
            sc.center.x = 1914.4328;
            sc.center.y = 1073.7937;
        }

        sc.img = ProcessImages(sc.ori_img);
        int ret = GetFeature(&sc);
        cal_group.push_back(sc);

        SetCurTrainScene(p->world);
        SetCurQueryScene(&cal_group[index]);
        ret = FindBaseCoordfromWd();

        //PostProcess();
        CalAdjustData();

        index++;
        Logger("Verify Done.");

        if (index == 1)
            break;
    }

    return ERR_NONE;
}

int Extractor::WarpingStep2()
{

    SetCurTrainScene(&cal_group[0]);
    SetCurQueryScene(&cal_group[1]);
    ADJST adj = CalAdjustData();
    AdjustImage(adj);
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

void Extractor::MakingLog()
{

    string filePath = "log/log_pts_" + getCurrentDateTime("date") + ".txt";
    ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
    char buf[2048];

    sprintf(buf, "{\n");
    ofs << '\t' << buf;
    sprintf(buf, "\"dsc_id\": ");
    ofs << '\t' << buf;
    sprintf(buf, "Filename : %s \n", image_paths[cur_query->id].c_str());
    ofs << '\t' << buf;     

 
    sprintf(buf, "\"pts_3d\": {\n");
    ofs << '\t' << buf;
    sprintf(buf, "\"X1\": %f,\n", cur_query->four_fpt[0].x);
    ofs << '\t' << buf;
    sprintf(buf, "\"Y1\": %f,\n", cur_query->four_fpt[0].y);
    ofs << '\t' << buf;
    sprintf(buf, "\"X2\": %f,\n", cur_query->four_fpt[1].x);
    ofs << '\t' << buf;
    sprintf(buf, "\"Y2\": %f,\n", cur_query->four_fpt[1].y);
    ofs << '\t' << buf;
    sprintf(buf, "\"X3\": %f,\n", cur_query->four_fpt[2].x);
    ofs << '\t' << buf;
    sprintf(buf, "\"Y3\": %f,\n", cur_query->four_fpt[2].y);
    ofs << '\t' << buf;
    sprintf(buf, "\"X4\": %f,\n", cur_query->four_fpt[3].x);
    ofs << '\t' << buf;
    sprintf(buf, "\"Y4\": %f,\n", cur_query->four_fpt[3].y);
    ofs << '\t' << buf;
    sprintf(buf, "\"CenterX\": %f,\n", cur_query->center.x);
    ofs << '\t' << buf;
    sprintf(buf, "\"CenterY\": %f\n", cur_query->center.y);
    ofs << '\t' << buf;    
    sprintf(buf, "}\n");
    ofs << '\t' << buf;
    sprintf(buf, "},\n");
    ofs << '\t' << buf;

    
    ofs.close();
}

void Extractor::DrawNormal() {

}

void Extractor::Export() {

    //world coord 
    //world point1 x = p->world->four_fpt[0].x;
    //world point1 y = p->world->four_fpt[0].x;
    //world center x = p->world->center.x;
    //world center y = p->world->center.y;    

    for (vector<SCENE>::const_iterator it = cal_group.begin(); it != cal_group.end(); it++)
    {
        //id = it-> image_paths[it->id] 
        //point1 x = it->four_fpt[0].x
        //point1 y = it->four_fpt[0].y
        //center x = it->center.x;
        //center y = it->center.y;
    }

    //file write. path = "/saved/xxx.json"

    json jObj = json::object();
    //World
    json world;
    world["X1"] = p->world->four_fpt[0].x;
    world["Y1"] = p->world->four_fpt[0].y;
    world["X2"] = p->world->four_fpt[1].x;
    world["Y2"] = p->world->four_fpt[1].y;
    world["X3"] = p->world->four_fpt[2].x;
    world["Y3"] = p->world->four_fpt[2].y;
    world["X4"] = p->world->four_fpt[3].x;
    world["Y4"] = p->world->four_fpt[3].y;
    //GDT
    //jObj["stadium"] = GROUNDTYPE.get();
    jObj["stadium"] = "SoccerHalf";

    //2dPoint
    json point2d = json::object();
    point2d["UpperPosX"] = -1.0;
    point2d["UpperPosY"] = -1.0;
    point2d["MiddlePosX"] = -1.0;
    point2d["MiddlePosX"] = -1.0;
    point2d["LowerPosX"] = -1.0;
    point2d["LowerPosX"] = -1.0;


    auto arr = json::array();

    for (vector<SCENE>::const_iterator it = cal_group.begin(); it != cal_group.end(); it++)
    {
  
        json jDsc = json::object();
  
        jDsc["dsc_id"] = image_paths[it->id];
        jDsc["pts_2d"] = point2d;

        //3dPoint
        json point3d = json::object();
        point3d["X1"] = it->four_fpt[0].x;
        point3d["Y1"] = it->four_fpt[0].y;
        point3d["X2"] = it->four_fpt[1].x;
        point3d["Y2"] = it->four_fpt[1].y;
        point3d["X3"] = it->four_fpt[2].x;
        point3d["Y3"] = it->four_fpt[2].y;
        point3d["X4"] = it->four_fpt[3].x;
        point3d["Y4"] = it->four_fpt[3].y;
        point3d["CenterX"] = it->center.x;
        point3d["CenterY"] = it->center.y;
        jDsc["pts_3d"] = point3d;

        arr.push_back(jDsc);

    }
    jObj["points"] = arr;

    //file write
    std::ofstream file("/saved/UserPointData_" + getCurrentDateTime("date") + ".pts");

    file << std::setw(4) << jObj << '\n';
    std::cout << std::setw(4) << jObj << '\n';
}