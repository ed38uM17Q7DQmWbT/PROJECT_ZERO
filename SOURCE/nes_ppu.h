#pragma once
#include <vector>
#include <memory>
#include <iostream>

#include "ppu_reg.h"
#include "nes_crt.h"


class nes_ppu
{
public:
	nes_ppu();
	~nes_ppu();

	class pixel_rgb
	{
	public:
		uint8_t red;
		uint8_t grn;
		uint8_t blu;
	};
	//std::array<pixel_rgb, 256 * 240> nameA;
	//std::array<pixel_rgb, 256 * 240> nameB;
	//std::array<pixel_rgb, 256 * 240> nameC;
	//std::array<pixel_rgb, 256 * 240> nameD;
	//std::array<pixel_rgb, 128 * 128> pattA;
	//std::array<pixel_rgb, 128 * 128> pattB;

	nes_crt* system_cartridge = nullptr;
	uint8_t name_mem[0x0800];
	uint8_t pltt_mem[0x0020];
	bool frame_complete = false;

	class oam_entry
	{
	public:
		uint8_t ypos;
		uint8_t tile;
		uint8_t attr;
		uint8_t xpos;
	};

	enum attribute_flags : uint8_t
	{
		plttlo = 0x01,
		pltthi = 0x02,
		attrpadA = 0x04,
		attrpadB = 0x08,
		attrpadC = 0x10,
		priority = 0x20,	// (false: infront of bg,	true: behind bg)
		hflip = 0x40,
		vflip = 0x80
	};
	// convenience enumeration into OAM entry attribute string

	oam_entry	ppu_oamA[64];
	uint8_t* oamA_ptr = (uint8_t*)ppu_oamA;
	oam_entry	ppu_oamB[8];
	uint8_t* oamB_ptr = (uint8_t*)ppu_oamB;
	uint8_t		ppu_ctrl = 0x00;
	uint8_t		ppu_mask = 0x00;
	uint8_t		ppu_stat = 0x00;
	uint8_t		oam_addr = 0x00;
	uint8_t		oam_data = 0x00;
	uint8_t		ppu_scrl = 0x00;
	uint8_t		ppu_addr = 0x00;
	uint8_t		ppu_data = 0x00;
	// internal registers

	enum control_flags : uint8_t
	{
		nameX = 0x01,				// base nametable address			{0: $2000,		1: $2400,	}
		nameY = 0x02,				// base nametable address			{2: $2800,		3: $2C00	}
		increment_mode = 0x04,		// patterntable increment			(false: +1,		true: +32	)
		pattern_foreground = 0x08,	// foreground patterntable offset	(false: $0000,	true: $1000	)
		pattern_background = 0x10,	// background patterntable offset	(false: $0000,	true: $1000	)
		sprite_size = 0x20,			// sprite size select				(false: 8*8,	true: 8*16	)
		slave_mode = 0x40,			// PPU master/slave select
		enable_nmi = 0x80			// generate NMI at vblank			(false: false,	true: true	)
	};
	// convenience enumeration to index into the control register

	enum mask_flags : uint8_t
	{
		greyscale = 0x01,	// produce greyscale display
		bg_fadein = 0x02,	// render background in leftmost 8 pixels of frame
		fg_fadein = 0x04,	// render foreground in leftmost 8 pixels of frame
		render_bg = 0x08,	// render background
		render_fg = 0x10,	// render foreground
		empha_red = 0x20,	// emphasize red
		empha_grn = 0x40,	// emphasize green
		empha_blu = 0x80	// emphasize blue
	};
	// convenience enumeration to index into the mask register

	enum status_flags : uint8_t
	{
		statpadA = 0x01,
		statpadB = 0x02,
		statpadC = 0x04,
		statpadD = 0x08,
		statpadE = 0x10,
		sprite_overflow = 0x20,
		sprite_zero_hit = 0x40,
		vblank = 0x80
	};
	// convenience enumeration to index into the status register

	bool first_write = true;
	uint8_t ppu_data_buffer = 0x00;
	ppu_reg rendering_frame;
	ppu_reg rendering_cycle;
	uint16_t cycl_ctr = 0;
	uint16_t scan_ctr = 0;

	void dati(uint16_t addr, uint8_t data);
	void dato(uint16_t addr, uint8_t* data);
	void read(uint16_t addr, uint8_t* data);
	void writ(uint16_t addr, uint8_t data);
	void clock();
	bool nmirq = false;
};