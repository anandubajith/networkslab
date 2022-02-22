#!/usr/bin/python

import networkx as nx
import sys
import matplotlib.pyplot as plt


if ( len(sys.argv) < 2):
    print("Usage: ")
    print("./graph_helper.py <filename>")
    sys.exit(1)

filename = sys.argv[1]

with open(filename) as f:

    G = nx.Graph()

    lines = [l.strip() for l in f.readlines()]
    num_nodes, num_edges = lines[0].split(' ')
    # print(f"num_nodes={num_nodes} num_edges={num_edges}")

    for line in lines[1:]:
        src, dest, cost = line.split(' ')
        G.add_edge(src, dest, weight=cost)

    pos = nx.spring_layout(G)
    nx.draw_networkx(G, pos , node_size=400)
    labels = nx.get_edge_attributes(G,'weight')
    nx.draw_networkx_edge_labels(G,pos,edge_labels=labels)
    plt.show()
