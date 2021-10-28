import os
import cv2
import pathlib
import numpy as np
import random


datapath = "/Users/4dreplay/work/genesis/image/new"
imglist = os.listdir(datapath)
want_cnt = 10

for i in imglist : 
    if i == ".DS_Store" or i == "Thumbs.db": 
        continue

    for j in range(0, want_cnt) :
        filename = datapath +"/"+ i
        imgsrc = cv2.imread(filename)
        print(imgsrc)
        shift_x = random.uniform(-20, 20)
        shift_y = random.uniform(-30, 60)
        brightness = random.uniform(0.4, 2)
        print(filename)
        print("random shift ", shift_x, shift_y)
        M = np.float32([[1, 0, shift_x], [0, 1, shift_y]])
        sftimg = cv2.warpAffine(imgsrc, M, (imgsrc.shape[1], imgsrc.shape[0]))
        gamma = 1.0 / brightness
        table = np.array([((i / 255.0) ** gamma) * 255
		for i in np.arange(0, 256)]).astype("uint8")
        final = cv2.LUT(sftimg, table)
        newname = filename[:-4] + "_" + str(j)+ "_sfhit_"+ str(shift_x)[:6] + "_" + str(shift_y)[:6] + ".png"
        print(newname)
        cv2.imwrite(newname, final)