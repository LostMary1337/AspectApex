class vec3_t
{
public:
	float x, y, z;

	inline vec3_t() {
		x = y = z = 0.0f;
	}

	inline vec3_t(float X, float Y, float Z) {
		x = X; y = Y; z = Z;
	}

	inline float operator[](int i) const {
		return ((float*)this)[i];
	}

	inline vec3_t& operator+=(float v) {
		x += v; y += v; z += v; return *this;
	}

	inline vec3_t& operator-=(float v) {
		x -= v; y -= v; z -= v; return *this;
	}

	inline vec3_t& operator-=(const vec3_t& v) {
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline vec3_t operator*(float v) const {
		return vec3_t(x * v, y * v, z * v);
	}

	inline vec3_t operator/(float v) const {
		return vec3_t(x / v, y / v, z / v);
	}

	inline vec3_t& operator+=(const vec3_t& v) {
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline vec3_t operator-(const vec3_t& v) const {
		return vec3_t(x - v.x, y - v.y, z - v.z);
	}

	inline vec3_t operator+(const vec3_t& v) const {
		return vec3_t(x + v.x, y + v.y, z + v.z);
	}

	inline vec3_t& operator/=(float v) {
		x /= v; y /= v; z /= v; return *this;
	}

	inline bool zero() const {
		return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f && z > -0.1f && z < 0.1f);
	}
};

class vec2_t {
public:
	float x, y;

	inline vec2_t() {
		x = y = 0.0f;
	}

	inline vec2_t(float X, float Y) {
		x = X; y = Y;
	}

	inline vec2_t operator+(float v) const {
		return vec2_t(x + v, y + v);
	}

	inline vec2_t operator-(float v) const {
		return vec2_t(x - v, y - v);
	}

	inline vec2_t& operator+=(float v) {
		x += v; y += v; return *this;
	}

	inline vec2_t& operator*=(float v) {
		x *= v; y *= v; return *this;
	}

	inline vec2_t& operator/=(float v) {
		x /= v; y /= v; return *this;
	}

	inline vec2_t operator-(const vec2_t& v) const {
		return vec2_t(x - v.x, y - v.y);
	}

	inline vec2_t operator+(const vec2_t& v) const {
		return vec2_t(x + v.x, y + v.y);
	}

	inline vec2_t& operator+=(const vec2_t& v) {
		x += v.x; y += v.y; return *this;
	}

	inline vec2_t& operator-=(const vec2_t& v) {
		x -= v.x; y -= v.y; return *this;
	}

	inline vec2_t operator/(float v) const {
		return vec2_t(x / v, y / v);
	}

	inline vec2_t operator*(float v) const {
		return vec2_t(x * v, y * v);
	}

	inline vec2_t operator/(const vec2_t& v) const {
		return vec2_t(x / v.x, y / v.y);
	}

	inline bool zero() const {
		return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f);
	}
};

class matrix3x4_t
{
public:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
		};

		float m[3][4];
		float mm[12];
	};
};

class matrix4x4_t
{
public:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};

		float m[4][4];
		float mm[16];
	};
};

namespace math
{
	#define M_PI 3.14159265358979323846f
	#define M_RADPI	57.295779513082f
	#define M_PI_F ((float)(M_PI))
	#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
	#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

	#define sinf(a) ((float)FC(ntdll, sin, (double)(a)))
	#define cosf(a) ((float)FC(ntdll, cos, (double)(a)))
	#define atanf(a) ((float)FC(ntdll, atan, (double)(a)))
	#define atan2f(a, b) ((float)FC(ntdll, atan2, (double)(a), (double)(b)))

	#define powf(a, b) ((a) * (a))
	//#define F(A) (A)

	float acosf(float x) {
		float negate = float(x < 0);
		x = abs(x);
		float ret = -0.0187293;
		ret = ret * x;
		ret = ret + 0.0742610;
		ret = ret * x;
		ret = ret - 0.2121144;
		ret = ret * x;
		ret = ret + 1.5707288;
		ret = ret * sqrt(1.0 - x);
		ret = ret - 2 * negate * ret;
		return negate * 3.14159265358979 + ret;
	}

	__forceinline float FastSQRT(float x) {
		union { int i; float x; } u;
		u.x = x; u.i = (u.i >> 1) + 0x1FC00000;
		u.x = u.x + x / u.x;
		return .25f * u.x + x / u.x;
	}

	__forceinline float FastDist2D(const vec2_t& Src, const vec2_t& Dst) {
		return FastSQRT(powf(Src.x - Dst.x, 2.f) + powf(Src.y - Dst.y, 2.f));
	}

	__forceinline float FastDist3D(const vec3_t& Src, const vec3_t& Dst) {
		return FastSQRT(powf(Src.x - Dst.x, 2.f) + powf(Src.y - Dst.y, 2.f) + powf(Src.z - Dst.z, 2.f));
	}

	__forceinline float GameDist(const vec3_t& Src, const vec3_t& Dst) {
		return FastDist3D(Src, Dst) * 0.01905f;
	}

	__forceinline float Dist3D(const vec3_t& Src, const vec3_t& Dst) {
		return sqrtf(powf(Src.x - Dst.x, 2.f) + powf(Src.y - Dst.y, 2.f) + powf(Src.z - Dst.z, 2.f));
	}

	__forceinline float Vec3Length(const vec3_t& Src) {
		return sqrtf(powf(Src.x, 2.f) + powf(Src.y, 2.f) + powf(Src.z, 2.f));
	}

	__forceinline void clamp_angle(vec3_t& Ang) {
		if (Ang.x < -89.f) Ang.x = -89.f;
		if (Ang.x > 89.f) Ang.x = 89.f;
		while (Ang.y < -180.f) Ang.y += 360.f;
		while (Ang.y > 180.f) Ang.y -= 360.f;
		Ang.z = 0.f;
	}

	__forceinline vec3_t CalcAngle(const vec3_t& Src, const vec3_t& Dst) {
		vec3_t Delta = Src - Dst, AimAngles;
		float Hyp = sqrtf(powf(Delta.x, 2.f) + powf(Delta.y, 2.f));
		AimAngles.y = atanf(Delta.y / Delta.x) * M_RADPI;
		AimAngles.x = atanf(Delta.z / Hyp) * M_RADPI;
		if (Delta.x >= 0.f) AimAngles.y += 180.f;
		AimAngles.z = 0.f; return AimAngles;
	}
	
	__forceinline void VectorNormalize(vec3_t& Src)
	{
		float l = Vec3Length(Src);
		if (l != 0.0f)
		{
			Src /= l;
		}
		else
		{
			Src.x = Src.y = 0.0f; Src.z = 1.0f;
		}
	}

	void MakeVector(vec3_t angle, vec3_t& vector) {
		float pitch = float(angle[0] * M_PI / 180);
		float yaw = float(angle[1] * M_PI / 180);
		float tmp = float(cosf(pitch));
		vector.x = float(-tmp * -cosf(yaw));
		vector.y = float(sinf(yaw) * tmp);
		vector.z = float(-sinf(pitch));
	}

	__forceinline float Dot(const vec3_t& v1, const vec3_t& v2) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	float GetFov(const vec3_t& viewAngle, const vec3_t& aimAngle) {
		vec3_t ang, aim; MakeVector(viewAngle, aim); MakeVector(aimAngle, ang);
		float mag = (float)FastSQRT(powf(aim.x, 2) + powf(aim.y, 2) + powf(aim.z, 2));
		float u_dot_v = Dot(aim, ang);
		auto fov = RAD2DEG(acosf(u_dot_v / (pow(mag, 2))));
		return ((fov < 0) ? 0.f : fov); //fix pls
	}
}