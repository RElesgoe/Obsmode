#ifndef OFFSETS_H
#define OFFSETS_H

#include "structs.h"

enum eHookOffsets {
	SelectUnit = 0x12345678, // dummy
};

enum eFuncOffsets {
	setScreenPos = 0x49C330,
	sendTextIngameClient = 0x48D0C0,
};

enum eVarOffsets {
	PlayerNum = 0x57F090,		// BYTE*
	PlayerRace = 0x57F1A0,		// BYTE*
	SupplyZ = 0x582154,		// DWORD*
	SupplyT = 0x5821E4,		// DWORD*
	SupplyP = 0x582274,		// DWORD*
	MaxSupplyZ = 0x582124,		// DWORD*
	MaxSupplyT = 0x5821B4,		// DWORD*
	MaxSupplyP = 0x582244,		// DWORD*
	Minerals = 0x57F0D0,		// DWORD*
	Gas = 0x57F100,		// DWORD*
	CollectedMins = 0x57F160,		// DWORD* [all the minerals collected in the game]
	CornerText = 0x68C240,		// char* [most right text, can be 1 of the 3 supplys or gas]
	pSelectedUnit = 0x59722C,		// BWUNIT**
	isIngame = 0x6D11CC,		// BYTE*
};

namespace varOffsets {
	static BYTE*	pIsIngame = (BYTE*)0x6D11EC;
	static BYTE*	pCurPlayerId = (BYTE*)0x512684;
	static DWORD*	pPlayerIDs = (DWORD*)0x57EECC;  // 1.16.0
	static DWORD*	pCurrentFrame = (DWORD*)0x57F23C;
	static DWORD*	pSupplyZ = (DWORD*)0x582174;
	static DWORD*	pSupplyT = (DWORD*)0x582204;
	static DWORD*	pSupplyP = (DWORD*)0x582294;
	static DWORD*	pMaxSupplyZ = (DWORD*)0x582144;
	static DWORD*	pMaxSupplyT = (DWORD*)0x5821D4;
	static DWORD*	pMaxSupplyP = (DWORD*)0x582264;
	static DWORD*	pMinerals = (DWORD*)0x57F0F0;
	static DWORD*	pGas = (DWORD*)0x57F120;
	static DWORD*	pCollectedMins = (DWORD*)0x57F180;		// [all the minerals collected in the game]
	static DWORD*	pUnitDeaths = (DWORD*)0x58A364;

	static char* pCornerText = (char*)0x68C260;		// [most right text, can be 1 of the 3 supplys or gas]

	static BWUNIT** ppSelectedUnit = (BWUNIT**)0x59724C;
	static PlayerInfo* pPlayerEntry = (PlayerInfo*)0x57EEE0;
	static ReplayHeader* pRepHeader = (ReplayHeader*)0x6D0F30;

	// code offsets
	static BYTE* pShowSupplyZ = (BYTE*)0x48893F;
	static BYTE* pShowSupplyT = (BYTE*)0x48892B;
	static BYTE* pShowSupplyP = (BYTE*)0x488917;
}


#endif