#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "ImageProcess.hpp"
#include "Image.hpp"
#include "os_api.hpp"

using namespace cv;
using namespace std;

int Handle(int prc, int* pi, int* pr_arr, IMG* img_in, IMG* img_out, PARAM* p)
{
    int param1 = 0;
    int param2 = 0;
    int result;
    IMG* temp1 = CreateImage(img_in->width, img_in->height);
    IMG* temp2 = CreateImage(img_in->width, img_in->height);

    for(int i = 0 ; i < prc ; i ++)
    {
        param1 = pr_arr[i*2];
        param2 = pr_arr[i*2+1];

        if(i == 0)
        {
            CopyImage(img_in, temp1);
        }
        else
        {
            CopyImage(temp2, temp1);
            ClearImage(temp2);
        }
        result = funcs[pi[i]](temp1, temp2, p);

// #ifdef _DEBUG
//         char filename[50];
//         sprintf(filename, "out/%s_%d_%d.bmp",dname, result, i);
//         DumpImage(filename, temp2);
// #endif

        if(result != ERR_NONE)
        {
            Logger("Error result : %d  at [%d]\n", result, i);
            DestroyImage(temp1);
            DestroyImage(temp2);
            return result;
        }
    }

    CopyImage(temp2, img_out);
    DestroyImage(temp1);
    DestroyImage(temp2);
    return ERR_NONE;
}

void InsertionSort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

/* param1 = kernel size */
int ProcessMedian(IMG* img_in, IMG* img_out, PARAM* p)
{
    (void)p;
    int result = ERR_NONE;
    int kernel = 3;
    int kernel_t = kernel*kernel;
    int h_kernel = kernel/2;
    int* arr = (int *)g_os_malloc(sizeof(int)* kernel_t);

    for(int i = h_kernel; i < img_in->height - h_kernel; i++)
    {
        for(int j = h_kernel; j < img_in->width - h_kernel; j++)
        {

            memset(arr, 0, sizeof(int)* kernel_t);
            int t = 0;
            for(int k = -1; k < kernel -1; k ++, t++)
            {
                for(int l = -1; l < kernel - 1; l++, t++)
                {
                    arr[t] = img_in->buf[(i + k) * img_in->width + (j + l)];
                }
            }

            InsertionSort(arr,kernel_t);
            img_out->buf[i*img_in->width + j] = arr[kernel_t/2];
        }
    }

    g_os_free(arr);
    return result;
}

//no param
int ProcessNormalize(IMG* img_in, IMG* img_out, PARAM* p)
{
    (void)p;
    int result = ERR_NONE;
    int h_max = WH;
    int histogram[256] = { 0, };
    int max_freq = 0;
    int max_val = 0;
    int min = 0;
    int max = 0;
    long sum = 0;
    int mean = 0;

    for(int i = 0; i < img_in->height; i ++)
    {
        for(int j = 0 ; j < img_in->width ; j ++)
        {
            histogram[img_in->buf[i * img_in->width + j]]++;
            sum += img_in->buf[i * img_in->width + j];
//            Logger(" %d , ", img_in->buf[i * img_in->width + j]);
        }
//        Logger("\n");
    }

    mean = sum / img_in->area;

    int min_peak = 0;
    int max_peak = 0;

    for(int k = 0 ; k < h_max; k ++)
    {
        if(histogram[k] > min_peak && k < mean && min == 0)
        {
            min = k; min_peak = histogram[k];
        }

        if(histogram[k] > max_peak && k > mean && max == 0)
        {
            max = k; max_peak = histogram[k];
        }

        if(max_val < histogram[k])
        {
            max_freq = k;
            max_val = histogram[k];
//            Logger(" k : %d, max_freq : %d, val %d \n", k, max_freq, max_val);
        }
    }


    Logger("normalize min %d max %d freq %d maxval %d // mean %d \n",
           min, max, max_freq, max_val, mean);

    //own idea
    // if(max_freq - min  < 40)
    //     max_freq += 40;

//    max = max_freq;
    int diff = max - min;

    Logger(" diff %d \n", diff);
    //min - max normalize
    for(int i = 0; i < img_in->height; i ++)
    {
        for(int j = 0 ; j < img_in->width ; j ++)
        {
            int val = (img_in->buf[i *img_in->width + j] - min) *255 / diff;
            if(val < 0) val = 0;
            else if(val > 255) val = 255;
//            Logger(" %d -> %d \n", img_in->buf[i *img_in->width + j], val);
            img_out->buf[i *img_in->width + j] = val;
        }
    }

    //z score normalize ?

    return result;

}

int ProcessInvert(IMG* img_in, IMG* img_out, PARAM* p)
{
    (void)p;
    int result = ERR_NONE;

    for(int i = 0; i < img_in->height; i ++)
    {
        for(int j = 0 ; j < img_in->width ; j ++)
        {
            img_out->buf[i *img_in->width + j] = 255 - (img_in->buf[i *img_in->width + j]);
        }
    }

    return result;
}

//prama1 - threshold %
int ProcessAdaptiveBinarize(IMG* img_in, IMG* img_out, PARAM* p)
{
    (void)p;
    int result = ERR_NONE;
    int h_max = 256;
    int histogram[256] = { 0, };
    // int max_freq = 0;
    int sum = 0;
    // int total = 0;
    // int min = 0;
    // int max = 0;
    int threshold = 0;
    int param1 = 10;

    for(int i = 0; i < img_in->height; i ++)
    {
        for(int j = 0 ; j < img_in->width ; j ++)
        {
            histogram[img_in->buf[i * img_in->width + j]]++;
        }
    }

    for(int k = 0 ; k < h_max; k ++)
    {
        sum += histogram[k];
        if(sum > (img_in->area * param1 / 100))
        {
            threshold = k;
            break;
        }
    }

    Logger("param1 %d threshold %d \n", param1, threshold);

    for(int i = 0; i < img_in->height; i ++)
    {
        for(int j = 0 ; j < img_in->width ; j ++)
        {
            int val = img_in->buf[i * img_in->width + j];
            img_out->buf[i * img_in->width + j] = (val > threshold ? 255 : val);
        }
    }

    return result;
}

//param1 - angle
int ProcessRotation(IMG* img_in, IMG* img_raw, IMG* img_out, PARAM* p)
{
    int result = ERR_NONE;   
/*     int rotate = 360 - p->must_rotate;
    int rarea = p->must_rotate / 90;

    p->mes->onepin -= rarea;
    if(p->mes->onepin <= 0) p->mes->onepin += 4;

    RotateImage(img_in,rotate);
    RotateImage(img_raw,rotate);
    ChangeCanvas(img_out, img_in->width, img_in->height);
 */
    return result;
}

//param1 = border size : def 10px.


int ProcessCropBorder(IMG* img_in, IMG* img_out, PARAM* p)
{
    (void)p;
    int result = ERR_NONE;
    int param1 = 10;

    CopyImage(img_in, img_out);
    memset(&img_out->buf[0], 0, sizeof(unsigned char)*img_out->width*param1);
    memset(&img_out->buf[img_out->width * (img_out->height - param1)], 0, sizeof(unsigned char)*img_out->width*param1);

    for(int i = param1 ; i < img_in->height - param1 + 1; i ++)
    {
        memset(&img_out->buf[i*img_out->width], 0, sizeof(unsigned char)*param1);
        memset(&img_out->buf[i*img_out->width - param1], 0, sizeof(unsigned char)*param1);
    }

    return result;
}

// int ProcessBilateral(IMG* img_in, IMG* img_out,PARAM* p)
// {
//     float sigma_spatial = 0;
//     float sigma_range = 0;

//     float * buffer = (float *)g_malloc(sizeof(float) *
//                      ((img_in->width * img_in->height +
//                        img_in->width * img_in->height + img_in->width + img_in->width) * 2));

//     recursive_bf(img_in->buf, img_out->buf,
//                  sigma_spatial, sigma_range,
//                  img_in->width, img_in->height, 1,
//                  buffer);

//     delete buffer;

// }

int ProcessSharpen(IMG* img_in, IMG* img_out, PARAM* p)
{
    (void)p;
    int kernel = 3;
    int conv[3][3] = {
        {-1,  -1, -1},
        {-1,  12, -1},
        {-1,  -1, -1}   };

    for(int i = 1 ; i < img_in->height - 1 ; i++)
    {
        for(int j = 1 ; j < img_in->width - 1 ; j++)
        {
            int pt = 0;
            for(int ky = -1 ; ky < kernel-1 ; ky ++)
                for(int kx = -1; kx < kernel-1 ; kx ++)
                    pt += conv[ky+1][kx+1] * (img_in->buf[(i + ky)*img_in->width + (j + kx)]);
            Logger(" %3d ", pt);

            img_out->buf[i * img_in->width + j] = (pt > 255 ? 255 : pt) < 0 ? 0 : pt;
        }
    }

    return ERR_NONE;
}

