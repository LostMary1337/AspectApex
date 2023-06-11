class UserCmd
{
public:
	int command_number;
	int tick_count;
	float command_time;
	vec3_t viewangles;
	BYTE pad1[0xC];
	float forwardmove;
	float sidemove;
	float upmove;
	int buttons;
	BYTE pad2[0x24C];
};

struct kbutton_t {
	int down[2];
	int state;
};

class Ray
{
public:
	BYTE RayRaw[0x68];
	__forceinline Ray(const vec3_t& Start, const vec3_t& End) {
		SpoofCall(Fn::RayInit, RayRaw, &Start, &End);
	}
};

struct surface_t
{
	const char*		name;				//0x0000
	int				surfaceProp;		//0x0008
	unsigned short	flags;				//0x000C
};

struct plane_t
{
	vec3_t	surfaceNormal;	//0x0000
	float	dist;			//0x000C
};

struct TraceData
{
	vec3_t startpos;
	int unk0;
	vec3_t endpos;
	int unk01;
	plane_t plane;
	float fraction;
	int contents;
	unsigned short dispFlags;
	bool allsolid;
	bool startsolid;
	int unk02;
	float fractionLeftSolid;
	int unk03;
	surface_t surface;
	int hitgroup;
	short physicsBone;
	unsigned short m_nWorldSurfaceIndex;
	void* m_pEnt;
	int hitbox;
	char pad[0x114];
};

bool World2Screen(const vec3_t& Position, vec2_t& ScreenPosition)
{
	DWORD64 ViewRender = *(DWORD64*)Off::ViewRender; 
	if (!ViewRender) return false;

	matrix4x4_t* m_Matrix = *(matrix4x4_t**)(ViewRender + Off::ViewMatrix);
	if (!m_Matrix) return false;

	float w = 0.f;
	ScreenPosition.x = (m_Matrix->m[0][0] * Position.x) + (m_Matrix->m[0][1] * Position.y) + (m_Matrix->m[0][2] * Position.z + m_Matrix->m[0][3]);
	ScreenPosition.y = (m_Matrix->m[1][0] * Position.x) + (m_Matrix->m[1][1] * Position.y) + (m_Matrix->m[1][2] * Position.z + m_Matrix->m[1][3]);
	w = (m_Matrix->m[3][0] * Position.x) + (m_Matrix->m[3][1] * Position.y) + (m_Matrix->m[3][2] * Position.z + m_Matrix->m[3][3]);

	if (w < 0.01f)
		return false;

	float invw = 1.f / w;
	ScreenPosition.x *= invw;
	ScreenPosition.y *= invw;

	float x = Global::WndSize.x / 2.f;
	float y = Global::WndSize.y / 2.f;
	x += .5f * ScreenPosition.x * Global::WndSize.x + .5f;
	y -= .5f * ScreenPosition.y * Global::WndSize.y + .5f;
	ScreenPosition.x = x; ScreenPosition.y = y; return true;
}