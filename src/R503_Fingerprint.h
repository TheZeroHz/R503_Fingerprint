// R503_Fingerprint.h
#ifndef R503_FINGERPRINT_H
#define R503_FINGERPRINT_H

#include <Arduino.h>
#include <HardwareSerial.h>

// Package identifiers
#define R503_COMMAND_PACKET 0x01
#define R503_DATA_PACKET 0x02
#define R503_ACK_PACKET 0x07
#define R503_END_DATA_PACKET 0x08

// Instruction codes
#define R503_GENIMG 0x01
#define R503_IMG2TZ 0x02
#define R503_MATCH 0x03
#define R503_SEARCH 0x04
#define R503_REGMODEL 0x05
#define R503_STORE 0x06
#define R503_LOADCHAR 0x07
#define R503_UPCHAR 0x08
#define R503_DOWNCHAR 0x09
#define R503_UPIMAGE 0x0A
#define R503_DOWNIMAGE 0x0B
#define R503_DELETCHAR 0x0C
#define R503_EMPTY 0x0D
#define R503_SETSYSPARA 0x0E
#define R503_READSYSPARA 0x0F
#define R503_SETPWD 0x12
#define R503_VFYPWD 0x13
#define R503_GETRANDOMCODE 0x14
#define R503_SETADDER 0x15
#define R503_READINFPAGE 0x16
#define R503_CONTROL 0x17
#define R503_WRITENOTEPAD 0x18
#define R503_READNOTEPAD 0x19
#define R503_TEMPLATENUM 0x1D
#define R503_READINDEXTABLE 0x1F
#define R503_GETIMAGEEX 0x28
#define R503_CANCEL 0x30
#define R503_AURALEDCONFIG 0x35
#define R503_CHECKSENSOR 0x36
#define R503_GETALGVER 0x39
#define R503_GETFWVER 0x3A
#define R503_READPRODINFO 0x3C
#define R503_SOFTRST 0x3D
#define R503_HANDSHAKE 0x40

// Confirmation codes
#define R503_OK 0x00
#define R503_PACKETRECIEVEERR 0x01
#define R503_NOFINGER 0x02
#define R503_IMAGEFAIL 0x03
#define R503_IMAGEMESS 0x06
#define R503_FEATUREFAIL 0x07
#define R503_NOMATCH 0x08
#define R503_NOTFOUND 0x09
#define R503_ENROLLMISMATCH 0x0A
#define R503_BADLOCATION 0x0B
#define R503_DBRANGEFAIL 0x0C
#define R503_UPLOADFEATUREFAIL 0x0D
#define R503_PACKETRESPONSEFAIL 0x0E
#define R503_UPLOADFAIL 0x0F
#define R503_DELETEFAIL 0x10
#define R503_DBCLEARFAIL 0x11
#define R503_WRONGPASSWORD 0x13
#define R503_INVALIDIMAGE 0x15
#define R503_FLASHERR 0x18
#define R503_UNDEFINEDERROR 0x19
#define R503_INVALIDREG 0x1A
#define R503_REGCONFFAIL 0x1B
#define R503_WRONGNOTEPAGE 0x1C
#define R503_PORTOPFAIL 0x1D
#define R503_IMAGEQUALITY 0x07
#define R503_ABNORMALSENSOR 0x29

// Buffer IDs
#define R503_CHARBUFFER1 0x01
#define R503_CHARBUFFER2 0x02

// LED Control codes
#define R503_LED_BREATHING 0x01
#define R503_LED_FLASHING 0x02
#define R503_LED_ON 0x03
#define R503_LED_OFF 0x04
#define R503_LED_GRADUAL_ON 0x05
#define R503_LED_GRADUAL_OFF 0x06

// LED Colors
#define R503_LED_RED 0x01
#define R503_LED_BLUE 0x02
#define R503_LED_PURPLE 0x03

// System parameters
#define R503_PARAM_BAUD 4
#define R503_PARAM_SECURITY 5
#define R503_PARAM_PACKAGE_SIZE 6

// Default values
#define R503_DEFAULT_PASSWORD 0x00000000
#define R503_DEFAULT_ADDRESS 0xFFFFFFFF
#define R503_STARTCODE 0xEF01
#define R503_DEFAULT_TIMEOUT 2000
#define R503_RESET_DELAY 200

// Package size options
#define R503_PACKAGE_SIZE_32 0
#define R503_PACKAGE_SIZE_64 1
#define R503_PACKAGE_SIZE_128 2
#define R503_PACKAGE_SIZE_256 3

// System status register bits
#define R503_STATUS_BUSY 0x01
#define R503_STATUS_PASS 0x02
#define R503_STATUS_PWD 0x04
#define R503_STATUS_IMGBUF 0x08

// Structure for system parameters
struct R503_SystemParams {
  uint16_t statusRegister;
  uint16_t systemID;
  uint16_t librarySize;
  uint16_t securityLevel;
  uint32_t deviceAddress;
  uint16_t dataPacketSize;
  uint16_t baudRate;
};

// Structure for product information
struct R503_ProductInfo {
  char moduleType[17];
  char batchNumber[5];
  char serialNumber[9];
  uint16_t hardwareVersion;
  char sensorType[9];
  uint16_t sensorWidth;
  uint16_t sensorHeight;
  uint16_t templateSize;
  uint16_t databaseSize;
};

class R503_Fingerprint {
public:
  R503_Fingerprint(HardwareSerial *serial);
  
  // Initialization
  bool begin(uint32_t baud = 57600, uint32_t password = R503_DEFAULT_PASSWORD, 
             uint32_t address = R503_DEFAULT_ADDRESS);
  void setTimeout(uint32_t timeout);
  
  // System commands
  bool verifyPassword(uint32_t password = R503_DEFAULT_PASSWORD);
  bool setPassword(uint32_t password);
  bool setAddress(uint32_t address);
  bool setSystemParameter(uint8_t paramNumber, uint8_t value);
  bool readSystemParameters(R503_SystemParams &params);
  bool portControl(bool enable);
  bool getTemplateCount(uint16_t &count);
  bool readIndexTable(uint8_t page, uint8_t *indexTable);
  bool handshake();
  bool checkSensor();
  bool getAlgorithmVersion(char *version);
  bool getFirmwareVersion(char *version);
  bool readProductInfo(R503_ProductInfo &info);
  bool softReset();
  
  // Fingerprint processing
  bool getImage();
  bool getImageEx();
  bool image2Tz(uint8_t slot = R503_CHARBUFFER1);
  bool createModel();
  bool storeModel(uint8_t slot, uint16_t pageID);
  bool loadModel(uint8_t slot, uint16_t pageID);
  bool deleteModel(uint16_t startPage, uint16_t count = 1);
  bool emptyDatabase();
  bool matchTemplates(uint16_t &score);
  bool searchLibrary(uint8_t slot, uint16_t startPage, uint16_t count, 
                     uint16_t &fingerID, uint16_t &score);
  
  // Template transfer
  bool uploadCharacteristics(uint8_t slot, uint8_t *buffer, uint16_t &length);
  bool downloadCharacteristics(uint8_t slot, uint8_t *buffer, uint16_t length);
  bool uploadImage(uint8_t *buffer, uint32_t &length);
  bool downloadImage(uint8_t *buffer, uint32_t length);
  
  // LED control
  bool setLED(uint8_t control, uint8_t speed, uint8_t color, uint8_t times);
  bool ledOn(uint8_t color = R503_LED_BLUE);
  bool ledOff();
  bool ledBreathe(uint8_t color = R503_LED_BLUE, uint8_t speed = 0x80, uint8_t times = 0);
  bool ledFlash(uint8_t color = R503_LED_BLUE, uint8_t speed = 0x80, uint8_t times = 5);
  
  // Notepad operations
  bool writeNotepad(uint8_t page, uint8_t *data);
  bool readNotepad(uint8_t page, uint8_t *data);
  
  // Random number
  bool getRandomCode(uint32_t &randomNumber);
  
  // Information page
  bool readInformationPage(uint8_t *buffer);
  
  // Cancel operation
  bool cancel();
  
  // Helper enrollment functions
  bool enrollFingerprint(uint16_t pageID, uint8_t enrollCount = 6);
  bool verifyFingerprint(uint16_t &fingerID, uint16_t &confidence);
  
  // Getters
  uint8_t getLastConfirmationCode() { return lastConfirmCode; }
  uint32_t getPassword() { return password; }
  uint32_t getAddress() { return address; }
  
private:
  HardwareSerial *serial;
  uint32_t password;
  uint32_t address;
  uint32_t timeout;
  uint8_t lastConfirmCode;
  
  // Packet handling
  bool sendPacket(uint8_t packetType, uint8_t *data, uint16_t dataLen);
  bool receivePacket(uint8_t *buffer, uint16_t &length, uint8_t expectedType = R503_ACK_PACKET);
  bool receiveAck(uint8_t *buffer, uint16_t &length);
  bool receiveData(uint8_t *buffer, uint16_t maxLength, uint16_t &actualLength);
  uint16_t calculateChecksum(uint8_t *data, uint16_t length);
  bool writePacketHeader(uint8_t packetType, uint16_t length);
  void clearSerialBuffer();
  
  // Utility
  void writeU16(uint16_t value);
  void writeU32(uint32_t value);
  uint16_t readU16();
  uint32_t readU32();
};

#endif // R503_FINGERPRINT_H
