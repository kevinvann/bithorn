#include "BluetoothA2DPSource.h"
#include <SPI.h>
#include <SD.h>
#include <Debounce.h>

BluetoothA2DPSource a2dp_source;
File sound_file;
const int sd_ss_pin = 5;

// sd card must have these files or code will error out
const char* ready_file_name = "/ready.raw";
const char* horn_file_name = "/horn.raw";
const char* silence_file_name = "/silence.raw";

const int frame_size_bytes = sizeof(int16_t) * 2;
const int music_button_pin = 4;
const int horn_button_pin = 15;
const int input_bit_3 = 27;
const int input_bit_2 = 14;
const int input_bit_1 = 12;
const int input_bit_0 = 13;
const int led_bit_3 = 32;
const int led_bit_2 = 33;
const int led_bit_1 = 25;
const int led_bit_0 = 26;
const int led_bit_0_on = 22;
const int led_bit_1_on = 1;
const int led_bit_2_on = 3;
const int led_bit_3_on = 21;

String track_file_name = "";
int track_number;
bool is_music_playing = false;
bool music_button_released = false;
bool horn_button_pressed = false;
bool horn_button_released = false;
bool first_loop = true;

int music_button_state = LOW;
int horn_button_state = HIGH;

int input_bit_3_state;
int input_bit_2_state;
int input_bit_1_state;
int input_bit_0_state;

int last_input_bit_3_state;
int last_input_bit_2_state;
int last_input_bit_1_state;
int last_input_bit_0_state;

int last_music_button_state = LOW;
int last_horn_button_state = HIGH;

Debounce music_button(music_button_pin, 50, true);

// callback used by A2DP to provide the sound data
int32_t get_sound_data(Channels* data, int32_t len) {
  size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes);
  int32_t result_len = result_len_bytes / frame_size_bytes;

  // if there is sound data, music is playing
  is_music_playing = result_len > 0;

  return result_len;
}

// Arduino Setup
void setup(void) {
  Serial.begin(115200);

  SD.begin(sd_ss_pin);

  pinMode(music_button_pin, INPUT_PULLUP);
  pinMode(horn_button_pin, INPUT_PULLUP);

  pinMode(input_bit_3, INPUT_PULLUP);
  pinMode(input_bit_2, INPUT_PULLUP);
  pinMode(input_bit_1, INPUT_PULLUP);
  pinMode(input_bit_0, INPUT_PULLUP);
  pinMode(led_bit_3, OUTPUT);
  pinMode(led_bit_2, OUTPUT);
  pinMode(led_bit_1, OUTPUT);
  pinMode(led_bit_0, OUTPUT);
  pinMode(led_bit_3_on, OUTPUT);
  pinMode(led_bit_2_on, OUTPUT);
  pinMode(led_bit_1_on, OUTPUT);
  pinMode(led_bit_0_on, OUTPUT);
  pinMode(2, OUTPUT);

  sound_file = SD.open(ready_file_name, FILE_READ);
  a2dp_source.set_volume(30);
  a2dp_source.start("OontZ Angle 3 DS 7A9", get_sound_data);
}

void loop() {

  music_button_state = music_button.read();
  horn_button_state = digitalRead(horn_button_pin);
  input_bit_3_state = digitalRead(input_bit_3);
  input_bit_2_state = digitalRead(input_bit_2);
  input_bit_1_state = digitalRead(input_bit_1);
  input_bit_0_state = digitalRead(input_bit_0);


  if (horn_button_state != last_horn_button_state) {
    if (horn_button_state == LOW) {
      horn_button_pressed = true;
    }
    if (horn_button_state == HIGH) {
      horn_button_released = true;
    }
  }

  if (music_button_state != last_music_button_state) {
    if (music_button_state == LOW) {
      music_button_released = true;
    }
  }

  // if the state of the toggle switch changes arm the track name and adjust leds
  if (first_loop || input_bit_3_state != last_input_bit_3_state || input_bit_2_state != last_input_bit_2_state || input_bit_1_state != last_input_bit_1_state || input_bit_0_state != last_input_bit_0_state) {
    digitalWrite(led_bit_3, input_bit_3_state);
    digitalWrite(led_bit_2, input_bit_2_state);
    digitalWrite(led_bit_1, input_bit_1_state);
    digitalWrite(led_bit_0, input_bit_0_state);
    digitalWrite(led_bit_3_on, !input_bit_3_state);
    digitalWrite(led_bit_2_on, !input_bit_2_state);
    digitalWrite(led_bit_1_on, !input_bit_1_state);
    digitalWrite(led_bit_0_on, !input_bit_0_state);
    track_number = input_bit_3_state * 8 + input_bit_2_state * 4 + input_bit_1_state * 2 + input_bit_0_state;
    track_file_name = "/" + String(track_number) + ".raw";
  }

  // if the music is not yet playing and the button is pressed, play music
  if (music_button_released && !is_music_playing) {
    sound_file = SD.open(track_file_name, FILE_READ);
  }

  // if music is already playing and the button is pressed, play silence
  if (music_button_released && is_music_playing) {
    sound_file = SD.open(silence_file_name, FILE_READ);
  }

  // honk the horn when horn button is pressed
  if (horn_button_pressed) {
    sound_file = SD.open(horn_file_name, FILE_READ);
  }

  // silence when horn button in released
  if (horn_button_released) {
    sound_file = SD.open(silence_file_name, FILE_READ);
  }

  music_button_released = false;
  horn_button_pressed = false;
  horn_button_released = false;
  last_music_button_state = music_button_state;
  last_horn_button_state = horn_button_state;
  last_input_bit_3_state = input_bit_3_state;
  last_input_bit_2_state = input_bit_2_state;
  last_input_bit_1_state = input_bit_1_state;
  last_input_bit_0_state = input_bit_0_state;
  first_loop = false;
}