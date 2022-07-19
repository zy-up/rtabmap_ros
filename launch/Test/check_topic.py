from __future__ import print_function
from time import sleep

import roslib
import sys
import rospy
import cv2
from std_msgs.msg import String
from sensor_msgs.msg import Image,CameraInfo
from cv_bridge import CvBridge, CvBridgeError
from rtabmap_ros.msg import RGBDImage

class image_converter:

  def __init__(self):
    self.image_sub = rospy.Subscriber("/rgbimage_topic_2",RGBDImage,self.callback)
    self.depth_sub = rospy.Subscriber("/depth_topic_2",Image,self.depthcallback)

  def depthcallback(self,data):
    print("depth:"+ str(data.header))

  def callback(self,data):
    print("rgb:"+ str(data.header))


def main(args):
  rospy.init_node('image_converter', anonymous=True)
  ic = image_converter()

  try:
    rospy.spin()
  except KeyboardInterrupt:
    print("Shutting down")
  cv2.destroyAllWindows()

if __name__ == '__main__':
    main(sys.argv)