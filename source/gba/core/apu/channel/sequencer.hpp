/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <algorithm>

#include "../../scheduler.hpp"

namespace GameBoyAdvance {

class Envelope {
public:
  void Reset() {
    direction = Direction::Decrement;
    initial_volume = 0;
    divider = 0;
    Restart();
  }
  
  void Restart() {
    current_volume = initial_volume;
    step = 0;
    if (enabled) {
      active = true;
    }
  }
  
  void Tick() {
    if (!active || divider == 0) return;
    
    /* TODO: this does not seem to behave quite right yet. */
    if (step == (divider - 1)) {
      int new_volume = current_volume;
      
      if (direction == Direction::Increment) { 
        new_volume++;
      } else {
        new_volume--;
      }
      
      if (new_volume >= 0 && new_volume <= 15) {
        current_volume = new_volume;
      } else {
        active = false;
      }
      
      step = 0;
    } else {
      step++;
    }
  }
  
  bool active = false;
  bool enabled = false;
  
  enum Direction {
    Increment = 1,
    Decrement = 0
  } direction;
  
  int initial_volume;
  int current_volume;
  int divider;
  
private:
  int step;
};

class Sweep {
public:
  void Reset() { 
    direction = Direction::Increment;
    initial_freq = 0;
    divider = 0;
    shift = 0;
    Restart();
  }
  
  void Restart() {
    channel_disabled = false;
    
    /* TODO: If the sweep shift is non-zero, frequency calculation and the
     * overflow check are performed immediately.
     */
    if (enabled) {
      current_freq = initial_freq;
      shadow_freq = initial_freq;
      step = 0;
      active = shift != 0 || divider != 0;
    }
  }
  
  void Tick() {
    if (!active || divider == 0) return;
    
    if (step == (divider - 1)) {
      int new_freq;
      int offset = shadow_freq >> shift;
      
      if (direction == Direction::Increment) {
        new_freq = shadow_freq + offset;
      } else {
        new_freq = shadow_freq - offset;
      }
      
      if (new_freq >= 2048) {
        channel_disabled = true;
      } else if (shift != 0) {
        shadow_freq  = new_freq;
        current_freq = new_freq;
      }
      
      /* TODO: then frequency calculation and overflow check are run AGAIN immediately
       * using this new value, but this second new frequency is not written back.
       */
      step = 0;
    } else {
      step++;
    }
  }
  
  bool active = false;
  bool enabled = false;
  bool channel_disabled = false;
  
  enum Direction {
    Increment = 0,
    Decrement = 1
  } direction;
  
  int initial_freq;
  int current_freq;
  int shadow_freq;
  int divider;
  int shift;

private:
  int step;
};

class Sequencer {
public:
  Sequencer() { Reset(); }
    
  void Reset() {
    length = 0;
    envelope.Reset();
    sweep.Reset();
    
    step = 0;
    event.countdown = s_cycles_per_step;
  }
  
  void Restart() {
    if (length == 0) {
      length = length_default;
    }
    sweep.Restart();
    envelope.Restart();
    step = 0;
  }
  
  void Tick() {
    // http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer
    switch (step) {
      case 0: length--; break;
      case 1: break;
      case 2: length--; sweep.Tick(); break;
      case 3: break;
      case 4: length--; break;
      case 5: break;
      case 6: length--; sweep.Tick(); break;
      case 7: envelope.Tick(); break;
    }
    
//    /* TODO: find a better way to handle this. */
//    if (length < 0) {
//      length = 0;
//    }
    
    step = (step + 1) % 8;
    
    event.countdown += s_cycles_per_step;
  }
  
  Event event { 0, [this]() { this->Tick(); } };
  
  int length;
  int length_default = 64;
  Envelope envelope;
  Sweep sweep;
  
private:
  int step;
  
  static constexpr cycle_t s_cycles_per_step = 16777216/512;
};

}