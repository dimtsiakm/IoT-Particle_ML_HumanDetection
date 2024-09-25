import paho.mqtt.client as mqtt
import time
from sklearn.linear_model import LinearRegression

HUMIDITY_value = 0
TEMPERATURE_value = 0
DEWPOINT_value = 0
HEATINDEX_value = 0

# Load the data
data = []
with open("human.txt", "r") as f:
    for line in f:
        data.append([float(x) for x in line.strip().split(",")])

# Split the data into features and target
X = [[x[0], x[1], x[2], x[3]] for x in data]
y = [1 for _ in data]

with open("no_human.txt", "r") as f:
    for line in f:
        data.append([float(x) for x in line.strip().split(",")])
        
# Split the data into features and target
X += [[x[0], x[1], x[2], x[3]] for x in data]
y += [-1 for _ in data]

# Create a linear regression model
model = LinearRegression()

print(len(X), len(y))

# Train the model
model.fit(X, y)
# Callback function to handle incoming messages
def on_message(client, userdata, msg):
    global HUMIDITY_value, TEMPERATURE_value, DEWPOINT_value, HEATINDEX_value
    # check the topic of the message
    if msg.topic == "Humidity":
        HUMIDITY_value = float(msg.payload.decode())
    elif msg.topic == "Temperature":
        TEMPERATURE_value = float(msg.payload.decode())
    elif msg.topic == "DewPoint":
        DEWPOINT_value = float(msg.payload.decode())
    elif msg.topic == "HeatIndex":
        HEATINDEX_value = float(msg.payload.decode())
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

client.subscribe("isHuman")

# Start the MQTT client
import threading
thread = threading.Thread(target=client.loop_forever)
thread.start()

def inference():
    # Predict the class of the new data
    new_data = [[HUMIDITY_value, TEMPERATURE_value, DEWPOINT_value, HEATINDEX_value]]
    print(new_data)
    prediction = model.predict(new_data)
    print(f"Prediction: {prediction}")

    if prediction[0] > 0:
        print("Human detected!")
        client.publish("isHuman", "1")
    else:
        print("No human detected!")
        client.publish("isHuman", "0")
    time.sleep(1)

while True:
    inference()