from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

filename1 = '../basic_5000000_RTT_100_loss_0_rerun.txt'
filename2 = '../basic_5000000_RTT_100_loss_0.005_rerun.txt'
filename3 = '../basic_5000000_RTT_100_loss_0.01_rerun.txt'
filename4 = '../basic_5000000_RTT_100_loss_0.05_rerun.txt'
filename5 = '../basic_5000000_RTT_100_loss_0.1_rerun.txt'

data1 = np.loadtxt(filename1, unpack=True)
data2 = np.loadtxt(filename2, unpack=True)
data3 = np.loadtxt(filename3, unpack=True)
data4 = np.loadtxt(filename4, unpack=True)
data5 = np.loadtxt(filename5, unpack=True)

fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [0, 0.005, 0.01, 0.05, 0.1]

y1 = [np.average(data1), np.average(data2), np.average(data3), np.average(data4), np.average(data5)]

ax1.plot(x, y1, color='tab:orange')

# ax1.set_ylim(0, 180)

ax1.tick_params(labelsize=14)

# ax1.legend()

ax1.set_xlabel('network_loss (%)', fontsize=14)
ax1.set_ylabel('RWer_loss (%)', fontsize=14)

outname = '../picture/basic_loss.pdf'

fig.savefig(outname)

