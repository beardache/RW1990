//TODO
//1. Manual input
//2. Hex to BYTE
//3. LoRa
//
//-R

#include <OneWire.h>
#include <Devduino.h>

#define ONEWIRE_PIN 11
#define LED_GREEN 12
#define LED_RED 13

OneWire ibutton (ONEWIRE_PIN);
byte buffer[8];
String lastButtonId;
byte lastButtonIdByte[8];
String Id;

boolean waitForButton = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  devduino.begin();
  devduino.attachToIntButton(buttonPressed);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  sendToScreen("Ready","");

}

void sendToScreen(String ID, byte family) {
  display.clear();
  display.print(getOneWireInfo(family), 10, 40, &defaultFont);
  display.print(ID, 10, 30, &defaultFont);
  display.flush();
}

void ledWriteMode(boolean doWrite) {
  if (doWrite) {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH); 
  } else {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);     
  }

}

void readButtonId() {
  String buttonId;
  for(int i = 0; i < 8; i++) {
    buttonId += String(buffer[i], HEX);
    lastButtonIdByte[i] = buffer[i];
  }
  sendToScreen(buttonId, lastButtonIdByte[0]);
  ibutton.reset_search();
  delay(250);
  lastButtonId = buttonId;
}

void writeButtonId() {
  sendToScreen(lastButtonId, lastButtonIdByte[0]);

  while (true) {
    if (!ibutton.search(buffer)) {
      ibutton.reset_search();
      delay(250);
    } else {
      // key pressent, trying to program
      boolean ok = true;
      for (int i = 0; i < 8; i++) {
        if (buffer[i] != lastButtonIdByte[i]) {
          ok = false;
          break;
        }
      }
      if (ok) {
        break;
      }
      sendToScreen(lastButtonId, lastButtonIdByte[0]);

      ibutton.skip();
      ibutton.reset();
      ibutton.write(0xD5);
      for (byte i=0; i<8; i++) {
        wr1990writebyte(lastButtonIdByte[i]);
      }
      ibutton.reset();
    }
    
    if (!waitForButton) {
      waitForButton = false;
      sendToScreen(lastButtonId, lastButtonIdByte[0]);
      break;
    }
  }
  waitForButton = false;  
}

void buttonPressed() {
  //set the waitForButton flag false or true
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200){
   waitForButton = !waitForButton;
  }
  last_interrupt_time = interrupt_time;
}

//hardware implementation would be a DS2482-100
void wr1990writebyte(byte data) {
  for(int data_bit=0; data_bit<8; data_bit++){
    if (data & 1){
      digitalWrite(ONEWIRE_PIN, LOW); 
      pinMode(ONEWIRE_PIN, OUTPUT);
      delayMicroseconds(60);
      pinMode(ONEWIRE_PIN, INPUT); 
      digitalWrite(ONEWIRE_PIN, HIGH);
      delay(10);
    } else {
      digitalWrite(ONEWIRE_PIN, LOW); 
      pinMode(ONEWIRE_PIN, OUTPUT);
      pinMode(ONEWIRE_PIN, INPUT); 
      digitalWrite(ONEWIRE_PIN, HIGH);
      delay(10);
    }
    data = data >> 1;
  }
}

String getOneWireInfo(int family) { 
  switch(family) {
    case 0x01:
      return "DS1990A";
      break;
    case 0x02:
      return "DS1991";
      break;
    case 0x04:
      return "DS1994";
      break;
    case 0x06:
      return "DS1993";
      break;
    case 0x08:
      return "DS1992";
      break;
    case 0x09:
      return "DS1982";
      break;
    case 0x0A:
      return "DS1995";
      break;
    case 0x0B:
      return "DS1985";
      break;
    case 0x0C:
      return "DS1996";
      break;
    case 0x0F:
      return "DS1986";
      break;
    case 0x10:
      return "DS1920/DS18S20";
      break;
    case 0x14:
      return "DS1971";
      break;
    case 0x18:
      return "DS1963S";
      break;
    case 0x1A:
      return "DS1963L";
      break;
    case 0x21:
      return "DS1921";
      break;
    case 0x23:
      return "DS1973";
      break;
    case 0x28:
      return "DS18B20";
      break;
    case 0x33:
      return "DS1961S";
      break;
    case 0x37:
      return "DS1977";
      break;
    case 0x41:
      return "DS1923";
      break;
    case 0x81:
      return "DS1420";
      break;
    case 0x89:
      return "DS1982U";
      break;
    case 0x8B:
      return "DS1985U";
      break;
    case 0x8F:
      return "DS1986U";
      break;
    case 0x22:
      return "";
      break;
    default:
      return "0x"+String(family, HEX);
  }
}

void loop() {
  // check for button press to set waitForButton flag and update the screen according
  if (waitForButton) {
    ledWriteMode(true);
    writeButtonId();
  } else {
    ledWriteMode(false);
    if (ibutton.search (buffer)){
      readButtonId();
    }
  }
}
