#include "bot.h"

#include "../lib/global_values/global_values.h"
#include "../lib/network/socket/socket_utils.h"
#include "console_control.h"
#include "debug.h"

#include <stdio.h>
#include <windows.h>

int main(void)
{
    init_debug(NULL);
    enable_virtual_processing(true);
    set_alternate_console_buffer(true);
    DBG_DEBUG("Initializing winsock...");
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        DBG_FATAL("Error: Failed to initialize winsock");
        return 1;
    }

    DBG_INFO("Which test do you want to use?\n1-connection spamming");
    int choice = getchar();
    fseek(stdin, 0, SEEK_END); // skip all other chars

    if (choice == EOF) {
        DBG_FATAL("getchar() failed");
        return 1;
    }
    choice -= '0';

    switch (choice) {
    case 1:
        SOCKET socket;
        while (true) {
            socket = create_active_socket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
            if (socket == INVALID_SOCKET) {
                DBG_WARNING("Can't connect to the server!");
                return 1;
            }
            closesocket(socket);
        }
        break;
    default:
        DBG_WARNING("Unknown command");
        break;
    }

    return 0;
}
