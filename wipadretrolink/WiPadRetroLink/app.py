import sys
from pynput.keyboard import Controller
from PyQt5.QtCore import Qt, QEvent
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLineEdit, QPushButton, QGridLayout, QLabel, QGroupBox

from .listener_thread import WiPadRetroLinkListenerThread
from .config_util import WiPadRetroLinkConfigUtil

class WiPadRetroLink(QWidget):
    def __init__(self):
        super().__init__()
        self.listener = None
        self.is_connected = False
        self.keyboard = Controller()

        self.assign_to = None

        self.button_states = {
            "up": 0,
            "down": 0,
            "left": 0,
            "right": 0,
            "a": 0,
            "b": 0,
            "x": 0,
            "y": 0,
            "l1": 0,
            "l2": 0,
            "r1": 0,
            "r2": 0,
            "start": 0,
            "select": 0,
        }
        self.config = WiPadRetroLinkConfigUtil()

        self.init_ui()
        self.setFocusPolicy(Qt.StrongFocus)
        QApplication.instance().installEventFilter(self)



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

            key_button = QPushButton(self.config.get_config_value(key))
            key_button.setMaximumWidth(80)
            key_button.setObjectName(key)
            key_button.clicked.connect(self.enable_key_capture)
            self.key_inputs[key] = key_button
            grid_layout.addWidget(key_button, i//2, 2*(i%2) + 1)

        main_layout.addWidget(self.btn_config_group)

        self.setGeometry(300, 100, 320, 400)
        self.setWindowTitle('WiPadRetro Link')
        self.setFixedSize(self.size())
        self.setWindowFlags(Qt.Window | Qt.WindowCloseButtonHint)
        self.show()

    def enable_key_capture(self, keyname):
        self.assign_to = self.sender()
        self.assign_to.setText("<Press>")
        self.assign_to.setFocus()

    def set_key_config(self, key):
        if self.assign_to:
            for k in self.key_inputs:
                if self.key_inputs[k].text() == key:
                    self.key_inputs[k].setText("")
                    self.config.update_config(k, "")
            self.assign_to.setText(key)
            self.config.update_config(self.assign_to.objectName(), key)
            self.assign_to = None



    
    def eventFilter(self, source, event):
        if event.type() == QEvent.KeyPress and self.assign_to:
            arrowkey = ''
            if event.key() in (Qt.Key_Left, Qt.Key_Right, Qt.Key_Up, Qt.Key_Down):
                if event.key() == Qt.Key_Up:
                    arrowkey = 'up'
                elif event.key() == Qt.Key_Down:
                    arrowkey = 'down'
                elif event.key() == Qt.Key_Left:
                    arrowkey = 'left'
                elif event.key() == Qt.Key_Right:
                    arrowkey = 'right'
            else:
                modifiers = event.modifiers()
                mod_str = ''
                if modifiers & Qt.ShiftModifier:
                    mod_str = 'shift'
                if modifiers & Qt.ControlModifier:
                    mod_str = 'ctrl'
                if modifiers & Qt.AltModifier:
                    mod_str = 'alt'
                if modifiers & Qt.MetaModifier:
                    mod_str = 'meta'

            if arrowkey:
                self.set_key_config(arrowkey)
            elif mod_str:
                self.set_key_config(mod_str)
            else:
                self.set_key_config(event.text())
                
            return True
        
        return super().eventFilter(source, event)
    


    def connect_to_server(self):
        if not self.is_connected:
            ip = self.ip_input.text()
            if self.listener is None or not self.listener.isRunning():
                self.listener = WiPadRetroLinkListenerThread(ip)
                self.listener.connected_signal.connect(self.handle_connect)
                self.listener.received_signal.connect(self.update_button_states)
                self.listener.disconnected_signal.connect(self.handle_disconnect)
                self.listener.start()
        else:
            self.handle_disconnect()


    def unpack_button_states(self, data):
        button_states = self.button_states.copy()
        buttons = button_states.keys()
        
        for i, button in enumerate(buttons):
            if data & (1 << i):
                button_states[button] = 1
            else:
                button_states[button] = 0

        return button_states

    def update_button_states(self, data):
        button_states = self.unpack_button_states(data)
        
        for key in self.key_labels:
            if button_states[key]:
                self.key_labels[key].setStyleSheet("font-weight: bold")
                if self.config.get_config_value(key):
                    self.keyboard.press(self.config.get_config_value(key))
            else:
                self.key_labels[key].setStyleSheet("font-weight: normal")
                if self.config.get_config_value(key) and self.button_states[key]:
                    self.keyboard.release(self.config.get_config_value(key))

        self.button_states = button_states

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

def main():
    app = QApplication(sys.argv)
    ex = WiPadRetroLink()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
