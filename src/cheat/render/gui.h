#include "menu.h"
JBMenu menu;

namespace GUI
{
	bool Visible;
	WNDPROC NextWndProc;
	enum Button {
		NoPress,
		Pressed,
		Clicked
	};
	struct IO {
		Button LKM;
		Button RKM;
		bool Click;

		vec2_t MousePos;
		vec2_t MouseDelta;
		vec2_t OldMousePos;
		USHORT CurElement;
	} IO;
	struct WndData {
		vec2_t WndPos;
		vec2_t Size;
		vec2_t Pos;
	} CurWnd;

	//str hash
	unsigned short __forceinline HashStr(const wchar_t* Str)
	{
		unsigned char i;
		unsigned short crc = 0xFFFF;
		while (wchar_t DChar = *Str++) {
			unsigned char Char = (unsigned char)DChar;
			crc ^= Char << 8;
			for (i = 0; i < 8; i++)
				crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
			Char = (unsigned char)(DChar >> 8);
			crc ^= Char << 8;
			for (i = 0; i < 8; i++)
				crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
		} return crc;
	}

	vec2_t __forceinline CenterLine(const vec2_t& Pos) {
		return { (Pos.x + (CurWnd.Size.x / 2)), Pos.y };
	}

	bool __forceinline InRect(vec2_t Rect, vec2_t Size, vec2_t Dot) {
		return (Dot.x > Rect.x && Dot.x < Rect.x + Size.x && Dot.y > Rect.y && Dot.y < Rect.y + Size.y);
	}

	//input
	LRESULT __stdcall WndProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		int value = 0;
		switch (Msg)
		{
			case WM_LBUTTONDOWN:
				IO.LKM = Pressed;
				break;

			case WM_LBUTTONUP:
				if (IO.LKM == Pressed)
					IO.LKM = Clicked;
				else IO.LKM = NoPress;
				break;

			case WM_RBUTTONDOWN:
				IO.RKM = Pressed;
				break;

			case WM_RBUTTONUP:
				IO.RKM = NoPress;
				break;

			case WM_KEYUP:
				if (wParam == VK_INSERT)
					Visible = !Visible;

				if (wParam == VK_DOWN)
				{
					menu.Cur_Pos++;
					if (menu.sOptions[menu.Cur_Pos].Name == 0)
						menu.Cur_Pos--;
				}
				if (wParam == VK_UP)
				{
					menu.Cur_Pos--;
					if (menu.Cur_Pos == -1)
						menu.Cur_Pos++;
				}
				if (wParam == VK_RIGHT)
				{
					if (menu.sOptions[menu.Cur_Pos].Type == T_INT)
					{
						int func = *menu.sOptions[menu.Cur_Pos].Function;
						if (func < menu.sOptions[menu.Cur_Pos].limit)
							value++;
					}
					else
					{
						int func = *menu.sOptions[menu.Cur_Pos].Function;
						if (func < 1)
							value++;
					}
				}
				if (wParam == VK_LEFT)
				{
					if (*menu.sOptions[menu.Cur_Pos].Function > 0)
						value--;
				}
				if (value) {
					*menu.sOptions[menu.Cur_Pos].Function += value;
					if (menu.sOptions[menu.Cur_Pos].Type == T_FOLDER)
					{
						memset(&menu.sOptions, 0, sizeof(menu.sOptions));
						menu.Items = 0;
					}
				}
				break;

			case WM_MOUSEMOVE:
				IO.MousePos.x = (signed short)(lParam);
				IO.MousePos.y = (signed short)(lParam >> 16);
				break;
		}

		return 0;
		//return FC(user32, CallWindowProcA, NextWndProc, Wnd, Msg, wParam, lParam);
	}

	LRESULT __stdcall WndHook(int nCode, WPARAM wParam, LPARAM lParam)
	{
		MSG* WndProcData = (MSG*)lParam;
		return WndProc(WndProcData->hwnd, WndProcData->message, WndProcData->wParam, WndProcData->lParam);
	}

	void ProcessInput(bool End = false)
	{
		if (!End)
		{
			//calc mouse delta
			//if (IO.LKM == Clicked) sp("111");
			//IO.Updated = true;
			IO.MouseDelta = IO.MousePos - IO.OldMousePos;
			Global::MouseDelta = IO.MouseDelta;
			IO.OldMousePos = IO.MousePos;
		}

		else //if (IO.Updated)
		{
			if (IO.LKM == Clicked) {
				IO.LKM = NoPress;
				//IO.CurElement = 0;
				//sp("click");
			}

			if(IO.LKM == NoPress)
				IO.CurElement = 0;
		//	//update LKM button
		//	if (IO.LKM == Clicked)
		//		IO.LKM = Pressed;
		}
	}

	bool InputMgr(const wchar_t* Name, bool Reg = false) {
		unsigned short StrHash = HashStr(Name);
		if (Reg && !IO.CurElement) {
			IO.CurElement = StrHash;
			return true;
		}
		else if (!Reg)
			return (IO.CurElement == StrHash);
		return false;
	}

	//main
	void SliderFloat(const wchar_t* Name, float* Current, float Min, float Max)
	{
		auto Format = xorstr(L"%s : %d");
		Render::String({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, FC_FORMAT_W(Format.crypt_get(), Name, (int)*Current));
		CurWnd.Pos.y += 15.f;

		if (IO.LKM == Pressed && InRect({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, { CurWnd.Size.x - 20.f, 16.f }, IO.MousePos)) {
			float Val = ((((IO.MousePos.x - 18.f - CurWnd.Pos.x) / (CurWnd.Size.x - 36.f)) * (Max - Min)) + Min);
			*Current = ((Val > Max) ? Max : ((Val < Min) ? Min : Val)); InputMgr(Name, true);
		}

		float CurOff = (*Current - Min) / (Max - Min);
		Render::FillRoundedRectangle({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, { CurWnd.Size.x - 20.f, 16.f }, Render::Color(127, 255, 0), 8.f);
		Render::FillCircle({ CurWnd.Pos.x + 18.f + ((CurWnd.Size.x - 36.f) * CurOff), CurWnd.Pos.y + 8 }, Render::Color(255, 255, 255), 8.f);
		CurWnd.Pos.y += 20.f;
	}

	void SliderInt(const wchar_t* Name, int* Current, int Min, int Max)
	{
		auto Format = xorstr(L"%s : %d");
		Render::String({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, FC_FORMAT_W(Format.crypt_get(), Name, (int)*Current));
		CurWnd.Pos.y += 15.f;

		if (IO.LKM == Pressed && InRect({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, { CurWnd.Size.x - 20.f, 16.f }, IO.MousePos)) {
			int Val = ((((IO.MousePos.x - 18.f - CurWnd.Pos.x) / (CurWnd.Size.x - 36.f)) * (Max - Min)) + Min);
			*Current = ((Val > Max) ? Max : ((Val < Min) ? Min : Val)); InputMgr(Name, true);
		}

		int CurOff = (*Current - Min) / (Max - Min);
		Render::FillRoundedRectangle({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, { CurWnd.Size.x - 20.f, 16.f }, Render::Color(127, 255, 0), 8.f);
		Render::FillCircle({ CurWnd.Pos.x + 18.f + ((CurWnd.Size.x - 36.f) * CurOff), CurWnd.Pos.y + 8 }, Render::Color(255, 255, 255), 8.f);
		CurWnd.Pos.y += 20.f;
	}

	void CheckBox(const wchar_t* Name, bool& Switch) {
		Render::String({ CurWnd.Pos.x + 42.f, CurWnd.Pos.y }, (wchar_t*)Name);
		if (IO.LKM == Pressed && InRect({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, { 30.f, 16.f }, IO.MousePos) && InputMgr(Name, true)) Switch = !Switch;
		Render::FillRoundedRectangle({ CurWnd.Pos.x + 10.f, CurWnd.Pos.y }, { 30.f, 16.f }, Switch ? Render::Color(127, 255, 0) : Render::Color(255, 0, 0), 8.f);
		Render::FillCircle({ CurWnd.Pos.x + 18.f + (Switch ? 14.f : 0.f), CurWnd.Pos.y + 8.f }, Render::Color(255, 255, 255), 8.f); CurWnd.Pos.y += 20.f;
	}

	void Spacing(const wchar_t* Name) {
		CurWnd.Pos.y += 4.f;
		vec2_t CntLine = CenterLine(CurWnd.Pos); vec2_t TextSize = Render::String(CntLine, Name, true) / 2.f;
		Render::Line({ CntLine.x - TextSize.x - 10.f, CntLine.y }, { CurWnd.Pos.x + 4.f, CurWnd.Pos.y }, Render::Color(255, 255, 255));
		Render::Line({ CntLine.x + TextSize.x + 10.f, CntLine.y }, { CurWnd.Pos.x + CurWnd.Size.x - 4.f, CurWnd.Pos.y }, Render::Color(255, 255, 255));
		CurWnd.Pos.y += 8.f;
	}

	void Begin(const wchar_t* Name, vec2_t& Pos, const const vec2_t& Size, const Render::Color& Clr)
	{
		//base menu
		ProcessInput();
		if (!CurWnd.WndPos.zero()) Pos = CurWnd.WndPos;
		Render::FillRoundedRectangle(Pos, Size, Clr, 8.f); 
		Render::String(CenterLine({ Pos.x, Pos.y + 10.f }), Name, true);
		CurWnd.WndPos = { Pos.x, Pos.y }; CurWnd.Size = Size;
		CurWnd.Pos = { Pos.x, Pos.y + 25.f };
	}
	
	void End()
	{
		ProcessInput(true);

		//drag window
		if (InputMgr(E(L"##Drag")) || (IO.LKM == Pressed && InRect(CurWnd.WndPos, CurWnd.Size, IO.MousePos) && InputMgr(E(L"##Drag"), true)))
			CurWnd.WndPos += IO.MouseDelta;
	}
}