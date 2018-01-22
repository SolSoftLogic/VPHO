#include <windows.h>

int main(int argc, char **argv);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	char *argv[5];
	unsigned t;
	WCHAR ws[300];

	argv[0] = "encdec";
	argv[1] = "e";
	argv[2] = "yv12";
	argv[3] = "176x144.yuv";
	argv[4] = "176x144.avc";
	MessageBox(0, L"Encoding 176x144.yuv", L"encdec", 0);
	t = GetTickCount();
	main(5, argv);
	wsprintf(ws, L"Encoding took %d ms", GetTickCount() - t);
	MessageBox(0, ws, L"encdec", 0);
	argv[0] = "encdec";
	argv[1] = "d";
	argv[2] = "i420";
	argv[3] = "176x144.avc";
	argv[4] = "176x144dec.yuv";
	t = GetTickCount();
	main(5, argv);
	wsprintf(ws, L"Decoding took %d ms", GetTickCount() - t);
	MessageBox(0, ws, L"encdec", 0);
	return 0;
}
