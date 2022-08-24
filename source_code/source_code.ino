#include "WiFiEsp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#ifndef HAVE_HWSERIAL1
#endif
#include "SoftwareSerial.h"

SoftwareSerial Serial1(6, 7 );


char ssid[] = "SSU562";
char pass[] = "12345678910";
char date[] = "2021-12-08";
short status = WL_IDLE_STATUS;
short currtemp, currmax = -1;
short currhumidity = -1;
short coronaInfected = -1;
char server[] = "api.openweathermap.org";
char server2[] = "openapi.data.go.kr";
short trig = 12;
short echo = 13;

const short buttonPin[] = {8, 9, 10, 11};

WiFiEspClient client;

void setColorTemp(short red, short green, short blue)
{
  analogWrite(0, red);
  analogWrite(1, green);
  analogWrite(2, blue);
}
void setColorHumid(short red, short green, short blue) {
  analogWrite(3, red);
  analogWrite(4, green);
  analogWrite(5, blue);

}
void setColorCorona(short red, short green, short blue) {
  analogWrite(A2, red);
  analogWrite(A1, green);
  analogWrite(A0, blue);
}

void setup()

{
  for (short x = 0; x < 4; x++)
  {
    pinMode(buttonPin[x], INPUT_PULLUP);
  }

  lcd.init();

  Serial.begin(9600);
  Serial1.begin(9600);
  WiFi.init(&Serial1);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  while ( status != WL_CONNECTED) {
    //Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("Starting connection to server...");

  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    client.println("GET /data/2.5/weather?q=Seoul&appid=acafb560c9ee8dc539c7b579ff608fa8 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  }
  short cnt = 0;
  String humidity;
  String temp, temp1;

  delay(10);
  while (client.available()) {
    cnt++;
    char c = client.read();

    if (cnt > 445 && cnt <= 480)
      temp += c;
    else if (cnt > 500 && cnt <= 533)
      temp1 += c;
    else if (cnt > 534 && cnt <= 586) {
      humidity += c;
    }
  }

  temp = temp.substring(temp.indexOf("temp") + 6, temp.indexOf("temp") + 12);
  currtemp = temp.toFloat() - 271;
  temp1 = temp1.substring(temp1.indexOf("temp_max") + 10, temp1.indexOf("temp_max") + 16);
  currmax = temp1.toFloat() - 271;
  humidity = humidity.substring(humidity.indexOf("humidity") + 10, humidity.indexOf("humidity") + 12);
  currhumidity = humidity.toInt();
  Serial.println(currtemp);
  Serial.println(currmax);
  Serial.println(humidity);

  if (client.connect(server2, 80)) {
    Serial.println("Connected to server2");
    client.println("GET /openapi/service/rest/Covid19/getCovid19InfStateJson?serviceKey=sUKk1HhIkPxk%2BEzLGBaY6iCXuIjKNw0UzbQn3qhEDBjqcD4gLYySfsVNL9KUimayUJwidoUajfM3Qu0iuO68Bg%3D%3D&pageNo=1&numOfRows=10&startCreateDt=20211206&endCreateDt=20211207 HTTP/1.1");
    client.println("Host: openapi.data.go.kr");
    //client.println("Connection: close");
    client.println();
  }
  cnt = 0;
  String corona;
  String corona2;
  while (client.available()) {
    cnt++;
    char c = client.read();
    //Serial.write(c);

    if (cnt > 450 && cnt <= 500) {
      corona += c;
    }
    if (cnt > 710 && cnt <= 760) {
      corona2 += c;
    }
  }
  corona = corona.substring(corona.indexOf("decideCnt") + 10, corona.indexOf("decideCnt") + 16);
  corona2 = corona2.substring(corona2.indexOf("decideCnt") + 10, corona2.indexOf("decideCnt") + 16);
  coronaInfected = corona.toInt() - corona2.toInt();
  Serial.println(coronaInfected);

  pinMode(echo, INPUT);
  pinMode(trig, OUTPUT);

  if (currtemp < 10) {
    setColorTemp(0, 0, 255); //10도 이하 파랑
  } else if (currtemp < 25) {
    setColorTemp(0, 255, 0); //25도 이하 초록
  } else {
    setColorTemp(255, 0, 0); //빨강
  }

  if (currhumidity < 30) {
    setColorHumid(0, 0, 255);
  } else if (currhumidity < 80) {
    setColorHumid(0, 255, 0);
  } else {
    setColorHumid(255, 0, 0);
  }

  if (coronaInfected < 500) {
    setColorCorona(0, 0, 255);
  } else if (coronaInfected < 1500) {
    setColorCorona(0, 255, 0);
  } else {
    setColorCorona(255, 0, 0);
  }
}


long distance, pre;
short buttonState, tmp = 0;
void loop()
{

  long duration;
  digitalWrite(trig, HIGH);
  delay(50);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = ((float)(340 * duration) / 1000) / 2;

  if (distance <= 400 || pre <= 400) {
    if (tmp == -1) {
      lcd.backlight();
      lcd.print("[Main]");
      lcd.setCursor(0, 1);
      lcd.print("Have a nice day!");
      lcd.setCursor(0, 3);
      lcd.print("Today is ");
      lcd.print(date);
      tmp = 0;
    }
    for (short i = 0; i < 4; i++) {
      buttonState = digitalRead(buttonPin[i]);
      if (buttonState == LOW) {
        if (buttonPin[i] == 9) { //1번(메인)
          lcd.clear();
          lcd.print("[Main]");
          lcd.setCursor(0, 1);
          lcd.print("Have a nice day!");
          lcd.setCursor(0, 3);
          lcd.print("Today is ");
          lcd.print(date);
        }

        if (buttonPin[i] == 8) { //2번
          tmp = 0;
          lcd.clear();
          lcd.print("[Tempreture]");
          lcd.setCursor(0, 1);
          lcd.print("Current:");
          lcd.print(currtemp);
          lcd.print(" Max:");
          lcd.print(currmax);
          lcd.setCursor(0, 3);
          lcd.print(date);
          lcd.setCursor(0, 2);
          lcd.print("Threat : ");
          if (currtemp < 9) {
            lcd.print("LOW");
          } else if (currtemp < 20) {
            lcd.print("MIDDLE");
          } else {
            lcd.print("HIGH");
          }
        }

        if (buttonPin[i] == 11) { //3번
          tmp = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("[Humidity]");
          lcd.setCursor(0, 1);
          lcd.print("Current : ");
          lcd.print(currhumidity);
          lcd.setCursor(0, 3);
          lcd.print(date);
          lcd.setCursor(0, 2);
          lcd.print("Threat : ");
          if (currhumidity < 40) {
            lcd.print("LOW");
          } else if (currhumidity < 60) {
            lcd.print("MIDDLE");
          } else {
            lcd.print("HIGH");
          }
        }

        if ( buttonPin[i] == 10) {//4번
          tmp = 0;
          lcd.clear();
          lcd.print("[Corona]");
          lcd.setCursor(0, 1);
          lcd.print("Today : ");
          lcd.print(coronaInfected);
          lcd.setCursor(0, 3);
          lcd.print(date);
          lcd.setCursor(0, 2);
          lcd.print("Threat : ");
          if (coronaInfected < 1500) {
            lcd.print("LOW");
          } else if (coronaInfected < 2800) {
            lcd.print("MIDDLE");
          } else {
            lcd.print("HIGH");
          }
        }
      }
    }
  }
  else {
    tmp = -1;
    lcd.noBacklight();
    lcd.clear();
  }
  pre = ((float)(340 * duration) / 1000) / 2;
 
  if (!client.connected()) {
    client.stop();
  }
}
