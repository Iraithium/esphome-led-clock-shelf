#include "led_clock_shelf.h"
#include <driver/adc.h>

namespace esphome {
namespace led_clock_shelf {

static const char *TAG = "led_clock_shelf";
static const uint8_t NONE = 255;

LEDClockShelf::Segment LEDClockShelf::segments[32] = {
  {0, 8, false},     // 0: Unused, OFF
  {9, 17, false},    // 1: Hour units, bottom
  {18, 26, false},   // 2: Colon, bottom (for dual style)
  {27, 35, false},   // 3: Minute tens, bottom
  {36, 44, false},   // 4: Unused, OFF
  {45, 53, false},   // 5: Minute units, bottom
  {54, 62, false},   // 6: Minute units, lower right
  {63, 71, false},   // 7: Minute units, upper right
  {72, 80, false},   // 8: Minute units, top
  {81, 89, false},   // 9: Unused, OFF
  {90, 98, false},   // 10: Minute tens, top
  {99, 107, false},  // 11: Colon, top (for dual style)
  {108, 116, false}, // 12: Hour units, top
  {117, 125, false}, // 13: Unused, OFF
  {126, 134, false}, // 14: Hour tens, top
  {135, 143, false}, // 15: Hour tens, bottom
  {144, 152, false}, // 16: Unused, OFF
  {153, 161, false}, // 17: Hour units, lower left
  {162, 170, false}, // 18: Hour units, upper left
  {171, 179, false}, // 19: Hour units, middle
  {180, 188, false}, // 20: Hour units, lower right
  {189, 197, false}, // 21: Hour units, upper right
  {198, 206, false}, // 22: Colon, middle (for single style)
  {207, 215, false}, // 23: Minute tens, lower left
  {216, 224, false}, // 24: Minute tens, upper left
  {225, 233, false}, // 25: Minute tens, middle
  {234, 242, false}, // 26: Minute tens, lower right
  {243, 251, false}, // 27: Minute tens, upper right
  {252, 260, false}, // 28: Unused, OFF
  {261, 269, false}, // 29: Minute units, lower left
  {270, 278, false}, // 30: Minute units, upper left
  {279, 287, false}, // 31: Minute units, middle
};

void LEDClockShelf::setup() {
  ESP_LOGI("led_clock_shelf", "Setting up LED Clock Shelf on GPIO%d with light sensor on GPIO%d...", pin_, light_sensor_pin_);
  leds_ = new CRGB[300];
  
  if (pin_ != 0) {
    ESP_LOGI("led_clock_shelf", "Using pin: %d", pin_);
    controller_ = &FastLED.addLeds<WS2812B, 21, GRB>(leds_, 300);
    fill_solid(leds_, 300, CRGB::Black);

    if (downlights_enabled_) for (int i = 288; i <= 299; i++) leds_[i] = downlight_color_;

    if (colon_style_ == 0) {
      set_led(103, CRGB::Yellow);
      set_led(22, CRGB::Yellow);
    } else set_led(202, CRGB::Yellow);

    FastLED.show();
    delay(2000);
    fill_solid(leds_, 300, CRGB::Black);
    FastLED.show();
    
    ESP_LOGI("led_clock_shelf", "Test LED set to red on GPIO%d, downlights initialized to %s, colon tested", pin_, downlights_enabled_ ? "on" : "off");
  } else ESP_LOGE("led_clock_shelf", "No LED pin configured or invalid pin!");

  current_brightness_ = brightness_;
  FastLED.setBrightness(current_brightness_);
  FastLED.setMaxRefreshRate(20);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setDither(0);

  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

  brightness_pref_ = global_preferences->make_preference<uint8_t>(123456789U);
  downlights_enabled_pref_ = global_preferences->make_preference<bool>(123456791U);
  colon_style_pref_ = global_preferences->make_preference<uint8_t>(123456792U);
  hour_color_pref_ = global_preferences->make_preference<uint8_t>(123456793U);
  minute_color_pref_ = global_preferences->make_preference<uint8_t>(123456794U);
  colon_color_pref_ = global_preferences->make_preference<uint8_t>(123456795U);
  downlight_color_pref_ = global_preferences->make_preference<uint8_t>(123456796U);

  load_settings();
  ESP_LOGI("led_clock_shelf", "Setup complete, LEDs and light sensor initialized.");

  set_interval(10, [this]() { this->update(); });
  set_interval(10, [this]() { this->update_colon(); });
  set_interval(5000, [this]() { this->take_brightness_measurement(); this->set_global_brightness_based_on_light(); });
  set_interval(10000, [this]() { if (light_sensor_) light_sensor_->publish_state(average_brightness_); });
  set_interval(100, [this]() { this->update_downlights(); });
}

void LEDClockShelf::loop() {
  // yield();
}

void LEDClockShelf::update() {
  if (!time_) {
    ESP_LOGE("led_clock_shelf", "Time not set, cannot update display");
    return;
  }

  if (display_enabled_) {    
    for (int i = 0; i <= 287; i++) leds_[i] = CRGB::Black;

    handle_time();
    set_colon(colon_state_ != 0);

  } else fill_solid(leds_, 300, CRGB::Black);

  transition_brightness();

  static uint32_t last_show = 0;
  uint32_t current_ms = millis();
  if (current_ms - last_show >= 100) {
    FastLED.setBrightness(current_brightness_);
    FastLED.show();
    last_show = current_ms;
  }
  // yield();
}

void LEDClockShelf::handle_time() {
  esphome::ESPTime now = time_->now();
  uint8_t hours = now.hour;
  uint8_t minutes = now.minute;
  uint8_t twelve_hour = (hours % 12 == 0) ? 12 : hours % 12;
  uint8_t hour_tens = (twelve_hour >= 10) ? 1 : 0;
  uint8_t hour_units = twelve_hour % 10;
  uint8_t minute_tens = minutes / 10;
  uint8_t minute_units = minutes % 10;

  if (hour_tens == 1) display_digit(1, 0, hour_color_);
  display_digit(hour_units, 1, hour_color_);
  display_digit(minute_tens, 2, minute_color_);
  display_digit(minute_units, 3, minute_color_);
}

void LEDClockShelf::update_colon() {
  if (!display_enabled_) {
    set_colon(false);
    colon_state_ = 0;
    colon_timer_ = millis();
    return;
  }

  uint32_t now_ms = millis();

  if (colon_state_ == 0) {
    if (now_ms - colon_timer_ >= 2000) {
      colon_state_ = 1;
      colon_timer_ = now_ms;
      set_colon(true);
    }
  } else if (colon_state_ == 1) {
    if (now_ms - colon_timer_ >= 500) {
      colon_state_ = 0;
      colon_timer_ = now_ms;
      set_colon(false);
    }
  }
  // yield();
}

void LEDClockShelf::set_colon(bool state) {
  if (colon_style_ == 2) {
    return;
  }

  CRGB color = state ? colon_color_ : CRGB::Black;
  if (colon_style_ == 0) {
    set_led(103, color);
    set_led(22, color);
  } else if (colon_style_ == 1) {
    set_led(202, color);
  }
  // yield();
}

void LEDClockShelf::update_downlights() {
  CRGB color = (display_enabled_ && downlights_enabled_) ? downlight_color_ : CRGB::Black;
  for (int i = 288; i <= 299; i++) {
    leds_[i] = color;
  }
  FastLED.show();
  // yield();
}

void LEDClockShelf::take_brightness_measurement() {
  const int NUM_READINGS = 10;
  uint32_t total = 0;

  for (int i = 0; i < NUM_READINGS; i++) {
    total += adc1_get_raw(ADC1_CHANNEL_6);
    delayMicroseconds(50);
  }

  average_brightness_ = total / NUM_READINGS;
}

void LEDClockShelf::set_global_brightness_based_on_light() {
  float sensor_value = static_cast<float>(average_brightness_);
  float darkness_factor = sensor_value / 4095.0f;  // 0.0 (bright) to 1.0 (dark)
  float reduction = darkness_factor * brightness_reduction_dark_;  // 0.0 to 0.85
  float target_brightness_float = brightness_ * (1.0f - reduction);
  target_brightness_ = static_cast<uint8_t>(std::max(0.0f, target_brightness_float));
  brightness_transition_start_ = millis();
}

void LEDClockShelf::transition_brightness() {
  if (current_brightness_ != target_brightness_) {
      uint32_t now = millis();
      uint32_t elapsed = now - brightness_transition_start_;
      if (elapsed >= brightness_transition_duration_) {
          current_brightness_ = target_brightness_;
      } else {
          float progress = static_cast<float>(elapsed) / brightness_transition_duration_;
          progress = pow(progress, 4.0f);  // Smooth transition curve
          float delta = static_cast<float>(target_brightness_ - current_brightness_);
          current_brightness_ = static_cast<uint8_t>(round(current_brightness_ + (delta * progress)));
      }
      FastLED.setBrightness(current_brightness_);
  }
}

void LEDClockShelf::display_digit(uint8_t digit, uint8_t position, CRGB color) {
  // The first digit can only display 1.
  if (position == 0 && digit != 1) return;

  bool segments[7];
  switch (digit) {
    case 0:
      segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = false;
      segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 1:
      segments[0] = false; segments[1] = false; segments[2] = true; segments[3] = false;
      segments[4] = false; segments[5] = true; segments[6] = false;
      break;
    case 2:
      segments[0] = true; segments[1] = false; segments[2] = true; segments[3] = true;
      segments[4] = true; segments[5] = false; segments[6] = true;
      break;
    case 3:
      segments[0] = true; segments[1] = false; segments[2] = true; segments[3] = true;
      segments[4] = false; segments[5] = true; segments[6] = true;
      break;
    case 4:
      segments[0] = false; segments[1] = true; segments[2] = true; segments[3] = true;
      segments[4] = false; segments[5] = true; segments[6] = false;
      break;
    case 5:
      segments[0] = true; segments[1] = true; segments[2] = false; segments[3] = true;
      segments[4] = false; segments[5] = true; segments[6] = true;
      break;
    case 6:
      segments[0] = true; segments[1] = true; segments[2] = false; segments[3] = true;
      segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 7:
      segments[0] = true; segments[1] = false; segments[2] = true; segments[3] = false;
      segments[4] = false; segments[5] = true; segments[6] = false;
      break;
    case 8:
      segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = true;
      segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 9:
      segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = true;
      segments[4] = false; segments[5] = true; segments[6] = true;
      break;
    default:
      for (int i = 0; i < 7; i++) segments[i] = false;
      return;
  }

  static const uint8_t seg_map[4][7] = {
    {NONE, NONE, 14, NONE, NONE, 15, NONE}, 
    {12, 18, 21, 19, 17, 20, 1},
    {10, 24, 27, 25, 23, 26, 3},
    {8, 30, 7, 31, 29, 6, 5},
  };

  for (int i = 0; i < 7; i++) {
    if (segments[i]) {
      uint8_t segment_idx = seg_map[position][i];
      if (segment_idx != NONE) {
        set_segment(LEDClockShelf::segments[segment_idx], color);
      }
    }
  }
}

void LEDClockShelf::set_segment(Segment segment, CRGB color) {
  if (segment.reverse) {
    for (int i = segment.endLed; i >= segment.startLed; i--) {
      leds_[i] = color;
    }
  } else {
    for (int i = segment.startLed; i <= segment.endLed; i++) {
      leds_[i] = color;
    }
  }
}

void LEDClockShelf::set_led(uint16_t index, CRGB color) {
  if (index < 300) {
    leds_[index] = color;
  }
}

void LEDClockShelf::load_settings() {
  uint8_t loaded_brightness = 128;
  bool loaded_downlights_enabled = false;
  uint8_t loaded_colon_style = 0;
  uint8_t loaded_hour_color_idx = 0;
  uint8_t loaded_minute_color_idx = 1;
  uint8_t loaded_colon_color_idx = 3;
  uint8_t loaded_downlight_color_idx = 4;

  if (brightness_pref_.load(&loaded_brightness)) {
    brightness_ = loaded_brightness;
    current_brightness_ = brightness_;
  }
  if (downlights_enabled_pref_.load(&loaded_downlights_enabled)) downlights_enabled_ = loaded_downlights_enabled;
  if (colon_style_pref_.load(&loaded_colon_style)) colon_style_ = loaded_colon_style;

  // Use custom colors if provided, otherwise fall back to defaults
  if (custom_colors_.empty()) {
    static const std::pair<const char*, CRGB> default_colors[] = {
        {"Red", CRGB::Red}, {"Green", CRGB::Green}, {"Blue", CRGB::Blue},
        {"Yellow", CRGB::Yellow}, {"White", CRGB::White},
        {"Orange", CRGB(255, 165, 0)}, {"Purple", CRGB(128, 0, 128)},
        {"Cyan", CRGB(0, 255, 255)}, {"Pink", CRGB(255, 105, 180)},
        {"Magenta", CRGB(255, 0, 255)}
    };
    custom_colors_.assign(default_colors, default_colors + sizeof(default_colors) / sizeof(default_colors[0]));
  }

  if (hour_color_pref_.load(&loaded_hour_color_idx) && loaded_hour_color_idx < custom_colors_.size()) {
    hour_color_idx_ = loaded_hour_color_idx;
    hour_color_ = custom_colors_[hour_color_idx_].second;
    if (hour_color_select_) hour_color_select_->publish_state(custom_colors_[hour_color_idx_].first);
  }
  if (minute_color_pref_.load(&loaded_minute_color_idx) && loaded_minute_color_idx < custom_colors_.size()) {
    minute_color_idx_ = loaded_minute_color_idx;
    minute_color_ = custom_colors_[minute_color_idx_].second;
    if (minute_color_select_) minute_color_select_->publish_state(custom_colors_[minute_color_idx_].first);
  }
  if (colon_color_pref_.load(&loaded_colon_color_idx) && loaded_colon_color_idx < custom_colors_.size()) {
    colon_color_idx_ = loaded_colon_color_idx;
    colon_color_ = custom_colors_[colon_color_idx_].second;
    if (colon_color_select_) colon_color_select_->publish_state(custom_colors_[colon_color_idx_].first);
  }
  if (downlight_color_pref_.load(&loaded_downlight_color_idx) && loaded_downlight_color_idx < custom_colors_.size()) {
    downlight_color_idx_ = loaded_downlight_color_idx;
    downlight_color_ = custom_colors_[downlight_color_idx_].second;
    if (downlight_color_select_) downlight_color_select_->publish_state(custom_colors_[downlight_color_idx_].first);
  }

  FastLED.setBrightness(current_brightness_);
}

void LEDClockShelf::save_settings() {
  ESP_LOGI("led_clock_shelf", "Saving settings...");
  brightness_pref_.save(&brightness_);
  downlights_enabled_pref_.save(&downlights_enabled_);
  colon_style_pref_.save(&colon_style_);
  hour_color_pref_.save(&hour_color_idx_);
  minute_color_pref_.save(&minute_color_idx_);
  colon_color_pref_.save(&colon_color_idx_);
  downlight_color_pref_.save(&downlight_color_idx_);
  ESP_LOGI("led_clock_shelf",
           "Saved settings: hour=%d, minute=%d, colon=%d, downlight=%d",
           hour_color_idx_, minute_color_idx_, colon_color_idx_, downlight_color_idx_);
}

}  // namespace led_clock_shelf
}  // namespace esphome