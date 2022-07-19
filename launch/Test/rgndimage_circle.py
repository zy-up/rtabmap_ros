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
    self.rgbd_pub = rospy.Publisher("rgbimage_topic_2",RGBDImage,queue_size=1)
    self.depth_pub = rospy.Publisher("depth_topic_2",Image,queue_size=1)
    self.CameraInfo_pub = rospy.Publisher("CameraInfo_topic_2",CameraInfo,queue_size=1)

    self.bridge = CvBridge()
    self.image_sub = rospy.Subscriber("/rgbd_image",RGBDImage,self.callback)


  def callback(self,data):
    try:
      cv_image = self.bridge.imgmsg_to_cv2(data.rgb, "bgr8")
    except CvBridgeError as e:
      print(e)

    (rows,cols,channels) = cv_image.shape
    if cols > 60 and rows > 60 :
      cv2.circle(cv_image, (300,300), 100, 255, 50)

    cv2.imshow("Image window", cv_image)

    try:
      time_s = rospy.Time.now()
      print(time_s)
      image = self.bridge.cv2_to_imgmsg(cv_image, "bgr8")
      data.rgb.data = image.data
      # data.header.stamp = time_s
      # data.rgb.header.stamp = time_s
      # data.depth.header.stamp = time_s
      # data.rgb_camera_info.header.stamp = time_s
      # data.depth_camera_info.header.stamp = time_s
      self.rgbd_pub.publish(data)

      depth = data.depth
      # depth.header.stamp = time_s
      self.depth_pub.publish(depth)

      CameraI = data.rgb_camera_info
      # CameraI.header.stamp = time_s
      self.CameraInfo_pub.publish(CameraI)
      
    except CvBridgeError as e:
      print(e)

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