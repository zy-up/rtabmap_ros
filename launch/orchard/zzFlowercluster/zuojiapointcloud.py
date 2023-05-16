#!/usr/bin/env python  
# -*- coding:utf-8 -*-  

import os
import time
import rospy
import tf2_ros
import argparse
import numpy as np
import open3d as o3d
from lib.ROS_point_cloud_pub_and_sub import PointCloudSubscriber
from flower_cluster import Extract_tree, flowercluster


# 参数设置读取
parser = argparse.ArgumentParser()
parser.add_argument('-s', '--source_prefix', default='/H')

arg = parser.parse_args()

# 相机前缀
source_prefix = arg.source_prefix

# 定义相机与marker的tf订阅名
source_camera = source_prefix + '_camera_base'
source_depth = source_prefix + '_depth_camera_link'
source_marker = source_prefix + '_t1'
source_clouds = source_prefix + '_k4a' + source_prefix + '_cloud'

rospy.init_node("tf_name")

# 点云实时保存
def saveCurrentRGBD(source_cloud):
    # 时间戳
    pcd_time = time.strftime('%Y_%m_%d_%H_%M_%S', time.localtime(time.time()))

    if not os.path.exists('./pcd/'):
        os.makedirs('./pcd')
    o3d.io.write_point_cloud('./pcd/'+ source_camera +'_'+ str(pcd_time)+'.pcd', source_cloud)


if __name__ == '__main__':  
    # 初始化tf广播
    m=tf2_ros.TransformBroadcaster()    
    rate = rospy.Rate(5)

    # 初始化点云监听器
    source_cloud = PointCloudSubscriber(source_clouds)

    while not rospy.is_shutdown():  
        if source_cloud.has_cloud():
            source_cloud_o3d = source_cloud.get_cloud()
            
            flowercluster(source_cloud_o3d)

        else:
            print("点云未发布")

        rate.sleep()  
