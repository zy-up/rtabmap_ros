rtabmap_ros 中文注释
=======

## Build from source
This section shows how to install RTAB-Map ros-pkg on **ROS Hydro/Indigo/Jade/Kinetic/Lunar/Melodic/Noetic** (Catkin build). RTAB-Map works only with the PCL >=1.7, which is the default version installed with ROS Hydro/Indigo/Jade/Kinetic/Lunar/Melodic/Noetic (**Fuerte and Groovy are not supported**).

* The next instructions assume that you have set up your ROS workspace using this [tutorial](http://wiki.ros.org/catkin/Tutorials/create_a_workspace). I will use `noetic` prefix for convenience, but it should work with Hydro, Indigo, Jade, Kinetic, Lunar and Melodic. The workspace path is `~/catkin_ws` and your `~/.bashrc` contains:
 
    ```bash
    $ source /opt/ros/noetic/setup.bash
    $ source ~/catkin_ws/devel/setup.bash
    ```

 0. Required dependencies
     * The easiest way to get all them (Qt, PCL, VTK, OpenCV, ...) is to install/uninstall rtabmap binaries:
          ```bash
          $ sudo apt install ros-noetic-rtabmap ros-noetic-rtabmap-ros
          $ sudo apt remove ros-noetic-rtabmap ros-noetic-rtabmap-ros
          ```
 
 1. Optional dependencies
     * If you want SURF/SIFT on Indigo/Jade/Melodic/Noetic (Hydro/Kinetic has already SIFT/SURF), you have to build [OpenCV]([OpenCV](http://opencv.org/)) from source to have access to *xfeatures2d* and *nonfree* modules (note that SIFT is not in *nonfree* anymore since OpenCV 4.4.0). Install it in `/usr/local` (default) and rtabmap library should link with it instead of the one installed in ROS. 
         * On Indigo, I recommend to use latest 2.4 version ([2.4.11](https://github.com/Itseez/opencv/archive/2.4.11.zip)) and build it from source following these [instructions](http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html#building-opencv-from-source-using-cmake-using-the-command-line). RTAB-Map can build with OpenCV3+[xfeatures2d](https://github.com/Itseez/opencv_contrib/tree/master/modules/xfeatures2d) module, but `rtabmap_ros` package will have libraries conflict as `cv_bridge` is depending on OpenCV2. If you want OpenCV3+, you should build [vision-opencv](https://github.com/ros-perception/vision_opencv) package yourself (and all ros packages depending on it) so it can link on OpenCV3+.
         * On Kinetic/Melodic/Noetic, build from source with *xfeatures2d* module (and *nonfree* module if needed) the same OpenCV version already installed on the system. You will then avoid breaking `cv_bridge` with `rtabmap_ros`. If you want to install a more recent OpenCV version, I recommend to uninstall `libopencv*` libraries (with all ros packages depending on it) and rebuild all those ros packages in your catkin workspace (to make sure `cv_bridge` is linked on the OpenCV version you just compiled).
  
    * [g2o](https://github.com/RainerKuemmerle/g2o): Should be already installed by `ros-noetic-libg2o`.

    * [GTSAM](https://gtsam.org/get_started/): Install via PPA to avoid building from source. If you install from source, make sure to build with `cmake  -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF -DGTSAM_USE_SYSTEM_EIGEN=ON`.
    
    * [libpointmatcher](https://github.com/ethz-asl/libpointmatcher): **Recommended** if you are going to use lidars. Follow their [instructions](https://github.com/ethz-asl/libpointmatcher#quick-start) to install.

2. Install RTAB-Map standalone libraries. **Do not clone in your Catkin workspace**.
    ```bash
    $ cd ~
    $ git clone https://github.com/introlab/rtabmap.git rtabmap
    $ cd rtabmap/build
    $ cmake ..  [<---double dots included]
    $ make -j6
    $ sudo make install
    ```
if you meet this problem:
    ```bash
    $ rtabmap: error while loading shared libraries: librtabmap_gui.so.0.20: cannot open shared object file: No such file or directory
    ```
Run:
    ```bash
    $ sudo ldconfig
    ```


3. Install RTAB-Map ros-pkg in your src folder of your Catkin workspace.
 
    ```bash
    $ cd ~/catkin_ws
    $ git clone https://github.com/introlab/rtabmap_ros.git src/rtabmap_ros
    $ catkin_make -j4
    ```
    * Use `catkin_make -j1` if compilation requires more RAM than you have (e.g., some files require up to ~2 GB to build depending on gcc version).
    * Options:
        * Add `-DRTABMAP_SYNC_MULTI_RGBD=ON` to `catkin_make` if you plan to use multiple cameras.
        * Add `-DRTABMAP_SYNC_USER_DATA=ON` to `catkin_make` if you plan to use user data synchronized topics.
