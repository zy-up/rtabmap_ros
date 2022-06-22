import numpy as np
import cv2
import open3d as o3d
import matplotlib.pyplot as plt

def Extract_tree(data):
    '''
    提取树枝，去除白点
    :param data:
    :return:
    '''
    # data[:, 3] = data[:, 3] / 255.0  # r
    # data[:, 4] = data[:, 4] / 255.0  # g
    # data[:, 5] = data[:, 5] / 255.0  # b

    # 灰度定义 grey = (B+G+R)/3.0
    gray = (data[:, 5] + data[:, 3] - data[:, 4])/3.0

    (minVal, maxVal, minLoc, maxLoc) = cv2.minMaxLoc(gray)

    # 转换为u8类型，进行otsu二值化
    gray_u8 = np.array((gray - minVal) / (maxVal - minVal) * 255, dtype=np.uint8)
    (thresh, bin_point) = cv2.threshold(gray_u8, -1.0, 255, cv2.THRESH_OTSU)  # -1.0
    print('OTSU threshold:', thresh)
    noise_part = data[np.where(bin_point == 255)[0]]
    tree_part = data[np.where(bin_point != 255)[0]]

    return noise_part, tree_part


def flowercluster(pcd):
    data_xyz = np.asarray(pcd.points)
    data_rgb = np.asarray(pcd.colors)
    data_xyzrgb = np.concatenate((data_xyz, data_rgb), axis=1)
    # print(data_xyzrgb)

    noise_part, tree_part = Extract_tree(data_xyzrgb)

    # print(noise_part)

    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(noise_part[:, :3])
    pcd.colors = o3d.utility.Vector3dVector(noise_part[:, 3:])

    pcd, ind = pcd.remove_radius_outlier(nb_points=300, radius=0.03)

    pcd.estimate_normals(search_param=o3d.geometry.KDTreeSearchParamHybrid(radius=0.04, max_nn=500))

    with o3d.utility.VerbosityContextManager(
            o3d.utility.VerbosityLevel.Debug) as cm:
        labels = np.array(
            pcd.cluster_dbscan(eps=0.02, min_points=50, print_progress=True))

    # print(labels)

    max_label = labels.max()
    print(f"point cloud has {max_label + 1} clusters")
    colors = plt.get_cmap("tab20")(labels / (max_label if max_label > 0 else 1))
    colors[labels < 0] = 0
    pcd.colors = o3d.utility.Vector3dVector(colors[:, :3])

    print(colors)

    o3d.visualization.draw_geometries([pcd])


pcd = o3d.io.read_point_cloud("/home/sbdx/Codes/catkin_kin/src/Azure_Kinect_ROS_Driver/launch/zuojia/pcd/M_camera_base_2022_04_25_13_49_08.pcd")
flowercluster(pcd)
