#pragma once
#include <cstdint>

class ppu_reg
{
public:

	union address
	{
		struct
		{
			uint16_t coarseX	: 5;
			uint16_t coarseY	: 5;
			uint16_t nametableX	: 1;
			uint16_t nametableY	: 1;
			uint16_t unused		: 4;
		};
		struct
		{
			uint16_t lo : 8;
			uint16_t hi : 8;
		};
		uint16_t raw = 0x0000;
	};
	address address;

	union scroll
	{
		struct
		{
			uint8_t fine	: 3;
			uint8_t coarse	: 5;
		};
		uint8_t raw = 0x00;
	};
	scroll scrollX;
	scroll scrollY;

	void control_push(uint8_t data);
	void scrollX_push(uint8_t data);
	void scrollY_push(uint8_t data);
	void addressLO_push(uint8_t data);
	void addressHI_push(uint8_t data);
	void increment_address(bool increment_mode);
	void raw_copy(uint16_t data);
	uint16_t raw();
};