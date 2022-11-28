from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

filename1 = '../KnightKing_5000000_RTT_0_loss_0.txt'
filename2 = '../KnightKing_5000000_RTT_20_loss_0.txt'
filename3 = '../KnightKing_5000000_RTT_40_loss_0.txt'
filename4 = '../KnightKing_5000000_RTT_60_loss_0.txt'
filename5 = '../KnightKing_5000000_RTT_80_loss_0.txt'
filename6 = '../KnightKing_5000000_RTT_100_loss_0.txt'
filename7 = '../KnightKing_5000000_RTT_120_loss_0.txt'
filename8 = '../KnightKing_5000000_RTT_140_loss_0.txt'
filename9 = '../KnightKing_5000000_RTT_160_loss_0.txt'
filename10 = '../KnightKing_5000000_RTT_180_loss_0.txt'
filename11 = '../KnightKing_5000000_RTT_200_loss_0.txt'

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
data11 = np.loadtxt(filename11, unpack=True)

fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5), np.average(data6), np.average(data7), np.average(data8), np.average(data9), np.average(data10), np.average(data11)]

ax1.plot(x, y1)

ax1.set_ylim(0, 180)

ax1.tick_params(labelsize=14)

# ax1.legend()

ax1.set_xlabel('RTT (ms)', fontsize=14)
ax1.set_ylabel('execution_time (sec)', fontsize=14)

outname = '../picture/Knight_RTT.pdf'

fig.savefig(outname)

