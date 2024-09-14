#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
#include <ESP_Mail_Client.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define BUZZER 5
#define RELAY_PIN 15

#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 465
#define SENDER_EMAIL "techpsit43@gmail.com" 
#define SENDER_PASSWORD "xxxxxxxxxxxxxxxxxxxxxxxxxx" 
#define RECIPIENT_EMAIL "lavitrasahu123@gmail.com" 

const char* ssid = "PSITPSIT";
const char* password = "987654321";



String enteredOTP = "";
String correctOTP = "";
String ids= " ";
SMTPSession smtp;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 14, 27, 12};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String enteredCode = "";
String acode = "";
String secretCode = "123456";
String adminCode = "";
String adminsecretcode = "654321";


bool lockOpened = false;
bool fingerprintScanning = false;


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
unsigned long fingerprintStartTime;

uint8_t id;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for (;;) ; 
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display(); 
  SPI.begin();        
 
  Serial.println("Setup complete.");

  // Fingerprint setup
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi");
        display.clearDisplay();
  centerText("Connecting to WiFi", 25);
  display.display();
  delay(1000);
    
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
        display.clearDisplay();
  centerText(".", 25);
  display.display();
    
  }
  Serial.println("WiFi connected");
          display.clearDisplay();
  centerText("Wifi Connected", 35);
  display.display();
delay(1000);
display.clearDisplay();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  welcomeScreen();
}

void loop() {
  menu();
  if (!lockOpened && !fingerprintScanning) {
    char key = keypad.getKey();
    if (key == 'A') {
      Serial.println("ADMIN MODE");
      display.clearDisplay();
      centerText("ADMIN MODE", 10);
      display.display();
      delay(1000);
          display.clearDisplay();

display.display();
          centerText("Enter Code", 10);

          
      delay(500); 
      String acode = "";

      while(true) {
        key = keypad.getKey();
        if (key == '#') break;

        if (key != NO_KEY) {
          acode += key;
          Serial.println("Admin Code: " + acode);
          display.clearDisplay();
          centerText("Admin Code:", 10);
          centerText(getHiddenCode(acode.length()), 25);
          display.display();
        }

        if (acode.length() == adminsecretcode.length()) {
          if (acode == adminsecretcode) {
            Serial.println("Welcome Admin");
            display.clearDisplay();
            centerText("Hello Admin", 25);
            display.display();
            delay(2000);
            
            adminMode();

            display.clearDisplay();
            acode = "";
            menu();
            break;
          } else {
            Serial.println("Wrong code entered!");
            display.clearDisplay();
            centerText("Wrong code!", 25);
            display.display();
            delay(2000);
            display.clearDisplay();
            acode = "";
            break;
          }
        }
      }

    }
    else if (key == 'B') {
      Serial.println("Layered Mode");
      display.clearDisplay();
      centerText("LAYERED MODE", 10);
      display.display();
      delay(500);
      display.clearDisplay();
      centerText("Enter Code", 10);
      display.display();
      String enteredCode = "";

      while(true) {
        key = keypad.getKey();
        if (key == '#') break;

        if (key != NO_KEY) {
          enteredCode += key;
          Serial.println("Enter Code: " + enteredCode);
          display.clearDisplay();
          centerText("Enter Code:", 10);
          centerText(getHiddenCode(enteredCode.length()), 25);
          display.display();
        }

        if (enteredCode.length() == secretCode.length()) {
          if (enteredCode == secretCode) {
          Serial.println("First layer passed!");
          display.clearDisplay();
          centerText("First Layer Passed", 25);
          display.display();
          delay(2000);
          enteredCode = "";
          startFingerprintScanning();
          return;
          } else {
            Serial.println("Wrong code entered!");
            display.clearDisplay();
            centerText("Wrong code!", 25);
            display.display();
            delay(2000);
            display.clearDisplay();
            enteredCode = ""; 
            break; 
          }
        }
      }

    }
    else if (key == 'C'){
        display.clearDisplay();
  centerText("GO to ADMIN MODE", 10);
  centerText("FIRST", 20);
  display.display();
  delay(1000);
  display.clearDisplay();
    }
      else if (key == 'D'){
        display.clearDisplay();
  centerText("GO to ADMIN MODE", 10);
  centerText("FIRST", 20);
  display.display();
  delay(1000);
  display.clearDisplay();
    }
  } 
}
void adminMode(){

  display.clearDisplay();
  centerText("*:OPEN LOCK", 10);
  centerText("C:Enroll Mode", 20);
  centerText("D:Delete Mode", 35);
  centerText("#:Exit Mode", 50);
  display.display();

  Serial.println("Entered admin mode. Press 'C' to enroll, '#' to exit.");
  char key;
  do {
  centerText("*:OPEN LOCK", 10);
  centerText("C:Enroll Mode", 20);
  centerText("D:Delete Mode", 35);
  centerText("#:Exit Mode", 50);
  display.display();
    key = keypad.getKey();
    if (key) { 
      Serial.print("Key Pressed: ");
      Serial.println(key);
    }
    if (key == 'C') {
      Serial.println("Enroll mode activated.");
        display.clearDisplay();
  centerText("Enroll mode activated", 25);
  display.display();
  delay(1000);
      getFingerprintEnroll();
    }
    if (key == 'D') {
            Serial.println("DELETE mode activated.");
        display.clearDisplay();
  centerText("Delete mode activated", 25);
  display.display();
  delay(1000);
      getFingerprintdelete();
      }
    else if (key == '*'){
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(BUZZER, HIGH);
      lockOpened = true;

      display.clearDisplay();
      centerText("Lock Opened", 25);
      display.display();

      delay(2000);
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(BUZZER, LOW); 
      lockOpened = false;
      display.clearDisplay();  
      return;
    }
  } while (key != '#');
  Serial.println("Exiting admin mode.");
  return;
}
void menu(){

  centerText("A:Admin", 10);
  centerText("B:Layered Mode", 20);
  centerText("C:Enroll Mode", 35);
  centerText("D:Delete Mode", 50);
  return;
}
void welcomeScreen() {
  display.clearDisplay();
  centerText("Welcome", 10);
  display.display();
  delay(2000);
  display.clearDisplay();
  menu();
  
}

void startFingerprintScanning() {
  display.clearDisplay();
  centerText("PLACE YOUR FINGER", 25);
  display.display();
  delay(2000);

  fingerprintScanning = true;
  fingerprintStartTime = millis();

  while(!lockOpened && fingerprintScanning) {
    Serial.println(".");
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - fingerprintStartTime;

    
    if (elapsedTime < 10000) { 
      Serial.println(".");
      getFingerprintID();
      delay(50); 
    } else {
      Serial.println("Fingerprint scanning complete!");
      fingerprintScanning = false;
      
      display.clearDisplay();
      
      break;
    }
  }
}




void getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      return;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return;
    default:
      Serial.println("Unknown error");
      return;
  }

  

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return;
    default:
      Serial.println("Unknown error");
      return;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint scanned!");
        display.clearDisplay();
        centerText("Fingerprint Scanned", 25);
        display.display();
        delay(2000);
    display.clearDisplay();
    centerText("SENDING OTP", 25);
    display.display();
    delay(1000);
    otpcode();

    menu();
    return;

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
        display.clearDisplay();
        centerText("Did not find a match", 25);
        display.display();
        delay(2000);
    return;
  } else {
    Serial.println("Unknown error");
    return;
  }
}

String getHiddenCode(int length) {
  String hiddenCode = "";
  for (int i = 0; i < length; i++) {
    hiddenCode += "*";
  }
  return hiddenCode;
}

void centerText(String text, int yPosition) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, yPosition);
  display.println(text);
  display.display();
}

void showError() {
  display.clearDisplay();
  centerText("Error!", 25);
  display.display();
  delay(2000);
  display.clearDisplay();
  return;

}

void sendOTP(String otp) {
  ESP_Mail_Session session;
  session.server.host_name = SMTP_SERVER;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  session.login.user_domain = "";

  Serial.println("Connecting to SMTP server...");
  display.clearDisplay();
  centerText("Connecting to SMTP server...", 25);
  display.display();
  if (!smtp.connect(&session)) {
    Serial.println("Failed to connect to SMTP server");
    Serial.println("Error reason: " + smtp.errorReason());
    return;
  }

  SMTP_Message message;
  message.sender.name = "OTP Sender";
  message.sender.email = SENDER_EMAIL;
  message.subject = "Your OTP";
  message.addRecipient("Recipient", RECIPIENT_EMAIL);

  String body = "Your OTP is: " + otp;
  message.text.content = body.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  Serial.println("Sending email...");
  display.clearDisplay();
  centerText("Sending Email...", 25);
  display.display();
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email: " + smtp.errorReason());
  } else {
    Serial.println("OTP sent successfully!");
  display.clearDisplay();
  centerText("OTP SENT SUCCESSFULLY", 25);
  display.display();
  delay(2000);
    display.clearDisplay();
  centerText("Enter Code:", 10);
  display.display();

  }
}

void otpcode() {
  correctOTP = generateOTP();
  sendOTP(correctOTP);

  
  while (true) {
    char key = keypad.getKey();
    if (key != NO_KEY && key != 'A') {
      if (enteredOTP.length() < 6) {
        enteredOTP += key;
          display.display();
        display.clearDisplay();
        centerText(enteredOTP, 25);
        
        display.display();
      }

      if (enteredOTP.length() == 6) {
        if (enteredOTP == correctOTP) {
          Serial.println("Correct OTP entered");
          enteredOTP = ""; 
          
          display.clearDisplay();
          centerText("OTP Correct",25);
          display.display();
          delay(1000);
          display.clearDisplay();
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER, HIGH); 
    lockOpened = true;

    display.clearDisplay();
    centerText("Lock Opened", 25);
    display.display();

    delay(2000);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER, LOW); 
    lockOpened = false;
          
          display.display();
          centerText("Lock closed", 25);
          delay(2000);
          display.clearDisplay();

          return;
        } else {
          Serial.println("Incorrect OTP");
          enteredOTP = "";
          display.clearDisplay();
          centerText("Wrong OTP", 25);
          display.display();
          delay(2000);
          display.clearDisplay();
          showError();
        }
        break;
      }
    }
  }
}


String generateOTP() {
  String otp = "";
  for (int i = 0; i < 6; i++) {
    otp += String(random(0, 10));
  }
  return otp;
}


uint8_t getFingerprintEnroll() {
  Serial.println("Please enter the ID # (from 1 to 127), then press '*'.");
  display.clearDisplay();
  centerText("Please enter the ID # (from 1 to 127), then press '*'.", 10);
  display.display();
  id = 0;
  char key;
  do {
    key = keypad.getKey();
  
    if (key >= '0' && key <= '9') {
      id = id * 10 + (key - '0'); 
      Serial.print(key);
      ids+=key;
      display.clearDisplay();
  centerText(ids, 25);
  display.display();
    }
  } while (key != '*');

  Serial.println();

  if (id < 1 || id > 127) {
    Serial.println("Wrong ID input. Please try again.");
        display.clearDisplay();
  centerText("Wrong ID input. Please try again.", 25);
  display.display();
    return false;
  }
  ids = "";
  Serial.print("Enrolling ID #");
    display.clearDisplay();
  centerText("Enrolling ID #", 25);
  display.display();
  Serial.println(id);
  enrollFingerprint();
  return true;
}

void enrollFingerprint() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  display.clearDisplay();
  centerText("Waiting for valid finger to enroll", 10);
  display.display();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
          display.clearDisplay();
  centerText("Image Taken", 25);
  display.display();
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        delay(50);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
            display.clearDisplay();
  centerText("Error", 25);
  display.display();
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
                    display.clearDisplay();
  centerText("Error", 25);
  display.display();
        break;
      default:
        Serial.println("Unknown error");
                    display.clearDisplay();
  centerText("Error", 25);
  display.display();
        break;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image.");
                display.clearDisplay();
  centerText("Error", 25);
  display.display();
    return;
  }

  Serial.println("Remove finger");
    display.clearDisplay();
  centerText("Remove Finger", 25);
  display.display();
  delay(2000);
  p = finger.getImage();
  while (p != FINGERPRINT_NOFINGER) {
    Serial.print(".");
    delay(50);
    p = finger.getImage();
  }

  Serial.println("Place same finger again");
    display.clearDisplay();
  centerText("Place Again", 25);
  display.display();
  p = finger.getImage();
  while (p != FINGERPRINT_OK) {
    Serial.print(".");
    delay(50); 
    p = finger.getImage();
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image.");
                display.clearDisplay();
  centerText("Error", 25);
  display.display();
    return;
  }

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprints matched!");
      display.clearDisplay();
  centerText("Matched", 10);
  display.display();
  delay(1000);
  } else {
    Serial.println("Error creating model.");
                display.clearDisplay();
  centerText("Error", 25);
  display.display();
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored!");
      display.clearDisplay();
  centerText("Fingerprint Stored", 25);
  display.display();
  delay(1000);
  display.clearDisplay();
  } else {
    Serial.println("Error storing fingerprint model.");
                display.clearDisplay();
  centerText("Error", 25);
  display.display();
  }
}


void getFingerprintdelete() {
  Serial.println("Please enter the ID # (from 1 to 127), then press '*'.");
          display.clearDisplay();
  centerText("Please enter the ID # (from 1 to 127), then press '*'.", 25);
  display.display();
  uint8_t id = 0;
  char key;
  do {
    key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      id = id * 10 + (key - '0');
      Serial.print(key);
            ids+=key;
      display.clearDisplay();
  centerText(ids, 25);
  display.display();
      delay(100);
    }
  } while (key != '*');

  if (id < 1 || id > 127) {
    Serial.println("\nWrong ID input. Please try again.");
    display.clearDisplay();
  centerText("Wrong ID input. Please try again.", 25);
  display.display();
    return;
  }
  ids = "";
  Serial.print("\nEnrolling ID #");
  Serial.println(id);
  deleteFingerprint(id);
}

void deleteFingerprint(uint8_t id) {
  Serial.println(id);
  int p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
        display.clearDisplay();
  centerText("Deleted", 25);
  display.display();
  delay(1000);
  display.clearDisplay();
  } else {
    Serial.print("Error: 0x"); Serial.println(p, HEX);
        display.clearDisplay();
  centerText("Error", 25);
  display.display();
  }
}

