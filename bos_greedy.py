# bos_greedy.py — Version 9
# Timeline Simulation Engine (Final Edition)
# ------------------------------------------
# ★ 支援合法 δ-overlap：child 可比 latest-parent 提前 δ
# ★ 不會破壞 DAG 拓樸
# ★ 所有 release 都是獨立事件，不會 pending 回溯
# ★ event queue = {release events, finish events}
# ★ timeline-driven：穩定、無 idle-gap explosion

import heapq

def bos_greedy(instance, m=4, alpha=0.0):

    n = instance.n
    p = instance.p
    parents = instance.parents
    children = instance.children
    D = instance.D

    # δ[i] = alpha * processing_time
    delta = [alpha * p[i] for i in range(n)]

    # 時間狀態
    finish_time = [0.0] * n
    busy_until = [0.0] * m

    # 任務的最早合法開始時間（包含 δ overlap）
    release_time = [float("inf")] * n

    # ready queue：(-D[i], i)
    ready = []

    # event queue: (time, type, task)
    # type 0 = release
    # type 1 = finish
    event_q = []

    # initialize root tasks
    for i in range(n):
        if len(parents[i]) == 0:
            release_time[i] = 0.0
            heapq.heappush(event_q, (0.0, 0, i))

    current_time = 0.0

    # -------------------------------------------------------
    # 啟動所有可開工任務
    # -------------------------------------------------------
    def try_start_all():
        nonlocal current_time
        launched = True
        while launched:
            launched = False
            for j in range(m):
                if ready and busy_until[j] <= current_time:
                    _, task = heapq.heappop(ready)

                    start = max(release_time[task], busy_until[j])
                    end   = start + p[task]

                    finish_time[task] = end
                    busy_until[j] = end

                    heapq.heappush(event_q, (end, 1, task))
                    launched = True

    # =======================================================
    # Main timeline simulation loop
    # =======================================================
    while event_q or ready:

        # 取下一個事件時間
        t, typ, u = heapq.heappop(event_q)

        # 推進時間軸
        current_time = t

        # ---------------------------------------------------
        # Release Event
        # ---------------------------------------------------
        if typ == 0:
            # 把任務加入 ready
            heapq.heappush(ready, (-D[u], u))
            try_start_all()
            continue

        # ---------------------------------------------------
        # Finish Event
        # ---------------------------------------------------
        if typ == 1:
            # 任務 u 完成 → 更新 children
            for v in children[u]:

                # child 的合法最早開始 = latest-parent finish − δ
                latest_parent_finish = max(finish_time[p] for p in parents[v])
                cand = latest_parent_finish - delta[v]

                # clamp >= 0
                cand = max(cand, 0.0)

                # 更新 release_time
                if cand < release_time[v]:
                    release_time[v] = cand
                    heapq.heappush(event_q, (cand, 0, v))

            try_start_all()

    return max(finish_time)
