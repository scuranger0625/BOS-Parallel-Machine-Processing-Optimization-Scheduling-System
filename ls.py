# ls.py
# List Scheduling baseline for P|prec|Cmax

import heapq

def list_scheduling(instance, m=4):
    """
    最基本的 List Scheduling：
    - 使用拓樸序 (via instance.layers)
    - 任務一有空就排
    - 無 δ-overlap
    """

    n = instance.n
    p = instance.p
    parents = instance.parents
    children = instance.children
    layers = instance.layers

    # indegree copy
    indeg = [len(parents[i]) for i in range(n)]

    # ready queue（用 FIFO 比較經典，但 FIFO/priority 都可以）
    ready = []

    # 初始 ready
    for i in range(n):
        if indeg[i] == 0:
            ready.append(i)

    busy_until = [0.0] * m
    finish_time = [0.0] * n

    # 事件 queue，存 (end, task, machine)
    event_q = []
    t = 0.0

    # ----------------------------
    # 主迴圈
    # ----------------------------
    while ready or event_q:

        # Step A: 把任務指派給空閒機器
        for j in range(m):
            if ready and busy_until[j] <= t:
                task = ready.pop(0)  # FIFO
                start = t
                end = start + p[task]
                busy_until[j] = end
                finish_time[task] = end
                heapq.heappush(event_q, (end, task, j))

        # 如果沒有事件可處理 → 全部排完
        if not event_q:
            break

        # Step B: 下一個最早完成事件
        t, u, mach = heapq.heappop(event_q)

        # Step C: 釋放 children
        for v in children[u]:
            indeg[v] -= 1
            if indeg[v] == 0:
                ready.append(v)

    Cmax = max(finish_time)
    return Cmax
