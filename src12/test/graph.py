from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

fig = plt.figure()

ax1 = fig.add_subplot(111)

x = [0, 25000, 50000, 100000, 176468]

y1 = [86, 62, 49, 33, 25]

ax1.plot(x, y1)

# ax1.set_ylim(0, 180)

# ax1.tick_params(labelsize=14)

# ax1.legend()

# ax1.set_xlabel('network_loss (%)', fontsize=14)
# ax1.set_ylabel('execution_time (sec)', fontsize=14)

# outname = '../picture/Knight_loss.pdf'
outname = './test.pdf'

fig.savefig(outname)

