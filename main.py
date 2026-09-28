import serial
import time

class GameBoyCartridge:
    LOGO_CHECK = bytes.fromhex('CE ED 66 66 CC 0D 00 0B 03 73 00 83 00 0C 00 0D'
                               '00 08 11 1F 88 89 00 0E DC CC 6E E6 DD DD D9 99'
                               'BB BB 67 63 6E 0E EC CC DD DC 99 9F BB B9 33 3E')
    
    def __init__(self, bank0_data):
        self.bank0_data = bank0_data
        self.title = self.get_title()
        self.rom_size = self.get_rom_size()
        self.ram_size = self.get_ram_size()
        self.cartridge_type = self.get_cartridge_type()
        self.rom_banks = []

        if self.get_logo() != self.LOGO_CHECK:
            print("Warning: Nintendo logo does not match expected value. The cartridge may be invalid.")

        if self.get_header_checksum() != self.calculate_header_checksum():
            print("Warning: Header checksum does not match expected value. The cartridge may be corrupted.")

    def get_logo(self):
        return self.bank0_data[0x0104:0x0134]

    def get_title(self):
        return ''.join([chr(c) if c >= 32 and c <= 126 else '' for c in self.bank0_data[0x0134:0x0144]])
    
    def get_rom_size(self):
        return self.bank0_data[0x0148]
    
    def get_rom_banks_count(self):
        return 2 ** (self.rom_size + 1) if self.rom_size > 0 else 2
    
    def get_ram_size(self):
        return self.bank0_data[0x0149]
    
    def get_cartridge_type(self):
        return self.bank0_data[0x0147]
    
    def get_header_checksum(self):
        return self.bank0_data[0x014D]
    
    def calculate_header_checksum(self):
        checksum = 0
        for i in range(0x0134, 0x014D):
            checksum = (checksum - self.bank0_data[i] - 1) & 0xFF
        return checksum
    
    def add_rom_bank(self, bank_data):
        self.rom_banks.append(bank_data)

    def get_cartridge_type_name(self):
        cartridge_types = {
            0x00: "ROM ONLY",
            0x01: "MBC1",
            0x02: "MBC1+RAM",
            0x03: "MBC1+RAM+BATTERY",
            0x05: "MBC2",
            0x06: "MBC2+BATTERY",
            0x08: "ROM+RAM",
            0x09: "ROM+RAM+BATTERY",
            0x0B: "MMM01",
            0x0C: "MMM01+RAM",
            0x0D: "MMM01+RAM+BATTERY",
            0x0F: "MBC3+TIMER+BATTERY",
            0x10: "MBC3+TIMER+RAM+BATTERY",
            0x11: "MBC3",
            0x12: "MBC3+RAM",
            0x13: "MBC3+RAM+BATTERY",
            0x19: "MBC5",
            0x1A: "MBC5+RAM",
            0x1B: "MBC5+RAM+BATTERY",
            0x1C: "MBC5+RUMBLE",
            0x1D: "MBC5+RUMBLE+RAM",
            0x1E: "MBC5+RUMBLE+RAM+BATTERY"
        }
        return cartridge_types.get(self.cartridge_type, f"Unknown (0x{self.cartridge_type:02X})")
    
    def get_rom_size_name(self):
        rom_banks_mapping = {
            0x00: "2 banks (32KB)",
            0x01: "4 banks (64KB)",
            0x02: "8 banks (128KB)",
            0x03: "16 banks (256KB)",
            0x04: "32 banks (512KB)",
            0x05: "64 banks (1MB)",
            0x06: "128 banks (2MB)",
            0x07: "256 banks (4MB)"
        }
        return rom_banks_mapping.get(self.rom_size, f"Unknown (0x{self.rom_size:02X})")
    
    def get_ram_size_name(self):
        ram_banks_mapping = {
            0x00: "None",
            0x01: "1 bank (2KB)",
            0x02: "4 banks (8KB)",
            0x03: "16 banks (32KB)",
            0x04: "8 banks (16KB)"
        }
        return ram_banks_mapping.get(self.ram_size, f"Unknown (0x{self.ram_size:02X})")
    
    def save_rom(self, filename):
        with open(filename, 'wb') as f:
            f.write(self.bank0_data)
            for bank in self.rom_banks:
                f.write(bank)
        print(f"ROM saved to {filename}")

    def print_info(self):
        print(f"Title: {self.title}")
        print(f"Cartridge Type: {self.get_cartridge_type_name()}")
        print(f"ROM Size: {self.get_rom_size_name()}")
        print(f"RAM Size: {self.get_ram_size_name()}")
        print(f"Header Checksum: {self.get_header_checksum():02X} - " + ('PASS' if self.get_header_checksum() == self.calculate_header_checksum() else 'FAIL'))
        print(f"Logo Check: " + ('PASS' if self.get_logo() == self.LOGO_CHECK else 'FAIL'))

class SerialDumper:
    def __init__(self, port, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_port = None

    def connect(self):
        try:
            self.serial_port = serial.Serial(self.port, self.baudrate)
            print(f"Connected to {self.port} at {self.baudrate} baud.")
        except serial.SerialException as e:
            print(f"Error connecting to serial port: {e}")

    def send_command(self, command):
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.write(f"{command}\n".encode())
                self.serial_port.readline()
            except serial.SerialException as e:
                print(f"Error sending command: {e}")
        else:
            print("Serial port is not open.")

    def read_data(self, address, length):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.read_until(b'\n> ')
            try:
                self.send_command(f"readRange 0x{address:x} 0x{address+length-1:x}")
                self.serial_port.readline()
                response = self.serial_port.read(length)
                return response
            except serial.SerialException as e:
                print(f"Error reading from serial port: {e}")
        else:
            print("Serial port is not open.")

    def switch_bank(self, bank_number):
        if self.serial_port and self.serial_port.is_open:
            try:
                self.send_command(f"switchROMBank 0x{bank_number:x}")
                self.serial_port.readline()
            except serial.SerialException as e:
                print(f"Error switching bank: {e}")
        else:
            print("Serial port is not open.")


    def disconnect(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            print("Disconnected from serial port.")

def main():
    serial_dumper = SerialDumper(port='COM4')
    serial_dumper.connect()

    print("Reading ROM data from bank 0...")
    bank0 = serial_dumper.read_data(address=0x0000, length=0x4000) # Read the first 16KB of the ROM

    gbCartridge = GameBoyCartridge(bank0)

    total_banks = gbCartridge.get_rom_banks_count()
    for bank_number in range(1, total_banks):
        if bank_number > 1 and gbCartridge.cartridge_type != b'\x00':  # ROM ONLY
            print(f"Switching to bank {bank_number}...")
            serial_dumper.switch_bank(bank_number)        
        print(f"Reading ROM data from bank {bank_number}...")
        bank_data = serial_dumper.read_data(address=0x4000 , length=0x4000) # Read the next 16KB of the ROM
        gbCartridge.add_rom_bank(bank_data)

    gbCartridge.print_info()

    gbCartridge.save_rom(f"{gbCartridge.title}.GB")

    serial_dumper.disconnect()

if __name__ == "__main__":
    main()
