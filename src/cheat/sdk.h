#define Ent(Id) (*(CBasePlayer**)(Off::EntityList + (Id << 5)))
#define LP (*(LPlayer**)Off::LocalPlayer)

enum EntType {
	Player,
	LootEnt,
	NPC
};

class CBasePlayer
{
private:
	//get bone by hitbox
	int BoneByHitBox(int HitBox)
	{
		//get model ptr
		DWORD64 Model = *(DWORD64*)(this + Off::StudioHdr);
		if (!Model) return -1;

		//get studio hdr
		DWORD64 StudioHdr = *(DWORD64*)(Model + 8);
		if (!StudioHdr) return -1;

		//get hitbox array
		DWORD64 HitBoxsArray = StudioHdr + *(int*)(StudioHdr + 0xB4);
		if (!HitBoxsArray) return -1;

		//get bone index
		int Bone = *(int*)(HitBoxsArray + *(int*)(HitBoxsArray + 8) + (HitBox * 0x2C));
		return ((Bone < 0) || (Bone > 255)) ? -1 : Bone;
	}

public:
	//get current health
	int Health() {
		if (!this) return 0;
		return *(int*)(this + Off::Health);
	}

	//get ent state
	DWORD Flags() {
		if (!this) return 0;
		return *(DWORD*)(this + Off::Flags);
	}

	//is knocked out?
	bool Knocked() {
		if (!this) return true;
		return (*(int*)(this + Off::Bleedout) != 0);
	}
	
	//get feet position
	vec3_t AbsOrgin(bool Update = false) {
		if (!this) return vec3_t{ 0.f, 0.f, 0.f };
		if (Update) SpoofCall(Fn::UpdateAbsOrigin, this);
		return *(vec3_t*)(this + Off::AbsVecOrgin);
	}

	//get player speed
	vec3_t AbsVelocity(bool Update = false) {
		if (!this) return vec3_t();
		typedef void(__fastcall* GetAbsFn)(PVOID);
		if (Update) SpoofCall(Fn::UpdateAbsVelocity, this);
		return *(vec3_t*)(this + Off::AbsVelocity);
	}

	//get script id
	int ScriptID() {
		if (!this) return -1;
		return *(int*)(this + Off::ScriptInt);
	}

	//get player name
	const char* PlayerName() {
		if (!this) return nullptr;
		return SpoofCall<const char*>(Fn::GetEntName, this);
	}

	//get camera position
	vec3_t CameraPos() {
		if (!this) return vec3_t();
		return *(vec3_t*)(this + Off::CameraPos);
	}

	//get alive state
	bool Alive() {
		if (!this) return false;
		return bool(*(int*)(this + Off::LifeState) == 0);
	}

	//check entity type
	bool Type(EntType Type)
	{
		//basic check's
		CBasePlayer* LPlayer = *(CBasePlayer**)Off::LocalPlayer;
		if (!this || !LPlayer || (this == LPlayer)) return false;

		//get prop hash
		DWORD64 PropHash = *(DWORD64*)(this + Off::PropName); 
		if (!PropHash) return false; PropHash = *(DWORD64*)PropHash;

		//check is loot
		if (Type == LootEnt)
			return (PropHash == 0x7275735F706F7270);

		/*else if (Type == NPC) {
			const char* PropName = *(const char**)(this + Off::PropName);
			return FC::StrCmp(E("npc_dummie"), PropName, true);
		}*/

		//check if player
		else if ((PropHash == 0x726579616C70) && Alive()) {
			return (*(int*)(this + Off::TeamNum) != *(int*)((DWORD64)LPlayer + Off::TeamNum));
		}

		//ent invalid
		return false;
	}

	//glow esp
	void Glow(const vec3_t& Color)
	{
		//Highlight_SetCurrentContext
		*(int*)(this + 0x310) = 1;

		//Highlight_ShowOutline
		*(bool*)(this + 0x380) = true;

		//Highlight_GetFarFadeDist
		*(float*)(this + 0x2FC) = FLT_MAX;

		//set color
		*(vec3_t*)(this + 0x1D0) = Color;

		//Highlight_ShowInside
		*(float*)(this + 0x2D0) = FLT_MAX;
		*(float*)(this + 0x2D8) = FLT_MAX;
		*(float*)(this + 0x2E0) = FLT_MAX;

		//Highlight_ShowOutline
		*(float*)(this + 0x2D4) = FLT_MAX;
		*(float*)(this + 0x2DC) = FLT_MAX;
		*(float*)(this + 0x2E4) = FLT_MAX;

		//Highlight_SetLifeTime
		*(float*)(this + 0x2E8) = FLT_MAX;
	}

	//get bone position
	vec3_t HitBoxPos(int HitBox, bool UpdateBones)
	{
		//get bones base data
		DWORD64 Bones = *(DWORD64*)(this + Off::BoneClass); 
		if (!Bones) return vec3_t(); vec3_t BoneOff;
		int Bone = BoneByHitBox(HitBox);
		if (Bone == -1) return vec3_t();

		//use cached bones
		if (!UpdateBones) {
			matrix3x4_t* BoneMatrix = (matrix3x4_t*)(Bones + (Bone * sizeof(matrix3x4_t)));
			BoneOff = { BoneMatrix->_14, BoneMatrix->_24, BoneMatrix->_34 };
		}

		else 
		{ 
			//get updated bone matrix
			static matrix3x4_t MatrixArray[256];
			if (!VCall<bool>((PVOID)(this + 0x10), 16, MatrixArray, 256, 0xFFFF00, 0.f)) 
				return vec3_t();

			//get bone by hitbox
			BoneOff = { MatrixArray[Bone]._14, MatrixArray[Bone]._24, MatrixArray[Bone]._34 };
		}
		
		//build bone pos
		return AbsOrgin(UpdateBones) + BoneOff;
	}

	//check visible bone
	bool VisiblePos(CBasePlayer* LPlayer, vec3_t& Pos) {
		if (!this || !LPlayer) return false;
		Ray RayData(LPlayer->CameraPos(), Pos); TraceData TraceOut;
		VCall(I::EngineTrace, 3, RayData.RayRaw, 0x640400B, nullptr, &TraceOut);
		return (TraceOut.m_pEnt == this || TraceOut.fraction > 0.97f);
	}
};

class Weapon 
{
public:
	float BulletSpeed() {
		if (!this) return 0.f;
		return *(float*)(this + Off::BulletSpeed);
	}

	float BulletGravity() {
		if (!this) return 0.f;
		return *(float*)(this + Off::BulletGravity);
	}

	/*vec3_t MuzzlePos() {
		vec3_t Pos; if (!this) return Pos;
		SpoofCall(Fn::MuzzlePos, this, &Pos);
		return Pos;
	}*/
};

class LPlayer : public CBasePlayer
{
public:
	Weapon* ActiveWeapon() {
		if (!this) return nullptr;
		DWORD64 ActWeaponId = *(DWORD64*)(this + Off::ActiveWeapon) & 0xFFFF;
		return (ActWeaponId ? (Weapon*)Ent(ActWeaponId) : nullptr);
	}

	vec3_t PunchAngle() {
		if (!this) return vec3_t{};
		return *(vec3_t*)(this + Off::PunchAngle);
	}

	__forceinline vec3_t StaticAngles() {
		if (!this) return vec3_t{};
		return *(vec3_t*)(this + Off::CameraAng);
	}

	__forceinline void StaticAngles(const vec3_t& Ang) {
		if (!this) return;
		*(vec3_t*)(this + Off::CameraAng) = Ang;
	}

	vec3_t DynamicAngles() {
		if (!this) return vec3_t{};
		return *(vec3_t*)(this + Off::DynamicAng);
	}
};

class CEntityList
{
public:
	CBasePlayer* Ent;
private:
	int SerialNumber;
	CEntityList* PrevEnt;
	CEntityList* NextEnt;

public:
	__forceinline CEntityList* Start() {
		CEntityList* EntInfo = *(CEntityList**)(Off::EntityList + 0x200000);
		if (EntInfo) { if (EntInfo->Ent) return EntInfo; else EntInfo->Next(); }
		return nullptr;
	}

	__forceinline CEntityList* Next()
	{
		CEntityList* NextEntry = NextEnt;

		GetValidLoop: 
		if (NextEntry) {
			if (NextEntry->Ent) 
				return NextEntry;

			else {
				NextEntry = NextEntry->NextEnt;
				goto GetValidLoop;
			}
		} 
		
		return nullptr;
	}
};