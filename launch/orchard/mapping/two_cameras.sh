echo -e "\033[41;32mrun statsic_tf.launch\033[0m"
roslaunch rtabmap_ros statsic_tf_two_camera.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch rtabmap_ros L_kinect.launch & sleep 3

echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch rtabmap_ros M_kinect.launch & sleep 3

# echo -e "\033[41;32mrun H_kinect.launch\033[0m"
# roslaunch rtabmap_ros H_kinect.launch & sleep 3

echo -e "\033[41;32mrun L_apriltag_detect.launch\033[0m"
roslaunch rtabmap_ros L_apriltag_detect.launch & sleep 1

# current date
timestamp=$(date +%Y_%m_%d_%H_%M_%S)
database_path="/home/sbdx/Documents/"$timestamp".db"
echo $database_path

echo -e "\033[41;32mrun three_kinects_in_Rtabmap.launch\033[0m"
gnome-terminal --tab --active -e "roslaunch rtabmap_ros two_kinects_in_Rtabmap.launch database_path:=$database_path"

wait
exit 0