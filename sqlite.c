#include "dump1090.h"
#include <sqlite3.h>
#include <inttypes.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    MODES_NOTUSED(NotUsed);
    int i;
    for(i=0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

sqlite3 *db;
int sql_connected = 0;
uint32_t sql_session_id = 0;

void connectSQL() {
    int rc;
    char *sql;
    char *zErrMsg = 0;

    fprintf(stdout, "ConnectSQL ");
    rc = sqlite3_open("basestation.sqb", &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sql_connected = 0;
    } else {
        sql_connected = 1;
        fprintf(stdout, "=OK\n");

        // Create session
        sql = sqlite3_mprintf("INSERT INTO Sessions (LocationID, StartTime) VALUES (1, CURRENT_TIMESTAMP)");
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK )  {
            fprintf(stderr, "Unable to add session: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sql_session_id = 0;
        } else {
            long rowid = sqlite3_last_insert_rowid(db);
            sql_session_id = (uint32_t)rowid;
        }
        fprintf(stdout, "Using Session id %d\n", sql_session_id);

    }
}

void disconnectSQL() {
    int rc;
    char *sql;
    char *zErrMsg = 0;

    fprintf(stdout, "DisconnectSQL\n");
    if (sql_connected) {

        // Timeout aircrafts
        struct aircraft *a = Modes.aircrafts;
        while(a) {
            modesRemoveStaleSQL(a);
            a = a->next;
        }

        fprintf(stdout, "Stopping session id %d\n", sql_session_id);

        // Stop session
        sql = sqlite3_mprintf("UPDATE Sessions SET Endtime=CURRENT_TIMESTAMP WHERE SessionID='%d'", sql_session_id);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK )  {
            fprintf(stderr, "Error closing session: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        // Close database connection
        fprintf(stdout, "Disconnect db\n");
        sqlite3_close(db);
    }
}

void modesFeedSQL(struct modesMessage *mm, struct aircraft *a) {
// code here
    char *zErrMsg = 0;
    int rc;
    char *sql;
    if ( !sql_connected ) {
        exit(0);
    } else {

    if (a->flightId == 0) {
        // Add Unique "session id" to use for aircraft while it is within visible range
#if 0
        struct tm *seentm = gmtime(&a->seen);
        a->flightId = seentm->tm_hour*10000 + seentm->tm_min*100 + seentm->tm_sec;
#else
        sql = sqlite3_mprintf( "INSERT INTO flights (sessionid, aircraftid, starttime) VALUES ('%d', '%d', '%" PRIu64 "');",
                sql_session_id, 1, a->timestamp);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK )  {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            long rowid = sqlite3_last_insert_rowid(db);
            fprintf(stdout, "Aircraft %d flight id %ld\n", a->addr, rowid);
            a->flightId = (uint32_t)rowid;
        }
#endif

    }

//char msgFlights[1000], *p = msgFlights;

///////////////////////////////////////////////////
/// INSERT into sqlite db
///////////////////////////////////////////////////

    /* we flill a live 'flights' table - update old data */
    /* DF 0 (Short Air to Air, ACAS has: altitude, icao) */
    if (mm->msgtype == 0) {
        sql = sqlite3_mprintf( "INSERT OR IGNORE INTO flightslog (modes, alt, df, msgs) VALUES ('%06X', '%d', '%d', '%ld'); UPDATE flightslog SET modes='%06X',alt='%d',df='%d',msgs='%ld',last_update=CURRENT_TIMESTAMP WHERE modes='%06X';",mm->addr, mm->altitude, mm->msgtype, a->messages,mm->addr, mm->altitude, mm->msgtype, a->messages,mm->addr);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK )  {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
             sqlite3_free(zErrMsg);
        } else {
            //fprintf(stdout, "DF 0 == OK\n");
        }
    }


    /* DF 4/20 (Surveillance (roll call) Altitude has: altitude, icao, flight status, DR, UM) */
    /* TODO flight status, DR, UM */
    /*
    if (mm->msgtype == 4 || mm->msgtype == 20) {
        sql = sqlite3_mprintf( "INSERT INTO flights (modes, alt, df, msgs) VALUES ('%06X', '%d', '%d', '%ld')",mm->addr, a->altitude, mm->msgtype, a->messages);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
              sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "msgtype 4/20 == OK\n");
        }
    }
    */

    /* DF 5/21 (Surveillance (roll call) IDENT Reply, has: alt, icao, flight status, DR, UM, squawk) */
    if (mm->msgtype == 5 || mm->msgtype == 21) {
        sql = sqlite3_mprintf( "INSERT OR IGNORE INTO flightslog (modes, alt, squawk, df, msgs) VALUES ('%06X', '%d', '%d', '%d', '%ld'); UPDATE flightslog SET modes='%06X', alt='%d', squawk='%d', df='%d', msgs='%d', last_update=CURRENT_TIMESTAMP WHERE modes='%06X';",mm->addr, mm->altitude, mm->modeA, mm->msgtype, a->messages,mm->addr, mm->altitude, mm->modeA, mm->msgtype, a->messages);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            //fprintf(stdout, "DF %d== OK\n",mm->msgtype);
        }
    }

    /* DF 11 */
    /*
    if (mm->msgtype == 11) {
        sql = sqlite3_mprintf( "INSERT INTO flights (modes, df, msgs) VALUES ('%06X', '%d', '%ld')",mm->addr, mm->msgtype, a->messages);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "msgtype 11 == OK\n");
        }
    }
    */

    /* DF17 *with or without position data */

    if (mm->msgtype == 17) {
        sql = sqlite3_mprintf( "INSERT OR IGNORE INTO flightslog (df, flight, airline, modes, alt, vr, lat, lon, speed, heading, msgs) VALUES ('%d', '%s', '%3s', '%06X', '%d', '%d', '%1.5f', '%1.5f', '%d', '%d', '%ld'); UPDATE flightslog SET df='%d',flight='%s',airline='%3s',modes='%06X',alt='%d',vr='%d',lat='%1.5f',lon='%1.5f',speed='%d',heading='%d',msgs='%ld',last_update=CURRENT_TIMESTAMP WHERE modes='%06X';",
            mm->msgtype, a->flight, a->flight, mm->addr, mm->altitude, mm->vert_rate, a->lat, a->lon, a->speed,a->track, a->messages
            ,mm->msgtype, a->flight, a->flight, mm->addr, mm->altitude, mm->vert_rate, a->lat, a->lon, a->speed, a->track, a->messages);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            //fprintf(stdout, "DF 17 == OK\n");
        }

        if (mm->bFlags & MODES_ACFLAGS_LATLON_VALID) {
            sql = sqlite3_mprintf( "INSERT INTO trackslog (modes, alt, vr, lat, lon, speed, heading, flightId, last_update"
            ") VALUES ('%06X', '%d', '%d', '%1.5f', '%1.5f', '%d', '%d', '%lu', CURRENT_TIMESTAMP "
            ");", a->addr, a->altitude, a->vert_rate, a->lat, a->lon, a->speed, a->track, a->flightId
          );

            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
            if( rc != SQLITE_OK ) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            } else {
                //fprintf(stdout, "DF 17 FL == OK\n");
            }
        }
    }
    }
/////////////////////////
}


void modesRemoveStaleSQL(struct aircraft *a) {
// code here
    char *zErrMsg = 0;
    int rc;
    char *sql;
    fprintf(stdout, "Completed aircraft %d flight id %d\n", a->addr, a->flightId);
    if ( !sql_connected ) {
        exit(0);
    } else {
        sql = sqlite3_mprintf( "UPDATE flights SET EndTime=CURRENT_TIMESTAMP,NumModeSMsgRec='%d' WHERE flightid='%d';",
                a->messages, a->flightId);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK )  {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
}
