import collections
import sys

# arg[1]: source_data のファイル名, arg[2]: 分割数, arg[3]: 無効グラフなら u

filename = "./source_data/" + sys.argv[1] + ".txt"
split_num = int(sys.argv[2])
dir = "./" + sys.argv[1] + "/" + str(split_num) + "/"

# サーバ情報ファイル読み込み
f = open("./server.txt", 'r')
server_id = f.read().splitlines()
f.close()



# グラフファイル読み込み
f = open(filename, 'r')
datalist = f.readlines()
f.close()

sz = len(datalist)

edges = [list(map(int, datalist[i].split())) for i in range(sz)]

adjacent_dict = collections.defaultdict(list)
for frm, to in edges:
    # print(frm)
    # print(to)
    adjacent_dict[frm].append(to)
    if sys.argv[3] == 'u':
        adjacent_dict[to].append(frm)
    # adjacent_dict[to].append([frm])
# defaultdict(<class 'list'>, {1: [[2, 10]], 2: [[1, 10], [3, 10]], 3: [[2, 10]]})
# print(adjacent_dict)

# 初期化
for i in range(split_num):
    fname = dir + server_id[i] + ".txt"
    f = open(fname, 'w')
    f.close()

edge_count = 0
# グラフ書き込み
for u, lis in adjacent_dict.items():
    # print(u)
    # print(lis)
    fname = dir + server_id[int(u)%split_num] + ".txt"
    f = open(fname, 'a')
    for v in lis:
        f.write('{} {} {}\n'.format(u, v, int(v)%split_num))
        edge_count += 1
    f.close()

print(edge_count)
print(edge_count/2)