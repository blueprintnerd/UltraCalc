import PyQt6
import sys 

def main(): 
   app = PyQt6.QtWidgets.QApplication(sys.argv)
   window = PyQt6.QtWidgets.QWidget()
   window.show()
   display = PyQt6.QtWidgets.QLineEdit()
   
   sys.exit(app.exec()

if __name__ == "__main__":
    main()