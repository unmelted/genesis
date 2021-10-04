    
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
    InitializeData(cnt, roi);
    imgs = LoadImages(imgset);

#ifdef _IMGDEBUG
    //SaveImageSet(imgs);
#endif
}

Extractor::~Extractor()
{
    g_os_free(p->region);
    g_os_free(p->world);
    g_os_free(p->camera_matrix);
    g_os_free(p->skew_coeff);
    g_os_free(p);
}

void Extractor::InitializeData(int cnt, int *roi)
{

    p = (PARAM *)g_os_malloc(sizeof(PARAM));
    p->count = cnt / 2 - 1;
    p->region = (Pt *)g_os_malloc(sizeof(Pt) * p->count);
    p->p_scale = 1;

    for (int i = 0; i < p->count; i++)
    {
        int j = (i * 2) + 1;
        p->region[i].x = int(roi[j] / p->p_scale);
        p->region[i].y = int(roi[j + 1] / p->p_scale);
    }

    p->blur_ksize = 19;
    p->blur_sigma = 1;
    p->desc_byte = 32;
    p->use_ori = true;
    p->nms_k = 23;
    p->fast_k = 24;
    p->minx = 0;
    p->p_scale = 1;

    p->pwidth = 3840;  //4K width
    p->pheight = 2160; //4K height
    p->sensor_size = 17.30 / 1.35;
    Logger("Sensor Size -- %d", p->sensor_size);
    p->focal = 3840;

    p->world = new SCENE();
    p->world->four_fpt[0].x = 330.0;
    p->world->four_fpt[0].y = 601.0;
    p->world->four_fpt[1].x = 473.0;
    p->world->four_fpt[1].y = 601.0;
    p->world->four_fpt[2].x = 490.0;
    p->world->four_fpt[2].y = 710.0;
    p->world->four_fpt[3].x = 310.0;
    p->world->four_fpt[3].y = 709.0;
    p->world->center.x = 400.0;
    p->world->center.y = 656.0;
    p->world->rod_norm = 100;

    //p->world->normal = (float *)g_os_malloc(sizeof(float)* 6);
    p->camera_matrix = (float *)g_os_malloc(sizeof(float) * 9);
    p->skew_coeff = (float *)g_os_malloc(sizeof(float) * 4);

/*     p->world->normal[0][0] = 50.0;
    p->world->normal[0][1] = 50.0;
    p->world->normal[0][2] = 0.0;
    p->world->normal[1][0] = 50.0;
    p->world->normal[1][1] = 50.0;
    p->world->normal[1][2] = -100.0;
 */
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
    p->blur_ksize = 19;
    p->blur_sigma = 1;
    p->desc_byte = 32;
    p->use_ori = true;
    p->nms_k = 23;
    p->fast_k = 24;
    p->minx = 0;
    p->p_scale = 1;

    p->pwidth = 3840;  //4K width
    p->pheight = 2160; //4K height
    p->sensor_size = 17.30 / 1.35;
    p->focal = 3840;
}

void Extractor::NormalizePoint(SCENE* sc, int maxrange)
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

    sc->normal[0][0] = 50.0;
    sc->normal[0][1] = 50.0;
    sc->normal[0][2] = 0.0;
    sc->normal[1][0] = 50.0;
    sc->normal[1][1] = 50.0;
    sc->normal[1][2] = -100.0;

#if _DEBUG
    Logger("Normalized point 4pt");
    for (int i = 0; i < 4; i++)
        Logger(" %f, %f ", sc->four_fpt[i].x, sc->four_fpt[i].y);

    Logger("Normalized point normal.");
    for(int i = 0 ; i < 2 ; i ++)
        for( int j = 0 ; j < 3 ; j ++)
            Logger("[%d][%d]  %f" , i, j , sc->normal[i][j]);

#endif
}

void Extractor::DrawInfo()
{

    int index = 0;
    for (const auto &each : cal_group)
    {
        Mat dst;
        char filename[30] = {
            0,
        };
        drawKeypoints(each.img, each.ip, dst);
        sprintf(filename, "saved/%2d_feature.png", index);
        imwrite(filename, dst);
        index++;

        if(index > 1)
            break;
/* 
        for(int i = 0; i < each.ip.size(); i++) {
            Logger("%d, %d  oct %d class %d resp %lf size %lf angle %lf", int(each.ip[i].pt.x), int(each.ip[i].pt.y), each.ip[i].class_id,
            each.ip[i].class_id, each.ip[i].response, each.ip[i].size , each.ip[i].angle);
        }
 */    }
}

void Extractor::SaveImageSet(vector<Mat> &images)
{

    Logger("Save Image Set is called ");
    char filename[30] = {
        0,
    };
    int index = 0;
    for (const auto &img : images)
    {
        sprintf(filename, "saved/%2d_saveimg.png", index);
        imwrite(filename, img);
        index++;
    }
}

vector<Mat> Extractor::LoadImages(const string &path)
{
    const int FK = 3500;
    const int FHD = 1900;

    namespace fs = std::__fs::filesystem;

    vector<string> image_paths;
    for (const auto &entry : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(entry) &&
            entry.path().extension().string() == ".png")
        {
            image_paths.push_back(entry.path().string());
        }
    }

    if (p->p_scale == 0)
    {
        Mat sample = imread(image_paths[0]);
        if (sample.cols > FK)
        {
            p->p_scale = 1; //origin scale test first
        }
        else if (sample.cols > FHD)
        {
            p->p_scale = 1;
        }
        else
        {
            p->p_scale = 1;
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
    for (Mat &img : imgs) {

        SCENE sc;

        sc.id = index;
        sc.ori_img = img;
        sc.img = ProcessImages(img);
        int ret = GetFeature(&sc);

        if (index == 0) {

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
            cal_group.push_back(sc);            
            is_first = true;

            SetCurTrainScene(p->world);
            SetCurQueryScene(&cal_group[index]);
            ret = FindBaseCoordfromWd();
        }
        else {

            cal_group.push_back(sc);
            SetCurTrainScene(&cal_group[index - 1]);
            SetCurQueryScene(&cal_group[index]);
            ret = FindHomographyMatch();
        }

        if (ret > 0 ) {
            PostProcess();
        }
        else {
            Logger("Match Pair Fail. Can't keep going.");
        }

        is_first = false;
        index++;

#if _DEBUG
        if (index >= 1)
            break;
#endif
    }

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

int Extractor::GetFeature(SCENE *sc)
{
    // FAST + BRIEF
    auto feature_detector = FastFeatureDetector::create(p->fast_k, true, FastFeatureDetector::TYPE_9_16);
    //auto feature_detector = AgastFeatureDetector::create(); //AGAST
    //Ptr<xfeatures2d::MSDDetector>feature_detector = xfeatures2d::MSDDetector::create(9,11,15); //MSDETECTOR
    Ptr<xfeatures2d::BriefDescriptorExtractor> dscr;
    dscr = xfeatures2d::BriefDescriptorExtractor::create(p->desc_byte, p->use_ori);

    //AKAZE
    //auto feature_detector = KAZE::create(false, false, 0.001f, 4, 4, KAZE::DIFF_PM_G1);
    //Ptr<KAZE>dscr = feature_detector;

    Mat desc;
    vector<KeyPoint> kpt;
    vector<KeyPoint> f_kpt;

    feature_detector->detect(sc->img, kpt);
    Logger("extracted keypoints count : %d", kpt.size());
    f_kpt = KeypointMasking(&kpt);
    dscr->compute(sc->img, f_kpt, desc);

    sc->ip = f_kpt;
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

    for (auto it = oip->begin(); it != oip->end(); it++)
    {
        total++;
        Pt cp(int(it->pt.x), int(it->pt.y));
        int ret = isInside(p->region, p->count, cp);

        if (ret == 0 && it != oip->end()) {
            del++;
        }
       else  {
            ip.push_back(*it);
            left++;
        }
    }

    Logger("new vector  %d. left %d  del %d / total ip %d ", ip.size(), left, del, total);
    return ip;
}
int Extractor::FindHomographyMatch() {

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);

    vector<DMatch> in;
    vector<DMatch> matches;

    if (cur_train->desc.type() != CV_32F || cur_query->desc.type() != CV_32F)
    {
        cur_train->desc.convertTo(cur_train->desc, CV_32F);
        cur_query->desc.convertTo(cur_query->desc, CV_32F);
    }

    matcher->match(cur_query->desc, cur_train->desc, in);
    sort(in.begin(), in.end());
    Logger("matchees before cut %d  ", in.size());
    if( in.size() == 0)
        return 1;

/*     for (int i = 0; i < in.size(); i++)
    {
        Logger("Distance %f imgidx %d trainidx %d queryidx %d", in[i].distance,
               in[i].imgIdx, in[i].trainIdx, in[i].queryIdx);
    }
 */
    int min, max = 0;
    float max_score = in[0].distance * 3;
    Logger("max score %f ", max_score);
    if (cur_train->ip.size() >= cur_query->ip.size())
    {
        max = cur_train->ip.size();
        min = cur_query->ip.size();
    }
    else
    {
        max = cur_query->ip.size();
        min = cur_train->ip.size();
    }

    int *t_hist = (int *)g_os_malloc(sizeof(int) * cur_train->ip.size());
    int *q_hist = (int *)g_os_malloc(sizeof(int) * cur_query->ip.size());
    for (int a = 0; a < cur_train->ip.size(); a++)
        t_hist[a] = 0;
    for (int b = 0; b < cur_query->ip.size(); b++)
        q_hist[b] = 0;

    //min = 3;
    int is = 0;
    for (int t = 0; t < min; t++)
    {
        if (t_hist[in[t].trainIdx] == 0 && q_hist[in[t].queryIdx] == 0)
        {
            matches.push_back(in[t]);

            t_hist[in[t].trainIdx]++;
            q_hist[in[t].queryIdx]++;
            is++;
            if (is >= min)
                break;
        }
        else
            Logger("double check %d %d ", in[t].trainIdx, in[t].queryIdx);
    }

    g_os_free(t_hist);
    g_os_free(q_hist);

#if defined _DEBUG

    Logger("matchees after cut %d  ", matches.size());
    for (int i = 0; i < matches.size(); i++)
    {
        Logger("Distance %f imgidx %d trainidx %d queryidx %d", matches[i].distance,
               matches[i].imgIdx, matches[i].trainIdx, matches[i].queryIdx);
    }

#endif

    vector<Point2f> train_pt, query_pt;

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
            train_pt.push_back(Point2f(tx, ty));
            query_pt.push_back(Point2f(qx, qy));
        }
    }

    Mat _h = findHomography(train_pt, query_pt, FM_RANSAC);
    //Mat _h = getAffineTransform(query_pt, train_pt);
    //Mat _h = estimateAffine2D(query_pt, train_pt);
    // solvePnp();
    //Mat _h = estimateRigidTransform(query_pt, train_pt, false);

    cur_query->matrix_fromimg = _h;
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

    char filename[30] = {
        0,
    };
    sprintf(filename, "saved/%2d_matchimg.png", index);
    imwrite(filename, outputImg);

    for (int i = 0; i < _h.rows; i++)
        for (int j = 0; j < _h.cols; j++)
            Logger("[%d][%d] %lf ", i, j, _h.at<double>(i, j));

    FPt ttp = mtrx.TransformPtbyHomography(&cur_train->center, _h);
    Logger("refactoring function %f %f ", ttp.x, ttp.y);

/*     Mat fin, fin2;
    //warpPerspective(cur_train->img, fin, _h, Size(cur_train->img.cols, cur_train->img.rows));
    warpAffine(cur_query->ori_img, fin, _h, Size(cur_query->img.cols * p->p_scale, cur_train->img.rows * p->p_scale));
    //cur_query->img = fin;
    sprintf(filename, "saved/%2d_perspective.png", index);
    imwrite(filename, fin);

    warpAffine(cur_query->img, fin2, _h, Size(cur_query->img.cols, cur_train->img.rows));
    sprintf(filename, "saved/%2d_perspective_pp.png", index);
    imwrite(filename, fin2);
 */
    index++;
#endif
    return matches.size();
}

int Extractor::PostProcess()
{
    if(is_first)
        return ERR_NONE;

    //move centerpoint
    cur_query->center = mtrx.TransformPtbyHomography(&cur_train->center, cur_query->matrix_fromimg);
    Logger("Query center %f %f ", cur_query->center.x, cur_query->center.y);

    //move normal vector
    //move region point



    //CalAdjustData();
    //Warping();
}

int Extractor::FindBaseCoordfromWd()
{
    Mat ppset1(4, 3, CV_32F);
    Mat ppset2(4, 2, CV_32F);
    for (int i = 0; i < 4; i++)
    {
        ppset1.at<float>(i, 0) = cur_train->four_fpt[i].x;
        ppset1.at<float>(i, 1) = cur_train->four_fpt[i].y;
        ppset1.at<float>(i, 2) = cur_train->four_fpt[i].z;

        ppset2.at<float>(i, 0) = (float)cur_query->four_fpt[i].x;
        ppset2.at<float>(i, 1) = (float)cur_query->four_fpt[i].y;

        Logger("ppset %f %f %f -- %f %f", ppset1.at<float>(i, 0), ppset1.at<float>(i, 1), ppset1.at<float>(i, 2),
               ppset2.at<float>(i, 0), ppset2.at<float>(i, 1));
    }

    Mat cm(3, 3, CV_32F, p->camera_matrix);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Logger("[%d][%d] %f", i, j, cm.at<float>(i,j));
        }
    }
    Logger("skew %f %f %f %f ", p->skew_coeff[0], p->skew_coeff[1], p->skew_coeff[2], p->skew_coeff[3]);

    
    Mat sc(4, 1, CV_32F, p->skew_coeff);
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
    Mat tvec(2, 3, CV_32F, p->world->normal);
    Logger(" world normal -- %d %d", tvec.rows, tvec.cols);
    for( int i = 0 ; i < tvec.rows; i ++)
        for(int j = 0 ; j < tvec.cols; j ++)
            Logger("[%d][%d] %f ", i, j , tvec.at<float>(i,j));

    projectPoints(tvec, cur_query->rot_matrix, cur_query->trans_matrix,
                  cm, sc,
                  projectedNormal);

    Logger("project Points is done. ");
    vector<Point2f>pnormal;
    for (int i = 0; i < projectedNormal.rows; i++)
    {
        for (int j = 0; j < projectedNormal.cols; j++)
        {
            Point2f v1 = projectedNormal.at<Point2f>(i, j);
            pnormal.push_back((v1));
            Logger("[%d][%d] %f %f  ", i, j, v1.x, v1.y);
        }
    }


    Point2f angle_vec = Point2f(pnormal[1].x - pnormal[0].x, pnormal[1].y - pnormal[0].y);
    double degree = fastAtan2(angle_vec.x, angle_vec.y);
    double dnorm = norm(pnormal[0] - pnormal[1]);

    if (pnormal[1].y > pnormal[0].y)
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
    if(is_first)
        diffnorm = 1;
    else {
        diffnorm = cal_group[0].rod_norm /cur_query->rod_norm;        
        Logger("second train norm %f diff norm %f ", cal_group[0].rod_norm, diffnorm);
    }

    Logger("dnorm %f degree %f", cur_query->rod_norm, cur_query->rod_degree);
    Logger(" ---- ");
    if (is_first)
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, 1);
    else
    {
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, diffnorm);
    }

    for( int i = 0 ; i < cur_query->rod_rotation_matrix.rows; i ++)
        for(int j = 0 ; j < cur_query->rod_rotation_matrix.cols; j ++)
            Logger("[%d][%d] %f ", i, j , cur_query->rod_rotation_matrix.at<double>(i,j));

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
    else angle += 270;

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

    Logger("Adjust data .." );
    Logger("angle  %f  centerx %f  centery %f scale %f ", newadj.angle , newadj.rotate_centerx, newadj.rotate_centery,
            newadj.scale);
    Logger(" trans x %f y %f rect %f %f %f %f ", newadj.trans_x, newadj.trans_y, 
            newadj.rect.x, newadj.rect.y, newadj.rect.width, newadj.rect.height);

    return newadj;
}

int Extractor::VerifyNumeric() {

    verify_mode = true;
    
   int index = 0;
    for (Mat &img : imgs)
    {
        SCENE sc;
        sc.id = index;
        sc.ori_img = img;
        if( index == 0 ){
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
        else if( index == 1) {
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

        if (index == 0)
            is_first = true;

#if 1 //step 3
        if( index == 0 )
            SetCurTrainScene(p->world);
        else
            SetCurTrainScene(&cal_group[index -1]);

        SetCurQueryScene(&cal_group[index]);
        
        Logger(" -------- Test Calculaton ");

        if(index > 0) {           
            FindHomographyP2P(); 
            ret = FindHomographyMatch();
            //DecomposeHomography();
        }

        is_first = false;
        index++;

        if(index > 1)
            break;
    }
    DrawInfo();

#endif

#if 0 //step 2

        SetCurTrainScene(p->world);
        SetCurQueryScene(&cal_group[index]);

        FindBaseCoordfromWd();

        if( index > 0 ) {

            SetCurTrainScene(&cal_group[index-1]);
            ADJST adj = CalAdjustData();                    
            AdjustImage(adj);        
        }
        //WarpingStep1();
        is_first = false;
        index++;

        if(index > 1)
            break;
    }

#endif

#if 0 //step1
        SetCurTrainScene(p->world);
        SetCurQueryScene(&cal_group[index]);

        FindBaseCoordfromWd();
        is_first = false;
        index++;
    }

    WarpingStep2();
#endif    
    Logger("Verify Done.");
}

int Extractor::WarpingStep2(){

    SetCurTrainScene(&cal_group[0]);
    SetCurQueryScene(&cal_group[1]);
    ADJST adj = CalAdjustData();
    AdjustImage(adj);
}

int Extractor::WarpingStep1()
{   
    //Test
    //Image Warping
    vector<Point2f>t_pset;
    vector<Point2f>q_pset;

    for (int i = 0; i < 4; i++)
    {
        t_pset.push_back(Point2f(cur_train->four_fpt[i].x, cur_train->four_fpt[i].y));
        q_pset.push_back(Point2f(cur_query->four_fpt[i].x, cur_query->four_fpt[i].y));

        Logger("t_pset %f %f  -- t_qset %f %f", t_pset[i].x, t_pset[i].y,
               q_pset[i].x, q_pset[i].y);
    }
    Logger(" Start find homography ");

    Mat _h = findHomography(q_pset, t_pset, 0);
    Logger(" Get homography " );

    for (int i = 0; i < _h.rows; i++)
        for (int j = 0; j < _h.cols; j++)
            Logger("[%d][%d] %lf ", i, j, _h.at<double>(i, j));

    Logger("ori image size %d %d ", cur_query->ori_img.cols, cur_query->ori_img.rows);


    Mat final;
    warpPerspective(cur_query->ori_img, final, _h, Size(cur_query->ori_img.cols, cur_query->ori_img.rows));

    static int index = 0;
    char filename[30] = { 0, };        
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

int Extractor::DecomposeHomography() {

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
        Logger("tvec from homography decomposition: %f %f %f ", ts_decom_t.at<double>(0),ts_decom_t.at<double>(1),
            ts_decom_t.at<double>(2));
        Logger("plane normal from homography decomposition: %f %f %f ", normal_decom_t.at<double>(0),
            normal_decom_t.at<double>(1), normal_decom_t.at<double>(2));
    }
}

int Extractor::FindHomographyP2P() {

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
/*     adj.angle = -89.02552;
    adj.rotate_centerx = 1914.4328;
    adj.rotate_centery = 1073.7937;
    adj.scale = 1.01319;
    adj.trans_x = 0.2191;
    adj.trans_y = 2.9097;
 */
    Size sz = Size(cur_query->ori_img.cols, cur_query->ori_img.rows);
    double angle = adj.angle + 90;
    double rad = angle * M_PI/ 180.0;
    Logger("Adjust Image angle %f rad %f ", angle, rad);

    //Mat flipm = Mat::eye(3, 3, CV_32FC1);
    Mat mrot = mtrx.GetRotationMatrix(rad, adj.rotate_centerx, adj.rotate_centery);
    Mat mscale = mtrx.GetScaleMatrix(adj.scale, adj.scale, adj.rotate_centerx, adj.rotate_centery);
    Mat mtran = mtrx.GetTranslationMatrix(adj.trans_x, adj.trans_y);
    Mat mscaleout = mtrx.GetScaleMatrix(1, 1);


    Mat mfm = mscaleout * mtran * mscale * mrot;
    Logger("5 assembed mfm matrix ");        
    for( int i = 0 ; i < mfm.rows; i ++)
        for(int j = 0 ; j < mfm.cols; j ++)
            Logger("[%d][%d] %f ", i, j , mfm.at<float>(i,j));

    Mat mret = mfm(Rect(0, 0, 3, 2));
    Logger("6 mret = submatrix of mfm");    
    for( int i = 0 ; i < mret.rows; i ++)
        for(int j = 0 ; j < mret.cols; j ++)
            Logger("[%d][%d] %f ", i, j , mret.at<float>(i,j));


    Mat final;
    warpAffine(cur_query->ori_img, final, mret, Size(cur_query->ori_img.cols, cur_query->ori_img.rows));

    static int index = 0;
    char filename[30] = { 0, };        
    sprintf(filename, "saved/%2d_perspective.png", index);
    imwrite(filename, final);
}

