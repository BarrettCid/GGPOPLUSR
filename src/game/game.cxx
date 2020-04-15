#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <shlwapi.h>
#include <strsafe.h>

#include <detours.h>
#include <d3d9.h>
#include <ggponet.h>
#include <sstream>

#include "vdf_parser.hpp"

#include "./game.h"
#include "./trainingmodehelper.h"

namespace trainingmodehelper {
	int positionState = 0;
	int swapState = 0;
}

static GameMethods* g_lpGameMethods;
static GameState* g_lpGameState;

/* Parses input flags, which may be based on non-default button settings,
and returns normalized input flags */
unsigned int normalizeInput(unsigned int* input, GameState* g_lpGameState) {
	int p = (g_lpGameState->ggpoState).localPlayerIndex;
	unsigned int normalizedInput = 0;

	normalizedInput |= (*input & Up);
	normalizedInput |= (*input & Down);
	normalizedInput |= (*input & Left);
	normalizedInput |= (*input & Right);

	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlP) {
		normalizedInput |= Punch;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlK) {
		normalizedInput |= Kick;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlS) {
		normalizedInput |= Slash;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlH) {
		normalizedInput |= HSlash;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlD) {
		normalizedInput |= Dust;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlRespect) {
		normalizedInput |= Respect;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlPKMacro) {
		normalizedInput |= Punch;
		normalizedInput |= Kick;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlPDMacro) {
		normalizedInput |= Punch;
		normalizedInput |= Dust;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlPKSMacro) {
		normalizedInput |= Punch;
		normalizedInput |= Kick;
		normalizedInput |= Slash;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlPKSHMacro) {
		normalizedInput |= Punch;
		normalizedInput |= Kick;
		normalizedInput |= Slash;
		normalizedInput |= HSlash;
	}
	if (*input & (g_lpGameState->arrPlayerData)[p].ctrlReset) {
		normalizedInput |= Reset;
	}

	return normalizedInput;
}

/* Parses normalized input flags and returns translated input flags which will be compliant with local button settings */
unsigned int translateFromNormalizedInput(unsigned int normalizedInput, int p, GameState* g_lpGameState) {
	unsigned int translatedInput = 0;

	translatedInput |= (normalizedInput & Up);
	translatedInput |= (normalizedInput & Down);
	translatedInput |= (normalizedInput & Left);
	translatedInput |= (normalizedInput & Right);

	if (normalizedInput & Punch) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlP;
	}
	if (normalizedInput & Kick) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlK;
	}
	if (normalizedInput & Slash) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlS;
	}
	if (normalizedInput & HSlash) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlH;
	}
	if (normalizedInput & Dust) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlD;
	}
	if (normalizedInput & Respect) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlRespect;
	}
	if (normalizedInput & Reset) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlReset;
	}
	if (normalizedInput & Pause) {
		translatedInput |= (g_lpGameState->arrPlayerData)[p].ctrlPause;
	}
	return translatedInput;
}

int fletcher32_checksum(short* data, size_t len)
{
	int sum1 = 0xffff, sum2 = 0xffff;

	while (len) {
		size_t tlen = len > 360 ? 360 : len;
		len -= tlen;
		do {
			sum1 += *data++;
			sum2 += sum1;
		} while (--tlen);
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	return sum2 << 16 | sum1;
}

void EnterVersus2P(GameState* gameState, int* arrCharacters, StageSelection* stage) {
	g_lpGameMethods->CleanUpFibers();
	gameState->arrnConfirmedCharacters[0] = arrCharacters[0];
	gameState->arrnConfirmedCharacters[1] = arrCharacters[1];
	*gameState->nConfirmedStageIndex = stage->value;
	*gameState->nSystemState = 0x11;
	*gameState->nGameMode = 0x803;
	*gameState->nUnknownIsPlayerActive1 = 1;
	*gameState->nUnknownIsPlayerActive2 = 1;
	gameState->arrbPlayerCPUValues[0] = 0;
	gameState->arrbPlayerCPUValues[1] = 0;
}

void EnableHitboxes(GameState* gameState) {
	std::ofstream configFile(gameState->szConfigPath);
	gameState->config.attribs["ShowHitboxes"] = "true";
	*gameState->bHitboxDisplayEnabled = 1;
	tyti::vdf::write(configFile, gameState->config);
	configFile.close();

}

void DisableHitboxes(GameState* gameState) {
	std::ofstream configFile(gameState->szConfigPath);
	gameState->config.attribs["ShowHitboxes"] = "false";
	*gameState->bHitboxDisplayEnabled = 0;
	tyti::vdf::write(configFile, gameState->config);
	configFile.close();
}

void LoadGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort) {
	auto root = gameState->config;
	nOurGGPOPort = std::stoi(root.attribs["GGPOLocalPort"]);
	nSyncPort = std::stoi(root.attribs["GGPORmtPort"]);
}

void LoadGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort, char* szHostIp) {
	auto root = gameState->config;
	nOurGGPOPort = std::stoi(root.attribs["GGPOLocalPort"]);
	nSyncPort = std::stoi(root.attribs["GGPORmtPort"]);
	root.attribs["GGPOIPAddr"].copy(szHostIp, IP_BUFFER_SIZE);
}

void SaveGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort) {
	gameState->config.attribs["GGPOLocalPort"] = std::to_string(nOurGGPOPort);
	gameState->config.attribs["GGPORmtPort"] = std::to_string(nSyncPort);
	std::ofstream configFile(gameState->szConfigPath);
	tyti::vdf::write(configFile, gameState->config);
	configFile.close();
}

void SaveGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort, char* szHostIp) {
	gameState->config.attribs["GGPOLocalPort"] = std::to_string(nOurGGPOPort);
	gameState->config.attribs["GGPORmtPort"] = std::to_string(nSyncPort);
	gameState->config.attribs["GGPOIPAddr"] = szHostIp;
	std::ofstream configFile(gameState->szConfigPath);
	tyti::vdf::write(configFile, gameState->config);
	configFile.close();
}

HRESULT ApplyConfiguration(GameState* lpState) {
	std::unordered_map<std::string, std::string>::iterator it;
	for (it = lpState->config.attribs.begin(); it != lpState->config.attribs.end(); ++it) {
		if (it->first.compare("ShowHitboxes") == 0) {
			*lpState->bHitboxDisplayEnabled = it->second.compare("true") == 0 ? 1 : 0;
		}
	}
	return S_OK;
}

void SaveGameState(GameState* gameState, SavedGameState* dest) {
	char szMessageBuf[1024] = { 0 };
	StringCchPrintfA(szMessageBuf, 1024, "save game state");
	// MessageBoxA(NULL, szMessageBuf, NULL, MB_OK);

	if (*gameState->arrCharacters != NULL) {
		CopyMemory(dest->arrCharacters, *gameState->arrCharacters, sizeof(GameObjectData) * 2);
	}
	if (*gameState->arrNpcObjects != NULL) {
		CopyMemory(dest->arrNpcObjects, *gameState->arrNpcObjects, sizeof(GameObjectData) * 0x60);
	}
	if (*gameState->arrEffectObjects != NULL) {
		CopyMemory(dest->arrEffectObjects, *gameState->arrEffectObjects, sizeof(GameObjectData) * 0x180);
	}
	CopyMemory(dest->arrPlayerData, gameState->arrPlayerData, sizeof(PlayerData) * 2);
	CopyMemory(&dest->inactiveNPCObjectPool_LinkedList, gameState->inactiveNPCObjectPool_LinkedList, sizeof(GameObjectData));
	CopyMemory(&dest->activeEffectObjectPool_LinkedList, gameState->activeEffectObjectPool_LinkedList, sizeof(GameObjectData));
	CopyMemory(&dest->activeNPCObjectPool_LinkedList, gameState->activeNPCObjectPool_LinkedList, sizeof(GameObjectData));
	CopyMemory(&dest->inactiveEffectObjectPool_LinkedList, gameState->inactiveEffectObjectPool_LinkedList, sizeof(GameObjectData));

	dest->fCameraXPos = *gameState->fCameraXPos;
	dest->nCameraHoldTimer = *gameState->nCameraHoldTimer;
	dest->nCameraZoom = *gameState->nCameraZoom;
	dest->nRoundTimeRemaining = *gameState->nRoundTimeRemaining;
	CopyMemory(&dest->RNG1, gameState->lpRNG1, sizeof(RandomNumberGenerator));
	CopyMemory(&dest->RNG2, gameState->lpRNG2, sizeof(RandomNumberGenerator));
	CopyMemory(&dest->RNG3, gameState->lpRNG3, sizeof(RandomNumberGenerator));
	dest->nPlayfieldLeftEdge = *gameState->nPlayfieldLeftEdge;
	dest->nPlayfieldTopEdge = *gameState->nPlayfieldTopEdge;
	CopyMemory(&dest->nCameraPlayerXPositionHistory, gameState->nCameraPlayerXPositionHistory, sizeof(int) * 2);
	CopyMemory(&dest->nCameraPlayerXMovementMagnitudeHistory, gameState->nCameraPlayerXMovementMagnitudeHistory, sizeof(int) * 2);
	CopyMemory(&dest->arrnP1InputRingBuffer, gameState->arrnP1InputRingBuffer, sizeof(WORD) * 32);
	CopyMemory(&dest->arrnP2InputRingBuffer, gameState->arrnP2InputRingBuffer, sizeof(WORD) * 32);
	dest->nP1InputRingBufferPosition = *gameState->nP1InputRingBufferPosition;
	dest->nP2InputRingBufferPosition = *gameState->nP2InputRingBufferPosition;
}

bool __cdecl ggpo_save_game_state_callback(
	unsigned char** buffer,
	int* len,
	int* checksum,
	int frame
) {
	// MessageBoxA(NULL, "ggpo_save_game_state_callback", NULL, MB_OK);
	*len = sizeof(SavedGameState);
	char szMessageBuf[1024] = { 0 };

	StringCchPrintfA(szMessageBuf, 1024, "save size: %d", *len);
	// MessageBoxA(NULL, szMessageBuf, NULL, MB_OK);

	*buffer = (unsigned char*)malloc(*len);
	StringCchPrintfA(szMessageBuf, 1024, "buffer address: %x", *buffer);
	// MessageBoxA(NULL, szMessageBuf, NULL, MB_OK);
	SavedGameState* dest = *(SavedGameState**)buffer;

	SaveGameState(g_lpGameState, dest);
	// MessageBoxA(NULL, "ggpo_save_game_state_callback end", NULL, MB_OK);

	// *checksum = fletcher32_checksum((short*)*buffer, *len / 2);
	return true;
}

void LoadGameState(GameState* gameState, SavedGameState* src) {
	bool bP1ResetPaletteEffect = false;
	bool bP2ResetPaletteEffect = false;

	if (((*gameState->arrCharacters)[0].dwGraphicalEffects & CE_FLAME) || ((*gameState->arrCharacters)[0].dwGraphicalEffects & CE_THUNDER)) {
		bP1ResetPaletteEffect = true;
	}

	if (((*gameState->arrCharacters)[1].dwGraphicalEffects & CE_FLAME) || ((*gameState->arrCharacters)[1].dwGraphicalEffects & CE_THUNDER)) {
		bP2ResetPaletteEffect = true;
	}

	CopyMemory(*gameState->arrCharacters, src->arrCharacters, sizeof(GameObjectData) * 2);
	CopyMemory(*gameState->arrNpcObjects, src->arrNpcObjects, sizeof(GameObjectData) * 0x60);
	CopyMemory(*gameState->arrEffectObjects, src->arrEffectObjects, sizeof(GameObjectData) * 0x180);
	CopyMemory(gameState->arrPlayerData, src->arrPlayerData, sizeof(PlayerData) * 2);
	CopyMemory(gameState->inactiveNPCObjectPool_LinkedList, &src->inactiveNPCObjectPool_LinkedList, sizeof(GameObjectData));
	CopyMemory(gameState->activeEffectObjectPool_LinkedList, &src->activeEffectObjectPool_LinkedList, sizeof(GameObjectData));
	CopyMemory(gameState->activeNPCObjectPool_LinkedList, &src->activeNPCObjectPool_LinkedList, sizeof(GameObjectData));
	CopyMemory(gameState->inactiveEffectObjectPool_LinkedList, &src->inactiveEffectObjectPool_LinkedList, sizeof(GameObjectData));
	*gameState->fCameraXPos = src->fCameraXPos;
	*gameState->nCameraHoldTimer = src->nCameraHoldTimer;
	*gameState->nCameraZoom = src->nCameraZoom;
	*gameState->nRoundTimeRemaining = src->nRoundTimeRemaining;
	CopyMemory(gameState->lpRNG1, &src->RNG1, sizeof(RandomNumberGenerator));
	CopyMemory(gameState->lpRNG2, &src->RNG2, sizeof(RandomNumberGenerator));
	CopyMemory(gameState->lpRNG3, &src->RNG3, sizeof(RandomNumberGenerator));
	*gameState->nPlayfieldLeftEdge = src->nPlayfieldLeftEdge;
	*gameState->nPlayfieldTopEdge = src->nPlayfieldTopEdge;
	CopyMemory(gameState->nCameraPlayerXPositionHistory, &src->nCameraPlayerXPositionHistory, sizeof(int) * 2);
	CopyMemory(gameState->nCameraPlayerXMovementMagnitudeHistory, &src->nCameraPlayerXMovementMagnitudeHistory, sizeof(int) * 2);
	CopyMemory(gameState->arrnP1InputRingBuffer, &src->arrnP1InputRingBuffer, sizeof(WORD) * 32);
	CopyMemory(gameState->arrnP2InputRingBuffer, &src->arrnP2InputRingBuffer, sizeof(WORD) * 32);
	*gameState->nP1InputRingBufferPosition = src->nP1InputRingBufferPosition;
	*gameState->nP2InputRingBufferPosition = src->nP2InputRingBufferPosition;

	if (bP1ResetPaletteEffect) {
		(*gameState->arrCharacters)[0].dwGraphicalEffects = (*gameState->arrCharacters)[0].dwGraphicalEffects | CE_RESET_PALETTE_EFFECT;
	}

	if (bP2ResetPaletteEffect) {
		(*gameState->arrCharacters)[1].dwGraphicalEffects = (*gameState->arrCharacters)[1].dwGraphicalEffects | CE_RESET_PALETTE_EFFECT;
	}
}

bool __cdecl ggpo_load_game_state_callback(unsigned char* buffer, int len) {
	// MessageBoxA(NULL, "ggpo_load_game_state_callback", NULL, MB_OK);
	SavedGameState* src = (SavedGameState*)buffer;
	LoadGameState(g_lpGameState, src);
	// MessageBoxA(NULL, "ggpo_load_game_state_callback_end", NULL, MB_OK);
	return true;
}

void __cdecl ggpo_free_buffer(void* buffer) {
	free(buffer);
}

void WINAPI FakeSimulateCurrentState() {
	static GGPONetworkStats stats;
	GGPOState& gs = g_lpGameState->ggpoState;

	if (g_lpGameState && gs.ggpo != NULL) {
		if (gs.bIsSynchronized != 0 && gs.nFramesAhead <= 0 && GGPO_SUCCEEDED(gs.lastResult)) {
			g_lpGameMethods->SimulateCurrentState();
			ggpo_advance_frame(g_lpGameState->ggpoState.ggpo);
		}
		ggpo_idle(g_lpGameState->ggpoState.ggpo, 2);
	}
	else {
		g_lpGameMethods->SimulateCurrentState();
	}
}

bool __cdecl ggpo_advance_frame_callback(int flags) {
	// MessageBoxA(NULL, "GGPO advance frame called", NULL, MB_OK);
	unsigned int inputs[2];
	int disconnect_flags;
	GGPOState& gs = g_lpGameState->ggpoState;
	GGPOErrorCode ggpoErr = GGPO_OK;

	// Make sure we fetch new inputs from GGPO and use those to update
	// the game state instead of reading from the keyboard.
	ggpoErr = ggpo_synchronize_input(
		g_lpGameState->ggpoState.ggpo, (void*)inputs, sizeof(int) * 2, &disconnect_flags);
	if (GGPO_SUCCEEDED(ggpoErr)) {
		*g_lpGameState->nP1CurrentFrameInputs = translateFromNormalizedInput(inputs[0], 0, g_lpGameState);
		*g_lpGameState->nP2CurrentFrameInputs = translateFromNormalizedInput(inputs[1], 1, g_lpGameState);
		*g_lpGameState->nPrimitivesDrawn = 0;
		*g_lpGameState->nNextPrimitiveBufferOffset = 0;
		g_lpGameMethods->SimulateCurrentState();
		ggpo_advance_frame(g_lpGameState->ggpoState.ggpo);
	}
	else {
		MessageBoxA(NULL, "advance-frame-callback: synchronize input failed!", NULL, MB_OK);
	}
	return true;
}

bool __cdecl ggpo_log_game_state(char* filename, unsigned char* buffer, int len) {
	return true;
}

bool __cdecl ggpo_begin_game(const char* game) {
	return true;
}

bool __cdecl ggpo_on_event(GGPOEvent* info) {
	int progress;
	switch (info->code) {
	case GGPO_EVENTCODE_CONNECTED_TO_PEER:
		// MessageBoxA(NULL, "connected to peer", NULL, MB_OK);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
		// MessageBoxA(NULL, "synchronizing to peer", NULL, MB_OK);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
		// MessageBoxA(NULL, "synchronized to peer", NULL, MB_OK);
		g_lpGameState->ggpoState.bIsSynchronized = 1;
		break;
	case GGPO_EVENTCODE_RUNNING:
		// MessageBoxA(NULL, "running", NULL, MB_OK);
		EnterVersus2P(g_lpGameState, g_lpGameState->ggpoState.characters, &STAGES[0]);
		break;
	case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
		// MessageBoxA(NULL, "interrupted", NULL, MB_OK);
		break;
	case GGPO_EVENTCODE_CONNECTION_RESUMED:
		// MessageBoxA(NULL, "resumed", NULL, MB_OK);
		break;
	case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
		// MessageBoxA(NULL, "disconnected", NULL, MB_OK);
		break;
	case GGPO_EVENTCODE_TIMESYNC:
		// MessageBoxA(NULL, "timesync", NULL, MB_OK);
		g_lpGameState->ggpoState.nFramesAhead = info->u.timesync.frames_ahead;
		break;
	}
	return true;
}

HMODULE LocatePERoot() {
	return DetourGetContainingModule(DetourGetEntryPoint(NULL));
}

HRESULT LocateGameMethods(HMODULE peRoot, GameMethods* dest) {
	unsigned int peRootOffset = (unsigned int)peRoot;

	dest->GenerateAndShadePrimitives = (void(WINAPI*)())(peRootOffset + 0x149D20);
	dest->SetupD3D9 = (int(WINAPI*)())(peRootOffset + 0x148B60);
	dest->WindowFunc = (LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM))(peRootOffset + 0x146A00);
	dest->SteamAPI_Init = (bool(__cdecl*)())DetourFindFunction("steam_api", "SteamAPI_Init");
	dest->IsDebuggerPresent = IsDebuggerPresent;
	dest->BeginSceneAndDrawGamePrimitives = (void(__cdecl*)(int))(peRootOffset + 0x436F0);
	dest->DrawUIPrimitivesAndEndScene = (void(WINAPI*)())(peRootOffset + 0x14AD80);
	dest->PollForInputs = (void(WINAPI*)())(peRootOffset + 0x52630);
	dest->SimulateCurrentState = (void(WINAPI*)())(peRootOffset + 0xE7AE0);
	dest->CleanUpFibers = (void(WINAPI*)())(peRootOffset + 0x3D720);
	dest->HandlePossibleSteamInvites = (void(WINAPI*)())(peRootOffset + 0xAE440);
	dest->IncrementRNGCursorWhileOffline = (void(WINAPI*)())(peRootOffset + 0x43220);
	dest->WaitForNextFrame = (void(WINAPI*)())(peRootOffset + 0x1475E0);
	dest->MarkAllUnlocksOn = (void(WINAPI*)())(peRootOffset + 0x72790);
	dest->MarkAllUnlocksOff = (void(WINAPI*)())(peRootOffset + 0x72660);
	dest->PlayerGameObjectInitialization = (void(WINAPI*)())(peRootOffset + 0x6A070);
	dest->CheckForTrainingModeRestart = (void(WINAPI*)())(peRootOffset + 0x1DA670);
	dest->AdjustCamera = (void(WINAPI*)())(peRootOffset + 0x5EA40);
	g_lpGameMethods = dest;

	return S_OK;
}

void LocatePlayData(HMODULE peRoot, PlayData* dest) {
	unsigned int peRootOffset = (unsigned int)peRoot;

	dest->arrnFWalkVel = (short*)(peRootOffset + 0x519FA0);
	dest->arrnBWalkVel = (short*)(peRootOffset + 0x51A02C);
	dest->arrnFDashStartupSpeed = (short*)(peRootOffset + 0x51A020);
	dest->arrnBDashXVel = (short*)(peRootOffset + 0x519FF0);
	dest->arrnBDashYVel = (short*)(peRootOffset + 0x519FA8);
	dest->arrnBDashGravity = (short*)(peRootOffset + 0x519F88);
	dest->arrnFJumpXVel = (short*)(peRootOffset + 0x519FF8);
	dest->arrnBJumpXVel = (short*)(peRootOffset + 0x519FF4);
	dest->arrnJumpHeight = (short*)(peRootOffset + 0x519FD8);
	dest->arrnGravity = (short*)(peRootOffset + 0x519F9C);
	dest->arrnFSuperJumpXVel = (short*)(peRootOffset + 0x51A008);
	dest->arrnBSuperJumpXVel = (short*)(peRootOffset + 0x519F8C);
	dest->arrnSuperJumpYVel = (short*)(peRootOffset + 0x519FEC);
	dest->arrnSuperJumpGravity = (short*)(peRootOffset + 0x519FB4);
	dest->arrnAirdashesGranted = (short*)(peRootOffset + 0x51A01C);
	dest->arrnAirJumpsGranted = (short*)(peRootOffset + 0x519FE0);
	dest->arrnFWalkTension = (short*)(peRootOffset + 0x519F90);
	dest->arrnFJumpAscentTension = (short*)(peRootOffset + 0x519FAC);
	dest->arrnFDashTension = (short*)(peRootOffset + 0x519F94);
	dest->arrnFAirdashTension = (short*)(peRootOffset + 0x519FB0);
}

void LocateCharacterConstants(HMODULE peRoot, CharacterConstants* dest) {
	unsigned int peRootOffset = (unsigned int)peRoot;

	dest->arrnStandingPushboxWidth = (short*)(peRootOffset + 0x3D3534);
	dest->arrnVanillaStandingPushboxHeight = (short*)(peRootOffset + 0x3D3754);
	dest->arrnPlusRStandingPushboxHeight = (short*)(peRootOffset + 0x3D3E3C);
	dest->arrnCrouchingPushboxWidth = (short*)(peRootOffset + 0x3D5124);
	dest->arrnCrouchingPushboxHeight = (short*)(peRootOffset + 0x3D5B08);
	dest->arrnAerialPushboxWidth = (short*)(peRootOffset + 0x3D5B3C);
	dest->arrnAerialPushboxHeight = (short*)(peRootOffset + 0x3D5B70);
	dest->arrnVanillaAerialPushboxYOffset = (short*)(peRootOffset + 0x3D5BA4);
	dest->arrnPlusRAerialPushboxYOffset = (short*)(peRootOffset + 0x3D5BD8);
	dest->arrnCloseSlashMaxDistance = (short*)(peRootOffset + 0x3D5C0C);
	dest->arrnVanillaAllowedNormals = (DWORD*)(peRootOffset + 0x3D5C40);
	dest->arrnVanillaEXAllowedNormals = (DWORD*)(peRootOffset + 0x3D5CE0);
	dest->arrnPlusRAllowedNormals = (DWORD*)(peRootOffset + 0x3D5D50);
	dest->arrnPlusREXAllowedNormals = (DWORD*)(peRootOffset + 0x3D5DC8);
	dest->arrnVanillaStandingThrowDistance = (short*)(peRootOffset + 0x3D1FFC);
	dest->arrnPlusRStandingThrowDistance = (short*)(peRootOffset + 0x3D2114);
	dest->arrnVanillaAerialThrowDistance = (short*)(peRootOffset + 0x3D287C);
	dest->arrnPlusRAerialThrowDistance = (short*)(peRootOffset + 0x3D2954);
	dest->arrnMaxAerialThrowVerticalDifference = (short*)(peRootOffset + 0x3D2A2C);
	dest->arrnMinAerialThrowVerticalDifference = (short*)(peRootOffset + 0x3D2FC4);
}

HRESULT LocateGameState(HMODULE peRoot, GameState* dest) {
	unsigned int peRootOffset = (unsigned int)peRoot;
	dest->nFramesToSkipRender = 0;
	dest->nFramesSkipped = 0;
	dest->lastSecondNumFramesSimulated = 0;
	dest->ggpoState.nFramesAhead = 0;
	dest->ggpoState.lastResult = GGPO_OK;

	dest->gameRenderTarget = (LPDIRECT3DSURFACE9*)(peRootOffset + 0x505AE0);
	dest->uiRenderTarget = (LPDIRECT3DSURFACE9*)(peRootOffset + 0x555B98);
	dest->d3dDevice = (LPDIRECT3DDEVICE9*)(peRootOffset + 0x555B94);
	dest->hWnd = (HWND*)(peRootOffset + 0x506554);
	dest->bHitboxDisplayEnabled = (DWORD*)(peRootOffset + 0x520B3C);
	dest->arrCharacters = (GameObjectData**)(peRootOffset + 0x516778);
	dest->arrNpcObjects = (GameObjectData**)(peRootOffset + 0x51677c);
	dest->arrEffectObjects = (GameObjectData**)(peRootOffset + 0x519E50);
	dest->nCameraHoldTimer = (unsigned int*)(peRootOffset + 0x51B114);
	dest->fCameraXPos = (float*)(peRootOffset + 0x51B14C);
	dest->nCameraZoom = (unsigned int*)(peRootOffset + 0x51B110);
	dest->arrPlayerData = (PlayerData*)(peRootOffset + 0x51A038);
	dest->nRoundTimeRemaining = (int*)(peRootOffset + 0x50F800);
	dest->lpRNG1 = (RandomNumberGenerator*)(peRootOffset + 0x565F20);
	dest->lpRNG2 = (RandomNumberGenerator*)(peRootOffset + 0x564B60);
	dest->lpRNG3 = (RandomNumberGenerator*)(peRootOffset + 0x5561B0);
	dest->inactiveNPCObjectPool_LinkedList = (GameObjectData*)(peRootOffset + 0x517A78);
	dest->activeEffectObjectPool_LinkedList = (GameObjectData*)(peRootOffset + 0x519E58);
	dest->activeNPCObjectPool_LinkedList = (GameObjectData*)(peRootOffset + 0x517BA8);
	dest->inactiveEffectObjectPool_LinkedList = (GameObjectData*)(peRootOffset + 0x5163E0);
	dest->nPlayfieldLeftEdge = (int*)(peRootOffset + 0x51B0F4);
	dest->nPlayfieldTopEdge = (int*)(peRootOffset + 0x51B0F8);
	dest->nCameraPlayerXPositionHistory = (int*)(peRootOffset + 0x51B12C);
	dest->nCameraPlayerXMovementMagnitudeHistory = (int*)(peRootOffset + 0x51B138);
	dest->arrnP1InputRingBuffer = (WORD*)(peRootOffset + 0x516200);
	dest->arrnP2InputRingBuffer = (WORD*)(peRootOffset + 0x516240);
	dest->nP1InputRingBufferPosition = (int*)(peRootOffset + 0x516280);
	dest->nP2InputRingBufferPosition = (int*)(peRootOffset + 0x516284);
	dest->nP1CurrentFrameInputs = (unsigned int*)(peRootOffset + 0x51EDC8);
	dest->nP2CurrentFrameInputs = (unsigned int*)(peRootOffset + 0x51EE60);
	dest->recTarget = (TrainingModeRec*)(peRootOffset + 0x4FDD28);
	dest->recStatus = (int*)(peRootOffset + 0x4FDD24);
	dest->recEnabled = (DWORD*)(peRootOffset + 0x4FDD2C);
	dest->nSystemState = (DWORD*)(peRootOffset + 0x555FF4);
	dest->nGameMode = (DWORD*)(peRootOffset + 0x51B8CC);
	dest->arrnConfirmedCharacters = (WORD*)(peRootOffset + 0x51B9E4);
	dest->nCharacterSelectStageIndex = (WORD*)(peRootOffset + 0x516048);
	dest->nConfirmedStageIndex = (DWORD*)(peRootOffset + 0x555FE4);
	dest->nUnknownIsPlayerActive1 = (DWORD*)(peRootOffset + 0x50BF30);
	dest->nUnknownIsPlayerActive2 = (DWORD*)(peRootOffset + 0x50BF68);
	dest->arrbPlayerCPUValues = (WORD*)(peRootOffset + 0x51B81C);

	LocateCharacterConstants(peRoot, &dest->characterConstants);
	LocatePlayData(peRoot, &dest->playData);

	dest->nPrimitivesDrawn = (int*)(peRootOffset + 0x54B200);
	dest->nNextPrimitiveBufferOffset = (int*)(peRootOffset + 0x548104);

	dest->sessionInitState.bHasRequest = 0;
	dest->sessionInitState.bHasResponse = 0;
	dest->sessionInitState.hSyncThread = NULL;
	InitializeCriticalSection(&dest->sessionInitState.criticalSection);

	return S_OK;
}

void SaveRecording(char* cLogpath, GameState* gameState) {
	FILE* pFile;
	pFile = fopen(cLogpath, "wb");

	fwrite(&gameState->recTarget->nPlayer, sizeof(byte), 1, pFile);
	fwrite(gameState->recTarget->nUnknown, sizeof(byte) * 3, 1, pFile);
	fwrite(&gameState->recTarget->RecInputs, sizeof(Inputs) * 3599, 1, pFile);
	fclose(pFile);

}

void LoadRecording(char* cLogpath, GameState* gameState) {
	FILE* pFile;
	pFile = fopen(cLogpath, "rb");

	fread(&gameState->recTarget->nPlayer, sizeof(byte), 1, pFile);
	fread(gameState->recTarget->nUnknown, sizeof(byte) * 3, 1, pFile);
	fread(&gameState->recTarget->RecInputs, sizeof(Inputs) * 3599, 1, pFile);
	fclose(pFile);
}

void PrepareGGPOSession(GameState* lpGameState) {
	char szMessageBuf[1024] = { 0 };

	ClientSynchronizationRequest* request = &lpGameState->sessionInitState.request;
	ServerSynchronizationResponse* response = &lpGameState->sessionInitState.response;
	bool bIsHost = lpGameState->sessionInitState.bIsHost;
	unsigned short nOurPort = bIsHost ? response->nPort : request->nPort;
	unsigned short nOpponentPort = bIsHost ? request->nPort : response->nPort;
	CopyMemory(lpGameState->lpRNG1, &response->RNG1, sizeof(RandomNumberGenerator));
	CopyMemory(lpGameState->lpRNG2, &response->RNG2, sizeof(RandomNumberGenerator));
	CopyMemory(lpGameState->lpRNG3, &response->RNG3, sizeof(RandomNumberGenerator));

	char msgBuffer[16];
	GGPOErrorCode result;

	GGPOSessionCallbacks* cb = &lpGameState->ggpoState.cb;
	cb->load_game_state = ggpo_load_game_state_callback;
	cb->save_game_state = ggpo_save_game_state_callback;
	cb->free_buffer = ggpo_free_buffer;
	cb->advance_frame = ggpo_advance_frame_callback;
	cb->log_game_state = ggpo_log_game_state;
	cb->begin_game = ggpo_begin_game;
	cb->on_event = ggpo_on_event;

	g_lpGameState = lpGameState;
	lpGameState->ggpoState.localPlayerIndex = bIsHost ? 0 : 1;
	lpGameState->ggpoState.characters[0] = (int)response->nSelectedCharacter;
	lpGameState->ggpoState.characters[1] = (int)request->nSelectedCharacter;
	lpGameState->ggpoState.ggpo = NULL;
	lpGameState->ggpoState.lastResult = GGPO_OK;
	lpGameState->ggpoState.bIsSynchronized = 0;
	result = ggpo_start_session(
		&lpGameState->ggpoState.ggpo,
		cb,
		"GGPOPLUSR",
		2,
		sizeof(int), // inputsize,
		nOurPort
	);
	ggpo_set_disconnect_timeout(lpGameState->ggpoState.ggpo, 3000);
	ggpo_set_disconnect_notify_start(lpGameState->ggpoState.ggpo, 3000);
	if (!GGPO_SUCCEEDED(result)) {
		MessageBoxA(NULL, "nope", NULL, MB_OK);
	}

	if (bIsHost) {
		lpGameState->ggpoState.localPlayer = &(lpGameState->ggpoState.p1);
		lpGameState->ggpoState.remotePlayer = &(lpGameState->ggpoState.p2);
	}
	else {
		lpGameState->ggpoState.localPlayer = &(lpGameState->ggpoState.p2);
		lpGameState->ggpoState.remotePlayer = &(lpGameState->ggpoState.p1);
	}
	lpGameState->ggpoState.p1.size = lpGameState->ggpoState.p2.size = sizeof(GGPOPlayer);
	lpGameState->ggpoState.p1.player_num = 1;
	lpGameState->ggpoState.p2.player_num = 2;
	lpGameState->ggpoState.remotePlayer->type = GGPO_PLAYERTYPE_REMOTE;
	lpGameState->ggpoState.localPlayer->type = GGPO_PLAYERTYPE_LOCAL;
	strcpy(lpGameState->ggpoState.remotePlayer->u.remote.ip_address, lpGameState->sessionInitState.szOpponentIP);
	lpGameState->ggpoState.remotePlayer->u.remote.port = nOpponentPort;

	result = ggpo_add_player(
		lpGameState->ggpoState.ggpo, &lpGameState->ggpoState.p1,
		&lpGameState->ggpoState.player_handles[0]);
	if (!GGPO_SUCCEEDED(result)) {
		sprintf(msgBuffer, "nope p1 %d", result);
		MessageBoxA(NULL, msgBuffer, NULL, MB_OK);
	}
	result = ggpo_add_player(
		lpGameState->ggpoState.ggpo,
		&lpGameState->ggpoState.p2,
		&lpGameState->ggpoState.player_handles[1]
	);
	if (!GGPO_SUCCEEDED(result)) {
		sprintf(msgBuffer, "nope p2 %d", result);
		MessageBoxA(NULL, msgBuffer, NULL, MB_OK);
	}

	ggpo_set_frame_delay(
		lpGameState->ggpoState.ggpo,
		lpGameState->ggpoState.player_handles[lpGameState->ggpoState.localPlayerIndex],
		2
	);
}
