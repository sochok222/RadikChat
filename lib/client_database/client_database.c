#include "client_database.h"
#include "debug.h"
#include "sqlite3.h"
#include "database_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool execute_stmt(sqlite3 *db, const char *stmt_string);
static inline bool create_users_table(sqlite3 *db);
static inline bool create_messages_table(sqlite3 *db);

static inline bool create_message_attachments_table(sqlite3 *db);
static inline bool create_message_types_table(sqlite3 *db);
static inline bool create_attachments_table(sqlite3 *db);
static inline bool create_video_attachments_table(sqlite3 *db);
static inline bool create_image_attachments_table(sqlite3 *db);
static inline bool create_document_attachments_table(sqlite3 *db);
static inline bool create_sticker_attachments_table(sqlite3 *db);
static inline bool create_stickers_table(sqlite3 *db);
static inline bool create_sticker_packs_table(sqlite3 *db);

static inline bool create_conversation_table(sqlite3 *db);
static inline bool create_conversation_types_table(sqlite3 *db);
static inline bool create_conversations_members_table(sqlite3 *db);
static inline bool create_conversations_roles_table(sqlite3 *db);
static inline bool create_conversations_roles_permissions_table(sqlite3 *db);
static inline bool create_group_conversations_table(sqlite3 *db);
static inline bool create_channel_conversations_table(sqlite3 *db);
static inline bool create_private_conversations_table(sqlite3 *db);

bool init_client_database(const char *db_path)
{
    // Open database (create file if not exists)
    sqlite3 *db;
    if (!create_database_connection(db_path, &db)) {
        DBG_FATAL("Failed to create database!");
        exit(1);
    }

    // Create needed tables
    bool ((*create_table_fns[])(sqlite3 *db)) = {
        create_users_table,
        create_messages_table,
        create_message_attachments_table,
        create_message_types_table,
        create_attachments_table,
        create_video_attachments_table,
        create_image_attachments_table,
        create_document_attachments_table,
        create_sticker_attachments_table,
        create_stickers_table,
        create_sticker_packs_table,
        create_conversation_table,
        create_conversation_types_table,
        create_conversations_members_table,
        create_conversations_roles_table,
        create_conversations_roles_permissions_table,
        create_group_conversations_table,
        create_channel_conversations_table,
        create_private_conversations_table
    };

    for (register int i = 0; i < sizeof(create_table_fns) / sizeof(create_table_fns[0]); i++) {
        if (!(*create_table_fns[i])(db)) {
            DBG_FATAL("Failed to create table!");
            return false;
        }
    }

    sqlite3_close(db);
    return true;
}

static bool execute_stmt(sqlite3 *db, const char *stmt_string)
{
    const uint16_t stmt_len = strlen(stmt_string);
    sqlite3_stmt *stmt = NULL;

    // Prepare statement
    int prepare_res = sqlite3_prepare_v2(db, stmt_string, stmt_len, &stmt, NULL);
    if (prepare_res != SQLITE_OK) {
        DBG_FATAL("Failed to prepare statement! %d %s", prepare_res, sqlite3_errmsg(db));
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

static bool create_users_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS Users ( "
                              "id INTEGER PRIMARY KEY, "
                              "username TEXT NOT NULL UNIQUE, "
                              "firstName TEXT NOT NULL, "
                              "lastName TEXT)";
    return execute_stmt(db, stmt_string);
}

static inline bool create_messages_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS Messages ("
                              "id INTEGER PRIMARY KEY, "
                              "conversationId REFERENCES Conversations(id) NOT NULL, "
                              "senderId REFERENCES Users(id) NOT NULL, "
                              "messageType REFERENCES MessageTypes(id) NOT NULL, "
                              "text TEXT, "
                              "sentAt INTEGER NOT NULL, "
                              "replyToMessage REFERENCES Messages(id) DEFAULT NULL)";
    return execute_stmt(db, stmt_string);
}

static inline bool create_message_attachments_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS MessageAttachments ("
                              "id INTEGER PRIMARY KEY, "
                              "attachmentId REFERENCES Attachments(id) NOT NULL, "
                              "messageId REFERENCES Messages(id) NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_message_types_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS MessageTypes ("
                              "id INTEGER PRIMARY KEY, "
                              "messageType TEXT UNIQUE NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_attachments_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS Attachments ("
                              "id INTEGER PRIMARY KEY, "
                              "type TEXT NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_video_attachments_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS VideoAttachments ("
                              "id INTEGER PRIMARY KEY, "
                              "attachmentId REFERENCES Attachments(id) NOT NULL, "
                              "path TEXT NOT NULL, "
                              "size INTEGER NOT NULL, "
                              "duration INTEGER NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_image_attachments_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS ImageAttachments ("
                              "id INTEGER PRIMARY KEY, "
                              "attachmentId REFERENCES Attachments(id) NOT NULL, "
                              "path TEXT NOT NULL, "
                              "size INTEGER NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_document_attachments_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS DocumentAttachments ("
                              "id INTEGER PRIMARY KEY, "
                              "attachmentId REFERENCES Attachments(id) NOT NULL, "
                              "path TEXT NOT NULL, "
                              "size INTEGER NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_sticker_attachments_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS StickerAttachments ("
                              "id INTEGER PRIMARY KEY, "
                              "attachmentId REFERENCES Attachments(id) UNIQUE NOT NULL, "
                              "stickerId REFERENCES Stickers(id) NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_stickers_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS Stickers ("
                              "id INTEGER PRIMARY KEY, "
                              "stickerPackId REFERENCES StickerPacks(id) NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_sticker_packs_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS StickerPacks ("
                              "id INTEGER PRIMARY KEY, "
                              "name TEXT NOT NULL UNIQUE);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_conversation_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS Conversations ("
                              "id INTEGER PRIMARY KEY, "
                              "typeId REFERENCES ConversationTypes(id) NOT NULL, "
                              "maxMembers INTEGER NOT NULL, "
                              "createdAt INTEGER NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_conversation_types_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS ConversationTypes ("
                              "id INTEGER PRIMARY KEY, "
                              "conversationType TEXT NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_conversations_members_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS ConversationMembers ("
                              "id INTEGER PRIMARY KEY, "
                              "userId REFERENCES Users(id) NOT NULL, "
                              "conversationId REFERENCES Conversations(id) NOT NULL, "
                              "roleId REFERENCES ConversationRoles(id) NOT NULL, "
                              "joinedAt INTEGER NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_conversations_roles_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS ConversationRoles ("
                              "id INTEGER PRIMARY KEY, "
                              "name TEXT NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_conversations_roles_permissions_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS RolesPermissions ("
                              "id INTEGER PRIMARY KEY, "
                              "roleId REFERENCES Roles(id) NOT NULL, "
                              "permission TEXT NOT NULL);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_group_conversations_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS GroupConversations ("
                              "id INTEGER PRIMARY KEY, "
                              "conversationId REFERENCES Conversations(id) NOT NULL, "
                              "isPrivate BOOL NOT NULL, "
                              "username TEXT UNIQUE, "
                              "name TEXT NOT NULL,"
                              "description TEXT);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_channel_conversations_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS ChannelConversations ("
                              "id INTEGER PRIMARY KEY, "
                              "conversationId REFERENCES Conversations(id) NOT NULL, "
                              "isPrivate BOOL NOT NULL, "
                              "username TEXT UNIQUE, "
                              "name TEXT NOT NULL, "
                              "description TEXT);";
    return execute_stmt(db, stmt_string);
}

static inline bool create_private_conversations_table(sqlite3 *db)
{
    DBG_FUNC();
    const char *stmt_string = "CREATE TABLE IF NOT EXISTS PrivateConversations ("
                              "id INTEGER PRIMARY KEY, "
                              "conversationId REFERENCES Conversations(id) NOT NULL);";
    return execute_stmt(db, stmt_string);
}
