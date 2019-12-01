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

#include <cstdint>
#include <functional>

#include "registers.hpp"
#include "../scheduler.hpp"

namespace GameBoyAdvance {

class CPU;

class PPU {

public:
  PPU(CPU* cpu);

  void Reset();  
  void Tick();

  Event event { 0, [this]() { this->Tick(); } };
  
  struct MMIO {
    DisplayControl dispcnt;
    DisplayStatus dispstat;

    std::uint8_t vcount;
    
    BackgroundControl bgcnt[4];
    
    std::uint16_t bghofs[4];
    std::uint16_t bgvofs[4];
    
    ReferencePoint bgx[2], bgy[2];
    std::int16_t bgpa[2];
    std::int16_t bgpb[2];
    std::int16_t bgpc[2];
    std::int16_t bgpd[2];

    WindowRange winh[2];
    WindowRange winv[2];
    WindowLayerSelect winin;
    WindowLayerSelect winout;
    
    Mosaic mosaic;

    BlendControl bldcnt;
    int eva;
    int evb;
    int evy;
  } mmio;

private:
  enum class Phase {
    SCANLINE = 0,
    HBLANK = 1,
    VBLANK_SCANLINE = 2,
    VBLANK_HBLANK = 3
  };

  enum ObjAttribute {
    OBJ_IS_ALPHA  = 1,
    OBJ_IS_WINDOW = 2
  };

  enum ObjectMode {
    OBJ_NORMAL = 0,
    OBJ_SEMI   = 1,
    OBJ_WINDOW = 2,
    OBJ_PROHIBITED = 3
  };
  
  enum Layer {
    LAYER_BG0 = 0,
    LAYER_BG1 = 1,
    LAYER_BG2 = 2,
    LAYER_BG3 = 3,
    LAYER_OBJ = 4,
    LAYER_SFX = 5,
    LAYER_BD  = 5
  };
  
  enum Enable {
    ENABLE_BG0 = 0,
    ENABLE_BG1 = 1,
    ENABLE_BG2 = 2,
    ENABLE_BG3 = 3,
    ENABLE_OBJ = 4,
    ENABLE_WIN0 = 5,
    ENABLE_WIN1 = 6,
    ENABLE_OBJWIN = 7 
  };

  static auto ConvertColor(std::uint16_t color) -> std::uint32_t;
  
  void InitBlendTable();
  void SetNextEvent(Phase phase);
  void RenderScanline();
  void RenderLayerText(int id);
  void RenderLayerAffine(int id);
  void RenderLayerBitmap1();
  void RenderLayerBitmap2();
  void RenderLayerBitmap3();
  void RenderLayerOAM(bool bitmap_mode = false);
  void RenderWindow(int id);
  void ComposeScanline(int bg_min, int bg_max);
  void Blend(std::uint16_t& target1, std::uint16_t target2, BlendControl::Effect sfx);
  void OnScanlineComplete();
  void OnHblankComplete();
  void OnVblankScanlineComplete();
  void OnVblankHblankComplete();
  
  #include "helper.inl"

  CPU* cpu;

  std::uint8_t* pram;
  std::uint8_t* vram;
  std::uint8_t* oam;

  std::uint16_t buffer_bg[4][240];
  
  struct ObjectPixel {
    std::uint16_t color;
    std::uint8_t  priority;
    unsigned alpha  : 1;
    unsigned window : 1;
  } buffer_obj[240];
  
  bool line_contains_alpha_obj;
  
  bool buffer_win[2][240];
  bool window_scanline_enable[2];
  
  std::uint32_t output[240*160];
  
  Phase phase;

  std::uint8_t blend_table[17][17][32][32];

  static constexpr std::uint16_t s_color_transparent = 0x8000;
  static constexpr int s_wait_cycles[4] = { 1006, 226, 1006, 226 };
  static const int s_obj_size[4][4][2];
};

}