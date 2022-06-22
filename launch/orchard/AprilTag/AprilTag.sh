# # 开启顺序开启相机，注意加延时
# roslaunch azure_kinect_ros_driver H_kinect.launch & sleep 3
# echo "run H_kinect.launch"

# roslaunch azure_kinect_ros_driver M_kinect.launch & sleep 3
# echo "run M_kinect.launch"

roslaunch azure_kinect_ros_driver L_kinect.launch & sleep 3
echo "run L_kinect.launch"

# 开启Apriltag marker的检测
# roslaunch azure_kinect_ros_driver H_apriltag_detect.launch & sleep 1
# echo "run H_apriltag_detect.launch"

# roslaunch azure_kinect_ros_driver M_apriltag_detect.launch & sleep 1
# echo "run M_apriltag_detect.launch"

roslaunch azure_kinect_ros_driver L_apriltag_detect.launch & sleep 1
echo "run L_apriltag_detect.launch"


wait
exit 0