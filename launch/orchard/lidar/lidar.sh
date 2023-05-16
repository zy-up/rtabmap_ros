# current date
timestamp=$(date +%Y_%m_%d_%H_%M_%S)
database_path="/home/sbdx/Documents/"$timestamp".db"
echo $database_path

echo -e "\033[41;32mrun Start Velodyne.launch \033[0m"
roslaunch rtabmap_ros Velodyne.launch database_path:=$database_path & sleep 4

echo -e "\033[41;32mrun Start Rviz \033[0m"
gnome-terminal --tab --active -e "rviz -d ~Codes/catkin_lidar/src/velodyne/velodyne_pointcloud/launch/velodyne.rviz"

wait
exit 0