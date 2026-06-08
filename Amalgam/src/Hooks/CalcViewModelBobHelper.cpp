#include "../SDK/SDK.h"

MAKE_SIGNATURE(CalcViewModelBobHelper, "client.dll", "40 53 48 81 EC ? ? ? ? 0F 29 B4 24", 0x0);

MAKE_HOOK(CalcViewModelBobHelper, S::CalcViewModelBobHelper(), float,
	void* rcx, void* pBobState)
{
	DEBUG_RETURN(CalcViewModelBobHelper, rcx, pBobState);

	if (Vars::Visuals::Removals::ViewmodelBob.Value)
		return 0.f;

	return CALL_ORIGINAL(rcx, pBobState);
}