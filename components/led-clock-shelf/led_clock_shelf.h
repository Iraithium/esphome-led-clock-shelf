#pragma once
#include "esphome.h"
#include <FastLED.h>
#include <esphome/components/time/real_time_clock.h>
#include <esphome/components/number/number.h>
#include <esphome/components/select/select.h>
#include <esphome/components/switch/switch.h>
#include <esphome/components/sensor/sensor.h>

namespace esphome {
namespace led_clock_shelf {

class LEDClockShelf : public Component {
 public:
  void setup() override;
  void loop() override;
  void update();
  void set_pin(uint8_t pin) { pin_ = pin; }
  void set_hour_color(const std::string &color);
  void set_minute_color(const std::string &color);
  void set_colon_color(const std::string &color);
  void set_downlight_color(const std::string &color);    
  void set_colon_style(uint8_t style) { colon_style_ = style; save_settings(); }
    
  void set_time(time::RealTimeClock *time) { time_ = time; }
  void set_downlights_enabled(bool enabled) { downlights_enabled_ = enabled; save_settings(); }
  void set_light_sensor_pin(uint8_t pin) { light_sensor_pin_ = pin; }
  void set_brightness_reduction_dark(float percentage) { brightness_reduction_dark_ = percentage / 100.0f; }
  void set_brightness_transition_duration(uint32_t duration_ms) { brightness_transition_duration_ = duration_ms; }
  void update_downlights();
  void handle_time();
  void set_colon(bool state);
  void update_colon();
  void save_settings();
  void take_brightness_measurement();
  void set_global_brightness_based_on_light();
  void set_display_enabled(bool enabled) { display_enabled_ = enabled; }
  void set_default_brightness(uint8_t brightness) { brightness_ = brightness; save_settings(); }
  uint8_t get_brightness() const { return brightness_; }
  uint8_t get_colon_style() const { return colon_style_; }
  bool get_downlights_enabled() const { return downlights_enabled_; }
  void set_hour_color_select(select::Select *sel) { hour_color_select_ = sel; }
  void set_minute_color_select(select::Select *sel) { minute_color_select_ = sel; }
  void set_colon_color_select(select::Select *sel) { colon_color_select_ = sel; }
  void set_colon_style_select(select::Select *sel) { colon_style_select_ = sel; }
  void set_downlight_color_select(select::Select *sel) { downlight_color_select_ = sel; }
  void set_custom_colors(const std::vector<std::pair<std::string, CRGB>>& colors) { custom_colors_ = colors; }
  float get_average_brightness() const { return average_brightness_; }
  void set_light_sensor(sensor::Sensor *sensor) { light_sensor_ = sensor; }

 private:
  struct Segment {
    uint16_t startLed;
    uint16_t endLed;
    bool reverse;
  };

  static Segment segments[32];
  uint8_t pin_ = 21;
  bool display_enabled_ = true;
  bool downlights_enabled_ = false;
  uint8_t light_sensor_pin_ = 34;
  uint8_t brightness_ = 128;
  uint8_t target_brightness_ = 128;
  uint8_t current_brightness_ = 128;
  uint16_t average_brightness_ = 0;
  float brightness_reduction_dark_ = 0.5f;
  uint32_t brightness_transition_start_ = 0;
  uint32_t brightness_transition_duration_ = 6000;
  uint8_t colon_style_ = 0;
  uint8_t colon_state_ = 0;
  uint32_t colon_timer_ = 0;
   
  // Custom colors
  uint8_t hour_color_idx_ = 0;
  uint8_t minute_color_idx_ = 1;
  uint8_t colon_color_idx_ = 3;
  uint8_t downlight_color_idx_ = 4;
  
  CRGB *leds_;
  CLEDController *controller_;
  time::RealTimeClock *time_{nullptr};
  select::Select *hour_color_select_{nullptr};
  select::Select *minute_color_select_{nullptr};
  select::Select *colon_color_select_{nullptr};
  select::Select *downlight_color_select_{nullptr};
  select::Select *colon_style_select_{nullptr};
  sensor::Sensor *light_sensor_{nullptr};
  
  ESPPreferenceObject brightness_pref_;
  ESPPreferenceObject downlights_enabled_pref_;
  ESPPreferenceObject colon_style_pref_;
  ESPPreferenceObject hour_color_pref_;
  ESPPreferenceObject minute_color_pref_;
  ESPPreferenceObject colon_color_pref_;
  ESPPreferenceObject downlight_color_pref_;
  
  std::vector<std::pair<std::string, CRGB>> custom_colors_;
  
  CRGB hour_color_ = CRGB::Red;
  CRGB minute_color_ = CRGB::Green;
  CRGB colon_color_ = CRGB::Yellow;
  CRGB downlight_color_ = CRGB::White;
  
  void set_segment(Segment segment, CRGB color);
  void set_led(uint16_t index, CRGB color);
  void load_settings();
  void display_digit(uint8_t digit, uint8_t position, CRGB color);
  void transition_brightness();
};

// Updated set_color template to use custom colors
template <typename T>
void set_color(T &color, uint8_t &idx, const std::string &value,
               select::Select *sel, const std::vector<std::pair<std::string,
               CRGB>> &custom_colors) {
  for (uint8_t i = 0; i < custom_colors.size(); i++) {
    if (value == custom_colors[i].first) {
      color = custom_colors[i].second;
      idx = i;
      if (sel) sel->publish_state(value);
      return;
    }
  }
}

inline void LEDClockShelf::set_hour_color(const std::string &color) {
  set_color(hour_color_, hour_color_idx_, color, hour_color_select_, custom_colors_);
  save_settings();
}

inline void LEDClockShelf::set_minute_color(const std::string &color) {
  set_color(minute_color_, minute_color_idx_, color, minute_color_select_, custom_colors_);
  save_settings();
}

inline void LEDClockShelf::set_colon_color(const std::string &color) {
  set_color(colon_color_, colon_color_idx_, color, colon_color_select_, custom_colors_);
  save_settings();
  set_colon(true);
}

inline void LEDClockShelf::set_downlight_color(const std::string &color) {
  set_color(downlight_color_, downlight_color_idx_, color, downlight_color_select_, custom_colors_);
  save_settings();
}

class LEDClockDisplaySwitch : public switch_::Switch, public Component {
 public:
  void setup() override {
    state_ = true;
    publish_state(state_);
  }
  void write_state(bool state) override {
    state_ = state;
    if (led_clock_) {
      led_clock_->set_display_enabled(state);
    }
    publish_state(state);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }
 protected:
  LEDClockShelf *led_clock_{nullptr};
  bool state_;
};

class LEDClockDownlightsSwitch : public switch_::Switch, public Component {
 public:
  void setup() override {
    if (led_clock_) {
      state = led_clock_->get_downlights_enabled();
      publish_state(state);
    }
  }
  void write_state(bool state) override {
    if (led_clock_) {
      led_clock_->set_downlights_enabled(state);
    }
    publish_state(state);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }
 protected:
  LEDClockShelf *led_clock_{nullptr};
  bool state;
};

class LEDClockBrightnessNumber : public number::Number, public Component {
 public:
  void setup() override {
    if (led_clock_) {
      publish_state(led_clock_->get_brightness());
    }
  }
  void control(float value) override {
    if (led_clock_) {
      led_clock_->set_default_brightness(static_cast<uint8_t>(value));
    }
    publish_state(value);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }
 protected:
  LEDClockShelf *led_clock_{nullptr};
};

class LEDClockColonStyleSelect : public select::Select, public Component {
 public:
  void setup() override {
    if (led_clock_) {
      uint8_t style = led_clock_->get_colon_style();
      publish_state(style == 0 ? "Dual" : style == 1 ? "Single" : "None");
    }
  }
  void control(const std::string &value) override {
    if (led_clock_) {
      if (value == "Dual") led_clock_->set_colon_style(0);
      else if (value == "Single") led_clock_->set_colon_style(1);
      else if (value == "None") led_clock_->set_colon_style(2);
    }
    publish_state(value);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }
 protected:
  LEDClockShelf *led_clock_{nullptr};
};

class LEDClockHourColorSelect : public select::Select, public Component {
 public:
  void control(const std::string &value) override {
    this->publish_state(value);
    if (led_clock_) led_clock_->set_hour_color(value);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }

 protected:
  LEDClockShelf *led_clock_{nullptr};
};

class LEDClockMinuteColorSelect : public select::Select, public Component {
 public:
  void control(const std::string &value) override {
    this->publish_state(value);
    if (led_clock_) led_clock_->set_minute_color(value);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }

 protected:
  LEDClockShelf *led_clock_{nullptr};
};

class LEDClockColonColorSelect : public select::Select, public Component {
 public:
  void control(const std::string &value) override {
    this->publish_state(value);
    if (led_clock_) led_clock_->set_colon_color(value);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }

 protected:
  LEDClockShelf *led_clock_{nullptr};
};

class LEDClockDownlightColorSelect : public select::Select, public Component {
 public:
  void control(const std::string &value) override {
    this->publish_state(value);
    if (led_clock_) led_clock_->set_downlight_color(value);
  }
  void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }

 protected:
  LEDClockShelf *led_clock_{nullptr};
};

class LEDClockLightSensor : public sensor::Sensor, public Component {
  public:
   void setup() {}
   void set_led_clock(LEDClockShelf *led_clock) { led_clock_ = led_clock; }
   void update() {
     if (led_clock_) publish_state(led_clock_->get_average_brightness());
   }
   float get_average_brightness() const { return led_clock_ ? led_clock_->get_average_brightness() : 0; }
 
  protected:
   LEDClockShelf *led_clock_{nullptr};
 };

}  // namespace led_clock_shelf
}  // namespace esphome