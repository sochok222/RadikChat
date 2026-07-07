#ifndef RADIKCHAT_DATABASE_UTILS_H
#define RADIKCHAT_DATABASE_UTILS_H

#include "sqlite3.h"

bool create_database_connection(const char *db_path, sqlite3 **db);

#endif //RADIKCHAT_DATABASE_UTILS_H
