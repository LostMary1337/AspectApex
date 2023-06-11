#pragma once

#pragma warning(disable : 4530)
#pragma comment(linker,"/MERGE:.rdata=.text")
#pragma comment(linker,"/MERGE:.pdata=.text")
extern "C" int _fltused = 0;

#include <windows.h>
#include <Psapi.h>
#include <cstdint>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
#include <xmmintrin.h>
#include <algorithm>
#include <iomanip>
#include <random>
#include <stdint.h>
#include <winternl.h>
#include <intrin.h>
#include <stdio.h>
#include <d3d11.h>
#include <cfloat>

#include "utils/enc_str.h"
#include "utils/utils.h"
#include "utils/math.h"


namespace AimBot
{
	int AimbotFolder;
	int Active = true;
	int Silent = false; //
	int Nearest = true;
	int WeaponFix = true;
	int DrawFOV = true;
	int NoKnocked = true;
	int PredictType = false;
	int VisCheck = true;
	int UseCamera = true;

	int FOV = 20;
	int Smooth = 3;
	int Distance = 200;

	int AimKey = 1;
}

namespace Visuals
{
	int VisualsFolder;
	int ESP = true;
	int GlowESP = true;
	int LootESP = true;
	int DrawCrossHair = true;
	int NicksOutLine = false;
	int LootOutLine = false;
	int FilterOutAttachments = true;
	int FilterOutGrenades = true;
	int FilterOutArmor = false;
	int FilterOutMedical = false;
	int FilterOutGuns = false;
	int FilterOutMisc = false;
	int FilterOutAmmo = false;

	float DistanceESP = 280.f;
	float DistanceLoot = 100.f;
	int FilterLevel = 0;
}

namespace Misc
{
	int MiscFolder;
	int Bhop = true;
}

namespace Global {
	vec2_t WndSize;
	vec3_t LP_HeadPos;
	vec2_t MouseDelta;
}

//offsets
namespace Off
{
	//global vars
	DWORD64 ViewMatrix;
	DWORD64 ViewRender;
	DWORD64 EntityList;
	DWORD64 LocalPlayer;

	//dinamic
	DWORD Flags;
	DWORD Health;
	DWORD TeamNum;
	DWORD PropName;
	DWORD Bleedout;
	DWORD LifeState;
	DWORD BoneClass;
	DWORD StudioHdr;
	DWORD ScriptInt;
	DWORD PunchAngle;
	DWORD AbsVelocity;
	DWORD ShieldHealth;
	DWORD ActiveWeapon;

	//"CameraPosition"
	DWORD CameraPos = 0x1DA8;

	//"GetAbsOrigin"
	DWORD AbsVecOrgin = 0x14C;

	//"Speed(%f) is greater than sv_maxvelocity(%f)"
	DWORD BulletSpeed = 0x1D2C;
	DWORD BulletGravity = BulletSpeed + 0x8;

	//"PlayerRunCommand_Prediction"
	DWORD CameraAng = 0x23D0;
	DWORD DynamicAng = CameraAng - 0x10;
}

namespace Fn {
	PVOID RayInit;
	//PVOID MuzzlePos;
	PVOID GetEntName;
	//PVOID World2Screen;
	PVOID UpdateAbsOrigin;
	PVOID UpdateAbsVelocity;
}

namespace I {
	PVOID CHLClient;
	PVOID EngineTrace;
	IDXGISwapChain* SwapChain;
}

typedef struct LDR_DATA_TABLE_ENTRY_FIX {
	PVOID Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];
	PVOID DllBase;
	PVOID Reserved3[1];
	ULONG64 SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	PVOID Reserved5[2];
#pragma warning(push)
#pragma warning(disable: 4201)
	union {
		ULONG CheckSum;
		PVOID Reserved6;
	} DUMMYUNIONNAME;
#pragma warning(pop)
	ULONG TimeDateStamp;
};