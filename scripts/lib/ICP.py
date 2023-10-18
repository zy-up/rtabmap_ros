#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
    Based Open3d 0.13 version for programing
'''

import open3d as o3d
import copy
import numpy as np

# 数据预处理，包括计算法线与fpfh特征点(极为耗时，需要根据需求调参)
def data_preprocessing(source, target, voxel_size, fpfh=True):
    source.estimate_normals(
        search_param=o3d.geometry.KDTreeSearchParamHybrid(radius=voxel_size*10, max_nn=30))

    target.estimate_normals(
        search_param=o3d.geometry.KDTreeSearchParamHybrid(radius=voxel_size*10, max_nn=30))

    if fpfh:
        source_fpfh = o3d.pipelines.registration.compute_fpfh_feature(
                source, o3d.geometry.KDTreeSearchParamHybrid(radius=voxel_size*10, max_nn=100))

        target_fpfh = o3d.pipelines.registration.compute_fpfh_feature(
                target, o3d.geometry.KDTreeSearchParamHybrid(radius=voxel_size*10, max_nn=100))

        return source_fpfh, target_fpfh

# 点云显示
def draw_registration_result(source, target, transformation):
    source_temp = copy.deepcopy(source)
    target_temp = copy.deepcopy(target)
    
    target_temp.paint_uniform_color([0.247, 0.8, 0.894])
    source_temp.transform(transformation)
    o3d.visualization.draw_geometries([source_temp, target_temp],)

# 全局快速粗注册，不需要提供初始变换矩阵，其结果一般不具备可靠性
def execute_global_registration(source, target, source_fpfh,target_fpfh, voxel_size):
    distance_threshold = voxel_size * 2

    result = o3d.pipelines.registration.registration_ransac_based_on_feature_matching(
        source, target, source_fpfh, target_fpfh, True,
        distance_threshold,
        o3d.pipelines.registration.TransformationEstimationPointToPoint(False),
        3, [
            o3d.pipelines.registration.CorrespondenceCheckerBasedOnEdgeLength(
                0.9),
            o3d.pipelines.registration.CorrespondenceCheckerBasedOnDistance(
                distance_threshold)
        ], o3d.pipelines.registration.RANSACConvergenceCriteria(100000, 0.999))

    # print("global_registration transformation is:")
    # print(result.transformation)

    return result

# 点对面的精细匹配
def execute_PtoP_registration(source, target, trans_init):
    threshold = 0.2
    reg_p2l = o3d.pipelines.registration.registration_icp(
        source, target, threshold, trans_init,
        o3d.pipelines.registration.TransformationEstimationPointToPlane(),
        o3d.pipelines.registration.ICPConvergenceCriteria(max_iteration=30))
    # print("PtoP_registration transformation is:")
    # print(reg_p2l.transformation)

    return reg_p2l

if __name__ == "__main__":
    voxel_size = 0.02
    trans_identity = np.identity(4)

    trans_init = [[ 0.20523679, -0.97865613, -0.01048977,  0.06844648],
                  [ 0.97853358,  0.20539236, -0.0169117,   0.3033964 ],
                  [ 0.01870526, -0.00679369,  0.99980196, -0.54422956],
                  [ 0.  ,        0.       ,   0.         , 1.        ]]


    source = o3d.io.read_point_cloud("/home/sibl/Codes/MultiKinectCalib/build/output0.ply")
    target = o3d.io.read_point_cloud("/home/sibl/Codes/MultiKinectCalib/build/output1.ply")
    
    draw_registration_result(source, target, trans_identity)

    source_fpfh, target_fpfh = data_preprocessing(source, target, voxel_size)

    result_ransac = execute_global_registration(source, target,source_fpfh, target_fpfh,voxel_size)

    draw_registration_result(source, target, result_ransac.transformation)

    reg_p2l = execute_PtoP_registration(source, target, result_ransac.transformation)
    
    draw_registration_result(source, target, reg_p2l.transformation)

    # data_preprocessing(source, target, voxel_size, False)

    # reg_p2l = execute_PtoP_registration(source, target, trans_init)

    # draw_registration_result(source, target, reg_p2l.transformation)
