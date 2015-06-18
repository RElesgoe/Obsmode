#ifndef STRUCTS_H
#define STRUCTS_H

#pragma pack(push)
#pragma pack(1)

struct _UNKNOWN {
	BYTE val;
};

// Thanks to Palomino for this
struct BWUNIT {
	/*000*/BWUNIT*  pPrev;              // ptr to previous unit struct
	/*004*/BWUNIT*  pNext;              // ptr to next unit struct
	/*008*/_UNKNOWN z_25[1];
	/*009*/WORD     iHealthPoints;      // health points (part of the dword at 008)
	/*011*/_UNKNOWN z_1[1];             // part of the dword at 008
	/*012*/PVOID    pSprite;            // sprite info for unit
	/*016*/WORD     iDest_X;            // destination x
	/*018*/WORD     iDest_Y;            // destination y 
	/*020*/BWUNIT*  pDestinationUnit;   // ptr to neutral this units destination (be it minerals, or cc, etc)
	/*024*/WORD     iDest_X2;           // destination x II (same as above)
	/*026*/WORD     iDest_Y2;           // destination y II (same as above)
	/*028*/_UNKNOWN z_2[12];
	/*040*/WORD     iTrue_X;            // current x
	/*042*/WORD     iTrue_Y;            // current y
	/*044*/_UNKNOWN z_3[32];
	/*076*/BYTE     iPlayerOwner;       // owner id
	/*077*/BYTE     iCurrentState;      // units current state
	/*078*/_UNKNOWN z_4[14];
	/*092*/BWUNIT*  pTargetUnit;        // current target unit for attacks/spell cast
	/*096*/_UNKNOWN z_12[1];
	/*097*/WORD     iShieldPoints;      // shield points
	/*099*/_UNKNOWN z_5[1];
	/*100*/BYTE     iUnitType;          // unit type
	/*101*/_UNKNOWN z_6[3];
	/*104*/BWUNIT*  pSecPrev;           // pointer to previous unit (units in cargo have original prev/next ptrs NULLed)
	/*108*/BWUNIT*  pSecNext;           // pointer to next unit (units in cargo have original prev/next ptrs NULLed)
	/*112*/_UNKNOWN z_20[21];
	/*133*/BYTE     iAliveTime;         // alive tick count
	/*134*/_UNKNOWN z_24[9];
	/*143*/BYTE     iKillCount;         // frag count
	/*144*/_UNKNOWN z_7[4];
	/*148*/BYTE     iUnitType2;         // unit type II
	/*149*/_UNKNOWN z_8[3];
	/*152*/WORD     iBuildingQueue[5];  // unit -types- for building queue (NOT UNIT TOKENS)
	/*162*/_UNKNOWN z_13[1];
	/*163*/WORD     iManaPoints;        // mana points
	/*165*/_UNKNOWN z_9[1];
	/*166*/BYTE     iBuildingUnits;     // (?) 0x26 = constructing, 0x17 = not constructing
	/*167*/_UNKNOWN z_10[9];
	/*176*/WORD     iLoadedUnitArray[8];// array of unit unique ids which are loaded into this unit (transport)
	/*192*/_UNKNOWN z_14[8];
	/*200*/BWUNIT*  pLastClickedUnit;   // last clicked unit (stays the same if state->idle)
	/*204*/_UNKNOWN z_11[32];
	/*236*/BWUNIT*  pChild;             // child unit (for terran buildings, its the addon)
	/*240*/_UNKNOWN z_21[28];
	/*268*/BYTE     iDefenseMatrixRemaining; // defense matrix time remaining
	/*269*/BYTE     iStimPackRemaining;      // stim pack remaining
	/*270*/BYTE     iEnsnareTimeRemaining;   // ensnare time remaining
	/*271*/BYTE     iLockdownTimeRemaining;  // lockdown time remaining
	/*272*/_UNKNOWN z_27[2];
	/*274*/BYTE     iPlagueTimeRemaining;
	/*275*/_UNKNOWN z_28[6];
	/*281*/BYTE     iParasiteStatus;
};

#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)

struct PlayerInfo {
	DWORD	PlayerID;
	DWORD	actionPlayerID;
	BYTE	Owner;
	BYTE	Race;
	BYTE	Force;
	char	PlayerName[25];
};
#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)

struct Action {
	DWORD	time;
	BYTE	size;
	BYTE	playerID;
	BYTE	actionType;
	BYTE	__params;	// dynamic size
};

#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)

struct ReplayHeader {
	BYTE	isBroodWar;
	DWORD	frameCount;
	BYTE	z_unknown1[3];
	DWORD	creationDate;
	BYTE	slots[12];
	char	gameName[28];
	WORD	mapWidth;
	WORD	mapHeight;
	BYTE	z_unknown2;
	BYTE	availSlots;
	WORD	gameSpeed;
	WORD	gameType;
	WORD	gameSubtype;
	DWORD	z_unknown3;
	WORD	tileSet;
	WORD	z_unknown4;
	char	creatorName[25];
	char	mapName[32];
	WORD	gameTypeID;
	WORD	subtypeID;
	WORD	subtypeDisplayValue;
	WORD	subtypeLabel;
	BYTE	victoryConditions;
	BYTE	resourceType;
	BYTE	unitStats;
	BYTE	fogOfWar;
	BYTE	startingUnits;
	BYTE	startingPositions;
	BYTE	playerTypes;
	BYTE	alliesAllowed;
	BYTE	numberOfTeams;
	BYTE	cheatCodesAllowed;
	BYTE	tournamentMode;
	DWORD	victoryConditionValue;
	DWORD	resourcesValue;
	DWORD	z_unknown5;
	BYTE	z_unknown6;
	PlayerInfo players[12];
	DWORD	playerColor[8];
	BYTE	z_unknown7[8];
};
#pragma pack(pop)

#endif