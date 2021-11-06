import cv2
import numpy as np
import os
import sys
from os.path import isfile, join
import time

# this file is created by Jindong, huojindong@gmail.com
start_time = time.time()
initdirectory = os.getcwd()
fig_path = os.path.join(os.getcwd(), 'figure')   
framerate=40

video_name = 'currdensity2.avi'
currdimages = [f for f in os.listdir(fig_path) if isfile(join(fig_path, f)) and f.startswith('currD_')]  #  currdimages is a list of string  ['currD_0020.png', ...] 
#print(currdimages)
currdimages.sort(key = lambda x: int(x[6:10]))
#print('This is current density figure sorted \n', currdimages)

frame_array = []
for i in currdimages:     
    img = cv2.imread(os.path.join(fig_path, i))     # os.path.join(fig_path, i)
    frame_array.append(img)                         # inserting the frames into an image array

height, width, layers = cv2.imread(os.path.join(fig_path, currdimages[0])).shape
size = (width,height)

video = cv2.VideoWriter(video_name, cv2.VideoWriter_fourcc(*'XVID'), framerate, size)      #cv2.VideoWriter_fourcc(*'DIVX')    FourCC is a 4-byte code used to specify the video codec.
for i in range(len(frame_array)):
    video.write(frame_array[i])                          # writing to a image array
video.release()
print('This vide output is sucessful!')
print ('It totally takes {0:f} seconds. \n'.format(time.time() - start_time))
sys.exit()

# imshow() is the function that displays the image on the screen.
# cap.read() returns a bool. If frame is read correctly, it will be True.
# The function to read from an image into OpenCv is imread().
# imshow() is the function that displays the image on the screen.