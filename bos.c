#include <stdio.h>
#include <string.h>

#define MAXN 100
#define MAXM 20
#define MAXE 300
#define INF 1e18

typedef struct {
    int u, v;
    double delta;
} Edge;

int n, m, e;
double p[MAXN], r[MAXN];
Edge edges[MAXE];

double D[MAXN];              // bottleneck priority
double start_t[MAXN], end_t[MAXN];
int machine_of[MAXN];
int scheduled[MAXN], finished[MAXN];

double machine_free[MAXM];   // 每台機器何時空出

/* 檢查 task x 在 time t 是否可啟動 */
int executable(int x, double t) {
    if (scheduled[x]) return 0;
    if (r[x] > t) return 0;

    for (int i = 0; i < e; i++) {
        if (edges[i].v == x) {
            int u = edges[i].u;
            double need = p[u] - edges[i].delta;

            if (!scheduled[u]) return 0;

            double done = 0.0;
            if (finished[u]) done = p[u];
            else if (start_t[u] >= 0 && t > start_t[u]) {
                done = t - start_t[u];
                if (done > p[u]) done = p[u];
            }

            if (done < need) return 0;
        }
    }
    return 1;
}

/* 用簡單反覆鬆弛法算 D_i = p_i + max(D_j - delta_ij) */
void compute_D() {
    for (int i = 0; i < n; i++) D[i] = p[i];

    for (int round = 0; round < n; round++) {
        for (int i = 0; i < e; i++) {
            int u = edges[i].u, v = edges[i].v;
            double cand = p[u] + D[v] - edges[i].delta;
            if (cand > D[u]) D[u] = cand;
        }
    }
}

/* 找最早空閒機器 */
int best_machine() {
    int k = 0;
    for (int i = 1; i < m; i++) {
        if (machine_free[i] < machine_free[k]) k = i;
    }
    return k;
}

int main() {
    scanf("%d %d", &n, &m);

    for (int i = 0; i < n; i++) {
        scanf("%lf %lf", &p[i], &r[i]);
        start_t[i] = end_t[i] = -1;
        machine_of[i] = -1;
    }

    scanf("%d", &e);
    for (int i = 0; i < e; i++) {
        scanf("%d %d %lf", &edges[i].u, &edges[i].v, &edges[i].delta);
        edges[i].u--;  // 改成 0-based
        edges[i].v--;
    }

    for (int i = 0; i < m; i++) machine_free[i] = 0.0;

    compute_D();

    int done = 0;
    while (done < n) {
        int mac = best_machine();
        double t = machine_free[mac];

        int best = -1;
        for (int i = 0; i < n; i++) {
            if (executable(i, t)) {
                if (best == -1 || D[i] > D[best]) best = i;
            }
        }

        /* 若目前沒有可排工作，就把時間往前推 */
        if (best == -1) {
            double nt = INF;

            for (int i = 0; i < n; i++) {
                if (!scheduled[i] && r[i] > t && r[i] < nt) nt = r[i];
            }
            for (int i = 0; i < n; i++) {
                if (scheduled[i] && !finished[i] && end_t[i] > t && end_t[i] < nt) nt = end_t[i];
            }

            machine_free[mac] = nt;
            for (int i = 0; i < n; i++) {
                if (scheduled[i] && !finished[i] && end_t[i] <= nt) {
                    finished[i] = 1;
                    done++;
                }
            }
            continue;
        }

        start_t[best] = t;
        end_t[best] = t + p[best];
        machine_of[best] = mac;
        scheduled[best] = 1;
        machine_free[mac] = end_t[best];

        /* 更新已完成任務 */
        for (int i = 0; i < n; i++) {
            if (scheduled[i] && !finished[i] && end_t[i] <= machine_free[mac]) {
                finished[i] = 1;
                done++;
            }
        }
    }

    double makespan = 0.0;
    for (int i = 0; i < n; i++) {
        if (end_t[i] > makespan) makespan = end_t[i];
    }

    printf("Makespan = %.2f\n", makespan);
    printf("Task  Start  End  Machine  D\n");
    for (int i = 0; i < n; i++) {
        printf("%4d  %5.1f  %5.1f    %2d    %5.1f\n",
               i + 1, start_t[i], end_t[i], machine_of[i] + 1, D[i]);
    }

    return 0;
}
