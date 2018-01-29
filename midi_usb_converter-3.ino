
/*
* ********************************************************
* Converter that sends midi messages from a 
* conventional 5pin din connection (midi-in)
* to the USB shield output.
* ********************************************************
* 

*/

// include the library code:
#include <MIDI.h>
#include <LiquidCrystal.h>
#include <Usb.h>
#include <usbh_midi.h>

/*   USB ERROR CODES
 hrSUCCESS   0x00
 hrBUSY      0x01
 hrBADREQ    0x02
 hrUNDEF     0x03
 hrNAK       0x04
 hrSTALL     0x05
 hrTOGERR    0x06
 hrWRONGPID  0x07
 hrBADBC     0x08
 hrPIDERR    0x09
 hrPKTERR    0x0A
 hrCRCERR    0x0B
 hrKERR      0x0C
 hrJERR      0x0D
 hrTIMEOUT   0x0E
 hrBABBLE    0x0F
 */
 
//////////////////////////
// MIDI MESAGES 
// midi.org/techspecs/
//////////////////////////
// STATUS BYTES
// 0x8n == noteOn
// 0x9n == noteOff
// 0xAn == afterTouch
// 0xBn == controlChange
// 0xCn == programChange
// 
// n == channel number (1-F)
//////////////////////////
//DATA BYTE 1
// note# == (0-127)
// or
// control# == (0-119)
//////////////////////////
// DATA BYTE 2
// velocity == (0-127)
// or
// controlVal == (0-127)
//////////////////////////

// Workaround for Arduino MIDI library v4.0 compatibility
#ifdef USE_SERIAL_PORT
#define _MIDI_SERIAL_PORT USE_SERIAL_PORT
#else
#define _MIDI_SERIAL_PORT MIDI_DEFAULT_SERIAL_PORT
#endif

USB  Usb;
USBH_MIDI  Midi(&Usb);

byte NoteOn = 0x80;
byte NoteOff = 0x90;
byte ControlChange = 0xB0;
byte ProgramChange = 0xC0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

MIDI_CREATE_DEFAULT_INSTANCE();

// define vu-bar chars
byte VU0[8] = 
{
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000
};
byte VU10[8] = 
{
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B11111
};
byte VU20[8] = 
{
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B11111,
 B11111
};
byte VU40[8] =
{
 B00000,
 B00000,
 B00000,
 B00000,
 B11111,
 B11111,
 B11111,
 B11111
};
byte VU60[8] =
{
 B00000,
 B00000,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111
};
byte VU80[8] =
{
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111,
 B11111
};


// Display a VU-like meter per channel on the second display line.
// Only for note-on messages
// Chan 1 bar is at display column 0!!

void VU(int CHAN)
{ 
// it seems that displaying a VU meter per channel is taking
// too much of the system timing.
// So only the 'full bar' is displayed.

 int x = CHAN-1;
 //lcd.setCursor(x,1);
 //lcd.write(2);
 //delay(10);
 // lcd.setCursor(x,1);
 //lcd.write(3);
 //delay(10);
 // lcd.setCursor(x,1);
 //lcd.write(4);
 //delay(10);
 lcd.setCursor(x,1);
 lcd.write(5);
 //delay(50);
 // lcd.setCursor(x,1);
 //lcd.write(1);
 }

// When a channel receives a note-off message, the VU bar is set to zero.
void ResetVu(int CHAN)
{ 
 int x = CHAN-1;
 lcd.setCursor(x,1);
 lcd.write(1);
}

// Write text on the top line of the display
void WriteTopLine(String TEXT) {
lcd.setCursor(0,0);
lcd.print(TEXT);  
}

// Write text on the bottom line of the display
// after clearing the line
void WriteBotLine(String TEXT) {
String CLRSTRING = "                ";
lcd.setCursor(0,1);
lcd.print(CLRSTRING);  
lcd.setCursor(1,1);
lcd.print(TEXT);
}

// Initial setup of the VU bar line (all 0's)
void InitVUBar() {
for (int i=0; i <= 15; i++){
      lcd.setCursor(i,1);
      lcd.write(1);
      delay(10);
   }  
}

// When midi data is received, blink an * sign on the top-right corner
// Is destroying the midi timing, so not used
void Blink() {
  lcd.setCursor(13,0);
  lcd.print("[*]");
  delay(10);
  lcd.setCursor(13,0);
  lcd.print("[ ]");
}

// Initial screen after welcome message 
void ClearScreen() {
  String CLRSTRING = "                ";
  WriteBotLine(CLRSTRING);
  WriteTopLine("   Midi2Usb    ");
}

// sends CC value 
void SendCC(byte CHAN, byte NR, byte VAL) {
//  MIDI.sendControlChange(NR, VAL, CHAN);
   uint8_t msg[4];
   msg[0] = ControlChange + CHAN;
   msg[1] = NR;
   msg[2] = VAL;
   Midi.SendData(msg, 0);
  }

// Sends progchange command
void SendPP(byte CHAN, byte PRG) {
//  MIDI.sendProgramChange(PRG, CHAN);
   uint8_t msg[3];
   msg[0] = ProgramChange + CHAN;
   msg[1] = PRG;
   Midi.SendData(msg, 0);
  }

// Sends NoteOn command 
void SendNoteOn(byte CHAN, byte NOTE, byte VEL) {
//  MIDI.sendNoteOn(NOTE, VEL, CHAN);
   uint8_t msg[4];
   msg[0] = NoteOn + CHAN;
   msg[1] = NOTE;
   msg[2] = VEL;
   Midi.SendData(msg, 0);
   VU(CHAN);
  }

// Sends NoteOff command
void SendNoteOff(byte CHAN, byte NOTE, byte VEL) {
//  MIDI.sendNoteOff(NOTE, VEL, CHAN);
   uint8_t msg[4];
   msg[0] = NoteOff + CHAN;
   msg[1] = NOTE;
   msg[2] = VEL;
   Midi.SendData(msg, 0);
   ResetVu(CHAN);
}

// Sends Sysex message 
void SendSysEx(byte ARRAY, byte SIZE) {
  MIDI.sendSysEx(SIZE, ARRAY, true);
}
  
void setup() {
// Vu Bar character definition
lcd.createChar(0,VU0);
lcd.createChar(1,VU10);
lcd.createChar(2,VU20);
lcd.createChar(3,VU40);
lcd.createChar(4,VU60);
lcd.createChar(5,VU80);

// Welcome message
lcd.begin(16, 2);
lcd.print("  Midi2Usb 1.6  "); 
lcd.setCursor(0,1);
lcd.print(" (c)Patrick Mans");
delay(4000);

// Check if USB host shield is available
if (Usb.Init() == -1) {
    WriteBotLine("USB init error");
 //   while(1); //Halt if not...
   }

// Init screen
ClearScreen();
InitVUBar();

// Handles for incoming midi messages
 MIDI.setHandleControlChange(SendCC);
 MIDI.setHandleProgramChange(SendPP);
 MIDI.setHandleNoteOn(SendNoteOn);
 MIDI.setHandleNoteOff(SendNoteOff);
 MIDI.setHandleSystemExclusive(SendSysEx);
 MIDI.begin();

 // Workaround for non UHS2.0 Shield 
 pinMode(7,OUTPUT);
 digitalWrite(7,HIGH);
}

void loop() {
MIDI.read(MIDI_CHANNEL_OMNI);
}
