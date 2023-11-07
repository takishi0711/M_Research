from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'LiveJournal'

f_lil = '../data/createGraphTime_' + dataset + '_lil' + '.txt'
f_lilPointer = '../data/createGraphTime_' + dataset + '_lilPointer' + '.txt'
f_umap = '../data/createGraphTime_' + dataset + '_umap' + '.txt'
f_csr = '../data/createGraphTime_' + dataset + '_csr' + '.txt'

height1 = np.average(np.loadtxt(f_lil, unpack=True))
height2 = np.average(np.loadtxt(f_lilPointer, unpack=True))
height3 = np.average(np.loadtxt(f_umap, unpack=True))
height4 = np.average(np.loadtxt(f_csr, unpack=True))

fig = plt.figure()
ax1 = fig.add_subplot(111)

x = np.array([1, 2, 3, 4])
ax1.set_xticks([1, 2, 3, 4])
# ax1.set_xticks(x, ['lil', 'lilPointer', 'umap', 'csr'])
ax1.set_xticklabels(['lil', 'lilPointer', 'umap', 'csr'])

w = 0.4
ax1.bar(x[0], height1, width=w)
ax1.bar(x[1], height2, width=w)
ax1.bar(x[2], height3, width=w)
ax1.bar(x[3], height4, width=w)

ax1.set_xlim(0, 5)

ax1.set_ylabel('execution_time (sec)')

outname = '../picture/createGraphTime.pdf'

fig.savefig(outname)