
SELECT load_extension("libspatialite.so");

SELECT InitSpatialMetadata();

DROP VIEW spatial_trackslog;
CREATE VIEW spatial_trackslog AS
       SELECT t.rowid AS ROWID, t.*, MakePoint(lon, lat, 4326) as Geometry
         FROM trackslog t
          WHERE lat>0;

INSERT INTO views_geometry_columns
    (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column)
  VALUES ('spatial_trackslog', 'geometry', 'ROWID', 'trackslog', 'geometry');


DROP VIEW spatial_flights;
CREATE VIEW spatial_flights AS
       SELECT t.rowid AS ROWID, t.modes, t.last_update, f.*, Makeline(t.Geometry) AS Geometry
         FROM spatial_trackslog t
          JOIN Flights f
            ON ( t.flightId = f.FlightID )
            GROUP BY t.flightId;

INSERT INTO views_geometry_columns
    (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column)
  VALUES ('spatial_flights', 'geometry', 'ROWID', 'trackslog', 'geometry');

SELECT * FROM views_geometry_columns;
