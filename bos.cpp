#include <bits/stdc++.h>
using namespace std;

/*
    BOS-Greedy Prototype (C++17)

    核心支援：
    1. identical parallel machines
    2. DAG precedence
    3. release time
    4. delta-overlap activation
    5. dynamic bottleneck priority Di
    6. event-driven scheduling

    -------------------------------
    輸入格式
    -------------------------------
    n m
    p[1] r[1]
    p[2] r[2]
    ...
    p[n] r[n]
    e
    u1 v1 delta1
    u2 v2 delta2
    ...
    ue ve deltae

    說明：
    - n: 任務數
    - m: 機器數
    - 每個任務 i 有 processing time p[i], release time r[i]
    - 每條邊 u -> v 表示 v 依賴 u
    - delta(u,v) 表示允許 v 在 u 完成前提早 delta 時間啟動
      也就是：若 task u 已執行時間 >= p[u] - delta(u,v)，
      則這條邊的啟動條件視為已滿足

    -------------------------------
    輸出
    -------------------------------
    - makespan
    - 每個 task 的 start / finish / machine
*/

struct Edge {
    int to;
    double delta;
};

struct RevEdge {
    int from;
    double delta;
};

struct Task {
    int id;
    double p;          // processing time
    double r;          // release time
    vector<Edge> out;  // successors
    vector<RevEdge> in;// predecessors
};

struct RunningTask {
    double finish_time;
    int machine_id;
    int task_id;

    bool operator<(const RunningTask& other) const {
        // priority_queue 預設是 max-heap，所以反過來寫
        return finish_time > other.finish_time;
    }
};

struct ReadyItem {
    double priority;   // Di
    int task_id;

    bool operator<(const ReadyItem& other) const {
        if (fabs(priority - other.priority) > 1e-12)
            return priority < other.priority; // max-heap
        return task_id > other.task_id;       // tie-break: 小 id 優先
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<Task> tasks(n + 1);

    for (int i = 1; i <= n; i++) {
        tasks[i].id = i;
        cin >> tasks[i].p >> tasks[i].r;
    }

    int e;
    cin >> e;

    vector<int> indeg(n + 1, 0);

    for (int i = 0; i < e; i++) {
        int u, v;
        double delta;
        cin >> u >> v >> delta;

        // 安全起見：delta 不應超過 p[u]
        delta = min(delta, tasks[u].p);
        delta = max(delta, 0.0);

        tasks[u].out.push_back({v, delta});
        tasks[v].in.push_back({u, delta});
        indeg[v]++;
    }

    /*
        Step 0: 計算 Dynamic Bottleneck Index Di
        參考你目前稿子的形式：
            Di = p_i + max_{(i->j)} (Dj - delta_ij)
        終端任務：Di = p_i
    */
    vector<int> topo;
    topo.reserve(n);

    queue<int> q;
    vector<int> indeg_copy = indeg;
    for (int i = 1; i <= n; i++) {
        if (indeg_copy[i] == 0) q.push(i);
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        topo.push_back(u);
        for (auto &ed : tasks[u].out) {
            if (--indeg_copy[ed.to] == 0) q.push(ed.to);
        }
    }

    if ((int)topo.size() != n) {
        cerr << "Error: Graph is not a DAG.\n";
        return 1;
    }

    vector<double> D(n + 1, 0.0);
    for (int i = n - 1; i >= 0; i--) {
        int u = topo[i];
        double best = 0.0;
        for (auto &ed : tasks[u].out) {
            best = max(best, D[ed.to] - ed.delta);
        }
        D[u] = tasks[u].p + best;
    }

    /*
        狀態變數
    */
    const double EPS = 1e-12;

    vector<bool> scheduled(n + 1, false); // 已啟動
    vector<bool> finished(n + 1, false);  // 已完成

    vector<double> start_time(n + 1, -1.0);
    vector<double> finish_time(n + 1, -1.0);
    vector<int> machine_of(n + 1, -1);

    // 已執行時間（非搶佔下，running 時 = current_time - start_time；完成後 = p）
    vector<double> executed(n + 1, 0.0);

    // 可用機器池
    priority_queue<int, vector<int>, greater<int>> free_machines;
    for (int i = 1; i <= m; i++) free_machines.push(i);

    // 正在執行的 task，以 finish time 排序
    priority_queue<RunningTask> running_pq;

    // ready heap：按 Di 最大優先
    priority_queue<ReadyItem> ready_pq;

    // 避免重複推入 ready heap
    vector<bool> in_ready_heap(n + 1, false);

    double current_time = 0.0;
    int done_count = 0;

    auto edge_satisfied = [&](int pred, double delta, double now) -> bool {
        /*
            邊 pred -> succ 滿足條件：
            pred 的已執行量 >= p[pred] - delta
        */
        double exec_amount = executed[pred];

        // 若 pred 正在執行，要把到 now 的執行量算進去
        if (scheduled[pred] && !finished[pred]) {
            exec_amount = max(exec_amount, now - start_time[pred]);
            exec_amount = min(exec_amount, tasks[pred].p);
        }

        return exec_amount + EPS >= tasks[pred].p - delta;
    };

    auto task_executable = [&](int v, double now) -> bool {
        if (scheduled[v]) return false;
        if (tasks[v].r > now + EPS) return false;

        for (auto &pre : tasks[v].in) {
            if (!edge_satisfied(pre.from, pre.delta, now)) {
                return false;
            }
        }
        return true;
    };

    auto push_new_ready_tasks = [&](double now) {
        for (int i = 1; i <= n; i++) {
            if (!scheduled[i] && !in_ready_heap[i] && task_executable(i, now)) {
                ready_pq.push({D[i], i});
                in_ready_heap[i] = true;
            }
        }
    };

    auto assign_tasks = [&](double now) {
        push_new_ready_tasks(now);

        while (!free_machines.empty() && !ready_pq.empty()) {
            auto item = ready_pq.top();
            ready_pq.pop();
            int u = item.task_id;
            in_ready_heap[u] = false;

            // heap 中的 task 可能因時間狀態變動而已不合法，重新檢查
            if (!task_executable(u, now)) continue;

            int machine_id = free_machines.top();
            free_machines.pop();

            scheduled[u] = true;
            start_time[u] = now;
            finish_time[u] = now + tasks[u].p;
            machine_of[u] = machine_id;

            running_pq.push({finish_time[u], machine_id, u});
        }
    };

    /*
        Event-driven 主迴圈
        事件來源：
        1. task completion
        2. future release time
        3. future delta-activation（某前驅跑到 p-delta 的時間點）
    */
    assign_tasks(current_time);

    while (done_count < n) {
        double next_event = numeric_limits<double>::infinity();

        // 1) completion event
        if (!running_pq.empty()) {
            next_event = min(next_event, running_pq.top().finish_time);
        }

        // 2) release event
        for (int i = 1; i <= n; i++) {
            if (!scheduled[i] && tasks[i].r > current_time + EPS) {
                next_event = min(next_event, tasks[i].r);
            }
        }

        // 3) delta-activation event
        //    若某未排任務 v 的某前驅 u 已開始但尚未達門檻，
        //    估算 u 達到 p[u]-delta(u,v) 的最早時刻
        for (int v = 1; v <= n; v++) {
            if (scheduled[v]) continue;
            if (tasks[v].r > current_time + EPS) continue;

            for (auto &pre : tasks[v].in) {
                int u = pre.from;
                double need = tasks[u].p - pre.delta;

                // 已滿足就不用看
                double exec_amount = executed[u];
                if (scheduled[u] && !finished[u]) {
                    exec_amount = max(exec_amount, current_time - start_time[u]);
                    exec_amount = min(exec_amount, tasks[u].p);
                }

                if (exec_amount + EPS >= need) continue;

                // 若前驅尚未開始，就無法預測這條邊何時滿足
                if (!scheduled[u] || finished[u]) continue;

                // 非搶佔下，u 會在 start_time[u] + need 時達門檻
                double trigger_time = start_time[u] + need;
                if (trigger_time > current_time + EPS) {
                    next_event = min(next_event, trigger_time);
                }
            }
        }

        if (!isfinite(next_event)) {
            cerr << "Error: No further event found, scheduling got stuck.\n";
            return 1;
        }

        current_time = next_event;

        /*
            更新所有 running task 的 executed amount
        */
        {
            // 因為 running_pq 不能直接遍歷更新，所以用 start_time 推導即可
            for (int i = 1; i <= n; i++) {
                if (scheduled[i] && !finished[i]) {
                    executed[i] = min(tasks[i].p, current_time - start_time[i]);
                }
            }
        }

        /*
            處理所有在 current_time 完成的 task
        */
        while (!running_pq.empty() &&
               running_pq.top().finish_time <= current_time + EPS) {
            auto rt = running_pq.top();
            running_pq.pop();

            int u = rt.task_id;
            if (finished[u]) continue;

            finished[u] = true;
            executed[u] = tasks[u].p;
            finish_time[u] = rt.finish_time;
            free_machines.push(rt.machine_id);
            done_count++;
        }

        /*
            每個事件點都重新嘗試派工
        */
        assign_tasks(current_time);
    }

    double makespan = 0.0;
    for (int i = 1; i <= n; i++) {
        makespan = max(makespan, finish_time[i]);
    }

    cout << fixed << setprecision(4);
    cout << "Makespan = " << makespan << "\n";
    cout << "Schedule:\n";
    cout << "Task  Start   Finish  Machine  D_i\n";
    for (int i = 1; i <= n; i++) {
        cout << setw(4) << i << "  "
             << setw(6) << start_time[i] << "  "
             << setw(6) << finish_time[i] << "  "
             << setw(7) << machine_of[i] << "  "
             << setw(6) << D[i] << "\n";
    }

    return 0;
}
