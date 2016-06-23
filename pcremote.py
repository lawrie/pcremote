import serial
import paho.mqtt.client as mqtt
import time
import struct

def writeInt(n):
  ser.write(struct.pack('>H', n))

def writeString(s):
  writeInt(len(s))
  for i in range (0, len(s)):
    ser.write(s[i])

def readInt():
  x = ord(ser.read()) * 256
  x += ord(ser.read())
  return x

def readString(n):
  s = ""
  for i in range(0, n):
    c = ser.read()
    s += c
  return s

def on_connect(client, userdata, flags, rc):
   print("Connected to MQTT with result code " + str(rc))

def on_message(client, userdata, msg):
   writeString(msg.topic)
   writeString(msg.payload)

CONNECT = 0
DISCONNECT = 1
PUBLISH = 2
SUBSCRIBE = 3

ser = serial.Serial('/dev/ttyUSB0')  # open serial port
connected = False

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

while True:
  # read the type byte
  t = ord(ser.read())
  if t == CONNECT:
    l = readInt()
    server =readString(l)
    p = readInt()
    print "Connecting to " + server + " , port " + str(p)
    r = client.connect(server, p)
    print "Connect result is " + str(r)
    if r == 0:
      client.loop_start()
      connected = True
    else:
      connected = False
  elif t == DISCONNECT:
    r = client.disconnect()
    print "Disconnect result is " + str(r)
    connected = False
    mqtt.loop_stop()
  elif t == PUBLISH:
    l = readInt()
    topic = readString(l)
    l = readInt()
    msg = readString(l)
    print "Publishing topic: " + topic + " msg: " + msg
    r = client.publish(topic, msg)
    print "Publish result is " + str(r)
  elif t == SUBSCRIBE:
    l = readInt()
    topic = readString(l)
    print "Subscribing to " + topic
    r = client.subscribe(topic)
    print "Subscribe result is " + str(r)
	