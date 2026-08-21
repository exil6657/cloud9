#ifdef _WIN32
#include <windows.h>

// Optional shell only. It deliberately does not install hooks, scan another
// process, or touch Minecraft memory. A host must call the normal Cloud9 API
// explicitly after loading this library.
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(module);
        break;
    default:
        break;
    }
    return TRUE;
}
#endif
