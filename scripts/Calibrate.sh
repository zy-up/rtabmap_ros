# 开启顺序开启相机，注意加延时
echo -e "\033[41;32mrun H_kinect.launch\033[0m"
roslaunch rtabmap_ros H_kinect.launch & sleep 3

echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch rtabmap_ros M_kinect.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch rtabmap_ros L_kinect.launch & sleep 3

# 开启Apriltag marker的检测
# echo -e "\033[41;32mrun H_apriltag_detect.launch\033[0m"
# roslaunch rtabmap_ros H_apriltag_detect.launch & sleep 1

# echo -e "\033[41;32mrun M_apriltag_detect.launch\033[0m"
# roslaunch rtabmap_ros M_apriltag_detect.launch & sleep 1

# echo -e "\033[41;32mrun L_apriltag_detect.launch\033[0m"
# roslaunch rtabmap_ros L_apriltag_detect.launch & sleep 1

# 利用深度图来产生点云，用于ICP校正
echo -e "\033[41;32mrun gen_pointcloud.launch\033[0m"
roslaunch rtabmap_ros gen_pointcloud.launch & sleep 3

# 开启相机外参校正:M-->H
echo -e "\033[41;32mrun Calibrate.py\033[0m"
gnome-terminal --tab --active -e "rosrun rtabmap_ros Calibrate.py -s '/M' -t '/H' "

# 开启相机外参校正:L-->M
echo -e "\033[41;32mrun Calibrate.py\033[0m"
gnome-terminal --tab --active -e "rosrun rtabmap_ros Calibrate.py -s '/L' -t '/M' "

# 开启IMU重力轴矫正
echo -e "\033[41;32mrun imu_init.py\033[0m"
gnome-terminal --tab --active -e "python /home/sbdx/Codes/catkin_rtab/src/rtabmap_ros/scripts/imu_init.py"

# 开启rviz可视化
echo "run rviz"
gnome-terminal --tab --active -e "rviz -d /home/sbdx/Codes/catkin_rtab/src/rtabmap_ros/scripts/rviz_para.rviz & sleep 2"

wait
exit 0