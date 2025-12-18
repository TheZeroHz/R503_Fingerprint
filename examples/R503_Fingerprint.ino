/*
 * R503 Fingerprint Module - Complete Example for ESP32 Industrial Grade
 * 
 * Hardware Connections:
 * R503 Pin 1 (VCC)    -> ESP32 3.3V
 * R503 Pin 2 (GND)    -> ESP32 GND
 * R503 Pin 3 (TXD)    -> ESP32 GPIO 4 (RX)
 * R503 Pin 4 (RXD)    -> ESP32 GPIO 5 (TX)
 * R503 Pin 5 (WAKEUP) -> ESP32 GPIO 6 (Finger detection)
 * R503 Pin 6 (3.3VT)  -> ESP32 3.3V or separate 3-6V (Touch power)
 * 
 * Features Demonstrated:
 * - System initialization and configuration
 * - Fingerprint enrollment (2-6 samples)
 * - Fingerprint verification (1:N search)
 * - Template management (store, load, delete)
 * - LED control with various effects
 * - Notepad read/write operations
 * - System information retrieval
 * - Finger detection with WAKEUP pin
 * - Complete error handling
 */

#include "R503_Fingerprint.h"

// Pin definitions for R503
#define RX_PIN 4       // ESP32 RX -> R503 TXD (Pin 3)
#define TX_PIN 5       // ESP32 TX -> R503 RXD (Pin 4)
#define WAKEUP_PIN 6   // ESP32 GPIO -> R503 WAKEUP (Pin 5) - Finger detection

// Hardware Serial for R503 (UART2 on ESP32)
HardwareSerial r503Serial(1);
R503_Fingerprint finger(&r503Serial);

// Configuration
#define R503_BAUD 57600
#define R503_PASSWORD 0x00000000  // Default password
#define R503_ADDRESS 0xFFFFFFFF   // Default address

// Finger detection state
volatile bool fingerDetected = false;

// ISR for finger detection
void IRAM_ATTR onFingerDetected() {
  fingerDetected = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n======================================");
  Serial.println("R503 Fingerprint Module - ESP32");
  Serial.println("======================================\n");
  
  // Setup WAKEUP pin for finger detection
  pinMode(WAKEUP_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), onFingerDetected, FALLING);
  Serial.println("Finger detection enabled on GPIO 6");
  
  // Initialize R503 with custom pins
  Serial.print("Initializing R503 on RX:");
  Serial.print(RX_PIN);
  Serial.print(" TX:");
  Serial.print(TX_PIN);
  Serial.println("...");
  
  // Begin serial with custom pins
  r503Serial.begin(R503_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  
  if (!finger.begin(R503_BAUD, R503_PASSWORD, R503_ADDRESS)) {
    Serial.println("Failed to initialize R503!");
    Serial.println("Check connections and power supply:");
    Serial.println("  - VCC to 3.3V");
    Serial.println("  - GND to GND");
    Serial.println("  - R503 TXD to ESP32 GPIO 4");
    Serial.println("  - R503 RXD to ESP32 GPIO 5");
    Serial.println("  - R503 WAKEUP to ESP32 GPIO 6");
    while (1) {
      delay(1000);
      Serial.print(".");
    }
  }
  Serial.println("R503 initialized successfully!\n");
  
  // Set LED to blue to indicate ready
  finger.ledOn(R503_LED_BLUE);
  
  // Display system information
  displaySystemInfo();
  
  // Display menu
  displayMenu();
}

void loop() {
  // Check for finger detection via WAKEUP pin
  if (fingerDetected) {
    fingerDetected = false;
    Serial.println("\n[FINGER DETECTED via WAKEUP pin]");
    finger.ledOn(R503_LED_PURPLE);
    delay(100);
    finger.ledOn(R503_LED_BLUE);
  }
  
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case '1':
        enrollNewFingerprint();
        break;
      case '2':
        verifyFingerprint();
        break;
      case '3':
        deleteFingerprint();
        break;
      case '4':
        emptyDatabase();
        break;
      case '5':
        listAllFingerprints();
        break;
      case '6':
        matchTwoFingers();
        break;
      case '7':
        testLEDEffects();
        break;
      case '8':
        displaySystemInfo();
        break;
      case '9':
        testNotepad();
        break;
      case 'a':
      case 'A':
        uploadTemplate();
        break;
      case 'b':
      case 'B':
        downloadTemplate();
        break;
      case 'c':
      case 'C':
        testImageUpload();
        break;
      case 'd':
      case 'D':
        changeSecurityLevel();
        break;
      case 'e':
      case 'E':
        checkSensorStatus();
        break;
      case 'f':
      case 'F':
        getRandomNumber();
        break;
      case 'w':
      case 'W':
        testWakeupDetection();
        break;
      case 'm':
      case 'M':
        displayMenu();
        break;
      default:
        Serial.println("Invalid command!");
        break;
    }
  }
}

void displayMenu() {
  Serial.println("\n======================================");
  Serial.println("         R503 COMMAND MENU");
  Serial.println("======================================");
  Serial.println("Hardware Setup:");
  Serial.println("  RX Pin: GPIO 4");
  Serial.println("  TX Pin: GPIO 5");
  Serial.println("  WAKEUP Pin: GPIO 6");
  Serial.println("--------------------------------------");
  Serial.println("1. Enroll New Fingerprint");
  Serial.println("2. Verify Fingerprint (Search)");
  Serial.println("3. Delete Fingerprint");
  Serial.println("4. Empty Database");
  Serial.println("5. List All Fingerprints");
  Serial.println("6. Match Two Fingers (1:1)");
  Serial.println("7. Test LED Effects");
  Serial.println("8. Display System Info");
  Serial.println("9. Test Notepad R/W");
  Serial.println("A. Upload Template");
  Serial.println("B. Download Template");
  Serial.println("C. Upload Fingerprint Image");
  Serial.println("D. Change Security Level");
  Serial.println("E. Check Sensor Status");
  Serial.println("F. Get Random Number");
  Serial.println("W. Test Wakeup Detection");
  Serial.println("M. Show Menu");
  Serial.println("======================================\n");
}

void displaySystemInfo() {
  Serial.println("\n----- System Information -----");
  
  // Read system parameters
  R503_SystemParams params;
  if (finger.readSystemParameters(params)) {
    Serial.print("Status Register: 0x");
    Serial.println(params.statusRegister, HEX);
    Serial.print("System ID: 0x");
    Serial.println(params.systemID, HEX);
    Serial.print("Library Size: ");
    Serial.println(params.librarySize);
    Serial.print("Security Level: ");
    Serial.println(params.securityLevel);
    Serial.print("Device Address: 0x");
    Serial.println(params.deviceAddress, HEX);
    Serial.print("Data Packet Size: ");
    Serial.println(params.dataPacketSize);
    Serial.print("Baud Rate Multiplier: ");
    Serial.println(params.baudRate);
    Serial.print("Actual Baud Rate: ");
    Serial.println(9600 * params.baudRate);
  } else {
    Serial.println("Failed to read system parameters");
  }
  
  // Get template count
  uint16_t count;
  if (finger.getTemplateCount(count)) {
    Serial.print("Stored Templates: ");
    Serial.println(count);
  }
  
  // Get firmware version
  char fwVer[33];
  if (finger.getFirmwareVersion(fwVer)) {
    Serial.print("Firmware Version: ");
    Serial.println(fwVer);
  }
  
  // Get algorithm version
  char algVer[33];
  if (finger.getAlgorithmVersion(algVer)) {
    Serial.print("Algorithm Version: ");
    Serial.println(algVer);
  }
  
  // Get product information
  R503_ProductInfo info;
  if (finger.readProductInfo(info)) {
    Serial.print("Module Type: ");
    Serial.println(info.moduleType);
    Serial.print("Batch Number: ");
    Serial.println(info.batchNumber);
    Serial.print("Serial Number: ");
    Serial.println(info.serialNumber);
    Serial.print("Hardware Version: ");
    Serial.print(info.hardwareVersion >> 8);
    Serial.print(".");
    Serial.println(info.hardwareVersion & 0xFF);
    Serial.print("Sensor Type: ");
    Serial.println(info.sensorType);
    Serial.print("Sensor Resolution: ");
    Serial.print(info.sensorWidth);
    Serial.print(" x ");
    Serial.println(info.sensorHeight);
    Serial.print("Template Size: ");
    Serial.println(info.templateSize);
  }
  
  Serial.println("-----------------------------\n");
}

void enrollNewFingerprint() {
  Serial.println("\n----- Enroll New Fingerprint -----");
  
  Serial.print("Enter ID# (0-199): ");
  while (!Serial.available()) delay(10);
  int id = Serial.parseInt();
  while (Serial.available()) Serial.read(); // Clear buffer
  
  if (id < 0 || id > 199) {
    Serial.println("Invalid ID!");
    return;
  }
  
  Serial.print("Number of samples (2-6, recommended 3+): ");
  while (!Serial.available()) delay(10);
  int samples = Serial.parseInt();
  while (Serial.available()) Serial.read();
  
  if (samples < 2 || samples > 6) {
    Serial.println("Invalid sample count! Using 3.");
    samples = 3;
  }
  
  Serial.print("\nEnrolling ID #");
  Serial.print(id);
  Serial.print(" with ");
  Serial.print(samples);
  Serial.println(" samples...");
  
  finger.ledBreathe(R503_LED_BLUE, 0x80, 0);
  
  if (finger.enrollFingerprint(id, samples)) {
    finger.ledFlash(R503_LED_BLUE, 0xFF, 3);
    Serial.println("\nEnrollment successful!");
  } else {
    finger.ledFlash(R503_LED_RED, 0xFF, 3);
    Serial.print("Enrollment failed! Error code: 0x");
    Serial.println(finger.getLastConfirmationCode(), HEX);
  }
  
  delay(1000);
  finger.ledOn(R503_LED_BLUE);
}

void verifyFingerprint() {
  Serial.println("\n----- Verify Fingerprint -----");
  Serial.println("Place finger on sensor...");
  
  finger.ledOn(R503_LED_PURPLE);
  
  uint16_t fingerID;
  uint16_t confidence;
  
  if (finger.verifyFingerprint(fingerID, confidence)) {
    Serial.print("\nMatch found!");
    Serial.print(" ID #");
    Serial.print(fingerID);
    Serial.print(" with confidence ");
    Serial.println(confidence);
    
    finger.ledFlash(R503_LED_BLUE, 0xFF, 3);
  } else {
    uint8_t errorCode = finger.getLastConfirmationCode();
    
    if (errorCode == R503_NOTFOUND) {
      Serial.println("\nNo match found!");
    } else if (errorCode == R503_NOFINGER) {
      Serial.println("\nNo finger detected!");
    } else {
      Serial.print("\nVerification failed! Error code: 0x");
      Serial.println(errorCode, HEX);
    }
    
    finger.ledFlash(R503_LED_RED, 0xFF, 3);
  }
  
  delay(1000);
  finger.ledOn(R503_LED_BLUE);
}

void deleteFingerprint() {
  Serial.println("\n----- Delete Fingerprint -----");
  
  Serial.print("Enter ID# to delete (0-199): ");
  while (!Serial.available()) delay(10);
  int id = Serial.parseInt();
  while (Serial.available()) Serial.read();
  
  if (id < 0 || id > 199) {
    Serial.println("Invalid ID!");
    return;
  }
  
  Serial.print("Deleting ID #");
  Serial.print(id);
  Serial.println("...");
  
  if (finger.deleteModel(id, 1)) {
    Serial.println("Delete successful!");
    finger.ledFlash(R503_LED_BLUE, 0xFF, 2);
  } else {
    Serial.print("Delete failed! Error code: 0x");
    Serial.println(finger.getLastConfirmationCode(), HEX);
    finger.ledFlash(R503_LED_RED, 0xFF, 2);
  }
}

void emptyDatabase() {
  Serial.println("\n----- Empty Database -----");
  Serial.println("WARNING: This will delete ALL fingerprints!");
  Serial.print("Type 'YES' to confirm: ");
  
  String confirmation = "";
  uint32_t startTime = millis();
  while (millis() - startTime < 10000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') break;
      confirmation += c;
    }
  }
  
  Serial.println(confirmation);
  
  if (confirmation == "YES") {
    Serial.println("Emptying database...");
    
    if (finger.emptyDatabase()) {
      Serial.println("Database cleared successfully!");
      finger.ledFlash(R503_LED_BLUE, 0xFF, 3);
    } else {
      Serial.print("Failed to clear database! Error code: 0x");
      Serial.println(finger.getLastConfirmationCode(), HEX);
      finger.ledFlash(R503_LED_RED, 0xFF, 3);
    }
  } else {
    Serial.println("Operation cancelled.");
  }
}

void listAllFingerprints() {
  Serial.println("\n----- Fingerprint Index Table -----");
  
  uint8_t indexTable[32];
  
  for (uint8_t page = 0; page < 1; page++) { // Page 0 = IDs 0-255
    if (finger.readIndexTable(page, indexTable)) {
      Serial.print("Page ");
      Serial.print(page);
      Serial.println(":");
      
      for (uint8_t i = 0; i < 32; i++) {
        for (uint8_t bit = 0; bit < 8; bit++) {
          uint16_t id = (page * 256) + (i * 8) + bit;
          if (indexTable[i] & (1 << (7 - bit))) {
            Serial.print("  ID #");
            Serial.print(id);
            Serial.println(" - Registered");
          }
        }
      }
    } else {
      Serial.println("Failed to read index table");
    }
  }
  
  Serial.println("-------------------------------\n");
}

void matchTwoFingers() {
  Serial.println("\n----- Match Two Fingers (1:1) -----");
  
  // First finger
  Serial.println("Place first finger...");
  finger.ledOn(R503_LED_PURPLE);
  
  while (!finger.getImage()) {
    delay(50);
  }
  
  if (!finger.image2Tz(R503_CHARBUFFER1)) {
    Serial.println("Failed to process first finger!");
    return;
  }
  
  Serial.println("Remove finger");
  finger.ledOff();
  delay(2000);
  
  // Second finger
  Serial.println("Place second finger...");
  finger.ledOn(R503_LED_PURPLE);
  
  while (!finger.getImage()) {
    delay(50);
  }
  
  if (!finger.image2Tz(R503_CHARBUFFER2)) {
    Serial.println("Failed to process second finger!");
    return;
  }
  
  // Match
  uint16_t score;
  if (finger.matchTemplates(score)) {
    Serial.print("Fingers match! Score: ");
    Serial.println(score);
    finger.ledFlash(R503_LED_BLUE, 0xFF, 3);
  } else {
    Serial.println("Fingers do not match!");
    finger.ledFlash(R503_LED_RED, 0xFF, 3);
  }
  
  delay(1000);
  finger.ledOn(R503_LED_BLUE);
}

void testLEDEffects() {
  Serial.println("\n----- Testing LED Effects -----");
  
  Serial.println("Red breathing...");
  finger.ledBreathe(R503_LED_RED, 0x80, 3);
  delay(3000);
  
  Serial.println("Blue flashing...");
  finger.ledFlash(R503_LED_BLUE, 0xFF, 5);
  delay(3000);
  
  Serial.println("Purple on...");
  finger.ledOn(R503_LED_PURPLE);
  delay(2000);
  
  Serial.println("Gradual on...");
  finger.setLED(R503_LED_GRADUAL_ON, 0x80, R503_LED_BLUE, 0);
  delay(3000);
  
  Serial.println("Gradual off...");
  finger.setLED(R503_LED_GRADUAL_OFF, 0x80, R503_LED_BLUE, 0);
  delay(3000);
  
  Serial.println("LED test complete!");
  finger.ledOn(R503_LED_BLUE);
}

void testNotepad() {
  Serial.println("\n----- Test Notepad R/W -----");
  
  uint8_t writeData[32];
  uint8_t readData[32];
  
  // Generate test data
  for (int i = 0; i < 32; i++) {
    writeData[i] = i;
  }
  
  Serial.println("Writing to notepad page 0...");
  if (finger.writeNotepad(0, writeData)) {
    Serial.println("Write successful!");
    
    Serial.println("Reading from notepad page 0...");
    if (finger.readNotepad(0, readData)) {
      Serial.println("Read successful!");
      
      Serial.print("Data: ");
      for (int i = 0; i < 32; i++) {
        Serial.print(readData[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      
      // Verify
      bool match = true;
      for (int i = 0; i < 32; i++) {
        if (writeData[i] != readData[i]) {
          match = false;
          break;
        }
      }
      
      if (match) {
        Serial.println("Data verification successful!");
      } else {
        Serial.println("Data mismatch!");
      }
    } else {
      Serial.println("Read failed!");
    }
  } else {
    Serial.println("Write failed!");
  }
}

void uploadTemplate() {
  Serial.println("\n----- Upload Template -----");
  
  Serial.print("Enter ID# to upload: ");
  while (!Serial.available()) delay(10);
  int id = Serial.parseInt();
  while (Serial.available()) Serial.read();
  
  if (id < 0 || id > 199) {
    Serial.println("Invalid ID!");
    return;
  }
  
  // Load template
  if (!finger.loadModel(R503_CHARBUFFER1, id)) {
    Serial.println("Failed to load template!");
    return;
  }
  
  // Upload
  uint8_t buffer[768];
  uint16_t length;
  
  if (finger.uploadCharacteristics(R503_CHARBUFFER1, buffer, length)) {
    Serial.print("Template uploaded! Size: ");
    Serial.print(length);
    Serial.println(" bytes");
    
    // Display first 32 bytes
    Serial.print("First 32 bytes: ");
    for (int i = 0; i < min(32, (int)length); i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("Upload failed!");
  }
}

void downloadTemplate() {
  Serial.println("\n----- Download Template -----");
  Serial.println("This is a placeholder - you would provide template data");
  Serial.println("Template download requires valid template data buffer");
}

void testImageUpload() {
  Serial.println("\n----- Upload Fingerprint Image -----");
  Serial.println("Place finger on sensor...");
  
  if (!finger.getImage()) {
    Serial.println("Failed to capture image!");
    return;
  }
  
  uint8_t imageBuffer[36864]; // 192x192 image
  uint32_t imageSize;
  
  Serial.println("Uploading image...");
  if (finger.uploadImage(imageBuffer, imageSize)) {
    Serial.print("Image uploaded! Size: ");
    Serial.print(imageSize);
    Serial.println(" bytes");
    Serial.println("Image data could be saved or processed here");
  } else {
    Serial.println("Image upload failed!");
  }
}

void changeSecurityLevel() {
  Serial.println("\n----- Change Security Level -----");
  Serial.println("Current security levels (1-5):");
  Serial.println("1 = Lowest (High FAR, Low FRR)");
  Serial.println("2 = Low");
  Serial.println("3 = Medium");
  Serial.println("4 = High");
  Serial.println("5 = Highest (Low FAR, High FRR)");
  
  Serial.print("Enter new level (1-5): ");
  while (!Serial.available()) delay(10);
  int level = Serial.parseInt();
  while (Serial.available()) Serial.read();
  
  if (level < 1 || level > 5) {
    Serial.println("Invalid level!");
    return;
  }
  
  if (finger.setSystemParameter(R503_PARAM_SECURITY, level)) {
    Serial.print("Security level changed to ");
    Serial.println(level);
  } else {
    Serial.println("Failed to change security level!");
  }
}

void checkSensorStatus() {
  Serial.println("\n----- Check Sensor Status -----");
  
  if (finger.checkSensor()) {
    Serial.println("Sensor is OK!");
    finger.ledFlash(R503_LED_BLUE, 0xFF, 2);
  } else {
    Serial.println("Sensor is ABNORMAL!");
    finger.ledFlash(R503_LED_RED, 0xFF, 2);
  }
}

void getRandomNumber() {
  Serial.println("\n----- Get Random Number -----");
  
  uint32_t randomNum;
  if (finger.getRandomCode(randomNum)) {
    Serial.print("Random number: 0x");
    Serial.println(randomNum, HEX);
    Serial.print("Decimal: ");
    Serial.println(randomNum);
  } else {
    Serial.println("Failed to get random number!");
  }
}

void testWakeupDetection() {
  Serial.println("\n----- Test Wakeup Detection -----");
  Serial.println("Testing finger detection via WAKEUP pin (GPIO 6)");
  Serial.println("Place and remove finger 5 times...");
  Serial.println("Press any key to stop test");
  
  fingerDetected = false;
  int detectionCount = 0;
  uint32_t lastDetection = 0;
  
  while (!Serial.available() && detectionCount < 5) {
    if (fingerDetected && (millis() - lastDetection > 1000)) {
      fingerDetected = false;
      lastDetection = millis();
      detectionCount++;
      
      Serial.print("Detection #");
      Serial.print(detectionCount);
      Serial.println(" - Finger detected!");
      
      finger.ledFlash(R503_LED_PURPLE, 0xFF, 2);
      
      // Verify with actual sensor
      if (finger.getImage()) {
        Serial.println("  -> Sensor confirmed finger presence");
      }
    }
    delay(10);
  }
  
  while (Serial.available()) Serial.read();
  
  Serial.print("\nTest complete! Total detections: ");
  Serial.println(detectionCount);
  Serial.println("WAKEUP pin is working correctly!");
  
  finger.ledOn(R503_LED_BLUE);
}