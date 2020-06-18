import msparser
import os
import signal
import subprocess
import sys
import time
from tabulate import tabulate

agent_sp = subprocess.Popen(["./install/bin/agent-profiling"], shell=True)
time.sleep(1)
sub_sp = subprocess.Popen(["./install/bin/subscriber-profiling 1 topic_name"], shell=True)
pub_sp = subprocess.Popen(("valgrind --tool=massif --stacks=yes --detailed-freq=1 --max-snapshots=300 --threshold=1.0 --massif-out-file=./massif-publisher.out ./install/bin/publisher-profiling 2 topic_name 8").split(), shell=False)

time.sleep(5)

pub_sp.send_signal(signal.SIGINT)
sub_sp.terminate()
agent_sp.terminate()
time.sleep(1)

std_heap_usage = 0
data = msparser.parse_file('massif-publisher.out')
peak_index = data['peak_snapshot_index']
peak_snapshot = data['snapshots'][peak_index]
for c in peak_snapshot['heap_tree']['children']:
    if c['details'] and c['details']['function'] == '???':
        std_heap_usage = c['nbytes']

stack_usage = round((peak_snapshot['mem_stack'] / 1000), 2)
heap_usage = round((peak_snapshot['mem_heap'] / 1000), 2)
total_usage = round(((peak_snapshot['mem_stack'] + peak_snapshot['mem_heap'] + peak_snapshot['mem_heap_extra'] - std_heap_usage) / 1000), 2)

print("stack usage: ", stack_usage)
print("heap usage: ", heap_usage)
print("total usage: ", total_usage)