import collections

# サーバ情報
ip = ["10.58.60.3", "10.58.60.5", "10.58.60.6", "10.58.60.7", "10.58.60.8"]
name = ['abilene03', 'abilene05', 'abilene06', 'abilene07', 'abilene08']
split_num = 5

filename = "./source_data/karate.txt"
dir = "./karate/"

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
    # adjacent_dict[to].append([frm])
# defaultdict(<class 'list'>, {1: [[2, 10]], 2: [[1, 10], [3, 10]], 3: [[2, 10]]})
# print(adjacent_dict)

# 初期化
for i in range(split_num):
    fname = dir + name[i] + ".txt"
    f = open(fname, 'w')
    f.close()

# グラフ書き込み
for u, lis in adjacent_dict.items():
    # print(u)
    # print(lis)
    fname = dir + name[int(u)%split_num] + ".txt"
    f = open(fname, 'a')
    for v in lis:
        f.write('{} {} {}\n'.format(u, v, ip[int(v)%split_num]))
    f.close()