from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

dataset = 'LiveJournal'
walker_num_per_vertex = 10

alpha = 0.2

f_AR_list = []
f_AR_list.append('../data/' + 'AR_' + '2000' + '_' + 'cache_RWer_num' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '3000' + '_' + 'cache_RWer_num' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '4000' + '_' + 'cache_RWer_num' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '5000' + '_' + 'cache_RWer_num' + '.txt')
f_AR_list.append('../data/' + 'AR_' + '6000' + '_' + 'cache_RWer_num' + '.txt')


data_AR_list = []
for i in range(5):
    data_AR_list.append(np.loadtxt(f_AR_list[i], unpack=True))
  
y_AR_list = []
for i in range(5):
    y_AR_list.append(np.average(data_AR_list[i]))




fig = plt.figure()
ax1 = fig.add_subplot(111)

origin = 1400
x = np.array([2000 - origin, 3000 - origin, 4000 - origin, 5000 - origin, 6000 - origin])

for i in range(5):
    y_AR_list[i] /= 1000000
    # x[i] /= 100

# xmin, xmax = 0, 8000

print(y_AR_list)
# ax1.bar(x, y_AR_list)

ax1.bar(x, y_AR_list, width=300, align="center")

# x2 = np.array([1400])
# ax1.bar(x2, y_normal_AR, width=300, align="center", label='MyMethod (normal)')

# ax1.hlines(y_normal_AR, xmin, xmax, linestyles='dotted', colors='#ff7f0e', label='MyMethod (normal)')


# ax1.set_xlim(xmin, xmax)
ax1.set_xticks([1000, 2000, 3000, 4000, 5000])
ax1.set_xticklabels(['10', '20', '30', '40', '50'])
ax1.set_xlabel('reuse edges (x1,000,000)')
ax1.set_ylabel('needed_execution_num (x1,000,000)')

# ax1.legend()

outname = '../picture/AR_cache_RWer_num.pdf'

fig.savefig(outname)