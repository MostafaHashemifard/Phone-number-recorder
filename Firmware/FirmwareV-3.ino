#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"
#include "Wire.h"
#include "Keypad.h"
#include <EEPROM.h>
#include "HardwareSerial.h"
#include <ESP8266WebServer.h>
#include "OLEDDisplayUi.h"
#include "images.h"
#include "fonts.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

IPAddress apIP(192, 168, 40, 1); // Defining a static IP address: local & gateway

const char *ver = "3.00";
const char *APssid = "Baran";
const char *APpassword = "1234ABCD"; //DEBUG_HTTPCLIENT

// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);

char ssid[100];
char pass[100];
char devid[100];
char url[100];
int ssidLen = 0;
int passLen = 0;
int devidLen = 0;
int urlLen = 0;
int httpCode = 0;
int cnt = 0;
int it = 0;
String mac;
HTTPClient http;
const int buzzer = D0;
const byte n_rows = 4;
const byte n_cols = 3;
char keys[n_rows][n_cols] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte colPins[n_cols] = {D8, D6, D4};//D8 D6 D4
byte rowPins[n_rows] = {D7, 3, D3, D5};
String pn;

Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

void handleRoot() {

  char html[2000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  // Build an HTML page to display on the web-server root address
  //snprintf ( html, 2000,
  String s = "<html>\
              <head>\
                <meta name='viewport' http-equiv='refresh' content='width=device-width, initial-scale=1'/>\
                <title>Baran WiFi Network</title>\
                <style>\
                  * {\
                    box-sizing: border-box;\
                  }\
                  input[type=text], select, textarea {\
                    width: 100%;\
                    padding: 12px;\
                    border: 1px solid #ccc;\
                    border-radius: 4px;\
                    resize: vertical;\
                  }\
                  input[type=submit] {\
                    background-color: #4CAF50;\
                    color: white;\
                    padding: 12px 20px;\
                    border: none;\
                    border-radius: 4px;\
                    cursor: pointer;\
                    float: right;\
                  }\
                  label {\
                    padding: 12px 12px 12px 0;\
                    display: inline-block;\
                  }\
                  input[type=submit]:hover {\
                    background-color: #45a049;\
                  }\
                  .container {\
                    border-radius: 5px;\
                    background-color: #f2f2f2;\
                    padding: 20px;\
                  }\
                  .col-25 {\
                    float: left;\
                    width: 25%;\
                    margin-top: 6px;\
                  }\
                  .col-75 {\
                    float: left;\
                    width: 75%;\
                    margin-top: 6px;\
                  }\
                 .row:after {\
                    content: '';\
                    display: table;\
                    clear: both;\
                  }\
                  @media screen and (max-width: 600px) {\
                    .col-25, .col-75, input[type=submit] {\
                      width: 100%;\
                      margin-top: 0;\
                    }\
                  }\
                </style>\
              </head>\
  <body>\
    <div class='container'>\
  <form action='/action_page'>\
    <div class='row'>\
      <div class='col-25'>\
        <label for='devid'>Device ID</label>\
      </div>\
      <div class='col-75'>\
        <input type='text' id='devid' name='deviceid' maxlength='10' placeholder='Device ID ...' required>\
      </div>\
    </div>\
    <div class='row'>\
      <div class='col-25'>\
        <label for='ssid'>WiFi SSID</label>\
      </div>\
      <div class='col-75'>\
        <input type='text' id='ssid' name='wifissid' maxlength='30' placeholder='WiFi SSID ...' required>\
      </div>\
    </div>\
    <div class='row'>\
      <div class='col-25'>\
        <label for='password'>WiFi Password</label>\
      </div>\
      <div class='col-75'>\
        <input type='text' id='password' name='wifipassword' maxlength='50' placeholder='WiFi Password ...' required>\
      </div>\
    </div>\
    <div class='row'>\
      <div class='col-25'>\
        <label for='url'>URL</label>\
      </div>\
      <div class='col-75'>\
        <input type='text' id='url' name='urladd' maxlength='100' placeholder='URL Address ...' required>\
      </div>\
    </div>\
    <div class='row'>\
      <input type='submit' value='Submit'>\
    </div>\
  </form>\
</div>\
  </body>\
</html>";
  //);
  server.send ( 200, "text/html", s );
}

void handleForm() {
  display.clear();
  display.setFont(Arimo_Bold_20);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Writing EEPROM");
  display.display();

  String tempSsid = server.arg("wifissid");
  String tempPass = server.arg("wifipassword");
  String tempDevid = server.arg("deviceid");
  String tempUrl = server.arg("urladd");
  //
  tempSsid.toCharArray(ssid, tempSsid.length() + 1);
  tempPass.toCharArray(pass, tempPass.length() + 1);
  tempDevid.toCharArray(devid, tempDevid.length() + 1);
  tempUrl.toCharArray(url, tempUrl.length() + 1);
  //
  ssidLen = tempSsid.length() + 1;
  passLen = tempPass.length() + 1;
  devidLen = tempDevid.length() + 1;
  urlLen = tempUrl.length() + 1;
  //
  ssidEepromWrite();
  passEepromWrite();
  devidEepromWrite();
  urlEepromWrite();
  //
  //DEBUG
  /*
    delay(1000);
    lcd.clear();
    lcd.print(ssid);
    delay(2000);
    lcd.clear();
    lcd.print(pass);
    delay(2000);
    lcd.clear();
    lcd.print(devid);
    delay(2000);
    lcd.clear();
    lcd.print(url);
    delay(2000);
  */
  //DEBUG-END
  //
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Done!");
  display.drawString(0, 20, "Re-plug device");
  display.display();
  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", s); //Send web page
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

String extractDevidFromMac()
{
  String s = WiFi.macAddress().substring(9);
  String devId;
  for (int i = 0; i < s.length(); i++)
  {
    if (s[i] != ':')
      devId += s[i];
  }
  return devId;
}

void setup()
{
  pinMode(buzzer, OUTPUT);
  //lcd.begin();
  //lcd.clear();
  //
  Serial.begin(115200);
  delay(10);
  //Serial.setDebugOutput(true);
  //
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.drawXbm(32, 1, 64, 32, Baran_Logo_bits_1); //WORKED
  //display.drawXbm(1, 1, 50, 50, Baran_Logo_bits_2);
  char txt[] = "گروه نرم افزاری باران";
  PutCharPE(txt, ALIGN_CENTER(txt), 40, 1);
  display.display();
  delay(5000);
  display.clear();
  //
  EEPROM.begin(512);
  //
  if (String(char(EEPROM.read(0x01))) == "f")
  {
    /*//DEBUG
      lcd.clear();
      lcd.print("0x02: " + String(readIntFromEEPROM(0x02)));
      delay(1000);
      lcd.clear();
    */
    ssidLen = readIntFromEEPROM(0x02);
    for (int i = 0; i < ssidLen; i++)
    {
      ssid[i] = char(EEPROM.read(0x04 + i));
    }
    ssid[ssidLen] = '\0';
    /*//Debug
      delay(1000);
      lcd.clear();
      lcd.print(String(ssid));
      delay(1000);*/
  }
  else
    it++;
  if (String(char(EEPROM.read(0x33))) == "f")
  {
    /*//DEBUG
      lcd.clear();
      lcd.print("0x34: " + String(readIntFromEEPROM(0x34)));
      delay(1000);
      lcd.clear();
    */
    passLen = readIntFromEEPROM(0x34);
    for (int i = 0; i < passLen; i++)
    {
      pass[i] = char(EEPROM.read(0x36 + i));
    }
    pass[passLen] = '\0';
    /*//Debug
      delay(1000);
      lcd.clear();
      lcd.print(String(pass));
      delay(1000);*/
  }
  else
    it++;
  if (String(char(EEPROM.read(0x65))) == "f")
  {
    /*//DEBUG
      lcd.clear();
      lcd.print("0x66: " + String(readIntFromEEPROM(0x66)));
      delay(1000);
      lcd.clear();
    */
    devidLen = readIntFromEEPROM(0x66);
    for (int i = 0; i < devidLen; i++)
    {
      devid[i] = char(EEPROM.read(0x68 + i));
    }
    devid[devidLen] = '\0';
    /*//Debug
      delay(1000);
      lcd.clear();
      lcd.print(String(devid));
      delay(1000);*/
  }
  else
    it++;
  if (String(char(EEPROM.read(0x74))) == "f")
  {
    /*//DEBUG
      lcd.clear();
      lcd.print("0x75: " + String(readIntFromEEPROM(0x75)));
      delay(1000);
      lcd.clear();
    */
    urlLen = readIntFromEEPROM(0x75);
    for (int i = 0; i < urlLen; i++)
    {
      url[i] = char(EEPROM.read(0x78 + i));
    }
    url[urlLen] = '\0';
    /*//Debug
      delay(1000);
      lcd.clear();
      lcd.print(String(url));
      delay(1000);*/
  }
  else
    it++;
  /*//Debug
    lcd.clear();
    lcd.print(ssid);
    delay(1000);
    lcd.clear();
    lcd.print(pass);
    delay(1000);
    lcd.clear();
  */
  if (it != 0)
  {
    //set-up the custom IP address
    WiFi.mode(WIFI_AP);//WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00

    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(APssid, APpassword);

    IPAddress myIP = WiFi.softAPIP();
    //Serial.print("AP IP address: ");
    //Serial.println(myIP);

    server.on ("/", handleRoot );
    server.on("/action_page", handleForm);
    server.onNotFound ( handleNotFound );

    server.begin();
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(2, 10, "Ready for setup");
    display.drawString(0, 30, "IP: 192.168.40.1");
    display.display();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }
}

int httpPostSend(String S)
{
  WiFiClientSecure Client;
  int ret;
  //
  if (isHttps(S))
  {
    Client.setInsecure();
    //Client.connect(String(url), 443);
    http.begin(Client, S);
  }
  else
  {
    http.begin(S);
  }
  http.addHeader("Content-Type", "text/plain");
  ret = http.POST("N");
  http.end();
  return ret;
}

int httpGetSend(String S)
{
  WiFiClientSecure Client;
  int ret;
  //
  if (isHttps(S))
  {
    Client.setInsecure();
    //Client.connect(String(url), 443);
    http.begin(Client, S);
  }
  else
  {
    http.begin(S);
  }
  http.addHeader("Content-Type", "text/plain");
  ret = http.GET();
  http.end();
  return ret;
}

bool isHttps(String s)
{
  if (s.substring(0, 5) == "https")
    return true;
  return false;
}

void loop()
{
  if (it != 0) //Access point mode
  {
    server.handleClient();
  }
  else
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(0, 0, "Looking WiFi ...");
      display.display();

      WiFi.begin(String(ssid), String(pass));
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(100);
        char key = myKeypad.getKey();
        if (key != NULL)
        {
          if (key == '*')
          {
            digitalWrite(buzzer, HIGH);
            delay(100);
            digitalWrite(buzzer, LOW);
            if (pn.length() == 4)
            {
              if ( pn == "4735")
              {
                display.clear();
                while (true)
                {
                  display.setFont(ArialMT_Plain_16);
                  display.setTextAlignment(TEXT_ALIGN_LEFT);
                  display.drawString(0, 0, "Flashing .");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing ..");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing ...");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing ....");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing .....");
                  display.display();
                  delay(900);
                  display.clear();
                  break;
                }
                flashEeprom();
                WiFi.disconnect();
                ESP.restart();
              }
              else if (pn == "4700")
              {
                showConfig();
                pn = "";
                display.clear();
                display.setFont(ArialMT_Plain_16);
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.drawString(0, 0, "Looking WiFi ...");
                display.display();
              }
              else
              {
                pn.remove(pn.length() - 1, 1);
                clearSecondLceLine(0, 0, "Looking WiFi ...");
                display.setFont(Arimo_Bold_20);
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.drawString(0, 20, pn);
                display.display();
              }
            }
            else
            {
              pn.remove(pn.length() - 1, 1);
              clearSecondLceLine(0, 0, "Looking WiFi ...");
              display.setFont(Arimo_Bold_20);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
              display.drawString(0, 20, pn);
              display.display();
            }
          }
          else if (pn.length() < 4)
          {
            digitalWrite(buzzer, HIGH);
            delay(100);
            digitalWrite(buzzer, LOW);
            pn += key;
            clearSecondLceLine(0, 0, "Looking WiFi ...");
            display.setFont(Arimo_Bold_20);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 20, pn);
            display.display();
          }
        }
      }
      //dumpEeprom();
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(0, 0, "Phone number:");
      display.display();
      WiFi.setSleepMode(WIFI_NONE_SLEEP);
      pn = "";
    }

    char myKey = myKeypad.getKey();
    if (myKey != NULL)
    {
      if (myKey == '*')
      {
        digitalWrite(buzzer, HIGH);
        delay(100);
        digitalWrite(buzzer, LOW);
        if (pn.length() != 0)
        {
          if (pn.length() == 4)
          {
            if ( pn == "4735")
            {
              display.clear();
              while (true)
              {
                  display.setFont(ArialMT_Plain_16);
                  display.setTextAlignment(TEXT_ALIGN_LEFT);
                  display.drawString(0, 0, "Flashing .");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing ..");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing ...");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing ....");
                  display.display();
                  delay(500);
                  display.clear();
                  display.drawString(0, 0, "Flashing .....");
                  display.display();
                  delay(900);
                  display.clear();
                  break;
              }
              flashEeprom();
              WiFi.disconnect();
              ESP.restart();
            }
            else if (pn == "4700")
            {
              showConfig();
              pn = "";
              display.clear();
              display.setFont(ArialMT_Plain_16);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
              display.drawString(0, 0, "Phone number:");
              display.display();
            }
            else
            {
              pn.remove(pn.length() - 1, 1);
              clearSecondLceLine(0, 0, "Phone number:");
              display.setFont(Arimo_Bold_20);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
              display.drawString(0, 20, pn);
              display.display();
            }
          }
          else
          {
            pn.remove(pn.length() - 1, 1);
            clearSecondLceLine(0, 0, "Phone number:");
            display.setFont(Arimo_Bold_20);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 20, pn);
            display.display();
          }
        }
      }
      else if (myKey == '#')
      {
        if (pn.length() == 11)
        {
          digitalWrite(buzzer, HIGH);
          delay(100);
          digitalWrite(buzzer, LOW);
          String tmpUrl = String(url);
          tmpUrl = tmpUrl + "?id=" + String(devid) + "-" + pn;
          clearSecondLceLine(0, 0, "Phone number:");
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 20, "Sending .");
          display.display();
          delay(500);
          //HTTP SEND
          httpCode = httpGetSend(tmpUrl);
          //HTTP SEND END
          //clearSecondLceLine();
          //lcd.setCursor(0, 1);
          while (true)
          {
            clearSecondLceLine(0, 0, "Phone number:");
            display.setFont(ArialMT_Plain_16);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 20, "Sending ..");
            display.display();
            delay(500);
            clearSecondLceLine(0, 0, "Phone number:");
            display.setFont(ArialMT_Plain_16);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 20, "Sending ...");
            display.display();
            delay(500);
            clearSecondLceLine(0, 0, "Phone number:");
            display.setFont(ArialMT_Plain_16);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 20, "Sending ....");
            display.display();
            delay(500);
            if (httpCode == 200)
            {
              clearSecondLceLine(0, 0, "Phone number:");
              display.setFont(ArialMT_Plain_16);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
              display.drawString(0, 20, "Sent");
              display.display();
              delay(500);
              pn = "";
              break;
            }
            else
            {
              clearSecondLceLine(0, 0, "Phone number:");
              display.setFont(ArialMT_Plain_16);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
              display.drawString(0, 20, "Failed " + httpCode);
              display.display();
              WiFi.reconnect();
              //
              delay(2000);
              clearSecondLceLine(0, 0, "Phone number:");
              display.setFont(ArialMT_Plain_16);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
              display.drawString(0, 20, "Re-sending .");
              display.display();
              delay(2000);
              //HTTP SEND
              httpCode = httpGetSend(tmpUrl);
              //HTTP SEND END
              //clearSecondLceLine();
              //lcd.setCursor(0, 1);
              while (true)
              {
                cnt++;
                clearSecondLceLine(0, 0, "Phone number:");
                display.setFont(ArialMT_Plain_16);
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.drawString(0, 20, "Re-sending ..");
                display.display();
                delay(500);
                clearSecondLceLine(0, 0, "Phone number:");
                display.setFont(ArialMT_Plain_16);
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.drawString(0, 20, "Re-sending ...");
                display.display();
                delay(500);
                clearSecondLceLine(0, 0, "Phone number:");
                display.setFont(Arimo_Bold_20);
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.drawString(0, 20, "Re-sending ....");
                display.display();
                delay(500);
                if (httpCode == 200)
                {
                  clearSecondLceLine(0, 0, "Phone number:");
                  display.setFont(ArialMT_Plain_16);
                  display.setTextAlignment(TEXT_ALIGN_LEFT);
                  display.drawString(0, 20, "Sent");
                  display.display();
                  pn = "";
                  delay(2000);
                  break;
                }
                else
                {
                  clearSecondLceLine(0, 0, "Phone number:");
                  display.setFont(ArialMT_Plain_16);
                  display.setTextAlignment(TEXT_ALIGN_LEFT);
                  display.drawString(0, 20, "Failed " + httpCode);
                  display.display();
                  delay(2000);
                  clearSecondLceLine(0, 0, "Phone number:");
                  break;
                }
              }
              if (cnt != 0)
              {
                cnt = 0;
                break;
              }
            }
          }
          clearSecondLceLine(0, 0, "Phone number:");
          display.setFont(Arimo_Bold_20);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 20, pn);
          display.display();
        }
      }
      else if (pn.length() < 11)
      {
        digitalWrite(buzzer, HIGH);
        delay(100);
        digitalWrite(buzzer, LOW);
        pn += String(myKey);
        clearSecondLceLine(0, 0, "Phone number:");
        display.setFont(Arimo_Bold_20);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0, 20, pn);
        display.display();
      }
    }
  }
}

void showConfig()
{
  int it = 0;
  int c = 0;
  int i = 0;
  int j = 0;
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Current Setting");
  display.drawString(0, 20, "Press Confirm");
  display.display();
  //
  while (true)
  {
    char myKey = myKeypad.getKey();
    if (myKey != NULL)
    {
      if (myKey == '#')
      {
        delay(100);
        if (it == 0)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "SSID");
          display.display();
          if (ssidLen > 26)
          {
            for (i = 0; i < ssidLen - 26; i++)
            {
              for (j = i; c < 26; j++)
              {
                display.setFont(ArialMT_Plain_10);
                display.drawString(c * 5, 20, String(ssid[j]));
                display.display();
                c++;
              }
              if ( i != ssidLen - 27)
              {
                c = 0;
                delay(300);
                clearSecondLceLine(0, 0, "SSID");
              }
            }
          }
          else
          {
            clearSecondLceLine(0, 0, "SSID");
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 20, ssid);
            display.display();
          }
        }
        if (it == 1)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "Password");
          display.display();
          if (passLen > 26)
          {
            for (i = 0; i < passLen - 26; i++)
            {
              for (j = i; c < 26; j++)
              {
                display.setFont(ArialMT_Plain_10);
                display.drawString(c * 5, 20, String(pass[j]));
                display.display();
                c++;
              }
              if ( i != passLen - 27)
              {
                c = 0;
                delay(300);
                clearSecondLceLine(0, 0, "Password");
              }
            }
          }
          else
          {
            clearSecondLceLine(0, 0, "Password");
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 20, pass);
            display.display();
          }
        }
        if (it == 2)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "Device ID");
          display.display();
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 20, devid);
          display.display();
        }
        if (it == 3)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "MAC ADDRESS");
          display.display();
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 20, WiFi.macAddress());
          display.display();
        }
        if (it == 4)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "RSSI");
          display.display();
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 20, String(WiFi.RSSI()));
          display.display();
        }
        if (it == 5)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "URL");
          display.display();
          display.setFont(ArialMT_Plain_10);
          if (urlLen > 26)
          {
            for (i = 0; i < urlLen - 26; i++)
            {
              for (j = i; c < 26; j++)
              {
                display.setFont(ArialMT_Plain_10);
                display.drawString(c * 5, 20, String(url[j]));
                display.display();
                c++;
              }
              if ( i != urlLen - 27)
              {
                c = 0;
                delay(300);
                clearSecondLceLine(0, 0, "URL");
              }
            }
          }
          else
          {
            clearSecondLceLine(0, 0, "URL");
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 20, url);   //26 char
            display.display();
          }
        }
        if (it == 6)
        {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 0, "Firmware Ver.");
          display.display();
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 20, ver);
          display.display();
        }
        if (it == 7)
        {
          return;
        }
        it++;
      }
    }
      delay(100);
  }
}

void ssidEepromWrite()
{
  EEPROM.begin(512);
  for (int i = 0; i < ssidLen; i++)
  {
    EEPROM.write(0x04 + i, ssid[i]);
  }
  EEPROM.write(0x01, 'f');
  writeIntIntoEEPROM(0x02, ssidLen);
  EEPROM.commit();
}

void passEepromWrite()
{
  EEPROM.begin(512);
  for (int i = 0; i < passLen; i++)
  {
    EEPROM.write(0x36 + i, pass[i]);
  }
  EEPROM.write(0x33, 'f');
  writeIntIntoEEPROM(0x34, passLen);
  EEPROM.commit();
}

void devidEepromWrite()
{
  EEPROM.begin(512);
  for (int i = 0; i < devidLen; i++)
  {
    EEPROM.write(0x68 + i, devid[i]);
  }
  EEPROM.write(0x65, 'f');
  writeIntIntoEEPROM(0x66, devidLen);
  EEPROM.commit();
}

void urlEepromWrite() //end of this entry would be: 78+100=178 =approx= 200.
{
  EEPROM.begin(512);
  for (int i = 0; i < urlLen; i++)
  {
    EEPROM.write(0x78 + i, url[i]);
  }
  EEPROM.write(0x74, 'f');
  writeIntIntoEEPROM(0x75, urlLen);
  EEPROM.commit();
}

void clearSecondLceLine(int x, int y, String firstLineText)
{
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(x, y, firstLineText);
  display.display();
}

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void flashEeprom()
{
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(0x00 + i, '.');
  }
  writeIntIntoEEPROM(0x02, 1);
  writeIntIntoEEPROM(0x34, 1);
  writeIntIntoEEPROM(0x66, 1);
  writeIntIntoEEPROM(0x75, 1);
  EEPROM.commit();
}

void dumpEeprom()
{
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++)
  {
    Serial.print(char(EEPROM.read(0x00 + i)));
  }
}
