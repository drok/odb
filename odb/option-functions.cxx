// file      : odb/option-functions.cxx
// copyright : Copyright (c) 2009-2012 Code Synthesis Tools CC
// license   : GNU GPL v3; see accompanying LICENSE file

#include <set>
#include <utility> // std::make_pair()

#include <odb/option-functions.hxx>

using namespace std;

void
process_options (options& o)
{
  database db (o.database ()[0]);

  // If --generate-schema-only was specified, then set --generate-schema
  // as well.
  //
  if (o.generate_schema_only ())
    o.generate_schema (true);

  // Set the default schema format depending on the database.
  //
  if (o.generate_schema () && o.schema_format ().empty ())
  {
    set<schema_format> f;

    switch (db)
    {
    case database::common:
      {
        break; // No schema for common.
      }
    case database::mssql:
    case database::mysql:
    case database::oracle:
    case database::pgsql:
      {
        f.insert (schema_format::sql);
        break;
      }
    case database::sqlite:
      {
        f.insert (schema_format::embedded);
        break;
      }
    }

    o.schema_format (f);
  }

  // Set default --*--file-suffix values.
  //
  {
    database cm (database::common);

    o.odb_file_suffix ().insert (make_pair (cm, string ("-odb")));
    o.sql_file_suffix ().insert (make_pair (cm, string ("")));
    o.schema_file_suffix ().insert (make_pair (cm, string ("-schema")));
  }

  if (o.multi_database () == multi_database::disabled)
  {
    o.odb_file_suffix ().insert (make_pair (db, string ("-odb")));
    o.sql_file_suffix ().insert (make_pair (db, string ("")));
    o.schema_file_suffix ().insert (make_pair (db, string ("-schema")));
  }
  else
  {
    o.odb_file_suffix ().insert (
      make_pair (db, string ("-odb-") + db.string ()));
    o.sql_file_suffix ().insert (
      make_pair (db, string ("-") + db.string ()));
    o.schema_file_suffix ().insert (
      make_pair (db, string ("-schema-") + db.string ()));
  }

  // Set default --default-database value.
  //
  if (!o.default_database_specified ())
  {
    switch (o.multi_database ())
    {
    case multi_database::disabled:
      {
        o.default_database (db);
        o.default_database_specified (true);
        break;
      }
    case multi_database::dynamic:
      {
        o.default_database (database::common);
        o.default_database_specified (true);
        break;
      }
    case multi_database::static_:
      {
        // No default database unless explicitly specified.
        break;
      }
    }
  }
}
