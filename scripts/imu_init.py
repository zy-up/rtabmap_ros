import numpy as np
import numpy.linalg as li
from scipy.spatial.transform import Rotation as R
import rospy
from sensor_msgs.msg import Imu

i = 0

class IMU_Init:
    def __init__(self):
        self.imu_sub = rospy.Subscriber("/L_k4a/imu", Imu, self.callback)
        self.i = 0

    def callback(self,data):
        self.i=self.i+1
        if self.i/1000 == 1:
            self.i = 0
            print(data.linear_acceleration.x)
            print(data.linear_acceleration.y)
            print(data.linear_acceleration.z)

            acc = np.array([[-data.linear_acceleration.x], [-data.linear_acceleration.y], [-data.linear_acceleration.z]])

            # z-axis.
            z_axis = acc/li.norm(acc)
            # print(z_axis)

            # x-axis.
            x_axis = np.array([[1.0], [0.0], [0.0]])
            x_axis = x_axis - np.dot((z_axis*z_axis.T), x_axis)
            x_axis = x_axis/li.norm(x_axis)
            # print(x_axis)

            # y-axis.
            y_axis = np.cross(z_axis, x_axis, axis=0)
            y_axis = y_axis/li.norm(y_axis)
            # print(y_axis)

            # 旋转矩阵
            I_R_G = np.hstack((x_axis, y_axis, z_axis))
            r3 = R.from_matrix(I_R_G.T)

            # # 转为欧拉角
            # euler_1 = r3.as_euler('xyz', degrees=True)
            # print(euler_1)

            # 转为四元数
            qua = r3.as_quat()
            print("0 0 0"+str(qua))


rospy.init_node('IMU_Init', anonymous=True)
imu = IMU_Init()

try:
    rospy.spin()    
except KeyboardInterrupt:
    print("Shutting down")


   
