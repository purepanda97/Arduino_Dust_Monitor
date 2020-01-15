#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT11.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte temperatureImage[] = {0x04,0x0A,0x0A,0x0A,0x0E,0x1F,0x1F,0x0E};

byte humidityImage[]= {0x04,0x0E,0x0E,0x1F,0x1F,0x1F,0x1F,0x0E};
byte doImage[] = {0x1C,0x14,0x1C,0x00,0x00,0x00,0x00,0x00};

byte microImage[] = {0x11,0x11,0x11,0x13,0x15,0x18,0x10,0x10};
byte threeImage[] = {0x18,0x04,0x18,0x04,0x18,0x00,0x00,0x00};

//Digital Pin
const int DHT_PIN = 2;
const int DUST_PIN = 8;
const int Speaker = 7;

//Humidity & Temparature Sensor
float humidity = 0;
float temperature = 0;

DHT11 dht11(DHT_PIN);

//Dust Sensor & var
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 2000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float dustDensity = 0;
float dustState = 0;
boolean DustCalculate_RUN = false;
boolean DustCalculate_Done = false;
unsigned int buzzer_count = 0;

//Speaker var
int frequency[3] = {262, 294, 330};
int i;

void initPin() {
  pinMode(DUST_PIN, INPUT);
  pinMode(Speaker, OUTPUT);
}

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, humidityImage);
  lcd.createChar(1, temperatureImage);
  lcd.createChar(2, doImage);
  lcd.createChar(3, microImage);
  lcd.createChar(4, threeImage);
  lcd.home();
  
  lcd.print(" *_* START *_* ");
}

void setup()
{
  initPin();
  initLCD();
  starttime = millis();
}

void loop()
{
  if(DustCalculate_RUN == true)
  {
    calcDustDensity();
  
    if(DustCalculate_Done == true) {
      calcHumidityAndTemperature();
      printLCD();
      DustCalculate_Done = false;
    }
  } else {
    if (buzzer_count > 0 ) {
      for(i = 0; i < 3; i++){
        tone(Speaker, frequency[i], 100);
        delay(200);
      }
      buzzer_count--;
    } else digitalWrite(Speaker, LOW);
    
    //Start Time Initialize
    if((dustState > 0 && buzzer_count == 0) || (dustState == 0)) 
    {
      DustCalculate_RUN = true;
      starttime = millis();
    }
  }
}

void printLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dustDensity);
    lcd.write(3);
    lcd.print("g/m");
    lcd.write(4);
    lcd.setCursor(10, 0);
    if(dustState == 0)lcd.print(" Great");
    else if(dustState == 1)lcd.print(" Good ");
    else if(dustState == 2)lcd.print(" Soso ");
    else if(dustState == 3)lcd.print(" Bad ");
    
    lcd.setCursor(0, 1);
    lcd.write(0);
    lcd.print(" ");
    lcd.print(humidity);
    lcd.print("% ");
    lcd.write(1);
    lcd.print(" ");
    lcd.print(temperature);
    lcd.write(2);
    lcd.print("C ");
}

void calcDustDensity() {
  duration = pulseIn(DUST_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;
    
  if ((millis() - starttime) > sampletime_ms) {
    DustCalculate_RUN = false;
    DustCalculate_Done = true;

    ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=>100
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
    dustDensity = concentration * 100 / 13000;
    lowpulseoccupancy = 0;
    
    if(dustDensity > 150) buzzer_count = 3;
    else if(dustDensity > 5) buzzer_count = 2;
    else if(dustDensity > 3) buzzer_count = 1;
    else buzzer_count = 0;
    
    dustState = buzzer_count;
  }
}

void calcHumidityAndTemperature() {
  dht11.read(humidity, temperature);
}
