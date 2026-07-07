#ifndef RADIKCHAT_SERVER_DB_MANAGER_H
#define RADIKCHAT_SERVER_DB_MANAGER_H

#include "sqlite3.h"

bool init_server_database(const char *db_path);

#endif //RADIKCHAT_SERVER_DB_MANAGER_H
