import os
import cv2
import ctypes
import pathlib
import numpy as np
import connector as cn


""" imglist = os.listdir(datapath)
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
 """

class Calibrator(object) :
    instance = None
    bd = cn.Handler.BaseData()    
    clib = ctypes.CDLL(bd.libname)
    print(bd.libname, clib)

    @staticmethod
    def getInstance():
        if Calibrator.instance == None:
            Calibrator.instance = Calibrator()
        return Calibrator.instance

    def extract(self, ground, imgset, region) :
"""         
        dstruc = c_int * (self.bd.dim * 2 + 1)
        dd = dstruc(ground, region)
        self.clib.Extract(byref(dd))
        print("Calibrator is called ")
 """        
        pass