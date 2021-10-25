import os
import cv2
import pathlib
import numpy as np
import random


datapath = "/Users/kelly/work/genesis/image/test"
imglist = os.listdir(datapath)
want_cnt = 10

for i in imglist : 
    if i == ".DS_Store" : 
        continue

    for j in range(0, want_cnt) :
        filename = datapath +"/"+ i
        imgsrc = cv2.imread(filename)
        shift_x = random.uniform(-20, 20)
        shift_y = random.uniform(-30, 60)
        print(filename)
        print("random shift ", shift_x, shift_y)
        M = np.float32([[1, 0, shift_x], [0, 1, shift_y]])
        sftimg = cv2.warpAffine(imgsrc, M, (imgsrc.shape[1], imgsrc.shape[0]))
        newname = filename[:-4] + "_sfhit_"+ str(shift_x)[:5] + "_" + str(shift_y)[:5] + ".png"
        print(newname)
        cv2.imwrite(newname, sftimg)