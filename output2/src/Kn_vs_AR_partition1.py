from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'LiveJournal'
walker_num_per_vertex = 1000

f_Kn_list = []
f_AR_list = []
f_AR_200_list = []
f_AR_400_list = []
f_Kn_list.append('../data/' + 'Kn' + '_' + 'SBM_5' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_0' + '_' + 'SBM_5' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_200_list.append('../data/' + 'AR_200' + '_' + 'SBM_5' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_400_list.append('../data/' + 'AR_400' + '_' + 'SBM_5' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_Kn_list.append('../data/' + 'Kn' + '_' + 'SBM_3' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_0' + '_' + 'SBM_3' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_200_list.append('../data/' + 'AR_200' + '_' + 'SBM_3' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_400_list.append('../data/' + 'AR_400' + '_' + 'SBM_3' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_Kn_list.append('../data/' + 'Kn' + '_' + 'SBM_6' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_0' + '_' + 'SBM_6' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_200_list.append('../data/' + 'AR_200' + '_' + 'SBM_6' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_400_list.append('../data/' + 'AR_400' + '_' + 'SBM_6' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_Kn_list.append('../data/' + 'Kn' + '_' + 'SBM_4' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_0' + '_' + 'SBM_4' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_200_list.append('../data/' + 'AR_200' + '_' + 'SBM_4' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_400_list.append('../data/' + 'AR_400' + '_' + 'SBM_4' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_Kn_list.append('../data/' + 'Kn' + '_' + 'SBM_7' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_list.append('../data/' + 'AR_0' + '_' + 'SBM_7' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_200_list.append('../data/' + 'AR_200' + '_' + 'SBM_7' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')
f_AR_400_list.append('../data/' + 'AR_400' + '_' + 'SBM_7' + '_' + str(walker_num_per_vertex) + "_RTT_" + '100' + '_loss_' + '0.03' + '.txt')

data_Kn_list = []
data_AR_list = []
data_AR_200_list = []
data_AR_400_list = []
for i in range(5):
    data_Kn_list.append(np.loadtxt(f_Kn_list[i], unpack=True))
    data_AR_list.append(np.loadtxt(f_AR_list[i], unpack=True))
    data_AR_200_list.append(np.loadtxt(f_AR_200_list[i], unpack=True))
    data_AR_400_list.append(np.loadtxt(f_AR_400_list[i], unpack=True))

y_Kn_list = []
y_AR_list = []
y_AR_200_list = []
y_AR_400_list = []
for i in range(5):
    y_Kn_list.append(np.average(data_Kn_list[i]))
    y_AR_list.append(np.average(data_AR_list[i]))
    y_AR_200_list.append(np.average(data_AR_200_list[i]))
    y_AR_400_list.append(np.average(data_AR_400_list[i]))

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([1, 2, 3, 4, 5])
data = [y_Kn_list, y_AR_list, y_AR_200_list, y_AR_400_list]

# マージンを設定
margin = 0
totoal_width = 0.4 - margin

# 棒グラフをプロット
name = ['KnightKing', 'MyMethod', 'origin + reuse = 2,000,000', 'origin + reuse = 4,000,000']
for i, h in enumerate(data):
  pos = x - totoal_width *( 1- (2*i+1)/len(data) )/2
#   if i == 0:
#       col = '#ff7f0e'
#   else:
#       col = '#9467bd'
  ax1.bar(pos, h, width = totoal_width/len(data), label = name[i])

ax1.set_xticks([1, 2, 3, 4, 5])
ax1.set_xticklabels(['1:9', '3:7', '5:5', '7:3', '9:1'])
ax1.set_xlim(0, 6)

ax1.set_xlabel('partition level')
ax1.set_ylabel('execution_time (sec)')

ax1.legend()

outname = '../picture/Kn_vs_AR_partition_level.pdf'

fig.savefig(outname)