#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <array>

class nes_crt
{
public:
	nes_crt();
	~nes_crt();

	bool failure = false;
	std::array<uint8_t, 16> hdr;
	std::array<uint8_t, 0x4000> prg;
	std::array<uint8_t, 0x2000> chr;
};