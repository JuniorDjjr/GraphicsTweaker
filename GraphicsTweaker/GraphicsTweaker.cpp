#include "plugin.h"
#include "..\injector\assembly.hpp"
#include "IniReader/IniReader.h"
#include "TestCheat.h"
#include "CTimeCycle.h"

#include "CMenuManager.h"
#include "CGame.h"
#include "CMessages.h"
#include "Fx_c.h"

using namespace plugin;
using namespace std;
using namespace injector;

const string sVersion = "v1.1";

fstream lg;

bool g_AlreadyInit = false;
int g_TempWeatherId = 999;
int g_TempTimeId = 999;
int g_ForceAnisotropicFilteringLevel;
bool g_ForceColorFilterOnlyIfNotSet;
float g_MultStaticAmbientLighting;
float g_MultDynamicAmbientLighting;
float g_ForceStaticInteriorAmbientLighting;
float g_ForceDynamicInteriorAmbientLighting;
int g_TweakDynamicInteriorAmbientLighting;
int g_LimitMaddDoggAmbientLighting;
float g_ForceColorFilterR;
float g_ForceColorFilterG;
float g_ForceColorFilterB;
float g_ForceColorFilterA;
float g_MultColorFilterR;
float g_MultColorFilterG;
float g_MultColorFilterB;
float g_MultColorFilterA;
float g_MultInteriorColorFilterR;
float g_MultInteriorColorFilterG;
float g_MultInteriorColorFilterB;
float g_MultInteriorColorFilterA;
float g_ForceFarClip;
float g_ForceFogStart;
float g_MultFarClip;
float g_MultFogStart;

uint8_t SmartAnisotropicFilteringLevel()
{
	switch (g_fx.GetFxQuality())
	{
	case 0:
		return 2;
	case 1:
		return 4;
	case 2:
		return 8;
	default: // 3 >
		return 16;
	}
}

uint8_t ForceAnisotropicFilteringLevel()
{
	return g_ForceAnisotropicFilteringLevel;
}

class GraphicsTweaker
{
public:
    GraphicsTweaker()
	{
		lg.open("GraphicsTweaker.SA.log", fstream::out | fstream::trunc);
		lg << "by Junior_Djjr - MixMods.com.br" << "\n";
		lg << "Version " << sVersion << "\n";
		lg.flush();

		ReadIni();

		Events::processScriptsEvent +=[]
		{
			if (TestCheat("GTINIRELOAD"))
			{
				lg << "Reloading ini." << "\n";
				ReadIni();
				CMessages::AddMessageJumpQ((char*)"'GraphicsTweaker.ini' reloaded.", 1500, 0, false);
			}
		};

    }

	static void ReadIni()
	{
		int i = 0;
		CIniReader ini("GraphicsTweaker.SA.ini");

		if (ini.data.size() <= 0)
		{
			lg << "ERROR: 'GraphicsTweaker.SA.ini' no found." << "\n";
			lg.flush();
			return;
		}

		if (ini.ReadInteger("Quality", "ForceAnisotropicFiltering", 0) == true)
		{
			lg << "ForceAnisotropicFiltering enabled" << "\n";
			// Force anisotropic filtering
			WriteMemory<uint8_t>(0x730F9C, 0, true);
			if (i = ini.ReadInteger("Quality", "ForceAnisotropicFilteringLevel", 0) > 0) {
				g_ForceAnisotropicFilteringLevel = i;
				if (!g_AlreadyInit) MakeCALL(0x730F9F, ForceAnisotropicFilteringLevel, true);
			}
		}
		else
		{
			if (ini.ReadInteger("Quality", "SmartAnisotropicFiltering", 0) == true)
			{
				lg << "SmartAnisotropicFiltering enabled" << "\n";
				// Use smart anisotropic filtering, by quality settings
				WriteMemory<uint8_t>(0x730F9C, 0, true);
				g_ForceAnisotropicFilteringLevel = 0;
				if (!g_AlreadyInit) MakeCALL(0x730F9F, SmartAnisotropicFilteringLevel, true);
			}
		}

		if (ini.ReadInteger("Quality", "FixBrightnessSettingForDynamic", 0) == true)
		{
			// Brightness setting for dynamic object lighting (peds, objects, vehicles)
			if (!g_AlreadyInit)
			{
				lg << "FixBrightnessSettingForDynamic enabled" << "\n";
				injector::MakeInline<0x561462, 0x561462 + 6>([](injector::reg_pack& regs)
				{
					float brightness = FrontEndMenuManager.m_nBrightness / 255.0f;
					brightness += 0.5f;
					*(float*)(regs.esi + 0xC) *= brightness;
					*(float*)(regs.esi + 0x10) *= brightness;
					*(float*)(regs.esi + 0x14) *= brightness;
				});
			}
		}

		if (ini.ReadInteger("Quality", "DisableGamma", 0) == true)
		{
			if (!g_AlreadyInit)
			{
				lg << "DisableGamma" << "\n";
				MakeJMP(0x74721C, 0x7472F3);
			}
		}

		if (ini.ReadInteger("Timecycle", "EnableTimecycleTweaks", 0) == true)
		{
			g_ForceColorFilterOnlyIfNotSet = ini.ReadInteger("Timecycle", "ForceColorFilterOnlyIfNotSet", 0);

			g_ForceColorFilterR = ini.ReadFloat("Timecycle", "ForceColorFilterR", -1.0f);

			g_ForceColorFilterG = ini.ReadFloat("Timecycle", "ForceColorFilterG", -1.0f);

			g_ForceColorFilterB = ini.ReadFloat("Timecycle", "ForceColorFilterB", -1.0f);

			g_ForceColorFilterA = ini.ReadFloat("Timecycle", "ForceColorFilterA", -1.0f);

			g_MultColorFilterR = ini.ReadFloat("Timecycle", "MultColorFilterR", 1.0f);

			g_MultColorFilterG = ini.ReadFloat("Timecycle", "MultColorFilterG", 1.0f);

			g_MultColorFilterB = ini.ReadFloat("Timecycle", "MultColorFilterB", 1.0f);

			g_MultColorFilterA = ini.ReadFloat("Timecycle", "MultColorFilterA", 1.0f);

			g_MultInteriorColorFilterR = ini.ReadFloat("Timecycle", "MultInteriorColorFilterR", 1.0f);

			g_MultInteriorColorFilterG = ini.ReadFloat("Timecycle", "MultInteriorColorFilterG", 1.0f);

			g_MultInteriorColorFilterB = ini.ReadFloat("Timecycle", "MultInteriorColorFilterB", 1.0f);

			g_MultInteriorColorFilterA = ini.ReadFloat("Timecycle", "MultInteriorColorFilterA", 1.0f);

			g_MultStaticAmbientLighting = ini.ReadFloat("Timecycle", "MultStaticAmbientLighting", 1.0f);

			g_MultDynamicAmbientLighting = ini.ReadFloat("Timecycle", "MultDynamicAmbientLighting", 1.0f);

			g_ForceStaticInteriorAmbientLighting = ini.ReadFloat("Timecycle", "ForceStaticInteriorAmbientLighting", -1.0f);

			g_ForceDynamicInteriorAmbientLighting = ini.ReadFloat("Timecycle", "ForceDynamicInteriorAmbientLighting", -1.0f);

			g_TweakDynamicInteriorAmbientLighting = ini.ReadInteger("Timecycle", "TweakDynamicInteriorAmbientLighting", 0);

			g_LimitMaddDoggAmbientLighting = ini.ReadInteger("Timecycle", "LimitMaddDoggAmbientLighting", 0);

			g_MultDynamicAmbientLighting = ini.ReadFloat("Timecycle", "MultDynamicAmbientLighting", 1.0f);

			g_ForceFarClip = ini.ReadFloat("Timecycle", "ForceFarClip", -1.0f);

			g_ForceFogStart = ini.ReadFloat("Timecycle", "ForceFogStart", -1.0f);

			g_MultFarClip = ini.ReadFloat("Timecycle", "MultFarClip", 1.0f);

			g_MultFogStart = ini.ReadFloat("Timecycle", "MultFogStart", 1.0f);

			// Store current weather id for post timecycle tweaks
			if (!g_AlreadyInit)
			{
				lg << "EnableTimecycleTweaks enabled." << "\n";

				injector::MakeInline<0x55F4B0, 0x55F4B0 + 6>([](injector::reg_pack& regs)
				{
					//mov     edx, [esp+weatherId]
					//mov     eax, ecx
					g_TempWeatherId = *(uint32_t*)(regs.esp + 0x4);
					g_TempTimeId = *(uint32_t*)(regs.esp + 0x8);
					regs.edx = g_TempWeatherId;
					regs.eax = regs.ecx;
				});

				// Post timecycle tweaks
				injector::MakeInline<0x55F7CF, 0x55F7CF + 10>([](injector::reg_pack& regs)
				{
					*(float*)(regs.eax + 0xA8) = 1.0f; //original code mov dword ptr [eax+0A8h], 3F800000h

					// I'm lazy to doc CTimeCycleCurrent

					if (g_ForceColorFilterOnlyIfNotSet)
					{
						// Default color filter if not set
						if (*(float*)(regs.eax + 0x78) == 0.0f &&
							*(float*)(regs.eax + 0x7C) == 0.0f &&
							*(float*)(regs.eax + 0x80) == 0.0f &&
							*(float*)(regs.eax + 0x84) == 0.0f)
						{
							if (g_ForceColorFilterR != -1.0f) *(float*)(regs.eax + 0x78) = g_ForceColorFilterR;
							if (g_ForceColorFilterG != -1.0f) *(float*)(regs.eax + 0x7C) = g_ForceColorFilterG;
							if (g_ForceColorFilterB != -1.0f) *(float*)(regs.eax + 0x80) = g_ForceColorFilterB;
							if (g_ForceColorFilterA != -1.0f) *(float*)(regs.eax + 0x84) = g_ForceColorFilterA;
						}

						if (*(float*)(regs.eax + 0x88) == 0.0f &&
							*(float*)(regs.eax + 0x8C) == 0.0f &&
							*(float*)(regs.eax + 0x90) == 0.0f &&
							*(float*)(regs.eax + 0x94) == 0.0f)
						{
							if (g_ForceColorFilterR != -1.0f) *(float*)(regs.eax + 0x88) = g_ForceColorFilterR;
							if (g_ForceColorFilterG != -1.0f) *(float*)(regs.eax + 0x8C) = g_ForceColorFilterG;
							if (g_ForceColorFilterB != -1.0f) *(float*)(regs.eax + 0x90) = g_ForceColorFilterB;
							if (g_ForceColorFilterA != -1.0f) *(float*)(regs.eax + 0x94) = g_ForceColorFilterA;
						}
					}
					else
					{
						if (g_ForceColorFilterR != -1.0f)
						{
							*(float*)(regs.eax + 0x78) = g_ForceColorFilterR;
							*(float*)(regs.eax + 0x88) = g_ForceColorFilterR;
						}
						if (g_ForceColorFilterG != -1.0f)
						{
							*(float*)(regs.eax + 0x7C) = g_ForceColorFilterG;
							*(float*)(regs.eax + 0x8C) = g_ForceColorFilterG;
						}
						if (g_ForceColorFilterB != -1.0f)
						{
							*(float*)(regs.eax + 0x80) = g_ForceColorFilterB;
							*(float*)(regs.eax + 0x90) = g_ForceColorFilterB;
						}
						if (g_ForceColorFilterA != -1.0f)
						{
							*(float*)(regs.eax + 0x84) = g_ForceColorFilterA;
							*(float*)(regs.eax + 0x94) = g_ForceColorFilterA;
						}
					}

					if (g_MultStaticAmbientLighting != 1.0f)
					{
						*(float*)(regs.eax + 0x0) *= g_MultStaticAmbientLighting;
						*(float*)(regs.eax + 0x4) *= g_MultStaticAmbientLighting;
						*(float*)(regs.eax + 0x8) *= g_MultStaticAmbientLighting;
					}

					if (g_MultDynamicAmbientLighting != 1.0f)
					{
						*(float*)(regs.eax + 0xC) *= g_MultDynamicAmbientLighting;
						*(float*)(regs.eax + 0x10) *= g_MultDynamicAmbientLighting;
						*(float*)(regs.eax + 0x14) *= g_MultDynamicAmbientLighting;
					}

					// Interior
					if (CTimeCycle::m_bExtraColourOn && CTimeCycle::m_ExtraColour == g_TempWeatherId && CTimeCycle::m_ExtraColourWeatherType == g_TempTimeId)
					{
						float colorFilterIllumination = 1.0f;
						if (g_TweakDynamicInteriorAmbientLighting)
						{
							colorFilterIllumination =
								*(float*)(regs.eax + 0x78)
								+ *(float*)(regs.eax + 0x7C)
								+ *(float*)(regs.eax + 0x80)
								+ *(float*)(regs.eax + 0x84)
								+ *(float*)(regs.eax + 0x88)
								+ *(float*)(regs.eax + 0x8C)
								+ *(float*)(regs.eax + 0x90)
								+ *(float*)(regs.eax + 0x94);

							colorFilterIllumination /= 6.0f;
							colorFilterIllumination -= 100.0f;
							if (colorFilterIllumination < 1.0f) {
								colorFilterIllumination = 1.0f;
							}
							else {
								if (colorFilterIllumination < 1.0f) {
									colorFilterIllumination = 1.0f;
								}
								else {
									if (colorFilterIllumination > 40.0f) {
										colorFilterIllumination = 40.0f;
									}
								}
							}
						}

						if (g_ForceDynamicInteriorAmbientLighting != -1.0f)
						{
							//lg << colorFilterIllumination << " " << (g_ForceDynamicInteriorAmbientLighting / colorFilterIllumination) << "\n";
							*(float*)(regs.eax + 0xC) = g_ForceDynamicInteriorAmbientLighting / colorFilterIllumination;
							*(float*)(regs.eax + 0x10) = g_ForceDynamicInteriorAmbientLighting / colorFilterIllumination;
							*(float*)(regs.eax + 0x14) = g_ForceDynamicInteriorAmbientLighting / colorFilterIllumination;
						}

						if (g_ForceStaticInteriorAmbientLighting != -1.0f)
						{
							float lighting = g_ForceStaticInteriorAmbientLighting;
							if (g_TweakDynamicInteriorAmbientLighting)
							{
								colorFilterIllumination /= 5.0f;
								if (colorFilterIllumination < 1.0f) colorFilterIllumination = 1.0f;
								lighting /= colorFilterIllumination;
							}
							*(float*)(regs.eax + 0x0) = lighting;
							*(float*)(regs.eax + 0x4) = lighting;
							*(float*)(regs.eax + 0x8) = lighting;
						}

						if (g_LimitMaddDoggAmbientLighting)
						{
							if (g_TempWeatherId == 0 && g_TempTimeId == 21) // interior used for Madd Dogg
							{
								if (*(float*)(regs.eax + 0x0) > 10.0f) *(float*)(regs.eax + 0x0) = 10.0f;
								if (*(float*)(regs.eax + 0x4) > 10.0f) *(float*)(regs.eax + 0x4) = 10.0f;
								if (*(float*)(regs.eax + 0x8) > 10.0f) *(float*)(regs.eax + 0x8) = 10.0f;
							}
						}

						if (g_MultInteriorColorFilterR != 1.0f)
						{
							*(float*)(regs.eax + 0x78) *= g_MultInteriorColorFilterR;
							*(float*)(regs.eax + 0x88) *= g_MultInteriorColorFilterR;
							if (*(float*)(regs.eax + 0x78) > 255.0f) *(float*)(regs.eax + 0x78) = 255.0f;
							if (*(float*)(regs.eax + 0x88) > 255.0f) *(float*)(regs.eax + 0x88) = 255.0f;
						}
						if (g_MultInteriorColorFilterG != 1.0f)
						{
							*(float*)(regs.eax + 0x7C) *= g_MultInteriorColorFilterG;
							*(float*)(regs.eax + 0x8C) *= g_MultInteriorColorFilterG;
							if (*(float*)(regs.eax + 0x7C) > 255.0f) *(float*)(regs.eax + 0x7C) = 255.0f;
							if (*(float*)(regs.eax + 0x8C) > 255.0f) *(float*)(regs.eax + 0x8C) = 255.0f;
						}
						if (g_MultInteriorColorFilterB != 1.0f)
						{
							*(float*)(regs.eax + 0x80) *= g_MultInteriorColorFilterB;
							*(float*)(regs.eax + 0x90) *= g_MultInteriorColorFilterB;
							if (*(float*)(regs.eax + 0x80) > 255.0f) *(float*)(regs.eax + 0x80) = 255.0f;
							if (*(float*)(regs.eax + 0x90) > 255.0f) *(float*)(regs.eax + 0x90) = 255.0f;
						}
						if (g_MultInteriorColorFilterA != 1.0f)
						{
							*(float*)(regs.eax + 0x84) *= g_MultInteriorColorFilterA;
							*(float*)(regs.eax + 0x94) *= g_MultInteriorColorFilterA;
							if (*(float*)(regs.eax + 0x84) > 255.0f) *(float*)(regs.eax + 0x84) = 255.0f;
							if (*(float*)(regs.eax + 0x94) > 255.0f) *(float*)(regs.eax + 0x94) = 255.0f;
						}

					}
					else
					{
						// Exterior
						if (g_MultColorFilterR != 1.0f)
						{
							*(float*)(regs.eax + 0x78) *= g_MultColorFilterR;
							*(float*)(regs.eax + 0x88) *= g_MultColorFilterR;
							if (*(float*)(regs.eax + 0x78) > 255.0f) *(float*)(regs.eax + 0x78) = 255.0f;
							if (*(float*)(regs.eax + 0x88) > 255.0f) *(float*)(regs.eax + 0x88) = 255.0f;
						}
						if (g_MultColorFilterG != 1.0f)
						{
							*(float*)(regs.eax + 0x7C) *= g_MultColorFilterG;
							*(float*)(regs.eax + 0x8C) *= g_MultColorFilterG;
							if (*(float*)(regs.eax + 0x7C) > 255.0f) *(float*)(regs.eax + 0x7C) = 255.0f;
							if (*(float*)(regs.eax + 0x8C) > 255.0f) *(float*)(regs.eax + 0x8C) = 255.0f;
						}
						if (g_MultColorFilterB != 1.0f)
						{
							*(float*)(regs.eax + 0x80) *= g_MultColorFilterB;
							*(float*)(regs.eax + 0x90) *= g_MultColorFilterB;
							if (*(float*)(regs.eax + 0x80) > 255.0f) *(float*)(regs.eax + 0x80) = 255.0f;
							if (*(float*)(regs.eax + 0x90) > 255.0f) *(float*)(regs.eax + 0x90) = 255.0f;
						}
						if (g_MultColorFilterA != 1.0f)
						{
							*(float*)(regs.eax + 0x84) *= g_MultColorFilterA;
							*(float*)(regs.eax + 0x94) *= g_MultColorFilterA;
							if (*(float*)(regs.eax + 0x84) > 255.0f) *(float*)(regs.eax + 0x84) = 255.0f;
							if (*(float*)(regs.eax + 0x94) > 255.0f) *(float*)(regs.eax + 0x94) = 255.0f;
						}

						if (g_ForceFarClip != -1.0f) *(float*)(regs.eax + 0x50) = g_ForceFarClip;
						if (g_ForceFogStart != -1.0f) *(float*)(regs.eax + 0x54) = g_ForceFogStart;

						if (g_MultFarClip != 1.0f) *(float*)(regs.eax + 0x50) *= g_MultFarClip;
						if (g_MultFogStart != 1.0f) *(float*)(regs.eax + 0x54) *= g_MultFogStart;

						if (*(float*)(regs.eax + 0x54) > (*(float*)(regs.eax + 0x50) * 0.1f)) *(float*)(regs.eax + 0x54) = (*(float*)(regs.eax + 0x50) * 0.1f); // fog start can't be greater than far clip
						if (*(float*)(regs.eax + 0x54) < 3.0f) *(float*)(regs.eax + 0x54) = 3.0f; // fog start can't be so low
					}
				});
			}
		}
		lg.flush();
		g_AlreadyInit = true;
	}
} graphicsTweaker;
