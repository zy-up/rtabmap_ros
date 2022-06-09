#!/usr/bin/python3
# -*- coding: utf-8 -*-
# generated from catkin/cmake/template/script.py.in
# creates a relay to a python script source file, acting as that file.
# The purpose is that of a symlink
<<<<<<< HEAD
python_script = '/home/zy/Code/catkin_rtab/src/rtabmap_ros/scripts/objects_to_tags.py'
=======
python_script = '/home/exdx/codes/catkin_rtab/src/rtabmap_ros/scripts/objects_to_tags.py'
>>>>>>> 8dcec0a9a1f95d57487b09c6cec170bd6adbd4be
with open(python_script, 'r') as fh:
    context = {
        '__builtins__': __builtins__,
        '__doc__': None,
        '__file__': python_script,
        '__name__': __name__,
        '__package__': None,
    }
    exec(compile(fh.read(), python_script, 'exec'), context)
