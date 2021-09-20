#pragma once
#include "DefData.hpp"
#include "Opencv2/Core/Core.hpp"

using namespace std;
using namespace cv;

typedef int (*fn)(IMG*, IMG*, PARAM*);

int ProcessMedian(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessBlur(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessNormalize(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessAdaptiveBinarize(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessRotation(IMG* img_in, IMG* img_raw, IMG* img_out, PARAM* p);
int ProcessCropBorder(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessInvert(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessSharpen(IMG* img_in, IMG* img_out, PARAM* p);
int ProcessHistoCurve(IMG* img_in, IMG* img_out, PARAM* p);

static fn funcs[] = {
    0,
    ProcessMedian,
    ProcessNormalize,
    ProcessAdaptiveBinarize,
    ProcessCropBorder,
    ProcessInvert,
    ProcessSharpen,
};
