#!/bin/sh

if [ -n "$DESTDIR" ] ; then
    case $DESTDIR in
        /*) # ok
            ;;
        *)
            /bin/echo "DESTDIR argument must be absolute... "
            /bin/echo "otherwise python's distutils will bork things."
            exit 1
    esac
fi

echo_and_run() { echo "+ $@" ; "$@" ; }

<<<<<<< HEAD
echo_and_run cd "/home/zy/Code/catkin_rtab/src/rtabmap_ros"
=======
echo_and_run cd "/home/exdx/codes/catkin_rtab/src/rtabmap_ros"
>>>>>>> 8dcec0a9a1f95d57487b09c6cec170bd6adbd4be

# ensure that Python install destination exists
echo_and_run mkdir -p "$DESTDIR/usr/local/lib/python3/dist-packages"

# Note that PYTHONPATH is pulled from the environment to support installing
# into one location when some dependencies were installed in another
# location, #123.
echo_and_run /usr/bin/env \
<<<<<<< HEAD
    PYTHONPATH="/usr/local/lib/python3/dist-packages:/home/zy/Code/catkin_rtab/src/rtabmap_ros/build/lib/python3/dist-packages:$PYTHONPATH" \
    CATKIN_BINARY_DIR="/home/zy/Code/catkin_rtab/src/rtabmap_ros/build" \
    "/usr/bin/python3" \
    "/home/zy/Code/catkin_rtab/src/rtabmap_ros/setup.py" \
    egg_info --egg-base /home/zy/Code/catkin_rtab/src/rtabmap_ros/build \
    build --build-base "/home/zy/Code/catkin_rtab/src/rtabmap_ros/build" \
=======
    PYTHONPATH="/usr/local/lib/python3/dist-packages:/home/exdx/codes/catkin_rtab/src/rtabmap_ros/build/lib/python3/dist-packages:$PYTHONPATH" \
    CATKIN_BINARY_DIR="/home/exdx/codes/catkin_rtab/src/rtabmap_ros/build" \
    "/usr/bin/python3" \
    "/home/exdx/codes/catkin_rtab/src/rtabmap_ros/setup.py" \
    egg_info --egg-base /home/exdx/codes/catkin_rtab/src/rtabmap_ros/build \
    build --build-base "/home/exdx/codes/catkin_rtab/src/rtabmap_ros/build" \
>>>>>>> 8dcec0a9a1f95d57487b09c6cec170bd6adbd4be
    install \
    --root="${DESTDIR-/}" \
    --install-layout=deb --prefix="/usr/local" --install-scripts="/usr/local/bin"
