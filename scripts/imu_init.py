# #!/usr/bin/env python3
# # -*- coding:utf-8 -*-  

# import numpy as np
# import numpy.linalg as li
# from scipy.spatial.transform import Rotation as Rotation
# import rospy
# from sensor_msgs.msg import Imu

# i = 0

# class IMU_Init:
#     def __init__(self):
#         self.imu_sub = rospy.Subscriber("/L_k4a/imu", Imu, self.callback)
#         self.i = 0

#     def callback(self,data):
#         self.i=self.i+1
#         if self.i/1000 == 1:
#             self.i = 0
#             print(data.linear_acceleration.x)
#             print(data.linear_acceleration.y)
#             print(data.linear_acceleration.z)

#             acc = np.array([[-data.linear_acceleration.x], [-data.linear_acceleration.y], [-data.linear_acceleration.z]])

#             # z-axis.
#             z_axis = acc/li.norm(acc)
#             # print(z_axis)

#             # x-axis.
#             x_axis = np.array([[1.0], [0.0], [0.0]])
#             x_axis = x_axis - np.dot((z_axis*z_axis.T), x_axis)
#             x_axis = x_axis/li.norm(x_axis)
#             # print(x_axis)

#             # y-axis.
#             y_axis = np.cross(z_axis, x_axis, axis=0)
#             y_axis = y_axis/li.norm(y_axis)
#             # print(y_axis)

#             # 旋转矩阵
#             I_R_G = np.hstack((x_axis, y_axis, z_axis))
#             r3 = Rotation.from_matrix(I_R_G.T)

#             # # 转为欧拉角
#             # euler_1 = r3.as_euler('xyz', degrees=True)
#             # print(euler_1)

#             # 转为四元数
#             qua = r3.as_quat()
#             print("0 0 0"+str(qua))


# rospy.init_node('IMU_Init', anonymous=True)
# imu = IMU_Init()

# try:
#     rospy.spin()    
# except KeyboardInterrupt:
#     print("Shutting down")

#!/usr/bin/env python3
# -*- coding:utf-8 -*-  

import numpy as np
import numpy.linalg as li
from scipy.spatial.transform import Rotation as Rotation
import rospy
from sensor_msgs.msg import Imu

class IMU_Init:
    def __init__(self):
        self.imu_sub = rospy.Subscriber("/L_k4a/imu", Imu, self.callback)
        self.i = 0

    def callback(self,data):
        self.i += 1
        if self.i % 1000 == 0:
            # acc = np.array([data.linear_acceleration.x, data.linear_acceleration.y, data.linear_acceleration.z])
            
            # # 计算重力轴方向
            # z_axis = -(acc / li.norm(acc))

            # # 计算x轴方向 
            # x_axis = np.array([1.0, 0.0, 0.0])
            # x_axis = x_axis - np.dot(z_axis, x_axis) * z_axis
            # x_axis = x_axis / li.norm(x_axis)

            # # 计算y轴方向
            # y_axis = np.cross(z_axis, x_axis)
            # y_axis = y_axis / li.norm(y_axis)

            # # 计算旋转矩阵
            # I_R_G = np.vstack((x_axis, y_axis, z_axis)).T
            # r_G_I = Rotation.from_matrix(I_R_G)

            # # # 转为四元数并发布
            # # qua = r_G_I.as_quat()
            # # print("0 0 0 " + str(qua[0]) + " " + str(qua[1]) + " " + str(qua[2]) + " " + str(qua[3]))

            # # 转为欧拉角并发布
            # euler_angles = r_G_I.as_euler('xyz', degrees=True) 
            # print(f"x: 0 y: 0 z: 0 roll pitch yaw: {euler_angles[0]} {euler_angles[1]} {euler_angles[2]}")

            theta = np.arctan2(data.linear_acceleration.x, np.sqrt(data.linear_acceleration.y**2 + data.linear_acceleration.z**2))
            
            # Calculate roll (phi)
            phi = np.arctan2(data.linear_acceleration.y, data.linear_acceleration.z)

            psi = 0

            print(f"x: 0 y: 0 z: 0 roll pitch yaw: {phi} {theta} {psi}")

            self.i = 0

rospy.init_node('IMU_Init', anonymous=True)
imu = IMU_Init()

try:
    rospy.spin()    
except KeyboardInterrupt:
    print("Shutting down")
