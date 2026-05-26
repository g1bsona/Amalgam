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
	case Scout_m_FestiveForceANature: { return "Force-A-Nature"; }
	case Scout_m_FestiveScattergun: { return "Scattergun"; }
	case Scout_m_BackcountryBlaster: { return "Backscatter"; }
	case Scout_s_MutatedMilk: { return "Milk"; }
	case Scout_s_TheWinger: { return "Winger"; }
	case Scout_s_FestiveBonk:
	case Scout_s_BonkAtomicPunch: { return "Bonk"; }
	case Scout_s_PrettyBoysPocketPistol: { return "Pocket Pistol"; }
	case Scout_s_CritaCola: { return "Crit a Cola"; }
	case Scout_t_FestiveBat: { return "Bat"; }
	case Scout_t_FestiveHolyMackerel: { return "Bat"; }
	case Scout_t_TheAtomizer: { return "Atomizer"; }
	case Scout_t_TheCandyCane: { return "Candy Cane"; }
	case Scout_t_TheFanOWar: { return "Fan O War"; }
	case Scout_t_SunonaStick: { return "Sun on a Stick"; }
	case Scout_t_TheBostonBasher: { return "Boston Basher"; }
	case Scout_t_ThreeRuneBlade: { return "Boston Basher"; }
	case Scout_t_TheFreedomStaff: { return "Freedom Staff"; }
	case Scout_t_TheBatOuttaHell: { return "Bat Outta Hell"; }
	case Scout_s_Lugermorph:
	case Scout_s_VintageLugermorph: { return "Pistol"; }
	case Scout_s_TheCAPPER: { return "Pistol"; }
	case Scout_t_UnarmedCombat: { return "Bat"; }
	case Scout_t_Batsaber: { return "Bat"; }
	case Scout_t_TheHamShank: { return "Ham"; }
	case Scout_t_TheNecroSmasher: { return "Hammer"; }
	case Scout_t_TheConscientiousObjector: { return "Objector"; }
	case Scout_t_TheCrossingGuard: { return "Crossing Guard"; }
	case Scout_t_TheMemoryMaker: { return "Memory Maker"; }

	case Soldier_m_FestiveRocketLauncher: { return "Rocket Launcher"; }
	case Soldier_m_RocketJumper: { return "Rocket Jumper"; }
	case Soldier_m_TheAirStrike: { return "Air Strike"; }
	case Soldier_m_TheLibertyLauncher: { return "Liberty Launcher"; }
	case Soldier_m_TheOriginal: { return "Original"; }
	case Soldier_m_FestiveBlackBox:
	case Soldier_m_TheBlackBox: { return "Black Box"; }
	case Soldier_m_TheBeggarsBazooka: { return "Beggars Bazooka"; }
	case Soldier_s_FestiveShotgun: { return "Shotgun"; }
	case Soldier_s_FestiveBuffBanner: { return "Buff Banner"; }
	case Soldier_s_TheConcheror: { return "Conch"; }
	case Soldier_s_TheBattalionsBackup: { return "Battalions"; }
	case Soldier_s_PanicAttack: { return "{Panic Attack"; }
	case Soldier_t_TheMarketGardener: { return "Market Gardener"; }
	case Soldier_t_TheDisciplinaryAction: { return "Whip"; }
	case Soldier_t_TheEqualizer: { return "Equalizer"; }
	case Soldier_t_ThePainTrain: { return "Pain Train"; }
	case Soldier_t_TheHalfZatoichi: { return "Half Zatoichi"; }

	case Pyro_m_FestiveFlameThrower: { return "Flame Thrower"; }
	case Pyro_m_ThePhlogistinator: { return "Phlog"; }
	case Pyro_m_FestiveBackburner:
	case Pyro_m_TheBackburner: { return "Backburner"; }
	case Pyro_m_TheRainblower: { return "Flame Thrower"; }
	case Pyro_m_TheDegreaser: { return "Degreaser"; }
	case Pyro_m_NostromoNapalmer: { return "Flame Thrower"; }
	case Pyro_s_FestiveFlareGun: { return "Flare Gun"; }
	case Pyro_s_TheScorchShot: { return "Scorch Shot"; }
	case Pyro_s_TheDetonator: { return "Detonator"; }
	case Pyro_s_TheReserveShooter: { return "Reserve Shooter"; }
	case Pyro_t_TheFestiveAxtinguisher:
	case Pyro_t_TheAxtinguisher: { return "Axtinguisher"; }
	case Pyro_t_Homewrecker: { return "Homewrecker"; }
	case Pyro_t_ThePowerjack: { return "Powerjack"; }
	case Pyro_t_TheBackScratcher: { return "Back Scratcher"; }
	case Pyro_t_TheThirdDegree: { return "Third Degree"; }
	case Pyro_t_ThePostalPummeler: { return "Postal Pummeler"; }
	case Pyro_t_PrinnyMachete: { return "Prinny Machete"; }
	case Pyro_t_SharpenedVolcanoFragment: { return "Volcano Fragment"; }
	case Pyro_t_TheMaul: { return "Maul"; }
	case Pyro_t_TheLollichop: { return "Fire Axe"; }

	case Demoman_m_FestiveGrenadeLauncher: { return "Grenade Launcher"; }
	case Demoman_m_TheIronBomber: { return "Iron Bomber"; }
	case Demoman_m_TheLochnLoad: { return "Loch N Load"; }
	case Demoman_s_FestiveStickybombLauncher: { return "Stickybomb Launcher"; }
	case Demoman_s_StickyJumper: { return "Sticky Jumper"; }
	case Demoman_s_TheQuickiebombLauncher: { return "Quickiebomb"; }
	case Demoman_s_TheScottishResistance: { return "Scottish Resistance"; }
	case Demoman_t_HorselessHeadlessHorsemannsHeadtaker: { return "Eyelander"; }
	case Demoman_t_TheScottishHandshake: { return "Bottle"; }
	case Demoman_t_FestiveEyelander:
	case Demoman_t_TheEyelander: { return "Eyelander"; }
	case Demoman_t_TheScotsmansSkullcutter: { return "Skullcutter"; }
	case Demoman_t_ThePersianPersuader: { return "Persian Persuader"; }
	case Demoman_t_NessiesNineIron: { return "Nessies Nine Iron"; }
	case Demoman_t_TheClaidheamhMor: { return "Claidheamh Mor"; }

	case Heavy_m_IronCurtain: { return "Minigun"; }
	case Heavy_m_FestiveMinigun: { return "Minigun"; }
	case Heavy_m_Tomislav: { return "Tomislav"; }
	case Heavy_m_TheBrassBeast: { return "Brass Beast"; }
	case Heavy_m_Natascha: { return "Natascha"; }
	case Heavy_m_TheHuoLongHeaterG:
	case Heavy_m_TheHuoLongHeater: { return "Huo-Long Heater"; }
	case Heavy_s_TheFamilyBusiness: { return "Family Business"; }
	case Heavy_s_FestiveSandvich:
	case Heavy_s_RoboSandvich:
	case Heavy_s_Sandvich: { return "Sandvich"; }
	case Heavy_s_Fishcake: { return "Chocolate"; }
	case Heavy_s_SecondBanana: { return "Banana"; }
	case Heavy_s_TheDalokohsBar: { return "Chocolate"; }
	case Heavy_s_TheBuffaloSteakSandvich: { return "Steak"; }
	case Heavy_t_FistsofSteel: { return "Fists of Steel"; }
	case Heavy_t_TheHolidayPunch: { return "Holiday Punch"; }
	case Heavy_t_WarriorsSpirit: { return "Warriors Spirit"; }
	case Heavy_t_TheEvictionNotice: { return "Eviction Notice"; }
	case Heavy_t_TheKillingGlovesofBoxing: { return "Killing Gloves of Boxing"; }
	case Heavy_t_ApocoFists: { return "Fists"; }
	case Heavy_t_FestiveGlovesofRunningUrgently:
	case Heavy_t_GlovesofRunningUrgently: { return "Gloves of Running Urgently"; }
	case Heavy_t_TheBreadBite: { return "Gloves of Running Urgently"; }

	case Engi_m_FestiveFrontierJustice: { return "Frontier Justice"; }
	case Engi_m_TheWidowmaker: { return "Widowmaker"; }
	case Engi_s_TheGigarCounter:
	case Engi_s_FestiveWrangler: { return "Wrangler"; }
	case Engi_s_TheShortCircuit: { return "Short Circuit"; }
	case Engi_t_FestiveWrench: { return "Wrench"; }
	case Engi_t_GoldenWrench: { return "Wrench"; }
	case Engi_t_TheGunslinger: { return "Gunslinger"; }
	case Engi_t_TheJag: { return "Jag"; }
	case Engi_t_TheEurekaEffect: { return "Eureka Effect"; }
	case Engi_t_TheSouthernHospitality: { return "Southern Hospitality"; }

	case Medic_m_FestiveCrusadersCrossbow: { return "Crossbow"; }
	case Medic_m_TheOverdose: { return "Overdose"; }
	case Medic_m_TheBlutsauger: { return "Blutsauger"; }
	case Medic_s_FestiveMediGun: { return "Medigun"; }
	case Medic_s_TheQuickFix: { return "Quick Fix"; }
	case Medic_s_TheKritzkrieg: { return "Kritzkreig"; }
	case Medic_s_TheVaccinator: { return "Vaccinator"; }
	case Medic_t_FestiveBonesaw: { return "Bonesaw"; }
	case Medic_t_FestiveUbersaw:
	case Medic_t_TheUbersaw: { return "Ubersaw"; }
	case Medic_t_TheVitaSaw: { return "Vitasaw"; }
	case Medic_t_TheSolemnVow: { return "Solemn Vow"; }
	case Medic_t_Amputator: { return "Amputator"; }

	case Sniper_m_FestiveSniperRifle: { return "Sniper Rifle"; }
	case Sniper_m_FestiveHuntsman:
	case Sniper_m_TheHuntsman: { return "Huntsman"; }
	case Sniper_m_TheMachina: { return "Machina"; }
	case Sniper_m_TheAWPerHand: { return "Sniper Rifle"; }
	case Sniper_m_TheHitmansHeatmaker: { return "Heatmaker"; }
	case Sniper_m_TheSydneySleeper: { return "Sydney Sleeper"; }
	case Sniper_m_ShootingStar: { return "Machina"; }
	case Sniper_s_FestiveJarate: { return "Jarate"; }
	case Sniper_s_TheSelfAwareBeautyMark: { return "Jarate"; }
	case Sniper_s_FestiveSMG: { return "SMG"; }
	case Sniper_t_TheBushwacka: { return "Bushwacka"; }
	case Sniper_t_KukriR:
	case Sniper_t_Kukri: { return "Kukri"; }
	case Sniper_t_TheShahanshah: { return "Shahanshah"; }
	case Sniper_t_TheTribalmansShiv: { return "Tribalmans Shiv"; }

	case Spy_m_FestiveRevolver: { return "Revolver"; }
	case Spy_m_FestiveAmbassador:
	case Spy_m_TheAmbassador: { return "Ambassador"; }
	case Spy_m_BigKill: { return "Revoler"; }
	case Spy_m_TheDiamondback: { return "Diamondback"; }
	case Spy_m_TheEnforcer: { return "Enforcer"; }
	case Spy_m_LEtranger: { return "Letranger"; }
	case Spy_s_Sapper:
	case Spy_s_SapperR:
	case Spy_s_FestiveSapper: { return "Sapper"; }
	case Spy_s_TheRedTapeRecorder:
	case Spy_s_TheRedTapeRecorderG: { return "Red Tape Recorder"; }
	case Spy_s_TheApSapG: { return "Sapper"; }
	case Spy_s_TheSnackAttack: { return "Sapper"; }
	case Spy_t_FestiveKnife: { return "Knife"; }
	case Spy_t_ConniversKunai: { return "Kunai"; }
	case Spy_t_YourEternalReward: { return "Eternal Reward"; }
	case Spy_t_TheBigEarner: { return "Big Earner"; }
	case Spy_t_TheSpycicle: { return "Spycicle"; }
	case Spy_t_TheSharpDresser: { return "Knife"; }
	case Spy_t_TheWangaPrick: { return "Eternal Reward"; }
	case Spy_t_TheBlackRose: { return "Knife"; }

	case Heavy_m_Deflector_mvm: { return "Deflector"; }
	case Misc_t_FryingPan: { return "Frying Pan"; }
	case Misc_t_GoldFryingPan: { return "Frying Pan"; }
	case Misc_t_Saxxy: { return "Saxxy"; }

	default:
	{
		switch (this->m_hActiveWeapon().Get()->As<CTFWeaponBase>()->GetWeaponID())
		{
			//scout
		case TF_WEAPON_SCATTERGUN: { return "Scattergun"; }
		case TF_WEAPON_HANDGUN_SCOUT_PRIMARY: { return "Shortstop"; }
		case TF_WEAPON_HANDGUN_SCOUT_SECONDARY: { return "Pistol"; }
		case TF_WEAPON_SODA_POPPER: { return "Soda Popper"; }
		case TF_WEAPON_PEP_BRAWLER_BLASTER: { return "Baby Faces Blaster"; }
		case TF_WEAPON_PISTOL_SCOUT: { return "Pistol"; }
		case TF_WEAPON_JAR_MILK: { return "Milk"; }
		case TF_WEAPON_CLEAVER: { return "Cleaver"; }
		case TF_WEAPON_BAT: { return "Bat"; }
		case TF_WEAPON_BAT_WOOD: { return "Sandman"; }
		case TF_WEAPON_BAT_FISH: { return "Bat"; }
		case TF_WEAPON_BAT_GIFTWRAP: { return "Wrap Assassin"; }

								   //soldier
		case TF_WEAPON_ROCKETLAUNCHER: { return "Rocket Launcher"; }
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT: { return "Direct Hot"; }
		case TF_WEAPON_PARTICLE_CANNON: { return "Cow Mangler"; }
		case TF_WEAPON_SHOTGUN_SOLDIER: { return "Shotgun"; }
		case TF_WEAPON_BUFF_ITEM: { return "Buff Banner"; }
		case TF_WEAPON_RAYGUN: { return "Bison"; }
		case TF_WEAPON_SHOVEL: { return "Shovel"; }

							 //pyro
		case TF_WEAPON_FLAMETHROWER: { return "Flame Thrower"; }
		case TF_WEAPON_FLAME_BALL: { return "Dragons Fury"; }
		case TF_WEAPON_SHOTGUN_PYRO: { return "Shotgun"; }
		case TF_WEAPON_FLAREGUN: { return "Flare Gun"; }
		case TF_WEAPON_FLAREGUN_REVENGE: { return "Manmelter"; }
		case TF_WEAPON_JAR_GAS: { return "Gas Passer"; }
		case TF_WEAPON_ROCKETPACK: { return "Thermal Thruster"; }
		case TF_WEAPON_FIREAXE: { return "Fire Axe"; }
		case TF_WEAPON_BREAKABLE_SIGN: { return "Neon Annihilator"; }
		case TF_WEAPON_SLAP: { return "Hot Hand"; }

						   //demoman
		case TF_WEAPON_GRENADELAUNCHER: { return "Grenade Launcher"; }
		case TF_WEAPON_PIPEBOMBLAUNCHER: { return "Stickybomb Launcher"; }
		case TF_WEAPON_CANNON: { return "Loose Cannon"; }
		case TF_WEAPON_BOTTLE: { return "Bottle"; }
		case TF_WEAPON_SWORD: { return "Sword"; }
		case TF_WEAPON_STICKBOMB: { return "Caber"; }

								//heavy
		case TF_WEAPON_MINIGUN: { return "Minigun"; }
		case TF_WEAPON_SHOTGUN_HWG: { return "Shotgun"; }
		case TF_WEAPON_LUNCHBOX: { return "Lunchbox"; }
		case TF_WEAPON_FISTS: { return "Fists"; }

							//engineer
		case TF_WEAPON_SHOTGUN_PRIMARY: { return "Shotgun"; }
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE: { return "Rescue Ranger"; }
		case TF_WEAPON_SENTRY_REVENGE: { return "Frontier Justice"; }
		case TF_WEAPON_DRG_POMSON: { return "Pomson"; }
		case TF_WEAPON_PISTOL: { return "Pistol"; }
		case TF_WEAPON_LASER_POINTER: { return "Wrangler"; }
		case TF_WEAPON_MECHANICAL_ARM: { return "Short Circuit"; }
		case TF_WEAPON_WRENCH: { return "Wrench"; }
		case TF_WEAPON_PDA_ENGINEER_DESTROY: { return "Destruction PDA"; }
		case TF_WEAPON_PDA_ENGINEER_BUILD: { return "Construction PDA"; }
		case TF_WEAPON_BUILDER: { return "Toolbox"; }

							  //medic
		case TF_WEAPON_SYRINGEGUN_MEDIC: { return "Syringe Gun"; }
		case TF_WEAPON_CROSSBOW: { return "Crossbow"; }
		case TF_WEAPON_MEDIGUN: { return "Medigun"; }
		case TF_WEAPON_BONESAW: { return "Bonesaw"; }

							  //sniper
		case TF_WEAPON_SNIPERRIFLE: { return "Sniper Rifle"; }
		case TF_WEAPON_COMPOUND_BOW: { return "Huntsman"; }
		case TF_WEAPON_SNIPERRIFLE_DECAP: { return "Bazaar Bargain"; }
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: { return "Classic"; }
		case TF_WEAPON_SMG: { return "SMG"; }
		case TF_WEAPON_CHARGED_SMG: { return "Cleaners Carbine"; }
		case TF_WEAPON_JAR: { return "Jarate"; }
		case TF_WEAPON_CLUB: { return "Kukri"; }

						   //spy
		case TF_WEAPON_REVOLVER: { return "Revolver"; }
		case TF_WEAPON_PDA_SPY_BUILD: { return "Sapper"; }
		case TF_WEAPON_KNIFE: { return "Knife"; }
		case TF_WEAPON_PDA_SPY: { return "Disguise Kit"; }
		case TF_WEAPON_INVIS: { return "Invis Watch"; }

		case TF_WEAPON_GRAPPLINGHOOK: { return "Grappling Hook"; }

		default: break;
		}
	}
	}

	return "";
}