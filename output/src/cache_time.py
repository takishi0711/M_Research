from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

fc1 = '../cache_gen100_time.txt'
fc2 = '../cache_gen250_time.txt'
fc3 = '../cache_gen500_time.txt'
fc4 = '../cache_gen750_time.txt'
fc5 = '../cache_gen1000_time.txt'
fc6 = '../cache_gen2000_time.txt'
fc7 = '../cache_gen4000_time.txt'
fc8 = '../cache_gen8000_time.txt'

# fn1 = '../normal_gen100_time.txt'
# fn2 = '../normal_gen250_time.txt'
# fn3 = '../normal_gen500_time.txt'
fn4 = '../normal_gen1000_time.txt'
fn5 = '../normal_gen2000_time.txt'
fn6 = '../normal_gen4000_time.txt'
fn7 = '../normal_gen8000_time.txt'


dc1 = np.loadtxt(fc1, unpack=True)
dc2 = np.loadtxt(fc2, unpack=True)
dc3 = np.loadtxt(fc3, unpack=True)
dc4 = np.loadtxt(fc4, unpack=True)
dc5 = np.loadtxt(fc5, unpack=True)
dc6 = np.loadtxt(fc6, unpack=True)
dc7 = np.loadtxt(fc7, unpack=True)
dc7 = np.loadtxt(fc7, unpack=True)

# dn1 = np.loadtxt(fn1, unpack=True)
# dn2 = np.loadtxt(fn2, unpack=True)
# dn3 = np.loadtxt(fn3, unpack=True)
dn4 = np.loadtxt(fn4, unpack=True)
dn5 = np.loadtxt(fn5, unpack=True)
dn6 = np.loadtxt(fn6, unpack=True)
dn7 = np.loadtxt(fn7, unpack=True)


fig = plt.figure()

ax1 = fig.add_subplot(111)

x1 = [100, 250, 500, 750, 1000]
# x1 = [100, 250, 500, 1000, 2000, 4000, 8000]
x2 = [1000, 2000, 4000, 8000]

y1 = [np.average(dc1), np.average(dc2), np.average(dc3), np.average(dc4), np.average(dc5)]
# y1 = [np.average(dc1), np.average(dc2), np.average(dc3), np.average(dc4), np.average(dc5), np.average(dc6), np.average(dc7)]
y2 = [np.average(dn4), np.average(dn5), np.average(dn6), np.average(dn7)]

ax1.plot(x1, y1, label='Reuse Path')
# ax1.plot(x2, y2, label='Normal')

# ax1.set_ylim(0, 1000)

ax1.tick_params(labelsize=12)

ax1.legend()

ax1.set_xlabel('RWer_MAX', fontsize=12)
ax1.set_ylabel('execution_time (sec)', fontsize=12)

outname = '../picture/cache_time.pdf'

fig.savefig(outname)

