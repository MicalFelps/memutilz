import sys
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QLineEdit,
    QVBoxLayout,
    QLabel,
    QCompleter,
)
from PySide6.QtCore import Qt


class CompleterDemo(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("QCompleter Demo")

        layout = QVBoxLayout(self)

        layout.addWidget(QLabel("Start typing a fruit name:"))

        self.edit = QLineEdit()
        layout.addWidget(self.edit)

        words = [
            "Apple",
            "Apricot",
            "Avocado",
            "Banana",
            "Blackberry",
            "Blueberry",
            "Cherry",
            "Coconut",
            "Date",
            "Dragonfruit",
            "Grape",
            "Grapefruit",
            "Kiwi",
            "Lemon",
            "Mango",
            "Orange",
            "Peach",
            "Pear",
            "Pineapple",
            "Strawberry",
            "Watermelon",
        ]

        completer = QCompleter(words, self)
        completer.setCaseSensitivity(Qt.CaseInsensitive)
        completer.setFilterMode(Qt.MatchContains)
        completer.setCompletionMode(QCompleter.PopupCompletion)

        self.edit.setCompleter(completer)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = CompleterDemo()
    win.resize(300, 120)
    win.show()
    sys.exit(app.exec())

