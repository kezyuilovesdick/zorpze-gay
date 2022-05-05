#include "CheatLoop.h"

void Render() {
	CreateThread(NULL, NULL, Menu, NULL, NULL, NULL);
	DirectOverlaySetOption(Font_ARIAL);
	Setup(DelusionMenu, hwnd);
}


int main()
{
	start_service();

	while (hwnd == NULL)
	{

		printf(" \n");
		hwnd = FindWindow(0, "Fortnite  ");
	}
	initdrv();

	Render();

	HANDLE handle = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(FNCache), nullptr, NULL, nullptr);
	ActorLoop();
	getchar();
	return 0;
}