#include "dump1090.h"
#include <sqlite3.h>

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

void connectSQL() {
    int rc;
    fprintf(stderr, "ConnectSQL ");
    rc = sqlite3_open("basestation.sqb", &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sql_connected = 0;
    } else {
        sql_connected = 1;
        fprintf(stderr, "=OK\n");
#ifdef USE_SPATIALITE
        sqlite3_enable_load_extension (db, 1);

        int rc;
        char *sql;
        char *zErrMsg = 0;

        sql = sqlite3_mprintf("SELECT load_extension('libspatialite.so')");
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK )  {
            fprintf(stderr, "Unable to load extension: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sql_connected = 0;
        }
#endif
    }
}

void disconnectSQL() {
    fprintf(stderr, "DisconnectSQL\n");
    if (sql_connected) {
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
        sql = sqlite3_mprintf( "INSERT OR IGNORE INTO flights (modes, alt, squawk, df, msgs) VALUES ('%06X', '%d', '%d', '%d', '%ld'); UPDATE flightslog SET modes='%06X', alt='%d', squawk='%d', df='%d', msgs='%d', last_update=CURRENT_TIMESTAMP WHERE modes='%06X';",mm->addr, mm->altitude, mm->modeA, mm->msgtype, a->messages,mm->addr, mm->altitude, mm->modeA, mm->msgtype, a->messages);
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


#ifdef USE_SPATIALITE
#define COLUMNSTR ", Geometry"
#define VALUESTR ", MakePoint(%f,%f, 4326)"
#else
#define COLUMNSTR ""
#define VALUESTR ""
#endif

        if (mm->bFlags & MODES_ACFLAGS_LATLON_VALID) {
            sql = sqlite3_mprintf( "INSERT INTO trackslog (modes, alt, vr, lat, lon, speed, heading, last_update"
            COLUMNSTR
            ") VALUES ('%06X', '%d', '%d', '%1.5f', '%1.5f', '%d', '%d', CURRENT_TIMESTAMP "
            VALUESTR
            ");", a->addr, a->altitude, a->vert_rate, a->lat, a->lon, a->speed, a->track
#ifdef USE_SPATIALITE
            , a->lat, a->lon
#endif
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
