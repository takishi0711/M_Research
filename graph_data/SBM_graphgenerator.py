import networkx as nx

# サーバ情報
ip = ["10.58.60.3", "10.58.60.5", "10.58.60.6", "10.58.60.7", "10.58.60.8"]
name = ['abilene03', 'abilene05', 'abilene06', 'abilene07', 'abilene08']

# サーバ毎のグラフサイズ
sizes = [100, 100, 100, 100, 100]

# gm間でエッジが張られる確率
probs = [[0.02, 0.02, 0.02, 0.02, 0.02], [0.02, 0.02, 0.02, 0.02, 0.02], [0.02, 0.02, 0.02, 0.02, 0.02], [0.02, 0.02, 0.02, 0.02, 0.02], [0.02, 0.02, 0.02, 0.02, 0.02]]

# SBMによるグラフ作成
g = nx.stochastic_block_model(sizes, probs, seed=0)

# ファイルに記述
for par in range(len(g.graph["partition"])):

    f = open('./SBM_100_normal/{}.txt'. format(name[par]), 'w')
    for node_id in list(g.graph["partition"][par]):
        for node_id_nex in list(g.adj[node_id]):
            f.write('{} {} {}\n'.format(node_id, node_id_nex, ip[g.nodes[node_id_nex]["block"]]))
    
    f.close()
