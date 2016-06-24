#define CONNECT 0
#define DISCONNECT 1
#define PUBLISH 2
#define SUBSCRIBE 3

#define MAX_TOPIC_SIZE 200
#define MAX_MSG_SIZE 500

static char topic[MAX_TOPIC_SIZE+1];
static char msg[MAX_MSG_SIZE+1];

#define NUMBER_OF_LEDS 4
static const char leds[4] = {11,10,7,6};

#define RESET 4
#define POWER 5

void writeInt(int n) {
  Serial.write(n >> 8);
  Serial.write(n & 0xFF);
}

int readInt() {
  while(!Serial.available());
  int x = Serial.read() << 8;
  while(!Serial.available());
  x += Serial.read();
  return x;
}

void  readString(char *s, int n) {
  int l = readInt();
  if (l > n) l = n;
  s[l] = 0;
  for(int i=0;i<l;i++) {
    while (!Serial.available());
    s[i] = (char) Serial.read();
  }
}

void connect(const char *server, int port) {
  Serial.write((byte) CONNECT);
  writeInt(strlen(server));
  Serial.write(server);
  writeInt(port);
}

void disconnect() {
  Serial.write((byte) DISCONNECT);
}

void publish(const char *topic, const char *msg) {
  Serial.write((byte) PUBLISH);
  writeInt(strlen(topic));
  Serial.write(topic);
  writeInt(strlen(msg));
  Serial.write(msg);
}

void subscribe(const char *topic) {
  Serial.write((byte) SUBSCRIBE);
  writeInt(strlen(topic));
  Serial.write(topic);
}

bool startsWith(const char *pre, const char *str){
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void msgReceived(const char *topic, const char * msg) {
  char buffer[MAX_TOPIC_SIZE + MAX_MSG_SIZE + 1];
  strcpy(buffer,topic);
  strcat(buffer," : ");
  strcat(buffer, msg);
  publish("pcreply/debug", buffer);
  char* subtopic = &strchr(topic, '/')[1];
  if (startsWith("led/", subtopic)) {
    int n = atoi(&subtopic[4]);
    if (n >= 1 && n <= NUMBER_OF_LEDS) {
      int state = (strcmp(msg,"on") == 0  ? LOW : HIGH);
      sprintf(buffer,"Switch %s led %d",msg, n);
      publish("pcreply/debug", buffer);     
      led(n-1, state);
    }
  } else if (startsWith("analog/", subtopic)) {
    int n = atoi(&subtopic[7]);
    if ( n >= 0 && n <= 7) {
      int a = analogRead(n);
      sprintf(buffer,"A%d: %d",n, a);
      publish("pcreply/analog", buffer);     
    }
  } else if (startsWith("power", subtopic)) {
    sprintf(buffer,"Switch %s power",msg);
    publish("pcreply/debug", buffer); 
    int state = (strcmp(msg,"on") == 0  ? HIGH : LOW);
    digitalWrite(POWER,state);
  } else if (startsWith("reset", subtopic)) {
    sprintf(buffer,"Switch %s reset",msg);
    publish("pcreply/debug", buffer); 
    int state = (strcmp(msg,"on") == 0  ? HIGH : LOW);
    digitalWrite(RESET,state);
  }
}

void led(int n, int state) {
  digitalWrite(leds[n], state);
}

void setup() {
  for(int i=0;i<NUMBER_OF_LEDS;i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], HIGH);
  }
  pinMode(POWER,OUTPUT);
  pinMode(RESET,OUTPUT);
  Serial.begin(9600);
  connect("192.168.0.101", 1883);
  delay(2000);
  subscribe("pcremote/#");
}

void loop() {
  if (Serial.available() >= 2) {
    readString(topic, MAX_TOPIC_SIZE);
    readString(msg, MAX_MSG_SIZE);
    msgReceived(topic, msg);
  }
}

