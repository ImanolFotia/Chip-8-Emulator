#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <bitset>
using mnemonic_map = std::map<uint16_t, std::string>;

mnemonic_map MNEMONICS = {
    { 0x00E0, "CLS"   }, { 0x00EE, "RET"   }, { 0x1000, "JP"   },/*{ 0x0000, SYS is ignored },*/ 
    { 0x2000, "CALL"  }, { 0x3000, "SE"  }, { 0x4000, "SNE"  }, { 0x5000, "SE"  }, 
    { 0x6000, "LD"  }, { 0x7000, "ADD" }, { 0x8000, "LD"   }, { 0x8001, "OR"    }, 
    { 0x8002, "AND"   }, { 0x8003, "XOR"   }, { 0x8004, "ADD"  }, { 0x8005, "SUB"   }, 
    { 0x8006, "SHR"   }, { 0x8007, "SUBN"  }, { 0x800E, "SHL"    }, { 0x9000, "SNE" }, 
    { 0xA000, "LD"  }, { 0xB000, "JP"  }, { 0xC000, "RND"    }, { 0xD000, "DRW"   }, 
    { 0xE09E, "SKP"   }, { 0xE0A1, "SKNP"  }, { 0xF007, "LD"  }, { 0xF00A, "LD" }, 
    { 0xF015, "LD"}, { 0xF018, "LD"}, { 0xF01E, "ADD"}, { 0xF029, "LD"}, 
    { 0xF033, "LD"}, { 0xF055, "LD"}, { 0xF065, "LD" },
    //Chip-48 instructions
    { 0x00B0, "SCU"   }, { 0x00C0, "SCD"   }, { 0x00FB, "SCR"    }, { 0x00FC, "SCL"   },
    { 0x00FD, "EXIT"  }, { 0x00FE, "LOW"   }, { 0x00FF, "HIGH"   }, { 0xF030, "LD" },
    { 0xF075, "LD"  }, { 0xF085, "LD" }
};

using opcode_set = std::set<uint16_t>;
// There are several types of opcodes, each type as to be & by a different value
// type 0xX0X0 : 0xF0FF
opcode_set opcode_type_1 = { 0x00E0, 0x00EE, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF/*0x0000 SYS is ignored*/};
// type 0xX000 : 0xF000
opcode_set opcode_type_2 = { 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000 };
// type 0xX00X : 0xF00F
opcode_set opcode_type_3 = { 0x8001, 0x8002, 0x8003, 0x8004, 0x8005, 0x8006, 0x8007, 0x800E, 0xF007, 0xF00A};
// type 0xX0XX : 0xF0FF
opcode_set opcode_type_4 = { 0xE09E, 0xE0A1, 0xF015, 0xF018, 0xF01E, 0xF029, 0xF030, 0xF033, 0xF055, 0xF065};
// type 0x00X0 : 0x00F0 (this are only used by Chip-48 instructions)
opcode_set opcode_type_5 = { 0x00B0, 0x00C0, 0xF015, 0xF018, 0xF01E, 0xF029, 0xF033, 0xF055, 0xF065, 0xF075, 0xF085};

static uint16_t get_opcode_type_1(uint16_t opcode) { return opcode & 0xF0FF; }
static uint16_t get_opcode_type_2(uint16_t opcode) { return opcode & 0xF000; }
static uint16_t get_opcode_type_3(uint16_t opcode) { return opcode & 0xF00F; }
static uint16_t get_opcode_type_4(uint16_t opcode) { return opcode & 0xF0FF; }
static uint16_t get_opcode_type_5(uint16_t opcode) { return opcode & 0x00F0; }

int main(int argc, char** argv)
{
    if(argc < 2){ std::cout << "No input file, aborting" << std::endl; return -1;}

    std::ifstream inFile(argv[1], std::ios::binary);

    uint16_t opcode;

    while(!inFile.eof())
    {
        int position = inFile.tellg();
        position += 0x200;
        uint8_t f, s;
		inFile.read((char*)&f, sizeof(uint8_t));
		inFile.read((char*)&s, sizeof(uint8_t));

        opcode = f << 8 | s;
        
        uint16_t nnn = (opcode << 0x0) & 0x0FFF;
        uint8_t kk =   (opcode >> 0x0);
        uint32_t x =   (opcode >> 0x8) & 0x0F;
        uint32_t y =   (opcode >> 0x4) & 0x0F;
        uint8_t  n =   (opcode << 0x0) & 0x0F;

        uint16_t op1 = get_opcode_type_1(opcode), op2 = get_opcode_type_2(opcode), op3 = get_opcode_type_3(opcode), op4 = get_opcode_type_4(opcode), op5 = get_opcode_type_5(opcode);
        uint16_t out_opcode = 0x0000;

        if(opcode_type_1.find(op1) != opcode_type_1.end()) out_opcode = op1;
        else if (opcode_type_2.find(op2) != opcode_type_2.end()) out_opcode = op2;
        else if (opcode_type_3.find(op3) != opcode_type_3.end()) out_opcode = op3;
        else if (opcode_type_4.find(op4) != opcode_type_4.end()) out_opcode = op4;
        else if (opcode_type_5.find(op5) != opcode_type_5.end()) out_opcode = op5;
        else {std::cout << "0x" <<position << ": 0x" << std::hex << std::bitset<16>(opcode).to_ullong() << "  \tDB " << opcode << std::endl; continue;}

        std::cout << "0x" <<position << ": 0x" << std::hex << std::bitset<16>(opcode).to_ullong() << "  \t" << MNEMONICS[out_opcode] << " ";
        
        switch(out_opcode)
        {
            case 0x1000: case 0x2000: case 0xB000:  std::cout << std::bitset<16>(nnn).to_ullong() << std::endl; break;

            case 0x3000: case 0x4000: case 0x5000: case 0x6000: case 0x7000: std::cout << " 0x" << std::bitset<8>(kk).to_ullong() << std::endl; break;

            case 0x8000: case 0x8001: case 0x8002: case 0x8003: std::cout << " " << std::bitset<8>(x).to_ullong() << ", " << std::bitset<8>(y).to_ullong() << std::endl; break;
            default: std::cout << "\n"; break;
        }
    }

    return 0;
}