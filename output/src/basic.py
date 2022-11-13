from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

filename1 = '../KnightKing_5000.txt'
filename2 = '../KnightKing_50000.txt'
filename3 = '../KnightKing_500000.txt'
filename4 = '../KnightKing_1000000.txt'
filename5 = '../KnightKing_2000000.txt'
filename6 = '../KnightKing_3000000.txt'
filename7 = '../KnightKing_4000000.txt'
filename8 = '../KnightKing_5000000.txt'
filename9 = '../KnightKing_10000000.txt'
filename10 = '../KnightKing_20000000.txt'
filename11 = '../KnightKing_30000000.txt'
filename12 = '../KnightKing_40000000.txt'
filename13 = '../KnightKing_50000000.txt'
filename14 = '../hard_basic_5000_time.txt'
filename15 = '../hard_basic_50000_time.txt'
filename16 = '../hard_basic_500000_time.txt'
filename17 = '../hard_basic_1000000_time.txt'
filename18 = '../hard_basic_2000000_time.txt'
filename19 = '../hard_basic_3000000_time.txt'
filename20 = '../hard_basic_4000000_time.txt'
filename21 = '../hard_basic_5000000_time.txt'
filename22 = '../hard_basic_10000000_time.txt'
filename23 = '../hard_basic_20000000_time.txt'
filename24 = '../hard_basic_30000000_time.txt'
filename25 = '../hard_basic_40000000_time.txt'
filename26 = '../hard_basic_50000000_time.txt'

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
data23 = np.loadtxt(filename23, unpack=True)
data24 = np.loadtxt(filename24, unpack=True)
data25 = np.loadtxt(filename25, unpack=True)
data26 = np.loadtxt(filename26, unpack=True)

fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [5000, 50000, 500000, 1000000, 2000000, 3000000, 4000000, 5000000, 10000000, 20000000, 30000000, 40000000, 50000000]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5), np.average(data6), np.average(data7), np.average(data8), np.average(data9), np.average(data10), np.average(data11), np.average(data12), np.average(data13)]
y2 = [np.average(data14), np.average(data15), np.average(data16), np.average(data17), np.average(data18), np.average(data19), np.average(data20), np.average(data21), np.average(data22), np.average(data23), np.average(data24), np.average(data25), np.average(data26)]

ax1.plot(x, y1, marker='.', label="KnightKing")
ax1.plot(x, y2, marker='.', label="myMethod")

ax1.legend()

ax1.set_xlabel('RW_num')
ax1.set_ylabel('execution_time (sec)')

outname = '../picture/time_basic.pdf'

fig.savefig(outname)

