CREATE VIEW planes AS
       SELECT f.modes,
              a.modescountry,
              a.registration,
              a.type,
              f.last_update
         FROM flightslog f
              JOIN Aircraft a
                ON ( f.modes = a.modes );
