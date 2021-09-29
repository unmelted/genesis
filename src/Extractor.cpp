
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
    p->focal = 3840;

    p->world = new SCENE();
    p->world->four_pt[0].x = 330;
    p->world->four_pt[0].y = 602;
    p->world->four_pt[1].x = 473;
    p->world->four_pt[1].y = 602;
    p->world->four_pt[2].x = 490;
    p->world->four_pt[2].y = 709;
    p->world->four_pt[3].x = 310;
    p->world->four_pt[3].y = 709;
    p->world->center.x = 400;
    p->world->center.y = 656;

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

    // 1st camera manual data input
    SCENE sc;
    sc.id = 0;
    sc.four_pt[0].x = 960;
    sc.four_pt[0].y = 830;
    sc.four_pt[1].x = 2638;
    sc.four_pt[1].y = 720;
    sc.four_pt[2].x = 3094;
    sc.four_pt[2].y = 1466;
    sc.four_pt[3].x = 1002;
    sc.four_pt[3].y = 1438;
    sc.center.x = 1908;
    sc.center.y = 1078;

    cal_group.push_back(sc);
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

    float minx = sc->four_pt[0].x;
    float miny = sc->four_pt[0].y;
    float maxx = sc->four_pt[0].x;
    float maxy = sc->four_pt[0].y;

    for (int i = 0; i < 4; i++)
    {
        if (minx > sc->four_pt[i].x)
            minx = sc->four_pt[i].x;
        if (miny > sc->four_pt[i].y)
            miny = sc->four_pt[i].y;
        if (maxx < sc->four_pt[i].x)
            maxx = sc->four_pt[i].x;
        if (maxy < sc->four_pt[i].y)
            maxy = sc->four_pt[i].y;
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
        sc->four_fpt[i].x = ((float)sc->four_pt[i].x - minx) * range + marginx;
        sc->four_fpt[i].y = ((float)sc->four_pt[i].y - miny) * range + marginy;
    }

    sc->normal[0][0] = (400.0 - minx) * range + marginx;
    sc->normal[0][1] = (656.0 - miny) * range + marginy;
    sc->normal[0][2] = 0.0;
    sc->normal[1][0] = (400.0 - minx) * range + marginx;
    sc->normal[1][1] = (656.0 - miny) * range + marginy;
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
    for (Mat &img : imgs)
    {
        SCENE sc;

        sc.id = index;
        sc.ori_img = img;
        sc.img = ProcessImages(img);
        int ret = GetFeature(&sc);

        if (index == 0)
        {
            is_first = true;
            SetCurTrainScene(p->world);
            SetCurQueryScene(&cal_group[index]);
        }
        else
        {
            cal_group.push_back(sc);
            SetCurTrainScene(&cal_group[index - 1]);
            SetCurQueryScene(&cal_group[index]);
            int ret = MakeMatchPair();
        }

        if (ret > 0 || sc.id == 0)
        {
            PostProcess();
        }
        else
        {
            Logger("Match Pari Fail. Can't keep going.");
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

int Extractor::CalculateCenter(SCENE *sc1, SCENE *sc2)
{
    vector<Point2f> pset1;
    vector<Point2f> pset2;
    for (int i = 0; i < 4; i++)
    {
        pset1.push_back(Point2f(float(sc1->four_pt[i].x), float(sc1->four_pt[i].y)));
        pset2.push_back(Point2f(float(sc2->four_pt[i].x), float(sc2->four_pt[i].y)));
    }

    Mat h = findHomography(pset1, pset2);
    Mat nCenter = Mat(3, 1, CV_32F);
    nCenter.at<float>(0) = sc2->center.x;
    nCenter.at<float>(1) = sc2->center.y;
    nCenter.at<float>(2) = 1;

    Mat mResult = h * nCenter;
    sc2->center.x = mResult.at<float>(0) / mResult.at<float>(2);
    sc2->center.y = mResult.at<float>(1) / mResult.at<float>(2);
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
    f_kpt = MaskKeypointWithROI(&kpt);
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

vector<KeyPoint> Extractor::MaskKeypointWithROI(vector<KeyPoint> *oip)
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

        if (ret == 0 && it != oip->end())
        {
            del++;
        }
        /*          else if (it->pt.x > 900 && it->pt.x < 1040 &&
                it->pt.y > 365 && it->pt.y < 490) {
                    Logger("except special case .. ");
        }
 */
        else
        {
            ip.push_back(*it);
            left++;
        }
    }

    Logger("new vector  %d. left %d  del %d / total ip %d ", ip.size(), left, del, total);
    return ip;
}
int Extractor::MakeMatchPair()
{

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
    for (int i = 0; i < in.size(); i++)
    {
        Logger("Distance %f imgidx %d trainidx %d queryidx %d", in[i].distance,
               in[i].imgIdx, in[i].trainIdx, in[i].queryIdx);
    }

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
#if 1
    for (vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); it++)
    {
        float tx = cur_train->ip[it->trainIdx].pt.x;
        float ty = cur_train->ip[it->trainIdx].pt.y;

        float qx = cur_query->ip[it->queryIdx].pt.x;
        float qy = cur_query->ip[it->queryIdx].pt.y;

        Logger("_pt push %f %f %f %f ", tx, ty, qx, qy);

        if ((tx > 0 && ty > 0) && (tx < cur_train->img.cols && ty < cur_train->img.rows) &&
            (qx > 0 && qy > 0) && (qx < cur_query->img.cols && qy < cur_query->img.rows))
        {
            train_pt.push_back(Point2f(tx, ty));
            query_pt.push_back(Point2f(qx, qy));
        }
    }
#else
    train_pt.push_back(Point2f(365, 559));
    query_pt.push_back(Point2f(348, 575));

    train_pt.push_back(Point2f(777, 740));
    query_pt.push_back(Point2f(813, 748));

    train_pt.push_back(Point2f(1529, 287));
    query_pt.push_back(Point2f(1460, 279));

#endif
    //Mat _h = findHomography(train_pt, query_pt, FM_RANSAC);
    //Mat _h = getAffineTransform(query_pt, train_pt);
    //Mat _h = estimateAffine2D(query_pt, train_pt);
    Mat _h = estimateRigidTransform(query_pt, train_pt, false);

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
    {
        for (int j = 0; j < _h.cols; j++)
        {
            Logger("[%d][%d] %lf ", i, j, _h.at<float>(i, j));
        }
    }

    Mat fin, fin2;
    //warpPerspective(cur_train->img, fin, _h, Size(cur_train->img.cols, cur_train->img.rows));
    warpAffine(cur_query->ori_img, fin, _h, Size(cur_query->img.cols * p->p_scale, cur_train->img.rows * p->p_scale));
    //cur_query->img = fin;
    sprintf(filename, "saved/%2d_perspective.png", index);
    imwrite(filename, fin);

    warpAffine(cur_query->img, fin2, _h, Size(cur_query->img.cols, cur_train->img.rows));
    sprintf(filename, "saved/%2d_perspective_pp.png", index);
    imwrite(filename, fin2);

    index++;
#endif
    return matches.size();
}

int Extractor::PostProcess()
{

    CalVirtualRod();
    //CalAdjustData();
    //Warping();
}

int Extractor::CalVirtualRod()
{

    if (is_first)
        SolvePnP();
    else
        SolveRnRbyH();
}

int Extractor::SolvePnP()
{
    Mat ppset1(4, 3, CV_32F);
    Mat ppset2(4, 2, CV_32F);
    for (int i = 0; i < 4; i++)
    {
        ppset1.at<float>(i, 0) = cur_train->four_fpt[i].x;
        ppset1.at<float>(i, 1) = cur_train->four_fpt[i].y;
        ppset1.at<float>(i, 2) = cur_train->four_fpt[i].z;

        ppset2.at<float>(i, 0) = (float)cur_query->four_pt[i].x;
        ppset2.at<float>(i, 1) = (float)cur_query->four_pt[i].y;

        Logger("ppset %f %f %f -- %f %f", ppset1.at<float>(i, 0), ppset1.at<float>(i, 1), ppset1.at<float>(i, 2),
               ppset2.at<float>(i, 0), ppset2.at<float>(i, 1));
    }
    Mat cm(3, 3, CV_32F, p->camera_matrix);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Logger("[%d][%d] %f -- %f ", i, j, p->camera_matrix[i*3 +j], cm.at<float>(i,j));
        }
    }

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

    Point2f angle_vec = Point2f(pnormal[1].x - pnormal[0].x, pnormal[1].y - pnormal[1].x);
    double degree = fastAtan2(angle_vec.x, angle_vec.y);
    double dnorm = norm(pnormal[0], pnormal[1]);

    if (pnormal[1].y > pnormal[0].y)
    {
        degree = 360 - degree;
        if (degree >= 360)
            degree -= 360;
    }
    else
    {
        degree = 180 - degree;
    }

    cur_query->rod_norm = dnorm;
    cur_query->rod_degree = degree;

    if (is_first)
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, 1);
    else
    {
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, (cur_train->rod_norm / cur_query->rod_norm));
    }
}

int Extractor::SolveRnRbyH()
{

    //homogeneous matrix multiply
    Mat output;
    Mat cm(3, 3, CV_32F, &p->camera_matrix);
    Mat sc(1, 4, CV_32F, &p->skew_coeff);
    Mat tvec(2, 3, CV_32F, &p->world->normal);
    Mat result_normal;
    multiply(cur_train->rod_rotation_matrix, cur_query->matrix_fromimg, output);
    int cnt = decomposeHomographyMat(output, cm,
                                     cur_query->rot_matrix, cur_query->trans_matrix, result_normal);

    if (cnt > 1)
    {
        Logger("Multiple soulution is occurred ..");
    }

    Mat projectedNormal;
    projectPoints(tvec, cur_query->rot_matrix, cur_query->trans_matrix,
                  cm, sc,
                  projectedNormal);

    Vec2f v0 = projectedNormal.at<Vec2f>(0);
    Vec2f v1 = projectedNormal.at<Vec2f>(1);
    Point2f angle_vec = Point2f(v1[0] - v0[0], v1[1] - v0[1]);
    double degree = fastAtan2(angle_vec.x, angle_vec.y);
    double dnorm = norm(v0, v1);

    if (v1[1] > v0[1])
    {
        degree = 360 - degree;
        if (degree >= 360)
            degree -= 360;
    }
    else
    {
        degree = 180 - degree;
    }

    cur_query->rod_norm = norm(v0, v1);
    cur_query->rod_degree = degree;
    if (is_first)
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, 1);
    else
    {
        cur_query->rod_rotation_matrix = getRotationMatrix2D(Point2f(cur_query->center.x, cur_query->center.y), degree, (cur_train->rod_norm / cur_query->rod_norm));
    }

#if 0
    //normal vector or grain trasform by homography
    perspectiveTransform(cur_train->normal, cur_query->normal, cur_query->matrix_fromimg);
#endif
}

int Extractor::CalAdjustData()
{
}

int Extractor::Warping()
{
    //Image Warping

    //ROI Warping
}
