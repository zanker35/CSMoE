#include <SDL.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstdio>

typedef void(*pfnChangeGame)(const char *progname);
typedef int(*pfnInit)(int argc, char **argv, const char *progname, int bChangeGame, pfnChangeGame func);

extern "C" int Host_Main(int szArgc, const char** szArgv, const char* szGameDir, int chg, void* callback);

int main(int argc, char **argv)
{
	// Keep argv[0] as the executable and preserve diagnostics when piped to a log.
	std::setvbuf(stdout, NULL, _IOLBF, 0);
	std::vector<const char*> av(argv, argv + argc);
	return Host_Main(static_cast<int>(av.size()), av.data(), "csmoe", 0, NULL);
}
