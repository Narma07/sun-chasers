from flask import Flask, render_template
from flask_socketio import SocketIO
import serial
import json
import threading
import time
import requests
from datetime import datetime
from config import *

app = Flask(__name__)
socketio = SocketIO(app)

# சீரியல் கம்யூனிகேஷன் அமைப்பு
SERIAL_PORT = 'COM3'  # உங்கள் Arduino போர்ட்டை பொறுத்து மாற்றவும்
BAUD_RATE = 9600

# தரவு சேமிப்பு
solar_data = {
    'current': [],
    'voltage': [],
    'power': [],
    'timestamp': []
}

def get_weather_data():
    """Fetch weather data from OpenWeatherMap API"""
    try:
        url = f"https://api.openweathermap.org/data/2.5/weather?lat={DEFAULT_LATITUDE}&lon={DEFAULT_LONGITUDE}&appid={OPENWEATHERMAP_API_KEY}&units=metric"
        response = requests.get(url)
        if response.status_code == 200:
            data = response.json()
            return {
                'temp': data['main']['temp'],
                'weather': data['weather'][0]['main'],
                'humidity': data['main']['humidity'],
                'windSpeed': round(data['wind']['speed'] * 3.6, 1)  # Convert m/s to km/h
            }
        return None
    except Exception as e:
        print(f"Weather API error: {e}")
        return None

def read_serial_data():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
        last_weather_update = 0
        
        while True:
            current_time = time.time()
            
            # Update weather data every 5 minutes
            if current_time - last_weather_update >= WEATHER_UPDATE_INTERVAL:
                weather_data = get_weather_data()
                if weather_data:
                    socketio.emit('weather_data', weather_data)
                last_weather_update = current_time
            
            if ser.in_waiting:
                line = ser.readline().decode('utf-8').strip()
                try:
                    data = json.loads(line)
                    # Store solar data
                    solar_data['current'].append(data['current'])
                    solar_data['voltage'].append(data['voltage'])
                    solar_data['power'].append(data['power'])
                    solar_data['timestamp'].append(time.strftime('%H:%M:%S'))
                    
                    # Keep only last 50 data points
                    for key in solar_data:
                        if len(solar_data[key]) > 50:
                            solar_data[key] = solar_data[key][-50:]
                    
                    # Emit solar data update
                    socketio.emit('monitoring_data', {
                        'energy': data['power'] * (5/3600),  # Convert to Wh
                        'efficiency': (data['power'] / 100) * 100,  # Calculate efficiency
                        'battery': data['voltage'] * 10,  # Estimate battery level
                        'temperature': data.get('temperature', 25)  # Get temperature or default
                    })
                    
                except json.JSONDecodeError:
                    print("Invalid JSON data received")
            time.sleep(0.1)
    except serial.SerialException:
        print(f"Could not open port {SERIAL_PORT}")

@app.route('/')
def index():
    return render_template('index.html', 
                         api_key=OPENWEATHERMAP_API_KEY,
                         latitude=DEFAULT_LATITUDE,
                         longitude=DEFAULT_LONGITUDE)

@socketio.on('connect')
def handle_connect():
    print('Client connected')

@socketio.on('request_weather')
def handle_weather_request():
    weather_data = get_weather_data()
    if weather_data:
        socketio.emit('weather_data', weather_data)

@socketio.on('request_monitoring')
def handle_monitoring_request():
    if solar_data['current']:
        latest_data = {
            'current': solar_data['current'][-1],
            'voltage': solar_data['voltage'][-1],
            'power': solar_data['power'][-1]
        }
        socketio.emit('monitoring_data', {
            'energy': latest_data['power'] * (5/3600),
            'efficiency': (latest_data['power'] / 100) * 100,
            'battery': latest_data['voltage'] * 10,
            'temperature': 25  # Default temperature
        })

if __name__ == '__main__':
    # சீரியல் தரவு படிக்கும் thread தொடங்குதல்
    serial_thread = threading.Thread(target=read_serial_data)
    serial_thread.daemon = True
    serial_thread.start()
    
    # Flask சர்வர் தொடங்குதல்
    socketio.run(app, debug=True) 