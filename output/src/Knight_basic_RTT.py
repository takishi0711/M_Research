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
filename12 = '../basic_5000000_RTT_0_loss_0_time.txt'
filename13 = '../basic_5000000_RTT_20_loss_0_time.txt'
filename14 = '../basic_5000000_RTT_40_loss_0_time.txt'
filename15 = '../basic_5000000_RTT_60_loss_0_time.txt'
filename16 = '../basic_5000000_RTT_80_loss_0_time.txt'
filename17 = '../basic_5000000_RTT_100_loss_0_time.txt'
filename18 = '../basic_5000000_RTT_120_loss_0_time.txt'
filename19 = '../basic_5000000_RTT_140_loss_0_time.txt'
filename20 = '../basic_5000000_RTT_160_loss_0_time.txt'
filename21 = '../basic_5000000_RTT_180_loss_0_time.txt'
filename22 = '../basic_5000000_RTT_200_loss_0_time.txt'

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
data12 = np.loadtxt(filename12, unpack=True)
data13 = np.loadtxt(filename13, unpack=True)
data14 = np.loadtxt(filename14, unpack=True)
data15 = np.loadtxt(filename15, unpack=True)
data16 = np.loadtxt(filename16, unpack=True)
data17 = np.loadtxt(filename17, unpack=True)
data18 = np.loadtxt(filename18, unpack=True)
data19 = np.loadtxt(filename19, unpack=True)
data20 = np.loadtxt(filename20, unpack=True)
data21 = np.loadtxt(filename21, unpack=True)
data22 = np.loadtxt(filename22, unpack=True)

fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5), np.average(data6), np.average(data7), np.average(data8), np.average(data9), np.average(data10), np.average(data11)]
y2 = [np.average(data12), np.average(data13), np.average(data14), np.average(data15), np.average(data16), np.average(data17), np.average(data18), np.average(data19), np.average(data20), np.average(data21), np.average(data22)]

ax1.plot(x, y1, label='KnightKing')
ax1.plot(x, y2, label='MyMethod')

ax1.set_ylim(0, 180)

ax1.tick_params(labelsize=15)

ax1.legend()

ax1.set_xlabel('RTT (ms)', fontsize=15)
ax1.set_ylabel('execution_time (sec)', fontsize=15)

outname = '../picture/Knight_basic_RTT.pdf'

fig.savefig(outname)

