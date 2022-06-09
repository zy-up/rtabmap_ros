execute_process(COMMAND "/home/zy/Code/catkin_rtab/src/rtabmap_ros/build/catkin_generated/python_distutils_install.sh" RESULT_VARIABLE res)

if(NOT res EQUAL 0)
  message(FATAL_ERROR "execute_process(/home/zy/Code/catkin_rtab/src/rtabmap_ros/build/catkin_generated/python_distutils_install.sh) returned error code ")
endif()
