import json
import os

class WiPadRetroLinkConfigUtil:
    def __init__(self):
        self.config_path = "config.json"
        self.default_config = {
            "up": "w",
            "down": "s",
            "left": "a",
            "right": "d",
            "a": "g",
            "b": "h",
            "x": "b",
            "y": "n",
            "l1": "q",
            "r1": "e",
            "l2": "z",
            "r2": "c",
            "select": "2",
            "start": "1"
        }
        
        # Ensure the config file exists on initialization
        self.ensure_config()
        self.load_config()

    def ensure_config(self):
        """Ensure the config file exists with the correct schema"""
        if not os.path.exists(self.config_path):
            with open(self.config_path, 'w') as file:
                json.dump(self.default_config, file, indent=4)
        else:
            with open(self.config_path, 'r') as file:
                config = json.load(file)
            for key in self.default_config:
                if key not in config:
                    os.remove(self.config_path)
                    self.ensure_config()

    def update_config(self, key, value):
        """Update a single key-value pair in the config file"""
        with open(self.config_path, 'r') as file:
            config = json.load(file)

        config[key] = value

        with open(self.config_path, 'w') as file:
            json.dump(config, file, indent=4)
        
        self.current_config = config
    
    def load_config(self):
        """Load the config file into memory"""
        with open(self.config_path, 'r') as file:
            self.current_config = json.load(file)

    def get_config_value(self, key):
        """Get the value for a given key from the config file"""
        return self.current_config[key]