import msparser
import os
import signal
import subprocess
import sys
import time
import matplotlib.pyplot as plt
from tabulate import tabulate

n_pubsub = [1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5]
topic_size = [1 << 8, 1 << 3]
stack_usage = []
heap_usage = []
total_usage = []

for t in topic_size:
    stack = [t]
    heap = [t]
    total = [t]
    for n in n_pubsub:
        agent_sp = subprocess.Popen(("valgrind --tool=massif --stacks=yes --detailed-freq=1 --max-snapshots=300 --threshold=0.0 --massif-out-file=./massif-agent.out ./install/bin/agent-profiling").split(), shell=False)
        time.sleep(1)

        client_key = 1
        sub_sps = []
        pub_sps = []
        for i in range(n):
            sub_sps.append(subprocess.Popen(["./install/bin/subscriber-profiling {} topic_name_{}".format(client_key, i)], shell=True))
            client_key += 1
            pub_sps.append(subprocess.Popen(["./install/bin/publisher-profiling {} topic_name_{} {}".format(client_key, i, t)], shell=True))
            client_key += 1

        time.sleep(5)

        for i in range(n):
            pub_sps[i].terminate()
            sub_sps[i].terminate()

        agent_sp.send_signal(signal.SIGINT)
        time.sleep(1.0)

        data = msparser.parse_file('massif-agent.out')
        peak_index = data['peak_snapshot_index']
        peak_snapshot = data['snapshots'][peak_index]
        for c in peak_snapshot['heap_tree']['children']:
            if c['details'] and c['details']['function'] == '???':
                std_heap = c['nbytes']

        stack.append(round((peak_snapshot['mem_stack'] / 1000), 2))
        heap.append(round((peak_snapshot['mem_heap'] / 1000), 2))
        total.append(round(((peak_snapshot['mem_stack'] + peak_snapshot['mem_heap'] + peak_snapshot['mem_heap_extra'] - std_heap) / 1000), 2))

    stack_usage.append(stack)
    heap_usage.append(heap)
    total_usage.append(total)

fig, ax = plt.subplots()
for i in range(len(topic_size)):
    ax.scatter(n_pubsub, total_usage[i][1:], label="topic size = {} B".format(topic_size[i]))
    ax.set_ylim(bottom=1)
ax.set_xlabel("Number of topics")
ax.set_ylabel("Memory usage (KB)")
ax.legend()
plt.show()

table_header = ["topic size (B) / #topics"]
for n in n_pubsub:
    table_header.append(n)

print(tabulate(total_usage, headers=table_header))