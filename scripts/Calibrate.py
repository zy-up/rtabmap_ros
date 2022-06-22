#!/usr/bin/env python  
# -*- coding:utf-8 -*-  

import os
import tf
import time
import rospy
import tf2_ros
import argparse
import numpy as np
import open3d as o3d
import lib.ICP as icp
import geometry_msgs.msg
from pyquaternion import Quaternion
from lib.ROS_point_cloud_pub_and_sub import PointCloudSubscriber

# 参数设置读取
parser = argparse.ArgumentParser()
parser.add_argument('-s', '--source_prefix', default='/H')
parser.add_argument('-t', '--target_prefix', default='/M')
parser.add_argument('-m', '--mod_ICP_Maraker', type=bool, default=False)

arg = parser.parse_args()

# 模式切换 Ture:ICP+Marker | False:omly ICP
mod_ICP_Maraker = arg.mod_ICP_Maraker
# 相机前缀
source_prefix = arg.source_prefix
target_prefix = arg.target_prefix

# 定义相机与marker的tf订阅名
source_camera = source_prefix + '_camera_base'
source_depth = source_prefix + '_depth_camera_link'
source_marker = source_prefix + '_t1'
source_clouds = source_prefix + '_k4a' + source_prefix + '_cloud'

target_camera = target_prefix + '_camera_base'
target_depth = target_prefix + '_depth_camera_link'
target_marker = target_prefix + '_t1'
target_clouds = target_prefix + '_k4a' + target_prefix +'_cloud'

Init_times = 2
result_m = trans_identity = np.identity(4)

# 发布节点名称为 py_tf_broadcaster
tf_name = source_prefix[1] + '_to_' + target_prefix[1] + '_tf_broadcaster'

print(source_camera + source_depth + source_marker +source_clouds)
print(target_camera + target_depth + target_marker +target_clouds)
print(tf_name)

rospy.init_node(tf_name)

# 初始化tf信息，由source_camera指向target_camera
t = geometry_msgs.msg.TransformStamped()
t.header.frame_id = source_camera
t.header.stamp = rospy.Time.now()
t.child_frame_id = target_camera
t.transform.translation.x = 0
t.transform.translation.y = 0
t.transform.translation.z = 0
t.transform.rotation.x = 0
t.transform.rotation.y = 0
t.transform.rotation.z = 0
t.transform.rotation.w = 1

# 点云实时保存
def saveCurrentRGBD(source_cloud, target_cloud):
    # 时间戳
    pcd_time = time.strftime('%Y_%m_%d_%H_%M_%S', time.localtime(time.time()))

    if not os.path.exists('./pcd/'):
        os.makedirs('./pcd')
    o3d.io.write_point_cloud('./pcd/'+ source_camera +'_'+ str(pcd_time)+'.pcd', source_cloud)
    o3d.io.write_point_cloud('./pcd/'+ target_camera +'_'+ str(pcd_time)+'.pcd', target_cloud)

# 更新tf信息
def tf_transform(trans):
    # 重新提取位移向量与四元数
    rot_c = tf.transformations.quaternion_from_matrix(trans)
    trans_c = tf.transformations.translation_from_matrix(trans)

    # 将计算后的四元数归一化，否则将出错，注意这里的四元数为（w,x,y,z）
    rot_n = Quaternion(rot_c[3],rot_c[0],rot_c[1],rot_c[2])
    rot_n = rot_n.normalised
    
    # 跟新tf位姿信息
    t.transform.translation.x = trans_c[0]
    t.transform.translation.y = trans_c[1]
    t.transform.translation.z = trans_c[2]
    t.transform.rotation.x = rot_n[1]
    t.transform.rotation.y = rot_n[2]
    t.transform.rotation.z = rot_n[3]
    t.transform.rotation.w = rot_n[0]

    rospy.loginfo('%s to %s 的平移与四元数（x,y,z,x,y,z,w）: %f %f %f %f %f %f %f',
        source_camera,target_camera,trans_c[0],trans_c[1],trans_c[2],rot_n[1],rot_n[2],rot_n[3],rot_n[0])

if __name__ == '__main__':  
    # 初始化tf广播
    m=tf2_ros.TransformBroadcaster()    
    rate = rospy.Rate(0.3)

    # 初始化TF监听器
    listener = tf.TransformListener() 

    # 初始化点云监听器
    source_cloud = PointCloudSubscriber(source_clouds)
    target_cloud = PointCloudSubscriber(target_clouds)

    while not rospy.is_shutdown():  
        if mod_ICP_Maraker:
            # 直到在两个相机中同时检测到了April marker，才开始校正相机
            if listener.canTransform(source_camera, source_marker, rospy.Time(0)) and listener.canTransform(target_camera, target_marker, rospy.Time(0)):
                try:  
                    # 监听对应的tf,返回平移和旋转.rospy.Time(0)不表示0时刻的tf，而是指最近一帧tf 
                    print ('检测到April marker点，开始监听对应的tf')
                    (trans_s,rot_s) = listener.lookupTransform(source_camera, source_marker,  rospy.Time(0)) 
                    (trans_t,rot_t) = listener.lookupTransform(target_camera, target_marker,  rospy.Time(0))
                    (trans_sd,rot_sd) = listener.lookupTransform(source_camera, source_depth,  rospy.Time(0)) 
                    (trans_td,rot_td) = listener.lookupTransform(target_camera, target_depth,  rospy.Time(0))
                except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):  
                    print("相机tf节点发布有误")
                    continue  
                
                # rospy.loginfo('%s to %s 位置: x=%f ,y= %f，z=%f \n 旋转四元数: x= %f，y=%f z=%f, w=%f ',
                #                 source_camera,source_marker,trans_s[0],trans_s[1],trans_s[2],rot_s[0],rot_s[1],rot_s[2],rot_s[3])
                # rospy.loginfo('%s to %s 位置: x=%f ,y= %f，z=%f \n 旋转四元数: x= %f，y=%f z=%f, w=%f ',
                #                 target_camera,target_marker,trans_t[0],trans_t[1],trans_t[2],rot_t[0],rot_t[1],rot_t[2],rot_t[3])
                
                # 将返回的位姿信息转化为4*4矩阵方便运算
                m_s = listener.fromTranslationRotation(trans_s,rot_s)
                m_t = listener.fromTranslationRotation(trans_t,rot_t)
                m_sd = listener.fromTranslationRotation(trans_sd,rot_sd)
                m_td = listener.fromTranslationRotation(trans_td,rot_td)

                # 4*4矩阵相乘，注意不是点积，结果为 source_camera --> target_camera 
                m_c = np.matmul(m_s,tf.transformations.inverse_matrix(m_t))


                # 订阅点云并进行ICP处理
                if source_cloud.has_cloud() and target_cloud.has_cloud():
                    source_cloud_o3d = source_cloud.get_cloud()
                    target_cloud_o3d = target_cloud.get_cloud()
                    
                    source_cloud_num_points = np.asarray(source_cloud_o3d.points).shape[0]
                    target_cloud_num_points = np.asarray(target_cloud_o3d.points).shape[0]
                    # rospy.loginfo("source_cloud: {}: target_cloud: {} .".format(source_cloud_num_points, target_cloud_num_points))

                    # saveCurrentRGBD(source_cloud_o3d, target_cloud_o3d)
                    icp.data_preprocessing(source_cloud_o3d, target_cloud_o3d, 0.025, fpfh=False)

                    # source_depth --> target_depth
                    m_c = np.matmul(tf.transformations.inverse_matrix(m_sd), m_c)
                    m_c = np.matmul(m_c, m_td)

                    reg_p2l = icp.execute_PtoP_registration(source_cloud_o3d, target_cloud_o3d, m_c)

                    # icp.draw_registration_result(source_cloud_o3d, target_cloud_o3d, reg_p2l.transformation)

                    m_c = np.matmul(m_sd, reg_p2l.transformation)
                    m_c = np.matmul(m_c, tf.transformations.inverse_matrix(m_td))
                    m_c = tf.transformations.inverse_matrix(m_c)
                
                tf_transform(m_c)
                
            else:
                print("未检测到marker点")
        
        else:
            if source_cloud.has_cloud() and target_cloud.has_cloud():
                try:  
                    # 监听对应的tf,返回平移和旋转.rospy.Time(0)不表示0时刻的tf，而是指最近一帧tf 
                    (trans_s,rot_s) = listener.lookupTransform(source_camera, source_depth,  rospy.Time(0)) 
                    (trans_t,rot_t) = listener.lookupTransform(target_depth, target_camera,  rospy.Time(0))
                except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):  
                    print("相机tf节点发布有误")
                    continue  
                
                source_cloud_o3d = source_cloud.get_cloud()
                target_cloud_o3d = target_cloud.get_cloud()
                
                m_s = listener.fromTranslationRotation(trans_s,rot_s)
                m_t = listener.fromTranslationRotation(trans_t,rot_t)
                source_cloud_num_points = np.asarray(source_cloud_o3d.points).shape[0]
                target_cloud_num_points = np.asarray(target_cloud_o3d.points).shape[0]
                # rospy.loginfo("source_cloud: {}: target_cloud: {} .".format(source_cloud_num_points, target_cloud_num_points))
                
                # saveCurrentRGBD(source_cloud_o3d, target_cloud_o3d)
                if Init_times:
                    source_fpfh, target_fpfh = icp.data_preprocessing(source_cloud_o3d, target_cloud_o3d, 0.045)
                    result_ransac = icp.execute_global_registration(source_cloud_o3d, target_cloud_o3d,source_fpfh, target_fpfh, 0.045)
                    result_m = result_ransac.transformation
                    Init_times=Init_times-1
                else:
                    icp.data_preprocessing(source_cloud_o3d, target_cloud_o3d, 0.045, fpfh=False)
                
                result_p2l = icp.execute_PtoP_registration(source_cloud_o3d, target_cloud_o3d, result_m)
                result_m = result_p2l.transformation

                # source_camera --> source_depth  -->  target_depth  -->  target_camera
                m_c = np.matmul(m_s, result_m)
                m_c = np.matmul(m_c, m_t)
                m_c = tf.transformations.inverse_matrix(m_c)

                # icp.draw_registration_result(source_cloud_o3d, target_cloud_o3d, result_m)
                print(result_p2l)
                tf_transform(m_c)
            
            else:
                print("点云未发布")

        t.header.stamp = rospy.Time.now()
        m.sendTransform(t)

        rate.sleep()  
