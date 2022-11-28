from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

filename1 = '../KnightKing_5000000_RTT_100_loss_0.txt'
filename2 = '../KnightKing_5000000_RTT_100_loss_0.005.txt'
filename3 = '../KnightKing_5000000_RTT_100_loss_0.01.txt'
filename4 = '../KnightKing_5000000_RTT_100_loss_0.05.txt'
filename5 = '../KnightKing_5000000_RTT_100_loss_0.1.txt'
filename6 = '../basic_5000000_RTT_100_loss_0_time.txt'
filename7 = '../basic_5000000_RTT_100_loss_0.005_time.txt'
filename8 = '../basic_5000000_RTT_100_loss_0.01_time.txt'
filename9 = '../basic_5000000_RTT_100_loss_0.05_time.txt'
filename10 = '../basic_5000000_RTT_100_loss_0.1_time.txt'

data1 = np.loadtxt(filename1, unpack=True)
data2 = np.loadtxt(filename2, unpack=True)
data3 = np.loadtxt(filename3, unpack=True)
data4 = np.loadtxt(filename4, unpack=True)
data5 = np.loadtxt(filename5, unpack=True)
data6 = np.loadtxt(filename6, unpack=True)
data7 = np.loadtxt(filename7, unpack=True)
data8 = np.loadtxt(filename8, unpack=True)
data9 = np.loadtxt(filename9, unpack=True)
data10 = np.loadtxt(filename10, unpack=True)

fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [0, 0.005, 0.01, 0.05, 0.1]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5)]
y2 = [np.average(data6), np.average(data7), np.average(data8), np.average(data9), np.average(data10)]

ax1.plot(x, y1, label='KnightKing')
ax1.plot(x, y2, label='MyMethod')

ax1.set_ylim(0, 180)

ax1.tick_params(labelsize=14)

ax1.legend()

ax1.set_xlabel('network_loss (%)', fontsize=14)
ax1.set_ylabel('execution_time (sec)', fontsize=14)

outname = '../picture/Knight_basic_loss.pdf'

fig.savefig(outname)

