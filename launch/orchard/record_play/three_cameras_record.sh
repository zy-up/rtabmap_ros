echo -e "\033[41;32mrun statsic_tf_record.launch\033[0m"
roslaunch rtabmap_ros statsic_tf_record.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch rtabmap_ros L_kinect.launch & sleep 3

echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch rtabmap_ros M_kinect.launch & sleep 3

echo -e "\033[41;32mrun H_kinect.launch\033[0m"
roslaunch rtabmap_ros H_kinect.launch & sleep 3

# current date
timestamp=$(date +%Y_%m_%d_%H_%M_%S)
database_path="/home/sbdx/Documents/"$timestamp
echo $database_path

echo -e "\033[41;32mrun bagrecord\033[0m"
gnome-terminal --tab --active -e "rosbag record -O $database_path.bag /tf /H_k4a/rgbd_image /M_k4a/rgbd_image /L_k4a/rgbd_image /L_k4a/imu /L_k4a/rgb/image_rect /L_k4a/rgb/camera_info"

# 开启rviz可视化
echo "run rviz"
gnome-terminal --tab --active -e "rviz -d /home/zy/Code/catkin_rtab/src/rtabmap_ros/launch/orchard/record_play/rviz_para_record.rviz & sleep 2"

wait
exit 0