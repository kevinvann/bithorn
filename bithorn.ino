#include "BluetoothA2DPSource.h"
#include <SPI.h>
#include <SD.h>
using namespace std;

BluetoothA2DPSource a2dp_source;
File sound_file;
const char* ready_file_name = "/ready.raw";
const int sd_ss_pin = 5;
const int frame_size_bytes = sizeof(int16_t) * 2;
const int hornButton = 4;
const int bit3 = 26;
const int bit2 = 27;
const int bit1 = 14;
const int bit0 = 12;
int buttonState;
int trackNumber = 0;
String trackName = "";

// callback used by A2DP to provide the sound data
int32_t get_sound_data(Channels* data, int32_t len) {
  // the data in the file must be in int16 with 2 channels 
  size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes);
  // result is in number of frames
  int32_t result_len = result_len_bytes / frame_size_bytes;
  ESP_LOGD("get_sound_data", "%d -> %d",len );
  return result_len;
}

// Arduino Setup
void setup(void) {
  Serial.begin(115200);



  pinMode(hornButton, INPUT_PULLUP);

  pinMode(bit3, INPUT_PULLUP);
  pinMode(bit2, INPUT_PULLUP);
  pinMode(bit1, INPUT_PULLUP);
  pinMode(bit0, INPUT_PULLUP);



  // Setup SD and open file
  SD.begin(sd_ss_pin);
  sound_file = SD.open(ready_file_name, FILE_READ);

  // start the bluetooth
  Serial.println("starting A2DP...");
  a2dp_source.set_volume(50);
  a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);
}

// Arduino loop - repeated processing 
void loop() {

  Serial.print(digitalRead(bit3));
  Serial.print(digitalRead(bit2));
  Serial.print(digitalRead(bit1));
  Serial.print(digitalRead(bit0));
  Serial.println();
  trackNumber = digitalRead(bit3)*8 + digitalRead(bit2)*4 + digitalRead(bit1)*2 + digitalRead(bit0);
  Serial.println(trackNumber);

  buttonState = digitalRead(hornButton);
  
  trackName = "/" + String(trackNumber) + ".raw";
  Serial.println(trackName);

  if (buttonState == LOW) {
    sound_file = SD.open(trackName, FILE_READ);
    a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data); 
    while (digitalRead(hornButton) == LOW) {
      delay(10);
    }
  }

  sound_file = SD.open("/silence.raw", FILE_READ);
  a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);

  while (digitalRead(hornButton) == HIGH) {
    delay(10);
  }
  

}