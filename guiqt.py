#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout,
    QLineEdit, QLCDNumber, QPushButton, QHBoxLayout, QLabel
)


def find_ultracalc_cli() -> str | None:
    """Resolve the ultracalc-cli binary dynamically."""
    app_dir = os.path.dirname(os.path.abspath(__file__))
    candidates = [
        os.path.join(app_dir, "ultracalc-cli"),
        os.path.join(app_dir, "bin", "ultracalc-cli"),
        os.path.join(os.path.dirname(app_dir), "bin", "ultracalc-cli"),
        os.path.join(app_dir, "build", "ultracalc-cli"),
    ]
    for path in candidates:
        if os.path.isfile(path) and os.access(path, os.X_OK):
            return path

    return shutil.which("ultracalc-cli")


class CalculatorApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("UltraCalc")
        self.setFixedSize(420, 320)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)

        self.lcd = QLCDNumber()
        self.lcd.setDigitCount(12)
        self.lcd.setSegmentStyle(QLCDNumber.SegmentStyle.Flat)
        self.lcd.setFixedHeight(60)

        self.input_field = QLineEdit()
        self.input_field.setPlaceholderText("Enter an equation (e.g. 12 + 5 or add 12, 5)")
        self.input_field.returnPressed.connect(self.calculate)

        button_layout = QHBoxLayout()
        button_layout.setSpacing(8)

        self.btn_add = QPushButton("+")
        self.btn_sub = QPushButton("-")
        self.btn_mul = QPushButton("*")
        self.btn_div = QPushButton("/")
        self.btn_mod = QPushButton("%")

        for btn in [self.btn_add, self.btn_sub, self.btn_mul, self.btn_div, self.btn_mod]:
            btn.setFixedSize(48, 48)
            btn.clicked.connect(self.on_operator_clicked)

        self.btn_equals = QPushButton("=")
        self.btn_equals.setFixedSize(48, 48)
        self.btn_equals.clicked.connect(self.calculate)

        self.btn_clear = QPushButton("C")
        self.btn_clear.setFixedSize(48, 48)
        self.btn_clear.clicked.connect(self.clear_input)

        button_layout.addWidget(self.btn_add)
        button_layout.addWidget(self.btn_sub)
        button_layout.addWidget(self.btn_mul)
        button_layout.addWidget(self.btn_div)
        button_layout.addWidget(self.btn_mod)
        button_layout.addWidget(self.btn_equals)
        button_layout.addWidget(self.btn_clear)

        self.status_label = QLabel("Ready")

        layout.addWidget(self.lcd)
        layout.addWidget(self.input_field)
        layout.addLayout(button_layout)
        layout.addWidget(self.status_label)

        self.create_menu_bar()

    def create_menu_bar(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu("File")

        exit_action = file_menu.addAction("Exit")
        exit_action.triggered.connect(self.close)

    def on_operator_clicked(self):
        sender = self.sender()
        if sender:
            op = sender.text()
            current_text = self.input_field.text()
            self.input_field.setText(current_text + " " + op + " ")
            self.input_field.setFocus()

    def clear_input(self):
        self.input_field.clear()
        self.lcd.display(0)
        self.status_label.setText("Ready")
        self.input_field.setFocus()

    def calculate(self):
        equation = self.input_field.text().strip()
        if not equation:
            self.status_label.setText("Please enter an equation")
            return

        try:
            result = self.run_ultracalc(equation)
            if result is not None:
                if result.is_integer():
                    formatted = str(int(result))
                    self.lcd.display(int(result))
                else:
                    formatted = f"{result:g}"
                    self.lcd.display(result)
                self.status_label.setText(f"Result: {formatted}")
        except Exception as e:
            self.status_label.setText(f"Error: {e}")

    def parse_equation(self, equation: str) -> tuple[str, str, str]:
        equation_lower = equation.lower()

        if equation_lower.startswith("add "):
            parts = equation[4:].split(",")
            if len(parts) == 2:
                return "add", parts[0].strip(), parts[1].strip()
        elif equation_lower.startswith("sub "):
            if " from " in equation_lower:
                idx = equation_lower.find(" from ")
                num1 = equation[4:idx].strip()
                num2 = equation[idx + 6:].strip()
                return "sub", num2, num1
            parts = equation[4:].split(",")
            if len(parts) == 2:
                return "sub", parts[0].strip(), parts[1].strip()
        elif equation_lower.startswith("mul "):
            parts = equation[4:].split(",")
            if len(parts) == 2:
                return "mul", parts[0].strip(), parts[1].strip()
        elif equation_lower.startswith("div "):
            if " by " in equation_lower:
                idx = equation_lower.find(" by ")
                return "div", equation[4:idx].strip(), equation[idx + 4:].strip()
            parts = equation[4:].split(",")
            if len(parts) == 2:
                return "div", parts[0].strip(), parts[1].strip()
        elif equation_lower.startswith("mod "):
            if " by " in equation_lower:
                idx = equation_lower.find(" by ")
                return "mod", equation[4:idx].strip(), equation[idx + 4:].strip()
            parts = equation[4:].split(",")
            if len(parts) == 2:
                return "mod", parts[0].strip(), parts[1].strip()
        else:
            op_map = {
                "+": "add",
                "-": "sub",
                "*": "mul",
                "x": "mul",
                "/": "div",
                "÷": "div",
                "%": "mod",
            }
            for i in range(1, len(equation)):
                ch = equation[i]
                if ch in op_map:
                    left = equation[:i].strip()
                    right = equation[i + 1:].strip()
                    if left and right:
                        return op_map[ch], left, right

        raise ValueError("Invalid equation syntax")

    def run_ultracalc(self, equation: str) -> float | None:
        op, num1, num2 = self.parse_equation(equation)
        cli_bin = find_ultracalc_cli()
        if not cli_bin:
            raise RuntimeError("ultracalc-cli binary not found")

        cmd = [cli_bin, "--calcapi", op, num1, num2]
        proc = subprocess.run(cmd, capture_output=True, text=True)

        if proc.returncode != 0:
            err_msg = proc.stderr.strip() or proc.stdout.strip() or f"Exit code {proc.returncode}"
            raise RuntimeError(err_msg)

        output = proc.stdout.strip()
        return float(output)


def main():
    app = QApplication(sys.argv)
    window = CalculatorApp()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
