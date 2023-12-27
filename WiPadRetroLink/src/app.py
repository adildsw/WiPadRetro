import sys
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLineEdit, QPushButton, QGridLayout, QLabel, QGroupBox
from listener_thread import ListenerThread

class WiPadRetroLink(QWidget):
    def __init__(self):
        super().__init__()
        self.listener = None
        self.is_connected = False
        self.init_ui()
    
    def init_ui(self):
        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(30)  # Adjust spacing between group boxes

        # Group box for IP and Connect
        self.connection_group = QGroupBox("Connection Settings")
        connection_layout = QVBoxLayout(self.connection_group)
        connection_layout.setContentsMargins(10, 10, 10, 10)

        self.ip_input = QLineEdit(self)
        self.ip_input.setPlaceholderText("Enter IP Address")
        self.ip_input.setText("127.0.0.1")
        self.ip_input.setText("192.168.0.102")
        connection_layout.addWidget(self.ip_input)

        self.connect_btn = QPushButton('Connect', self)
        self.connect_btn.clicked.connect(self.connect_to_server)
        connection_layout.addWidget(self.connect_btn)

        main_layout.addWidget(self.connection_group)

        # Group box for Button Configurations
        self.btn_config_group = QGroupBox("Button Configurations")
        grid_layout = QGridLayout(self.btn_config_group)
        grid_layout.setContentsMargins(10, 10, 10, 10)

        keys = ["up", "down", "left", "right", "a", "b", "x", "y", "l1", "r1", "l2", "r2", "select", "start"]
        self.key_labels = {}
        self.key_inputs = {}

        for i, key in enumerate(keys):
            label = QLabel(key.capitalize())
            self.key_labels[key] = label
            grid_layout.addWidget(label, i//2, 2*(i%2))
            line_edit = QLineEdit(self)
            line_edit.setPlaceholderText("Key")
            line_edit.setMaximumWidth(80)
            self.key_inputs[key] = line_edit
            grid_layout.addWidget(line_edit, i//2, 2*(i%2) + 1)

        main_layout.addWidget(self.btn_config_group)

        self.setGeometry(300, 100, 320, 400)
        self.setWindowTitle('WiPadRetro Link')
        self.setFixedSize(self.size())
        self.setWindowFlags(Qt.Window | Qt.WindowCloseButtonHint)
        self.show()

    def connect_to_server(self):
        if not self.is_connected:
            ip = self.ip_input.text()
            if self.listener is None or not self.listener.isRunning():
                self.listener = ListenerThread(ip)
                self.listener.connected_signal.connect(self.handle_connect)
                self.listener.received_signal.connect(self.update_keyinput)
                self.listener.disconnected_signal.connect(self.handle_disconnect)
                self.listener.start()
        else:
            self.handle_disconnect()


    def update_keyinput(self, key_input):
        for key in self.key_labels:
            self.key_labels[key].setStyleSheet("font-weight: normal")
            if key_input[key]:
                self.key_labels[key].setStyleSheet("font-weight: bold")

    def handle_connect(self):
        self.is_connected = True
        self.connect_btn.setText("Disconnect")

        self.ip_input.setEnabled(False)
        for key in self.key_inputs:
            self.key_inputs[key].setEnabled(False)

    def handle_disconnect(self):
        self.is_connected = False
        self.connect_btn.setText("Connect")
        if self.listener:
            self.listener.stop()

        self.ip_input.setEnabled(True)
        for key in self.key_inputs:
            self.key_inputs[key].setEnabled(True)
        for key in self.key_labels:
            self.key_labels[key].setStyleSheet("font-weight: normal")

    def closeEvent(self, event):
        if self.listener and self.listener.isRunning():
            self.listener.stop()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = WiPadRetroLink()
    sys.exit(app.exec_())
