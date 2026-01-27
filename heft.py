# heft.py
# HEFT baseline for DAG Scheduling (Simplified for homogeneous machines)

import heapq

def compute_rank_u(instance):
    """
    HEFT 的 rank_u 計算方式：
    rank_u[i] = p[i] + max(rank_u[child])  (從 bottom-up)
    """

    n = instance.n
    p = instance.p
    children = instance.children
    layers = instance.layers
    H = instance.H

    rank_u = [0.0] * n

    # bottom-up
    for h in reversed(range(H)):
        for u in layers[h]:
            if len(children[u]) == 0:
                rank_u[u] = p[u]
            else:
                rank_u[u] = p[u] + max(rank_u[v] for v in children[u])

    return rank_u



def heft(instance, m=4):
    """
    HEFT Baseline
    1. 計算 rank_u
    2. 用 max-heap 依 rank_u 指派任務
    3. 依 machine earliest finish time 分配
    """

    n = instance.n
    p = instance.p
    parents = instance.parents
    children = instance.children

    # ------------------------------------------------
    # Step 1: 計算 rank_u（HEFT 的精髓）
    # ------------------------------------------------
    rank_u = compute_rank_u(instance)

    # ------------------------------------------------
    # Step 2: indegree + 初始化 ready pool
    # ------------------------------------------------
    indeg = [len(parents[i]) for i in range(n)]
    ready = []

    for i in range(n):
        if indeg[i] == 0:
            heapq.heappush(ready, (-rank_u[i], i))  # max-heap

    # ------------------------------------------------
    # Step 3: 機器狀態
    # ------------------------------------------------
    busy_until = [0.0] * m
    finish_time = [0.0] * n
    event_q = []
    t = 0.0

    # ------------------------------------------------
    # Step 4: 主迴圈（與 BOS 類似，但排序依 rank_u）
    # ------------------------------------------------
    while ready or event_q:

        # A: 指派任務給空閒機器
        for j in range(m):
            if ready and busy_until[j] <= t:
                _, task = heapq.heappop(ready)

                start = t
                end = start + p[task]
                busy_until[j] = end
                finish_time[task] = end

                heapq.heappush(event_q, (end, task, j))

        # 若沒有更多事件，結束
        if not event_q:
            break

        # B: 下一個事件
        t, u, mach = heapq.heappop(event_q)

        # C: 釋放 children
        for v in children[u]:
            indeg[v] -= 1
            if indeg[v] == 0:
                heapq.heappush(ready, (-rank_u[v], v))

    return max(finish_time)
