#include <ESP8266WiFi.h>

#define RPin 15
#define GPin 12
#define BPin 14

WiFiServer server(80); //Set server port

String readString;//String to hold incoming request
String hexString = "000000",BrightString; //Define inititial color here (hex value)

int state;

int r, g, b, x, V;

float brightnessActual = 1;

struct rgb
{
  float r;
  float g;
  float b;
};

rgb StateActual;
rgb StateNext;
rgb StateMessge;

///// WiFi SETTINGS - Replace with your values /////////////////
const char* ssid = "dlink-8842";
const char* password = "vgxht89347";
IPAddress ip(192, 168, 0, 104);   // set a fixed IP for the NodeMCU
IPAddress gateway(192, 168, 0, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
////////////////////////////////////////////////////////////////////

void WiFiStart() {
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(WiFi.localIP());
  server.begin();
}

void OpenSide()
{
  for(int i=0;i<512;i++)
  {
    analogWrite(RPin,(int)(StateActual.r + (StateNext.r/511*i)));
    analogWrite(GPin,(int)(StateActual.g + (StateNext.g/511*i)));
    analogWrite(BPin,(int)(StateActual.b + (StateNext.b/511*i)));
    delay(2);
  }
}

void allOff() {
  state = 0;
  StateMessge.r = 0;
  StateMessge.g = 0;
  StateMessge.b = 0;
  
  StateNext.r = StateMessge.r - StateActual.r;
  StateNext.g = StateMessge.g - StateActual.g;
  StateNext.b = StateMessge.b - StateActual.b;

  OpenSide();

  StateActual.r = abs(StateMessge.r);
  StateActual.g = abs(StateMessge.g);
  StateActual.b = abs(StateMessge.b);
}

//Write requested hex-color to the pins (10bit pwm)
void setHex() 
{
  state = 1;
  SetColor();
}

// Change the brightness when requested.
void setHex(String brightnessString) 
{
  state = 1;
  float brightness = 0;
  brightness = (float)brightnessString.toInt() / 100;
  brightnessActual = brightness;

  SetColor();
}

void SetColor()
{
  long number = (long) strtol( &hexString[0], NULL, 16);
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  StateMessge.r = map(r,0,255,0,1023) * brightnessActual;
  StateMessge.g = map(g,0,255,0,1023) * brightnessActual;
  StateMessge.b = map(b,0,255,0,1023) * brightnessActual;
  
  StateNext.r = StateMessge.r - StateActual.r; // if the result is negative, it will be ignore with analogùWrite().
  StateNext.g = StateMessge.g - StateActual.g;
  StateNext.b = StateMessge.b - StateActual.b;
  
  OpenSide();

  StateActual.r = abs(StateMessge.r);
  StateActual.g = abs(StateMessge.g);
  StateActual.b = abs(StateMessge.b);
}

//send the actual brightness value
void getV() {
  V = brightnessActual * 100;
}

void setup() {
  Serial.begin(115200);
  delay(1);
  WiFi.mode(WIFI_STA);
  WiFiStart();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (client.connected() && !client.available()) {
    delay(1);
  }
  //Respond on certain Homebridge HTTP requests
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == '\n') {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          //On:
          if (readString.indexOf("on") > 0) {
            if(state != 1)
            {
              hexString = "002F2F";
              setHex();
            }
          }
          //Off:
          if (readString.indexOf("off") > 0) {
            allOff();
            //showValues();
          }
          //Set color:
          if (readString.indexOf("setC") > 0) {
            hexString = "";
            hexString = (readString.substring(10, 16));
            setHex();
          }
          if (readString.indexOf("setB") > 0) {
            BrightString = "";
            BrightString = (readString.substring(10, 13));
            setHex(BrightString);
          }
          //Status on/off:
          if (readString.indexOf("status") > 0) {
            client.println(state);
          }
          //Status color (hex):
          if (readString.indexOf("color") > 0) {
            client.println(hexString);
          }
          //Status brightness (%):
          if (readString.indexOf("bright") > 0) {
            getV();
            client.println(V);
          }
          delay(1);
          while (client.read() >= 0);  //added: clear remaining buffer to prevent ECONNRESET
          client.stop();
          readString.remove(0);
        }
      }
    }
  }
}