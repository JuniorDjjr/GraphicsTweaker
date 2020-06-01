#include "plugin.h"
#include "..\injector\assembly.hpp"
#include "IniReader/IniReader.h"
#include "TestCheat.h"

#include "CMenuManager.h"
#include "CGame.h"
#include "CMessages.h"
#include "Fx_c.h"

using namespace plugin;
using namespace std;
using namespace injector;

const string sVersion = "v1.0";

fstream lg;

bool g_AlreadyInit = false;
int g_TempWeatherId = 999;
int g_ForceAnisotropicFilteringLevel;
bool g_ForceColorFilterOnlyIfNotSet;
float g_MultStaticAmbientLighting;
float g_MultDynamicAmbientLighting;
float g_ForceColorFilterR;
float g_ForceColorFilterG;
float g_ForceColorFilterB;
float g_ForceColorFilterA;
float g_MultColorFilterR;
float g_MultColorFilterG;
float g_MultColorFilterB;
float g_MultColorFilterA;
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
		ReadIni();

		Events::processScriptsEvent +=[]
		{
			if (TestCheat("GTINIRELOAD"))
			{
				ReadIni();
				CMessages::AddMessageJumpQ("'GraphicsTweaker.ini' reloaded.", 1500, 0, false);
			}
		};

    }

	static void ReadIni()
	{
		lg.open("GraphicsTweaker.SA.log", fstream::out | fstream::trunc);
		lg << "by Junior_Djjr - MixMods.com.br" << endl;
		lg << "Version " << sVersion << endl;
		lg.flush();

		int i = 0;
		CIniReader ini("GraphicsTweaker.SA.ini");

		if (ini.data.size() <= 0)
		{
			lg << "ERROR: 'GraphicsTweaker.SA.ini' no found." << endl;
			lg.flush();
			return;
		}

		if (ini.ReadInteger("Quality", "ForceAnisotropicFiltering", 0) == true)
		{
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
				MakeJMP(0x74721C, 0x7472F3);
			}
		}

		if (ini.ReadInteger("Timecycle", "EnableTimecycleTweaks", 0) == true)
		{
			lg << "Timecycle tweaks enabled." << endl;

			g_ForceColorFilterOnlyIfNotSet = ini.ReadInteger("Timecycle", "ForceColorFilterOnlyIfNotSet", 0);

			g_ForceColorFilterR = ini.ReadFloat("Timecycle", "ForceColorFilterR", -1.0f);

			g_ForceColorFilterG = ini.ReadFloat("Timecycle", "ForceColorFilterG", -1.0f);

			g_ForceColorFilterB = ini.ReadFloat("Timecycle", "ForceColorFilterB", -1.0f);

			g_ForceColorFilterA = ini.ReadFloat("Timecycle", "ForceColorFilterA", -1.0f);

			g_MultColorFilterR = ini.ReadFloat("Timecycle", "MultColorFilterR", 1.0f);

			g_MultColorFilterG = ini.ReadFloat("Timecycle", "MultColorFilterG", 1.0f);

			g_MultColorFilterB = ini.ReadFloat("Timecycle", "MultColorFilterB", 1.0f);

			g_MultColorFilterA = ini.ReadFloat("Timecycle", "MultColorFilterA", 1.0f);

			g_MultStaticAmbientLighting = ini.ReadFloat("Timecycle", "MultStaticAmbientLighting", 1.0f);

			g_MultDynamicAmbientLighting = ini.ReadFloat("Timecycle", "MultDynamicAmbientLighting", 1.0f);

			g_ForceFarClip = ini.ReadFloat("Timecycle", "ForceFarClip", -1.0f);

			g_ForceFogStart = ini.ReadFloat("Timecycle", "ForceFogStart", -1.0f);

			g_MultFarClip = ini.ReadFloat("Timecycle", "MultFarClip", 1.0f);

			g_MultFogStart = ini.ReadFloat("Timecycle", "MultFogStart", 1.0f);

			// Store current weather id for post timecycle tweaks
			if (!g_AlreadyInit)
			{
				injector::MakeInline<0x55F4B0, 0x55F4B0 + 6>([](injector::reg_pack& regs)
				{
					//mov     edx, [esp+weatherId]
					//mov     eax, ecx
					g_TempWeatherId = *(uint32_t*)(regs.esp + 0x4);
					regs.edx = g_TempWeatherId;
					regs.eax = regs.ecx;
				});

				// Post timecycle tweaks
				injector::MakeInline<0x55F7CF, 0x55F7CF + 10>([](injector::reg_pack& regs)
				{
					*(float*)(regs.eax + 0xA8) = 1.0f; //original code mov dword ptr [eax+0A8h], 3F800000h

					// I'm lazy to doc CTimeCycleCurrent
					lg << g_ForceColorFilterOnlyIfNotSet << endl;

					if (g_ForceColorFilterOnlyIfNotSet)
					{
						// Default color filter if not set (eg Real Linear Graphics)
						if (*(float*)(regs.eax + 0x78) == 0.0f && g_ForceColorFilterR != -1.0f) *(float*)(regs.eax + 0x78) = g_ForceColorFilterR;
						if (*(float*)(regs.eax + 0x7C) == 0.0f && g_ForceColorFilterG != -1.0f) *(float*)(regs.eax + 0x78) = g_ForceColorFilterG;
						if (*(float*)(regs.eax + 0x80) == 0.0f && g_ForceColorFilterB != -1.0f) *(float*)(regs.eax + 0x78) = g_ForceColorFilterB;
						if (*(float*)(regs.eax + 0x84) == 0.0f && g_ForceColorFilterA != -1.0f) *(float*)(regs.eax + 0x78) = g_ForceColorFilterA;
						if (*(float*)(regs.eax + 0x88) == 0.0f && g_ForceColorFilterR != -1.0f) *(float*)(regs.eax + 0x88) = g_ForceColorFilterR;
						if (*(float*)(regs.eax + 0x8C) == 0.0f && g_ForceColorFilterG != -1.0f) *(float*)(regs.eax + 0x8C) = g_ForceColorFilterG;
						if (*(float*)(regs.eax + 0x90) == 0.0f && g_ForceColorFilterB != -1.0f) *(float*)(regs.eax + 0x90) = g_ForceColorFilterB;
						if (*(float*)(regs.eax + 0x94) == 0.0f && g_ForceColorFilterA != -1.0f) *(float*)(regs.eax + 0x94) = g_ForceColorFilterA;
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

					if (g_MultStaticAmbientLighting != 1.0f)
					{
						*(float*)(regs.eax + 0x0) *= g_MultStaticAmbientLighting;
						*(float*)(regs.eax + 0x4) *= g_MultStaticAmbientLighting;
						*(float*)(regs.eax + 0x8) *= g_MultStaticAmbientLighting;
					}

					if (g_MultDynamicAmbientLighting != 1.0f)
					{
						*(float*)(regs.eax + 0x0) *= g_MultDynamicAmbientLighting;
						*(float*)(regs.eax + 0x4) *= g_MultDynamicAmbientLighting;
						*(float*)(regs.eax + 0x8) *= g_MultDynamicAmbientLighting;
					}

					// Not for interiors
					if (CGame::currArea == 0 && g_TempWeatherId < 20)
					{
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
