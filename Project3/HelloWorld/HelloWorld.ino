
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x3F,16 ,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int sysMode=1;
String inputString;
int lightSensorPin = A0;
int modeSwitchPin = 2;
int switchTime;
int backLight=1;
int bootTimeH,bootTimeM,bootTimeS;//20XX-XX-XX 19:00:00=
String bootDate;
void setup()
{
  pinMode(modeSwitchPin, INPUT);
  lcd.init();                      // initialize the lcd 
  lcd.init();
  char newInc;
  Serial.begin(9600);
  // Print a message to the LCD.
  lcd.backlight();
  for(int i=16;i>=0;i--){
  lcd.setCursor(i,0);
  lcd.print("Clock Arduino   ");
  lcd.setCursor(0,1);
  lcd.print("Loading    ");
  if((i-2)%4){
  lcd.setCursor(7,1);
  lcd.print(".");
  if((i-1)%4){
  lcd.setCursor(8,1);
  lcd.print(".");
  if((i)%4){
  lcd.setCursor(9,1);
  lcd.print(".");
  }
  }
  }
  delay(200);
  }
  while(1){
    if(Serial.available()){
    newInc = char(Serial.read());
    if(newInc=='=')break;
    else{
           inputString=inputString+newInc;
           delay(2);}
  }
  }
  lcd.setCursor(0,1);
  lcd.print("Loading Success");
  bootDate = inputString.substring(0,10);
  bootTimeH = inputString.substring(11,13).toInt();
  bootTimeM = inputString.substring(14,16).toInt();
  bootTimeS = inputString.substring(18,19).toInt();
  delay(1000);
  lcd.clear();
}


void loop()
{
  if(sysMode){
    
  lcd.setCursor(0,0);
  lcd.print(bootDate);
  lcd.setCursor(8,1);
  lcd.print(String(bootTimeH));
  lcd.setCursor(10,1);
  if(((bootTimeM+millis()/60000)%60)<10)
  lcd.print(":0"+String((bootTimeM + millis()/60000)%60));
  else
  lcd.print(":"+String((bootTimeM + millis()/60000)%60));
  lcd.setCursor(13,1);
  if(((bootTimeS + millis()/1000)%60)<10)
  lcd.print(":0"+String((bootTimeS + millis()/1000)%60));
  else
  lcd.print(":"+String((bootTimeS + millis()/1000)%60));
  }
  if (digitalRead(modeSwitchPin) == HIGH&&(millis()-switchTime)>400) {
    backLight = !backLight;
    switchTime = millis();
  }
  if(backLight)lcd.backlight();
  else lcd.noBacklight();
  delay(5);
}
