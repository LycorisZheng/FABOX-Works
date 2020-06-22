

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <CurieBLE.h>
#include <Servo.h>
#include "CurieIMU.h"
#define ADDRESS_DS1307 0x68
#define BEEPPIN 3
#define BTN1PIN A0

int dispNum = 1;
String message;
bool alarmTriggered = false,isShocked=false;
LiquidCrystal_I2C lcd(0x3F,16 ,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
BLEService alarmService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE Service
BLEUnsignedCharCharacteristic alarmSetHourCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEUnsignedCharCharacteristic alarmSetMinCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLEWrite);
Servo closeBtn;
int servoAng,fuckCount=0;
int lastHighTick,lastBtnState;

BLEDevice central = BLE.central();

byte timeBcd[] = {0, 0, 0, 0, 0, 0, 0};
//time = {year, month, date, day, hours, minutes, seconds};

int year,month,day,hour,minute,second;
int alarmH=0,alarmM=0;
String weekday;

void setup()
{
  lcd.init();
  lcd.noBacklight();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Initializing");
  Wire.begin();
  Serial.begin(9600);
//  while(!Serial) ;    // wait for serial port to connect..
  CurieIMU.begin();
  CurieIMU.attachInterrupt(eventCallback);

  /* Enable Shock Detection */
  CurieIMU.setDetectionThreshold(CURIE_IMU_SHOCK, 1200); // 1.5g = 1500 mg
  CurieIMU.setDetectionDuration(CURIE_IMU_SHOCK, 50);   // 50ms
  CurieIMU.interrupts(CURIE_IMU_SHOCK);
  Serial.println("IMU initialisation complete, waiting for events...");
  
  BLE.begin();
  BLE.setLocalName("Alarm");
  BLE.setAdvertisedService(alarmService);
  alarmService.addCharacteristic(alarmSetHourCharacteristic);
  alarmService.addCharacteristic(alarmSetMinCharacteristic);
  BLE.addService(alarmService);
  alarmSetHourCharacteristic.setValue(0);
  alarmSetMinCharacteristic.setValue(0);
  BLE.advertise();
  Serial.println("BLE Alarm Peripheral Enabled");
  pinMode(BEEPPIN,OUTPUT);
  closeBtn.attach(9);
    closeBtn.write(180);
  

  lcd.setCursor(0,0);
  lcd.clear();
}

void readTime(){
  //read the time
  Wire.beginTransmission(ADDRESS_DS1307);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(ADDRESS_DS1307, 7);
  if (Wire.available() >= 7)
  {
      for (int i = 0; i < 7; i++)
      {
          timeBcd[6-i] = Wire.read();
      }
  }
//  Convert Hex to Decimal
  year = 2000 + timeBcd[0]/16 * 10 + timeBcd[0]%16;
  month = timeBcd[1]/16 * 10 + timeBcd[1]%16;
  day = timeBcd[2]/16 * 10 + timeBcd[2]%16;
  weekday = BcdToDay(timeBcd[3]);
  hour = timeBcd[4]/16 * 10 + timeBcd[4]%16;
  minute = timeBcd[5]/16 * 10 + timeBcd[5]%16;
  second = timeBcd[6]/16 * 10 + timeBcd[6]%16;
  
    //print
    Serial.print("20"); Serial.print(timeBcd[0], HEX); Serial.print("/");
    Serial.print(timeBcd[1], HEX); Serial.print("/"); Serial.print(timeBcd[2], HEX);
    Serial.print(" "); Serial.print(BcdToDay(timeBcd[3])); Serial.print(" ");
    Serial.print(timeBcd[4], HEX); Serial.print(":");
    Serial.print(timeBcd[5], HEX); Serial.print(":");
    Serial.print(timeBcd[6], HEX); Serial.println();
    
}

void dispDate(){
  lcd.setCursor(0,0);
  String printDay,printMon;
  printDay = (day<10)? "0"+String(day) : String(day);
  printMon = (month<10)? "0"+String(month) : String(month);
  lcd.print(String(year)+"/"+printMon+"/"+printDay+"         ");
  lcd.setCursor(0,1);
  lcd.print(weekday);
}

void dispTime(){
  lcd.setCursor(8,1);
  String printH,printM,printS;
  printH = (hour<10)? "0"+String(hour) : String(hour);
  printM = (minute<10)? "0"+String(minute) : String(minute);
  printS = (second<10) ? "0"+String(second) : String(second);
  lcd.print(printH+":"+printM+":"+printS);
}

void showMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(message);
  delay(1000);
  dispNum = 1;
}

void fuckUser(){
  if(millis()-fuckCount < 30000){
  lcd.setCursor(0,0);
  lcd.print("     SCREW             ");
  lcd.setCursor(0,1);
  lcd.print("      YOU                 ");
  closeBtn.write(35);
  }
  else
  isShocked = 0; 
  if(!isShocked)dispNum=1;
  
}

void display(){
  
  switch (dispNum)
  {
  case 1:
    readTime();
    dispDate();
    dispTime(); 
    closeBtn.write(180);
    break;
  case 2:
    showMsg();
    break;
  case 4:
    fuckUser();
  default:
    break;
  }

}

void alarm(){
  if(hour == alarmH&&minute == alarmM&&second == 0)
    alarmTriggered = true; 
  if(alarmTriggered&&(millis()%200)>100&&millis()%1000>500){
    analogWrite(BEEPPIN,128);
  }else  
  analogWrite(BEEPPIN,255);
  if(analogRead(BTN1PIN)==1023&&lastBtnState<1000)
    lastHighTick=millis();
  if(millis()-lastHighTick>200){
    if(analogRead(BTN1PIN)==1023)
    alarmTriggered = false;
  }
  lastBtnState = analogRead(BTN1PIN);
}

void loop()
{
  String alarmTime;
  
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    message = "BLE Connected";
    dispNum = 2;

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (alarmSetHourCharacteristic.written()) {
          alarmH = alarmSetHourCharacteristic.value();
          message = "Alarm at " + String(alarmH) + ":" + String(alarmM);
          dispNum = 2;
      }
      if (alarmSetMinCharacteristic.written()) {
          alarmM = alarmSetMinCharacteristic.value();
          message = "Alarm at " + String(alarmH) + ":" + String(alarmM);
          dispNum = 2;
      }
      display();
      alarm();
    }

    message = "BLE Disconn";
    dispNum = 2;
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
  display();
  alarm();
  Serial.print(String(analogRead(A0)));
}


// Convert binary coded decimal to day
String BcdToDay(byte val)
{
    String res;
    switch(val)
    {
        case 1: res = "SUN     "; break;
        case 2: res = "MON     "; break;
        case 3: res = "TUE     "; break;
        case 4: res = "WED     "; break;
        case 5: res = "THU     "; break;
        case 6: res = "FRI     "; break;
        case 7: res = "SAT     "; break;
        default: res = "        ";
    }
    return res;
}


static void eventCallback(void)
{
  if (CurieIMU.getInterruptStatus(CURIE_IMU_SHOCK)) {
    if (CurieIMU.shockDetected(X_AXIS, POSITIVE))
      Serial.println("Negative shock detected on X-axis");
    if (CurieIMU.shockDetected(X_AXIS, NEGATIVE))
      Serial.println("Positive shock detected on X-axis");
    if (CurieIMU.shockDetected(Y_AXIS, POSITIVE))
      Serial.println("Negative shock detected on Y-axis");
    if (CurieIMU.shockDetected(Y_AXIS, NEGATIVE))
      Serial.println("Positive shock detected on Y-axis");
    if (CurieIMU.shockDetected(Z_AXIS, POSITIVE)){
      Serial.println("Negative shock detected on Z-axis");
      if(alarmTriggered){
        fuckCount = millis();
        isShocked = true;
        dispNum = 4;
      }
    }
    if (CurieIMU.shockDetected(Z_AXIS, NEGATIVE))
      Serial.println("Positive shock detected on Z-axis");
  }
}
