echo -e "\033[41;32mrun statsic_tf.launch\033[0m"
roslaunch rtabmap_ros statsic_tf.launch & sleep 3

# echo -e "\033[41;32mrun H_kinect.launch\033[0m"
# roslaunch rtabmap_ros H_kinect.launch & sleep 6

# echo -e "\033[41;32mrun M_kinect.launch\033[0m"
# roslaunch rtabmap_ros M_kinect.launch & sleep 6

# echo -e "\033[41;32mrun L_kinect.launch\033[0m"
# roslaunch rtabmap_ros L_kinect.launch & sleep 6

# echo -e "\033[41;32mrun L_apriltag_detect.launch\033[0m"
# roslaunch rtabmap_ros L_apriltag_detect.launch & sleep 1

wait
exit 0