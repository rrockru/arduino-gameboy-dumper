// Data pins (D0-D7)
#define DATA_PIN_0 40
#define DATA_PIN_1 43
#define DATA_PIN_2 42
#define DATA_PIN_3 45
#define DATA_PIN_4 44
#define DATA_PIN_5 47
#define DATA_PIN_6 46
#define DATA_PIN_7 49

// Address pins (A0-A15)
#define ADDR_PIN_0 24
#define ADDR_PIN_1 27
#define ADDR_PIN_2 26
#define ADDR_PIN_3 29
#define ADDR_PIN_4 28
#define ADDR_PIN_5 31
#define ADDR_PIN_6 30
#define ADDR_PIN_7 33
#define ADDR_PIN_8 32
#define ADDR_PIN_9 35
#define ADDR_PIN_10 34
#define ADDR_PIN_11 37
#define ADDR_PIN_12 36
#define ADDR_PIN_13 39
#define ADDR_PIN_14 38
#define ADDR_PIN_15 41

// Management pins
#define WR_PIN 23
#define RD_PIN 22
#define CS_PIN 25
#define CLK_PIN 50
#define RST_PIN 48

const int dataPins[] = {DATA_PIN_0, DATA_PIN_1, DATA_PIN_2, DATA_PIN_3,
                        DATA_PIN_4, DATA_PIN_5, DATA_PIN_6, DATA_PIN_7};

const int addrPins[] = {ADDR_PIN_0, ADDR_PIN_1, ADDR_PIN_2, ADDR_PIN_3,
                        ADDR_PIN_4, ADDR_PIN_5, ADDR_PIN_6, ADDR_PIN_7,
                        ADDR_PIN_8, ADDR_PIN_9, ADDR_PIN_10, ADDR_PIN_11,
                        ADDR_PIN_12, ADDR_PIN_13, ADDR_PIN_14, ADDR_PIN_15};

String inputBuffer = "";

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // Init management pins
  pinMode(RD_PIN, OUTPUT);
  pinMode(WR_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);

  // Initial state (for inverted signals)
  digitalWrite(RD_PIN, HIGH);
  digitalWrite(WR_PIN, HIGH);
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(RST_PIN, HIGH);

  // Init address pins as output
  for (int i = 0; i < 16; i++) {
    pinMode(addrPins[i], OUTPUT);
    digitalWrite(addrPins[i], LOW);
  }

  // Init data pins as input by default
  setDataBusInput();

  // Reset cartridge
  resetCartridge();

  Serial.println("Game Boy Cartridge Reader Ready");
  printHelp();
  Serial.print("> ");
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        Serial.println();
        processCommand(inputBuffer);
        inputBuffer = "";
        Serial.print("> ");
      }
    } else {
      inputBuffer += c;
      Serial.print(c);
    }
  }
}

void processCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  if (cmd.startsWith("readbyte ")) {
    cmdReadByte(cmd.substring(9));
  }
  else if (cmd.startsWith("writebyte ")) {
    cmdWriteByte(cmd.substring(10));
  }
  else if (cmd.startsWith("readrange ")) {
    cmdReadRange(cmd.substring(10));
  }
  else if (cmd.startsWith("writeblock ")) {
    cmdWriteBlock(cmd.substring(11));
  }
  else if (cmd.startsWith("dump ")) {
    cmdDump(cmd.substring(5));
  }
  else if (cmd.startsWith("switchrombank ")) {
    cmdSwitchROMBank(cmd.substring(14));
  }
  else if (cmd.startsWith("switchrambank ")) {
    cmdSwitchRAMBank(cmd.substring(14));
  }  
  else if (cmd == "readheader") {
    cmdReadHeader();
  }
  else if (cmd == "readrom") {
    cmdReadROM();
  }
  else if (cmd == "readram") {
    cmdReadRAM();
  }
  else if (cmd == "enableram") {
    enableRAM();
  }
  else if (cmd == "disableram") {
    disableRAM();
  }
  else if (cmd == "test") {
    cmdTestReadWrite();
  }
  else if (cmd == "reset") {
    resetCartridge();
    Serial.println("Done\n");
  }
  else if (cmd == "help" || cmd == "?") {
    printHelp();
  }
  else {
    Serial.println("Unknown command!");
  }
}

void printHelp() {
  Serial.println("\n=== Game Boy Cartridge Commands ===");

  Serial.println("\n--- Single Byte Operations ---");
  Serial.println("readByte <addr>           - Read byte at address");
  Serial.println("  Example: read 0x0100");
  Serial.println("writeByte <addr> <data>   - Write byte to address");
  Serial.println("  Example: write 0xA000 0x42");

  Serial.println("\n--- Range Operations ---");
  Serial.println("readRange <start> <end>   - Read range of addresses");
  Serial.println("  Example: readRange 0x0000 0x00FF");
  Serial.println("writeBlock                - Write block of data");
  Serial.println("  Example: writeBlock 0xA000 0x0100");
  Serial.println("  (Then send binary (data) in hex)");
  Serial.println("dump <addr> <len>         - Hex dump of memory");
  Serial.println("  Example: dump 0x0100, 0x80");

  Serial.println("\n--- Bank Operations ---");
  Serial.println("switchROMBank <bank>      - Switch ROM bank");
  Serial.println("  Example: switchROMBank 0x01");
  Serial.println("switchRAMBank <bank>      - Switch RAM bank");
  Serial.println("  Example: switchRAMBank 0x01");
  Serial.println("enableRAM                 - Enable cartridge RAM");
  Serial.println("disableRAM                - Disable cartridge RAM");

  Serial.println("\n--- Bulk Operations ---");
  Serial.println("readHeader                - Read cartridge header");
  Serial.println("readROM                   - Read entire ROM (32KB)");
  Serial.println("readRAM                   - Read entire RAM");

  Serial.println("\n--- Other ---");
  Serial.println("test                      - Test read/write functionality");
  Serial.println("reset                     - Reset cartridge");
  Serial.println("help or ?                 - Show this help");
  Serial.println("==========================================\n");
}

/*
--- Low-level operations
*/
// Set data pins as input
void setDataBusInput() {
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], INPUT);
  }
}

// Set data pins as output
void setDataBusOutput() {
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], OUTPUT);
  }
}

// Write address to address pins
void setAddress(uint16_t addr) {
  for (int i = 0; i < 16; i++) {
    digitalWrite(addrPins[i], (addr >> i) & 0x01);
  }
}

// Write data to data pins
void setData(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(dataPins[i], (data >> i) & 0x01);
  }
}

// Read data from data pins
uint8_t getData() {
  uint8_t data = 0;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(dataPins[i])) {
      data |= (1 << i);
    }    
  }
  return data;
}

// Enable cartridge RAM
void enableRAM() {
  writeByte(0x0000, 0x0A);
  delay(10);
  Serial.println("RAM enabled");
}

// Disable cartridge RAM
void disableRAM() {
  writeByte(0x0000, 0x00);
  delay(10);
  Serial.println("RAM disabled");
}

// Reset cartridge
void resetCartridge() {
  digitalWrite(RST_PIN, LOW);
  delay(10);
  digitalWrite(RST_PIN, HIGH);
  delay(100);
}

// Read byte at address operation
uint8_t readByte(uint16_t addr) {  
  setDataBusInput();
  setAddress(addr);
  
  if (addr >= 0x8000) {
    digitalWrite(CS_PIN, LOW);
  }
  digitalWrite(RD_PIN, LOW);
  delayMicroseconds(1);
  
  // digitalWrite(CLK_PIN, HIGH);

  uint8_t data = getData();

  // digitalWrite(CLK_PIN, LOW);

  digitalWrite(RD_PIN, HIGH);
  digitalWrite(CS_PIN, HIGH);

  return data;
}

// Write byte to address operation
void writeByte(uint16_t addr, uint8_t data) {
  setDataBusOutput();
  setAddress(addr);  
  setData(data);
  
  if (addr >= 0x8000) {
    digitalWrite(CS_PIN, LOW);
  }
  digitalWrite(WR_PIN, LOW);
  delayMicroseconds(1);

  digitalWrite(WR_PIN, HIGH);
  digitalWrite(CS_PIN, HIGH);

  setDataBusInput();
}

// Parse hex input
uint32_t parseHex(String str) {
  str.trim();
  str.toLowerCase();

  if (str.startsWith("0x")) {
    str = str.substring(2);
  }

  uint32_t value = 0;
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    value <<= 4;

    if (c >= '0' && c <= '9') {
      value |= (c - '0');
    } else if (c >= 'a' && c <= 'f') {
      value |= (c - 'a' + 10);
    } else {
      break;
    }
  }

  return value;
}

/*
--- Single Byte Operations ---
*/
// Read byte at address
void cmdReadByte(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println("Error: Missing address");
    Serial.println("Using: readByte <addr>");
    return;
  }

  uint16_t addr = (uint16_t)parseHex(args);
  uint8_t data = readByte(addr);

  Serial.print("0x");
  if (addr < 0x1000) Serial.print("0");
  if (addr < 0x0100) Serial.print("0");
  if (addr < 0x0010) Serial.print("0");
  Serial.print(addr, HEX);
  Serial.print(": 0x");
  if (data < 0x10) Serial.print("0");
  Serial.println(data, HEX);
}

// Write byte to address
void cmdWriteByte(String args) {
  args.trim();

  int spacePos = args.indexOf(' ');
  if (spacePos == -1) {
    Serial.println("Error: Missing value");
    Serial.println("Usage: write <addr> <data>");
    return;
  }

  String addrStr = args.substring(0, spacePos);
  String datStr = args.substring(spacePos + 1);

  uint16_t addr = (uint16_t)parseHex(addrStr);
  uint8_t data = (uint8_t)parseHex(datStr);

  writeByte(addr, data);

  Serial.print("Written 0x");
  if (data < 0x10) Serial.print("0");
  Serial.print(data, HEX);
  Serial.print(" to address 0x");
  if (addr < 0x1000) Serial.print("0");
  if (addr < 0x0100) Serial.print("0");
  if (addr < 0x0010) Serial.print("0");
  Serial.println(addr, HEX);

  uint8_t verify = readByte(addr);
  Serial.print("Verification: 0x");
  if (verify < 0x10) Serial.print("0");
  Serial.print(verify, HEX);
  if (verify == data) {
    Serial.println(" - OK");
  } else {
    Serial.println(" - FAILED!");
  }
}

/*
--- Range Operations
*/
// Read range of addresses
void cmdReadRange(String args) {
  args.trim();

  int spacePos = args.indexOf(' ');
  if (spacePos == -1) {
    Serial.println("Error: Missing end address");
    Serial.println("Usage: readRange <start> <end>");
    return;
  }

  String startStr = args.substring(0, spacePos);
  String endStr = args.substring(spacePos + 1);

  uint16_t startAddr = (uint16_t)parseHex(startStr);
  uint16_t endAddr = (uint16_t)parseHex(endStr);

  if (endAddr < startAddr) {
    Serial.println("Error: end address < start address");
    return;
  }

  Serial.print("Reading 0x");
  if (startAddr < 0x1000) Serial.print("0");
  if (startAddr < 0x0100) Serial.print("0");
  if (startAddr < 0x0010) Serial.print("0");
  Serial.print(startAddr, HEX);
  Serial.print(" to 0x");
  if (endAddr < 0x1000) Serial.print("0");
  if (endAddr < 0x0100) Serial.print("0");
  if (endAddr < 0x0010) Serial.print("0");
  Serial.print(endAddr, HEX);
  Serial.print(" (");
  Serial.print(endAddr - startAddr + 1);
  Serial.println(" bytes)...");

  for (uint16_t addr = startAddr; addr <= endAddr; addr++) {
    uint8_t data = readByte(addr);
    Serial.write(data);
  }

  Serial.println("\nRead complete");
}

// Write block of data
void cmdWriteBlock(String args) {
  args.trim();

  int spacePos = args.indexOf(' ');
  if (spacePos == -1) {
    Serial.println("Error: Missing length");
    Serial.println("Usage: writeBlock <addr> <len>");
    return;
  }
  
  String addrStr = args.substring(0, spacePos);
  String lenStr = args.substring(spacePos + 1);
  
  uint16_t startAddr = (uint16_t)parseHex(addrStr);
  uint16_t length = (uint16_t)parseHex(lenStr);

  Serial.print("Ready to receive ");
  Serial.print(length, DEC);
  Serial.println(" bytes. Send binary data now...");

  uint16_t bytesWritten = 0;
  uint32_t timeout = millis();
  
  while (bytesWritten < length) {
    if (Serial.available() > 0) {
      uint8_t data = Serial.read();
      writeByte(startAddr + bytesWritten, data);
      bytesWritten++;
      timeout = millis();
      
      if (bytesWritten % 256 == 0) {
        Serial.print("Progress: ");
        Serial.print(bytesWritten);
        Serial.println(" bytes");
      }
    }

    if (millis() - timeout > 10000) {
      Serial.println("Timeout!");
      break;
    }
  }

  Serial.print("Write complete: ");
  Serial.print(bytesWritten);
  Serial.println(" bytes written");
}

// Hex dump of memory
void cmdDump(String args) {
  args.trim();

  int spacePos = args.indexOf(' ');
  if (spacePos == -1) {
    Serial.println("Error: Missing length");
    Serial.println("Usage: dump <addr> <len>");
    return;
  }

  String addrStr = args.substring(0, spacePos);
  String lenStr = args.substring(spacePos + 1);

  uint16_t startAddr = (uint16_t)parseHex(addrStr);
  uint16_t length = (uint16_t)parseHex(lenStr);

  Serial.println("\nAddress | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII");
  Serial.println("--------|-------------------------------------------------|------------------");

  for (uint16_t offset = 0; offset < length; offset += 16) {
    uint16_t addr = startAddr + offset;

    Serial.print("0x");
    if (addr < 0x1000) Serial.print("0");
    if (addr < 0x0100) Serial.print("0");
    if (addr < 0x0010) Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print("  | ");

    uint8_t lineData[16];
    int bytesToRead = min(16, length - offset);
 
    for (int i = 0; i < bytesToRead; i++) {
      lineData[i] = readByte(addr + i);
      if (lineData[i] < 0x10) Serial.print("0");
      Serial.print(lineData[i], HEX);
      Serial.print(" ");
    }

    for (int i = bytesToRead; i < 16; i++) {
      Serial.print("   ");
    }

    Serial.print("| ");
    for (int i = 0; i < bytesToRead; i++) {
      if (lineData[i] >= 32 && lineData[i] <= 126) {
        Serial.print((char)lineData[i]);
      } else {
        Serial.print(".");
      }
    }

    Serial.println();
  }

  Serial.println("Dump complete\n");
}

/*
-- Bank Operations
*/
// Switch ROM Bank
void cmdSwitchROMBank(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println("Error: Missing bank number");
    Serial.println("Usage: switchROMBank <bank>");
    return;
  }

  uint8_t bank = (uint8_t)parseHex(args);

  // MBC1/MBC3/MBC5
  writeByte(0x2000, bank);

  Serial.println("Switched to ROM bank 0x");
  if (bank < 0x10) Serial.print("0");
  Serial.println(bank, HEX);
}

// Switch RAM Bank
void cmdSwitchRAMBank(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println("Error: Missing bank number");
    Serial.println("Usage: switchRAMBank <bank>");
    return;
  }

  uint8_t bank = (uint8_t)parseHex(args);

  // MBC1/MBC3/MBC5
  writeByte(0x4000, bank);

  Serial.println("Switched to RAM bank 0x");
  if (bank < 0x10) Serial.print("0");
  Serial.println(bank, HEX);
}

/*
--- Bulk Operations
*/
// Read certridge header
void cmdReadHeader() {
  Serial.println("\n=== Cartridge Header ===");

  // Game name
  Serial.print("Title: ");
  for (uint16_t addr = 0x134; addr <= 0x143; addr++) {
    char c = readByte(addr);
    if (c == 0) break;
    Serial.print(c);
  }
  Serial.println();

  // Cartridge type
  uint8_t cartType = readByte(0x0147);
  Serial.print("Cartridge Type: 0x");
  if (cartType < 0x10) Serial.print("0");
  Serial.print(cartType, HEX);
  Serial.print(" - ");
  printCartridgeType(cartType);

  // ROM size
  uint8_t romSize = readByte(0x0148);
  Serial.print("ROM Size: 0x");
  if (romSize < 0x10) Serial.print("0");
  Serial.print(romSize, HEX);
  Serial.print(" - ");
  printROMSize(romSize);

  // RAM size
  uint8_t ramSize = readByte(0x0149);
  Serial.print("RAM Size: 0x");
  if (ramSize < 0x10) Serial.print("0");
  Serial.print(ramSize, HEX);
  Serial.print(" - ");
  printRAMSize(ramSize);

  // Header checksum
  uint8_t headerChecksum = readByte(0x014D);
  Serial.print("Header Checksum: 0x");
  if (headerChecksum < 0x10) Serial.print("0");
  Serial.println(headerChecksum, HEX);

  Serial.println("========================\n");
}

void printCartridgeType(uint8_t type) {
  switch(type) {
    case 0x00: Serial.println("ROM ONLY"); break;
    case 0x01: Serial.println("MBC1"); break;
    case 0x02: Serial.println("MBC1+RAM"); break;
    case 0x03: Serial.println("MBC1+RAM+BATTERY"); break;
    case 0x05: Serial.println("MBC2"); break;
    case 0x06: Serial.println("MBC2+BATTERY"); break;
    case 0x08: Serial.println("ROM+RAM"); break;
    case 0x09: Serial.println("ROM+RAM+BATTERY"); break;
    case 0x0B: Serial.println("MMM01"); break;
    case 0x0C: Serial.println("MMM01+RAM"); break;
    case 0x0D: Serial.println("MMM01+RAM+BATTERY"); break;
    case 0x0F: Serial.println("MBC3+TIMER+BATTERY"); break;
    case 0x10: Serial.println("MBC3+TIMER+RAM+BATTERY"); break;
    case 0x11: Serial.println("MBC3"); break;
    case 0x12: Serial.println("MBC3+RAM"); break;
    case 0x13: Serial.println("MBC3+RAM+BATTERY"); break;
    case 0x19: Serial.println("MBC5"); break;
    case 0x1A: Serial.println("MBC5+RAM"); break;
    case 0x1B: Serial.println("MBC5+RAM+BATTERY"); break;
    case 0x1C: Serial.println("MBC5+RUMBLE"); break;
    case 0x1D: Serial.println("MBC5+RUMBLE+RAM"); break;
    case 0x1E: Serial.println("MBC5+RUMBLE+RAM+BATTERY"); break;
    default: Serial.println("Unknown"); break;
  }
}

void printROMSize(uint8_t size) {
  switch(size) {
    case 0x00: Serial.println("32 KB (2 banks)"); break;
    case 0x01: Serial.println("64 KB (4 banks)"); break;
    case 0x02: Serial.println("128 KB (8 banks)"); break;
    case 0x03: Serial.println("256 KB (16 banks)"); break;
    case 0x04: Serial.println("512 KB (32 banks)"); break;
    case 0x05: Serial.println("1 MB (64 banks)"); break;
    case 0x06: Serial.println("2 MB (128 banks)"); break;
    case 0x07: Serial.println("4 MB (256 banks)"); break;
    case 0x08: Serial.println("8 MB (512 banks)"); break;
    default: Serial.println("Unknown"); break;
  }
}

void printRAMSize(uint8_t size) {
  switch(size) {
    case 0x00: Serial.println("None"); break;
    case 0x01: Serial.println("2 KB"); break;
    case 0x02: Serial.println("8 KB (1 bank)"); break;
    case 0x03: Serial.println("32 KB (4 banks)"); break;
    case 0x04: Serial.println("128 KB (16 banks)"); break;
    case 0x05: Serial.println("64 KB (8 banks)"); break;
    default: Serial.println("Unknown"); break;
  }
}

// Read entire ROM (32KB)
void cmdReadROM() {
  Serial.println("Reading ROM (first 32KB)...");
  Serial.println("Binary data follows:");

  for (uint16_t addr = 0x0000; addr < 0x8000; addr++) {
    uint8_t data = readByte(addr);
      Serial.write(data);
  }

  Serial.println("\nROM read complete");
}

// Read entire RAM
void cmdReadRAM() {
  Serial.println("Reading RAM...");

  enableRAM();

  Serial.println("Binary data follows:");

  for (uint16_t addr = 0x0000; addr < 0x8000; addr++) {
    uint8_t data = readByte(addr);
    Serial.write(data);
  }

  disableRAM();

  Serial.println("\nRAM read complete");
}

/*
--- Other ---
*/
void cmdTestReadWrite() {
  Serial.println("\n=== Read/Write Test ===");

  // Test ROM reading
  Serial.println("Reading first 16 bytes of ROM:");
  for (uint16_t i = 0; i < 16; i++) {
    uint8_t data = readByte(i);
    if (data < 0x10) Serial.print("0");
    Serial.print(data, HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Test RAM write/read
  Serial.println("\nTesing RAM write/read:");
  enableRAM();

  uint16_t testAddr = 0xA000;
  uint8_t testValue = 0x42;

  Serial.print("Writing 0x");
  if (testValue < 0x10) Serial.print("0");
  Serial.print(testValue, HEX);
  Serial.print(" to 0x");
  if (testAddr < 0x1000) Serial.print("0");
  if (testAddr < 0x0100) Serial.print("0");
  if (testAddr < 0x0010) Serial.print("0");
  Serial.println(testAddr, HEX);

  writeByte(testAddr, testValue);
  uint8_t readBack = readByte(testAddr);

  Serial.print("Read back: 0x");
  if (readBack < 0x10) Serial.print("0");
  Serial.print(readBack, HEX);

  if (readBack == testValue) {
    Serial.println(" - PASS");
  } else {
    Serial.println(" - FAIL");
  }

  disableRAM();
  Serial.println("Test complete\n");
}
