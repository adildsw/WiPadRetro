import sys
import socket
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLineEdit, QPushButton, QTextEdit
from PyQt5.QtCore import QThread, pyqtSignal

class ListenerThread(QThread):
    received_signal = pyqtSignal(str)
    disconnected_signal = pyqtSignal()  # Signal for disconnection

    def __init__(self, ip, port):
        super().__init__()
        self.ip = ip
        self.port = port
        self.is_running = True

    def run(self):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((self.ip, self.port))
                s.sendall(b"WIPADRETRO_CONNECT")
                s.settimeout(2.0)
                print(f"Connected to {self.ip}:{self.port}")
                while self.is_running:
                    try:
                        data = s.recv(1024)
                        if data:
                            message = data.decode('utf-8')
                            self.received_signal.emit(message)
                            # Check for the disconnect message
                            if message == "WIPADRETRO_DISCONNECT":
                                print("Disconnect command received.")
                                self.disconnected_signal.emit()  # Emit disconnection signal
                                break  # Exit the loop
                    except socket.timeout:
                        continue
        except Exception as e:
            print(f"Connection closed or error: {e}")

    def stop(self):
        self.is_running = False
        self.quit()
        self.wait()

class WiPadRetroLink(QWidget):
    def __init__(self):
        super().__init__()
        self.listener = None
        self.initUI()
    
    def initUI(self):
        layout = QVBoxLayout()

        self.ip_input = QLineEdit(self)
        self.ip_input.setPlaceholderText("Enter IP Address")
        layout.addWidget(self.ip_input)

        self.connect_btn = QPushButton('Connect', self)
        self.connect_btn.clicked.connect(self.connect_to_server)
        layout.addWidget(self.connect_btn)

        self.messages = QTextEdit(self)
        self.messages.setReadOnly(True)
        layout.addWidget(self.messages)

        self.setLayout(layout)
        self.setGeometry(300, 300, 300, 200)
        self.setWindowTitle('WiPadRetro Link')
        self.show()

    def connect_to_server(self):
        ip = self.ip_input.text()
        port = 1803
        if self.listener is None or not self.listener.isRunning():
            self.listener = ListenerThread(ip, port)
            self.listener.received_signal.connect(self.update_messages)
            self.listener.disconnected_signal.connect(self.handle_disconnect)
            self.listener.start()

    def update_messages(self, message):
        self.messages.append(message)

    def handle_disconnect(self):
        if self.listener and self.listener.isRunning():
            self.listener.stop()
        self.messages.append("Disconnected from server.")

    def closeEvent(self, event):
        if self.listener and self.listener.isRunning():
            self.listener.stop()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = WiPadRetroLink()
    sys.exit(app.exec_())
