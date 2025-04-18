void setup() {
  // put your setup code here, to run once:

}
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

const int sensorPins[] = {A0, A1, A2, A3};  
const int newSensorPin = 8;  
const int extraSensorPin = 5; 
const int entrySensorPin = 7; 
const int servoPin1 = 3; 
const int servoPin2 = 4; 

#define SS_PIN 10
#define RST_PIN 9

Servo myservo1;
Servo myservo2;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);

unsigned long blockStart = 0;
const unsigned long BLOCK_TIME = 30000;
bool isBlocked = false;

String allowedCards[] = {
  "D5A006AD",
  "11223344"
};
const int MAX_CARDS = 2;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {  
    pinMode(sensorPins[i], INPUT);
  }
  pinMode(newSensorPin, INPUT);  
  pinMode(extraSensorPin, INPUT);
  pinMode(entrySensorPin, INPUT);

  myservo1.attach(servoPin1);            
  myservo1.write(0);                    
  myservo2.attach(servoPin2);
  myservo2.write(0);

  lcd.init();
  lcd.backlight();
  lcd.print("Parking System");
  delay(1000);
  lcd.clear();

  SPI.begin();
  rfid.PCD_Init();

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("RFID Activated");
  delay(1500);
  lcd.clear();
}

void loop() {
  int F1 = 0;
  int F2 = 0;

  
  for (int i = 0; i < 4; i++) {
    if (analogRead(sensorPins[i]) > 100) {
      F1++;
    }
  }
  
  if (digitalRead(newSensorPin) == HIGH) {
    F2++;
  }
  if (digitalRead(extraSensorPin) == HIGH) {
    F2++;
  }

  lcd.setCursor(0, 0);
  lcd.print("F1: ");
  lcd.print(F1);
  lcd.print("  F2: ");
  lcd.print(F2);

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardID = getCardID();
    if (isCardAllowed(cardID)) {
      openExitGate();
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Access Denied");
      delay(2000);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    lcd.clear();
  }

  handleGateLogic(F1 + F2, digitalRead(entrySensorPin) == HIGH);
  delay(500);
}

String getCardID() {
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardID += rfid.uid.uidByte[i] < 0x10 ? "0" : "";
    cardID += String(rfid.uid.uidByte[i], HEX);
  }
  cardID.toUpperCase();
  return cardID;
}

bool isCardAllowed(String cardID) {
  for (int i = 0; i < MAX_CARDS; i++) {
    if (cardID == allowedCards[i]) {
      return true;
    }
  }
  return false;
}

void openExitGate() {
  myservo2.write(90);  
  lcd.setCursor(0, 1);
  lcd.print("Exit Gate Opened");
  Serial.println("Exit Gate Opened");
  delay(3000);        
  myservo2.write(0);  
  lcd.clear();
}

void handleGateLogic(int totalOccupied, bool gateTriggered) {
  if (totalOccupied == 0) {
    if (!isBlocked) { 
      blockStart = millis();
      isBlocked = true;
      lcd.setCursor(0, 1);
      lcd.print("PARKING FULL!      ");
      Serial.println("Parking Full");
    }
    myservo1.write(90);
    if (millis() - blockStart >= BLOCK_TIME) isBlocked = false;
  } 
  else if (gateTriggered) {
    myservo1.write(90);    
    lcd.setCursor(0, 1);
    lcd.print("Entry Gate Open");
    Serial.println("Entry Gate Open");
    isBlocked = false;
  } 
  else {                 
    myservo1.write(0); 
    delay(3000);
    myservo2.write(0);     
    lcd.setCursor(0, 1);
    lcd.print("Gates Closed");
    Serial.println("Gates Closed");
    isBlocked = false;
  }
}
void loop() {
  // put your main code here, to run repeatedly:

}
