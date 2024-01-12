from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'LiveJournal'
walker_num_per_vertex = 10

f_Kn_list = []
f_AR_list = []
for i in range(5):
    f_Kn_list.append('../data/' + 'Kn' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + str(i*50) + '_loss_' + '0.03' + '.txt')
    f_AR_list.append('../data/' + 'AR_0' + '_' + dataset + '_' + str(walker_num_per_vertex) + "_RTT_" + str(i*50) + '_loss_' + '0.03' + '.txt')

data_Kn_list = []
data_AR_list = []
for i in range(5):
    data_Kn_list.append(np.loadtxt(f_Kn_list[i], unpack=True))
    data_AR_list.append(np.loadtxt(f_AR_list[i], unpack=True))

y_Kn_list = []
y_AR_list = []
for i in range(5):
    y_Kn_list.append(np.average(data_Kn_list[i]))
    y_AR_list.append(np.average(data_AR_list[i]))

print(y_AR_list)
print(y_Kn_list)

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([0, 50, 100, 150, 200])
data = [y_Kn_list, y_AR_list]

# マージンを設定
margin = 0
totoal_width = 20 - margin

# 棒グラフをプロット
name = ['KnightKing', 'MyMethod']
for i, h in enumerate(data):
  pos = x - totoal_width *( 1- (2*i+1)/len(data) )/2
#   if i == 0:
#       col = '#ff7f0e'
#   else:
#       col = '#9467bd'
  ax1.bar(pos, h, width = totoal_width/len(data), label = name[i])

ax1.set_xticks([0, 50, 100, 150, 200])
ax1.set_xlim(-30, 230)

ax1.set_xlabel('RTT (ms)')
ax1.set_ylabel('execution_time (sec)')

ax1.legend()

outname = '../picture/Kn_vs_AR_RTT.pdf'

# fig.savefig(outname)