#include <LiquidCrystal_I2C.h>

const int RxD2 = 16;
const int TxD2 = 17;
const int velocityPin = 27;
const int notePin = 14;

const char* midiNoteNumberToName[128] = {
"C-1", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
"C0", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A0", "A#0", "B0",
"C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1", 
"C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
"C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
"C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
"C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
"C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
"C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
"C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "A#8", "B8",
"C9", "C#9", "D9", "D#9", "E9", "F9", "F#9", "G9"
};

const char* keyNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// TODO: add more commands and change them to enum:
const uint8_t noteOnCommand = 144;
const uint8_t noteOffCommand = 128;

const uint8_t numberOfScales = 3;
const char* scaleNames[numberOfScales] = {"Pentatonic", "Chromatic", "Major"};
const uint8_t scaleLengths[numberOfScales] = {5, 12, 7};
const uint8_t intervalsPentatonic[] = {0, 3, 5, 7, 10};
const uint8_t intervalsChromatic[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const uint8_t intervalsMajor[] = {0, 2, 4, 5, 7, 9, 11};
const uint8_t* intervals[] = {intervalsPentatonic, intervalsChromatic, intervalsMajor};

uint64_t lastScreenUpdateTime = 0;

// TODO: adjustable parameters:
uint8_t currentKey = 0;
uint8_t currentOctave = 4;
uint8_t numberOfNotes = 13;
uint8_t currentScaleIndex = 2;
uint8_t currentNoteNumber = 60;
uint8_t currentVelocity = 100;

// TODO: replace LCD with OLED display
LiquidCrystal_I2C lcd(0x27, 16, 2); 


void updateMidiNoteNumber()
{
  const int sensorRead = analogRead(notePin);
  const int sensorNoteChange = map(sensorRead, 0, 4095, 0, numberOfNotes - 1);
  const int octaveNoteChange = (currentOctave + 1) * 12;
  const int currentScaleLength = scaleLengths[currentScaleIndex];
  const int intervalsNoteChange = intervals[currentScaleIndex][sensorNoteChange % currentScaleLength];
  const int intervalsOverflowNoteChange = sensorNoteChange / currentScaleLength * 12;
  currentNoteNumber = octaveNoteChange + currentKey + intervalsNoteChange + intervalsOverflowNoteChange;
}


void updateVelocity()
{
  int sensorRead = analogRead(velocityPin);
  int velo = map(sensorRead, 0, 4095, 0, 127);
  currentVelocity = velo;
}


void MIDImessage(int command, int MIDInote, int MIDIvelocity)
{
  // TODO: change midi channel
  Serial2.write(command);
  Serial2.write(MIDInote);
  Serial2.write(MIDIvelocity); 
}


void updateDisplay()
{
  Serial.println(esp_timer_get_time());
  Serial.println(lastScreenUpdateTime);
  Serial.println("");
  if ((esp_timer_get_time() - lastScreenUpdateTime) > 300000)
  {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(midiNoteNumberToName[currentNoteNumber]);

    lcd.setCursor(6, 0);
    lcd.print(scaleNames[currentScaleIndex]);

    lcd.setCursor(11, 1);
    lcd.print("oct:");
    lcd.print(currentOctave);

    lcd.setCursor(0, 1);
    lcd.print("velo:");
    lcd.print(currentVelocity);

    lastScreenUpdateTime = esp_timer_get_time();
  }
}


void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ThereMIDIn");
  lcd.setCursor(0, 1);
  lcd.print("firmware v0.0.1");
  delay(1000);

  pinMode(velocityPin, INPUT);
  pinMode(notePin, INPUT);
  Serial2.begin(31250, SERIAL_8N1, RxD2, TxD2);
  Serial.begin(115200);
}


void loop()
{
  updateDisplay();
  updateMidiNoteNumber();
  updateVelocity();

  MIDImessage(noteOnCommand, currentNoteNumber, currentVelocity);
  delay(100);
  MIDImessage(noteOffCommand, currentNoteNumber, 0);
  delay(100);
}
