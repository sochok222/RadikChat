#include "database_utils.h"

#include "debug.h"

#include <stdint.h>
#include <string.h>

static bool is_foreign_keys_enabled(sqlite3 *db);
static bool enable_foreign_keys(sqlite3 *db);

static bool is_foreign_keys_enabled(sqlite3 *db)
{
    const char *stmt_check_fk = "SELECT * FROM pragma_foreign_keys";
    uint16_t stmt_len = strlen(stmt_check_fk);
    sqlite3_stmt *stmt = NULL;

    // Prepare statement
    int prepare_res = sqlite3_prepare_v2(db, stmt_check_fk, stmt_len, &stmt, NULL);
    if (prepare_res != SQLITE_OK) {
        DBG_FATAL("Failed to prepare statement %s. %d %s", stmt_check_fk, prepare_res, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    // execute
    int step_res = sqlite3_step(stmt);
    if (step_res != SQLITE_ROW) {
        step_res = sqlite3_finalize(stmt);
        DBG_FATAL("Failed to step statement! %d %s", step_res, sqlite3_errmsg(db));
        return false;
    }
    if (sqlite3_column_count(stmt) < 1) {
        step_res = sqlite3_finalize(stmt);
        DBG_FATAL("Failed to check if foreign keys are enabled. Query returned 0 rows %d %s", step_res, sqlite3_errmsg(db));
        return false;
    }

    // Means that FK are enabled
    if (sqlite3_column_int(stmt, 0)) {
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

static bool enable_foreign_keys(sqlite3 *db)
{
    const char *stmt_enable_fk = "PRAGMA foreign_keys = ON";
    uint16_t stmt_len = strlen(stmt_enable_fk);
    sqlite3_stmt *stmt = NULL;

    // Prepare statement
    int prepare_res = sqlite3_prepare_v2(db, stmt_enable_fk, stmt_len, &stmt, NULL);
    if (prepare_res != SQLITE_OK) {
        DBG_FATAL("Failed to prepare statement %s. %d %s", stmt_enable_fk, prepare_res, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    // execute
    int step_res = sqlite3_step(stmt);
    if (step_res != SQLITE_DONE) {
        step_res = sqlite3_finalize(stmt);
        DBG_FATAL("Failed to step statement! %d %s", step_res, sqlite3_errmsg(db));
        return false;
    }
    sqlite3_finalize(stmt);

    return true;
}

bool create_database_connection(const char *db_path, sqlite3 **db)
{
    DBG_DEBUG("Creating database connection...");
    sqlite3 *result_db;
    int open_res = sqlite3_open_v2(db_path, &result_db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        NULL);

    if (open_res != SQLITE_OK) {
        DBG_FATAL("Failed to open database! %d %s", open_res, sqlite3_errmsg(result_db));
        sqlite3_close(result_db);
        return false;
    }

    if (!is_foreign_keys_enabled(result_db)) {
        DBG_DEBUG("Foreign keys are not enabled. Trying to enable");
        if (!enable_foreign_keys(result_db)) {
            DBG_FATAL("Failed to enable foreign keys!");
            sqlite3_close(result_db);
            return false;
        }
        if (!is_foreign_keys_enabled(result_db)) {
            DBG_FATAL("Foreign keys are not enabled after try to enable them");
            sqlite3_close(result_db);
            return false;
        }
        DBG_DEBUG("Foreign keys are successfully enabled");
    }
    DBG_DEBUG("Database connection created");

    *db = result_db;
    return true;
}