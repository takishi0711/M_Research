from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

filename1 = '../KnightKing_5000.txt'
filename2 = '../KnightKing_50000.txt'
filename3 = '../KnightKing_500000.txt'
filename4 = '../KnightKing_2500000.txt'
filename5 = '../KnightKing_5000000.txt'
filename6 = '../hard_5000_time.txt'
filename7 = '../hard_50000_time.txt'
filename8 = '../hard_500000_time.txt'
filename9 = '../hard_2500000_time.txt'
filename10 = '../hard_5000000_time.txt'

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

x = [1, 2, 3, 4, 5]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5)]
y2 = [np.average(data6), np.average(data7), np.average(data8), np.average(data9), np.average(data10)]

ax1.bar(x, y1, align="edge", width=-0.3, label="KnightKing")
ax1.bar(x, y2, align="edge", width=0.3, label="myMethod")

ax1.legend()

ax1.set_xticks([1, 2, 3, 4, 5])
ax1.set_xticklabels(["5,000", "50,000", "500,000", "2,500,000", "5,000,000"])

ax1.set_xlabel('RWer')
ax1.set_ylabel('execution_time (sec)')

outname = '../picture/time1.pdf'

fig.savefig(outname)

