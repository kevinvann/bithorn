#include "BluetoothA2DPSource.h"
#include <SPI.h>
#include <SD.h>
// #define DEBUG
using namespace std;

BluetoothA2DPSource a2dp_source;
File sound_file;
const int sd_ss_pin = 5;

// sd card must have these files or code will error out
const char* ready_file_name = "/ready.raw";
const char* horn_file_name = "/horn.raw";
const char* silence_file_name = "/silence.raw";

const int frame_size_bytes = sizeof(int16_t) * 2;
const int musicButton = 4;
const int bit3 = 26;
const int bit2 = 27;
const int bit1 = 14;
const int bit0 = 12;

String trackFileName = "";
int trackNumber;
bool isMusicPlaying = false;
bool musicButtonPressed = false;

int musicButtonState = HIGH;
int lastMusicButtonState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// callback used by A2DP to provide the sound data
int32_t get_sound_data(Channels* data, int32_t len) {
  size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes);
  int32_t result_len = result_len_bytes / frame_size_bytes;

  // if sound data is being sent, music is playing
  if (result_len > 0) {
    isMusicPlaying = true;
  } else {
    isMusicPlaying = false;
  }

  return result_len;
}

// Arduino Setup
void setup(void) {
  Serial.begin(115200);

  SD.begin(sd_ss_pin);

  pinMode(musicButton, INPUT_PULLUP);

  pinMode(bit3, INPUT_PULLUP);
  pinMode(bit2, INPUT_PULLUP);
  pinMode(bit1, INPUT_PULLUP);
  pinMode(bit0, INPUT_PULLUP);
  pinMode(2, OUTPUT);

  sound_file = SD.open(ready_file_name, FILE_READ);
  a2dp_source.set_volume(20);
  a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);
}

// Arduino loop - repeated processing
void loop() {

  int musicButtonReading = digitalRead(musicButton);

  // If the switch changed, due to noise or pressing:
  if (musicButtonReading != lastMusicButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (musicButtonReading != musicButtonState) {
      musicButtonState = musicButtonReading;
      if (musicButtonState == HIGH) {
        musicButtonPressed = true;
      }
    }
  }

#ifdef DEBUG
  Serial.print("BIT READING: ");
  Serial.print(digitalRead(bit3));
  Serial.print(digitalRead(bit2));
  Serial.print(digitalRead(bit1));
  Serial.print(digitalRead(bit0));
  Serial.println();

  Serial.print("MUSIC BUTTON STATE: ");
  Serial.print(musicButtonState);
  Serial.println();
#endif

  // set the track name based on toggle switch inputs
  trackNumber = digitalRead(bit3)*8 + digitalRead(bit2)*4 + digitalRead(bit1)*2 + digitalRead(bit0);
  trackFileName = "/" + String(trackNumber) + ".raw";

#ifdef DEBUG
  Serial.print("TRACK FILE NAME: ");
  Serial.println(trackFileName);
#endif

  // if music is not playing yet, play on button press
  if (musicButtonPressed && !isMusicPlaying) {
    sound_file = SD.open(trackFileName, FILE_READ);
    a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);
    musicButtonPressed = false;
  }

  // if music is already playing silence it
  if (musicButtonPressed && isMusicPlaying) {
    sound_file = SD.open(silence_file_name, FILE_READ);
    musicButtonPressed = false;
  }

  lastMusicButtonState = musicButtonReading;

  musicButtonPressed = false;
}