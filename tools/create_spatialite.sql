
SELECT load_extension("libspatialite.so");

SELECT InitSpatialMetadata();

CREATE VIEW spatial_trackslog AS
       SELECT t.*, MakePoint(lon, lat, 4326) as Geometry
         FROM trackslog t
          WHERE lat>0;

CREATE VIEW spatial_flights AS
       SELECT t.modes, t.last_update, f.*, Makeline(t.Geometry) AS Geometry
         FROM spatial_trackslog t
          JOIN Flights f
            ON ( t.flightId = f.FlightID )
            GROUP BY t.flightId;

