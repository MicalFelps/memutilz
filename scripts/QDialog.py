# A QDialog is a small, temporary window that exists to complete one focused interaction or decision.
import sys
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QPushButton,
    QVBoxLayout,
    QLabel,
    QDialog,
    QLineEdit,
    QTextEdit,
    QDialogButtonBox,
)
from PySide6.QtCore import Qt, Signal


# -------------------------------------------------
# 1) Simple modal OK / Cancel dialog
# -------------------------------------------------
class SimpleModalDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Simple Modal Dialog")

        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("Do you accept?"))

        buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)

        layout.addWidget(buttons)


# -------------------------------------------------
# 2) Modal dialog returning data
# -------------------------------------------------
class DataDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Return Data")

        self.edit = QLineEdit()

        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("Enter your name:"))
        layout.addWidget(self.edit)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok)
        buttons.accepted.connect(self.accept)

        layout.addWidget(buttons)

    def name(self):
        return self.edit.text()


# -------------------------------------------------
# 3) Modeless dialog (non-blocking)
# -------------------------------------------------
class ModelessDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Modeless Dialog")

        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("You can keep using the main window!"))


# -------------------------------------------------
# 4) Dialog emitting a signal
# -------------------------------------------------
class SignalDialog(QDialog):
    confirmed = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Signal Dialog")

        self.edit = QLineEdit()

        btn = QPushButton("Emit signal & close")
        btn.clicked.connect(self.emit_and_close)

        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("Type something:"))
        layout.addWidget(self.edit)
        layout.addWidget(btn)

    def emit_and_close(self):
        self.confirmed.emit(self.edit.text())
        self.accept()


# -------------------------------------------------
# 5) Tool-window dialog
# -------------------------------------------------
class ToolDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Tool Window")
        self.setWindowFlags(Qt.Tool | Qt.WindowStaysOnTopHint)

        self.log = QTextEdit(readOnly=True)

        layout = QVBoxLayout(self)
        layout.addWidget(self.log)

    def append(self, text):
        self.log.append(text)


# -------------------------------------------------
# 6) Modal type demo (window vs application modal)
# -------------------------------------------------
class ModalTypeDialog(QDialog):
    def __init__(self, modal_type, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Modal Type Demo")
        self.setWindowModality(modal_type)

        label = QLabel(f"Modality: {modal_type}")
        layout = QVBoxLayout(self)
        layout.addWidget(label)

        buttons = QDialogButtonBox(QDialogButtonBox.Close)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)


# -------------------------------------------------
# Main window that launches everything
# -------------------------------------------------
class DemoWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("QDialog Playground")

        layout = QVBoxLayout(self)

        # Keep references for modeless dialogs
        self.modeless = None
        self.tool_dialog = None

        # Buttons
        btn_simple = QPushButton("1) Simple modal dialog")
        btn_data = QPushButton("2) Modal dialog returning data")
        btn_modeless = QPushButton("3) Modeless dialog")
        btn_signal = QPushButton("4) Signal-based dialog")
        btn_tool = QPushButton("5) Tool window dialog")
        btn_window_modal = QPushButton("6) Window-modal dialog")
        btn_app_modal = QPushButton("7) Application-modal dialog")

        layout.addWidget(btn_simple)
        layout.addWidget(btn_data)
        layout.addWidget(btn_modeless)
        layout.addWidget(btn_signal)
        layout.addWidget(btn_tool)
        layout.addWidget(btn_window_modal)
        layout.addWidget(btn_app_modal)

        # Connections
        btn_simple.clicked.connect(self.show_simple)
        btn_data.clicked.connect(self.show_data)
        btn_modeless.clicked.connect(self.show_modeless)
        btn_signal.clicked.connect(self.show_signal)
        btn_tool.clicked.connect(self.show_tool)
        btn_window_modal.clicked.connect(
            lambda: self.show_modal(Qt.WindowModal)
        )
        btn_app_modal.clicked.connect(
            lambda: self.show_modal(Qt.ApplicationModal)
        )

    def show_simple(self):
        dlg = SimpleModalDialog(self)
        result = dlg.exec()
        print("Accepted" if result == QDialog.Accepted else "Cancelled")

    def show_data(self):
        dlg = DataDialog(self)
        if dlg.exec() == QDialog.Accepted:
            print("Name:", dlg.name())

    def show_modeless(self):
        if not self.modeless:
            self.modeless = ModelessDialog(self)
        self.modeless.show()

    def show_signal(self):
        dlg = SignalDialog(self)
        dlg.confirmed.connect(lambda text: print("Signal received:", text))
        dlg.exec()

    def show_tool(self):
        if not self.tool_dialog:
            self.tool_dialog = ToolDialog(self)
        self.tool_dialog.show()
        self.tool_dialog.append("Hello from main window")

    def show_modal(self, modality):
        dlg = ModalTypeDialog(modality, self)
        dlg.show()


# -------------------------------------------------
# Entry point
# -------------------------------------------------
if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = DemoWindow()
    win.resize(420, 300)
    win.show()
    sys.exit(app.exec())

