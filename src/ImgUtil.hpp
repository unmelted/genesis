
/*****************************************************************************
*                                                                            *
*                            ImgUtil          								 *
*                                                                            *
*   Copyright (C) 2021 By 4dreplay, Incoporated. All Rights Reserved.        *
******************************************************************************

    File Name       : ImgUtilr.hpp
    Author(S)       : Me Eunkyung
    Created         : 23 Oct 2021

    Description     : ImgUtil.Hpp
    Notes           : Utility related image.
*/

#include <filesystem>
#include "DefData.hpp"

using namespace std;
using namespace cv;

class ImgUtil {
    
    public :
        void SaveImage(SCENE* sc, int type = 0, SCENE* sc2 = 0, PARAM* p = 0, int opt = -1);
        vector<Mat>LoadImages(const string& path, vector<string>* dsc_id);     

};