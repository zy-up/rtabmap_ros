# 开启顺序开启相机，注意加延时
echo -e "\033[41;32mrun H_kinect.launch\033[0m"
roslaunch azure_kinect_ros_driver H_kinect.launch & sleep 3

# 利用深度图来产生点云，用于ICP校正
echo -e "\033[41;32mrun gen_pointcloud.launch\033[0m"
roslaunch azure_kinect_ros_driver gen_pointcloud.launch & sleep 3


# # current date
# timestamp=$(date +%Y_%m_%d_%H_%M_%S)
# database_path="/home/sbdx/Documents/"$timestamp".db"
# echo $database_path    

echo -e "\033[41;32mrun pointcloud.py\033[0m"
rosrun azure_kinect_ros_driver pointcloud.py -s '/H'

wait
exit 0