#include <ESP8266WiFi.h>

#define RPin 14
#define GPin 12
#define BPin 13
#define Delta 5

WiFiServer server(80); //Set server port

String readString;//String to hold incoming request
String hexString = "FFFFFF",BrightString = "100"; //Define inititial color here (hex value)

int state;

int r, g, b, x, V;

float brightnessActual = 100;
bool BrightChange = false;

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
const char* ssid = "XXXXXXX";
const char* password = "XXXXXXX";
IPAddress ip(192, 168, 1, 12);   // set a fixed IP for the NodeMCU
IPAddress gateway(192, 168, 1, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
////////////////////////////////////////////////////////////////////

void WiFiStart() {
  Serial.begin(115200);
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
    delay(Delta);
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
  static float previousAlpha = 0,alpha = 0;
  state = 1;
  float newB = (float)brightnessString.toInt();
  alpha = newB / 100;
  if(alpha > previousAlpha && previousAlpha > 0)
    alpha = (alpha / previousAlpha);
  SetColorB(alpha);

  previousAlpha = alpha;
}

void SetColorB(float bright)
{
  long number = (long) strtol( &hexString[0], NULL, 16);
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  StateMessge.r = map(r,0,255,0,1023) * bright;
  StateMessge.g = map(g,0,255,0,1023) * bright;
  StateMessge.b = map(b,0,255,0,1023) * bright;
  
  StateNext.r = StateMessge.r - StateActual.r; // if the result is negative, it will be ignore with analogùWrite().
  StateNext.g = StateMessge.g - StateActual.g;
  StateNext.b = StateMessge.b - StateActual.b;
  
  OpenSide();

  StateActual.r = abs(StateMessge.r);
  StateActual.g = abs(StateMessge.g);
  StateActual.b = abs(StateMessge.b);
}

void SetColor()
{
  long number = (long) strtol( &hexString[0], NULL, 16);
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  StateMessge.r = map(r,0,255,0,1023);
  StateMessge.g = map(g,0,255,0,1023);
  StateMessge.b = map(b,0,255,0,1023);
  
  StateNext.r = StateMessge.r - StateActual.r; // if the result is negative, it will be ignore with analogùWrite().
  StateNext.g = StateMessge.g - StateActual.g;
  StateNext.b = StateMessge.b - StateActual.b;
  
  OpenSide();

  StateActual.r = abs(StateMessge.r);
  StateActual.g = abs(StateMessge.g);
  StateActual.b = abs(StateMessge.b);
}

//send the actual brightness value
void getV() 
{
  int max1 = max(r,g);
  int max2 = max(r,b);
  int Cmax = max(max1,max2);
  int min1 = min(r,g);
  int min2 = min(r,b);
  int Cmin = min(min1,min2);
  V = round((Cmax + Cmin)/2.55);
  brightnessActual = V;
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
          Serial.println(readString);
          if (readString.indexOf("Lon") > 0) {
            setHex();
          }
          //Off:
          if (readString.indexOf("Loff") > 0) {
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
