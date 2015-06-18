#include <windows.h>
#include <stdlib.h>


#define MAX_JUMP_PATCHES 250

struct Registers {
	DWORD EAX;
	DWORD ECX;
	DWORD EDX;
	DWORD EBX;
	DWORD ESP;
	DWORD EBP;
	DWORD ESI;
	DWORD EDI;
};

enum eParamFlags {
	EAX = 0x01,
	ECX = 0x02,
	EDX = 0x04,
	EBX = 0x08,
	ESP = 0x10,
	EBP = 0x20,
	ESI = 0x40,
	EDI = 0x80,
};

static const UINT16 moveToRamCodes[] = {
	0x0D89,
	0x1589,
	0x1D89,
	0x2589,
	0x2D89,
	0x3589,
	0x3D89,
};
static const UINT16 moveToRegsCodes[] = {
	0x0D8B,
	0x158B,
	0x1D8B,
	0x258B,
	0x2D8B,
	0x358B,
	0x3D8B,
};





class CJumpPatches {
private:
	DWORD	savedRegisters[MAX_JUMP_PATCHES];
	DWORD*	pJmpBackAddrs;
	BYTE*	codeBlocks[MAX_JUMP_PATCHES];
	DWORD	oldCode[MAX_JUMP_PATCHES];
	DWORD	patchesCounter;

	DWORD getBitsSet(BYTE b) {
		DWORD count = 0;
		for (count = 0; b; b >>= 1) {
			count += b & 1;
		}
		return count;
	}


public:
	CJumpPatches() {
		patchesCounter = -1;
		pJmpBackAddrs = (DWORD*)malloc(MAX_JUMP_PATCHES * 4);
	}

	void setJmpBackAddr(DWORD jmpPatchID, DWORD addr) {
		*(pJmpBackAddrs + jmpPatchID) = addr;
	}
	DWORD getJmpBackAddr(DWORD jmpPatchID) {
		return *(pJmpBackAddrs + jmpPatchID);
	}

	// Detours code from source to destination.
	// padSize fills restcode with NOPs (max 122).
	// jumpBackAddress = self explaining. (default source + patchedCode).
	// preProcessing if true the oldCode comes before our function (default false).
	// .------------------------
	//     returns jmpPatchID
	DWORD jumpPatch(DWORD source, DWORD destination, signed char padSize, DWORD jumpBackAddress, BOOL preProcessing) {
		DWORD	oldProtection = NULL;
		DWORD	distJmpPatch = 0;
		DWORD	distCallPatch = 0;
		DWORD	distJmpBack = 0;
		BYTE*	pCodeBlock = 0;
		DWORD*	pRegs = 0;



		/*MessageBoxA(0, "hui", "hui", 0);
		char c[200];
		int i = hde32_disasm((const void*)source, &instruction);
		sprintf(c, "%u", i);
		MessageBoxA(0, c, "hui", 0);*/


		// RECALC!!!
		// old code (5 + pad size)  +  save registers to ram (5 + 7*6)  +  push regs ptr (5)  
		// +  function call (5)  +  return check (2 + 2)  +  restore registers (5 + 7*6)  +  jumpBack (5)
		DWORD size = 150 + padSize;
		patchesCounter++;
		oldCode[patchesCounter] = (DWORD)malloc(5 + padSize);
		pRegs = (DWORD*)malloc(32);
		savedRegisters[patchesCounter] = (DWORD)pRegs;
		codeBlocks[patchesCounter] = (BYTE*)malloc(size);
		pCodeBlock = codeBlocks[patchesCounter];
		*(pJmpBackAddrs + patchesCounter) = jumpBackAddress;
		distJmpPatch = (DWORD)pCodeBlock - (source + 5);

		//MessageBoxA(0,"s","s",0);

				// Set read/write/execute access to the codeBlock we create and the code we want to edit (jmppatch)
		VirtualProtect((LPVOID)codeBlocks[patchesCounter], size, PAGE_EXECUTE_READWRITE, &oldProtection);
		VirtualProtect((LPVOID)source, 5 + padSize, PAGE_EXECUTE_READWRITE, &oldProtection);

		// save old Code
		memcpy((void*)oldCode[patchesCounter], (void*)source, 5 + padSize);

		// copy old code to our new codeBlock
		if (preProcessing) {
			memcpy((void*)pCodeBlock, (void*)oldCode[patchesCounter], 5 + padSize);
			pCodeBlock += 5 + padSize;
		}


		// write jmpPatch to our codeBlock
		*(BYTE*)source = 0xE9;
		*(DWORD*)(source + 1) = distJmpPatch;
		memset((void*)(source + 5), 0x90, padSize);

		// save registers to ram
		*(BYTE*)pCodeBlock = 0xA3;
		pCodeBlock++;
		*(DWORD*)pCodeBlock = (DWORD)pRegs;
		pCodeBlock += 4;
		for (int i = 0; i < 7; i++) {
			*(UINT16*)pCodeBlock = moveToRamCodes[i];
			pCodeBlock += 2;
			*(DWORD*)pCodeBlock = (DWORD)pRegs + ((i + 1) * 4);
			pCodeBlock += 4;
		}

		// push the pointer to the saved register values (arguments of our hook function)
		*(BYTE*)pCodeBlock = 0x68;
		pCodeBlock++;
		*(DWORD*)pCodeBlock = (DWORD)pRegs;
		pCodeBlock += 4;

		// write call to our hook function from the codeBlock
		distCallPatch = destination - ((DWORD)pCodeBlock + 5);
		*(BYTE*)pCodeBlock = 0xE8;
		pCodeBlock++;
		*(DWORD*)pCodeBlock = distCallPatch;
		pCodeBlock += 4;

		// check if code should be processed
		*(WORD*)pCodeBlock = 0xC085;								// Test EAX,EAX
		pCodeBlock += 2;

		// restore saved (and maybe modified) register values
		*(BYTE*)pCodeBlock = 0xA1;
		pCodeBlock++;
		*(DWORD*)pCodeBlock = (DWORD)pRegs;
		pCodeBlock += 4;
		for (int i = 0; i < 7; i++) {
			*(UINT16*)pCodeBlock = moveToRegsCodes[i];
			pCodeBlock += 2;
			*(DWORD*)pCodeBlock = (DWORD)pRegs + ((i + 1) * 4);
			pCodeBlock += 4;
		}


		// copy old code to our codeBlock (if postProcessing)
		if (!preProcessing) {

			*(BYTE*)pCodeBlock = (BYTE)0x74;							// JE
			pCodeBlock++;
			*(BYTE*)pCodeBlock = (BYTE)(5 + padSize);
			pCodeBlock++;

			memcpy(pCodeBlock, (void*)oldCode[patchesCounter], 5 + padSize);
			pCodeBlock += 5 + padSize;
		}


		// FIX THIS SHIT !!
		// jump back
		//distJmpPatch = (source+5) - ((DWORD)pCodeBlock + 5);
		*(WORD*)pCodeBlock = 0x25FF;
		pCodeBlock += 2;

		//char c[200];
		//sprintf(c, "%X \n%X", (DWORD)(&jumpBackAddress), jumpBackAddress);
		//MessageBoxA(0, c, "hui", 0);
		//DWORD hure = 0x49C335;

		*(DWORD*)pCodeBlock = (DWORD)(pJmpBackAddrs + patchesCounter);
		pCodeBlock += 4;

		// Restore the original access flags for the affected virtual memory page.
		VirtualProtect((LPVOID)source, 5 + padSize, oldProtection, &oldProtection);

		return patchesCounter;
	}
};



// MACH AN DESTRUKTOR ZUM DE GANZ SCHEISS WIDR FREIGEH!!!