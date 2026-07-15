#include "miscellaneous.h"

void safe_close_handle(HANDLE handle)
{
    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
}