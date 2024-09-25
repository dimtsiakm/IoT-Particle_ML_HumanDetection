import paho.mqtt.client as mqtt
import time
from sklearn.linear_model import LinearRegression

humidity = []
temperature = []
dew_point = []
heat_index = []

# Callback function to handle incoming messages
def on_message(client, userdata, msg):
    # check the topic of the message
    if msg.topic == "Humidity":
        print(f"Received humidity: {msg.payload.decode()}")
        humidity.append(float(msg.payload.decode()))
    elif msg.topic == "Temperature":
        print(f"Received temperature: {msg.payload.decode()}")
        temperature.append(float(msg.payload.decode()))
    elif msg.topic == "DewPoint":
        print(f"Received dew point: {msg.payload.decode()}")
        dew_point.append(float(msg.payload.decode()))
    elif msg.topic == "HeatIndex":
        print(f"Received heat index: {msg.payload.decode()}")
        heat_index.append(float(msg.payload.decode()))
    else:
        print(f"Received message: {msg.payload.decode()}")

# Create MQTT client
client = mqtt.Client()

# Set the callback function
client.on_message = on_message

# Connect to the MQTT broker
client.connect("localhost", 1883)

# Subscribe to the "Humidity" topic
client.subscribe("Humidity")
client.subscribe("Temperature")
client.subscribe("DewPoint")
client.subscribe("HeatIndex")

# Acquire the dataset
def save_data():
    # Start the network loop with a timeout
    start_time = time.time()
    while time.time() - start_time < 60.0:
        client.loop(timeout=0.1)

    # save the human to a file
    with open("human.txt", "w") as f:
        for i in range(len(humidity)):
            f.write(f"{humidity[i]},{temperature[i]},{dew_point[i]},{heat_index[i]}\n")