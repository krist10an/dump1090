
SELECT load_extension("libspatialite.so");
SELECT spatialite_version();

SELECT InitSpatialMetadata();

DROP VIEW spatial_trackslog;
CREATE VIEW spatial_trackslog AS
       SELECT t.rowid AS rowid, t.*, MakePoint(lon, lat, 4326) as Geometry
         FROM trackslog t
          WHERE lat>0;

DROP VIEW spatial_flights;
CREATE VIEW spatial_flights AS
       SELECT t.rowid AS rowid, t.modes, t.last_update, f.*, Makeline(t.Geometry) AS Geometry
         FROM spatial_trackslog t
          JOIN Flights f
            ON ( t.flightId = f.FlightID )
            GROUP BY t.flightId;


/* Spatialite 4.0+ */
INSERT INTO views_geometry_columns
    (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column, read_only)
  VALUES ('spatial_trackslog', 'geometry', 'rowid', 'trackslog', 'geometry', 1);

INSERT INTO views_geometry_columns
    (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column, read_only)
  VALUES ('spatial_flights', 'geometry', 'rowid', 'trackslog', 'geometry', 1);

/* Spatialite 3.x */
INSERT INTO views_geometry_columns
    (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column)
  VALUES ('spatial_trackslog', 'geometry', 'rowid', 'trackslog', 'geometry');

INSERT INTO views_geometry_columns
    (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column)
  VALUES ('spatial_flights', 'geometry', 'rowid', 'trackslog', 'geometry');

SELECT * FROM views_geometry_columns;
