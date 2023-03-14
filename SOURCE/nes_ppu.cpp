#include "nes_ppu.h"

nes_ppu::nes_ppu() {}
nes_ppu::~nes_ppu() {}

void nes_ppu::dati(uint16_t addr, uint8_t data)
{
	addr %= 0x0008;
	switch (addr)
	{
	case 0x0000:	//ppu_ctrl
		ppu_ctrl = data;
		rendering_frame.control_push(data);
		break;
	case 0x0001:	//ppu_mask
		ppu_mask = data;
		break;
	case 0x0002:	//ppu_stat
		//no write access
		break;
	case 0x0003:	//oam_addr
		oam_addr = data;
		break;
	case 0x0004:	//oam_data
		oamA_ptr[oam_addr] = data;
		break;
	case 0x0005:	//ppu_scrl
		if (first_write)
		{
			rendering_frame.scrollX_push(data);
			first_write = false;
		}
		else
		{
			rendering_frame.scrollY_push(data);
			first_write = true;
		}
		break;
	case 0x0006:	//ppu_addr
		if (first_write)
		{
			rendering_frame.addressHI_push(data);
			first_write = false;
		}
		else
		{
			rendering_frame.addressLO_push(data);
			rendering_cycle.raw_copy(rendering_frame.raw());
			first_write = true;
		}
		break;
	case 0x0007:	//ppu_data
		writ(rendering_cycle.raw(), data);
		rendering_cycle.increment_address(ppu_ctrl & increment_mode);
		break;
	}
}

void nes_ppu::dato(uint16_t addr, uint8_t* data)
{
	addr %= 0x0008;
	switch (addr)
	{
	case 0x0000:	//ppu_ctrl
		//no read access
		break;
	case 0x0001:	//ppu_mask
		//no read access
		break;
	case 0x0002:	//ppu_stat
		*data = ppu_stat;
		ppu_stat &= ~vblank;
		first_write = true;
		break;
	case 0x0003:	//oam_addr
		//no read access
		break;
	case 0x0004:	//oam_data
		*data = oamA_ptr[oam_addr];
		break;
	case 0x0005:	//ppu_scrl
		//no read access
		break;
	case 0x0006:	//ppu_addr
		//no read access
		break;
	case 0x0007:	//ppu_data
		*data = ppu_data_buffer;
		read(rendering_cycle.raw(), &ppu_data_buffer);
		if (rendering_cycle.raw() >= 0x3F00)
			*data = ppu_data_buffer;
		rendering_cycle.increment_address(ppu_ctrl & increment_mode);
		break;
	}
}

void nes_ppu::read(uint16_t addr, uint8_t* data)
{
	addr %= 0x4000;
	if (addr >= 0x0000 && addr <= 0x1FFF)	//ptrn_mem
	{
		*data = system_cartridge->chr[addr];
		return;
	}
	if (addr >= 0x2000 && addr <= 0X3EFF)	//name_mem
	{
		addr -= 0x2000;
		addr %= 0x0800;
		*data = name_mem[addr];
		return;
	}
	if (addr >= 0x3F00 && addr <= 0X3FFF)	//pltt_mem
	{
		addr -= 0x3F00;
		addr %= 0x0020;
		if (addr % 4 == 0)
			addr = 0x0000;
		*data = pltt_mem[addr];
		return;
	}
}

void nes_ppu::writ(uint16_t addr, uint8_t data)
{
	addr %= 0x4000;
	if (addr >= 0x0000 && addr <= 0x1FFF)	//ptrn_mem
	{
		//no write access
		return;
	}
	if (addr >= 0x2000 && addr <= 0X3EFF)	//name_mem
	{
		addr -= 0x2000;
		addr %= 0x0800;
		name_mem[addr] = data;
		return;
	}
	if (addr >= 0x3F00 && addr <= 0X3FFF)	//pltt_mem
	{
		addr -= 0x3F00;
		addr %= 0x0020;
		if (addr % 4 == 0)
			addr = 0x0000;
		pltt_mem[addr] = data;
		return;
	}
}

void nes_ppu::clock()
{
	if (scan_ctr == 0 && cycl_ctr == 1)
	{ 
		//print screen
		frame_complete = true;
		ppu_stat &= ~vblank;
		ppu_stat &= ~sprite_zero_hit;
		ppu_stat &= ~sprite_overflow;
	}
	if (scan_ctr == 1 && cycl_ctr == 0)
	{
		//broken
	}
	if (scan_ctr == 242 && cycl_ctr == 1)
	{
		ppu_stat |= vblank;
		if (ppu_ctrl & enable_nmi)
			nmirq = true;
	}

	cycl_ctr++;
	if (cycl_ctr > 340)
	{
		cycl_ctr = 0;
		scan_ctr++;
		if (scan_ctr > 261)
			scan_ctr = 0;
	}
}