from flask import Flask, request, jsonify, render_template
import sqlite3
import os
import json
import csv
from datetime import datetime
from langdetect import detect
import paho.mqtt.client as mqtt
import threading
import traceback
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

app = Flask(__name__, template_folder='templates', static_folder='static')

# Configuration
os.makedirs('databases', exist_ok=True)
os.makedirs('sensor_data', exist_ok=True)

# MQTT Configuration
MQTT_BROKER = "192.168.42.172"  # Your MQTT broker IP
MQTT_PORT = 1883
MQTT_TOPICS = [
    "esp32/gas/level",
    "esp32/flame/status",
    "esp32/alert",
    "esp32/dht11/temperature",
    "esp32/dht11/humidity",
    "esp32/sound/level",
    "esp32/sound/status"
]

# Email Configuration (Gmail SMTP)
EMAIL_SENDER = "w.d.b.house.3@gmail.com"  
EMAIL_PASSWORD = "kowq nlhs tutx cdho"    
EMAIL_RECEIVER = "w.d.b.house.3@gmail.com"  
SMTP_SERVER = "smtp.gmail.com"
SMTP_PORT = 587  #  465 for SSL or 587 for TLS


# Global sensor data storage
sensor_data = {
    "temperature": None,
    "humidity": None,
    "sound_level": None,
    "sound_status": "disabled",
    "last_updated": None,
    "gas_level": None,
    "flame_status": None,
    "white_led_status": "OFF"

}

# Database initialization
def init_db():
    conn = sqlite3.connect('databases/w_d_b.db')
    c = conn.cursor()

    c.execute('''CREATE TABLE IF NOT EXISTS logins 
                 (house_number TEXT, username TEXT, email TEXT, login_time TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS logouts 
                 (house_number TEXT, username TEXT, email TEXT, logout_time TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS feedback 
                 (name TEXT, email TEXT, house_number TEXT, message TEXT, rating TEXT, timestamp TEXT)''')

    for house_number in [1, 2, 3]:
        c.execute(f'''CREATE TABLE IF NOT EXISTS chat_history_{house_number} 
                     (username TEXT, message TEXT, response TEXT, timestamp TEXT)''')

    conn.commit()
    conn.close()

init_db()

# MQTT Client Setup
def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    for topic in MQTT_TOPICS:
        client.subscribe(topic)
        print(f"Subscribed to {topic}")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    print(f"Received message on {topic}: {payload}")
    
    try:
        # Alert thresholds (adjust as needed)
        TEMP_THRESHOLD = 35.0     # °C
        HUMIDITY_THRESHOLD = 80    # %
        SOUND_THRESHOLD = 4000     # Analog value
        GAS_THRESHOLD = 2000       # ppm

        if topic == "esp32/dht11/temperature":
            temp = float(payload)
            sensor_data["temperature"] = temp
            save_sensor_data("temperature.csv", payload, timestamp)
            if temp > TEMP_THRESHOLD:
                send_email_alert("High Temperature", f"Temperature: {temp}°C")
                
        elif topic == "esp32/dht11/humidity":
            hum = float(payload)
            sensor_data["humidity"] = hum
            save_sensor_data("humidity.csv", payload, timestamp)
            if hum > HUMIDITY_THRESHOLD:
                send_email_alert("High Humidity", f"Humidity: {hum}%")
                
        elif topic == "esp32/sound/level":
            sound = int(payload)
            sensor_data["sound_level"] = sound
            save_sensor_data("sound.csv", payload, timestamp)
            if sound > SOUND_THRESHOLD:
                send_email_alert("High Sound", f"Sound level: {sound}")
                
        elif topic == "esp32/sound/status":
            sensor_data["sound_status"] = payload
            
        elif topic == "esp32/gas/level":
            gas_level = int(payload)
            save_sensor_data("gas.csv", payload, timestamp)
            sensor_data["gas_level"] = gas_level
            if gas_level > GAS_THRESHOLD:
                send_email_alert("Gas Alert", f"Dangerous gas level: {gas_level} ppm")

        elif topic == "esp32/flame/status":
            flame_status = payload  # "DETECTED" or "SAFE"
            save_sensor_data("flame.csv", payload, timestamp)
            sensor_data["flame_status"] = flame_status
            if flame_status == "DETECTED":
                send_email_alert("Fire Alert", "Flame detected by sensor!")

        elif topic == "esp32/alert":
            if payload.startswith("GAS:"):
                gas_value = payload.split(":")[1]
                send_email_alert("CRITICAL GAS ALERT", f"Emergency gas level: {gas_value} ppm")
            elif payload == "FIRE":
                send_email_alert("FIRE EMERGENCY", "Immediate action required! Fire detected")
        
        sensor_data["last_updated"] = timestamp

    except Exception as e:
        print(f"Error processing sensor data: {e}")

def save_sensor_data(filename, value, timestamp):
    filepath = os.path.join('sensor_data', filename)
    file_exists = os.path.isfile(filepath)
    
    with open(filepath, 'a', newline='') as f:
        writer = csv.writer(f)
        if not file_exists:
            writer.writerow(["timestamp", "value"])
        writer.writerow([timestamp, value])

def start_mqtt_client():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_forever()
    except Exception as e:
        print(f"MQTT connection error: {e}")

# Start MQTT client in a background thread
mqtt_thread = threading.Thread(target=start_mqtt_client)
mqtt_thread.daemon = True
mqtt_thread.start()

# Load chatbot intents
def load_intents():
    try:
        with open('json/intent.json', 'r', encoding='utf-8') as file:
            return json.load(file)
    except Exception as e:
        print(f"Error loading intents: {e}")
        return None

# Routes
@app.route('/')
def home():
    return render_template('index.html')

@app.route('/get-sensor-data')
def get_sensor_data():
    return jsonify({
        "status": "success",
        "data": sensor_data,
        "last_updated": sensor_data["last_updated"]
    })

@app.route('/submit-feedback', methods=['POST'])
def submit_feedback():
    try:
        data = request.get_json()
        if not data:
            return jsonify({"status": "error", "message": "No data provided!"}), 400

        required_fields = ['name', 'email', 'house_number', 'message', 'rating']
        if not all(field in data for field in required_fields):
            return jsonify({"status": "error", "message": "All fields are required!"}), 400

        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        conn = sqlite3.connect('databases/w_d_b.db')
        c = conn.cursor()
        c.execute(
            "INSERT INTO feedback (name, email, house_number, message, rating, timestamp) VALUES (?, ?, ?, ?, ?, ?)",
            (data['name'], data['email'], data['house_number'], data['message'], data['rating'], timestamp)
        )
        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Feedback submitted successfully!"}), 200

    except sqlite3.Error as e:
        print(f"Database error: {e}")
        return jsonify({"status": "error", "message": "Database error occurred!"}), 500
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"status": "error", "message": "An unexpected error occurred!"}), 500

@app.route('/login', methods=['POST'])
def login():
    try:
        data = request.get_json()
        if not data or 'house_number' not in data or 'username' not in data or 'email' not in data:
            return jsonify({"status": "error", "message": "All fields are required!"}), 400

        if data['house_number'] not in ['1', '2', '3']:
            return jsonify({"status": "error", "message": "Invalid house number!"}), 400

        login_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        conn = sqlite3.connect('databases/w_d_b.db')
        c = conn.cursor()
        c.execute(
            "INSERT INTO logins (house_number, username, email, login_time) VALUES (?, ?, ?, ?)",
            (data['house_number'], data['username'], data['email'], login_time)
        )
        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Login recorded successfully!"}), 200

    except sqlite3.Error as e:
        print(f"Database error: {e}")
        return jsonify({"status": "error", "message": "Database error occurred!"}), 500
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"status": "error", "message": "An unexpected error occurred!"}), 500

@app.route('/logout', methods=['POST'])
def logout():
    try:
        data = request.get_json()
        if not data or 'house_number' not in data or 'username' not in data or 'email' not in data:
            return jsonify({"status": "error", "message": "All fields are required!"}), 400

        logout_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        conn = sqlite3.connect('databases/w_d_b.db')
        c = conn.cursor()
        c.execute(
            "INSERT INTO logouts (house_number, username, email, logout_time) VALUES (?, ?, ?, ?)",
            (data['house_number'], data['username'], data['email'], logout_time)
        )
        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Logout recorded successfully!"}), 200

    except sqlite3.Error as e:
        print(f"Database error: {e}")
        return jsonify({"status": "error", "message": "Database error occurred!"}), 500
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"status": "error", "message": "An unexpected error occurred!"}), 500

@app.route('/chatbot', methods=['POST'])
def chatbot():
    try:
        data = request.get_json()
        if not data:
            return jsonify({"status": "error", "message": "No data provided"}), 400

        required_fields = ['house_number', 'username', 'message']
        if not all(field in data for field in required_fields):
            return jsonify({"status": "error", "message": "Missing required fields"}), 400

        if data['house_number'] not in ['1', '2', '3']:
            return jsonify({"status": "error", "message": "Invalid house number"}), 400

        intents = load_intents()
        if not intents:
            return jsonify({"status": "error", "message": "Failed to load intents"}), 500

        # Détection de langue
        try:
            language = detect(data['message'])
            lang_suffix = '_fr' if language == 'fr' else '_ar' if language == 'ar' else '_en'
        except:
            lang_suffix = '_en'

        user_message_lower = data['message'].lower()
        response = "I'm sorry, I don't understand that."

        # Vérification des données capteurs
        sensor_available = sensor_data['temperature'] is not None and sensor_data['humidity'] is not None

        # Recherche d'intent correspondant
        for intent in intents['intents']:
            if intent['tag'].endswith(lang_suffix):
                for pattern in intent['patterns']:
                    if pattern.lower() in user_message_lower:
                        # Remplacement des variables dans la réponse
                        response = intent['responses'][0] \
                            .replace('%temperature%', str(sensor_data['temperature'])) \
                            .replace('%humidity%', str(sensor_data['humidity']))
                        break

        # Si question sur capteurs mais pas dans les intents
        if any(w in user_message_lower for w in ['temp', 'temperature', 'humidity', 'humid']):
            if sensor_available:
                if 'temp' in user_message_lower and 'humid' in user_message_lower:
                    response = get_sensor_response('both', lang_suffix)
                elif 'temp' in user_message_lower:
                    response = get_sensor_response('temperature', lang_suffix)
                elif 'humid' in user_message_lower:
                    response = get_sensor_response('humidity', lang_suffix)
            else:
                response = get_sensor_response('unavailable', lang_suffix)

        # Enregistrement conversation
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        try:
            conn = sqlite3.connect('databases/w_d_b.db')
            c = conn.cursor()
            c.execute(
                f"INSERT INTO chat_history_{data['house_number']} VALUES (?, ?, ?, ?)",
                (data['username'], data['message'], response, timestamp)
            )
            conn.commit()
        except sqlite3.Error as e:
            print(f"Database error: {e}")
        finally:
            conn.close()

        return jsonify({"response": response})

    except Exception as e:
        print(f"Chatbot error: {traceback.format_exc()}")
        return jsonify({"status": "error", "message": str(e)}), 500

def get_sensor_response(data_type, lang_suffix):
    templates = {
        'temperature': {
            '_en': f"The current temperature is {sensor_data['temperature']}°C",
            '_fr': f"La température actuelle est de {sensor_data['temperature']}°C",
            '_ar': f"درجة الحرارة الحالية هي {sensor_data['temperature']}°C"
        },
        'humidity': {
            '_en': f"The current humidity is {sensor_data['humidity']}%",
            '_fr': f"L'humidité actuelle est de {sensor_data['humidity']}%",
            '_ar': f"الرطوبة الحالية هي {sensor_data['humidity']}%"
        },
        'both': {
            '_en': f"Current conditions: Temperature {sensor_data['temperature']}°C, Humidity {sensor_data['humidity']}%",
            '_fr': f"Conditions actuelles: Température {sensor_data['temperature']}°C, Humidité {sensor_data['humidity']}%",
            '_ar': f"الظروف الحالية: درجة الحرارة {sensor_data['temperature']}°C, الرطوبة {sensor_data['humidity']}%"
        },
        'unavailable': {
            '_en': "Sensor data is currently unavailable",
            '_fr': "Les données des capteurs ne sont pas disponibles",
            '_ar': "بيانات المستشعر غير متوفرة حاليا"
        }
    }
    return templates[data_type].get(lang_suffix, templates[data_type]['_en'])

def send_email_alert(subject, message):
    try:
        from email.mime.text import MIMEText
        from email.mime.multipart import MIMEMultipart
        import smtplib

        msg = MIMEMultipart()
        msg['From'] = EMAIL_SENDER
        msg['To'] = EMAIL_RECEIVER
        msg['Subject'] = f"ESP32 Alert: {subject}"  # Add prefix for clarity

        body = f"""
        🚨 ALERT FROM ESP32 🚨
        --------------------------
        {message}
         Current Sensor Status:
    - Temperature: {sensor_data.get('temperature', 'N/A')}°C
    - Humidity: {sensor_data.get('humidity', 'N/A')}%
    - Gas Level: {sensor_data.get('gas_level', 'N/A')} ppm
    - Flame Status: {sensor_data.get('flame_status', 'N/A')}
    - Sound Level: {sensor_data.get('sound_level', 'N/A')}
        --------------------------
        Timestamp: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
        """
        msg.attach(MIMEText(body, 'plain'))

        with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
            server.starttls()  # Enable TLS encryption
            server.login(EMAIL_SENDER, EMAIL_PASSWORD)
            server.send_message(msg)
            print("Email alert sent to self successfully")
    except Exception as e:
        print(f"Failed to send email: {e}")



if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)