from cProfile import label
import numpy as np
import matplotlib.pyplot as plt
import sys

f_list = []
f_list.append('../data/drop_0.txt')
f_list.append('../data/drop_5.txt')
f_list.append('../data/drop_10.txt')
f_list.append('../data/drop_15.txt')
f_list.append('../data/drop_20.txt')

data_list = []
for i in range(5):
    data_list.append(np.loadtxt(f_list[i], unpack=True))

y_list = []
for i in range(5):
    y_list.append(np.average(data_list[i]))

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([0, 5, 10, 15, 20])

ax1.plot(x, y_list, marker='o')

ax1.hlines(0.999, 0, 20, linestyles='dotted', colors='#2ca02c')

ax1.set_ylim(0.990, 1.0)
ax1.set_xlabel('loss (%)')
ax1.set_ylabel('Average NDCG')
ax1.set_xticks([0, 5, 10, 15, 20])
ax1.set_yticks([0.990, 0.992, 0.994, 0.996, 0.998, 0.999, 1.000])

outname = '../picture/drop.pdf'

fig.savefig(outname)