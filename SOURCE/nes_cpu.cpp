#include "nes_cpu.h"

nes_cpu::nes_cpu(nes_crt* cartridge)
{
	my_crt = cartridge;
	my_ppu.system_cartridge = cartridge;
}
nes_cpu::~nes_cpu() {}

/*
------------------------------------------------------------------------------------------------------------------------------------------------------------------external event functions
*/

void nes_cpu::read(uint16_t addr, uint8_t* data)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		*data = my_ram[(addr - 0x0000) % 0x0800];
		return;
	}
	if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		my_ppu.dato(addr, data);
		return;
	}
	if (addr >= 0x4016 && addr <= 0x4017)
	{
		*data = (controllerA_state & 0x80) > 0;
		controllerA_state <<= 1;
		return;
	}
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		*data = my_crt->prg[(addr - 0x8000) % 0x4000];
		return;
	}
}


void nes_cpu::write(uint16_t addr, uint8_t data)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		my_ram[(addr - 0x0000) % 0x0800] = data;
		return;
	}
	if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		my_ppu.dati(addr, data);
		return;
	}
	if (addr >= 0x4016 && addr <= 0x4017)
	{
		controllerA_state = controllerA;
		return;
	}
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		my_crt->prg[(addr - 0x8000) % 0x4000] = data;
		return;
	}
}


void nes_cpu::reset()
{
	addr = 0xFFFC;
	uint8_t lo;
	read(addr + 0, &lo);
	uint8_t hi;
	read(addr + 1, &hi);


	cpu_prgc = hi;
	cpu_prgc <<= 8;
	cpu_prgc |= lo;


	cpu_accu = 0x00;
	cpu_xreg = 0x00;
	cpu_yreg = 0x00;
	cpu_stkp = 0xFD;
	cpu_stat = 0x34;


	addr = 0x0000;
	brch = 0x00;
	data = 0x00;
	tick = 8;
}


void nes_cpu::intrq()
{
	if ((cpu_stat & trpt) == 0x00)
	{
		write(0x0100 + cpu_stkp, (cpu_prgc >> 8) & 0x00FF);
		cpu_stkp--;
		write(0x0100 + cpu_stkp, (cpu_prgc >> 0) & 0x00FF);
		cpu_stkp--;


		cpu_stat &= ~brek;
		cpu_stat |= dead;
		cpu_stat |= trpt;
		write(0x0100 + cpu_stkp, cpu_stat);
		cpu_stkp--;


		addr = 0xFFFE;
		uint8_t lo;
		read(addr + 0, &lo);
		uint8_t hi;
		read(addr + 1, &hi);


		cpu_prgc = hi;
		cpu_prgc <<= 8;
		cpu_prgc |= lo;


		tick = 7;
	}
}


void nes_cpu::nmirq()
{
	write(0x0100 + cpu_stkp, (cpu_prgc >> 8) & 0x00FF);
	cpu_stkp--;
	write(0x0100 + cpu_stkp, (cpu_prgc >> 0) & 0x00FF);
	cpu_stkp--;


	cpu_stat &= ~brek;
	cpu_stat |= dead;
	cpu_stat |= trpt;
	write(0x0100 + cpu_stkp, cpu_stat);
	cpu_stkp--;


	addr = 0xFFFA;
	uint8_t lo;
	read(addr + 0, &lo);
	uint8_t hi;
	read(addr + 1, &hi);


	cpu_prgc = hi;
	cpu_prgc <<= 8;
	cpu_prgc |= lo;


	tick = 8;
}


void nes_cpu::clock()
{
	my_ppu.clock();
	if (nSystemClockCounter % 3 == 0)
	{
		if (tick == 0)
		{
			read(cpu_prgc, &opcd);
			cpu_stat |= dead;
			cpu_prgc++;
			tick = opcode_set[opcd].ticks;
			bool addrmd_return = (this->*opcode_set[opcd].addrmd)();
			bool nstrct_return = (this->*opcode_set[opcd].nstrct)();
			if (addrmd_return && nstrct_return)
				tick++;
			cpu_stat |= dead;
		}
		tick--;
	}

	if (my_ppu.nmirq)
	{
		my_ppu.nmirq = false;
		nmirq();
	}

	nSystemClockCounter++;
}


uint8_t nes_cpu::get_flag(status_flags flag)
{
	return ((cpu_stat & flag) > 0) ? 1 : 0;
}


void nes_cpu::set_flag(status_flags flag, bool value)
{
	if (value)
		cpu_stat |= flag;
	else
		cpu_stat &= ~flag;
}


void nes_cpu::fetch()
{
	if (!(opcode_set[opcd].addrmd == &nes_cpu::IMP))
		read(addr, &data);
}


/*
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------adressing modes
*/


// addressing mode: implied
bool nes_cpu::IMP()
{
	data = cpu_accu;
	return false;
}


// addressing mode: immediate
bool nes_cpu::IMM()
{
	addr = cpu_prgc++;
	return false;
}


// addressing mode: zero page
bool nes_cpu::ZP0()
{
	uint8_t temp;
	read(cpu_prgc, &temp);
	cpu_prgc++;
	addr = temp;
	addr &= 0x00FF;
	return false;
}


// addressing mode: zero page with x offset
bool nes_cpu::ZPX()
{
	uint8_t temp;
	read(cpu_prgc, &temp);
	cpu_prgc++;
	addr = temp + cpu_xreg;
	addr &= 0x00FF;
	return false;
}


// addressing mode: zero page with y offset
bool nes_cpu::ZPY()
{
	uint8_t temp;
	read(cpu_prgc, &temp);
	cpu_prgc++;
	addr = temp + cpu_yreg;
	addr &= 0x00FF;
	return false;
}


// addressing mode: relative
bool nes_cpu::REL()
{
	uint8_t temp = 0x00;
	read(cpu_prgc, &temp);

	brch = temp;
	cpu_prgc++;
	if (brch & 0x80)
		brch |= 0xFF00;
	return false;
}


// addressing mode: absolute
bool nes_cpu::ABS()
{
	uint8_t lo;
	read(cpu_prgc, &lo);
	cpu_prgc++;
	uint8_t hi;
	read(cpu_prgc, &hi);
	cpu_prgc++;
	addr = hi;
	addr <<= 8;
	addr |= lo;
	return false;
}


// addressing mode: absolute with x offset
bool nes_cpu::ABX()
{
	uint8_t lo;
	read(cpu_prgc, &lo);
	cpu_prgc++;
	uint8_t hi;
	read(cpu_prgc, &hi);
	cpu_prgc++;


	addr = hi;
	addr <<= 8;
	addr |= lo;
	addr += cpu_xreg;


	if ((addr >> 8) != hi)
		return true;
	else
		return false;
}


// addressing mode: absolute with y offset
bool nes_cpu::ABY()
{
	uint8_t lo;
	read(cpu_prgc, &lo);
	cpu_prgc++;
	uint8_t hi;
	read(cpu_prgc, &hi);
	cpu_prgc++;


	addr = hi;
	addr <<= 8;
	addr |= lo;
	addr += cpu_yreg;


	if ((addr >> 8) != hi)
		return true;
	else
		return false;
}


// addressing mode: indirect
bool nes_cpu::IND()
{
	uint8_t lo;
	read(cpu_prgc, &lo);
	cpu_prgc++;
	uint8_t hi;
	read(cpu_prgc, &hi);
	cpu_prgc++;


	uint16_t temp;
	temp = hi;
	temp <<= 8;
	temp |= lo;


	if (lo == 0xFF)
	{
		read(temp, &lo);
		read(temp & 0xFF00, &hi);
		addr = hi;
		addr <<= 8;
		addr |= lo;
	}
	else
	{
		read(temp + 0, &lo);
		read(temp + 1, &hi);
		addr = hi;
		addr <<= 8;
		addr |= lo;
	}


	return false;
}


// addressing mode: indirect x
bool nes_cpu::IZX()
{
	uint8_t temp;
	read(cpu_prgc, &temp);
	cpu_prgc++;
	uint16_t brkn = temp;

	uint8_t lo;
	read((brkn + (uint16_t)cpu_xreg + 0) & 0x00FF, &lo);
	uint8_t hi;
	read((brkn + (uint16_t)cpu_xreg + 1) & 0x00FF, &hi);


	addr = hi;
	addr <<= 8;
	addr |= lo;


	return false;
}


// addressing mode: indirect y
bool nes_cpu::IZY()
{
	uint8_t temp;
	read(cpu_prgc, &temp);
	cpu_prgc++;
	uint16_t brkn = temp;

	uint8_t lo;
	read(brkn + 0, &lo);
	uint8_t hi;
	read((brkn + 1) & 0x00FF, &hi);


	addr = hi;
	addr <<= 8;
	addr |= lo;
	addr += cpu_yreg;


	if (addr >> 8 != hi)
		return true;
	else
		return false;
}


/*
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------instructions
*/


// instruction: addition
bool nes_cpu::ADC()
{
	fetch();
	uint16_t temp = (uint16_t)cpu_accu + (uint16_t)data + (uint16_t)get_flag(crry);
	set_flag(crry, temp > 255);
	set_flag(zero, (temp & 0x00FF) == 0);
	set_flag(vrfl, (~((uint16_t)cpu_accu ^ (uint16_t)data) & ((uint16_t)cpu_accu ^ (uint16_t)temp)) & 0x0080);
	set_flag(ngtv, temp & 0x80);
	cpu_accu = temp & 0x00FF;
	return true;
}


// instruction: subtraction
bool nes_cpu::SBC()
{
	fetch();
	uint16_t value = ((uint16_t)data) ^ 0x00FF;
	uint16_t temp = (uint16_t)cpu_accu + value + (uint16_t)get_flag(crry);
	set_flag(crry, temp & 0xFF00);
	set_flag(zero, ((temp & 0x00FF) == 0));
	set_flag(vrfl, (temp ^ (uint16_t)cpu_accu) & (temp ^ value) & 0x0080);
	set_flag(ngtv, temp & 0x0080);
	cpu_accu = temp & 0x00FF;
	return 1;
}


// instruction: bitwise logic and
bool nes_cpu::AND()
{
	fetch();
	cpu_accu = cpu_accu & data;
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return true;
}


// instruction: arithmetic shift left
bool nes_cpu::ASL()
{
	fetch();
	uint8_t temp = data << 1;
	set_flag(crry, data & 0x80);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	if (opcode_set[opcd].addrmd == &nes_cpu::IMP)
		cpu_accu = temp;
	else
		write(addr, temp);
	return false;
}


// instruction: branch if carry clear
bool nes_cpu::BCC()
{
	if (!(cpu_stat & crry))
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: branch if carry set
bool nes_cpu::BCS()
{
	if (cpu_stat & crry)
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: branch if equal
bool nes_cpu::BEQ()
{
	if (cpu_stat & zero)
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: test bits in memory with accumulator
bool nes_cpu::BIT()
{
	fetch();
	uint8_t temp = cpu_accu & data;
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, data & ngtv);
	set_flag(vrfl, data & vrfl);
	return false;
}


// instruction: branch if negative
bool nes_cpu::BMI()
{
	if (cpu_stat & ngtv)
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: branch if not equal
bool nes_cpu::BNE()
{
	if (!(cpu_stat & zero))
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: branch if positive
bool nes_cpu::BPL()
{
	if (!(cpu_stat & ngtv))
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: break
bool nes_cpu::BRK()
{
	cpu_prgc++;


	set_flag(trpt, true);
	write(0x0100 + cpu_stkp, (cpu_prgc >> 8) & 0x00FF);
	cpu_stkp--;
	write(0x0100 + cpu_stkp, cpu_prgc & 0x00FF);
	cpu_stkp--;


	set_flag(brek, true);
	write(0x0100 + cpu_stkp, cpu_stat);
	cpu_stkp--;
	set_flag(brek, false);


	uint8_t lo;
	read(0xFFFE, &lo);
	uint8_t hi;
	read(0xFFFF, &hi);
	cpu_prgc = hi;
	cpu_prgc <<= 8;
	cpu_prgc |= lo;


	return false;
}


// instruction: branch if overflow clear
bool nes_cpu::BVC()
{
	if (!(cpu_stat & vrfl))
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: branch if overflow set
bool nes_cpu::BVS()
{
	if (cpu_stat & vrfl)
	{
		tick++;
		addr = cpu_prgc + brch;
		if ((addr & 0xFF00) != (cpu_prgc & 0xFF00))
			tick++;
		cpu_prgc = addr;
	}
	return false;
}


// instruction: clear carry flag
bool nes_cpu::CLC()
{
	set_flag(crry, false);
	return false;
}


// instruction: clear decimal flag
bool nes_cpu::CLD()
{
	set_flag(dcmd, false);
	return false;
}


// instruction: disable interrupts
bool nes_cpu::CLI()
{
	set_flag(trpt, false);
	return false;
}


// instruction: clear overflow flag
bool nes_cpu::CLV()
{
	set_flag(vrfl, false);
	return false;
}


// instruction: compare accumulator
bool nes_cpu::CMP()
{
	fetch();
	uint8_t temp = cpu_accu - data;
	set_flag(crry, cpu_accu >= data);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	return true;
}


// instruction: compare x register
bool nes_cpu::CPX()
{
	fetch();
	uint8_t temp = cpu_xreg - data;
	set_flag(crry, cpu_xreg >= data);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	return false;
}


// instruction: compare y register
bool nes_cpu::CPY()
{
	fetch();
	uint8_t temp = cpu_yreg - data;
	set_flag(crry, cpu_yreg >= data);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	return false;
}


// instruction: decrement value at memory location
bool nes_cpu::DEC()
{
	fetch();
	uint8_t temp = data - 1;
	write(addr, temp);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	return false;
}


// instruction: decrement x register
bool nes_cpu::DEX()
{
	cpu_xreg--;
	set_flag(zero, cpu_xreg == 0x00);
	set_flag(ngtv, cpu_xreg & 0x80);
	return false;
}


// instruction: decrement y register
bool nes_cpu::DEY()
{
	cpu_yreg--;
	set_flag(zero, cpu_yreg == 0x00);
	set_flag(ngtv, cpu_yreg & 0x80);
	return false;
}


// instruction: bitwise logic xor
bool nes_cpu::EOR()
{
	fetch();
	cpu_accu = cpu_accu ^ data;
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return true;
}


// instruction: increment value at memory location
bool nes_cpu::INC()
{
	fetch();
	uint8_t temp = data + 1;
	write(addr, temp);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	return false;
}


// instruction: increment x register
bool nes_cpu::INX()
{
	cpu_xreg++;
	set_flag(zero, cpu_xreg == 0x00);
	set_flag(ngtv, cpu_xreg & 0x80);
	return false;
}


// instruction: increment y register
bool nes_cpu::INY()
{
	cpu_yreg++;
	set_flag(zero, cpu_yreg == 0x00);
	set_flag(ngtv, cpu_yreg & 0x80);
	return false;
}


// instruction: jump to new location
bool nes_cpu::JMP()
{
	cpu_prgc = addr;
	return false;
}


// instruction: jump to new location saving return address
bool nes_cpu::JSR()
{
	cpu_prgc--;
	write(0x0100 + cpu_stkp, (cpu_prgc >> 8) & 0x00FF);
	cpu_stkp--;
	write(0x0100 + cpu_stkp, cpu_prgc & 0x00FF);
	cpu_stkp--;
	cpu_prgc = addr;
	return false;
}


// instruction: load the accumulator
bool nes_cpu::LDA()
{
	fetch();
	cpu_accu = data;
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return true;
}


// instruction: load the x register
bool nes_cpu::LDX()
{
	fetch();
	cpu_xreg = data;
	set_flag(zero, cpu_xreg == 0x00);
	set_flag(ngtv, cpu_xreg & 0x80);
	return true;
}


// instruction: load the y register
bool nes_cpu::LDY()
{
	fetch();
	cpu_yreg = data;
	set_flag(zero, cpu_yreg == 0x00);
	set_flag(ngtv, cpu_yreg & 0x80);
	return true;
}


// instruction: shift one bit right
bool nes_cpu::LSR()
{
	fetch();
	uint8_t temp = data >> 1;
	set_flag(crry, data & 0x01);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	if (opcode_set[opcd].addrmd == &nes_cpu::IMP)
		cpu_accu = temp;
	else
		write(addr, temp);
	return false;
}


// instruction: no operation
bool nes_cpu::NOP()
{
	switch (opcd)
	{
	case 0x1C:
	case 0x3C:
	case 0x5C:
	case 0x7C:
	case 0xDC:
	case 0xFC:
		return true;
		break;
	}
	return false;
}


// instruction: bitwise logic or
bool nes_cpu::ORA()
{
	fetch();
	cpu_accu = cpu_accu | data;
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return true;
}


// instruction: push accumulator to stack
bool nes_cpu::PHA()
{
	write(0x0100 + cpu_stkp, cpu_accu);
	cpu_stkp--;
	return false;
}


// instruction: push status register to stack
bool nes_cpu::PHP()
{
	write(0x0100 + cpu_stkp, cpu_stat | brek | dead);
	set_flag(brek, false);
	set_flag(dead, false);
	cpu_stkp--;
	return false;
}


// instruction: pull accumulator from stack
bool nes_cpu::PLA()
{
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &cpu_accu);
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return false;
}


// instruction: pull processor status from stack
bool nes_cpu::PLP()
{
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &cpu_stat);
	set_flag(dead, true);
	return false;
}


// instruction: rotate one bit left
bool nes_cpu::ROL()
{
	fetch();
	uint8_t temp = data << 1;
	if (cpu_stat & crry)
		temp |= 0x01;
	set_flag(crry, data & 0x80);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	if (opcode_set[opcd].addrmd == &nes_cpu::IMP)
		cpu_accu = temp;
	else
		write(addr, temp);
	return false;
}


// instruction: rotate one bit right
bool nes_cpu::ROR()
{
	fetch();
	uint8_t temp = data >> 1;
	if (cpu_stat & crry)
		temp |= 0x80;
	set_flag(crry, data & 0x01);
	set_flag(zero, temp == 0x00);
	set_flag(ngtv, temp & 0x80);
	if (opcode_set[opcd].addrmd == &nes_cpu::IMP)
		cpu_accu = temp;
	else
		write(addr, temp);
	return false;
}


// instruction: return from interrupt
bool nes_cpu::RTI()
{
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &cpu_stat);
	cpu_stat &= ~brek;
	cpu_stat &= ~dead;


	uint8_t lo;
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &lo);
	uint8_t hi;
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &hi);
	cpu_prgc = hi;
	cpu_prgc <<= 8;
	cpu_prgc |= lo;
	return false;
}


// instruction: return from subroutine
bool nes_cpu::RTS()
{
	uint8_t lo;
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &lo);
	uint8_t hi;
	cpu_stkp++;
	read(0x0100 + cpu_stkp, &hi);
	cpu_prgc = hi;
	cpu_prgc <<= 8;
	cpu_prgc |= lo;
	cpu_prgc++;
	return false;
}


// instruction: set carry flag
bool nes_cpu::SEC()
{
	set_flag(crry, true);
	return false;
}


// instruction: set decimal flag
bool nes_cpu::SED()
{
	set_flag(dcmd, true);
	return false;
}


// instruction: set interrupt flag
bool nes_cpu::SEI()
{
	set_flag(trpt, true);
	return false;
}


// instruction: store accumulator at address
bool nes_cpu::STA()
{
	write(addr, cpu_accu);
	return false;
}


// instruction: store x register at address
bool nes_cpu::STX()
{
	write(addr, cpu_xreg);
	return false;
}


// instruction: store y register at address
bool nes_cpu::STY()
{
	write(addr, cpu_yreg);
	return false;
}


// instruction: transfer accumulator to x register
bool nes_cpu::TAX()
{
	cpu_xreg = cpu_accu;
	set_flag(zero, cpu_xreg == 0x00);
	set_flag(ngtv, cpu_xreg & 0x80);
	return false;
}


// instruction: transfer accumulator to y register
bool nes_cpu::TAY()
{
	cpu_yreg = cpu_accu;
	set_flag(zero, cpu_yreg == 0x00);
	set_flag(ngtv, cpu_yreg & 0x80);
	return false;
}


// instruction: transfer stack pointer to x register
bool nes_cpu::TSX()
{
	cpu_xreg = cpu_stkp;
	set_flag(zero, cpu_xreg == 0x00);
	set_flag(ngtv, cpu_xreg & 0x80);
	return false;
}


// instruction: transfer x register to accumulator
bool nes_cpu::TXA()
{
	cpu_accu = cpu_xreg;
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return false;
}


// instruction: transfer x register to stack pointer
bool nes_cpu::TXS()
{
	cpu_stkp = cpu_xreg;
	return false;
}


// instruction: transfer y register to accumulator
bool nes_cpu::TYA()
{
	cpu_accu = cpu_yreg;
	set_flag(zero, cpu_accu == 0x00);
	set_flag(ngtv, cpu_accu & 0x80);
	return false;
}


// instruction: void
bool nes_cpu::XXX()
{
	return false;
}