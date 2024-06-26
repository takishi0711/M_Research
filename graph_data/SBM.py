import networkx as nx

# サーバ情報
ip = ["10.58.60.3", "10.58.60.5", "10.58.60.6", "10.58.60.7", "10.58.60.8"]

# サーバ毎のグラフサイズ
# sizes = [10000, 10000, 10000, 10000, 10000]
sizes = [10000, 10000, 10000, 10000, 10000, 10000, 10000]
# sizes = [10000, 10000, 10000]

# サーバ間でエッジが張られる確率
# SBM_1
# サーバ５に集中, 毎回通信
# probs = [[0, 0, 0, 0, 0.01], 
#         [0, 0, 0, 0, 0.01], 
#         [0, 0, 0, 0, 0.01], 
#         [0, 0, 0, 0, 0.01], 
#         [0.01, 0.01, 0.01, 0.01, 0]]
# SBM_2
# 集中なし, 毎回通信 0 : 10
# probs = [[0, 0.0025, 0.0025, 0.0025, 0.0025], 
#         [0.0025, 0, 0.0025, 0.0025, 0.0025], 
#         [0.0025, 0.0025, 0, 0.0025, 0.0025], 
#         [0.0025, 0.0025, 0.0025, 0, 0.0025], 
#         [0.0025, 0.0025, 0.0025, 0.0025, 0]]
# SBM_3
# 集中なし, 通信そこそこ 3 : 7
# probs = [[0.003, 0.00175, 0.00175, 0.00175, 0.00175], 
#         [0.00175, 0.003, 0.00175, 0.00175, 0.00175], 
#         [0.00175, 0.00175, 0.003, 0.00175, 0.00175], 
#         [0.00175, 0.00175, 0.00175, 0.003, 0.00175], 
#         [0.00175, 0.00175, 0.00175, 0.00175, 0.003]]
# probs = [[0.003, 0.0035, 0.0035],
#          [0.0035, 0.003, 0.0035],
#          [0.0035, 0.0035, 0.003]]
probs = [[0.003, 0.001166, 0.001166, 0.001166, 0.001166, 0.001166, 0.001166],
         [0.001166, 0.003, 0.001166, 0.001166, 0.001166, 0.001166, 0.001166],
         [0.001166, 0.001166, 0.003, 0.001166, 0.001166, 0.001166, 0.001166],
         [0.001166, 0.001166, 0.001166, 0.003, 0.001166, 0.001166, 0.001166],
         [0.001166, 0.001166, 0.001166, 0.001166, 0.003, 0.001166, 0.001166],
         [0.001166, 0.001166, 0.001166, 0.001166, 0.001166, 0.003, 0.001166],
         [0.001166, 0.001166, 0.001166, 0.001166, 0.001166, 0.001166, 0.003]]
# SBM_4
# 集中なし, 通信あまりしない 7 : 3
# probs = [[0.007, 0.00075, 0.00075, 0.00075, 0.00075], 
#         [0.00075, 0.007, 0.00075, 0.00075, 0.00075], 
#         [0.00075, 0.00075, 0.007, 0.00075, 0.00075], 
#         [0.00075, 0.00075, 0.00075, 0.007, 0.00075], 
#         [0.00075, 0.00075, 0.00075, 0.00075, 0.007]]
# SBM_5
# 集中なし, 通信 1 : 9
# probs = [[0.001, 0.00225, 0.00225, 0.00225, 0.00225], 
#         [0.00225, 0.001, 0.00225, 0.00225, 0.00225], 
#         [0.00225, 0.00225, 0.001, 0.00225, 0.00225], 
#         [0.00225, 0.00225, 0.00225, 0.001, 0.00225], 
#         [0.00225, 0.00225, 0.00225, 0.00225, 0.001]]
# SBM_6
# 集中なし, 通信 5 : 5
# probs = [[0.005, 0.00125, 0.00125, 0.00125, 0.00125], 
#         [0.00125, 0.005, 0.00125, 0.00125, 0.00125], 
#         [0.00125, 0.00125, 0.005, 0.00125, 0.00125], 
#         [0.00125, 0.00125, 0.00125, 0.005, 0.00125], 
#         [0.00125, 0.00125, 0.00125, 0.00125, 0.005]]
# SBM_7
# 集中なし, 通信 9 : 1
# probs = [[0.009, 0.00025, 0.00025, 0.00025, 0.00025], 
#         [0.00025, 0.009, 0.00025, 0.00025, 0.00025], 
#         [0.00025, 0.00025, 0.009, 0.00025, 0.00025], 
#         [0.00025, 0.00025, 0.00025, 0.009, 0.00025], 
#         [0.00025, 0.00025, 0.00025, 0.00025, 0.009]]


# SBMによるグラフ作成
g = nx.stochastic_block_model(sizes, probs, seed=0)

# ファイルに記述
f = open('./source_data/SBM_3_7.txt', 'w')
for node_id in list(g.nodes()):
    for node_id_nex in list(g.adj[node_id]):
        f.write('{} {}\n'.format(node_id, node_id_nex))