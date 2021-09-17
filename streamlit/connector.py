import os
import pathlib

from PIL.Image import QUAD
import genesis as gn

   
class Handler(object):
    instance = None
    data = None

    @staticmethod
    def getInstance():
        if Handler.instance == None :
            Handler.instance = Handler()
        return Handler.instance

    def __init__(self):
        pass

    class BaseData() : 
        cur_path = os.getcwd() +'/'
        os.chdir('../')
        prj_path = os.getcwd() +'/'
        img_path = prj_path + 'image/'
        libname = prj_path + 'libgenesis.dylib'

        def getImageList(self):
            newlist = []
            imglist = os.listdir(self.img_path)
            imglist.sort()
            for i in imglist :
                if i == '.DS_Store':
                    continue
                newlist.append(self.img_path + i)

            return newlist