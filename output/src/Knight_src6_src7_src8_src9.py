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

filename1a = '../KnightKing_5000000_RTT_0_loss_0.01.txt'
filename2a = '../KnightKing_5000000_RTT_20_loss_0.01.txt'
filename3a= '../KnightKing_5000000_RTT_40_loss_0.01.txt'
filename4a = '../KnightKing_5000000_RTT_60_loss_0.01.txt'
filename5a = '../KnightKing_5000000_RTT_80_loss_0.01.txt'
filename6a = '../KnightKing_5000000_RTT_100_loss_0.01.txt'

RW_num = '5000000'

loss = '0.01'

src6_list = []
for i in range(6):
    src6_list.append('../src6_' + RW_num + '_RTT_' + str(20 * i) + '_loss_' + loss + '_time.txt')

src7_list = []
for i in range(6):
    src7_list.append('../src7_' + RW_num + '_RTT_' + str(20 * i) + '_loss_' + loss + '_time.txt')

src8_list = []
for i in range(6):
    src8_list.append('../src8_' + RW_num + '_RTT_' + str(20 * i) + '_loss_' + loss + '_time.txt')

src9_list = []
for i in range(6):
    src9_list.append('../src9_' + RW_num + '_RTT_' + str(20 * i) + '_loss_' + loss + '_time.txt')



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

data1a = np.loadtxt(filename1a, unpack=True)
data2a = np.loadtxt(filename2a, unpack=True)
data3a = np.loadtxt(filename3a, unpack=True)
data4a = np.loadtxt(filename4a, unpack=True)
data5a = np.loadtxt(filename5a, unpack=True)
data6a = np.loadtxt(filename6a, unpack=True)

data_src6 = []
for item in src6_list:
    data_src6.append(np.loadtxt(item, unpack=True))

data_src7 = []
for item in src7_list:
    data_src7.append(np.loadtxt(item, unpack=True))

data_src8 = []
for item in src8_list:
    data_src8.append(np.loadtxt(item, unpack=True))

data_src9 = []
for item in src9_list:
    data_src9.append(np.loadtxt(item, unpack=True))



fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [0, 20, 40, 60, 80, 100]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5), np.average(data6)]

y2 = [np.average(data1a), np.average(data2a), np.average(data3a), np.average(data4a), np.average(data5a), np.average(data6a)]

y3 = []
for item in data_src6:
    y3.append(np.average(item))

y4 = []
for item in data_src8:
    y4.append(np.average(item))

y5 = []
for item in data_src9:
    y5.append(np.average(item))

y6 = []
for item in data_src7:
    y6.append(np.average(item))


ax1.plot(x, y1)
ax1.plot(x, y2)
ax1.plot(x, y3)
ax1.plot(x, y4)
ax1.plot(x, y5)
ax1.plot(x, y6)

ax1.set_ylim(0, 50)

ax1.tick_params(labelsize=14)

# ax1.legend()

ax1.plot(x, y1, label='KnightKing loss = 0')
ax1.plot(x, y2, label='KnightKing loss = 0.01')
ax1.plot(x, y4, label='single RWer')
ax1.plot(x, y3, label='port_queue')
ax1.plot(x, y5, label='dst_queue')
ax1.plot(x, y6, label='no send_queue')

ax1.legend()

ax1.set_xlabel('RTT (ms)', fontsize=14)
ax1.set_ylabel('execution_time (sec)', fontsize=14)

outname = '../picture/Knight_src6_src7_src8_src9.pdf'

fig.savefig(outname)

