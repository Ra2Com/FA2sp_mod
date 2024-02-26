#include "Body.h"

#include "../../FA2sp.h"
#include "../CIsoView/Body.h"
#include "../CFinalSunApp/Body.h"

#include <CLoading.h>
#include <CInputMessageBox.h>
#include "../../Miscs/Palettes.h"
#include "../../Helpers/STDHelpers.h"

#include "../../Helpers/Translations.h"

int CFinalSunDlgExt::CurrentLighting = 31000;

void CFinalSunDlgExt::ProgramStartupInit()
{
	// RunTime::ResetMemoryContentAt(0x5937E8, &CFinalSunDlgExt::PreTranslateMessageExt);
	RunTime::ResetMemoryContentAt(0x5937D0, &CFinalSunDlgExt::OnCommandExt);
}

BOOL CFinalSunDlgExt::OnCommandExt(WPARAM wParam, LPARAM lParam)
{
	WORD wmID = LOWORD(wParam);
	WORD wmMsg = HIWORD(wParam);

	HMENU hMenu = *this->GetMenu();
	auto SetLayerStatus = [this, &hMenu](int id, bool& param)
	{
		if (GetMenuState(hMenu, id, MF_BYCOMMAND) & MF_CHECKED)
		{
			param = false;
			CheckMenuItem(hMenu, id, MF_UNCHECKED);
		}
		else
		{
			param = true;
			CheckMenuItem(hMenu, id, MF_CHECKED);
		}
		this->MyViewFrame.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	};

	auto SetLightingStatus = [this, &hMenu](int id)
	{
		CheckMenuRadioItem(hMenu, 31000, 31003, id, MF_UNCHECKED);
		if (CFinalSunDlgExt::CurrentLighting != id)
		{
			CFinalSunDlgExt::CurrentLighting = id;

			PalettesManager::ManualReloadTMP = true;
			PalettesManager::CacheAndTintCurrentIso();
			CLoading::Instance->FreeTMPs();
			CLoading::Instance->InitTMPs();
			PalettesManager::RestoreCurrentIso();
			PalettesManager::ManualReloadTMP = false;

			this->MyViewFrame.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	};

	switch (wmID)
	{
	case 30000:
		SetLayerStatus(30000, CIsoViewExt::DrawStructures);
		return TRUE;
	case 30001:
		SetLayerStatus(30001, CIsoViewExt::DrawInfantries);
		return TRUE;
	case 30002:
		SetLayerStatus(30002, CIsoViewExt::DrawUnits);
		return TRUE;
	case 30003:
		SetLayerStatus(30003, CIsoViewExt::DrawAircrafts);
		return TRUE;
	case 30004:
		SetLayerStatus(30004, CIsoViewExt::DrawBasenodes);
		return TRUE;
	case 30005:
		SetLayerStatus(30005, CIsoViewExt::DrawWaypoints);
		return TRUE;
	case 30006:
		SetLayerStatus(30006, CIsoViewExt::DrawCelltags);
		return TRUE;
	case 30007:
		SetLayerStatus(30007, CIsoViewExt::DrawMoneyOnMap);
		return TRUE;
	case 30008:
		SetLayerStatus(30008, CIsoViewExt::DrawOverlays);
		return TRUE;
	case 30009:
		SetLayerStatus(30009, CIsoViewExt::DrawTerrains);
		return TRUE;
	case 30010:
		SetLayerStatus(30010, CIsoViewExt::DrawSmudges);
		return TRUE;
	case 30011:
		SetLayerStatus(30011, CIsoViewExt::DrawTubes);
		return TRUE;
	case 30012:
		SetLayerStatus(30012, CIsoViewExt::DrawBounds);
		return TRUE;
	case 31000:
	case 31001:
	case 31002:
	case 31003:
		SetLightingStatus(wmID);
		break;
	case 32000:
	case 32001:
	case 32002:
	case 32003:
	{
		HMENU hMenu = *this->GetMenu();
		if (GetMenuState(hMenu, wmID, MF_BYCOMMAND) & MF_CHECKED)
		{
			// set to false
			CIsoViewExt::AutoPropertyBrush[wmID - 32000] = false;
			CheckMenuItem(hMenu, wmID, MF_UNCHECKED);
		}
		else
		{
			// set to true
			CIsoViewExt::AutoPropertyBrush[wmID - 32000] = true;
			CheckMenuItem(hMenu, wmID, MF_CHECKED);
		}
		break;
	}
	case 33000:
	case 33001:
	case 33002:
	case 33003:
	case 33004:
	case 33005:
	case 33006:
	{
		const auto& url = CFinalSunAppExt::ExternalLinks[wmID - 33000].first;
		if (url.empty())
			break;
		if (!ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL))
		{
			std::string buffer = "Failed to open url, try manually: ";
			buffer += url;
			MessageBox(buffer.c_str());
		}
		break;
	}
	default:
		break;
	}

	if (wmID >= 40140 && wmID < 40149)
	{
		auto& file = CFinalSunAppExt::RecentFilesExt[wmID - 40140];
		if (CLoading::IsFileExists(file.c_str()))
			this->LoadMap(file.c_str());
	}

	// navigate to coordinate
	if (wmID == 40134 && CMapData::Instance->MapWidthPlusHeight)
	{
		while (true)
		{
			const ppmfc::CString title = Translations::TranslateOrDefault(
				"NavigateCoordTitle", "Navigate to coordinate"
			);
			const ppmfc::CString message = Translations::TranslateOrDefault(
				"NavigateCoordMessage", "Please input the coordinate in the format like X,Y"
			);
			const ppmfc::CString invalid_title = Translations::TranslateOrDefault(
				"NavigateCoordInvalidTitle", "Error!"
			);

			const auto result = CInputMessageBox::GetString(message, title);
			
			// canceled
			if (STDHelpers::IsNullOrWhitespace(result))
				break;

			const auto data = STDHelpers::SplitString(result);
			if (data.size() != 2)
			{
				const ppmfc::CString invalid_format = Translations::TranslateOrDefault(
					"NavigateCoordInvalidFormat", "Invalid format!"
				);
				::MessageBox(NULL, invalid_format, invalid_title, MB_OK | MB_ICONERROR);
				continue;
			}

			const int x = atoi(data[0]);
			const int y = atoi(data[1]);

			if (!CMapData::Instance->IsCoordInMap(x, y))
			{
				const ppmfc::CString invalid_coord = Translations::TranslateOrDefault(
					"NavigateCoordInvalidCoord", "Invalid coordinate!"
				);
				::MessageBox(NULL, invalid_coord, invalid_title, MB_OK | MB_ICONERROR);
				continue;
			}

			CIsoView::GetInstance()->MoveToMapCoord(x, y);
			break;
		}
	}

	return this->ppmfc::CDialog::OnCommand(wParam, lParam);
}

BOOL CFinalSunDlgExt::PreTranslateMessageExt(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_INITDIALOG:
		;
		///*SetWindowTheme(*this, L"DarkMode_Explorer", NULL);*/
	}
	return ppmfc::CDialog::PreTranslateMessage(pMsg);
}