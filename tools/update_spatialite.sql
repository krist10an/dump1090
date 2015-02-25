
SELECT load_extension("libspatialite.so");

UPDATE trackslog SET Geometry=MakePoint(lon, lat, 4326) WHERE lat>0;

/*
# Query furthest away points (converted to meters)
#SELECT Distance(MakePoint(10.3, 63.3, 4326),trackslog.Geometry) AS distance FROM trackslog WHERE lat>0 ORDER BY distance DESC LIMIT 10;
*/
SELECT Distance(Transform(MakePoint(10.3, 63.3, 4326),25832),Transform(trackslog.Geometry,25832)) AS distance FROM trackslog WHERE lat>0 ORDER BY distance DESC LIMIT 10;
