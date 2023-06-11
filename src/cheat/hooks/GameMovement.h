//store original functions & old camera angles
PVOID SilentStartOrg = nullptr;
PVOID SilentEndOrg = nullptr;
vec3_t CameraAngBackup;

//Start Move Hook (indx 4)
void SilentStart(PVOID a1, PVOID a2) {
	//call org func & save angles
	SpoofCall(SilentStartOrg, a1, a2);
	CameraAngBackup = LP->StaticAngles();
}

//End Move Hook (indx 5)
void SilentEnd(PVOID a1, PVOID a2)
{
	if (a2 == LP) { //update camera LP pos
		Global::LP_HeadPos = LP->CameraPos();
	}

	//call FinishTrackPredictionErrors
	SpoofCall(SilentEndOrg, a1, a2);

	//restore angles
	if (AimBot::Active && AimBot::Silent)
		LP->StaticAngles(CameraAngBackup);
}