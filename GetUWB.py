#UWBセンサデータを取得

import rospy
import numpy as np
from nlink_parser.msg import LinktrackAnchorframe0
import math
import time
import serial
import csv
import os

class UWBReader():
    def __init__(self):
        self.uwb0_x = 0.0
        self.uwb0_y = 0.0
        self.uwb0_z = 0.0

        self.uwb1_x = 0.0
        self.uwb1_y = 0.0
        self.uwb1_z = 0.0

        self.tag0_velocities = []
        self.tag1_velocities = []
        self.relative_speeds = []

        self.pre_uwb0_x = 0.0
        self.pre_uwb0_y = 0.0
        self.pre_uwb1_x = 0.0
        self.pre_uwb1_y = 0.0

        self.pre_time = time.time()

        self.ser = serial.Serial()
        self.ser.port = "/dev/ttyACM1"
        self.ser.baudrate = 115200
        self.ser.setDTR(False)
        self.ser.open()

        self.csv_filename = "20250130_exp1-3_off1.csv"
        self.csv_file = open(self.csv_filename, mode='w', newline='', encoding='utf-8')
        self.csv_writer = csv.writer(self.csv_file)
        self.csv_writer.writerow([
            "Time", "Distance", "Angle", 
            "Tag0_X", "Tag0_Y", 
            "Tag1_X", "Tag1_Y", 
            "Tag0_Velocity_X", "Tag0_Velocity_Y", 
            "Tag1_Velocity_X", "Tag1_Velocity_Y", 
            "Relative_Speed"
        ])

    def callback_uwb_multi(self, msg):
        if len(msg.nodes) != 0:
            for i in range(len(msg.nodes)):
                if msg.nodes[i].id == 2:
                    self.uwb0_x = msg.nodes[i].pos_3d[0]
                    self.uwb0_y = msg.nodes[i].pos_3d[1]
                    self.uwb0_z = msg.nodes[i].pos_3d[2]
                if msg.nodes[i].id == 3:
                    self.uwb1_x = msg.nodes[i].pos_3d[0]
                    self.uwb1_y = msg.nodes[i].pos_3d[1]
                    self.uwb1_z = msg.nodes[i].pos_3d[2]

    def calculate_velocity_vector(self, prev_x, prev_y, curr_x, curr_y, time_diff):
        if time_diff > 0:
            vx = (curr_x - prev_x) / time_diff
            vy = (curr_y - prev_y) / time_diff
            return vx, vy
        else:
            return 0.0, 0.0

    def calculate_distance_and_angle(self):
        dx = self.uwb1_x - self.uwb0_x
        dy = self.uwb1_y - self.uwb0_y
        distance = math.sqrt(dx**2 + dy**2)
        angle = math.atan2(dy, dx) * 180 / math.pi
        if angle < 0:
            angle += 360
        return distance, angle

    def calculate_speeds(self):
        current_time = time.time()
        time_diff = current_time - self.pre_time

        tag0_velocity = self.calculate_velocity_vector(self.pre_uwb0_x, self.pre_uwb0_y, self.uwb0_x, self.uwb0_y, time_diff)
        tag1_velocity = self.calculate_velocity_vector(self.pre_uwb1_x, self.pre_uwb1_y, self.uwb1_x, self.uwb1_y, time_diff)

        if not self.tag0_velocities:
            tag0_velocity = (0.0, 0.0)
            tag1_velocity = (0.0, 0.0)

        relative_speed = math.sqrt((tag1_velocity[0] - tag0_velocity[0])**2 + (tag1_velocity[1] - tag0_velocity[1])**2)

        self.tag0_velocities.append(tag0_velocity)
        self.tag1_velocities.append(tag1_velocity)
        self.relative_speeds.append(relative_speed)

        if len(self.tag0_velocities) > 5:
            self.tag0_velocities.pop(0)
            self.tag1_velocities.pop(0)
            self.relative_speeds.pop(0)

        if len(self.tag0_velocities) < 5:
            avg_tag0_velocity = tag0_velocity
            avg_tag1_velocity = tag1_velocity
            avg_relative_speed = relative_speed
        else:
            avg_tag0_velocity = (
                sum(v[0] for v in self.tag0_velocities) / len(self.tag0_velocities),
                sum(v[1] for v in self.tag0_velocities) / len(self.tag0_velocities)
            )
            avg_tag1_velocity = (
                sum(v[0] for v in self.tag1_velocities) / len(self.tag1_velocities),
                sum(v[1] for v in self.tag1_velocities) / len(self.tag1_velocities)
            )
            avg_relative_speed = sum(self.relative_speeds) / len(self.relative_speeds)

        self.pre_uwb0_x = self.uwb0_x
        self.pre_uwb0_y = self.uwb0_y
        self.pre_uwb1_x = self.uwb1_x
        self.pre_uwb1_y = self.uwb1_y
        self.pre_time = current_time

        return avg_tag0_velocity, avg_tag1_velocity, avg_relative_speed

    def main(self):
        rospy.init_node('uwb_node', anonymous=True)
        rate = rospy.Rate(100) #100HZで実行
        sub_uwb = rospy.Subscriber("/nlink_linktrack_anchorframe0", LinktrackAnchorframe0, self.callback_uwb_multi)
        self.ser.write("1\n".encode())
        time.sleep(1) 
        os.system('echo -e "\a"')

        try:
            while not rospy.is_shutdown():
                distance, angle = self.calculate_distance_and_angle()
                avg_tag0_velocity, avg_tag1_velocity, avg_relative_speed = self.calculate_speeds()
                current_time = time.time()

                self.csv_writer.writerow([
                    current_time, distance, angle,
                    self.uwb0_x, self.uwb0_y,
                    self.uwb1_x, self.uwb1_y,
                    avg_tag0_velocity[0], avg_tag0_velocity[1],
                    avg_tag1_velocity[0], avg_tag1_velocity[1],
                    avg_relative_speed
                ])


                # 接近判定モデルに基づく接近判定
                tag0_speed_magnitude = math.sqrt(avg_tag0_velocity[0]**2 + avg_tag0_velocity[1]**2)
                tag1_speed_magnitude = math.sqrt(avg_tag1_velocity[0]**2 + avg_tag1_velocity[1]**2)

                if tag0_speed_magnitude >= 0.5 and tag1_speed_magnitude >= 0.5 and avg_relative_speed >= 0.5:
                    if distance < 5:
                        self.ser.write(f"{angle}\n".encode())
                elif distance < 3:
                    self.ser.write(f"{angle}\n".encode())

                rate.sleep()
        except KeyboardInterrupt:
            self.ser.close()
            self.csv_file.close()

if __name__ == "__main__":
    re = UWBReader()
    re.main()
