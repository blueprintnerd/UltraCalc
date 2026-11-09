#!/usr/bin/env python3
"""UltraCalc Qt GUI - Uses ultracalcbase executable for calculations."""

import sys
import subprocess
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, 
    QLineEdit, QLCDNumber, QPushButton, QHBoxLayout,
    QLabel, QFrame
)
from PyQt6.QtCore import Qt, pyqtSignal, QObject


class CalculatorApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("UltraCalc")
        self.setFixedSize(400, 300)
        
        # Central widget and layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)
        
        # LCD Display
        self.lcd = QLCDNumber()
        self.lcd.setDigitCount(12)
        self.lcd.setSegmentStyle(QLCDNumber.SegmentStyle.Flat)
        self.lcd.setStyleSheet("background-color: #f0f0f0; color: #000;")
        self.lcd.setFixedHeight(60)
        
        # Input field
        self.input_field = QLineEdit()
        self.input_field.setPlaceholderText("Enter equation (e.g., add 5, 3 or 10 + 5)")
        self.input_field.setStyleSheet("font-size: 16px; padding: 8px;")
        self.input_field.returnPressed.connect(self.calculate)
        
        # Button layout
        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        
        # Operator buttons
        self.btn_add = QPushButton("+")
        self.btn_sub = QPushButton("-")
        self.btn_mul = QPushButton("*")
        self.btn_div = QPushButton("/")
        self.btn_mod = QPushButton("%")
        
        for btn in [self.btn_add, self.btn_sub, self.btn_mul, self.btn_div, self.btn_mod]:
            btn.setFixedSize(50, 50)
            btn.setStyleSheet("font-size: 20px; font-weight: bold;")
            btn.clicked.connect(self.on_operator_clicked)
        
        # Equals button
        self.btn_equals = QPushButton("=")
        self.btn_equals.setFixedSize(50, 50)
        self.btn_equals.setStyleSheet("font-size: 20px; font-weight: bold; background-color: #4CAF50; color: white;")
        self.btn_equals.clicked.connect(self.calculate)
        
        # Clear button
        self.btn_clear = QPushButton("C")
        self.btn_clear.setFixedSize(50, 50)
        self.btn_clear.setStyleSheet("font-size: 20px; font-weight: bold; background-color: #f44336; color: white;")
        self.btn_clear.clicked.connect(self.clear_input)
        
        # Add buttons to layout
        button_layout.addWidget(self.btn_add)
        button_layout.addWidget(self.btn_sub)
        button_layout.addWidget(self.btn_mul)
        button_layout.addWidget(self.btn_div)
        button_layout.addWidget(self.btn_mod)
        button_layout.addWidget(self.btn_equals)
        button_layout.addWidget(self.btn_clear)
        
        # Status label
        self.status_label = QLabel("Ready")
        self.status_label.setStyleSheet("color: #666; font-size: 12px;")
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Add widgets to main layout
        layout.addWidget(self.lcd)
        layout.addWidget(self.input_field)
        layout.addLayout(button_layout)
        layout.addWidget(self.status_label)
        
        # Menu bar
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
        
        self.status_label.setText("Calculating...")
        
        try:
            # Use ultracalcbase with --calcapi flag
            result = self.run_ultracalc(equation)
            
            if result is not None:
                self.lcd.display(float(result))
                self.status_label.setText(f"Result: {result}")
            else:
                self.status_label.setText("Could not calculate")
        except Exception as e:
            self.status_label.setText(f"Error: {str(e)}")
    
    def run_ultracalc(self, equation):
        """Run ultracalcbase with --calcapi flag to compute the result."""
        # Parse the equation into operation and operands
        # Supports: "add 5, 3", "5 + 3", "sub 10 from 20", etc.
        
        # Try to extract operation and numbers
        op = None
        num1 = None
        num2 = None
        
        # Check for command-style input (add 5, 3)
        equation_lower = equation.lower()
        
        if equation_lower.startswith("add "):
            op = "add"
            parts = equation[4:].split(",")
            if len(parts) >= 2:
                num1 = parts[0].strip()
                num2 = parts[1].strip()
        elif equation_lower.startswith("sub "):
            op = "sub"
            if " from " in equation_lower:
                parts = equation[4:].split(" from ")
                if len(parts) >= 2:
                    num1 = parts[0].strip()
                    num2 = parts[1].strip()
            else:
                parts = equation[4:].split(",")
                if len(parts) >= 2:
                    num1 = parts[0].strip()
                    num2 = parts[1].strip()
        elif equation_lower.startswith("mul "):
            op = "mul"
            parts = equation[4:].split(",")
            if len(parts) >= 2:
                num1 = parts[0].strip()
                num2 = parts[1].strip()
        elif equation_lower.startswith("div "):
            op = "div"
            parts = equation[4:].split(",")
            if len(parts) >= 2:
                num1 = parts[0].strip()
                num2 = parts[1].strip()
        elif equation_lower.startswith("mod "):
            op = "mod"
            parts = equation[4:].split(",")
            if len(parts) >= 2:
                num1 = parts[0].strip()
                num2 = parts[1].strip()
        else:
            # Try to parse infix notation: "5 + 3", "10 - 2", etc.
            for operator in ["+", "-", "*", "/", "%", "x", "÷"]:
                if operator in equation:
                    parts = equation.split(operator)
                    if len(parts) >= 2:
                        op = operator
                        num1 = parts[0].strip()
                        num2 = parts[1].strip()
                        # Normalize operator for ultracalcbase
                        if op in ["*", "x", "÷"]:
                            op = "mul" if op in ["*", "x"] else "div"
                        break
        
        if op and num1 and num2:
            try:
                # Call ultracalcbase with --calcapi
                cmd = ["./ultracalcbase", "--calcapi", op, num1, num2]
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    cwd="/vercel/sandbox/UltraCalc"
                )
                
                if result.returncode == 0:
                    try:
                        return float(result.stdout.strip())
                    except ValueError:
                        return None
                else:
                    print(f"Error: {result.stderr}")
                    return None
            except Exception as e:
                print(f"Failed to run ultracalcbase: {e}")
                return None
        
        return None


def main():
    app = QApplication(sys.argv)
        
    window = CalculatorApp()
    window.show()
    
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
