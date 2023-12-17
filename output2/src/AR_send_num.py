from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'LiveJournal'
walker_num_per_vertex = 10

alpha = 0.2

f_AR_list = []
f_AR_list.append('../data/' + 'AR_' + '0_RWer_single' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '0_RWer_multi' + '.txt')

data_AR_list = []
for i in range(2):
    data_AR_list.append(np.loadtxt(f_AR_list[i], unpack=True))
  
y_AR_list = []
for i in range(2):
    y_AR_list.append(np.average(data_AR_list[i]))


fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([1, 2])

print(y_AR_list)
# ax1.bar(x, y_AR_list)

ax1.bar(x, y_AR_list, width=0.3, align="center")

# x2 = np.array([1400])
# ax1.bar(x2, y_normal_AR, width=300, align="center", label='MyMethod (normal)')

# ax1.hlines(y_normal_AR, xmin, xmax, linestyles='dotted', colors='#ff7f0e', label='MyMethod (normal)')

xmin, xmax = 0, 3
ax1.set_xlim(xmin, xmax)
# ax1.set_ylim(0, 150)

# ax1.set_xlabel('original edges + reuse edges (x10000)')
ax1.set_xticks([1, 2])
ax1.set_xticklabels(['single', 'multi'])
ax1.set_ylabel('execution_time (sec)')

# ax1.legend()

outname = '../picture/AR_send_num.pdf'

fig.savefig(outname)