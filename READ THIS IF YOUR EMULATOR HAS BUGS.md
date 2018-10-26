##### In this file I will list things that are not in most documentations you find online, these oversights will lead to hours of innecesary debugging.

* Before start implementing the opcodes, check that your `fetch_opcode()` function is giving you the right opcode.
* Initialize your `PC` with `0x200`

### Possible opcode oversights: 
* `0x00E0 - CLS`: If you don't refresh your image every cpu cycle, set your redraw texture to true when calling this function
* `0x00EE - RET`: Increase your Program counter after this call, or you will get infinite loops
* `0xB000 - JP` : Dont increment the program counter after any jump
* `0x8004 - ADD`: Check for carry before adding
* `0x8006 - SHR`: Bit shifting is performed on `Vy` and stored in `Vx`. Most documentations will perform the shifting in `Vx` and store it in `Vx`, **THIS IS INCORRECT**.
* `0x800E - SHR`: Bit shifting is performed on `Vy` and stored in `Vx`. Most documentations will perform the shifting in `Vx` and store it in `Vx`, **THIS IS INCORRECT**.
* `0xD000 - DRW`: Depending on your implementation performing XOR on your pixel will not make it completely black or white, do `pixel ^= 0xFF` instead
* `0xF029 - LD` : Multiply `Vx` by `0x0005`
* `0xF033 - LD` : `RAM[I] = (Vx / 100); RAM[I+1] = (Vx / 10) % 10; RAM[I+2] = (Vx%100)%10;`
* `0xF055 and 0xF065 - LD` : When reading or writing from V0 to Vx, Vx is included (`for(int i = i <= x; i++)` note the <= ) and also, at the end of the operation do `I += x + 1`,
                                not doing this will cause many bugs, like currupted text in Space Invaders, games with scores like pong will keep it wrongly, etc, etc, etc.  

                                
Once your Emulator is running kinda decent, check this ROM out https://slack-files.com/T3CH37TNX-F3RF5KT43-0fb93dbd1f
it'll give you error codes that you can check here https://slack-files.com/T3CH37TNX-F3RKEUKL4-b05ab4930d

Cowgod's reference (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) has tons of oversights that will lead to hours of despair, take a look at http://mattmik.com/files/chip8/mastering/chip8.html
or even Wikipedia's article on it, is pretty much perfect.

You may also want to add shift and load/store quirks since some roms rely on Cowgod's misleading documentation

I will update this file on SCHIP once I'm done with it