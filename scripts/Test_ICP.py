#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import time
import rospy
import numpy as np
from lib.ROS_point_cloud_pub_and_sub import PointCloudSubscriber
import lib.ICP as icp
import open3d as o3d

# Set ROS topic names for subscribing.
source_clouds = '/H_k4a/H_cloud'
target_clouds = '/M_k4a/M_cloud'


def saveCurrentRGBD(source_cloud, target_cloud):

    # 时间戳
    pcd_time = time.strftime('%Y_%m_%d_%H_%M_%S', time.localtime(time.time()))

    if not os.path.exists('./pcd/'):
        os.makedirs('./pcd')
    o3d.io.write_point_cloud('./pcd/source_cloud_'+str(pcd_time)+'.pcd', source_cloud)
    o3d.io.write_point_cloud('./pcd/target_cloud_'+str(pcd_time)+'.pcd', target_cloud)


# -- Subscribe data and print.
def main():
    # -- Set subscribers.
    source_cloud = PointCloudSubscriber(source_clouds)
    target_cloud = PointCloudSubscriber(target_clouds)

    while not rospy.is_shutdown():

        # 订阅点云并进行ICP处理
        if source_cloud.has_cloud() and target_cloud.has_cloud():
            source_cloud_o3d = source_cloud.get_cloud()
            target_cloud_o3d = target_cloud.get_cloud()
            
            source_cloud_num_points = np.asarray(source_cloud_o3d.points).shape[0]
            target_cloud_num_points = np.asarray(target_cloud_o3d.points).shape[0]
            rospy.loginfo("source_cloud: {}: target_cloud: {} .".format(source_cloud_num_points, target_cloud_num_points))

            # saveCurrentRGBD(source_cloud_o3d, target_cloud_o3d)

            source_fpfh, target_fpfh = icp.data_preprocessing(source_cloud_o3d, target_cloud_o3d, 0.02)

            result_ransac = icp.execute_global_registration(source_cloud_o3d, target_cloud_o3d,source_fpfh, target_fpfh, 0.02)

            reg_p2l = icp.execute_PtoP_registration(source_cloud_o3d, target_cloud_o3d, result_ransac.transformation)

            icp.draw_registration_result(source_cloud_o3d, target_cloud_o3d, reg_p2l.transformation)

        rospy.sleep(1.5)

if __name__ == '__main__':
    node_name = "sub_rgbd_and_cloud"
    rospy.init_node(node_name)
    print("Start sub data from driver")
    main()
    rospy.logwarn("Node `{}` stops.".format(node_name))
