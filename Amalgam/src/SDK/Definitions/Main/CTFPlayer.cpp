#include "CTFPlayer.h"

#include "../../SDK.h"

Vec3 CTFPlayer::GetEyeAngles()
{
	return { m_angEyeAnglesX(), m_angEyeAnglesY(), 0.f };
}

Vec3 CTFPlayer::GetViewOffset(bool bScale)
{
	Vec3 vOffset = GetOffset() / 2;

	if (!IsPlayer())
		return vOffset;

	if (IsDucking())
		vOffset = { 0.f, 0.f, 45.f };
	else switch (m_iClass())
		{
		case TF_CLASS_SCOUT: vOffset = { 0.f, 0.f, 65.f }; break;
		case TF_CLASS_SOLDIER: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_PYRO: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_DEMOMAN: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_HEAVY: vOffset = { 0.f, 0.f, 75.f }; break;
		case TF_CLASS_ENGINEER: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_MEDIC: vOffset = { 0.f, 0.f, 75.f }; break;
		case TF_CLASS_SNIPER: vOffset = { 0.f, 0.f, 75.f }; break;
		case TF_CLASS_SPY: vOffset = { 0.f, 0.f, 75.f }; break;
		default: vOffset = m_vecViewOffset().z ? m_vecViewOffset() : Vec3(0.f, 0.f, 72.f);
		}

	return bScale ? vOffset * m_flModelScale() : vOffset;
}

bool CTFPlayer::InCond(ETFCond eCond)
{
	switch (eCond / 32)
	{
	case 0: return m_nPlayerCond() & (1 << eCond) || _condition_bits() & (1 << eCond);
	case 1: return m_nPlayerCondEx() & (1 << (eCond - 32));
	case 2: return m_nPlayerCondEx2() & (1 << (eCond - 64));
	case 3: return m_nPlayerCondEx3() & (1 << (eCond - 96));
	case 4: return m_nPlayerCondEx4() & (1 << (eCond - 128));
	}
	return false;
}

void CTFPlayer::AddCond(ETFCond eCond)
{
	switch (eCond / 32)
	{
	case 0: m_nPlayerCond() |= (1 << eCond), _condition_bits() |= (1 << eCond); break;
	case 1: m_nPlayerCondEx() |= (1 << (eCond - 32)); break;
	case 2: m_nPlayerCondEx2() |= (1 << (eCond - 64)); break;
	case 3: m_nPlayerCondEx3() |= (1 << (eCond - 96)); break;
	case 4: m_nPlayerCondEx4() |= (1 << (eCond - 128)); break;
	}
}

void CTFPlayer::RemoveCond(ETFCond eCond)
{
	switch (eCond / 32)
	{
	case 0: m_nPlayerCond() &= ~(1 << eCond), _condition_bits() &= ~(1 << eCond); break;
	case 1: m_nPlayerCondEx() &= ~(1 << (eCond - 32)); break;
	case 2: m_nPlayerCondEx2() &= ~(1 << (eCond - 64)); break;
	case 3: m_nPlayerCondEx3() &= ~(1 << (eCond - 96)); break;
	case 4: m_nPlayerCondEx4() &= ~(1 << (eCond - 128)); break;
	}
}

bool CTFPlayer::IsAGhost()
{
	return InCond(TF_COND_HALLOWEEN_GHOST_MODE);
};
bool CTFPlayer::IsTaunting()
{
	return InCond(TF_COND_TAUNTING);
};

bool CTFPlayer::IsInvisible(float flValue)
{
	if (InCond(TF_COND_BURNING)
		|| InCond(TF_COND_BURNING_PYRO)
		|| InCond(TF_COND_MAD_MILK)
		|| InCond(TF_COND_URINE))
		return false;

	float flInvis = GetEffectiveInvisibilityLevel();
	return flInvis && flInvis >= 1.f;
}

bool CTFPlayer::IsInvulnerable()
{
	return InCond(TF_COND_INVULNERABLE)
		|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
		|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED)
		|| InCond(TF_COND_INVULNERABLE_USER_BUFF)
		|| InCond(TF_COND_PHASE);
}

bool CTFPlayer::IsUbered()
{
	return InCond(TF_COND_INVULNERABLE)
		|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
		|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED)
		|| InCond(TF_COND_INVULNERABLE_USER_BUFF);
}

bool CTFPlayer::IsCritBoosted()
{
	if (InCond(TF_COND_CRITBOOSTED)
		|| InCond(TF_COND_CRITBOOSTED_PUMPKIN)
		|| InCond(TF_COND_CRITBOOSTED_USER_BUFF)
		|| InCond(TF_COND_CRITBOOSTED_FIRST_BLOOD)
		|| InCond(TF_COND_CRITBOOSTED_BONUS_TIME)
		|| InCond(TF_COND_CRITBOOSTED_CTF_CAPTURE)
		|| InCond(TF_COND_CRITBOOSTED_ON_KILL)
		|| InCond(TF_COND_CRITBOOSTED_CARD_EFFECT)
		|| InCond(TF_COND_CRITBOOSTED_RUNE_TEMP))
		return true;

	if (auto pWeapon = m_hActiveWeapon()->As<CTFWeaponBase>())
	{
		if (auto pWeaponInfo = pWeapon->m_pWeaponInfo())
		{
			if (InCond(TF_COND_CRITBOOSTED_RAGE_BUFF) && pWeaponInfo->m_iWeaponType == TF_WPN_TYPE_PRIMARY)
				return true;
		}
	}

	return false;
}

bool CTFPlayer::IsMiniCritBoosted()
{
	return InCond(TF_COND_OFFENSEBUFF)
		|| InCond(TF_COND_ENERGY_BUFF)
		|| InCond(TF_COND_NOHEALINGDAMAGEBUFF)
		/*|| InCond(TF_COND_CRITBOOSTED_DEMO_CHARGE)*/;
}

bool CTFPlayer::IsMarked()
{
	return InCond(TF_COND_URINE)
		|| InCond(TF_COND_MARKEDFORDEATH)
		|| InCond(TF_COND_MARKEDFORDEATH_SILENT)
		|| InCond(TF_COND_PASSTIME_PENALTY_DEBUFF);
}

bool CTFPlayer::CanAttack(bool bCloak, bool bLocal)
{
	if (!IsAlive() || IsAGhost() || IsTaunting() || m_bViewingCYOAPDA()
		|| InCond(TF_COND_PHASE) || InCond(TF_COND_HALLOWEEN_KART) || InCond(TF_COND_STUNNED) && m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE))
		return false;

	if (bCloak)
	{
		if (bLocal
			? (m_flStealthNoAttackExpire() > TICKS_TO_TIME(m_nTickBase()) && !InCond(TF_COND_STEALTHED_USER_BUFF)) || InCond(TF_COND_STEALTHED)
			: m_flInvisibility() && (InCond(TF_COND_STEALTHED) || !InCond(TF_COND_STEALTHED_USER_BUFF) && !InCond(TF_COND_STEALTHED_USER_BUFF_FADING)))
		{
			auto pWeapon = m_hActiveWeapon()->As<CTFWeaponBase>();
			if (!pWeapon || pWeapon->GetWeaponID() != TF_WEAPON_GRAPPLINGHOOK)
				return false;
		}

		if (m_bFeignDeathReady())
			return false;
	}

	auto pGameRules = I::TFGameRules();
	if (pGameRules)
	{
		switch (pGameRules->m_iRoundState())
		{
		case GR_STATE_TEAM_WIN:
			if (m_iTeamNum() != pGameRules->m_iWinningTeam())
				return false;
			break;
		case GR_STATE_BETWEEN_RNDS:
			if (m_fFlags() & FL_FROZEN)
				return false;
			break;
		case GR_STATE_GAME_OVER:
			if (m_fFlags() & FL_FROZEN || m_iTeamNum() != pGameRules->m_iWinningTeam())
				return false;
			break;
		}
	}

	if (SDK::AttribHookValue(0, "no_attack", this))
		return false;

	return true;
}

float CTFPlayer::GetCritMult()
{
	return Math::RemapVal(m_iCritMult(), 0.f, 255.f, 1.f, 4.f);
}

const char* CTFPlayer::GetWeaponName()
{
	if (!this->m_hActiveWeapon().Get()) return "";

	switch (this->m_hActiveWeapon().Get()->As<CTFWeaponBase>()->m_iItemDefinitionIndex())
	{
	case Scout_m_ForceANature:
	case Scout_m_FestiveForceANature: { return "FORCE-A-NATURE"; }
	case Scout_m_FestiveScattergun: { return "SCATTERGUN"; }
	case Scout_m_BackcountryBlaster: { return "BACKSCATTER"; }
	case Scout_s_MutatedMilk: { return "MILK"; }
	case Scout_s_TheWinger: { return "WINGER"; }
	case Scout_s_FestiveBonk:
	case Scout_s_BonkAtomicPunch: { return "BONK"; }
	case Scout_s_PrettyBoysPocketPistol: { return "POCKET PISTOL"; }
	case Scout_s_CritaCola: { return "CRIT A COLA"; }
	case Scout_t_FestiveBat: { return "BAT"; }
	case Scout_t_FestiveHolyMackerel: { return "BAT"; }
	case Scout_t_TheAtomizer: { return "ATOMIZER"; }
	case Scout_t_TheCandyCane: { return "CANDY CANE"; }
	case Scout_t_TheFanOWar: { return "FAN O WAR"; }
	case Scout_t_SunonaStick: { return "SUN ON A STICK"; }
	case Scout_t_TheBostonBasher: { return "BOSTON BASHER"; }
	case Scout_t_ThreeRuneBlade: { return "BOSTON BASHER"; }
	case Scout_t_TheFreedomStaff: { return "FREEDOM STAFF"; }
	case Scout_t_TheBatOuttaHell: { return "BAT OUTTA HELL"; }
	case Scout_s_Lugermorph:
	case Scout_s_VintageLugermorph: { return "PISTOL"; }
	case Scout_s_TheCAPPER: { return "PISTOL"; }
	case Scout_t_UnarmedCombat: { return "BAT"; }
	case Scout_t_Batsaber: { return "BAT"; }
	case Scout_t_TheHamShank: { return "HAM"; }
	case Scout_t_TheNecroSmasher: { return "NECRO SMASHER"; }
	case Scout_t_TheConscientiousObjector: { return "OBJECTOR"; }
	case Scout_t_TheCrossingGuard: { return "CROSSING GUARD"; }
	case Scout_t_TheMemoryMaker: { return "MEMORY MAKER"; }

	case Soldier_m_FestiveRocketLauncher: { return "ROCKET LAUNCHER"; }
	case Soldier_m_RocketJumper: { return "ROCKET JUMPER"; }
	case Soldier_m_TheAirStrike: { return "AIR STRIKE"; }
	case Soldier_m_TheLibertyLauncher: { return "LIBERTY LAUNCHER"; }
	case Soldier_m_TheOriginal: { return "ORIGINAL"; }
	case Soldier_m_FestiveBlackBox:
	case Soldier_m_TheBlackBox: { return "BLACK BOX"; }
	case Soldier_m_TheBeggarsBazooka: { return "BEGGARS BAZOOKA"; }
	case Soldier_s_FestiveShotgun: { return "SHOTGUN"; }
	case Soldier_s_FestiveBuffBanner: { return "BUFF BANNER"; }
	case Soldier_s_TheConcheror: { return "CONCH"; }
	case Soldier_s_TheBattalionsBackup: { return "BATTALIONS"; }
	case Soldier_s_PanicAttack: { return "{PANIC ATTACK"; }
	case Soldier_t_TheMarketGardener: { return "MARKET GARDENER"; }
	case Soldier_t_TheDisciplinaryAction: { return "WHIP"; }
	case Soldier_t_TheEqualizer: { return "EQUALIZER"; }
	case Soldier_t_ThePainTrain: { return "PAIN TRAIN"; }
	case Soldier_t_TheHalfZatoichi: { return "HALF ZATOICHI"; }

	case Pyro_m_FestiveFlameThrower: { return "FLAME THROWER"; }
	case Pyro_m_ThePhlogistinator: { return "PHLOG"; }
	case Pyro_m_FestiveBackburner:
	case Pyro_m_TheBackburner: { return "BACKBURNER"; }
	case Pyro_m_TheRainblower: { return "FLAME THROWER"; }
	case Pyro_m_TheDegreaser: { return "DEGREASER"; }
	case Pyro_m_NostromoNapalmer: { return "FLAME THROWER"; }
	case Pyro_s_FestiveFlareGun: { return "FLARE GUN"; }
	case Pyro_s_TheScorchShot: { return "SCORCH SHOT"; }
	case Pyro_s_TheDetonator: { return "DETONATOR"; }
	case Pyro_s_TheReserveShooter: { return "RESERVE SHOOTER"; }
	case Pyro_t_TheFestiveAxtinguisher:
	case Pyro_t_TheAxtinguisher: { return "AXTINGUISHER"; }
	case Pyro_t_Homewrecker: { return "HOMEWRECKER"; }
	case Pyro_t_ThePowerjack: { return "PWERJACK"; }
	case Pyro_t_TheBackScratcher: { return "BACK SCRATCHER"; }
	case Pyro_t_TheThirdDegree: { return "THIRD DEGREE"; }
	case Pyro_t_ThePostalPummeler: { return "POSTAL PUMMELER"; }
	case Pyro_t_PrinnyMachete: { return "PRINNY MACHETE"; }
	case Pyro_t_SharpenedVolcanoFragment: { return "VOLCANO FRAGMENT"; }
	case Pyro_t_TheMaul: { return "MAUL"; }
	case Pyro_t_TheLollichop: { return "FIRE AXE"; }

	case Demoman_m_FestiveGrenadeLauncher: { return "GRENADE LAUNCHER"; }
	case Demoman_m_TheIronBomber: { return "IRON BOMBER"; }
	case Demoman_m_TheLochnLoad: { return "LOCH N LOAD"; }
	case Demoman_s_FestiveStickybombLauncher: { return "STICKYBOMB LAUNCHER"; }
	case Demoman_s_StickyJumper: { return "STICKY JUMPER"; }
	case Demoman_s_TheQuickiebombLauncher: { return "QUICKIEBOMB"; }
	case Demoman_s_TheScottishResistance: { return "SCOTTISH"; }
	case Demoman_t_HorselessHeadlessHorsemannsHeadtaker: { return "EYELANDER"; }
	case Demoman_t_TheScottishHandshake: { return "BOTTLE"; }
	case Demoman_t_FestiveEyelander:
	case Demoman_t_TheEyelander: { return "EYELANDER"; }
	case Demoman_t_TheScotsmansSkullcutter: { return "SKULLCUTTER"; }
	case Demoman_t_ThePersianPersuader: { return "PERSIAN PERSUADER"; }
	case Demoman_t_NessiesNineIron: { return "EYELANDER"; }
	case Demoman_t_TheClaidheamhMor: { return "CLAIDHEAMH MOR"; }

	case Heavy_m_IronCurtain: { return "MINIGUN"; }
	case Heavy_m_FestiveMinigun: { return "MINIGUN"; }
	case Heavy_m_Tomislav: { return "TOMISLAV"; }
	case Heavy_m_TheBrassBeast: { return "BRASS BEAST"; }
	case Heavy_m_Natascha: { return "NATASCHA"; }
	case Heavy_m_TheHuoLongHeaterG:
	case Heavy_m_TheHuoLongHeater: { return "HUO-LONG HEATER"; }
	case Heavy_s_TheFamilyBusiness: { return "FAMILY BUSINESS"; }
	case Heavy_s_FestiveSandvich:
	case Heavy_s_RoboSandvich:
	case Heavy_s_Sandvich: { return "SANDVICH"; }
	case Heavy_s_Fishcake: { return "CHOCOLATE"; }
	case Heavy_s_SecondBanana: { return "BANANA"; }
	case Heavy_s_TheDalokohsBar: { return "CHOCOLATE"; }
	case Heavy_s_TheBuffaloSteakSandvich: { return "STEAK"; }
	case Heavy_t_FistsofSteel: { return "FISTS OF STEEL"; }
	case Heavy_t_TheHolidayPunch: { return "HOLIDAY PUNCH"; }
	case Heavy_t_WarriorsSpirit: { return "WARRIORS SPIRIT"; }
	case Heavy_t_TheEvictionNotice: { return "EVICTION NOTICE"; }
	case Heavy_t_TheKillingGlovesofBoxing: { return "KILLING GLOVES OF BOXING"; }
	case Heavy_t_ApocoFists: { return "FISTS"; }
	case Heavy_t_FestiveGlovesofRunningUrgently:
	case Heavy_t_GlovesofRunningUrgently: { return "GLOVES OF RUNNING URGENTLY"; }
	case Heavy_t_TheBreadBite: { return "GLOVES OF RUNNING URGENTLY"; }

	case Engi_m_FestiveFrontierJustice: { return "FRONTIER JUSTICE"; }
	case Engi_m_TheWidowmaker: { return "WIDOWMAKER"; }
	case Engi_s_TheGigarCounter:
	case Engi_s_FestiveWrangler: { return "WRANGLER"; }
	case Engi_s_TheShortCircuit: { return "SHORT CIRCUIT"; }
	case Engi_t_FestiveWrench: { return "WRENCH"; }
	case Engi_t_GoldenWrench: { return "WRENCH"; }
	case Engi_t_TheGunslinger: { return "GUNSLINGER"; }
	case Engi_t_TheJag: { return "JAG"; }
	case Engi_t_TheEurekaEffect: { return "EUREKA EFFECT"; }
	case Engi_t_TheSouthernHospitality: { return "SOUTHERN HOSPITALITY"; }

	case Medic_m_FestiveCrusadersCrossbow: { return "CROSSBOW"; }
	case Medic_m_TheOverdose: { return "OVERDOSE"; }
	case Medic_m_TheBlutsauger: { return "BLUTSAUGER"; }
	case Medic_s_FestiveMediGun: { return "MEDIGUN"; }
	case Medic_s_TheQuickFix: { return "QUICK FIX"; }
	case Medic_s_TheKritzkrieg: { return "KRITZKRIEG"; }
	case Medic_s_TheVaccinator: { return "VACCINATOR"; }
	case Medic_t_FestiveBonesaw: { return "BONESAW"; }
	case Medic_t_FestiveUbersaw:
	case Medic_t_TheUbersaw: { return "UBERSAW"; }
	case Medic_t_TheVitaSaw: { return "VITASAW"; }
	case Medic_t_TheSolemnVow: { return "SOLEMN VOW"; }
	case Medic_t_Amputator: { return "AMPUTATOR"; }

	case Sniper_m_FestiveSniperRifle: { return "SNIPER RIFLE"; }
	case Sniper_m_FestiveHuntsman:
	case Sniper_m_TheHuntsman: { return "HUNTSMAN"; }
	case Sniper_m_TheMachina: { return "MACHINA"; }
	case Sniper_m_TheAWPerHand: { return "SNIPER RIFLE"; }
	case Sniper_m_TheHitmansHeatmaker: { return "HEATMAKER"; }
	case Sniper_m_TheSydneySleeper: { return "SYDNEY SLEEPER"; }
	case Sniper_m_ShootingStar: { return "MACHINA"; }
	case Sniper_s_FestiveJarate: { return "JARATE"; }
	case Sniper_s_TheSelfAwareBeautyMark: { return "JARATE"; }
	case Sniper_s_FestiveSMG: { return "SMG"; }
	case Sniper_t_TheBushwacka: { return "BUSHWACKA"; }
	case Sniper_t_KukriR:
	case Sniper_t_Kukri: { return "KUKRI"; }
	case Sniper_t_TheShahanshah: { return "SHAHANSHAH"; }
	case Sniper_t_TheTribalmansShiv: { return "TRIBALMANS SHIV"; }

	case Spy_m_FestiveRevolver: { return "REVOLVER"; }
	case Spy_m_FestiveAmbassador:
	case Spy_m_TheAmbassador: { return "AMBASSADOR"; }
	case Spy_m_BigKill: { return "REVOLVER"; }
	case Spy_m_TheDiamondback: { return "DIAMONDBACK"; }
	case Spy_m_TheEnforcer: { return "ENFORCER"; }
	case Spy_m_LEtranger: { return "LETRANGER"; }
	case Spy_s_Sapper:
	case Spy_s_SapperR:
	case Spy_s_FestiveSapper: { return "SAPPER"; }
	case Spy_s_TheRedTapeRecorder:
	case Spy_s_TheRedTapeRecorderG: { return "RED TAPE RECORDER"; }
	case Spy_s_TheApSapG: { return "SAPPER"; }
	case Spy_s_TheSnackAttack: { return "SAPPER"; }
	case Spy_t_FestiveKnife: { return "KNIFE"; }
	case Spy_t_ConniversKunai: { return "KUNAI"; }
	case Spy_t_YourEternalReward: { return "ETERNAL REWARD"; }
	case Spy_t_TheBigEarner: { return "BIG EARNER"; }
	case Spy_t_TheSpycicle: { return "SPYCICLE"; }
	case Spy_t_TheSharpDresser: { return "KNIFE"; }
	case Spy_t_TheWangaPrick: { return "ETERNAL REWARD"; }
	case Spy_t_TheBlackRose: { return "KNIFE"; }

	case Heavy_m_Deflector_mvm: { return "DEFLECTOR"; }
	case Misc_t_FryingPan: { return "FRYING PAN"; }
	case Misc_t_GoldFryingPan: { return "FRYING PAN"; }
	case Misc_t_Saxxy: { return "SAXXY"; }

	default:
	{
		switch (this->m_hActiveWeapon().Get()->As<CTFWeaponBase>()->GetWeaponID())
		{
			//scout
		case TF_WEAPON_SCATTERGUN: { return "SCATTERGUN"; }
		case TF_WEAPON_HANDGUN_SCOUT_PRIMARY: { return "SHORTSTOP"; }
		case TF_WEAPON_HANDGUN_SCOUT_SECONDARY: { return "PISTOL"; }
		case TF_WEAPON_SODA_POPPER: { return "SODA POPPER"; }
		case TF_WEAPON_PEP_BRAWLER_BLASTER: { return "BABY FACES BLASTER"; }
		case TF_WEAPON_PISTOL_SCOUT: { return "PISTOL"; }
		case TF_WEAPON_JAR_MILK: { return "MILK"; }
		case TF_WEAPON_CLEAVER: { return "CLEAVER"; }
		case TF_WEAPON_BAT: { return "BAT"; }
		case TF_WEAPON_BAT_WOOD: { return "SANDMAN"; }
		case TF_WEAPON_BAT_FISH: { return "BAT"; }
		case TF_WEAPON_BAT_GIFTWRAP: { return "WRAP ASSASSIN"; }

								   //soldier
		case TF_WEAPON_ROCKETLAUNCHER: { return "ROCKET LAUNCHER"; }
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT: { return "DIRECT HIT"; }
		case TF_WEAPON_PARTICLE_CANNON: { return "COW MANGLER"; }
		case TF_WEAPON_SHOTGUN_SOLDIER: { return "SHOTGUN"; }
		case TF_WEAPON_BUFF_ITEM: { return "BUFF BANNER"; }
		case TF_WEAPON_RAYGUN: { return "BISON"; }
		case TF_WEAPON_SHOVEL: { return "SHOVEL"; }

							 //pyro
		case TF_WEAPON_FLAMETHROWER: { return "FLAME THROWER"; }
		case TF_WEAPON_FLAME_BALL: { return "DRAGONS FURY"; }
		case TF_WEAPON_SHOTGUN_PYRO: { return "SHOTGUN"; }
		case TF_WEAPON_FLAREGUN: { return "FLARE GUN"; }
		case TF_WEAPON_FLAREGUN_REVENGE: { return "MANMELTER"; }
		case TF_WEAPON_JAR_GAS: { return "GAS PASSER"; }
		case TF_WEAPON_ROCKETPACK: { return "THERMAL THRUSTER"; }
		case TF_WEAPON_FIREAXE: { return "FIRE AXE"; }
		case TF_WEAPON_BREAKABLE_SIGN: { return "NEON ANNIHILATOR"; }
		case TF_WEAPON_SLAP: { return "HOT HAND"; }

						   //demoman
		case TF_WEAPON_GRENADELAUNCHER: { return "GRENADE LAUNCHER"; }
		case TF_WEAPON_PIPEBOMBLAUNCHER: { return "STICKYBOMB LAUNCHER"; }
		case TF_WEAPON_CANNON: { return "LOOSE CANNON"; }
		case TF_WEAPON_BOTTLE: { return "BOTTLE"; }
		case TF_WEAPON_SWORD: { return "SWORD"; }
		case TF_WEAPON_STICKBOMB: { return "CABER"; }

								//heavy
		case TF_WEAPON_MINIGUN: { return "MINIGUN"; }
		case TF_WEAPON_SHOTGUN_HWG: { return "SHOTGUN"; }
		case TF_WEAPON_LUNCHBOX: { return "LUNCHBOX"; }
		case TF_WEAPON_FISTS: { return "FISTS"; }

							//engineer
		case TF_WEAPON_SHOTGUN_PRIMARY: { return "SHOTGUN"; }
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE: { return "RESCUE RANGER"; }
		case TF_WEAPON_SENTRY_REVENGE: { return "FRONTIER JUSTICE"; }
		case TF_WEAPON_DRG_POMSON: { return "POMSON"; }
		case TF_WEAPON_PISTOL: { return "PISTOL"; }
		case TF_WEAPON_LASER_POINTER: { return "WRANGLER"; }
		case TF_WEAPON_MECHANICAL_ARM: { return "SHORT CIRCUIT"; }
		case TF_WEAPON_WRENCH: { return "WRENCH"; }
		case TF_WEAPON_PDA_ENGINEER_DESTROY: { return "DESTRUCTION PDA"; }
		case TF_WEAPON_PDA_ENGINEER_BUILD: { return "CONSTRUCTION PDA"; }
		case TF_WEAPON_BUILDER: { return "TOOLBOX"; }

							  //medic
		case TF_WEAPON_SYRINGEGUN_MEDIC: { return "SYRINGE GUN"; }
		case TF_WEAPON_CROSSBOW: { return "CROSSBOW"; }
		case TF_WEAPON_MEDIGUN: { return "MEDIGUN"; }
		case TF_WEAPON_BONESAW: { return "BONESAW"; }

							  //sniper
		case TF_WEAPON_SNIPERRIFLE: { return "SNIPER RIFLE"; }
		case TF_WEAPON_COMPOUND_BOW: { return "HUNTSMAN"; }
		case TF_WEAPON_SNIPERRIFLE_DECAP: { return "BAZAAR BARGAIN"; }
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: { return "CLASSIC"; }
		case TF_WEAPON_SMG: { return "SMG"; }
		case TF_WEAPON_CHARGED_SMG: { return "CLEANERS CARBINE"; }
		case TF_WEAPON_JAR: { return "JARATE"; }
		case TF_WEAPON_CLUB: { return "KUKRI"; }

						   //spy
		case TF_WEAPON_REVOLVER: { return "REVOLVER"; }
		case TF_WEAPON_PDA_SPY_BUILD: { return "SAPPER"; }
		case TF_WEAPON_KNIFE: { return "KNIFE"; }
		case TF_WEAPON_PDA_SPY: { return "DISGUISE KIT"; }
		case TF_WEAPON_INVIS: { return "INVIS WATCH"; }

		case TF_WEAPON_GRAPPLINGHOOK: { return "GRAPPLING HOOK"; }

		default: break;
		}
	}
	}

	return "";
}