# 加载相机与相机tf模型（已硬件同步，否则无法工作）
echo -e "\033[41;32mrun statsic_tf.launch\033[0m"
roslaunch rtabmap_ros statsic_tf.launch & sleep 3

echo -e "\033[41;32mrun H_kinect.launch\033[0m"
roslaunch rtabmap_ros H_kinect_Syn.launch & sleep 3

echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch rtabmap_ros M_kinect_Syn.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch rtabmap_ros L_kinect_Syn.launch & sleep 3

# 开启相机识别地面标记与点云产生
echo -e "\033[41;32mrun L_apriltag_detect.launch\033[0m"
roslaunch rtabmap_ros L_apriltag_detect.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch rtabmap_ros L_gen_pointcloud.launch & sleep 3

# 获取当前时间来保存数据，并开启重建程序
timestamp="2023_04_19_14_19_28"
database_path="/home/sbdx/Documents/"$timestamp".db"

echo -e "\033[41;32mrun three_kinects_in_Rtabmap.launch\033[0m"
gnome-terminal --tab --active -e "roslaunch rtabmap_ros three_kinects_ICP_location.launch database_path:=$database_path"

wait
exit 0