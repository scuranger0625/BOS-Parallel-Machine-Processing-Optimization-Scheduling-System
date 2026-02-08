import sys
from PyQt5.QtWidgets import (
    QApplication, QWidget, QPushButton, QVBoxLayout,
    QHBoxLayout, QLabel, QFrame
)
from PyQt5.QtCore import QTimer
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
import matplotlib.pyplot as plt
import networkx as nx


class BOSAI(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("BOS AI Scheduling Dashboard")
        self.resize(1000,650)

        main_layout = QVBoxLayout()

        # ===== Header =====
        title = QLabel("BOS AI Scheduling Dashboard")
        title.setStyleSheet("""
            font-size:20px;
            font-weight:bold;
            color:#00e5ff;
        """)
        main_layout.addWidget(title)

        # ===== 中間區域 =====
        center_layout = QHBoxLayout()

        # ===== 左側控制 Panel =====
        control_panel = QFrame()
        control_layout = QVBoxLayout()

        self.run_btn = QPushButton("▶ RUN BOS")
        self.run_btn.clicked.connect(self.run_bos)

        control_layout.addWidget(QLabel("Controls"))
        control_layout.addWidget(self.run_btn)
        control_layout.addStretch()
        control_panel.setLayout(control_layout)

        # ===== Gantt Panel =====
        self.figure = plt.figure(facecolor="#1a1a1a")
        self.canvas = FigureCanvas(self.figure)

        graph_panel = QFrame()
        graph_layout = QVBoxLayout()
        graph_layout.addWidget(self.canvas)
        graph_panel.setLayout(graph_layout)

        center_layout.addWidget(control_panel,1)
        center_layout.addWidget(graph_panel,3)

        main_layout.addLayout(center_layout)

        # ===== Metrics =====
        self.metrics = QLabel("   LIST: 124      HEFT: 111      BOS: 101")
        self.metrics.setStyleSheet("""
            background-color:#1a1a1a;
            color:#00e5ff;
            padding:12px;
            font-size:12pt;
            font-weight:bold;
        """)
        main_layout.addWidget(self.metrics)

        self.setLayout(main_layout)

        # ===== NVIDIA Skin =====
        self.setStyleSheet("""
        QWidget {
            background-color: #121212;
            color: #e0e0e0;
            font-family: Segoe UI;
            font-size: 11pt;
        }
        QPushButton {
            background-color: #1f1f1f;
            border: 1px solid #00e5ff;
            padding: 8px;
            color: #00e5ff;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #00e5ff;
            color: #000000;
        }
        QFrame {
            background-color: #1a1a1a;
            border-radius: 6px;
        }
        """)

        self.draw_graph()

    # ===== 按鈕觸發 =====
    def run_bos(self):
        self.draw_graph()

    # ===== Gantt Chart =====
    def draw_graph(self):

        self.figure.clear()
        ax = self.figure.add_subplot(111)
        ax.set_facecolor("#1a1a1a")

        schedule = [
            ("Machine 1", 0, 3, "#444444"),
            ("Machine 1", 3, 5, "#00e5ff"),
            ("Machine 2", 1, 4, "#444444"),
            ("Machine 2", 4, 7, "#00e5ff"),
            ("Machine 3", 2, 6, "#444444"),
        ]

        for i, (machine, start, end, color) in enumerate(schedule):
            ax.barh(i, end-start, left=start,
                    color=color, edgecolor="#00e5ff", alpha=0.85)

        ax.set_yticks(range(len(schedule)))
        ax.set_yticklabels([s[0] for s in schedule], color="white")

        ax.set_xlabel("Time", color="white")
        ax.tick_params(axis='x', colors='white')
        ax.tick_params(axis='y', colors='white')

        ax.grid(color="#333333", linestyle="--", linewidth=0.5)

        self.canvas.draw()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = BOSAI()
    ui.show()
    sys.exit(app.exec_())
