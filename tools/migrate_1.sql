ALTER TABLE trackslog ADD COLUMN flightid    INT( 8 );
ALTER TABLE trackslog ADD COLUMN signallevel INT( 1 );

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
