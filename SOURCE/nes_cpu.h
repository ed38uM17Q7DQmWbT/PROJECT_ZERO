#pragma once
#include <string>
#include <vector>

#include "nes_crt.h"
#include "nes_ppu.h"


class nes_cpu
{
public:
	nes_cpu(nes_crt* cartridge);
	~nes_cpu();

	nes_crt* my_crt = nullptr;
	nes_ppu  my_ppu;
	uint8_t  my_ram[0x0800];
	uint8_t controllerA = 0x00;
	uint8_t controllerA_state = 0x00;
	uint8_t controllerB = 0x00;
	uint8_t controllerB_state = 0x00;
	// devices

	uint32_t nSystemClockCounter = 0;
	uint16_t addr = 0x0000;	// represents working address of the cpu
	uint16_t brch = 0x0000;	// represents working branch address from progam counter
	uint8_t	 data = 0x00;	// represents working data of the cpu
	uint8_t  opcd = 0x00;	// represents working opcode byte of the cpu
	uint8_t  tick = 0x00;	// tick counter
	// facilitative variables

	uint8_t  cpu_accu = 0x00;	// CPU accumulator register
	uint8_t  cpu_xreg = 0x00;	// CPU x register
	uint8_t  cpu_yreg = 0X00;	// CPU y register
	uint8_t  cpu_stat = 0X34;	// CPU status register
	uint8_t  cpu_stkp = 0XFD;	// CPU stack pointer 
	uint16_t cpu_prgc = 0xC000;	// CPU program counter
	// internal registers

	enum status_flags : uint8_t
	{
		crry = 0x01,	// carry flag
		zero = 0x02,	// zero flag
		trpt = 0x04,	// interrupt flag
		dcmd = 0x08,	// decimal mode, retrofitting artifact, dead flag
		brek = 0x10,	// break flag
		dead = 0x20,	// dead flag
		vrfl = 0x40,	// overflow flag
		ngtv = 0x80		// negative flag
	};
	// convenience enumeration to index into the status register

	void read(uint16_t addr, uint8_t* data);
	void write(uint16_t addr, uint8_t data);
	void clock();
	void reset();
	void intrq();
	void nmirq();

	uint8_t get_flag(status_flags flag);
	void set_flag(status_flags flag, bool value);
	void fetch();

	bool IMP();	bool IMM(); bool ZP0();	bool ZPX();
	bool ZPY();	bool REL(); bool ABS();	bool ABX();
	bool ABY();	bool IND(); bool IZX();	bool IZY();
	// *12 adressing modes

	bool ADC();	bool AND();	bool ASL();	bool BCC();
	bool BCS();	bool BEQ();	bool BIT();	bool BMI();
	bool BNE();	bool BPL();	bool BRK();	bool BVC();
	bool BVS();	bool CLC();	bool CLD();	bool CLI();
	bool CLV();	bool CMP();	bool CPX();	bool CPY();
	bool DEC();	bool DEX();	bool DEY();	bool EOR();
	bool INC();	bool INX();	bool INY();	bool JMP();
	bool JSR();	bool LDA();	bool LDX();	bool LDY();
	bool LSR();	bool NOP();	bool ORA();	bool PHA();
	bool PHP();	bool PLA();	bool PLP();	bool ROL();
	bool ROR();	bool RTI();	bool RTS();	bool SBC();
	bool SEC();	bool SED();	bool SEI();	bool STA();
	bool STX();	bool STY();	bool TAX();	bool TAY();
	bool TSX();	bool TXA();	bool TXS();	bool TYA();
	// *56 instructions

	bool XXX();
	// capture illegal opcodes

	class opcode	// opcode class
	{
	public:
		std::string name;						// opcode pneumonic
		bool(nes_cpu::* nstrct)() = nullptr;	// pointer to opcode instruction
		bool(nes_cpu::* addrmd)() = nullptr;	// pointer to opcode addressing mode
		uint8_t ticks = 0;						// opcode tick count
	};

	std::vector<opcode> opcode_set =	// 16*16 opcode matrix
	{
		{ "BRK", &nes_cpu::BRK, &nes_cpu::IMM, 7 }, { "ORA", &nes_cpu::ORA, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 3 }, { "ORA", &nes_cpu::ORA, &nes_cpu::ZP0, 3 }, { "ASL", &nes_cpu::ASL, &nes_cpu::ZP0, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "PHP", &nes_cpu::PHP, &nes_cpu::IMP, 3 }, { "ORA", &nes_cpu::ORA, &nes_cpu::IMM, 2 }, { "ASL", &nes_cpu::ASL, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "ORA", &nes_cpu::ORA, &nes_cpu::ABS, 4 }, { "ASL", &nes_cpu::ASL, &nes_cpu::ABS, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 },
		{ "BPL", &nes_cpu::BPL, &nes_cpu::REL, 2 }, { "ORA", &nes_cpu::ORA, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "ORA", &nes_cpu::ORA, &nes_cpu::ZPX, 4 }, { "ASL", &nes_cpu::ASL, &nes_cpu::ZPX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "CLC", &nes_cpu::CLC, &nes_cpu::IMP, 2 }, { "ORA", &nes_cpu::ORA, &nes_cpu::ABY, 4 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "ORA", &nes_cpu::ORA, &nes_cpu::ABX, 4 }, { "ASL", &nes_cpu::ASL, &nes_cpu::ABX, 7 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 },
		{ "JSR", &nes_cpu::JSR, &nes_cpu::ABS, 6 }, { "AND", &nes_cpu::AND, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "BIT", &nes_cpu::BIT, &nes_cpu::ZP0, 3 }, { "AND", &nes_cpu::AND, &nes_cpu::ZP0, 3 }, { "ROL", &nes_cpu::ROL, &nes_cpu::ZP0, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "PLP", &nes_cpu::PLP, &nes_cpu::IMP, 4 }, { "AND", &nes_cpu::AND, &nes_cpu::IMM, 2 }, { "ROL", &nes_cpu::ROL, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "BIT", &nes_cpu::BIT, &nes_cpu::ABS, 4 }, { "AND", &nes_cpu::AND, &nes_cpu::ABS, 4 }, { "ROL", &nes_cpu::ROL, &nes_cpu::ABS, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 },
		{ "BMI", &nes_cpu::BMI, &nes_cpu::REL, 2 }, { "AND", &nes_cpu::AND, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "AND", &nes_cpu::AND, &nes_cpu::ZPX, 4 }, { "ROL", &nes_cpu::ROL, &nes_cpu::ZPX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "SEC", &nes_cpu::SEC, &nes_cpu::IMP, 2 }, { "AND", &nes_cpu::AND, &nes_cpu::ABY, 4 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "AND", &nes_cpu::AND, &nes_cpu::ABX, 4 }, { "ROL", &nes_cpu::ROL, &nes_cpu::ABX, 7 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 },
		{ "RTI", &nes_cpu::RTI, &nes_cpu::IMP, 6 }, { "EOR", &nes_cpu::EOR, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 3 }, { "EOR", &nes_cpu::EOR, &nes_cpu::ZP0, 3 }, { "LSR", &nes_cpu::LSR, &nes_cpu::ZP0, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "PHA", &nes_cpu::PHA, &nes_cpu::IMP, 3 }, { "EOR", &nes_cpu::EOR, &nes_cpu::IMM, 2 }, { "LSR", &nes_cpu::LSR, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "JMP", &nes_cpu::JMP, &nes_cpu::ABS, 3 }, { "EOR", &nes_cpu::EOR, &nes_cpu::ABS, 4 }, { "LSR", &nes_cpu::LSR, &nes_cpu::ABS, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 },
		{ "BVC", &nes_cpu::BVC, &nes_cpu::REL, 2 }, { "EOR", &nes_cpu::EOR, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "EOR", &nes_cpu::EOR, &nes_cpu::ZPX, 4 }, { "LSR", &nes_cpu::LSR, &nes_cpu::ZPX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "CLI", &nes_cpu::CLI, &nes_cpu::IMP, 2 }, { "EOR", &nes_cpu::EOR, &nes_cpu::ABY, 4 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "EOR", &nes_cpu::EOR, &nes_cpu::ABX, 4 }, { "LSR", &nes_cpu::LSR, &nes_cpu::ABX, 7 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 },
		{ "RTS", &nes_cpu::RTS, &nes_cpu::IMP, 6 }, { "ADC", &nes_cpu::ADC, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 3 }, { "ADC", &nes_cpu::ADC, &nes_cpu::ZP0, 3 }, { "ROR", &nes_cpu::ROR, &nes_cpu::ZP0, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "PLA", &nes_cpu::PLA, &nes_cpu::IMP, 4 }, { "ADC", &nes_cpu::ADC, &nes_cpu::IMM, 2 }, { "ROR", &nes_cpu::ROR, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "JMP", &nes_cpu::JMP, &nes_cpu::IND, 5 }, { "ADC", &nes_cpu::ADC, &nes_cpu::ABS, 4 }, { "ROR", &nes_cpu::ROR, &nes_cpu::ABS, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 },
		{ "BVS", &nes_cpu::BVS, &nes_cpu::REL, 2 }, { "ADC", &nes_cpu::ADC, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "ADC", &nes_cpu::ADC, &nes_cpu::ZPX, 4 }, { "ROR", &nes_cpu::ROR, &nes_cpu::ZPX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "SEI", &nes_cpu::SEI, &nes_cpu::IMP, 2 }, { "ADC", &nes_cpu::ADC, &nes_cpu::ABY, 4 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "ADC", &nes_cpu::ADC, &nes_cpu::ABX, 4 }, { "ROR", &nes_cpu::ROR, &nes_cpu::ABX, 7 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 },
		{ "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "STA", &nes_cpu::STA, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "STY", &nes_cpu::STY, &nes_cpu::ZP0, 3 }, { "STA", &nes_cpu::STA, &nes_cpu::ZP0, 3 }, { "STX", &nes_cpu::STX, &nes_cpu::ZP0, 3 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 3 }, { "DEY", &nes_cpu::DEY, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "TXA", &nes_cpu::TXA, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "STY", &nes_cpu::STY, &nes_cpu::ABS, 4 }, { "STA", &nes_cpu::STA, &nes_cpu::ABS, 4 }, { "STX", &nes_cpu::STX, &nes_cpu::ABS, 4 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 4 },
		{ "BCC", &nes_cpu::BCC, &nes_cpu::REL, 2 }, { "STA", &nes_cpu::STA, &nes_cpu::IZY, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "STY", &nes_cpu::STY, &nes_cpu::ZPX, 4 }, { "STA", &nes_cpu::STA, &nes_cpu::ZPX, 4 }, { "STX", &nes_cpu::STX, &nes_cpu::ZPY, 4 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 4 }, { "TYA", &nes_cpu::TYA, &nes_cpu::IMP, 2 }, { "STA", &nes_cpu::STA, &nes_cpu::ABY, 5 }, { "TXS", &nes_cpu::TXS, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 5 }, { "STA", &nes_cpu::STA, &nes_cpu::ABX, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 },
		{ "LDY", &nes_cpu::LDY, &nes_cpu::IMM, 2 }, { "LDA", &nes_cpu::LDA, &nes_cpu::IZX, 6 }, { "LDX", &nes_cpu::LDX, &nes_cpu::IMM, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "LDY", &nes_cpu::LDY, &nes_cpu::ZP0, 3 }, { "LDA", &nes_cpu::LDA, &nes_cpu::ZP0, 3 }, { "LDX", &nes_cpu::LDX, &nes_cpu::ZP0, 3 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 3 }, { "TAY", &nes_cpu::TAY, &nes_cpu::IMP, 2 }, { "LDA", &nes_cpu::LDA, &nes_cpu::IMM, 2 }, { "TAX", &nes_cpu::TAX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "LDY", &nes_cpu::LDY, &nes_cpu::ABS, 4 }, { "LDA", &nes_cpu::LDA, &nes_cpu::ABS, 4 }, { "LDX", &nes_cpu::LDX, &nes_cpu::ABS, 4 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 4 },
		{ "BCS", &nes_cpu::BCS, &nes_cpu::REL, 2 }, { "LDA", &nes_cpu::LDA, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "LDY", &nes_cpu::LDY, &nes_cpu::ZPX, 4 }, { "LDA", &nes_cpu::LDA, &nes_cpu::ZPX, 4 }, { "LDX", &nes_cpu::LDX, &nes_cpu::ZPY, 4 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 4 }, { "CLV", &nes_cpu::CLV, &nes_cpu::IMP, 2 }, { "LDA", &nes_cpu::LDA, &nes_cpu::ABY, 4 }, { "TSX", &nes_cpu::TSX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 4 }, { "LDY", &nes_cpu::LDY, &nes_cpu::ABX, 4 }, { "LDA", &nes_cpu::LDA, &nes_cpu::ABX, 4 }, { "LDX", &nes_cpu::LDX, &nes_cpu::ABY, 4 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 4 },
		{ "CPY", &nes_cpu::CPY, &nes_cpu::IMM, 2 }, { "CMP", &nes_cpu::CMP, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "CPY", &nes_cpu::CPY, &nes_cpu::ZP0, 3 }, { "CMP", &nes_cpu::CMP, &nes_cpu::ZP0, 3 }, { "DEC", &nes_cpu::DEC, &nes_cpu::ZP0, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "INY", &nes_cpu::INY, &nes_cpu::IMP, 2 }, { "CMP", &nes_cpu::CMP, &nes_cpu::IMM, 2 }, { "DEX", &nes_cpu::DEX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "CPY", &nes_cpu::CPY, &nes_cpu::ABS, 4 }, { "CMP", &nes_cpu::CMP, &nes_cpu::ABS, 4 }, { "DEC", &nes_cpu::DEC, &nes_cpu::ABS, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 },
		{ "BNE", &nes_cpu::BNE, &nes_cpu::REL, 2 }, { "CMP", &nes_cpu::CMP, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "CMP", &nes_cpu::CMP, &nes_cpu::ZPX, 4 }, { "DEC", &nes_cpu::DEC, &nes_cpu::ZPX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "CLD", &nes_cpu::CLD, &nes_cpu::IMP, 2 }, { "CMP", &nes_cpu::CMP, &nes_cpu::ABY, 4 }, { "NOP", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "CMP", &nes_cpu::CMP, &nes_cpu::ABX, 4 }, { "DEC", &nes_cpu::DEC, &nes_cpu::ABX, 7 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 },
		{ "CPX", &nes_cpu::CPX, &nes_cpu::IMM, 2 }, { "SBC", &nes_cpu::SBC, &nes_cpu::IZX, 6 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "CPX", &nes_cpu::CPX, &nes_cpu::ZP0, 3 }, { "SBC", &nes_cpu::SBC, &nes_cpu::ZP0, 3 }, { "INC", &nes_cpu::INC, &nes_cpu::ZP0, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 5 }, { "INX", &nes_cpu::INX, &nes_cpu::IMP, 2 }, { "SBC", &nes_cpu::SBC, &nes_cpu::IMM, 2 }, { "NOP", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::SBC, &nes_cpu::IMP, 2 }, { "CPX", &nes_cpu::CPX, &nes_cpu::ABS, 4 }, { "SBC", &nes_cpu::SBC, &nes_cpu::ABS, 4 }, { "INC", &nes_cpu::INC, &nes_cpu::ABS, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 },
		{ "BEQ", &nes_cpu::BEQ, &nes_cpu::REL, 2 }, { "SBC", &nes_cpu::SBC, &nes_cpu::IZY, 5 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 8 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "SBC", &nes_cpu::SBC, &nes_cpu::ZPX, 4 }, { "INC", &nes_cpu::INC, &nes_cpu::ZPX, 6 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 6 }, { "SED", &nes_cpu::SED, &nes_cpu::IMP, 2 }, { "SBC", &nes_cpu::SBC, &nes_cpu::ABY, 4 }, { "NOP", &nes_cpu::NOP, &nes_cpu::IMP, 2 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }, { "???", &nes_cpu::NOP, &nes_cpu::IMP, 4 }, { "SBC", &nes_cpu::SBC, &nes_cpu::ABX, 4 }, { "INC", &nes_cpu::INC, &nes_cpu::ABX, 7 }, { "???", &nes_cpu::XXX, &nes_cpu::IMP, 7 }
	};
};