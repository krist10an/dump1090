CREATE TABLE flightslog ( 
    df          SMALLINT( 2 ),
    modes       CHAR( 6 )          UNIQUE ON CONFLICT REPLACE,
    country     CHAR( 2 ),
    flight      VARCHAR( 7 ),
    airline     CHAR( 3 ),
    squawk      INT( 4 ),
    regn        VARCHAR( 16 ),
    type        VARCHAR( 16 ),
    alt         INT( 6 ),
    vr          SMALLINT( 2 ),
    lat         DECIMAL( 17, 14 ),
    lon         DECIMAL( 17, 14 ),
    heading     INT( 6 ),
    speed       INT( 6 ),
    msgs        INT( 6 ),
    last_update TIMESTAMP          DEFAULT ( CURRENT_TIMESTAMP ) 
);

CREATE TABLE trackslog ( 
    id          INTEGER            PRIMARY KEY AUTOINCREMENT,
    modes       CHAR( 6 ),
    alt         INT( 6 ),
    vr          SMALLINT( 2 ),
    lat         DECIMAL( 17, 14 ),
    lon         DECIMAL( 17, 14 ),
    heading     INT( 6 ),
    speed       INT( 6 ),
    flightid    INT( 8 ),
    signallevel INT ( 2 ),
    last_update TIMESTAMP          DEFAULT ( CURRENT_TIMESTAMP ) 
);

CREATE TABLE session (
  sessionid integer primary key,
  locationid integer not null,
  starttime datetime not null,
  endtime datetime
);

CREATE TABLE flight (
  flightid integer primary key,
  sessionid integer not null,
  modes       CHAR( 6 ),
  flight      VARCHAR( 7 ),
  squawk      INT( 4 ),
  msgs        INT( 6 ),
  starttime datetime not null,
  endtime datetime
);

CREATE TABLE dbversion (
    version  INTEGER NOT NULL,
    created  DATETIME NOT NULL
);

INSERT INTO dbversion (version, created) VALUES (2, CURRENT_TIMESTAMP);
