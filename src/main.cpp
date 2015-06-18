#include <windows.h>
#include <stdio.h>
//#include <iostream>
//#include <fstream>
//using namespace std;

#include "offsets.h"
#include "structs.h"
#include "CJumpPatch.h"
#include "action_type.h"

CJumpPatches jumpPatches;

BYTE displayCodeZ[] = { 0x8B, 0x88, 0x44, 0x21, 0x58, 0x00 };
BYTE displayCodeT[] = { 0x8B, 0x88, 0xD4, 0x21, 0x58, 0x00 };
BYTE displayCodeP[] = { 0x8B, 0x88, 0x64, 0x22, 0x58, 0x00 };

BYTE oldCodes[] = {
	0x74, 0x75, 0x74, 0x75, 0x85,
	0x85, 0x85, 0x85, 0x85, 0x85,
	0x75, 0x74, 0x75, 0x75, 0x75,
	0x75, 0x85, 0x75, 0x75, 0x75, 0x75,
};
BYTE patchCodes[] = {
	0xEB, 0x70, 0xEB, 0x70, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80,
	0x70, 0xEB, 0x70, 0x70, 0x70,
	0x70, 0x80, 0x70, 0x70, 0x70, 0x70,
};
DWORD patchAddrs[] = {
	0x424A2A, 0x424F28, 0x4251B4, 0x425A6E,	0x425F4A,
	0x42651E, 0x42670C, 0x426919, 0x42727E, 0x4273FC,
	0x4274BD, 0x427901,	0x427CAF, 0x427CDB, 0x427D4B,
	0x46FB18, 0x46FC37, 0x46FE81, 0x496C2C, 0x4E6237, 0x4E62D2
};

BYTE jumpPatchOldCode[] = { 0xC7, 0x40, 0x14, 0x00, 0x00, 0x00, 0x00 };


struct Settings {
	char DeadAlive;
};
Settings TheSettings;

static char* pszMap_Replay_Path = (char*)0x57FD1C;
static BYTE* pPlayerNum1 = (BYTE*)0x512684;
static BYTE* pPlayerNum2 = (BYTE*)0x512688;

DWORD	mainThreadID = 0;
HANDLE	hMainThread = 0;
DWORD	oldWinProc = 0;

bool	bObsModeEnabled = true;
long	startTime = 0;

int actions[8] = { 0 };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int mainThread();
void checkEnableDisable();
int getAPM(BYTE player);

void bwSetScreenPos(DWORD x, DWORD y);
void bwPrintTextIngame(const char* szText);
void bwRefreshScreen(int x1, int y1, int x2, int y2);
void bwCenteredText(const char* szText);

VOID CodePatch(BYTE* pNewCode, BYTE* pCodeToPatch, int nBytes);

bool recordAction(Registers* regs);


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	char szSettingsPath[MAX_PATH];

	switch (ul_reason_for_call) {
	case DLL_THREAD_ATTACH:
		break;
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		GetModuleFileNameA(hModule, szSettingsPath, MAX_PATH);
		sprintf(strrchr(szSettingsPath, '.'), "%s", ".inj");
		TheSettings.DeadAlive = GetPrivateProfileIntA("Settings", "deadalive", 0, szSettingsPath);
		hMainThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mainThread, 0, 0, &mainThreadID);
		return TRUE;

	case DLL_PROCESS_DETACH:
		//SetWindowLong(FindWindow("SWarClass", 0), GWL_WNDPROC, oldWinProc);
		TerminateThread(hMainThread, 0);
		// ufruma dude
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}

	return TRUE;
}


int mainThread() {
	HWND hWndBW = 0;
	while (!hWndBW) {
		hWndBW = FindWindow("SWarClass", 0);
		Sleep(50);
	}

	//oldWinProc = SetWindowLong(hWndBW, GWL_WNDPROC, (LONG)WindowProc);

	// main loop
	char* pCornerText = (char *)varOffsets::pCornerText;
	BWUNIT* selectedUnit = 0;
	while (true) {
		checkEnableDisable();

		selectedUnit = *(BWUNIT**)varOffsets::ppSelectedUnit;
		if (bObsModeEnabled && selectedUnit != 0) {
			BYTE selPlayer = selectedUnit->iPlayerOwner;
			int mins = *(varOffsets::pMinerals + selPlayer);
			int gas = *(varOffsets::pGas + selPlayer);
			int supply = 0;
			int maxSupply = 0;
			PlayerInfo* pPlayer = varOffsets::pPlayerEntry + selPlayer;


			/* debug shiz */
			//static int ctr = 0;
			//if(ctr >= 50){
			//	char c[1212];
			//	char* races[] = {"Z", "T", "P"};
			//	fstream f ("C:\\ObsMode.log", ios::out | ios::app);
			//	for(int i=0; i<12; i++){
			//		sprintf(c, "%s", races[*(varOffsets::pPlayerRace + i)]);
			//		f.write(c, strlen(c));
			//	}
			//	f.write("\n\n", 2);

			//	PlayerEntry* pPlayer;
			//	for(int i=0; i<12; i++){
			//		pPlayer = varOffsets::pPlayerEntry + i;
			//		sprintf(c, "pPlayer: %X\n", pPlayer);
			//		f.write(c, strlen(c));
			//		sprintf(c, "Force:\t%u\nOwner:\t%u\nPlayerID:\t%u\nPlayerName:\t%s\nRace:\t%s\n", pPlayer->Force, pPlayer->Owner, pPlayer->PlayerID, pPlayer->PlayerName, races[pPlayer->Race]);
			//		f.write(c, strlen(c));
			//	}
			//	f.write("\n\n", 2);

			//	sprintf(c, "Player:\t%u\nRace:\t%s\nMins:\t%u\nGas:\t%u\nSupplyT:\t%u\nSupplyP:\t%u\nSupplyZ:\t%u\nSelected Building HP:\t%u\n\n", 
			//				selPlayer, races[*(varOffsets::pPlayerRace + selPlayer)], mins, gas, *varOffsets::pSupplyT, *varOffsets::pSupplyP, *varOffsets::pSupplyZ, selectedUnit->iHealthPoints);
			//	f.write(c, strlen(c));
			//	f.close();
			//	ctr=0;
			//}else{
			//	ctr++;
			//}
			// end debug shiz

			//switch( pPlayer->Race ){
			//	case 0:	// zerg
			//		supply	= *(varOffsets::pSupplyZ + selPlayer);
			//		maxSupply	= *(varOffsets::pMaxSupplyZ + selPlayer);
			//		break;
			//	case 1:	// terran
			//		supply	= *(varOffsets::pSupplyT + selPlayer);
			//		maxSupply	= *(varOffsets::pMaxSupplyT + selPlayer);
			//		break;
			//	case 2:	// toss
			//		supply	= *(varOffsets::pSupplyP + selPlayer);
			//		maxSupply	= *(varOffsets::pMaxSupplyP + selPlayer);
			//		break;
			//	default:
			//		MessageBox(0,"Wrong Race.","ERROR",0);
			//}

			BYTE movECX = 0xB9;	DWORD zero = 0;	BYTE nop = 0x90;
			DWORD supplyCount = 0;
			DWORD* pMaxSupplyZ = varOffsets::pMaxSupplyZ + selPlayer;
			DWORD* pMaxSupplyT = varOffsets::pMaxSupplyT + selPlayer;
			DWORD* pMaxSupplyP = varOffsets::pMaxSupplyP + selPlayer;
			DWORD* pSupplyZ = varOffsets::pSupplyZ + selPlayer;
			DWORD* pSupplyT = varOffsets::pSupplyT + selPlayer;
			DWORD* pSupplyP = varOffsets::pSupplyP + selPlayer;

			if (*pMaxSupplyZ > 0) {
				CodePatch(&movECX, varOffsets::pShowSupplyZ, 1);
				CodePatch((BYTE*)pMaxSupplyZ, varOffsets::pShowSupplyZ + 1, 4);
				CodePatch(&nop, varOffsets::pShowSupplyZ + 5, 1);
				sprintf(pCornerText, "%u/%u", *pSupplyZ / 2, *pMaxSupplyZ / 2);
				supplyCount++;
			}
			else {
				CodePatch(&movECX, varOffsets::pShowSupplyZ, 1);
				CodePatch((BYTE*)&zero, varOffsets::pShowSupplyZ + 1, 4);
				CodePatch(&nop, varOffsets::pShowSupplyZ + 5, 1);
			}
			if (*pMaxSupplyT > 0) {
				CodePatch(&movECX, varOffsets::pShowSupplyT, 1);
				CodePatch((BYTE*)pMaxSupplyT, varOffsets::pShowSupplyT + 1, 4);
				CodePatch(&nop, varOffsets::pShowSupplyT + 5, 1);
				sprintf(pCornerText + (0x20 * supplyCount), "%u/%u", *pSupplyT / 2, *pMaxSupplyT / 2);
				supplyCount++;
			}
			else {
				CodePatch(&movECX, varOffsets::pShowSupplyT, 1);
				CodePatch((BYTE*)&zero, varOffsets::pShowSupplyT + 1, 4);
				CodePatch(&nop, varOffsets::pShowSupplyT + 5, 1);
			}
			if (*pMaxSupplyP > 0) {
				CodePatch(&movECX, varOffsets::pShowSupplyP, 1);
				CodePatch((BYTE*)pMaxSupplyP, varOffsets::pShowSupplyP + 1, 4);
				CodePatch(&nop, varOffsets::pShowSupplyP + 5, 1);
				sprintf(pCornerText + (0x20 * supplyCount), "%u/%u", *pSupplyP / 2, *pMaxSupplyP / 2);
				supplyCount++;
			}
			else {
				CodePatch(&movECX, varOffsets::pShowSupplyP, 1);
				CodePatch((BYTE*)&zero, varOffsets::pShowSupplyP + 1, 4);
				CodePatch(&nop, varOffsets::pShowSupplyP + 5, 1);
			}

			sprintf(pCornerText + (0x20 * supplyCount), "%u", gas);
			sprintf(pCornerText + (0x20 * (supplyCount + 1)), "%u", mins);
			bwRefreshScreen(200, 0, 640, 15);


			if (TheSettings.DeadAlive) {
				char szCenteredText[0xFF];
				char replayPlayerID = *(varOffsets::pPlayerIDs + selPlayer);
				ReplayHeader* pReplayHeader = varOffsets::pRepHeader;

				//int apm = getAPM(selPlayer);

				//sprintf(szCenteredText, "selPlayer: %u\nrepPlayerID: %u\n%u", selPlayer, replayPlayerID, pReplayHeader->players->PlayerID);
				//MessageBox(0,szCenteredText, "hui", 0);
				//sprintf(szCenteredText, "%s\n%s", pPlayer->PlayerName, pReplayHeader->players[selPlayer].PlayerName);
				//MessageBox(0,szCenteredText, "hui", 0);
				//sprintf(szCenteredText, "%u\n%X\n%u\n%X\n%u\n%X\n%u\n%X\n%s\n%X", 
				//	pReplayHeader->isBroodWar,
				//	&pReplayHeader->isBroodWar,
				//	pReplayHeader->frameCount,
				//	&pReplayHeader->frameCount,
				//	pReplayHeader->creationDate,
				//	&pReplayHeader->creationDate,
				//	pReplayHeader->slots[0],
				//	&pReplayHeader->slots,
				//	pReplayHeader->gameName,
				//	&pReplayHeader->gameName
				//);
				//MessageBox(0,szCenteredText, "hui", 0);
				//for(int i=0; i<8; i++){
				//	sprintf(szCenteredText, "%u %s\n%d", i, pReplayHeader->players[i].PlayerName, pReplayHeader->players[i].actionCount);
				//	MessageBox(0,szCenteredText, "hui", 0);
				//}

				int unitCount = 0;
				BWUNIT* pUnit = selectedUnit;
				for (int i = 0; true; i++) {
					if (pUnit->iUnitType == selectedUnit->iUnitType && pUnit->iPlayerOwner == selectedUnit->iPlayerOwner) {
						unitCount++;
					}
					if ((pUnit->pNext == 0 && pUnit->pSecNext == 0) || (pUnit == selectedUnit && i > 1)) {
						break;
					}
					else {
						if (pUnit->pNext == 0) {
							pUnit = pUnit->pSecNext;
						}
						else {
							pUnit = pUnit->pNext;
						}
					}
				}


				DWORD* pDeaths = varOffsets::pUnitDeaths + selPlayer;
				pDeaths += 12 * selectedUnit->iUnitType;
				sprintf(szCenteredText, "Dead: %u    Alive: %u", *pDeaths, --unitCount);
				bwCenteredText(szCenteredText);
			}
		}
		Sleep(125);
	}

	return 0;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//static bool toggleObsMode = false;

	//switch(uMsg){
	//	case WM_KEYUP:
	//		//if(wParam == VK_F12){
	//		//	toggleObsMode = !toggleObsMode;
	//		//	if(toggleObsMode){
	//		//		for(int i=0; i < sizeof(patchCodes); i++){
	//		//			CodePatch(&patchCodes[i], (BYTE*)patchAddrs[i], 1);
	//		//		}
	//		//		bwPrintTextIngame("\x07ObsMode enabled.");
	//		//	}else{
	//		//		for(int i=0; i < sizeof(patchCodes); i++){
	//		//			CodePatch(&oldCodes[i], (BYTE*)patchAddrs[i], 1);
	//		//		}
	//		//		bwPrintTextIngame("\x06ObsMode disabled.");
	//		//	}
	//		//}
	//		break;
	//}

	return CallWindowProc((WNDPROC)oldWinProc, hWnd, uMsg, wParam, lParam);
}


void bwSetScreenPos(DWORD x, DWORD y) {
	static long function_addr = eFuncOffsets::setScreenPos;
	__asm {
		MOV EAX, x
			MOV ECX, y
			CALL[function_addr]
	}
}
void bwPrintTextIngame(const char* szText) {
	static long function_addr = eFuncOffsets::sendTextIngameClient;
	__asm {
		xor eax, eax
			mov edi, szText
			CALL DWORD PTR[function_addr]
	}
}
void bwRefreshScreen(int x1, int y1, int x2, int y2) {
	static const int BWFXN_RefreshScreen = 0x41E0D0;
	__asm {
		push x2
			mov edx, y2
			mov ecx, y1
			mov eax, x1
			CALL DWORD PTR[BWFXN_RefreshScreen]
	}
}



void bwCenteredText(const char* szText) {
	static int FunctionAddr = 0x48D1C0;
	__asm {
		xor eax, eax
			or eax, 0xFFFFFFFF
			push 0x0
			push szText
			call dword ptr[FunctionAddr]
	}
}
VOID CodePatch(BYTE* pNewCode, BYTE* pCodeToPatch, int nBytes) {
	DWORD OldProt;
	VirtualProtect(pCodeToPatch, nBytes, PAGE_EXECUTE_READWRITE, &OldProt);
	for (int i = 0; i < nBytes; i++) {
		*(pCodeToPatch + i) = *(pNewCode + i);
	}
	VirtualProtect(pCodeToPatch, nBytes, OldProt, &OldProt);
}


void checkEnableDisable() {
	bool isIngame = *varOffsets::pIsIngame;
	int playerId = *varOffsets::pCurPlayerId;
	int mins = *(varOffsets::pCollectedMins + playerId);
	int supply = *(varOffsets::pSupplyP + playerId) + *(varOffsets::pSupplyT + playerId) + *(varOffsets::pSupplyZ + playerId);


	if (isIngame && !bObsModeEnabled && mins < 250 && supply <= 2) {
		bObsModeEnabled = true;
		for (int i = 0; i < sizeof(patchCodes); i++) {
			CodePatch(&patchCodes[i], (BYTE*)patchAddrs[i], 1);
		}
		bwPrintTextIngame("\x07ObsMode enabled.");
		startTime = GetTickCount();
		memset(actions, 0, 8);

		//DWORD hJumpPatch = jumpPatches.jumpPatch(0x4CDCBF, (DWORD)recordAction, 2, 0x4CDCBF + 7, false);
	}
	if (isIngame && bObsModeEnabled && mins >= 250) {
		bObsModeEnabled = false;
		for (int i = 0; i < sizeof(patchCodes); i++) {
			CodePatch(&oldCodes[i], (BYTE*)patchAddrs[i], 1);
		}
		bwPrintTextIngame("\x06ObsMode disabled.");
		bwCenteredText("");
		// patch suppply display
		CodePatch(displayCodeZ, varOffsets::pShowSupplyZ, 6);
		CodePatch(displayCodeT, varOffsets::pShowSupplyT, 6);
		CodePatch(displayCodeP, varOffsets::pShowSupplyP, 6);

		//CodePatch(jumpPatchOldCode, (BYTE*)0x4CDCBF, 7);
	}
	if (!isIngame && bObsModeEnabled) {
		bObsModeEnabled = false;
		for (int i = 0; i < sizeof(patchCodes); i++) {
			CodePatch(&oldCodes[i], (BYTE*)patchAddrs[i], 1);
		}
		// patch suppply display
		CodePatch(displayCodeZ, varOffsets::pShowSupplyZ, 6);
		CodePatch(displayCodeT, varOffsets::pShowSupplyT, 6);
		CodePatch(displayCodeP, varOffsets::pShowSupplyP, 6);

		//CodePatch(jumpPatchOldCode, (BYTE*)0x4CDCBF, 7);
	}
}

int getAPM(BYTE player) {

	int time = *varOffsets::pCurrentFrame;
	double minutes = time / (24 * 60.);	// 1 tick ~= 1/24 sec.
	if (time <= 0) time = 1;
	double apm = (double)actions[player] / minutes;



	//BYTE* pbAction = *varOffsets::ppNetRecord + 0x90;
	//Action* pAction = (Action*)pbAction;
	//int actionCounter = 0;
	//int actions = 0;
	//int time = 1;

	//while(pAction->size != 0){
	//	if(pAction->playerID == player){
	//		actions++;
	//		time = pAction->time;
	//	}
	//	pbAction += pAction->size + 5;
	//	pAction = (Action*)pbAction;

	//	if(deb && (pAction->time / 24) > 60){
	//		deb = false;
	//	}

	//}
	//time = *varOffsets::pCurrentFrame;
	//double minutes = time / (24 * 60.);	// 1 tick ~= 1/24 sec.
	//if(time <= 0) time = 1;
	//double apm = (double)actions / minutes;

	return (int)apm;
}

bool recordAction(Registers* regs) {
	char c[1212];
	char playerName[30];
	Action* pAction = (Action*)(regs->EDX - 5);
	ReplayHeader* pRepHeader = varOffsets::pRepHeader;
	for (int i = 0; i < 8; i++) {
		if (pRepHeader->players[i].actionPlayerID == pAction->playerID) {
			actions[i]++;
			sprintf(playerName, "%s", pRepHeader->players[i].PlayerName);
			//return true;
		}
	}
	//sprintf(c, "%u\t%s\t%s\n", pAction->time, playerName, actionNames[pAction->actionType]);
	//FILE* fp = fopen("C:\\actions.txt", "a");
	//fputs(c, fp);
	//fclose(fp);
	//OutputDebugString(c);

	return true;
}
