/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * *****************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "sqlite3.h"
#include "logger.h"
#include "oic_malloc.h"
#include "provisioningdatabasemanager.h"
#include "pmutility.h"
#include "oic_string.h"
#include "utlist.h"


#define DB_FILE "PDM.db"

#define TAG "PDM"

#define PDM_STALE_STATE 1
#define PDM_ACTIVE_STATE 0

#define PDM_FIRST_INDEX 0
#define PDM_SECOND_INDEX 1

#define PDM_BIND_INDEX_FIRST 1
#define PDM_BIND_INDEX_SECOND 2
#define PDM_BIND_INDEX_THIRD 3

#define PDM_CREATE_T_DEVICE_LIST "create table T_DEVICE_LIST(ID INTEGER PRIMARY KEY AUTOINCREMENT,\
                                  UUID BLOB NOT NULL UNIQUE);"

#define PDM_CREATE_T_DEVICE_LINK  "create table T_DEVICE_LINK_STATE(ID INT NOT NULL, ID2 INT NOT \
                                   NULL,STATE INT NOT NULL, PRIMARY KEY (ID, ID2));"
/**
 * Macro to verify sqlite success.
 * eg: VERIFY_NON_NULL(TAG, ptrData, ERROR,OC_STACK_ERROR);
 */
#define PDM_VERIFY_SQLITE_OK(tag, arg, logLevel, retValue) do{ if (SQLITE_OK != (arg)) \
            { OC_LOG_V((logLevel), tag, "Error in " #arg ", Error Message: %s", \
               sqlite3_errmsg(g_db)); return retValue; }}while(0)

#define PDM_SQLITE_TRANSACTION_BEGIN "BEGIN TRANSACTION;"
#define PDM_SQLITE_TRANSACTION_COMMIT "COMMIT;"
#define PDM_SQLITE_TRANSACTION_ROLLBACK "ROLLBACK;"
#define PDM_SQLITE_GET_STALE_INFO "SELECT ID,ID2 FROM T_DEVICE_LINK_STATE WHERE STATE = ?"
#define PDM_SQLITE_INSERT_T_DEVICE_LIST "INSERT INTO T_DEVICE_LIST VALUES(?,?)"
#define PDM_SQLITE_GET_ID "SELECT ID FROM T_DEVICE_LIST WHERE UUID like ?"
#define PDM_SQLITE_INSERT_LINK_DATA "INSERT INTO T_DEVICE_LINK_STATE VALUES(?,?,?)"
#define PDM_SQLITE_DELETE_LINK "DELETE FROM T_DEVICE_LINK_STATE WHERE ID = ? and ID2 = ?"
#define PDM_SQLITE_DELETE_DEVICE_LINK "DELETE FROM T_DEVICE_LINK_STATE WHERE ID = ? or ID2 = ?"
#define PDM_SQLITE_DELETE_DEVICE "DELETE FROM T_DEVICE_LIST  WHERE ID = ?"
#define PDM_SQLITE_UPDATE_LINK "UPDATE T_DEVICE_LINK_STATE SET STATE = ?  WHERE ID = ? and ID2 = ?"
#define PDM_SQLITE_LIST_ALL_UUID "SELECT UUID FROM T_DEVICE_LIST"
#define PDM_SQLITE_GET_UUID "SELECT UUID FROM T_DEVICE_LIST WHERE ID = ?"
#define PDM_SQLITE_GET_LINKED_DEVICES "SELECT ID,ID2 FROM T_DEVICE_LINK_STATE WHERE ID = ? or ID2 = ?"
#define PDM_SQLITE_GET_DEVICE_LINKS "SELECT ID,ID2 FROM T_DEVICE_LINK_STATE WHERE ID = ? and ID2 = ?"

#define ASCENDING_ORDER(id1, id2) do{if( (id1) > (id2) )\
  { int temp; temp = id1; id1 = id2; id2 = temp; }}while(0)

#define CHECK_PDM_INIT(tag) do{if(true != gInit)\
  { OC_LOG(ERROR, (tag), "PDB is not initialized"); \
    return OC_STACK_PDM_IS_NOT_INITIALIZED; }}while(0)

static sqlite3 *g_db = NULL;
static bool gInit = false;  /* Only if we can open sqlite db successfully, gInit is true. */

/**
 * function to create DB in case DB doesn't exists
 */
static OCStackResult createDB(const char* path)
{

    int result = 0;
    result = sqlite3_open_v2(path, &g_db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, result, ERROR, OC_STACK_ERROR);

    result = sqlite3_exec(g_db, PDM_CREATE_T_DEVICE_LIST, NULL, NULL, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, result, ERROR, OC_STACK_ERROR);

    OC_LOG(INFO, TAG, "Created T_DEVICE_LIST");
    result = sqlite3_exec(g_db, PDM_CREATE_T_DEVICE_LINK, NULL, NULL, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, result, ERROR, OC_STACK_ERROR);

    OC_LOG(INFO, TAG, "Created T_DEVICE_LINK_STATE");
    gInit = true;
    return OC_STACK_OK;
}


/**
 * Function to begin any transaction
 */
static OCStackResult begin()
{
    int res = 0;
    res = sqlite3_exec(g_db, PDM_SQLITE_TRANSACTION_BEGIN, NULL, NULL, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);
    return OC_STACK_OK;
}

/**
 * Function to commit any transaction
 */
static OCStackResult commit()
{
    int res = 0;
    res = sqlite3_exec(g_db, PDM_SQLITE_TRANSACTION_COMMIT, NULL, NULL, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);
    return OC_STACK_OK;
}

/**
 * Function to rollback any transaction
 */
static OCStackResult rollback()
{
    int res = 0;
    res = sqlite3_exec(g_db, PDM_SQLITE_TRANSACTION_ROLLBACK, NULL, NULL, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);
    return OC_STACK_OK;
}

/**
 * Error log callback called by SQLite stack in case of error
 */
void errLogCallback(void *pArg, int iErrCode, const char *zMsg)
{
    (void) pArg;
    (void) iErrCode;
    (void) zMsg;
    OC_LOG_V(DEBUG,TAG, "(%d) %s", iErrCode, zMsg);
}

OCStackResult PDMInit(const char *path)
{
    int rc;
    const char *dbPath = NULL;
    if (SQLITE_OK !=  sqlite3_config(SQLITE_CONFIG_LOG, errLogCallback, NULL))
    {
        OC_LOG(INFO, TAG, "Unable to enable debug log of sqlite");
    }

    if (NULL == path || !*path)
    {
        dbPath = DB_FILE;
    }
    else
    {
        dbPath = path;
    }
    rc = sqlite3_open_v2(dbPath, &g_db, SQLITE_OPEN_READWRITE, NULL);
    if (SQLITE_OK != rc)
    {
        OC_LOG_V(INFO, TAG, "ERROR: Can't open database: %s", sqlite3_errmsg(g_db));
        return createDB(dbPath);
    }
    gInit = true;
    return OC_STACK_OK;
}


OCStackResult PDMAddDevice(const OicUuid_t *UUID)
{
    CHECK_PDM_INIT(TAG);
    if (NULL == UUID)
    {
        return OC_STACK_INVALID_PARAM;
    }

    sqlite3_stmt *stmt = 0;
    int res =0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_INSERT_T_DEVICE_LIST,
                              strlen(PDM_SQLITE_INSERT_T_DEVICE_LIST) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_blob(stmt, PDM_BIND_INDEX_SECOND, UUID, UUID_LENGTH, SQLITE_STATIC);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_step(stmt);
    if (SQLITE_DONE != res)
    {
        if (SQLITE_CONSTRAINT == res)
        {
            //new OCStack result code
            OC_LOG_V(ERROR, TAG, "Error Occured: %s",sqlite3_errmsg(g_db));
            sqlite3_finalize(stmt);
            return OC_STACK_DUPLICATE_UUID;
        }
        OC_LOG_V(ERROR, TAG, "Error Occured: %s",sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return OC_STACK_ERROR;
    }
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

/**
 *function to get Id for given UUID
 */
static OCStackResult getIdForUUID(const OicUuid_t *UUID , int *id)
{
    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_GET_ID, strlen(PDM_SQLITE_GET_ID) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_blob(stmt, PDM_BIND_INDEX_FIRST, UUID, UUID_LENGTH, SQLITE_STATIC);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    OC_LOG(DEBUG, TAG, "Binding Done");
    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        int tempId = sqlite3_column_int(stmt, PDM_FIRST_INDEX);
        OC_LOG_V(DEBUG, TAG, "ID is %d", tempId);
        *id = tempId;
        sqlite3_finalize(stmt);
        return OC_STACK_OK;
    }
    sqlite3_finalize(stmt);
    return OC_STACK_INVALID_PARAM;
}

/**
 * Function to check duplication of device's Device ID.
 */
OCStackResult PDMIsDuplicateDevice(const OicUuid_t* UUID, bool *result)
{

    CHECK_PDM_INIT(TAG);
    if (NULL == UUID || NULL == result)
    {
        OC_LOG(ERROR, TAG, "UUID or result is NULL");
        return OC_STACK_INVALID_PARAM;
    }
    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_GET_ID, strlen(PDM_SQLITE_GET_ID) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_blob(stmt, PDM_BIND_INDEX_FIRST, UUID, UUID_LENGTH, SQLITE_STATIC);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    OC_LOG(DEBUG, TAG, "Binding Done");
    bool retValue = false;
    while(SQLITE_ROW == sqlite3_step(stmt))
    {
        OC_LOG(INFO, TAG, "Duplicated UUID");
        retValue = true;
    }

    sqlite3_finalize(stmt);
    *result = retValue;
    return OC_STACK_OK;
}

/**
 * Function to add link in sqlite
 */
static OCStackResult addlink(int id1, int id2)
{
    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_INSERT_LINK_DATA,
                              strlen(PDM_SQLITE_INSERT_LINK_DATA) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, id1);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_SECOND, id2);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_THIRD, PDM_ACTIVE_STATE);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        OC_LOG_V(ERROR, TAG, "Error Occured: %s",sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return OC_STACK_ERROR;
    }
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

OCStackResult PDMLinkDevices(const OicUuid_t *UUID1, const OicUuid_t *UUID2)
{
    CHECK_PDM_INIT(TAG);
    if (NULL == UUID1 || NULL == UUID2)
    {
        OC_LOG(ERROR, TAG, "Invalid PARAM");
        return  OC_STACK_INVALID_PARAM;
    }
    int id1 = 0;
    if (OC_STACK_OK != getIdForUUID(UUID1, &id1))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }
    int id2 = 0;
    if (OC_STACK_OK != getIdForUUID(UUID2, &id2))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }
    ASCENDING_ORDER(id1, id2);
    return addlink(id1, id2);
}

/**
 * Function to remove created link
 */
static OCStackResult removeLink(int id1, int id2)
{
    int res = 0;
    sqlite3_stmt *stmt = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_DELETE_LINK, strlen(PDM_SQLITE_DELETE_LINK) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, id1);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_SECOND, id2);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    if (SQLITE_DONE != sqlite3_step(stmt))
    {
        OC_LOG_V(ERROR, TAG, "Error message: %s", sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return OC_STACK_ERROR;
    }
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

OCStackResult PDMUnlinkDevices(const OicUuid_t *UUID1, const OicUuid_t *UUID2)
{
    CHECK_PDM_INIT(TAG);
    if (NULL == UUID1 || NULL == UUID2)
    {
        OC_LOG(ERROR, TAG, "Invalid PARAM");
        return  OC_STACK_INVALID_PARAM;
    }

    int id1 = 0;
    if (OC_STACK_OK != getIdForUUID(UUID1, &id1))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }

    int id2 = 0;
    if (OC_STACK_OK != getIdForUUID(UUID2, &id2))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }
    ASCENDING_ORDER(id1, id2);
    return removeLink(id1, id2);
}

static OCStackResult removeFromDeviceList(int id)
{
    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_DELETE_DEVICE,
                              strlen(PDM_SQLITE_DELETE_DEVICE) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, id);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        OC_LOG_V(ERROR, TAG, "Error message: %s", sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return OC_STACK_ERROR;
    }
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

OCStackResult PDMDeleteDevice(const OicUuid_t *UUID)
{
    CHECK_PDM_INIT(TAG);
    if (NULL == UUID)
    {
        return OC_STACK_INVALID_PARAM;
    }
    int id = 0;
    if (OC_STACK_OK != getIdForUUID(UUID, &id))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }
    begin();
    if(OC_STACK_OK != removeFromDeviceList(id))
    {
        rollback();
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_ERROR;
    }
    commit();
    return OC_STACK_OK;
}


static OCStackResult updateLinkState(int id1, int id2, int state)
{
    sqlite3_stmt *stmt = 0;
    int res = 0 ;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_UPDATE_LINK,
                              strlen(PDM_SQLITE_UPDATE_LINK) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, state);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_SECOND, id1);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_THIRD, id2);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    if (SQLITE_DONE != sqlite3_step(stmt))
    {
        OC_LOG_V(ERROR, TAG, "Error message: %s", sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return OC_STACK_ERROR;
    }
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

OCStackResult PDMSetLinkStale(const OicUuid_t* uuidOfDevice1, const OicUuid_t* uuidOfDevice2)
{
    CHECK_PDM_INIT(TAG);
    if (NULL == uuidOfDevice1 || NULL == uuidOfDevice2)
    {
        OC_LOG(ERROR, TAG, "Invalid PARAM");
        return  OC_STACK_INVALID_PARAM;
    }

    int id1 = 0;
    if (OC_STACK_OK != getIdForUUID(uuidOfDevice1, &id1))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }

    int id2 = 0;
    if (OC_STACK_OK != getIdForUUID(uuidOfDevice2, &id2))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }
    ASCENDING_ORDER(id1, id2);
    return updateLinkState(id1, id2, PDM_STALE_STATE);

}

OCStackResult PDMGetOwnedDevices(OCUuidList_t **uuidList, size_t *numOfDevices)
{
    CHECK_PDM_INIT(TAG);
    if (NULL != *uuidList)
    {
        OC_LOG(ERROR, TAG, "Not null list will cause memory leak");
        return OC_STACK_INVALID_PARAM;
    }
    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_LIST_ALL_UUID,
                              strlen(PDM_SQLITE_LIST_ALL_UUID) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    int counter  = 0;
    while (SQLITE_ROW == sqlite3_step(stmt))
    {

        const void *ptr = sqlite3_column_blob(stmt, PDM_FIRST_INDEX);
        OicUuid_t *uid = (OicUuid_t *)ptr;
        OCUuidList_t *temp = (OCUuidList_t *) OICCalloc(1,sizeof(OCUuidList_t));
        if (NULL == temp)
        {
            OC_LOG_V(ERROR, TAG, "Memory allocation problem");
            sqlite3_finalize(stmt);
            return OC_STACK_NO_MEMORY;
        }
        memcpy(&temp->dev.id, uid->id, UUID_LENGTH);
        LL_PREPEND(*uuidList,temp);
        ++counter;
    }
    *numOfDevices = counter;
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

static OCStackResult getUUIDforId(int id, OicUuid_t *uid)
{
    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_GET_UUID,
                              strlen(PDM_SQLITE_GET_UUID) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, id);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        const void *ptr = sqlite3_column_blob(stmt, PDM_FIRST_INDEX);
        memcpy(uid, ptr, sizeof(OicUuid_t));
    }
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

OCStackResult PDMGetLinkedDevices(const OicUuid_t *UUID, OCUuidList_t **UUIDLIST, size_t *numOfDevices)
{
    CHECK_PDM_INIT(TAG);
    if (NULL != *UUIDLIST)
    {
        OC_LOG(ERROR, TAG, "Not null list will cause memory leak");
        return OC_STACK_INVALID_PARAM;
    }
    if (NULL == UUID)
    {
        return OC_STACK_INVALID_PARAM;
    }
    int id = 0;
    if (OC_STACK_OK != getIdForUUID(UUID, &id))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }

    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_GET_LINKED_DEVICES,
                              strlen(PDM_SQLITE_GET_LINKED_DEVICES) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, id);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_SECOND, id);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    int counter  = 0;
    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        int i1 = sqlite3_column_int(stmt, PDM_FIRST_INDEX);
        int i2 = sqlite3_column_int(stmt, PDM_SECOND_INDEX);

        OicUuid_t temp = {{0,}};
        if (i1 != id)
        {
            getUUIDforId(i1, &temp);
        }
        if (i2 != id)
        {
            getUUIDforId(i2, &temp);
        }

        OCUuidList_t *tempNode = (OCUuidList_t *) OICCalloc(1,sizeof(OCUuidList_t));
        if (NULL == tempNode)
        {
            OC_LOG(ERROR, TAG, "No Memory");
            sqlite3_finalize(stmt);
            return OC_STACK_NO_MEMORY;
        }
        memcpy(&tempNode->dev.id, &temp.id, UUID_LENGTH);
        LL_PREPEND(*UUIDLIST,tempNode);
        ++counter;
    }
    *numOfDevices = counter;
     sqlite3_finalize(stmt);
     return OC_STACK_OK;
}

OCStackResult PDMGetToBeUnlinkedDevices(OCPairList_t **staleDevList, size_t *numOfDevices)
{
    CHECK_PDM_INIT(TAG);
    if (NULL != *staleDevList)
    {
        OC_LOG(ERROR, TAG, "Not null list will cause memory leak");
        return OC_STACK_INVALID_PARAM;
    }

    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_GET_STALE_INFO,
                              strlen(PDM_SQLITE_GET_STALE_INFO) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, PDM_STALE_STATE);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    int counter  = 0;
    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        int i1 = sqlite3_column_int(stmt, PDM_FIRST_INDEX);
        int i2 = sqlite3_column_int(stmt, PDM_SECOND_INDEX);
        OicUuid_t temp1 = {{0,}};
        OicUuid_t temp2 = {{0,}};;
        getUUIDforId(i1, &temp1);
        getUUIDforId(i2, &temp2);

        OCPairList_t *tempNode = (OCPairList_t *) OICCalloc(1, sizeof(OCPairList_t));
        if (NULL == tempNode)
        {
            OC_LOG(ERROR, TAG, "No Memory");
            sqlite3_finalize(stmt);
            return OC_STACK_NO_MEMORY;
        }
        memcpy(&tempNode->dev.id, &temp1.id, UUID_LENGTH);
        memcpy(&tempNode->dev2.id, &temp2.id, UUID_LENGTH);
        LL_PREPEND(*staleDevList, tempNode);
        ++counter;
    }
    *numOfDevices = counter;
    sqlite3_finalize(stmt);
    return OC_STACK_OK;
}

OCStackResult PDMClose()
{
    CHECK_PDM_INIT(TAG);
    int res = 0;
    res = sqlite3_close(g_db);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);
    return OC_STACK_OK;
}

void PDMDestoryOicUuidLinkList(OCUuidList_t* ptr)
{
    if(ptr)
    {
        OCUuidList_t *tmp1 = NULL,*tmp2=NULL;
        LL_FOREACH_SAFE(ptr, tmp1, tmp2)
        {
            LL_DELETE(ptr, tmp1);
            OICFree(tmp1);
        }
    }
}

void PDMDestoryStaleLinkList(OCPairList_t* ptr)
{
    if(ptr)
    {
        OCPairList_t *tmp1 = NULL,*tmp2=NULL;
        LL_FOREACH_SAFE(ptr, tmp1, tmp2)
        {
            LL_DELETE(ptr, tmp1);
            OICFree(tmp1);
        }
    }
}

OCStackResult PDMIsLinkExists(const OicUuid_t* uuidOfDevice1, const OicUuid_t* uuidOfDevice2,
                               bool* result)
{
    CHECK_PDM_INIT(TAG);
    if (NULL == uuidOfDevice1 || NULL == uuidOfDevice2 || NULL == result)
    {
        return OC_STACK_INVALID_PARAM;
    }
    int id1 = 0;
    int id2 = 0;
    if (OC_STACK_OK != getIdForUUID(uuidOfDevice1, &id1))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }

    if (OC_STACK_OK != getIdForUUID(uuidOfDevice2, &id2))
    {
        OC_LOG(ERROR, TAG, "Requested value not found");
        return OC_STACK_INVALID_PARAM;
    }

    ASCENDING_ORDER(id1, id2);

    sqlite3_stmt *stmt = 0;
    int res = 0;
    res = sqlite3_prepare_v2(g_db, PDM_SQLITE_GET_DEVICE_LINKS,
                              strlen(PDM_SQLITE_GET_DEVICE_LINKS) + 1, &stmt, NULL);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_FIRST, id1);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    res = sqlite3_bind_int(stmt, PDM_BIND_INDEX_SECOND, id2);
    PDM_VERIFY_SQLITE_OK(TAG, res, ERROR, OC_STACK_ERROR);

    bool ret = false;
    while(SQLITE_ROW == sqlite3_step(stmt))
    {
        OC_LOG(INFO, TAG, "Link already exists between devices");
        ret = true;
    }
    sqlite3_finalize(stmt);
    *result = ret;
    return OC_STACK_OK;
}
