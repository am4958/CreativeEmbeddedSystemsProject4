#include <ESP32Servo.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h> //new
#define USE_SERIAL Serial //new

int outPorts[] = {14, 27, 26, 25};
Servo myservo; // create servo object to control a servo
int posVal = 0; // variable to store the servo position
int servoPin = 15; // Servo motor pin

int delayval = 100;

//wifi stuff
//set up to connect to an existing network (e.g. mobile hotspot from laptop that will run the python code)
const char* ssid = "APlaceToDequeue";
const char* password = "elsebeer++";
String address= "http://165.227.76.232:3000/am4958/running";
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  //  port to listen on
char incomingPacket[255];  // buffer for incoming packets
//wifi end

void setup() {
// set pins to output
for (int i = 0; i < 4; i++) {
pinMode(outPorts[i], OUTPUT);
}
//servo code
myservo.setPeriodHertz(50); // standard 50 hz servo
myservo.attach(servoPin, 500, 2500);
 
//wifi
int status = WL_IDLE_STATUS;
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to wifi");
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  // we recv one packet from the remote so we can know its IP and port
  bool readPacket = false;
  while (!readPacket) {
    int packetSize = Udp.parsePacket();
    if (packetSize)
     {
      // receive incoming UDP packets
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(incomingPacket, 255);
      if (len > 0)
      {
        incomingPacket[len] = 0;
      }
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
      readPacket = true;
    } 
  }
  //end wifi
}

void loop(){
if((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(address);
 
    int httpCode = http.GET(); // start connection and send HTTP header
    if (httpCode == HTTP_CODE_OK) { 
        String response = http.getString();
        if (response.equals("false")) {
         delay(3000);
            // Do not run sculpture, perhaps sleep for a couple seconds
        }
        else if(response.equals("true")) {
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.endPacket();
          delay(1000);
          Serial.println("pre swing post circle");
          servomoveright();
          servomoveleft();
          servomovecenter();
          delay(1000);
          Serial.println("bottom of swingy");
          moveSteps(false, 32 * 64, 5); //3 to 5
          delay(1000); 
            // Run sculpture
        }
        USE_SERIAL.println("Response was: " + response);
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(500); // sleep for half of a second
  }

//end max code
  
//wifi if this doesnt work probs check in on the delays
  // once we know where we got the inital packet from, send data back to that IP address and port

 //moveSteps(false, 32 * 64, 3);
 //delay(1000);

}


void servomoveright(){
   for (posVal = 90; posVal <= 150; posVal += 1) { // goes from 90 degrees to 150 degrees
      // in steps of 1 degree
      Serial.println("here");
      myservo.write(posVal); // tell servo to go to position in variable 'pos'
      Serial.println("here2");
      delay(15); // waits 15ms for the servo to reach the position
    }
}
void servomoveleft(){   
  for (posVal = 150; posVal >= 30; posVal -= 1) { // goes from 150 degrees to 30 degrees
      Serial.println("here1");
      myservo.write(posVal); // tell servo to go to position in variable 'pos'
      Serial.println("here12");
      delay(15); // waits 15ms for the servo to reach the position
  }
}
void servomovecenter(){
  for (posVal = 30; posVal <= 90; posVal += 1) { // goes from 30 degrees to 90 degrees (recenter)
        // in steps of 1 degree
        Serial.println("here");
        myservo.write(posVal); // tell servo to go to position in variable 'pos'
        Serial.println("here2");
        delay(15); // waits 15ms for the servo to reach the position
      }
  
}

//Suggestion: the motor turns precisely when the ms range is between 3 and 20
void moveSteps(bool dir, int steps, byte ms) {
  for (unsigned long i = 0; i < steps; i++) {
     moveOneStep(dir); // Rotate a step
     delay(constrain(ms,3,20)); // Control the speed
}
}
void moveOneStep(bool dir) {
  // Define a variable, use four low bit to indicate the state of port
  static byte out = 0x01;
  Serial.println("ksdjhs");
  // Decide the shift direction according to the rotation direction
  if (dir) { // ring shift left
    out != 0x08 ? out = out << 1 : out = 0x01;
  }
  else { // ring shift right
    out != 0x01 ? out = out >> 1 : out = 0x08;
  }
  // Output singal to each port
  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts[i], (out & (0x01 << i)) ? HIGH : LOW);
  }
}
void moveAround(bool dir, int turns, byte ms){
  for(int i=0;i<turns;i++)
  moveSteps(dir,32*64,ms);
}
void moveAngle(bool dir, int angle, byte ms){
  moveSteps(dir,(angle*32*64/360),ms);
}
