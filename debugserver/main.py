#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import scipy.signal
import matplotlib
import socket
import struct
import sys

DEBUGUDP_PORT = 1234
DEBUGUDP_HOST = "0.0.0.0"

DEBUGUDP_CATEGORIES = {
	"acc" : 0
}

ACC_SIGNAL_RETENTION = 500
ACC_SIGNAL_DECIMATION = 1
ACC_SIGNAL_MAXIMUM = 0xffff / 2

acc_signal = [0] * ACC_SIGNAL_RETENTION

# Listen for UDP packets
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = (DEBUGUDP_HOST, DEBUGUDP_PORT)
print("Listening on {}, port {}".format(*server_address))
sock.bind(server_address)

# Plot acceleration signal value over time
fig = plt.figure()
acc_ax = fig.add_subplot(111)
acc_ax.set_xlim(0, ACC_SIGNAL_RETENTION)
acc_ax.set_ylim(-ACC_SIGNAL_MAXIMUM, ACC_SIGNAL_MAXIMUM)
acc_line, = acc_ax.plot(acc_signal)

def acc_signal_redraw():
	acc_line.set_ydata(acc_signal)
	plt.pause(0.001)

while True:
	data, address = sock.recvfrom(4096)

	print("received {} bytes from {}".format(len(data), address))

	if (data[0] == DEBUGUDP_CATEGORIES["acc"]):
		print("Got Accelerometer data")
		halfwords = (len(data) - 1) / 2
		acceleration = struct.unpack("<" + str(int(halfwords)) + "h", data[1:])
		acc_signal.extend(scipy.signal.decimate(acceleration, ACC_SIGNAL_DECIMATION))
		acc_signal = acc_signal[-ACC_SIGNAL_RETENTION:]
		acc_signal_redraw()
