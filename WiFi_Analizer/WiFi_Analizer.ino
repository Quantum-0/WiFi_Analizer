/*
 * WiFi Analizer
 * 
 * by Quantum0
 */
#include <LiquidCrystal.h>

#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const IPAddress remote_ip(192, 168, 1, 1);
//IPAddress remote_ip;
int networkId = -1;

LiquidCrystal lcd(0, 2, 4, 5, 1, 3);

// This strange shield pinout
// 3 1 16 5 4 X X X       0 2 X X X X GND

int BTN = 16; // Button pin

void(* Reboot) (void) = 0;

void ChooseWiFi(int count)
{
  lcd.print("\x91 Choose Wi-Fi \x90");
  lcd.setCursor(0,1);

  int i = 0;
  while(true)
  {
    if (i == count)
        i = 0;
        
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(i+1);
    lcd.print(") ");
    lcd.print(WiFi.SSID(i));

    delay(100);
    while(digitalRead(BTN))
      delay(5);

    if (digitalRead(BTN) == LOW)
    {
      i++;
      delay(100);
      if (digitalRead(BTN) == LOW)
      {
        delay(300);
        if (digitalRead(BTN) == LOW)
        {
          networkId = --i;
          return;
        }
      }
    }
  }
}

void Connect()
{
  // Print 
  lcd.clear();
  lcd.print(WiFi.SSID(networkId));
  lcd.setCursor(0,1);
  lcd.print("Connecting");

  // Connecting
  WiFi.begin(WiFi.SSID(networkId).c_str(), "");

  int timeout = 40;
  // Wait for connection establishing
  while (WiFi.status() != WL_CONNECTED && timeout > 0)
  {
    timeout--;
    delay(400);
    if (timeout >= 35) // 5 times
      lcd.print(".");
  }
  lcd.setCursor(0,1);

  // Check if connection failed
  if (WiFi.status() != WL_CONNECTED)
  {
    lcd.print("Failed to connect");
    lcd.setCursor(0,1);
    delay(1500);
    lcd.print("Rebooting..      ");
    delay(300);
    Reboot();
  }

  lcd.print(">  Connected.  <");
  //remote_ip = WiFi.localIP();
  
  delay(750);
}

void noWifi()
{
  while(true)
  {
    lcd.setCursor(0,0);
    lcd.print("WiFi networks");
    lcd.setCursor(0,1);
    lcd.print(" are not found");
    
    for(int i = 0; i < 100; i++)
    {
      delay(10);
      if (digitalRead(BTN) == LOW)
        Reboot();
    }
    
    lcd.setCursor(0,0);
    lcd.print("Press button ");
    lcd.setCursor(0,1);
    lcd.print("     to update");
    
    for(int i = 0; i < 100; i++)
    {
      delay(10);
      if (digitalRead(BTN) == LOW)
        Reboot();
    }
  }
}

void setup()
{
  // Setup Button as input
  pinMode(BTN, INPUT);

  // Start LCD with Code Page #1
  lcd.begin(16, 2);
  lcd.command(0b101010);
  lcd.clear();
  lcd.setCursor(0,0);

  // STOP if no WiFi shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    lcd.print("Wi-Fi shield");
    lcd.setCursor(1,1);
    lcd.print("is not found");
    while (true);
  }

  // Set Station Mode for Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Scan for Wi-Fi Networks
  lcd.print("Scan WiFi's...");
  int n = WiFi.scanNetworks();
  lcd.setCursor(0,1);
  if (n == 0)
    lcd.print("WiFi's not found");
  else if (n == 1)
    lcd.print("Found 1 network");
  else
    lcd.print("Found " + String(n) + " networks");
    
  delay(750);
  lcd.clear();
  lcd.setCursor(0,0);

  // Continue waiting for networks
  if (n == 0)
    noWifi();

  // Connect to only one network
  if (n == 1)
  {
    networkId = 0;
    Connect();
  }

  // Choose network and connect
  if (n > 1)
  {
    ChooseWiFi(n);
    lcd.setCursor(0,0);
    Connect();
  }

  // Print static text
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("RSSI:");
  lcd.setCursor(0,1);
  lcd.print("PING:");
}

// Отображение индикатора
void PrintProcessIndicator(char state)
{
  state = state % 4;
  
  char* c;
  if (state == 0)
      c = "|";
  else if (state == 1)
      c = "/";
  else if (state == 2)
      c = "-";
  else
      c = "\\";

  
  lcd.setCursor(15, 0);
  lcd.print(c);
}

char v = 0;
bool loosed = false;
void loop()
{
  // No connection
  if (loosed)
  {
    lcd.clear();
    lcd.print("Lost connection");
    lcd.setCursor(1,1);
    lcd.print("Reconnecting...");
    delay(100);

    if (WiFi.RSSI() < 0)
    {
      lcd.clear();
      lcd.print("The connection");
      lcd.setCursor(1,1);
      lcd.print("is restored");
      delay(500);
      loosed = false;

      // Print static text
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RSSI:");
      lcd.setCursor(0,1);
      lcd.print("PING:");
    }
  }
  else
  // Has connection
  {
    delay(20);
    v++;
    if (v == 32)
      v = 0;
      
    PrintProcessIndicator(v / 4);
    
    long rssi = WiFi.RSSI();
  
    if (rssi < 0)
    {
      lcd.setCursor(6,0);
      lcd.print(rssi);
      lcd.print("db  ");
    
      if (v != 0)
        return;
      
      bool png = Ping.ping(remote_ip, 1);
      lcd.setCursor(6, 1);
      lcd.print("          ");
      lcd.setCursor(6, 1);
      if (png)
        lcd.print(String(Ping.averageTime()) + "ms");
      else
      {
        lcd.print("No ping!");
        delay(100);
      }
    }
    else
      loosed = true;
  }
}
