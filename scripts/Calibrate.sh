# 开启顺序开启相机，注意加延时
# echo -e "\033[41;32mrun H_kinect.launch\033[0m"
# roslaunch azure_kinect_ros_driver H_kinect.launch & sleep 3

echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch azure_kinect_ros_driver M_kinect.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch azure_kinect_ros_driver L_kinect.launch & sleep 3

# # 开启Apriltag marker的检测
# echo -e "\033[41;32mrun H_apriltag_detect.launch\033[0m"
# roslaunch azure_kinect_ros_driver H_apriltag_detect.launch & sleep 1

# echo -e "\033[41;32mrun M_apriltag_detect.launch\033[0m"
# roslaunch azure_kinect_ros_driver M_apriltag_detect.launch & sleep 1

# echo -e "\033[41;32mrun L_apriltag_detect.launch\033[0m"
# roslaunch azure_kinect_ros_driver L_apriltag_detect.launch & sleep 1

# 利用深度图来产生点云，用于ICP校正
echo -e "\033[41;32mrun gen_pointcloud.launch\033[0m"
roslaunch azure_kinect_ros_driver gen_pointcloud.launch & sleep 3

# 开启相机外参校正:M-->H
# echo -e "\033[41;32mrun Calibrate.py\033[0m"
# gnome-terminal --tab --active -e "rosrun azure_kinect_ros_driver Calibrate.py -s '/M' -t '/H' "

# 开启相机外参校正:L-->M
echo -e "\033[41;32mrun Calibrate.py\033[0m"
gnome-terminal --tab --active -e "rosrun azure_kinect_ros_driver Calibrate.py -s '/L' -t '/M' "

# 开启rviz可视化
echo "run rviz"
gnome-terminal --tab --active -e "rviz -d /home/sbdx/Codes/catkin_kin/src/Azure_Kinect_ROS_Driver/scripts/rviz_para.rviz & sleep 2"

wait
exit 0