#include "commons.h"
#include "../dependencies/framework/include.h"
#include "bypass/eac.h"

//game sdk
#include "sdk/netvar_mgr.h"
#include "sdk/sdk.h"

//cheat sdk
#include "cheat/sdk.h"
#include "cheat/hooks/GameMovement.h"
#include "cheat/hooks/CreateMove.h"
#include "cheat/hooks/DirectX.h"


fxx_entry
{
	//VIRTUALIZER_MUTATE_ONLY_START
	if (r == 1)
	{
		//EAC::GetInstance().Init();
		//base init
		FC::SetSpoofStub(FindPattern(E("FF 27")));
		
		//get game funcs
		Fn::UpdateAbsOrigin = (PVOID)RVA(FindPattern(E("E8 ? ? ? ? F3 0F 10 76 ? F3 0F 5C")), 5);
		//Fn::MuzzlePos = (PVOID)FindPattern(E("40 53 48 83 EC 30 48 8B DA 8B 91"));
		Fn::RayInit = (PVOID)FindPattern(E("F3 41 0F 10 08 F3 0F 5C 0A F3 0F 11"));
		Fn::GetEntName = (PVOID)FindPattern(E("48 83 3D ? ? ? ? ? 74 08 8B"));
		Fn::UpdateAbsVelocity = (PVOID)FindPattern(E("40 57 48 83 EC 50 F7"));

		//get global vars
		Off::LocalPlayer = RVA(FindPattern(E("48 8D 0D ? ? ? ? 48 8B D7 FF 50 58")), 7) + 8;
		Off::EntityList = RVA(FindPattern(E("48 8D 05 ? ? ? ? 44 2B")), 7);
		
		//get interfaces
		I::CHLClient = (PVOID)RVA(FindPattern(E("48 8D 0D ? ? ? ? 48 89 0D ? ? ? ? 48 8D 05")), 7);
		I::SwapChain = *(IDXGISwapChain**)RVA(FindPattern(E("4C 8D 0D ? ? ? ? 2B 45")), 7);
		I::EngineTrace = *(PVOID*)(RVA(FindPattern(E("48 8B 0D ? ? ? ? 89 44 24 50")), 7));
		PVOID CInputClass = (PVOID)RVA(FindPattern(E("48 8D 0D ? ? ? ? 44 0F 29 48")), 7);
		PVOID CGameMoveClass = (PVOID)RVA(FindPattern(E("48 8D 0D ? ? ? ? 8B 83")), 7);
		
		//get offsets
		Off::PunchAngle = *(DWORD*)(FindPattern(E("E8 ? ? ? ? F2 0F 10 93 ? ? ? ? 0F 28 D8")) + 9);
		Off::StudioHdr = *(DWORD*)(FindPattern(E("48 83 B9 ? ? ? ? ? 48 8B D9 75 25")) + 3);
		Off::ViewRender = RVA(FindPattern(E("48 8B 05 ? ? ? ? 4C 8B 01 F3 0F 10 90")), 7);
		Off::AbsVelocity = GetOffset(E("DT_LocalPlayerExclusive"), E("m_vecAbsVelocity"));
		Off::ViewMatrix = *(DWORD*)(FindPattern(E("48 89 AB ? ? ? ? 4C 89 9B")) + 3);
		Off::ActiveWeapon = GetOffset(E("DT_Player"), E("m_latestPrimaryWeapons"));
		Off::BoneClass = GetOffset(E("DT_BaseEntity"), E("m_nForceBone")) + 0x50;
		Off::ScriptInt = GetOffset(E("DT_PropSurvival"), E("m_customScriptInt"));
		Off::PropName = GetOffset(E("DT_BaseEntity"), E("m_iSignifierName"));
		Off::Bleedout = GetOffset(E("DT_Player"), E("m_bleedoutState"));
		Off::LifeState = GetOffset(E("DT_Player"), E("m_lifeState"));
		Off::TeamNum = GetOffset(E("DT_Player"), E("m_iTeamNum"));
		Off::Health = GetOffset(E("DT_Player"), E("m_iHealth"));
		Off::Flags = GetOffset(E("DT_Player"), E("m_fFlags"));
		
		//game movement hook
		VMT_Hook CGameMove(nullptr, CGameMoveClass);
		CGameMove.SetupHook(&SilentEndOrg, SilentEnd, 5);
		CGameMove.SetupHook(&SilentStartOrg, SilentStart, 4);

		//cinput hook
		VMT_Hook CCreateMove(nullptr, CInputClass);
		CCreateMove.SetupHook(&CreateMoveOrg, CreateMoveHk, 4);
		
		//present hook
		BYTE ClassOffset = *(BYTE*)(FindPattern(E("8B ? ? ? ? ? 48 8B 01 48 8D 55"), E("dxgi")) + 2);
		VMT_Hook D3D(E("d3d11"), *(DWORD64*)((DWORD64)I::SwapChain + ClassOffset));
		D3D.SetupHook(&PresentOrg, Present, 13);
	}
	return true;
}