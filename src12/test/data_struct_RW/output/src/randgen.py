from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'LiveJournal'
walker_num_per_vertex = 10

f_lilPointer_list = []
f_lilPointer_sameRandGen_list = []
for i in range(4):
    if i == 0:
        f_lilPointer_list.append('../data/' + dataset + '_lilPointer' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str(1) + '.txt')
        f_lilPointer_sameRandGen_list.append('../data/' + dataset + '_lilPointer' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str(1) + '_sameRandGen' + '.txt')

    f_lilPointer_list.append('../data/' + dataset + '_lilPointer' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str((i+1)*5) + '.txt')
    f_lilPointer_sameRandGen_list.append('../data/' + dataset + '_lilPointer' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str((i+1)*5) + '_sameRandGen' + '.txt')

data_lilPointer_list = []
data_lilPointer_sameRandGen_list = []
for i in range(5):
    data_lilPointer_list.append(np.loadtxt(f_lilPointer_list[i], unpack=True))
    data_lilPointer_sameRandGen_list.append(np.loadtxt(f_lilPointer_sameRandGen_list[i], unpack=True))

y_lilPointer = []
y_lilPointer_sameRandGen = []
for i in range(5):
    y_lilPointer.append(np.average(data_lilPointer_list[i]))
    y_lilPointer_sameRandGen.append(np.average(data_lilPointer_sameRandGen_list[i]))

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([1, 5, 10, 15, 20])
data = [y_lilPointer, y_lilPointer_sameRandGen]

# マージンを設定
margin = 0.2 
totoal_width = 2 - margin

# 棒グラフをプロット
name = ['lilPointer', 'lilPointer_sameRandGen']
for i, h in enumerate(data):
  pos = x - totoal_width *( 1- (2*i+1)/len(data) )/2
  if i == 0:
      col = '#ff7f0e'
  else:
      col = '#9467bd'
  ax1.bar(pos, h, width = totoal_width/len(data), label = name[i], color=col)
# ax1.bar(x, data[0], align="edge", width=-1)
# ax1.bar(x, data[1], align="edge", width=1)

ax1.set_xticks([1, 5, 10, 15, 20])
ax1.set_xlim(-3, 25)

ax1.set_xlabel('num_threads')
ax1.set_ylabel('execution_time (sec)')



ax1.legend()

outname = '../picture/2methods_theads.pdf'

fig.savefig(outname)