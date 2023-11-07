from cProfile import label
import numpy as np
import matplotlib.pyplot as plt
import sys

dataset = 'LiveJournal'
walker_num_per_vertex = 10

f_lil_list = []
f_lilPointer_list = []
f_umap_list = []
f_csr_list = []
for i in range(4):
    if i == 0:
        f_lil_list.append('../data/' + dataset + '_lil' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str(1) + '.txt')
        f_lilPointer_list.append('../data/' + dataset + '_lilPointer' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str(1) + '.txt')
        f_umap_list.append('../data/' + dataset + '_umap' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str(1) + '.txt')
        f_csr_list.append('../data/' + dataset + '_csr' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str(1) + '.txt')

    f_lil_list.append('../data/' + dataset + '_lil' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str((i+1)*5) + '.txt')
    f_lilPointer_list.append('../data/' + dataset + '_lilPointer' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str((i+1)*5) + '.txt')
    f_umap_list.append('../data/' + dataset + '_umap' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str((i+1)*5) + '.txt')
    f_csr_list.append('../data/' + dataset + '_csr' + '_walkerNumPerVertex_' + str(walker_num_per_vertex) + '_threads_' + str((i+1)*5) + '.txt')

data_lil_list = []
data_lilPointer_list = []
data_umap_list = []
data_csr_list = []
for i in range(5):
    data_lil_list.append(np.loadtxt(f_lil_list[i], unpack=True))
    data_lilPointer_list.append(np.loadtxt(f_lilPointer_list[i], unpack=True))
    data_umap_list.append(np.loadtxt(f_umap_list[i], unpack=True))
    data_csr_list.append(np.loadtxt(f_csr_list[i], unpack=True))

y_lil = []
y_lilPointer = []
y_umap = []
y_csr = []
for i in range(5):
    y_lil.append(np.average(data_lil_list[i]))
    y_lilPointer.append(np.average(data_lilPointer_list[i]))
    y_umap.append(np.average(data_umap_list[i]))
    y_csr.append(np.average(data_csr_list[i]))

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([1, 5, 10, 15, 20])
data = [y_lil, y_lilPointer, y_umap, y_csr]

# マージンを設定
margin = 0.2  
totoal_width = 3 - margin

# 棒グラフをプロット
name = ['lil', 'lilPointer', 'umap', 'csr']
for i, h in enumerate(data):
  pos = x - totoal_width *( 1- (2*i+1)/len(data) )/2
  ax1.bar(pos, h, width = totoal_width/len(data), label = name[i])

ax1.set_xticks([1, 5, 10, 15, 20])
ax1.set_xlim(-3, 25)

ax1.set_xlabel('num_threads')
ax1.set_ylabel('execution_time (sec)')


ax1.legend()

outname = '../picture/4methods_theads.pdf'

fig.savefig(outname)