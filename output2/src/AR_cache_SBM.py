from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'SBM_1'
walker_num_per_vertex = 200

f_AR_list = []
f_AR_list.append('../data/' + 'AR_' + '200' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '400' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '600' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '800' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')

y_normal_AR = np.average(np.loadtxt('../data/' + 'AR_' + '0' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt', unpack=True))
y_Kn = np.average(np.loadtxt('../data/' + 'Kn_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt', unpack=True))

data_AR_list = []
for i in range(4):
    data_AR_list.append(np.loadtxt(f_AR_list[i], unpack=True))
  
y_AR_list = []
for i in range(4):
    y_AR_list.append(np.average(data_AR_list[i]))


fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([200, 400, 600, 800])

xmin, xmax = 0, 900

print(y_AR_list)
# ax1.bar(x, y_AR_list)
ax1.bar(x, y_AR_list, width=30, align="center", label='MyMethod (reuse edges)')

ax1.hlines(y_Kn, xmin, xmax, linestyles='dotted', colors='#2ca02c', label='KnightKing')
ax1.hlines(y_normal_AR, xmin, xmax, linestyles='dotted', colors='#ff7f0e', label='MyMethod (normal)')


ax1.set_xlim(xmin, xmax)
ax1.set_ylim(0, 100)

ax1.set_xlabel('original edges + reuse edges (x10000)')
ax1.set_ylabel('execution_time (sec)')

ax1.legend()

outname = '../picture/AR_cache_SBM_1.pdf'

fig.savefig(outname)