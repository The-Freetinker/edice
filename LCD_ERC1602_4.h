/* Based on https://os.mbed.com/users/star297/code/ERC1602-4-i2c/ */

#include <Wire.h>

class LCD_ERC1602_4 {

public:
  
  explicit LCD_ERC1602_4(char at = 0x3E) : dev_address(at) {}
  
  ~LCD_ERC1602_4() {}
  
  void begin() {
    Wire.begin(); // join i2c bus (address optional for master)
    restart();
  }
  
  void restart(void) {
    const char init_seq0[init_seq0_length] = {
      Comm_FunctionSet_Normal,
      Comm_ReturnHome, // this may be required to reset the scroll function
      Comm_FunctionSet_Extended,
      Comm_InternalOscFrequency,
      Comm_ContrastSet | (default_Contrast & 0xF),
      Comm_PwrIconContrast | ((default_Contrast >> 4) & 0x3),
      Comm_FollowerCtrl | 0x03,
    };
    delay(100);
    for (int i = 0; i < init_seq0_length; i++) {
      lcd_command(init_seq0[i]);
      delayMicroseconds(30);
    }
    const char init_seq1[init_seq1_length] = {
      Comm_DisplayOnOff,
      Comm_ClearDisplay,
      Comm_EntryModeSet,
    };
    delay(250);
    for (int i = 0; i < init_seq1_length; i++) {
      lcd_command(init_seq1[i]);
      delayMicroseconds(30);
    }
    curs[0] = 0;
    curs[1] = 0;
  }
  
  void cls(void) {
    lcd_command(Comm_ClearDisplay);
    delay(2);
    curs[0] = 0;
    curs[1] = 0;
  }
  
  void put_custom_char(char c_code, const char *cg, char x, char y) {
    for (int i = 0; i < 5; i++) {
      set_CGRAM(c_code, cg);
      putcxy(c_code, x, y);
    }
  }
  
  void contrast(char contrast) {
    lcd_command(Comm_FunctionSet_Extended);
    lcd_command(Comm_ContrastSet | (contrast & 0x0F));
    lcd_command(Comm_PwrIconContrast | ((contrast >> 4) & 0x03));
    lcd_command(Comm_FunctionSet_Normal);
  }
  
  void set_CGRAM(char char_code, const char* cg) {
    for (int i = 0; i < 8; i++) {
      lcd_command((Comm_SetCGRAM | (char_code << 3) | i));
      lcd_data(*cg++);
    }
  }
  
  void set_CGRAM(char char_code, char v) {
    char c[8];
    for (int i = 0; i < 8; i++) c[i] = v;
    set_CGRAM(char_code, c);
  }
  
  void putcxy(char c, char x, char y) {
    if ((x >= MaxCharsInALine) || (y >= 2)) return;
    static const char DDRAMAddress_Ofst[] = {0x00, 0x40};
    lcd_command((Comm_SetDDRAMAddress | DDRAMAddress_Ofst[y]) + x);
    lcd_data(c);
  }
  
  void putc(char line, char c) {
    if ((c == '\n') || (c == '\r')) {
      clear_rest_of_line(line);
      curs[line] = 0;
      return;
    }
    putcxy(c, curs[line]++, line);
  }

  void printxy(char* s, char x, char y) {
    while (char c = *s++) putcxy(c, x++, y);
  }
  
  void print(char line, char *s) {
    while (char c = *s++) putc(line, c);
  }
  
  void printf(char line, char *format, ...) {
    char s[32];
    va_list args;
    va_start(args, format);
    vsnprintf(s, 32, format, args);
    va_end(args);
    print(line, s);
  }
  
private:
  
  static const char Comm_FunctionSet_Normal = 0x38;
  static const char Comm_FunctionSet_Extended = 0x39;
  static const char Comm_FunctionSet_DblLine = 0x3D;
  static const char Comm_InternalOscFrequency = 0x1C;
  static const char Comm_ContrastSet = 0x70;
  static const char Comm_PwrIconContrast = 0x54;
  static const char Comm_FollowerCtrl = 0x68;
  static const char Comm_DisplayOnOff = 0x0C;
  static const char Comm_ClearDisplay = 0x01;
  static const char Comm_EntryModeSet = 0x04;
  static const char Comm_ReturnHome = 0x02;
  static const char Comm_SetDDRAMAddress = 0x80;
  static const char Comm_SetCGRAM = 0x40;
  static const char default_Contrast = 0x2F;
  static const char COMMAND = 0x00;
  static const char DATA = 0x40;
  static const char MaxCharsInALine = 0x10; // buffer depth for one line (no scroll function used)
  static const char init_seq0_length = 7;
  static const char init_seq1_length = 3;
  
  char dev_address;
  char cmd[2];
  char curs[2];
  
  void clear_rest_of_line(char line) {
    for (int i = curs[line]; i < MaxCharsInALine; i++) putcxy(' ', i, line);
  }
  
  int lcd_write(char first, char second) {
    cmd[0] = first;
    cmd[1] = second;
    Wire.beginTransmission(dev_address);
    int i = Wire.write(cmd, 2);
    Wire.endTransmission();
    return i;
  }
  
  int lcd_command(char command) {
    return lcd_write(COMMAND, command);
  }
  
  int lcd_data(char data) {
    return lcd_write(DATA, data);
  }
  
};
