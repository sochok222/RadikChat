#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "chatsManager.h"
#define LOG_TO_FILE
#include "clientUtils.h"
#include "consoleControl.h"
#include "consoleInput.h"
#include "consoleOutput.h"
#include "contactsManager.h"

#include <conio.h>
#include <debug.h>
#include <process.h>
#include <socketUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <synchapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "client.h"

#include "request.h"

#define BUFSIZE 1024
#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

SOCKET  socket_server = INVALID_SOCKET;
HANDLE  socket_thread_run_mutex;
AppData app_data;

static void init_console(void)
{
    init_debug("logs.log");
    enable_virtual_processing(true);
    set_alternate_console_buffer(true);
    disable_selection(true);
    hide_cursor();
    init_console_size();

    lock_console_size(true);
    _beginthread(console_draw_thread, 0, 0);
}

int main(void)
{
    WSADATA wsadata;
    int     choice;
    HANDLE  socket_thread_mutex;
    app_data.contact_count = 0;
    app_data.message_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    socket_thread_run_mutex = CreateMutex(NULL, TRUE, NULL);

    init_console();
    init_requests();
    // Set fixed console size
    if (get_console_width() < 119 || get_console_height() < 29) {
        printf("Invalid console size %dx%d.\nPlease, resize to 119x29 or larger size and start the program again.\n",
               get_console_width(), get_console_height());
        return 0;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }

reconnect:
    print_notification(formatDefault, "Trying to connect...");
    socket_server = create_active_socket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
    if (socket_server == INVALID_SOCKET) {
        // printf("Error creating socket\n");
        print_notification(formatError, "Can't connect to the server. Press any button to try again or q to quit");
        if (getch() == 'q') {
            system("cls");
            return 0;
        }
        goto reconnect;
    }
    socket_thread_mutex = (HANDLE)_beginthread(socket_thread, 0, NULL);

    if (!log_in(socket_server)) {
        print_error("Can't login\n");
        closesocket(socket_server);
        WSACleanup();
        return 1;
    }
    printSuccess("Login success\n");

    socket_server_mutex = CreateMutex(NULL, FALSE, NULL);

    while (1) {
        print_request("Enter command: 1-list chats; 2-create chat; 0 - quit: ");

        choice = read_char(false) - '0';
        clear_request();

        switch (choice) {
        case 0:
            goto exit;
        case 1:
            show_private_chats();
            break;
        case 2:
            create_chat(socket_server);
            break;
        default:
            print_error("Invalid choice");
        }
    }

    exit:
    lock_console_size(false);
    disable_selection(false);
    if (socket_server != INVALID_SOCKET)
        closesocket(socket_server);

    // Wait while thread stops
    ReleaseMutex(socket_thread_run_mutex);
    WaitForSingleObject(socket_thread_mutex, INFINITE);

    WSACleanup();
}