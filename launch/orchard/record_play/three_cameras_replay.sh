# 需要修改该文件中的bag名
gnome-terminal --tab --active -e "roslaunch rtabmap_ros remap_tf.launch & sleep 1"

echo -e "\033[41;32mrun L_apriltag_detect.launch\033[0m"
gnome-terminal --tab --active -e "roslaunch rtabmap_ros L_apriltag_detect.launch"

current date
timestamp=$(date +%Y_%m_%d_%H_%M_%S)
database_path="/home/sbdx/Documents/"$timestamp".db"
echo $database_path

echo -e "\033[41;32mrun three_kinects_in_Rtabmap.launch\033[0m"
gnome-terminal --tab --active -e "roslaunch rtabmap_ros three_kinects_in_Rtabmap.launch database_path:=$database_path"

wait
exit 0