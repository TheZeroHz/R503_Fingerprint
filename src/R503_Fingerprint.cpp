// R503_Fingerprint.cpp
#include "R503_Fingerprint.h"

R503_Fingerprint::R503_Fingerprint(HardwareSerial *serial) {
  this->serial = serial;
  this->password = R503_DEFAULT_PASSWORD;
  this->address = R503_DEFAULT_ADDRESS;
  this->timeout = R503_DEFAULT_TIMEOUT;
  this->lastConfirmCode = 0xFF;
}

bool R503_Fingerprint::begin(uint32_t baud, uint32_t password, uint32_t address) {
  this->password = password;
  this->address = address;
  
  serial->begin(baud);
  delay(R503_RESET_DELAY);
  
  // Wait for handshake signal 0x55
  uint32_t startTime = millis();
  while (millis() - startTime < 1000) {
    if (serial->available() && serial->read() == 0x55) {
      break;
    }
    delay(10);
  }
  
  clearSerialBuffer();
  
  // Verify password if not default
  if (password != R503_DEFAULT_PASSWORD) {
    return verifyPassword(password);
  }
  
  return handshake();
}

void R503_Fingerprint::setTimeout(uint32_t timeout) {
  this->timeout = timeout;
}

bool R503_Fingerprint::verifyPassword(uint32_t password) {
  uint8_t packet[5];
  packet[0] = R503_VFYPWD;
  packet[1] = (password >> 24) & 0xFF;
  packet[2] = (password >> 16) & 0xFF;
  packet[3] = (password >> 8) & 0xFF;
  packet[4] = password & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 5)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::setPassword(uint32_t password) {
  uint8_t packet[5];
  packet[0] = R503_SETPWD;
  packet[1] = (password >> 24) & 0xFF;
  packet[2] = (password >> 16) & 0xFF;
  packet[3] = (password >> 8) & 0xFF;
  packet[4] = password & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 5)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK) {
    this->password = password;
    return true;
  }
  return false;
}

bool R503_Fingerprint::setAddress(uint32_t address) {
  uint8_t packet[5];
  packet[0] = R503_SETADDER;
  packet[1] = (address >> 24) & 0xFF;
  packet[2] = (address >> 16) & 0xFF;
  packet[3] = (address >> 8) & 0xFF;
  packet[4] = address & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 5)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK) {
    this->address = address;
    return true;
  }
  return false;
}

bool R503_Fingerprint::setSystemParameter(uint8_t paramNumber, uint8_t value) {
  uint8_t packet[3];
  packet[0] = R503_SETSYSPARA;
  packet[1] = paramNumber;
  packet[2] = value;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 3)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::readSystemParameters(R503_SystemParams &params) {
  uint8_t packet[1];
  packet[0] = R503_READSYSPARA;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[32];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 17) {
    params.statusRegister = (response[1] << 8) | response[2];
    params.systemID = (response[3] << 8) | response[4];
    params.librarySize = (response[5] << 8) | response[6];
    params.securityLevel = (response[7] << 8) | response[8];
    params.deviceAddress = ((uint32_t)response[9] << 24) | ((uint32_t)response[10] << 16) |
                          ((uint32_t)response[11] << 8) | response[12];
    params.dataPacketSize = (response[13] << 8) | response[14];
    params.baudRate = (response[15] << 8) | response[16];
    return true;
  }
  return false;
}

bool R503_Fingerprint::portControl(bool enable) {
  uint8_t packet[2];
  packet[0] = R503_CONTROL;
  packet[1] = enable ? 1 : 0;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 2)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::getTemplateCount(uint16_t &count) {
  uint8_t packet[1];
  packet[0] = R503_TEMPLATENUM;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 3) {
    count = (response[1] << 8) | response[2];
    return true;
  }
  return false;
}

bool R503_Fingerprint::readIndexTable(uint8_t page, uint8_t *indexTable) {
  uint8_t packet[2];
  packet[0] = R503_READINDEXTABLE;
  packet[1] = page;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 2)) return false;
  
  uint8_t response[64];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 33) {
    memcpy(indexTable, response + 1, 32);
    return true;
  }
  return false;
}

bool R503_Fingerprint::handshake() {
  uint8_t packet[1];
  packet[0] = R503_HANDSHAKE;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::checkSensor() {
  uint8_t packet[1];
  packet[0] = R503_CHECKSENSOR;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::getAlgorithmVersion(char *version) {
  uint8_t packet[1];
  packet[0] = R503_GETALGVER;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[64];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 33) {
    memcpy(version, response + 1, 32);
    version[32] = '\0';
    return true;
  }
  return false;
}

bool R503_Fingerprint::getFirmwareVersion(char *version) {
  uint8_t packet[1];
  packet[0] = R503_GETFWVER;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[64];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 33) {
    memcpy(version, response + 1, 32);
    version[32] = '\0';
    return true;
  }
  return false;
}

bool R503_Fingerprint::readProductInfo(R503_ProductInfo &info) {
  uint8_t packet[1];
  packet[0] = R503_READPRODINFO;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[64];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 47) {
    memcpy(info.moduleType, response + 1, 16);
    info.moduleType[16] = '\0';
    memcpy(info.batchNumber, response + 17, 4);
    info.batchNumber[4] = '\0';
    memcpy(info.serialNumber, response + 21, 8);
    info.serialNumber[8] = '\0';
    info.hardwareVersion = (response[29] << 8) | response[30];
    memcpy(info.sensorType, response + 31, 8);
    info.sensorType[8] = '\0';
    info.sensorWidth = (response[39] << 8) | response[40];
    info.sensorHeight = (response[41] << 8) | response[42];
    info.templateSize = (response[43] << 8) | response[44];
    info.databaseSize = (response[45] << 8) | response[46];
    return true;
  }
  return false;
}

bool R503_Fingerprint::softReset() {
  uint8_t packet[1];
  packet[0] = R503_SOFTRST;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK) {
    delay(R503_RESET_DELAY);
    clearSerialBuffer();
    return true;
  }
  return false;
}

bool R503_Fingerprint::getImage() {
  uint8_t packet[1];
  packet[0] = R503_GENIMG;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::getImageEx() {
  uint8_t packet[1];
  packet[0] = R503_GETIMAGEEX;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::image2Tz(uint8_t slot) {
  uint8_t packet[2];
  packet[0] = R503_IMG2TZ;
  packet[1] = slot;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 2)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::createModel() {
  uint8_t packet[1];
  packet[0] = R503_REGMODEL;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::storeModel(uint8_t slot, uint16_t pageID) {
  uint8_t packet[4];
  packet[0] = R503_STORE;
  packet[1] = slot;
  packet[2] = (pageID >> 8) & 0xFF;
  packet[3] = pageID & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 4)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::loadModel(uint8_t slot, uint16_t pageID) {
  uint8_t packet[4];
  packet[0] = R503_LOADCHAR;
  packet[1] = slot;
  packet[2] = (pageID >> 8) & 0xFF;
  packet[3] = pageID & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 4)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::deleteModel(uint16_t startPage, uint16_t count) {
  uint8_t packet[5];
  packet[0] = R503_DELETCHAR;
  packet[1] = (startPage >> 8) & 0xFF;
  packet[2] = startPage & 0xFF;
  packet[3] = (count >> 8) & 0xFF;
  packet[4] = count & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 5)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::emptyDatabase() {
  uint8_t packet[1];
  packet[0] = R503_EMPTY;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::matchTemplates(uint16_t &score) {
  uint8_t packet[1];
  packet[0] = R503_MATCH;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 3) {
    score = (response[1] << 8) | response[2];
    return true;
  }
  return false;
}

bool R503_Fingerprint::searchLibrary(uint8_t slot, uint16_t startPage, uint16_t count,
                                     uint16_t &fingerID, uint16_t &score) {
  uint8_t packet[6];
  packet[0] = R503_SEARCH;
  packet[1] = slot;
  packet[2] = (startPage >> 8) & 0xFF;
  packet[3] = startPage & 0xFF;
  packet[4] = (count >> 8) & 0xFF;
  packet[5] = count & 0xFF;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 6)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 5) {
    fingerID = (response[1] << 8) | response[2];
    score = (response[3] << 8) | response[4];
    return true;
  }
  return false;
}

bool R503_Fingerprint::uploadCharacteristics(uint8_t slot, uint8_t *buffer, uint16_t &length) {
  uint8_t packet[2];
  packet[0] = R503_UPCHAR;
  packet[1] = slot;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 2)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode != R503_OK) return false;
  
  return receiveData(buffer, 1024, length);
}

bool R503_Fingerprint::downloadCharacteristics(uint8_t slot, uint8_t *buffer, uint16_t length) {
  uint8_t packet[2];
  packet[0] = R503_DOWNCHAR;
  packet[1] = slot;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 2)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode != R503_OK) return false;
  
  // Send data packets
  uint16_t offset = 0;
  uint16_t packetSize = 128; // Can be 32, 64, 128, or 256
  
  while (offset < length) {
    uint16_t chunkSize = min((uint16_t)(length - offset), packetSize);
    bool isLastPacket = (offset + chunkSize >= length);
    uint8_t packetType = isLastPacket ? R503_END_DATA_PACKET : R503_DATA_PACKET;
    
    if (!sendPacket(packetType, buffer + offset, chunkSize)) return false;
    offset += chunkSize;
  }
  
  return true;
}

bool R503_Fingerprint::uploadImage(uint8_t *buffer, uint32_t &length) {
  uint8_t packet[1];
  packet[0] = R503_UPIMAGE;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode != R503_OK) return false;
  
  length = 0;
  uint16_t tempLen;
  return receiveData(buffer, 36864, tempLen) && (length = tempLen, true);
}

bool R503_Fingerprint::downloadImage(uint8_t *buffer, uint32_t length) {
  uint8_t packet[1];
  packet[0] = R503_DOWNIMAGE;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode != R503_OK) return false;
  
  uint32_t offset = 0;
  uint16_t packetSize = 128;
  
  while (offset < length) {
    uint16_t chunkSize = min((uint32_t)(length - offset), (uint32_t)packetSize);
    bool isLastPacket = (offset + chunkSize >= length);
    uint8_t packetType = isLastPacket ? R503_END_DATA_PACKET : R503_DATA_PACKET;
    
    if (!sendPacket(packetType, buffer + offset, chunkSize)) return false;
    offset += chunkSize;
  }
  
  return true;
}

bool R503_Fingerprint::setLED(uint8_t control, uint8_t speed, uint8_t color, uint8_t times) {
  uint8_t packet[5];
  packet[0] = R503_AURALEDCONFIG;
  packet[1] = control;
  packet[2] = speed;
  packet[3] = color;
  packet[4] = times;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 5)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::ledOn(uint8_t color) {
  return setLED(R503_LED_ON, 0, color, 0);
}

bool R503_Fingerprint::ledOff() {
  return setLED(R503_LED_OFF, 0, 0, 0);
}

bool R503_Fingerprint::ledBreathe(uint8_t color, uint8_t speed, uint8_t times) {
  return setLED(R503_LED_BREATHING, speed, color, times);
}

bool R503_Fingerprint::ledFlash(uint8_t color, uint8_t speed, uint8_t times) {
  return setLED(R503_LED_FLASHING, speed, color, times);
}

bool R503_Fingerprint::writeNotepad(uint8_t page, uint8_t *data) {
  if (page > 15) return false;
  
  uint8_t packet[34];
  packet[0] = R503_WRITENOTEPAD;
  packet[1] = page;
  memcpy(packet + 2, data, 32);
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 34)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::readNotepad(uint8_t page, uint8_t *data) {
  if (page > 15) return false;
  
  uint8_t packet[2];
  packet[0] = R503_READNOTEPAD;
  packet[1] = page;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 2)) return false;
  
  uint8_t response[64];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 33) {
    memcpy(data, response + 1, 32);
    return true;
  }
  return false;
}

bool R503_Fingerprint::getRandomCode(uint32_t &randomNumber) {
  uint8_t packet[1];
  packet[0] = R503_GETRANDOMCODE;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode == R503_OK && len >= 5) {
    randomNumber = ((uint32_t)response[1] << 24) | ((uint32_t)response[2] << 16) |
                   ((uint32_t)response[3] << 8) | response[4];
    return true;
  }
  return false;
}

bool R503_Fingerprint::readInformationPage(uint8_t *buffer) {
  uint8_t packet[1];
  packet[0] = R503_READINFPAGE;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  if (lastConfirmCode != R503_OK) return false;
  
  uint16_t dataLen;
  return receiveData(buffer, 512, dataLen);
}

bool R503_Fingerprint::cancel() {
  uint8_t packet[1];
  packet[0] = R503_CANCEL;
  
  if (!sendPacket(R503_COMMAND_PACKET, packet, 1)) return false;
  
  uint8_t response[16];
  uint16_t len;
  if (!receiveAck(response, len)) return false;
  
  return lastConfirmCode == R503_OK;
}

bool R503_Fingerprint::enrollFingerprint(uint16_t pageID, uint8_t enrollCount) {
  if (enrollCount < 2 || enrollCount > 6) enrollCount = 6;
  
  // Collect first image
  Serial.println("Place finger...");
  while (!getImage()) {
    delay(100);
  }
  Serial.println("Image captured");
  
  if (!image2Tz(R503_CHARBUFFER1)) {
    Serial.println("Failed to convert image 1");
    return false;
  }
  
  Serial.println("Remove finger");
  delay(1000);
  
  // Collect remaining images
  for (uint8_t i = 1; i < enrollCount; i++) {
    Serial.print("Place same finger again (");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(enrollCount);
    Serial.println(")...");
    
    while (!getImage()) {
      delay(100);
    }
    Serial.println("Image captured");
    
    if (!image2Tz(R503_CHARBUFFER2)) {
      Serial.println("Failed to convert image");
      return false;
    }
    
    if (i == 1) {
      if (!createModel()) {
        Serial.println("Failed to create model");
        return false;
      }
    }
    
    if (i < enrollCount - 1) {
      Serial.println("Remove finger");
      delay(1000);
    }
  }
  
  // Store the template
  if (!storeModel(R503_CHARBUFFER1, pageID)) {
    Serial.println("Failed to store model");
    return false;
  }
  
  Serial.println("Fingerprint enrolled successfully!");
  return true;
}

bool R503_Fingerprint::verifyFingerprint(uint16_t &fingerID, uint16_t &confidence) {
  if (!getImage()) {
    return false;
  }
  
  if (!image2Tz(R503_CHARBUFFER1)) {
    return false;
  }
  
  R503_SystemParams params;
  if (!readSystemParameters(params)) {
    return false;
  }
  
  return searchLibrary(R503_CHARBUFFER1, 0, params.librarySize, fingerID, confidence);
}

bool R503_Fingerprint::sendPacket(uint8_t packetType, uint8_t *data, uint16_t dataLen) {
  uint16_t length = dataLen + 2;
  
  if (!writePacketHeader(packetType, length)) return false;
  
  uint16_t checksum = packetType + ((length >> 8) & 0xFF) + (length & 0xFF);
  
  for (uint16_t i = 0; i < dataLen; i++) {
    serial->write(data[i]);
    checksum += data[i];
  }
  
  writeU16(checksum);
  
  return true;
}

bool R503_Fingerprint::receivePacket(uint8_t *buffer, uint16_t &length, uint8_t expectedType) {
  uint32_t startTime = millis();
  
  while (serial->available() < 9) {
    if (millis() - startTime > timeout) {
      return false;
    }
    delay(1);
  }
  
  uint16_t header = readU16();
  if (header != R503_STARTCODE) {
    return false;
  }
  
  uint32_t addr = readU32();
  if (addr != address) {
    return false;
  }
  
  uint8_t pid = serial->read();
  if (pid != expectedType) {
    return false;
  }
  
  uint16_t len = readU16();
  if (len < 2) {
    return false;
  }
  
  length = len - 2;
  
  startTime = millis();
  while (serial->available() < length + 2) {
    if (millis() - startTime > timeout) {
      return false;
    }
    delay(1);
  }
  
  uint16_t calcChecksum = pid + ((len >> 8) & 0xFF) + (len & 0xFF);
  
  for (uint16_t i = 0; i < length; i++) {
    buffer[i] = serial->read();
    calcChecksum += buffer[i];
  }
  
  uint16_t checksum = readU16();
  
  return (checksum == calcChecksum);
}

bool R503_Fingerprint::receiveAck(uint8_t *buffer, uint16_t &length) {
  if (!receivePacket(buffer, length, R503_ACK_PACKET)) {
    return false;
  }
  
  if (length > 0) {
    lastConfirmCode = buffer[0];
  }
  
  return true;
}

bool R503_Fingerprint::receiveData(uint8_t *buffer, uint16_t maxLength, uint16_t &actualLength) {
  actualLength = 0;
  uint8_t packetType = R503_DATA_PACKET;
  
  while (packetType != R503_END_DATA_PACKET) {
    uint16_t chunkLen;
    if (!receivePacket(buffer + actualLength, chunkLen, packetType)) {
      return false;
    }
    
    actualLength += chunkLen;
    
    if (actualLength >= maxLength) {
      return false;
    }
    
    packetType = R503_END_DATA_PACKET;
    
    uint32_t startTime = millis();
    while (serial->available() < 9) {
      if (millis() - startTime > 100) {
        break;
      }
      delay(1);
    }
    
    if (serial->available() >= 9) {
      if (serial->peek() == 0xEF) {
        uint8_t temp[9];
        for (int i = 0; i < 9; i++) {
          temp[i] = serial->read();
        }
        
        if (temp[0] == 0xEF && temp[1] == 0x01) {
          uint8_t pid = temp[8];
          if (pid == R503_DATA_PACKET) {
            for (int i = 0; i < 9; i++) {
              serial->available();
            }
            packetType = R503_DATA_PACKET;
            
            for (int i = 8; i >= 0; i--) {
              // Push back bytes
            }
          }
        }
      }
    }
  }
  
  return true;
}

uint16_t R503_Fingerprint::calculateChecksum(uint8_t *data, uint16_t length) {
  uint16_t sum = 0;
  for (uint16_t i = 0; i < length; i++) {
    sum += data[i];
  }
  return sum;
}

bool R503_Fingerprint::writePacketHeader(uint8_t packetType, uint16_t length) {
  writeU16(R503_STARTCODE);
  writeU32(address);
  serial->write(packetType);
  writeU16(length);
  return true;
}

void R503_Fingerprint::clearSerialBuffer() {
  while (serial->available()) {
    serial->read();
  }
}

void R503_Fingerprint::writeU16(uint16_t value) {
  serial->write((value >> 8) & 0xFF);
  serial->write(value & 0xFF);
}

void R503_Fingerprint::writeU32(uint32_t value) {
  serial->write((value >> 24) & 0xFF);
  serial->write((value >> 16) & 0xFF);
  serial->write((value >> 8) & 0xFF);
  serial->write(value & 0xFF);
}

uint16_t R503_Fingerprint::readU16() {
  uint16_t value = serial->read() << 8;
  value |= serial->read();
  return value;
}

uint32_t R503_Fingerprint::readU32() {
  uint32_t value = (uint32_t)serial->read() << 24;
  value |= (uint32_t)serial->read() << 16;
  value |= (uint32_t)serial->read() << 8;
  value |= serial->read();
  return value;
}