#include "ppu_reg.h"

void ppu_reg::control_push(uint8_t data)
{
	address.raw &= ~0x0C00;
	address.raw |= ((uint16_t)(data << 10) & 0x0C00);
}

void ppu_reg::scrollX_push(uint8_t data)
{
	scrollX.raw = data;
	address.coarseX = scrollX.coarse;
}

void ppu_reg::scrollY_push(uint8_t data)
{
	scrollY.raw = data;
	address.coarseY = scrollY.coarse;
}

void ppu_reg::addressLO_push(uint8_t data)
{
	address.lo = data;
}

void ppu_reg::addressHI_push(uint8_t data)
{
	address.hi = (data & 0x3F);
}

void ppu_reg::increment_address(bool increment_mode)
{
	address.raw += increment_mode ? 32 : 1;
}

void ppu_reg::raw_copy(uint16_t data)
{
	address.raw = data;
}

uint16_t ppu_reg::raw()
{
	return address.raw;
}