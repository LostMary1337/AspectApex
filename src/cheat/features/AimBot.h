//thx ApexBot for good prediction
#include "BulletPrediction.h"

inline float distance_cursor(vec2_t vec)
{
	POINT p;
	if (GetCursorPos(&p))
	{
		float ydist = (vec.y - p.y);
		float xdist = (vec.x - p.x);
		float ret = sqrt(pow(ydist, 2) + pow(xdist, 2));
		return ret;
	}
}

vec2_t smooth(vec2_t pos)
{
	vec2_t center{ (float)(Global::WndSize.x / 2), (float)(Global::WndSize.y / 2) };
	vec2_t target{ 0, 0 };
	if (pos.x != 0) {
		if (pos.x > center.x) {
			target.x = -(center.x - pos.x);
			target.x /= 1;
			if (target.x + center.x > center.x * 2)
				target.x = 0;
		}

		if (pos.x < center.x) {
			target.x = pos.x - center.x;
			target.x /= 1;
			if (target.x + center.x < 0)
				target.x = 0;
		}
	}

	if (pos.y != 0) {
		if (pos.y > center.y) {
			target.y = -(center.y - pos.y);
			target.y /= 1;
			if (target.y + center.y > center.y * 2)
				target.y = 0;
		}

		if (pos.y < center.y) {
			target.y = pos.y - center.y;
			target.y /= 1;
			if (target.y + center.y < 0)
				target.y = 0;
		}
	}

	target.x /= AimBot::Smooth;
	target.y /= AimBot::Smooth;

	if (abs(target.x) < 1) {
		if (target.x > 0) {
			target.x = 1;
		}
		if (target.x < 0) {
			target.x = -1;
		}
	}
	if (abs(target.y) < 1) {
		if (target.y > 0) {
			target.y = 1;
		}
		if (target.y < 0) {
			target.y = -1;
		}
	}

	return target;
}

//Aim Code
void SmoothAngle(vec3_t src, vec3_t& dst, float factor) {
	vec3_t delta = dst - src;
	math::clamp_angle(delta);
	dst = src + delta / factor;
}

CBasePlayer* FindTarget(int& AimEntIndex, int& BestHitBox)
{
	//set global data
	LPlayer* LP_Ent = LP;
	CBasePlayer* FOV_Target = nullptr;
	float f_Best = AimBot::FOV, d_Best = AimBot::Distance;
	if (!LP_Ent->Alive())
	{
		AimEntIndex = -1;
		return nullptr;
	}

	//aim cache
	if (AimEntIndex != -1)
	{
		//check player
		CBasePlayer* CurEnt = Ent(AimEntIndex);
		if (CurEnt->Type(Player))
		{
			//check no knocked & on screen & visible
			auto HBoxPos = CurEnt->HitBoxPos(BestHitBox, false); vec2_t ScreenPos;
			if (!CurEnt->Knocked() && World2Screen(HBoxPos, ScreenPos) && CurEnt->VisiblePos(LP_Ent, HBoxPos))
			{
				auto currentfov = math::GetFov(LP->DynamicAngles(), math::CalcAngle(LP->CameraPos(), HBoxPos));
				if (currentfov < f_Best && currentfov < AimBot::FOV)
					return CurEnt;
			}
		}
	}

	//set invalid cache
	AimEntIndex = -1;

	//new find target
	for (int i = 0; i < 80; i++)
	{
		//get entity & check is player
		CBasePlayer* CurEnt = Ent(i);
		if (CurEnt->Type(Player))
		{
			//skip knocked
			if (AimBot::NoKnocked && CurEnt->Knocked())
				continue;

			//cut distance
			vec3_t BodyBonePos = CurEnt->HitBoxPos(3, false);
			float CurDist = math::GameDist(BodyBonePos, LP->CameraPos());
			if (CurDist > AimBot::Distance) continue;

			//check head
			vec3_t HeadBonePos = CurEnt->HitBoxPos(0, false); vec2_t ScreenPos;
			if (World2Screen(HeadBonePos, ScreenPos) && CurEnt->VisiblePos(LP_Ent, HeadBonePos))
			{
				//calc fov
				float CurFOV = distance_cursor(ScreenPos);

				//set fov target
				if (CurFOV < f_Best && CurFOV < AimBot::FOV) {
					FOV_Target = CurEnt;
					f_Best = CurFOV;
					AimEntIndex = i;
					BestHitBox = 0;
				}
			}

			//check body
			if (AimBot::Nearest && World2Screen(BodyBonePos, ScreenPos) && CurEnt->VisiblePos(LP_Ent, BodyBonePos))
			{
				//calc fov
				float CurFOV = distance_cursor(ScreenPos);

				//set fov target
				if (CurFOV < f_Best && CurFOV < AimBot::FOV)
				{
					FOV_Target = CurEnt;
					f_Best = CurFOV;
					AimEntIndex = i;
					BestHitBox = 3;
				}
			}
		}
	}

	//set best target
	return FOV_Target;
}

__forceinline bool Aim(UserCmd* Cmd)
{
	static int AimHitBox, AimID = -1;
	if (AimBot::Active && (GetKeyState(AimBot::AimKey) & 0x8000))//if (AimBot::Active && ((Cmd->buttons & 1) || (Cmd->buttons & 0x20000)))
	{
		//get weapon speed & gravity
		float BulletSpeed = LP->ActiveWeapon()->BulletSpeed(); //bullet speed (units)
		float BulletGrav = LP->ActiveWeapon()->BulletGravity(); //(gravity scale * 750.f)

		//is weapon check
		if ((BulletSpeed > 1.f) || (BulletGrav > 1.f))
		{
			//find target
			vec3_t AimAngles;
			CBasePlayer* AimTarget;
			if ((AimTarget = FindTarget(AimID, AimHitBox)))
			{
				//get target & muzzle positions
				vec3_t MuzzlePos = LP->CameraPos();//->ActiveWeapon()->MuzzlePos();
				vec3_t TargetPos = AimTarget->HitBoxPos(AimHitBox, true);

				//check need prediction
				if (BulletSpeed > 1.f)
				{
					//simple prediction
					if (AimBot::PredictType)
					{
						//compensate bullet drop 
						float VerticalTime = math::Dist3D(TargetPos, MuzzlePos) / BulletSpeed;
						TargetPos.z += (750.f * LP->ActiveWeapon()->BulletGravity() * 0.5f) * (VerticalTime * VerticalTime);

						//compensate velocity
						float HorizontalTime = math::Dist3D(TargetPos, MuzzlePos) / BulletSpeed;
						TargetPos += (AimTarget->AbsVelocity(true) * HorizontalTime);

						//calculate angle
						AimAngles = math::CalcAngle(MuzzlePos, TargetPos);
					}

					else {
						//set pred ctx
						PredictCtx Ctx;
						Ctx.StartPos = MuzzlePos;
						Ctx.TargetPos = TargetPos;
						Ctx.BulletSpeed = BulletSpeed;
						Ctx.BulletGravity = (BulletGrav * 750.f);
						Ctx.TargetVel = AimTarget->AbsVelocity();

						//target not hittable?!
						if (!BulletPredict(Ctx)) { AimID = -1; return false; }
						AimAngles = vec3_t{ Ctx.AimAngles.x, Ctx.AimAngles.y, 0.f };
					}
				}

				else {
					//not needed bullet prediction
					AimAngles = math::CalcAngle(MuzzlePos, TargetPos);
				}

				//remove sway & recoil
				if (AimBot::WeaponFix) {
					AimAngles -= LP->PunchAngle();
					AimAngles -= ((LP->DynamicAngles() - LP->PunchAngle()) - Cmd->viewangles);
				}

				//apply angle & set aim type
				if (AimBot::UseCamera)
				{
					math::clamp_angle(AimAngles);
					SmoothAngle(Cmd->viewangles, AimAngles, AimBot::Smooth);
					Cmd->viewangles = AimAngles; return true;
				}
				else
				{
					vec2_t ScreenPos;
					if (World2Screen(AimAngles, ScreenPos))
					{
						vec2_t target = smooth(ScreenPos);
						INPUT input;
						input.type = INPUT_MOUSE;
						input.mi.mouseData = 0;
						input.mi.time = 0;
						input.mi.dx = target.x;
						input.mi.dy = target.y;
						input.mi.dwFlags = MOUSEEVENTF_MOVE;
						SendInput(1, &input, sizeof(input));
					}
				}
			}
		}
	}

	//invalid
	AimID = -1;
	return false;
}