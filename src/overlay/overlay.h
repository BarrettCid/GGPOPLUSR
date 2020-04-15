#pragma once

#include "../game/game.h"

void InitializeOverlay(GameState* lpGameState);
void DrawOverlay(GameMethods* lpGameMethods, GameState* lpGameState);
void FreeOverlay();
void InvalidateImGuiDeviceObjects();
void CreateImGuiDeviceObjects();

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
static bool show_mark_unlocks = false;
static bool load_config = false;
static bool show_ggpo_host = false;
static bool show_ggpo_join = false;
static bool show_character_select = false;
static bool show_character_data = false;
static bool show_position_reset = false;

LRESULT WINAPI OverlayWindowFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);