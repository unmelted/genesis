import os
import sys
import ctypes
import pathlib
import numpy as np

datapath = 'test/'
libname = pathlib.Path().absolute() / 'libgenesis.dylib'
clib = ctypes.CDLL(libname)
print(clib)

#clib.main(1)

imglist = os.litdir(datapath)
print(imglist)

clib.TestFeature(4, imglist)
