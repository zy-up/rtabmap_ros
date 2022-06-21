#!/usr/bin/env sh
# generated from catkin/cmake/template/local_setup.sh.in

# since this file is sourced either use the provided _CATKIN_SETUP_DIR
# or fall back to the destination set at configure time
<<<<<<< HEAD
: ${_CATKIN_SETUP_DIR:=/home/zy/Code/catkin_rtab/src/rtabmap_ros/build/devel}
=======
: ${_CATKIN_SETUP_DIR:=/home/exdx/codes/catkin_rtab/src/rtabmap_ros/build/devel}
>>>>>>> 8dcec0a9a1f95d57487b09c6cec170bd6adbd4be
CATKIN_SETUP_UTIL_ARGS="--extend --local"
. "$_CATKIN_SETUP_DIR/setup.sh"
unset CATKIN_SETUP_UTIL_ARGS
