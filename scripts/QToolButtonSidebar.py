# pip install PySide6    (or PyQt6)

import sys
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QToolButton, QButtonGroup
)
from PySide6.QtGui import QPainter, QColor, QPen
from PySide6.QtCore import Qt

class AccentToolButton(QToolButton):
    def __init__(self, text="", parent=None):
        super().__init__(parent)
        self.setText(text)
        self.setCheckable(True)           # important — we use checked state
        self.setToolButtonStyle(Qt.ToolButtonTextOnly)  # or TextBesideIcon etc.
        # Optional: make it look more sidebar-like
        self.setFixedHeight(48)
        self.setMinimumWidth(160)

    def paintEvent(self, event):
        # Let Qt draw the normal button (background, hover, text, etc.)
        super().paintEvent(event)

        if not self.isChecked():
            return  # only draw accent when checked

        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Vertical line on the very left — 3–4 px thick, accent color
        pen = QPen(QColor("#0066CC"))   # nice blue like many apps use
        pen.setWidth(4)
        painter.setPen(pen)

        # Draw from top to almost bottom (small margin)
        h = self.height()
        painter.drawLine(2, 4, 2, h - 4)   # x=2 so it's nicely inset

        # Optional: faint background tint when selected (very common)
        # painter.setPen(Qt.NoPen)
        # painter.setBrush(QColor(0, 100, 200, 25))  # very transparent blue
        # painter.drawRect(0, 0, self.width(), h)


class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("QToolButton with left accent line on checked")
        self.resize(280, 400)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 10, 0, 10)
        layout.setSpacing(0)

        # Group so only one can be checked at a time (sidebar style)
        group = QButtonGroup(self)
        group.setExclusive(True)

        items = ["Processes", "Performance", "App history", "Startup apps", "Users"]

        for name in items:
            btn = AccentToolButton(name, self)
            btn.setChecked(name == "Performance")  # start with one selected
            group.addButton(btn)
            layout.addWidget(btn)

        layout.addStretch()


if __name__ == "__main__":
    app = QApplication(sys.argv)

    # Optional: make the whole window look a bit more sidebar-like
    app.setStyleSheet("""
        QWidget {
            background: #f0f0f0;
        }
        QToolButton {
            text-align: left;
            padding-left: 16px;
            border: none;
        }
        QToolButton:hover {
            background: #e0e0e0;
        }
        QToolButton:checked {
            background: #e8f0ff;   /* very light blue tint */
            font-weight: bold;
        }
    """)

    window = MainWindow()
    window.show()
    sys.exit(app.exec())
