#include "nes_crt.h"

nes_crt::nes_crt()
{
	std::ifstream ifstream;
	ifstream.open("C:\\Users\\owenh\\OneDrive\\Desktop\\ROMs\\TEST.nes", std::ifstream::binary);
	if (ifstream.is_open())
	{
		ifstream.read((char*)hdr.data(), hdr.size());
		ifstream.read((char*)prg.data(), prg.size());
		ifstream.read((char*)chr.data(), chr.size());
	}
	else
		failure = true;
}

nes_crt::~nes_crt() {}