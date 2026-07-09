# SPDX-FileCopyrightText: Copyright (c) 2023 Jose D. Montoya
#
# SPDX-License-Identifier: MIT

import time
import board
import bmi160 as BMI160
import busio


i2c = busio.I2C(board.D43, board.D44)  # uses board.SCL and board.SDA
bmi = BMI160.BMI160(i2c)

while True:
    accx, accy, accz = bmi.acceleration
    print("x:{:.2f}m/s2, y:{:.2f}m/s2, z{:.2f}m/s2".format(accx, accy, accz))
    gyrox, gyroy, gyroz = bmi.gyro
    print("x:{:.2f}°/s, y:{:.2f}°/s, z{:.2f}°/s".format(gyrox, gyroy, gyroz))
    print()
    time.sleep(0.5)

