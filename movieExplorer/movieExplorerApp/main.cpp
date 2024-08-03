#include "DrawThread.h"
#include "GuiMain.h"
#include "commonObject.h"
#include <thread>
#include "DownloadThread.h"

int main() {
	DrawThread draw;
	DownloadThread down;
	common common_obj;
	auto draw_thread = std::thread([&] {draw.call_Gui_main(common_obj); });
	auto down_thread = std::thread([&] {down.download(common_obj); });

	draw_thread.join();
	down_thread.join();
}
