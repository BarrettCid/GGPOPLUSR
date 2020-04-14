#pragma once

#include <windows.h>
#include <d3d9.h>
#include <dinput.h>
#include <ggponet.h>
#include <vdf_parser.hpp>

#include "scripting.h"

const unsigned short MIN_TENSION = 0;
const unsigned short MAX_TENSION = 10000;
const unsigned short MIN_BURST = 0;
const unsigned short MAX_BURST = 15000;
const unsigned short MIN_HEALTH = 00;
const unsigned short MAX_HEALTH = 460;
const short MAX_GUARD_BALANCE = 128;
const short MIN_GUARD_BALANCE = -128;
const int MIN_FAINT = 0;
const short MIN_NEGATIVE_PENALTY = 0;
const short MAX_NEGATIVE_PENALTY = 10000;
const int IP_BUFFER_SIZE = 32;

enum gameinputs : unsigned short {
	Up = 0x10,
	Right = 0x20,
	Down = 0x40,
	Left = 0x80,
	Respect = 0x400,
	Punch = 0x4000,
	Kick = 0x2000,
	Slash = 0x8000,
	HSlash = 0x200,
	Dust = 0x100,
    Reset = 0x1,
    Pause = 0x8
};

typedef enum TensionMode {
    TENSION_ATTACK_IK = 1,
    TENSION_NOMETER = 2,
    TENSION_NORMAL = 0
} TensionMode;

typedef enum HitboxTypes {
    HITBOXTYPEMASK_0 = 0,
    HITBOXTYPEMASK_16 = 16,
    HITBOXTYPEMASK_32 = 32,
    HITBOXTYPEMASK_64 = 64,
    HITBOXTYPEMASK_8 = 8,
    HITBOXTYPEMASK_HURTBOX = 2,
    HITBOXTYPEMASK_STRIKE = 1,
    HITBOXTYPEMASK_UNIQUEPUSHBOX = 4,
    _HITBOXTYPEMASK_PAD = 0xffffffff
} HitboxTypes;

typedef struct Hitbox {
    short xOffset;
    short yOffset;
    unsigned short width;
    unsigned short height;
    enum HitboxTypes type;
} Hitbox;

typedef struct SpriteInfo {
    short xOffset;
    short yOffset;
    unsigned int unknown;
    unsigned int nSpriteIndex;
} SpriteInfo;

typedef enum PlayerDataAllowedNormals {
    ALLNORMALS = 0xffffffff,

    NORMAL_5P = 1,
    NORMAL_6P = 2,
    NORMAL_5K = 4,
    NORMAL_fS = 8,
    NORMAL_cS = 16,
    NORMAL_5H = 32,
    NORMAL_6H = 64,
    NORMAL_2P = 256,
    NORMAL_2K = 512,
    NORMAL_2S = 1024,
    NORMAL_2H = 2048,
    NORMAL_JP = 4096,
    NORMAL_JK = 8192,
    NORMAL_JS = 16384,
    NORMAL_JH = 32768,
    NORMAL_6K = 131072,

} PlayerDataAllowedNormals;

typedef unsigned char   undefined;
typedef unsigned char    byte;
typedef long long    longlong;
typedef unsigned int    uint;
typedef unsigned long    ulong;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
typedef unsigned int    undefined4;
typedef struct GameObjectData GameObjectData;
typedef struct PlayerData PlayerData;
typedef struct SavedGameState SavedGameState;
typedef struct TrainingModeRec TrainingModeRec;

typedef struct RandomNumberGenerator RandomNumberGenerator, * PRandomNumberGenerator;

struct RandomNumberGenerator {
    int cursor;
    DWORD dwordarr_0x4_size0xe3[227];
    DWORD dwordarr_0x390_size0x18d[397];
    int unc_seedTracker;
    DWORD dwordarr_9c8_size0x270[624];
    int bIsInitialized;
};

typedef struct ClientSynchronizationRequest {
    unsigned short nPort;
    int nSelectedCharacter;
} ClientSynchronizationRequest;

typedef struct ServerSynchronizationResponse {
    unsigned short nPort;
    int nSelectedCharacter;
    RandomNumberGenerator RNG1;
    RandomNumberGenerator RNG2;
    RandomNumberGenerator RNG3;
} ServerSynchronizationResponse;

typedef struct CharacterSelection {
    char* name;
    int value;
} CharacterSelection;

static CharacterSelection CHARACTERS[] = {
    {"Sol Badguy", 0x1},
    {"Ky Kiske", 0x2},
    {"May", 0x3},
    {"Millia Rage", 0x4},
    {"Axl Low", 0x5},
    {"Potemkin", 0x6},
    {"Chipp Zanuff", 0x7},
    {"Eddie", 0x8},
    {"Baiken", 0x9},
    {"Faust", 0xa},
    {"Testament", 0xb},
    {"Jam Kuradoberi", 0xc},
    {"Anji Mito", 0xd},
    {"Johnny", 0xe},
    {"Venom", 0xf},
    {"Dizzy", 0x10},
    {"Slayer", 0x11},
    {"I-No", 0x12},
    {"Zappa", 0x13},
    {"Bridget", 0x14},
    {"Robo-Ky", 0x15},
    {"ABA", 0x16},
    {"Order-Sol", 0x17},
    {"Kliff Undersn", 0x18},
    {"Justice", 0x19}
};

typedef struct StageSelection {
    char* name;
    int value;
} StageSelection;

static StageSelection STAGES[] = {
    {"London", 0x1},
    {"Colony", 0x2},
    {"China", 0x4},
    {"Mayship", 0x5},
    {"Zepp", 0x6},
    {"Paris", 0x8},
    {"Hell", 0x9},
    {"Grove", 0xa},
    {"Verdant", 0xb},
    {"Castle", 0xc},
    {"Babylon", 0xd},
    {"Phantomcity", 0xe},
    {"Unknown [Boss I-No]", 0xf},
    {"Frasco", 0x10},
    {"AD2172", 0x11},
    {"Grave", 0x12},
    {"Heaven", 0x13},
    {"London (Reload)", 0x15},
    {"Colony (Reload)", 0x16},
    {"Russia (Reload)", 0x17},
    {"China (Reload)", 0x18},
    {"Mayship (Reload)", 0x19},
    {"Zepp (Reload)", 0x1a},
    {"Nirvana (Reload)", 0x1b},
    {"Paris (Reload)", 0x1c},
    {"Hell (Reload)", 0x1d},
    {"Grove (Reload)", 0x1e},
    {"Verdant (Reload)", 0x1f},
    {"Castle (Reload)", 0x20},
    {"Babylon (Reload)", 0x21},
    {"Phantomcity (Reload)", 0x22},
    {"Unknown [Boss I-No] (Reload)", 0x23},
    {"London (Slash)", 0x29},
    {"Colony (Slash)", 0x2a},
    {"Russia (Slash)", 0x2b},
    {"China (Slash)", 0x2c},
    {"Mayship (Slash)", 0x2d},
    {"Zepp (Slash)", 0x2e},
    {"Nirvana (Slash)", 0x2f},
    {"Paris (Slash)", 0x30},
    {"Hell (Slash)", 0x31},
    {"Grove (Slash)", 0x32},
    {"Verdant (Slash)", 0x33},
    {"Castle (Slash)", 0x34},
    {"Babylon (Slash)", 0x35},
    {"Phantomcity (Slash)", 0x36},
    {"Unknown [Boss I-No] (Slash)", 0x37},
    {"Frasco (Slash)", 0x38},
    {"AD2172 (Slash)", 0x39},
};

struct InputRewriteStruct {
    bool left;
    bool down;
    bool up;
    bool right;
    bool p;
    bool k;
    bool s;
    bool h;
    bool d;
    bool respect;
};

typedef struct GameMethods {
    void(WINAPI* GenerateAndShadePrimitives)();
    int(WINAPI* SetupD3D9)();
    bool(__cdecl* SteamAPI_Init)();
    LRESULT(WINAPI* WindowFunc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL(WINAPI* IsDebuggerPresent)();
    void(__cdecl* BeginSceneAndDrawGamePrimitives)(int bShouldBeginScene);
    void(WINAPI* DrawUIPrimitivesAndEndScene)();
    void(WINAPI* PollForInputs)();
    void(WINAPI* SimulateCurrentState)();
    void(WINAPI* CleanUpFibers)();
    void(WINAPI* HandlePossibleSteamInvites)();
    void(WINAPI* IncrementRNGCursorWhileOffline)();
    void(WINAPI* WaitForNextFrame)();
    void(WINAPI* MarkAllUnlocksOn)();
    void(WINAPI* MarkAllUnlocksOff)();
    void(WINAPI* PlayerGameObjectInitialization)();
    void(WINAPI* CheckForTrainingModeRestart)();
    void(WINAPI* AdjustCamera)();
} GameMethods;

typedef struct GGPOState {
    GGPOSession* ggpo;
    GGPOPlayer p1;
    GGPOPlayer p2;
    GGPOPlayer* localPlayer;
    GGPOPlayer* remotePlayer;
    GGPOPlayerHandle player_handles[2];
    GGPOSessionCallbacks cb;
    GGPOErrorCode lastResult;
    int localPlayerIndex;
    int characters[2];
    char bIsSynchronized;
    int nFramesAhead;
} GGPOState;

typedef struct SessionInitiationState {
    ClientSynchronizationRequest request;
    ServerSynchronizationResponse response;
    bool bHasRequest;
    bool bHasResponse;
    bool bIsHost;
    char szOpponentIP[32];
    CRITICAL_SECTION criticalSection;
    HANDLE hSyncThread;
} SessionInitializationState;

typedef struct PlayData {
    short* arrnFWalkVel;
    short* arrnBWalkVel;
    short* arrnFDashStartupSpeed;
    short* arrnBDashXVel;
    short* arrnBDashYVel;
    short* arrnBDashGravity;
    short* arrnFJumpXVel;
    short* arrnBJumpXVel;
    short* arrnJumpHeight;
    short* arrnGravity;
    short* arrnFSuperJumpXVel;
    short* arrnBSuperJumpXVel;
    short* arrnSuperJumpYVel;
    short* arrnSuperJumpGravity;
    short* arrnAirdashesGranted;
    short* arrnAirJumpsGranted;
    short* arrnFWalkTension;
    short* arrnFJumpAscentTension;
    short* arrnFDashTension;
    short* arrnFAirdashTension;
} PlayData;

typedef struct CharacterConstants {
    short* arrnStandingPushboxWidth;
    short* arrnVanillaStandingPushboxHeight;
    short* arrnPlusRStandingPushboxHeight;
    short* arrnCrouchingPushboxWidth;
    short* arrnCrouchingPushboxHeight;
    short* arrnAerialPushboxWidth;
    short* arrnAerialPushboxHeight;
    short* arrnVanillaAerialPushboxYOffset;
    short* arrnPlusRAerialPushboxYOffset;
    short* arrnCloseSlashMaxDistance;
    DWORD* arrnVanillaAllowedNormals;
    DWORD* arrnVanillaEXAllowedNormals;
    DWORD* arrnPlusRAllowedNormals;
    DWORD* arrnPlusREXAllowedNormals;
    short* arrnVanillaStandingThrowDistance;
    short* arrnPlusRStandingThrowDistance;
    short* arrnVanillaAerialThrowDistance;
    short* arrnPlusRAerialThrowDistance;
    short* arrnMinAerialThrowVerticalDifference;
    short* arrnMaxAerialThrowVerticalDifference;
} CharacterConstants;

typedef struct GameState {
    int nFramesToSkipRender;
    int nFramesSkipped;
    int lastSecondNumFramesSimulated;
    unsigned int arrInputsDuringFrameSkip[60][2];

    GGPOState ggpoState;
    SessionInitiationState sessionInitState;

	char* szConfigPath;
	tyti::vdf::object config;

    LPDIRECT3DSURFACE9* gameRenderTarget;
    LPDIRECT3DSURFACE9* uiRenderTarget;
    LPDIRECT3DDEVICE9* d3dDevice;
    HWND* hWnd;
    GameObjectData** arrCharacters;
    GameObjectData** arrNpcObjects;
    GameObjectData** arrEffectObjects;
    DWORD* bHitboxDisplayEnabled;

    unsigned int* nCameraHoldTimer;
    unsigned int* nCameraZoom;
    float* fCameraXPos;

    PlayerData* arrPlayerData;
    int* nRoundTimeRemaining;
    RandomNumberGenerator* lpRNG1;
    RandomNumberGenerator* lpRNG2;
    RandomNumberGenerator* lpRNG3;
    GameObjectData* inactiveNPCObjectPool_LinkedList;
    GameObjectData* activeEffectObjectPool_LinkedList;
    GameObjectData* activeNPCObjectPool_LinkedList;
    GameObjectData* inactiveEffectObjectPool_LinkedList;
    int* nPlayfieldLeftEdge;
    int* nPlayfieldTopEdge;
    int* nCameraPlayerXPositionHistory;
    int* nCameraPlayerXMovementMagnitudeHistory;
    WORD* arrnP1InputRingBuffer;
    WORD* arrnP2InputRingBuffer;
    int* nP1InputRingBufferPosition;
    int* nP2InputRingBufferPosition;
    unsigned int* nP1CurrentFrameInputs;
    unsigned int* nP2CurrentFrameInputs;
    TrainingModeRec* recTarget;
    int* recStatus;
    DWORD* recEnabled;
    DWORD* nSystemState;
    DWORD* nGameMode;
    WORD* arrnConfirmedCharacters;
    WORD* nCharacterSelectStageIndex;
    DWORD* nConfirmedStageIndex;
    DWORD* nUnknownIsPlayerActive1;
    DWORD* nUnknownIsPlayerActive2;
    WORD* arrbPlayerCPUValues;

    int* nPrimitivesDrawn;
    int* nNextPrimitiveBufferOffset;

    CharacterConstants characterConstants;
    PlayData playData;
} GameState;

void DisableHitboxes(GameState* gameState);
void EnableHitboxes(GameState* gameState);
void LoadGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort);
void LoadGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort, char* szHostIp);
void SaveGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort);
void SaveGGPOInfo(GameState* gameState, unsigned short& nSyncPort, unsigned short& nOurGGPOPort, char* szHostIp);
void EnterVersus2P(GameState* gameState, int* arrCharacters, StageSelection* stage);
void SaveGameState(GameState* gameState, SavedGameState* dest);
void LoadGameState(GameState* gameState, SavedGameState* src);
void SaveRecording(char* cLogpath, GameState* gameState);
void LoadRecording(char* cLogpath, GameState* gameState);
HMODULE LocatePERoot();
HRESULT LocateGameMethods(HMODULE peRoot, GameMethods* methods);
HRESULT LocateGameState(HMODULE peRoot, GameState* state);
HRESULT ApplyConfiguration(GameState* lpState);
void WINAPI FakeSimulateCurrentState();

HANDLE CreateSynchronizeClientThread(GameState* lpGameState,
    char* szHostIP,
    unsigned short nSyncPort,
    unsigned short nGGPOPort,
    int nOurCharacter);
HANDLE CreateSynchronizeServerThread(GameState* lpGameState, unsigned short nSyncPort, unsigned short nGGPOPort, int nOurCharacter);
void PrepareGGPOSession(GameState* lpGameState);

typedef unsigned short XInputButton;
const XInputButton XINPUTBTN_BACK = 0x0001;
const XInputButton XINPUTBTN_LS = 0x0002;
const XInputButton XINPUTBTN_RS = 0x0004;
const XInputButton XINPUTBTN_START = 0x0008;
const XInputButton XINPUTBTN_DPAD_UP = 0x0010;
const XInputButton XINPUTBTN_DPAD_RIGHT = 0x0020;
const XInputButton XINPUTBTN_DPAD_DOWN = 0x0040;
const XInputButton XINPUTBTN_DPAD_LEFT = 0x0080;
const XInputButton XINPUTBTN_LT = 0x0100;
const XInputButton XINPUTBTN_RT = 0x0200;
const XInputButton XINPUTBTN_LB = 0x0400;
const XInputButton XINPUTBTN_RB = 0x0800;
const XInputButton XINPUTBTN_Y = 0x1000;
const XInputButton XINPUTBTN_B = 0x2000;
const XInputButton XINPUTBTN_A = 0x4000;
const XInputButton XINPUTBTN_X = 0x8000;


struct GameObjectScriptingStruct {
    short field_0x0;
    short field_0x2;
    short field_0x4;
    short field_0x6;
    short attackStunValue;
    undefined field_0xa;
    undefined field_0xb;
    uint field_0xc;
    short field_0x10;
    short field_0x12;
    byte sound2[2];
    byte field_0x16[2];
    byte sound[2];
    byte field_0x1a[2];
    byte field_0x1c;
    byte field_0x1d;
    byte field_0x1e;
    byte unknRandomThresh;
    short attackPushback;
    short slipAmount;
    byte field_0x24;
    undefined field_0x25;
    undefined field_0x26;
    undefined field_0x27;
    int (*scriptingCallback)(struct GameObjectData*, uint*);
    short field_0x2c;
    short field_0x2e;
    short field_0x30;
    short field_0x32;
    undefined field_0x34;
    undefined field_0x35;
    undefined field_0x36;
    undefined field_0x37;
    short field_0x38;
    short field_0x3a;
    char field_0x3c;
    char field_0x3d;
    char field_0x3e;
    char field_0x3f;
    char field_0x40;
    char field_0x41;
    char field_0x42;
    char field_0x43;
    char field_0x44;
    undefined field_0x45;
    undefined field_0x46;
    undefined field_0x47;
    short field_0x48;
    short field_0x4a;
    short field_0x4c;
    short field_0x4e;
    short field_0x50;
    byte field_0x52;
    undefined field_0x53;
    undefined field_0x54;
    undefined field_0x55;
    undefined field_0x56;
    undefined field_0x57;
    undefined field_0x58;
    undefined field_0x59;
    undefined field_0x5a;
    undefined field_0x5b;
};

/**
 * Since the representation of poses in character resources involves a variable
 * length array right in the middle of it, we can't represent it as a struct.
 * Instead, just treat it as an opaque pointer and add methods that _can_ pull
 * structured data out of this opaque pointer.
 */
typedef void* LPPOSE;

typedef enum GameObjectID {
    GO_CHIPP = 7,
    GO_MILLIA_TANDEMTOP_SECRETGARDEN = 33,
    GO_SLAYER_UNK_5A = 90,
    GO_ZAPPA_DOG = 96,
    GO_FAUST_UNK_40 = 64,
    GO_ZAPPA = 19,
    GO_SLAYER_UNK_61 = 97,
    GO_POT_UNK_3D = 61,
    GO_FAUST_UNK_4A = 74,
    GO_MILLIA = 4,
    GO_ANJI = 13,
    GO_DIZZY = 16,
    GO_ROBOKY = 21,
    GO_SLAYER = 17,
    GO_KY = 2,
    GO_INO_UNK_5F = 95,
    GO_DIZZYICE_SCYTHE_SPIKE = 32,
    GO_SOL = 1,
    GO_ROBOKY_UNK_62 = 98,
    GO_JOHNNY_BACCHUS = 63,
    GO_ROBOKY_UNK_63 = 99,
    GO_MAY_UNK_3E = 62,
    GO_POTEMKIN = 6,
    GO_JAM_UNK_2F = 47,
    GO_ZAPPA_GHOSTS = 91,
    GO_JAM_UNK_30 = 48,
    GO_ORDERSOL = 23,
    GO_FAUST = 10,
    GO_KLIFF = 24,
    GO_MAY_UNK_37 = 55,
    GO_CHIPP_UNK_3C = 60,
    GO_INO = 18,
    GO_YAMADASAN = 70,
    GO_ZAPPA_GHOSTITEMS = 89,
    GO_FAUSTITEMS_2B = 43,
    GO_BRIDGET_ROGER_YOYO = 92,
    GO_EDDIE_DRILL = 65,
    GO_CHIPP_UNK_45 = 69,
    GO_BAIKEN = 9,
    GO_SLAYER_UNK_58 = 88,
    GO_ANJI_UNK_4C = 76,
    GO_ANJI_UNK_4E = 78,
    GO_MILLIA_HAIRPIN = 46,
    GO_ABA_UNK_64 = 100,
    GO_ABA_UNK_65 = 101,
    GO_ABA_UNK_66 = 102,
    GO_JUSTICE_UNK_6E = 110,
    GO_LITTLE_EDDIE = 35,
    GO_DIZZY_UNK_24 = 36,
    GO_MILLIA_UNK_34 = 52,
    GO_SOL_FLAMES_VV_GF = 44,
    GO_JUSTICE_UNK_6F = 111,
    GO_AXL_UNK_51 = 81,
    GO_VENOM_UNK_31 = 49,
    GO_ZAPPA_RAOU = 87,
    GO_VENOM_UNK_32 = 50,
    GO_AXL_UNK_4D = 77,
    GO_KLIFF_UNK_6C = 108,
    GO_VENOM_UNK_35 = 53,
    GO_VENOM_UNK_36 = 54,
    GO_JOHNNY_UNK_47 = 71,
    GO_VENOM_UNK_38 = 56,
    GO_POLYGONAL_HIT_EFFECT = 142,
    GO_DIZZYFISH_ICESPEAR_BUBBLE_IMPRAY = 67,
    GO_JAM = 12,
    GO_AXL = 5,
    GO_MAY_UNK_48 = 72,
    GO_JAM_UNK_27 = 39,
    GO_JAM_UNK_26 = 38,
    GO_JOHNNY_UNK_55 = 85,
    GO_ZAPPA_SWORD = 93,
    GO_BAIKEN_UNK_3A = 58,
    GO_JAM_UNK_25 = 37,
    GO_JAM_UNK_2A = 42,
    GO_KY_LIGHTNING_STUNEDGE_JD = 41,
    GO_MAY_UNK_4B = 75,
    GO_BAIKEN_UNK_39 = 57,
    GO_HOS_UNK_68 = 104,
    GO_HOS_UNK_67 = 103,
    GO_EDDIE = 8,
    GO_INO_UNK_56 = 86,
    GO_MAY = 3,
    GO_ABA = 22,
    GO_TESTAMENT = 11,
    GO_TESTAMENT_UNK_4F = 79,
    GO_SOL_UNK_42 = 66,
    GO_CHIPP_GAMMABLADE = 40,
    GO_STAGEOBJECT_3 = 178,
    GO_STAGEOBJECT_4 = 179,
    GO_CHIPP_SHURIKEN = 59,
    GO_STAGEOBJECT_1 = 176,
    GO_STAGEOBJECT_2 = 177,
    GO_STAGEOBJECT_5 = 180,
    GO_TESTAMENT_UNK_54 = 84,
    GO_VENOM_UNK_2D = 45,
    GO_TESTAMENT_UNK_50 = 80,
    GO_TESTAMENT_UNK_52 = 82,
    GO_JOHNNY = 14,
    GO_IRUKASAN = 68,
    GO_BRIDGET = 20,
    GO_VENOM_CUEBALLS = 34,
    GO_POT_UNK_49 = 73,
    GO_JUSTICE = 25,
    GO_VENOM = 15,
    GO_ANJI_UNK_53 = 83,
    GO_BRIDGET_UNK_5E = 94,
    GO_FORCEWORD = 0xffff
} GameObjectID;

typedef enum GameObjectStateFlags {
    CSF_SLIP = 33554432,
    CSF_REAP = 2147483648,
    CSF_HITSTOP = 524288,
    CSF_AERIAL = 16,
    CSF_TRAPTOWORLD = 262144,
    CSF_BLOCKSTUN = 512,
    CSF_HITSTUN = 32,
    CSF_CORNERED = 2048,
    CSF_BURST_DISABLED = 16384,
    CSF_SKIPOBJ = 1073741824,
    CSF_REVERSAL = 32768,
    CSF_TRAPTOPLAYFIELD = 16777216,
    CSF_CROUCHING = 1024
} GameObjectStateFlags;

typedef enum GameObjectGraphicalEffectsFlags {
    CE_COUNTERHIT = 16777216,
    CE_FLAME = 4096,
    CE_CHIPPINVIS = 65536,
    CE_RESET_PALETTE_AFTER_EFFECT = 1024,
    CE_FAUST_POISON_TESTAMENTOD = 1048576,
    CE_RCFLASH = 512,
    CE_SHADOWPALETTE = 262144,
    CE_BAKU_P_ABAGOKU_JUSTICEOMEGA = 524288,
    CE_EASYFRC_BLUEFLASH = 67108864,
    CE_FIXEDVERTICALSCALE = 1,
    CE_BAKU_K = 268435456,
    CE_THUNDER = 8192,
    CE_NOSHADOW = 131072,
    CE_BAKU_S = 536870912,
    CE_INVIS2 = 8,
    CE_DRAGONINSTALL = 16384,
    CE_RESET_PALETTE_EFFECT = 0x400,
    _CE_FORCE_DWORD = 0xffffffff
} GameObjectGraphicalEffectsFlags;
DEFINE_ENUM_FLAG_OPERATORS(GameObjectGraphicalEffectsFlags)

typedef enum GameObjectAttackInformationFlags {
    AI_FLAGSOFF = 4294967295,
    AI_FLAME = 262144,
    AI_KNOCKDOWN = 16,
    AI_DUSTHOMING = 512,
    AI_UNKLAUNCH = 128,
    AI_SMALLHITEFFECT = 1,
    AI_SKIPEFFECTS = 64,
    AI_THUNDER = 524288
} GameObjectAttackInformationFlags;

typedef struct GameObjectAttackInformation {
    GameObjectAttackInformationFlags flags;
    unsigned short unc_LS3BattackLevel;
    byte damage;
    byte unc_collisionMask;
} GameObjectAttackInformation;

struct GameObjectData {
    WORD objectID;
    char facing;
    char side;
    struct GameObjectData* lpPrevObject;
    struct GameObjectData* lpNextObject;
    GameObjectStateFlags stateFlags;
    int queuedAction;
    DWORD unc_upcomingAction;
    WORD actNo;
    WORD field_0x1a;
    short field_0x1c;
    short field_0x1e;
    struct GameObjectData* lpParent;
    undefined field_0x24;
    undefined field_0x25;
    undefined field_0x26;
    char playerIndex;
    unsigned short field_0x28;
    WORD field_0x2a;
    struct PlayerData* playerData;
    void (*UNC_EffectDrawCallback)(struct GameObjectData*);
    DWORD field_0x34;
    DWORD field_0x38;
    undefined field_0x3c;
    undefined field_0x3d;
    undefined field_0x3e;
    undefined field_0x3f;
    unsigned short nPoseIndex;
    undefined field_0x42;
    undefined field_0x43;
    unsigned short nSpriteIndex;
    undefined field_0x46;
    undefined field_0x47;
    LPPOSE* arrlpPoses;
    short field_0x4c;
    undefined field_0x4e;
    undefined field_0x4f;
    short preservedAspectScalingFactor;
    short verticalScalingFactor;
    Hitbox* arrlpCurrentHitboxes;
    undefined field_0x58;
    undefined field_0x59;
    undefined field_0x5a;
    undefined field_0x5b;
    byte field_0x5c;
    byte field_0x5d;
    byte field_0x5e;
    undefined field_0x5f;
    WORD field_0x60;
    undefined field_0x62;
    undefined field_0x63;
    struct GameObjectData* lpVictim;
    undefined field_0x68;
    undefined field_0x69;
    undefined field_0x6a;
    undefined field_0x6b;
    struct GameObjectData* lpAttacker;
    int unc_xPosMemory;
    int unc_yPosMemory;
    DWORD field_0x78;
    char attackerScriptVariables[4];
    undefined field_0x80;
    undefined field_0x81;
    undefined field_0x82;
    undefined field_0x83;
    byte numHitboxes;
    undefined field_0x85;
    short field_0x86;
    struct GameObjectScriptingStruct* scriptingStruct;
    struct GameObjectScriptingStruct* scriptingStruct2;
    undefined field_0x90;
    undefined field_0x91;
    undefined field_0x92;
    undefined field_0x93;
    void (*resetPaletteCallback)(struct GameObjectData*);
    short d3dTextureIndex; /* Created by retype action */
    WORD field_0x9a;
    short textureMetadataOffsetStart;
    short field_0x9e;
    short textureMetadataLength;
    short imageNo; /* Created by retype action */
    int field_0xa4;
    undefined field_0xa8;
    undefined field_0xa9;
    undefined field_0xaa;
    undefined field_0xab;
    GameObjectGraphicalEffectsFlags dwGraphicalEffects;
    int xPos;
    int ypos;
    int xvel;
    int yvel;
    undefined field_0xc0;
    undefined field_0xc1;
    undefined field_0xc2;
    undefined field_0xc3;
    undefined field_0xc4;
    undefined field_0xc5;
    undefined field_0xc6;
    undefined field_0xc7;
    DWORD field_0xc8;
    DWORD field_0xcc;
    int field_0xd0;
    DWORD gravity;
    undefined field_0xd8;
    undefined field_0xd9;
    undefined field_0xda;
    undefined field_0xdb;
    undefined field_0xdc;
    undefined field_0xdd;
    short field_0xde;
    DWORD** scriptingData;
    struct GameObjectAttackInformation attackInfo_228;
    DWORD* scriptStart;
    DWORD* instructionPointer;
    short flags_0xf4;
    undefined field_0xf6;
    undefined field_0xf7;
    WORD nScriptID;
    WORD nPoseFramesElapsed;
    char nFramesToHoldPose;
    byte field_0xfd;
    undefined field_0xfe;
    undefined field_0xff;
    undefined field_0x100;
    undefined field_0x101;
    undefined field_0x102;
    undefined field_0x103;
    undefined field_0x104;
    undefined field_0x105;
    undefined field_0x106;
    undefined field_0x107;
    undefined field_0x108;
    undefined field_0x109;
    byte field_0x10a;
    undefined field_0x10b;
    undefined field_0x10c;
    undefined field_0x10d;
    undefined field_0x10e;
    undefined field_0x10f;
    undefined field_0x110;
    undefined field_0x111;
    undefined field_0x112;
    undefined field_0x113;
    undefined field_0x114;
    undefined field_0x115;
    undefined field_0x116;
    undefined field_0x117;
    undefined field_0x118;
    undefined field_0x119;
    undefined field_0x11a;
    undefined field_0x11b;
    undefined field_0x11c;
    undefined field_0x11d;
    undefined field_0x11e;
    undefined field_0x11f;
    undefined field_0x120;
    undefined field_0x121;
    undefined field_0x122;
    undefined field_0x123;
    undefined field_0x124;
    undefined field_0x125;
    undefined field_0x126;
    undefined field_0x127;
    undefined field_0x128;
    undefined field_0x129;
    undefined field_0x12a;
    undefined field_0x12b;
    undefined field_0x12c;
    undefined field_0x12d;
    undefined field_0x12e;
    undefined field_0x12f;
};

struct PlayerData {
    short currentTension;
    byte field_0x2;
    byte field_0x3;
    unsigned short field_0x4;
    short negativePenaltyCounter;
    undefined field_0x8;
    undefined field_0x9;
    WORD field_0xa;
    undefined field_0xc;
    undefined field_0xd;
    undefined field_0xe;
    undefined field_0xf;
    int unc_RunningXVel;
    int gameObjectXOffset;
    undefined field_0x18;
    undefined field_0x19;
    short guardBalance; /* SHORT, not unsigned short. Has negative values. */
    undefined field_0x1c;
    undefined field_0x1d;
    unsigned short field_0x1e;
    undefined field_0x20;
    undefined field_0x21;
    undefined field_0x22;
    undefined field_0x23;
    undefined field_0x24;
    undefined field_0x25;
    undefined field_0x26;
    undefined field_0x27;
    undefined field_0x28;
    undefined field_0x29;
    undefined field_0x2a;
    byte field_0x2b;
    undefined field_0x2c;
    undefined field_0x2d;
    undefined field_0x2e;
    undefined field_0x2f;
    short field_0x30;
    char nFRCWindow;
    char field_0x33;
    char field_0x34;
    bool receiveRakushoBonus; /* Created by retype action */
    undefined field_0x36;
    undefined field_0x37;
    enum PlayerDataAllowedNormals allowedNormals;
    undefined field_0x3c;
    undefined field_0x3d;
    undefined field_0x3e;
    undefined field_0x3f;
    void* field_0x40;
    struct GameObjectData* otherPlayerGameObject;
    XInputButton ctrlP;
    XInputButton ctrlK;
    XInputButton ctrlS;
    XInputButton ctrlH;
    XInputButton ctrlD;
    XInputButton ctrlRespect;
    XInputButton ctrlReset;
    XInputButton ctrlPause;
    XInputButton ctrlRecPlayer;
    XInputButton ctrlRecEnemy;
    XInputButton ctrlPlayMemory;
    XInputButton ctrlSwitch;
    XInputButton ctrlEnemyWalk;
    XInputButton ctrlEnemyJump;
    XInputButton ctrlPKMacro;
    XInputButton ctrlPDMacro;
    XInputButton ctrlPKSMacro;
    XInputButton ctrlPKSHMacro;
    unsigned short currentHealth;
    unsigned short redHealth;
    byte nAirJumpsRemaining;
    byte nAirDashesRemaining;
    WORD field_0x72;
    undefined field_0x74;
    undefined field_0x75;
    undefined field_0x76;
    undefined field_0x77;
    undefined field_0x78;
    undefined field_0x79;
    undefined field_0x7a;
    undefined field_0x7b;
    short currentFaint;
    undefined field_0x7e;
    undefined field_0x7f;
    void* unc_funcPtr9;
    undefined field_0x84;
    undefined1 field_0x85;
    undefined field_0x86;
    byte maxFaint;
    unsigned short characterMeter1; /* Johnny: Mist finer level */
    short field_0x8a;
    WORD characterMeter2; /* Johnny: Number of coins left, Eddie: Eddie gauge */
    short field_0x8e;
    undefined field_0x90;
    undefined field_0x91;
    undefined field_0x92;
    undefined field_0x93;
    short field_0x94;
    undefined field_0x96;
    undefined field_0x97;
    undefined field_0x98;
    undefined field_0x99;
    undefined field_0x9a;
    undefined field_0x9b;
    undefined field_0x9c;
    undefined field_0x9d;
    undefined field_0x9e;
    undefined field_0x9f;
    undefined field_0xa0;
    undefined field_0xa1;
    undefined field_0xa2;
    undefined field_0xa3;
    undefined field_0xa4;
    undefined field_0xa5;
    undefined field_0xa6;
    undefined field_0xa7;
    undefined field_0xa8;
    undefined field_0xa9;
    undefined field_0xaa;
    undefined field_0xab;
    DWORD field_0xac;
    undefined field_0xb0;
    undefined field_0xb1;
    undefined field_0xb2;
    undefined field_0xb3;
    void (*unc_hitCallback1)(struct GameObjectData*);
    void (*unc_hitCallback2)(struct GameObjectData*);
    void* unc_funcPtrs3;
    void (*landingCallback)(struct GameObjectData*);
    void* unc_funcPtrs5;
    void* unc_funcPtrs6;
    void* unc_funcPtrs7;
    void* unc_funcPtrs8;
    short* UNC_ThrowOffsetPointer; /* Created by retype action */
    byte tensionMode;
    undefined1 IsProjectileThrown; /* Created by retype action */
    undefined field_0xda;
    undefined field_0xdb;
    DWORD field_0xdc;
    undefined field_0xe0;
    undefined field_0xe1;
    undefined field_0xe2;
    undefined field_0xe3;
    undefined field_0xe4;
    undefined field_0xe5;
    undefined field_0xe6;
    undefined field_0xe7;
    undefined field_0xe8;
    undefined field_0xe9;
    undefined field_0xea;
    undefined field_0xeb;
    undefined field_0xec;
    undefined field_0xed;
    byte field_0xee;
    undefined field_0xef;
    byte field_0xf0;
    byte field_0xf1;
    byte field_0xf2;
    byte field_0xf3;
    byte field_0xf4;
    byte field_0xf5;
    undefined field_0xf6;
    undefined field_0xf7;
    unsigned short currentBurst;
    undefined field_0xfa;
    undefined field_0xfb;
    undefined field_0xfc;
    undefined field_0xfd;
    undefined field_0xfe;
    undefined field_0xff;
    short field_0x100;
    byte baikenBakuPEffectCounter;
    byte baikenBakuKEffectCounter;
    byte baikenBakuSEffectCounter;
    byte bakuPDisableCounter;
    byte bakuKDisableCounter;
    byte bakuSDisableCounter;
    byte baikenHDisableCounter;
    byte baikenDDisableCounter;
    undefined field_0x10a;
    undefined field_0x10b;
    undefined field_0x10c;
    undefined field_0x10d;
    undefined field_0x10e;
    undefined field_0x10f;
    undefined field_0x110;
    undefined field_0x111;
    byte field_0x112;
    undefined field_0x113;
    undefined field_0x114;
    byte field_0x115;
    undefined field_0x116;
    undefined field_0x117;
    int gameObjectYOffset;
    int numTimesCleanHit;
    int field_0x120;
    undefined field_0x124;
    undefined field_0x125;
    undefined field_0x126;
    undefined field_0x127;
    undefined field_0x128;
    undefined field_0x129;
    undefined field_0x12a;
    undefined field_0x12b;
    undefined field_0x12c;
    undefined field_0x12d;
    undefined field_0x12e;
    undefined field_0x12f;
    undefined field_0x130;
    undefined field_0x131;
    undefined field_0x132;
    undefined field_0x133;
    undefined field_0x134;
    undefined field_0x135;
    undefined field_0x136;
    undefined field_0x137;
    undefined field_0x138;
    undefined field_0x139;
    undefined field_0x13a;
    undefined field_0x13b;
    undefined field_0x13c;
    undefined field_0x13d;
    undefined field_0x13e;
    undefined field_0x13f;
    undefined field_0x140;
    undefined field_0x141;
    undefined field_0x142;
    undefined field_0x143;
    short recoveryFrame;
    undefined field_0x146;
    undefined field_0x147;
};

typedef struct SavedGameState {
    struct GameObjectData arrCharacters[2];
    struct GameObjectData arrNpcObjects[0x60];
    struct GameObjectData arrEffectObjects[0x180];
    DWORD bHitboxDisplayEnabled;

    unsigned int nCameraHoldTimer;
    unsigned int nCameraZoom;
    float fCameraXPos;

    struct PlayerData arrPlayerData[2];
    int nRoundTimeRemaining;
    RandomNumberGenerator RNG1;
    RandomNumberGenerator RNG2;
    RandomNumberGenerator RNG3;

    struct GameObjectData inactiveNPCObjectPool_LinkedList;
    struct GameObjectData activeEffectObjectPool_LinkedList;
    struct GameObjectData activeNPCObjectPool_LinkedList;
    struct GameObjectData inactiveEffectObjectPool_LinkedList;
    int nPlayfieldLeftEdge;
    int nPlayfieldTopEdge;

    int nCameraPlayerXPositionHistory[2];
    int nCameraPlayerXMovementMagnitudeHistory[2];

    WORD arrnP1InputRingBuffer[32];
    WORD arrnP2InputRingBuffer[32];
    int nP1InputRingBufferPosition;
    int nP2InputRingBufferPosition;
} SavedGameState;

struct Inputs {
    byte nDirection;
    byte nButton;
    unsigned short sPad;

};
typedef struct TrainingModeRec {
    byte nPlayer;
    byte nUnknown[3];
    struct Inputs RecInputs[3599];
} TrainingModeRec;

unsigned int translateFromNormalizedInput(unsigned int normalizedInput, int p, GameState* g_lpGameState);
unsigned int normalizeInput(unsigned int* input, GameState* g_lpGameState);