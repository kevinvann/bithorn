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
const int music_button = 4;
const int input_bit_3 = 27;
const int input_bit_2 = 14;
const int input_bit_1 = 12;
const int input_bit_0 = 13;
const int led_bit_3 = 32;
const int led_bit_2 = 33;
const int led_bit_1 = 25;
const int led_bit_0 = 26;

String track_file_name = "";
int track_number;
bool is_music_playing = false;
bool music_button_pressed = false;

int music_button_state = HIGH;
int last_music_button_state = HIGH;

unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50;

// callback used by A2DP to provide the sound data
int32_t get_sound_data(Channels* data, int32_t len) {
  size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes);
  int32_t result_len = result_len_bytes / frame_size_bytes;

  // if sound data is being sent, music is playing
  if (result_len > 0) {
    is_music_playing = true;
  } else {
    is_music_playing = false;
  }

  return result_len;
}

// Arduino Setup
void setup(void) {
  Serial.begin(115200);

  SD.begin(sd_ss_pin);

  pinMode(music_button, INPUT_PULLUP);

  pinMode(input_bit_3, INPUT_PULLUP);
  pinMode(input_bit_2, INPUT_PULLUP);
  pinMode(input_bit_1, INPUT_PULLUP);
  pinMode(input_bit_0, INPUT_PULLUP);
  pinMode(led_bit_3, OUTPUT);
  pinMode(led_bit_2, OUTPUT);
  pinMode(led_bit_1, OUTPUT);
  pinMode(led_bit_0, OUTPUT);
  pinMode(2, OUTPUT);

  sound_file = SD.open(ready_file_name, FILE_READ);
  a2dp_source.set_volume(20);
  a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);
}

// Arduino loop - repeated processing
void loop() {

  // read inputs
  int music_button_reading = digitalRead(music_button);
  int input_bit_3_reading = digitalRead(input_bit_3);
  int input_bit_2_reading = digitalRead(input_bit_2);
  int input_bit_1_reading = digitalRead(input_bit_1);
  int input_bit_0_reading = digitalRead(input_bit_0);

  
  // light onboard LED if bluetooth is connected
  if (a2dp_source.get_connection_state() == 2) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }

  digitalWrite(led_bit_3, input_bit_3_reading);
  digitalWrite(led_bit_2, input_bit_2_reading);
  digitalWrite(led_bit_1, input_bit_1_reading);
  digitalWrite(led_bit_0, input_bit_0_reading);



  // If the switch changed, due to noise or pressing:
  if (music_button_reading != last_music_button_state) {
    // reset the debouncing timer
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (music_button_reading != music_button_state) {
      music_button_state = music_button_reading;
      if (music_button_state == HIGH) {
        music_button_pressed = true;
      }
    }
  }

  // set the track name based on toggle switch inputs
  track_number = input_bit_3_reading*8 + input_bit_2_reading*4 + input_bit_1_reading*2 + input_bit_0_reading;
  track_file_name = "/" + String(track_number) + ".raw";


  // if music is not playing yet, play on button press
  if (music_button_pressed && !is_music_playing) {
    sound_file = SD.open(track_file_name, FILE_READ);
    a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);
    music_button_pressed = false;
  }


  // if music is already playing silence it
  if (music_button_pressed && is_music_playing) {
    sound_file = SD.open(silence_file_name, FILE_READ);
    music_button_pressed = false;
  }

  last_music_button_state = music_button_reading;

  music_button_pressed = false;
}