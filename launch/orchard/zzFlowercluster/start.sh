echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch azure_kinect_ros_driver zhongjianxiangji.launch & sleep 3

# 利用深度图来产生点云，用于ICP校正
echo -e "\033[41;32mrun gen_pointcloud.launch\033[0m"
roslaunch azure_kinect_ros_driver gen_pointcloud_fromMid.launch & sleep 3


echo -e "\033[41;32mrun pointcloud.py\033[0m"
rosrun azure_kinect_ros_driver zuojiapointcloud.py -s '/M'

wait
exit 0