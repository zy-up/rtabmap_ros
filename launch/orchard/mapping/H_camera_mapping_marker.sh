roslaunch rtabmap_ros H_kinect.launch & sleep 3
echo "run H_kinect.launch"

roslaunch rtabmap_ros one_kinects_in_Rtabmap.launch & sleep 1
echo "run one_kinects_in_Rtabmap.launch"

roslaunch rtabmap_ros H_apriltag_detect.launch
echo "run H_apriltag_detect.launch"

wait
exit 0