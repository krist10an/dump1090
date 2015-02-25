
SELECT load_extension("libspatialite.so");

SELECT InitSpatialMetadata();
SELECT AddGeometryColumn('trackslog', 'Geometry', 4326, 'POINT','XY');
