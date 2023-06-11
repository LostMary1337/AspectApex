struct PredictCtx
{
	//input data
	vec3_t StartPos;
	vec3_t TargetPos;
	vec3_t TargetVel;
	float BulletSpeed;
	float BulletGravity;

	//out data
	vec2_t AimAngles;
};

__forceinline vec3_t ExtrapolatePos(const PredictCtx& Ctx, float Time) {
	return Ctx.TargetPos + (Ctx.TargetVel * Time);
}

__forceinline bool OptimalPitch(const PredictCtx& Ctx, const vec2_t& Dir2D, float* OutPitch) {
	float Vel = Ctx.BulletSpeed, Grav = Ctx.BulletGravity, DirX = Dir2D.x, DirY = Dir2D.y;
	float Root = Vel * Vel * Vel * Vel - Grav * (Grav * DirX * DirX + 2.f * DirY * Vel * Vel);
	if (Root >= 0.f) { *OutPitch = atanf((Vel * Vel - sqrt(Root)) / (Grav * DirX)); return true; }
	return false;
}

__forceinline bool SolveTrajectory(PredictCtx& Ctx, const vec3_t& ExtrPos, float* TravelTime)
{
	vec3_t Dir = ExtrPos - Ctx.StartPos;
	vec2_t Dir2D = { sqrtf(Dir.x * Dir.x + Dir.y * Dir.y), Dir.z };

	float CurPitch;
	if (!OptimalPitch(Ctx, Dir2D, &CurPitch)) {
		return false;
	}

	*TravelTime = Dir2D.x / (cosf(CurPitch) * Ctx.BulletSpeed);
	Ctx.AimAngles.y = atan2f(Dir.y, Dir.x);
	Ctx.AimAngles.x = CurPitch;
	return true;
}

__forceinline bool BulletPredict(PredictCtx& Ctx)
{
	//VIRTUALIZER_MUTATE_ONLY_START

	float MAX_TIME = 1.f, TIME_STEP = (1.f / 256.f);
	for (float CurrentTime = 0.f; CurrentTime <= MAX_TIME; CurrentTime += TIME_STEP)
	{
		float TravelTime;
		vec3_t ExtrPos = ExtrapolatePos(Ctx, CurrentTime);
		if (!SolveTrajectory(Ctx, ExtrPos, &TravelTime)) {
			return false;
		}

		if (TravelTime < CurrentTime) {
			Ctx.AimAngles = { -RAD2DEG(Ctx.AimAngles.x), RAD2DEG(Ctx.AimAngles.y) };
			return true;
		}
	}

	//VIRTUALIZER_MUTATE_ONLY_END
}