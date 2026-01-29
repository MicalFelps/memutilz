import sys

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QIcon, QIntValidator
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QVBoxLayout,
    QLabel,
    QPushButton,
    QComboBox,
    QMessageBox,
    QCompleter,
)


class ComboDemo(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("QComboBox – Full Demo")

        layout = QVBoxLayout(self)

        # ------------------------------------------------------------
        # Basic label
        # ------------------------------------------------------------
        self.info = QLabel("Interact with the combo box and watch the console")
        layout.addWidget(self.info)

        # ------------------------------------------------------------
        # Main ComboBox
        # ------------------------------------------------------------
        self.combo = QComboBox()
        layout.addWidget(self.combo)

        # Size adjust behavior
        self.combo.setSizeAdjustPolicy(
            QComboBox.SizeAdjustPolicy.AdjustToContents
        )

        # Insert behavior when editable
        self.combo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtBottom)

        # ------------------------------------------------------------
        # Populate items
        # ------------------------------------------------------------
        self.combo.addItem("Apple", {"type": "fruit", "id": 1})
        self.combo.addItem("Banana", {"type": "fruit", "id": 2})
        self.combo.addItem("Cherry", {"type": "fruit", "id": 3})

        # Separator
        self.combo.insertSeparator(self.combo.count())

        self.combo.addItem("Carrot", {"type": "vegetable", "id": 4})
        self.combo.addItem("Broccoli", {"type": "vegetable", "id": 5})

        # Disabled item
        idx = self.combo.count()
        self.combo.addItem("Disabled item", {"disabled": True})
        self.combo.model().item(idx).setEnabled(False)

        # Tooltips via model roles
        for i in range(self.combo.count()):
            self.combo.setItemData(
                i,
                f"Tooltip for index {i}",
                Qt.ItemDataRole.ToolTipRole,
            )

        # ------------------------------------------------------------
        # Editable combo box features
        # ------------------------------------------------------------
        self.combo.setEditable(True)

        # Validator for edit text
        self.combo.setValidator(QIntValidator(0, 9999, self))

        # Completer
        completer = QCompleter(
            ["100", "200", "300", "400", "500"],
            self.combo,
        )
        completer.setCaseSensitivity(Qt.CaseInsensitive)
        self.combo.setCompleter(completer)

        # ------------------------------------------------------------
        # Signals
        # ------------------------------------------------------------
        self.combo.currentIndexChanged.connect(self.on_index_changed)
        self.combo.currentTextChanged.connect(self.on_text_changed)
        self.combo.activated.connect(self.on_activated)
        self.combo.editTextChanged.connect(self.on_edit_text_changed)

        # ------------------------------------------------------------
        # Buttons to demonstrate API usage
        # ------------------------------------------------------------
        btn_layout = QVBoxLayout()

        btn_find = QPushButton("Find 'Banana'")
        btn_find.clicked.connect(self.find_banana)
        btn_layout.addWidget(btn_find)

        btn_find_data = QPushButton("Find item with id=4")
        btn_find_data.clicked.connect(self.find_data)
        btn_layout.addWidget(btn_find_data)

        btn_popup = QPushButton("Show popup programmatically")
        btn_popup.clicked.connect(self.combo.showPopup)
        btn_layout.addWidget(btn_popup)

        btn_clear = QPushButton("Clear and repopulate")
        btn_clear.clicked.connect(self.reset_items)
        btn_layout.addWidget(btn_clear)

        layout.addLayout(btn_layout)

        # Initial selection
        self.combo.setCurrentIndex(0)

    # ------------------------------------------------------------
    # Signal handlers
    # ------------------------------------------------------------
    def on_index_changed(self, index):
        data = self.combo.itemData(index)
        print(f"[currentIndexChanged] index={index}, data={data}")

    def on_text_changed(self, text):
        print(f"[currentTextChanged] text='{text}'")

    def on_activated(self, index):
        print(f"[activated] index={index}")

    def on_edit_text_changed(self, text):
        print(f"[editTextChanged] text='{text}'")

    # ------------------------------------------------------------
    # Actions
    # ------------------------------------------------------------
    def find_banana(self):
        index = self.combo.findText("Banana")
        if index != -1:
            self.combo.setCurrentIndex(index)

    def find_data(self):
        index = self.combo.findData(
            4,
            role=Qt.ItemDataRole.UserRole,
            flags=Qt.MatchFlag.MatchRecursive,
        )
        if index != -1:
            self.combo.setCurrentIndex(index)
        else:
            QMessageBox.information(self, "Not found", "No item with id=4")

    def reset_items(self):
        self.combo.clear()
        self.combo.addItems(["One", "Two", "Three"])
        self.combo.setCurrentText("Two")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = ComboDemo()
    w.resize(400, 300)
    w.show()
    sys.exit(app.exec())

