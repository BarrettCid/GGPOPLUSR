#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_demo.cpp>

#include "../game/game.h"

#define DEFAULT_ALPHA 0.87f

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); 

void DrawEnterVersus2PWindow(GameState* lpGameState, bool* pOpen) {
	static CharacterSelection* characters[2] = { &CHARACTERS[0], &CHARACTERS[1] };
	static int characterIDs[2] = { 1, 2 };
	static StageSelection* stage = &STAGES[0];

	ImGui::Begin("Character Select Helper", pOpen);

	for (int i = 0; i < 2; i++) {
		char* comboLabel = i == 0 ? "Character 1" : "Character 2";
		CharacterSelection* c = characters[i];
		if (ImGui::BeginCombo(comboLabel, c->name, 0)) {
			for (int j = 0; j < IM_ARRAYSIZE(CHARACTERS); j++) {
				bool is_selected = (c == &CHARACTERS[j]);
				if (ImGui::Selectable(CHARACTERS[j].name, is_selected)) {
					c = &CHARACTERS[j];
					characters[i] = c;
					characterIDs[i] = (unsigned short)CHARACTERS[j].value;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	if (ImGui::BeginCombo("Stage", stage->name, 0))
	{
		for (int n = 0; n < IM_ARRAYSIZE(STAGES); n++)
		{
			bool is_selected = (stage == &STAGES[n]);
			if (ImGui::Selectable(STAGES[n].name, is_selected)) {
				stage = &STAGES[n];
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Go")) {
		EnterVersus2P(lpGameState, characterIDs, stage);
	}
	ImGui::End();
}

void DrawGGPOStats(GameState* lpGameState) {
	static GGPONetworkStats stats;

	GGPOState* gs = &(lpGameState->ggpoState);
	int remotePlayerIndex = gs->localPlayerIndex == 0 ? 1 : 0;

	ggpo_get_network_stats(gs->ggpo, gs->player_handles[remotePlayerIndex], &stats);
	ImGui::Text("Handles: %d, %d", gs->player_handles[0], gs->player_handles[1]);
	ImGui::Text("Player IDs: %d, %d", gs->p1.player_num, gs->p2.player_num);
	ImGui::Text("Player addresses: %p, %p", &(gs->p1), &(gs->p2));
	ImGui::Text("Remote player address: %p", gs->remotePlayer);
	ImGui::Text("Local player address: %p", gs->localPlayer);

	ImGui::Text("Remote player values");
	ImGui::Text("num: %d", gs->remotePlayer->player_num);
	ImGui::Text("IP: %s", gs->remotePlayer->u.remote.ip_address);
	ImGui::Text("port: %d", gs->remotePlayer->u.remote.port);
	ImGui::Text("type: %d", gs->remotePlayer->type);

	ImGui::Text("Stats");
	ImGui::Text("send_queue_len %d", stats.network.send_queue_len);
	ImGui::Text("recv_queue_len %d", stats.network.recv_queue_len);
	ImGui::Text("ping %d", stats.network.ping);
	ImGui::Text("kbps_sent %d", stats.network.kbps_sent);
	ImGui::Text("local_frames_behind %d", stats.timesync.local_frames_behind);
	ImGui::Text("remote_frames_behind %d", stats.timesync.remote_frames_behind);
}

void DrawGGPOJoinWindow(GameState* lpGameState, bool* pOpen) {
	static GGPONetworkStats stats;

	static char szHostIp[32] = "192.168.1.182";
	static unsigned short nSyncPort = 44443;
	static unsigned short nOurGGPOPort = 44445;

	static CharacterSelection* lpCharacter = &CHARACTERS[0];

	GGPOState* gs = &(lpGameState->ggpoState);

	ImGui::Begin("GGPO Join", pOpen);
	ImGui::Text("Num frames simulated per second: %d", lpGameState->lastSecondNumFramesSimulated);
	
	ImGui::InputText("Host IP", szHostIp, 32);
	ImGui::InputScalar("Sync port", ImGuiDataType_U16, &nSyncPort);
	ImGui::InputScalar("GGPO Our port", ImGuiDataType_U16, &nOurGGPOPort);

	if (ImGui::BeginCombo("Selected character", lpCharacter->name, 0)) {
		for (int n = 0; n < IM_ARRAYSIZE(CHARACTERS); n++) {
			bool is_selected = (lpCharacter == &CHARACTERS[n]);
			if (ImGui::Selectable(CHARACTERS[n].name, is_selected)) {
				lpCharacter = &CHARACTERS[n];
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (lpGameState->ggpoState.ggpo == NULL) {
		if (lpGameState->sessionInitState.hSyncThread != NULL) {
			ImGui::Text("Synchronization thread started...");
		} else if (ImGui::Button("Prepare for connection")) {
			lpGameState->sessionInitState.hSyncThread = CreateSynchronizeClientThread(lpGameState,
				szHostIp,
				nSyncPort,
				nOurGGPOPort,
				lpCharacter->value);
		}
	}
	else {
		DrawGGPOStats(lpGameState);
	}
	ImGui::End();
}

void DrawGGPOHostWindow(GameState* lpGameState, bool* pOpen) {
	static GGPONetworkStats stats;
	static unsigned short nSyncPort = 44443;
	static unsigned short nOurGGPOPort = 44444;
	char buf[2];
	static CharacterSelection* lpCharacter = &CHARACTERS[0];

	ImGui::Begin("GGPO Host", pOpen);
	ImGui::InputScalar("Sync port", ImGuiDataType_U16, &nSyncPort);
	ImGui::InputScalar("Our GGPO port", ImGuiDataType_U16, &nOurGGPOPort);
	if (ImGui::BeginCombo("Selected character", lpCharacter->name, 0)) {
		for (int n = 0; n < IM_ARRAYSIZE(CHARACTERS); n++) {
			bool is_selected = (lpCharacter == &CHARACTERS[n]);
			if (ImGui::Selectable(CHARACTERS[n].name, is_selected)) {
				lpCharacter = &CHARACTERS[n];
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (lpGameState->ggpoState.ggpo == NULL) {
		if (lpGameState->sessionInitState.hSyncThread != NULL) {
			ImGui::Text("Synchronization thread started...");
		} else if (ImGui::Button("Prepare for connection")) {
			lpGameState->sessionInitState.hSyncThread = CreateSynchronizeServerThread(lpGameState,
				nSyncPort,
				nOurGGPOPort,
				lpCharacter->value);
		}
	}
	else {
		DrawGGPOStats(lpGameState);
	}
	ImGui::End();
}

void DrawGlobalStateWindow(GameState* lpGameState, bool* pOpen) {
	ImGui::Begin("Global State", pOpen, ImGuiWindowFlags_None);

	ImGui::Columns(2, NULL, false);

	ImGui::Text("Hitbox display enabled:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->bHitboxDisplayEnabled); ImGui::NextColumn();
	ImGui::Text("Camera x position:"); ImGui::NextColumn(); ImGui::Text("%f", *lpGameState->fCameraXPos); ImGui::NextColumn();
	ImGui::Text("Camera hold timer:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->nCameraHoldTimer); ImGui::NextColumn();
	ImGui::Text("Camera zoom:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->nCameraZoom); ImGui::NextColumn();
	ImGui::Text("Playfield left edge:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->nPlayfieldLeftEdge); ImGui::NextColumn();
	ImGui::Text("Playfield top edge:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->nPlayfieldTopEdge); ImGui::NextColumn();
	ImGui::Text("Round time remaining:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->nRoundTimeRemaining); ImGui::NextColumn();
	ImGui::Text("RNG index:"); ImGui::NextColumn(); ImGui::Text("%d", lpGameState->nRandomTable[0]); ImGui::NextColumn();
	ImGui::Text("Character root:"); ImGui::NextColumn(); ImGui::Text("%p", *lpGameState->arrCharacters); ImGui::NextColumn();
	ImGui::Text("NPC root:"); ImGui::NextColumn(); ImGui::Text("%p", *lpGameState->arrNpcObjects); ImGui::NextColumn();
	ImGui::Text("Player data root:"); ImGui::NextColumn(); ImGui::Text("%p", lpGameState->arrPlayerData); ImGui::NextColumn();
	ImGui::Text("Window handle:"); ImGui::NextColumn(); ImGui::Text("%d", *lpGameState->hWnd); ImGui::NextColumn();

	ImGui::Columns(1);

	ImGui::End();
}

void DrawObjectStateWindow(GameObjectData* lpGameObject, bool* pOpen) {
	ImGui::Begin(
		lpGameObject->playerIndex == 0 ? "Player 1 Object State" : "Player 2 Object State",
		pOpen,
		ImGuiWindowFlags_None
	);

	ImGui::Columns(2, NULL, false);

	ImGui::Text("Object address:");
	ImGui::Text("Object ID:");
	ImGui::Text("Action ID:");
	ImGui::Text("Object Facing:");
	ImGui::Text("Object Side:");
	ImGui::Text("Player data address:");
	ImGui::Text("X Position:");
	ImGui::Text("Y Position:");
	ImGui::Text("X Velocity:");
	ImGui::Text("Y Velocity:");

	ImGui::NextColumn();

	ImGui::Text("%p", lpGameObject);
	ImGui::Text("%X", lpGameObject->objectID);
	ImGui::Text("%X", lpGameObject->actNo);
	ImGui::Text("%X", lpGameObject->facing);
	ImGui::Text("%X", lpGameObject->side);
	ImGui::Text("%p", lpGameObject->playerData);
	ImGui::Text("%i", lpGameObject->xPos);
	ImGui::Text("%i", lpGameObject->ypos);
	ImGui::Text("%i", lpGameObject->xvel);
	ImGui::Text("%i", lpGameObject->yvel);

	ImGui::End();
}

void DrawPlayerStateWindow(TCHAR* windowName, PlayerData* lpPlayerData, bool* pOpen) {
	// Current faint gets stored as a short and divided by 100 before compared
	// to the character's maxFaint, which is stored as a single byte. Be careful
	// with integer truncation- comparing currentFaint to maxFaint*100 is not the
	// same as comparing currentFaint / 100 to maxFaint!
	int currentFaint = (int)(lpPlayerData->currentFaint / 100);
	int maxFaint = (int)lpPlayerData->maxFaint;

	ImGui::Begin(
		windowName,
		pOpen,
		ImGuiWindowFlags_None
	);

	ImGui::SliderScalar("Health", ImGuiDataType_U16, &lpPlayerData->currentHealth, &MIN_HEALTH, &MAX_HEALTH);
	ImGui::SliderScalar("Burst", ImGuiDataType_U16, &lpPlayerData->currentBurst, &MIN_BURST, &MAX_BURST);
	ImGui::SliderScalar("Tension", ImGuiDataType_U16, &lpPlayerData->currentTension, &MIN_TENSION, &MAX_TENSION);
	ImGui::SliderScalar("Guard Balance", ImGuiDataType_S16, &lpPlayerData->guardBalance, &MIN_GUARD_BALANCE, &MAX_GUARD_BALANCE);
	ImGui::SliderScalar("Current Stun", ImGuiDataType_S32, &currentFaint, &MIN_FAINT, &maxFaint);
	ImGui::SliderScalar("Negative Penalty", ImGuiDataType_S32, &lpPlayerData->negativePenaltyCounter, &MIN_NEGATIVE_PENALTY, &MAX_NEGATIVE_PENALTY);

	ImGui::Separator();

	ImGui::Columns(2);
	ImGui::Text("Address"); ImGui::NextColumn(); ImGui::Text("%p", lpPlayerData); ImGui::NextColumn();
	ImGui::Text("Airdashes remaining"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->nAirDashesRemaining); ImGui::NextColumn();
	ImGui::Text("Airjumps remaining"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->nAirJumpsRemaining); ImGui::NextColumn();
	ImGui::Text("Rakusyo bonus"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->receiveRakushoBonus); ImGui::NextColumn();
	ImGui::Text("Instant kill mode"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->isInstantKillMode); ImGui::NextColumn();
	ImGui::Text("Projectile thrown"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->IsProjectileThrown); ImGui::NextColumn();
	ImGui::Text("Character meter 1"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->characterMeter1); ImGui::NextColumn();
	ImGui::Text("Character meter 2"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->characterMeter2); ImGui::NextColumn();
	ImGui::Text("Num times clean hit"); ImGui::NextColumn(); ImGui::Text("%d", lpPlayerData->numTimesCleanHit); ImGui::NextColumn();
	ImGui::Text("Max stun before faint"); ImGui::NextColumn(); ImGui::Text("%d", maxFaint); ImGui::NextColumn();
	ImGui::Columns(1);

	ImGui::End();
}

void DrawActionLogWindow(GameObjectData* lpGameObject, bool* pOpen) {
	static int lastAction[2] = { 0, 0 };
	static ExampleAppLog actionLogs[2];

	if (lpGameObject->actNo != lastAction[lpGameObject->playerIndex]) {
		actionLogs[lpGameObject->playerIndex].AddLog("Action ID: %04X\n", lpGameObject->actNo);
		lastAction[lpGameObject->playerIndex] = lpGameObject->actNo;
	}

	actionLogs[lpGameObject->playerIndex].Draw(
		lpGameObject->playerIndex == 0 ? "Player 1 Action Log" : "Player 2 Action Log", 
		pOpen
	);
}

void DrawSaveLoadStateWindow(GameState* lpGameState, bool* pOpen) {
	static SavedGameState savedState;
	static int nFramesToSkipRender = 0;
	static int nMinSkip = 0;
	static int nMaxSkip = 60;
	static int nFrameStep = 1;

	ImGui::Begin("Save/Load State", pOpen, ImGuiWindowFlags_None);
	if (nFramesToSkipRender < 0) {
		nFramesToSkipRender = 0;
	}
	else if (nFramesToSkipRender > 60) {
		nFramesToSkipRender = 60;
	}

	ImGui::InputInt("Num frames to skip", &nFramesToSkipRender);
	for (int p = 0; p < 2; p++) {
		const char* headerLabel = p == 0 ? "P1 inputs during skip" : "P2 inputs during skip";
		PlayerData* lpPlayerData = &lpGameState->arrPlayerData[p];
		if (ImGui::CollapsingHeader(headerLabel)) {
			// left
			static int selected = 0;
			ImGui::BeginChild("left pane", ImVec2(150, 0), true);
			for (int i = 0; i < nFramesToSkipRender; i++)
			{
				char label[128];
				sprintf(label, "Frame %d", i);
				if (ImGui::Selectable(label, selected == i))
					selected = i;
			}
			ImGui::EndChild();
			ImGui::SameLine();

			// right
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
			ImGui::CheckboxFlags("Left", &lpGameState->arrInputsDuringFrameSkip[selected][p], Left); ImGui::NextColumn();
			ImGui::CheckboxFlags("Down", &lpGameState->arrInputsDuringFrameSkip[selected][p], Down); ImGui::NextColumn();
			ImGui::CheckboxFlags("Up", &lpGameState->arrInputsDuringFrameSkip[selected][p], Up); ImGui::NextColumn();
			ImGui::CheckboxFlags("Right", &lpGameState->arrInputsDuringFrameSkip[selected][p], Right); ImGui::NextColumn();
			ImGui::CheckboxFlags("P", &lpGameState->arrInputsDuringFrameSkip[selected][p], lpPlayerData->ctrlP); ImGui::NextColumn();
			ImGui::CheckboxFlags("K", &lpGameState->arrInputsDuringFrameSkip[selected][p], lpPlayerData->ctrlK); ImGui::NextColumn();
			ImGui::CheckboxFlags("S", &lpGameState->arrInputsDuringFrameSkip[selected][p], lpPlayerData->ctrlS); ImGui::NextColumn();
			ImGui::CheckboxFlags("H", &lpGameState->arrInputsDuringFrameSkip[selected][p], lpPlayerData->ctrlH); ImGui::NextColumn();
			ImGui::CheckboxFlags("D", &lpGameState->arrInputsDuringFrameSkip[selected][p], lpPlayerData->ctrlD); ImGui::NextColumn();
			ImGui::CheckboxFlags("Respect", &lpGameState->arrInputsDuringFrameSkip[selected][p], lpPlayerData->ctrlRespect); ImGui::NextColumn();
			ImGui::EndChild();
		}
	}

	if (ImGui::Button("Save")) {

		SaveGameState(lpGameState, &savedState);
	}

	if (ImGui::Button("Load")) {
		// This should probably trigger a load on the _next_ frame, or we're
		// likely to do something bad to graphics memory.
		lpGameState->nFramesSkipped = 0;
		lpGameState->nFramesToSkipRender = nFramesToSkipRender;
		LoadGameState(lpGameState, &savedState);
	}

	ImGui::End();
}

void DrawHelpWindow(bool* pOpen) {
	ImGui::Begin(
		"ImGui Help", 
		pOpen, 
		ImGuiWindowFlags_None
	);

	ImGui::ShowUserGuide();

	ImGui::End();
}

void DrawSaveLoadReplayWindow(GameState* lpGameState, bool* pOpen) {
	static ExampleAppLog saveloadreplay;
	static char* cLogpath = "./rec.acrec";
	static int prevRecStatus = 0;
	static int prevRecPlayer = 0;


	ImGui::Begin(
		"+R Recording Manager", 
		pOpen, 
		ImGuiWindowFlags_None
	);

	if (ImGui::Button("Read Config")) {
		saveloadreplay.Clear();
		saveloadreplay.AutoScroll = false;
		for (int p = 0; p < 2; p++) {
			saveloadreplay.AddLog("---P%X Button Config ---\n", p + 1);
			saveloadreplay.AddLog("P%i Punch = 0x%X\n", p+1,lpGameState->arrPlayerData[p].ctrlP);
			saveloadreplay.AddLog("P%i Kick = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlK);
			saveloadreplay.AddLog("P%i Slash = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlS);
			saveloadreplay.AddLog("P%i H-Slash = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlH);
			saveloadreplay.AddLog("P%i Dust = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlD);
			saveloadreplay.AddLog("P%i Respect = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlRespect);
			saveloadreplay.AddLog("P%i Reset = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlReset);
			saveloadreplay.AddLog("P%i Pause = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlPause);
			saveloadreplay.AddLog("P%i Rec Player = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlRecPlayer);
			saveloadreplay.AddLog("P%i Rec Enemy = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlRecEnemy);
			saveloadreplay.AddLog("P%i Play Memory = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlPlayMemory);
			saveloadreplay.AddLog("P%i Switch = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlSwitch);
			saveloadreplay.AddLog("P%i Enemy Walk = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlEnemyWalk);
			saveloadreplay.AddLog("P%i Enemy Jump = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlEnemyJump);
			saveloadreplay.AddLog("P%i P K = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlPKMacro);
			saveloadreplay.AddLog("P%i P D = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlPDMacro);
			saveloadreplay.AddLog("P%i P K S = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlPKSMacro);
			saveloadreplay.AddLog("P%i P K S H = 0x%X\n", p+1, lpGameState->arrPlayerData[p].ctrlPKSHMacro);
			saveloadreplay.AddLog("\n");
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Save")) {

		SaveRecording(cLogpath, lpGameState);
		saveloadreplay.AddLog("Save - Player %X recording saved to: %s\n", (lpGameState->recTarget->nPlayer + 1), cLogpath);

	}

	ImGui::SameLine();

	if (ImGui::Button("Load")) {

		LoadRecording(cLogpath, lpGameState);
		saveloadreplay.AddLog("Load - P%X recording loaded from: %s\n", (lpGameState->recTarget->nPlayer + 1), cLogpath);

	}

	ImGui::SameLine();

	if (ImGui::Button("Current Slot")) {
	
		saveloadreplay.AddLog("Slot - P%X recording currently loaded\n", (lpGameState->recTarget->nPlayer + 1));

	}

	//logs current Training Mode recording status
	if (*lpGameState->recStatus != prevRecStatus) {
		if (*lpGameState->recStatus == 0) {
			if (prevRecStatus == 1) {

				saveloadreplay.AddLog("State - Player Switch Cancelled \n");
				prevRecStatus = *lpGameState->recStatus;
			}
			if (prevRecStatus == 2) {

				saveloadreplay.AddLog("State - P%X Rec Standby Cancelled \n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;

			}
			if (prevRecStatus == 3) {

				saveloadreplay.AddLog("State - P%X Recording Stopped \n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;

			}
			if (prevRecStatus == 4) {

				saveloadreplay.AddLog("State - P%X Recording Playback Stopped \n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;

			}
		}
		if (*lpGameState->recStatus == 1) {

			saveloadreplay.AddLog("State - Switched to other Character\n");
			prevRecStatus = *lpGameState->recStatus;
		}
		if (*lpGameState->recStatus == 2) {


				saveloadreplay.AddLog("State - P%X Rec Standby\n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;
				prevRecPlayer = (lpGameState->recTarget->nPlayer + 1);

		}
		if (*lpGameState->recStatus == 3) {

				saveloadreplay.AddLog("State - P%X Rec Started\n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;

		}
		if (*lpGameState->recStatus == 4) {

				saveloadreplay.AddLog("State - P%X Rec Playback Started\n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;

		}
	}
	else {
		if(*lpGameState->recStatus != 0){
			if ((lpGameState->recTarget->nPlayer + 1) != prevRecPlayer) {

				saveloadreplay.AddLog("State - P%X Rec Standby\n", (lpGameState->recTarget->nPlayer + 1));
				prevRecStatus = *lpGameState->recStatus;
				prevRecPlayer = (lpGameState->recTarget->nPlayer + 1);

			}
		}
	}

	ImGui::Separator();

	saveloadreplay.Draw("+R Recording Manager", pOpen);

	ImGui::End();

}

void InitializeOverlay(GameState* lpGameState) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = DEFAULT_ALPHA;
	ImGui_ImplWin32_Init(*lpGameState->hWnd);
	ImGui_ImplDX9_Init(*lpGameState->d3dDevice);
}

void DrawOverlay(GameMethods* lpGameMethods, GameState* lpGameState) {
	static bool show_global_state = false;
	static bool show_p1_object_state = false;
	static bool show_p1_state = false;
	static bool show_p1_log = false;
	static bool show_p2_object_state = false;
	static bool show_p2_state = false;
	static bool show_p2_log = false;
	static bool show_saveload = false;
	static bool show_help = false;
	static bool show_save_load_replay = false;
	static bool load_config = false;

	if (!load_config) {
		ApplyConfiguration(lpGameState);
		load_config = true;
	}

	bool show_hitboxes = *lpGameState->bHitboxDisplayEnabled != 0;
	static bool show_ggpo_host = false;
	static bool show_ggpo_join = false;
	static bool show_character_select = false;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::IsMousePosValid() && ImGui::GetIO().MousePos.y < 200) {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Windows")) {
				ImGui::MenuItem("Global State", NULL, &show_global_state);

				if (ImGui::BeginMenu("GGPO")) {
					ImGui::MenuItem("Host...", NULL, &show_ggpo_host);
					ImGui::MenuItem("Join...", NULL, &show_ggpo_join);
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Player State")) {
					ImGui::MenuItem("Player 1 State", NULL, &show_p1_state);
					ImGui::MenuItem("Player 2 State", NULL, &show_p2_state);
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Object State")) {
					ImGui::MenuItem("Player 1 Object State", NULL, &show_p1_object_state, *lpGameState->arrCharacters != 0);
					ImGui::MenuItem("Player 1 Object Action Log", NULL, &show_p1_log, *lpGameState->arrCharacters != 0);
					ImGui::MenuItem("Player 2 Object State", NULL, &show_p2_object_state, *lpGameState->arrCharacters != 0);
					ImGui::MenuItem("Player 2 Object Action Log", NULL, &show_p2_log, *lpGameState->arrCharacters != 0);
					ImGui::EndMenu();
				}

				ImGui::MenuItem("Save/Load State", NULL, &show_saveload);
				ImGui::MenuItem("Save/Load Replay", NULL, &show_save_load_replay, *lpGameState->arrCharacters != 0);
				ImGui::MenuItem("Character select", NULL, &show_character_select);
				ImGui::EndMenu();
			}

			ImGui::MenuItem(
				show_hitboxes ? "Hide Hitboxes" : "Show Hitboxes",
				NULL,
				&show_hitboxes
			);

			ImGui::MenuItem(
				show_help ? "Hide Help" : "Show Help",
				NULL,
				&show_help
			);

			ImGui::EndMainMenuBar();
		}
	}
	if (show_character_select) {
		DrawEnterVersus2PWindow(lpGameState, &show_character_select);
	}
	if (show_global_state) {
		DrawGlobalStateWindow(lpGameState, &show_global_state);
	}
	if (show_p1_state) {
		DrawPlayerStateWindow("Player 1 State", &lpGameState->arrPlayerData[0], &show_p1_state);
	}
	if (show_p1_object_state) {
		DrawObjectStateWindow(&(*lpGameState->arrCharacters)[0], &show_p1_object_state);
	}
	if (show_p1_log) {
		DrawActionLogWindow(&(*lpGameState->arrCharacters)[0], &show_p1_log);
	}
	if (show_p2_state) {
		DrawPlayerStateWindow("Player 2 State", &lpGameState->arrPlayerData[1], &show_p2_state);
	}
	if (show_p2_object_state) {
		DrawObjectStateWindow(&(*lpGameState->arrCharacters)[1], &show_p2_object_state);
	}
	if (show_p2_log) {
		DrawActionLogWindow(&(*lpGameState->arrCharacters)[1], &show_p2_log);
	}
	if (show_help) {
		DrawHelpWindow(&show_help);
	}
	if (show_save_load_replay) {
		DrawSaveLoadReplayWindow(lpGameState, &show_save_load_replay);
	}
	if (show_hitboxes) {
		if (*lpGameState->bHitboxDisplayEnabled == 0) {
			EnableHitboxes(lpGameState);
		}
	}
	else {
		if (*lpGameState->bHitboxDisplayEnabled != 0) {
			DisableHitboxes(lpGameState);
		}
	}
	if (show_saveload) {
		DrawSaveLoadStateWindow(lpGameState, &show_saveload);
	}
	if (show_ggpo_host) {
		DrawGGPOHostWindow(lpGameState, &show_ggpo_host);
	}
	if (show_ggpo_join) {
		DrawGGPOJoinWindow(lpGameState, &show_ggpo_join);
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void FreeOverlay() {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void InvalidateImGuiDeviceObjects() {
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void CreateImGuiDeviceObjects() {
	ImGui_ImplDX9_CreateDeviceObjects();
}

LRESULT WINAPI OverlayWindowFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}
