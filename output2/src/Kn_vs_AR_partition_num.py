from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'SBM_3'
walker_num_per_vertex = 1000

f_Kn_list = []
f_AR_list = []

f_Kn_list.append('../data/' + '3_' + 'Kn' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + '3_' + 'AR_0' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_Kn_list.append('../data/' + '5_' + 'Kn' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + '5_' + 'AR_0' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_Kn_list.append('../data/' + '7_' + 'Kn' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + '7_' + 'AR_0' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')

data_Kn_list = []
data_AR_list = []
for i in range(3):
    data_Kn_list.append(np.loadtxt(f_Kn_list[i], unpack=True))
    data_AR_list.append(np.loadtxt(f_AR_list[i], unpack=True))

y_Kn_list = []
y_AR_list = []
for i in range(3):
    y_Kn_list.append(np.average(data_Kn_list[i]))
    y_AR_list.append(np.average(data_AR_list[i]))

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([3, 5, 7])
data = [y_Kn_list, y_AR_list]

# マージンを設定
margin = 0
totoal_width = 0.7 - margin

# 棒グラフをプロット
name = ['KnightKing', 'MyMethod']
for i, h in enumerate(data):
  pos = x - totoal_width *( 1- (2*i+1)/len(data) )/2
#   if i == 0:
#       col = '#ff7f0e'
#   else:
#       col = '#9467bd'
  ax1.bar(pos, h, width = totoal_width/len(data), label = name[i])

ax1.set_xticks([3, 5, 7])
ax1.set_xlim(2, 8)

ax1.set_xlabel('partition_num')
ax1.set_ylabel('execution_time (sec)')

ax1.legend()

outname = '../picture/Kn_vs_AR_partition_num_SBM_3.pdf'

fig.savefig(outname)