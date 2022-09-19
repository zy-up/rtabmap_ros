echo -e "\033[41;32mrun statsic_tf.launch\033[0m"
roslaunch rtabmap_ros statsic_tf.launch & sleep 3

echo -e "\033[41;32mrun L_kinect.launch\033[0m"
roslaunch rtabmap_ros L_kinect.launch & sleep 3

echo -e "\033[41;32mrun M_kinect.launch\033[0m"
roslaunch rtabmap_ros M_kinect.launch & sleep 3

echo -e "\033[41;32mrun H_kinect.launch\033[0m"
roslaunch rtabmap_ros H_kinect.launch & sleep 3

# current date
timestamp="2022_03_07_14_15_37"
database_path="/home/sbdx/Documents/"$timestamp".db"

echo -e "\033[41;32mrun three_kinects_location.launch\033[0m"
gnome-terminal --tab --active -e "roslaunch rtabmap_ros three_kinects_localization.launch database_path:=$database_path"

wait
exit 0