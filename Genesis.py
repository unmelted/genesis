import os
import cv2
import ctypes
import pathlib
import numpy as np

datapath = 'test/'
libname = pathlib.Path().absolute() / 'libgenesis.dylib'
clib = ctypes.CDLL(libname)
print(clib)

clib.main(1)

imglist = os.listdir(datapath)
print(imglist)

for i in imglist :
    if i == ".DS_Store" :
        continue
        
    filename = datapath + i
    imgsrc = cv2.imread(filename)
    testarray1 = np.frombuffer(imgsrc, np.uint8)
    testarray2 = np.reshape(testarray1, (1024, 688, 3))
    framearray = testarray2.tobytes()
    print("CAll clib")
    clib.Feature( framearray)
