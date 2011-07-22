// file      : odb/relational/pgsql/schema.cxx
// author    : Constantin Michael <constantin@codesynthesis.com>
// copyright : Copyright (c) 2009-2011 Code Synthesis Tools CC
// license   : GNU GPL v3; see accompanying LICENSE file

#include <odb/relational/schema.hxx>

#include <odb/relational/pgsql/common.hxx>
#include <odb/relational/pgsql/context.hxx>

namespace relational
{
  namespace pgsql
  {
    namespace schema
    {
      namespace relational = relational::schema;

      //
      // Drop.
      //

      struct drop_common: virtual relational::drop_common
      {
        virtual void
        drop_table (string const& table)
        {
          os << "DROP TABLE IF EXISTS " << quote_id (table) << " CASCADE"
             << endl;
        }
      };

      struct member_drop: relational::member_drop, drop_common
      {
        member_drop (base const& x): base (x) {}
      };
      entry<member_drop> member_drop_;

      struct class_drop: relational::class_drop, drop_common
      {
        class_drop (base const& x): base (x) {}
      };
      entry<class_drop> class_drop_;

      //
      // Create.
      //

      struct object_columns: relational::object_columns, context
      {
        object_columns (base const& x): base (x) {}

        virtual void
        type (semantics::data_member& m)
        {
          if (m.count ("auto"))
          {
            sql_type const& t (column_sql_type (m));

            if (t.type == sql_type::INTEGER)
              os << "SERIAL";
            else if (t.type == sql_type::BIGINT)
              os << "BIGSERIAL";
            else
            {
              cerr << m.file () << ":" << m.line () << ":" << m.column ()
                   << ": error: automatically assigned object id must map "
                   << "to PostgreSQL INTEGER or BIGINT" << endl;

              throw generation_failed ();
            }
          }
          else
          {
            base::type (m);
          }
        }

        virtual void
        reference (semantics::data_member&)
        {
        }
      };
      entry<object_columns> object_columns_;

      struct object_columns_references:
        object_columns_base, relational::common, context
      {
        object_columns_references (emitter& e,
                                   ostream& os,
                                   string const& table,
                                   string const& prefix = string ())
            : relational::common (e, os),
              table_ (table),
              prefix_ (prefix)
        {
        }

        virtual bool
        column (semantics::data_member& m, string const& name, bool)
        {
          semantics::class_* c (object_pointer (member_type (m, prefix_)));

          if (c != 0 && !inverse (m))
          {
            pre_statement ();

            os << "ALTER TABLE " << quote_id (table_) << endl
               << "  ADD FOREIGN KEY (" << quote_id (name) << ")" <<
              endl
               << "  REFERENCES " << quote_id (table_name (*c)) << endl
               << "  INITIALLY DEFERRED" << endl;

            post_statement ();
          }

          return true;
        }

      private:
        string table_;
        string prefix_;
      };

      struct member_create: object_members_base, context
      {
        member_create (emitter& e, ostream& os, relational::tables& tables)
            : object_members_base (false, true),
              e_ (e),
              os_ (os),
              tables_ (tables)
        {
        }

        virtual void
        container (semantics::data_member& m)
        {
          using semantics::type;
          using semantics::data_member;

          // Ignore inverse containers of object pointers.
          //
          if (inverse (m, "value"))
            return;

          string const& name (table_name (m, table_prefix_));

          if (tables_.count (name))
            return;

          type& t (m.type ());
          type& vt (container_vt (t));

          if (semantics::class_* cvt = comp_value (vt))
          {
            object_columns_references ocr (e_, os_, name);
            ocr.traverse_composite (m, *cvt, "value", "value");
          }
          else
          {
            object_columns_references ocr (e_, os_, name, "value");
            string const& name (column_name (m, "value", "value"));
            ocr.column (m, name, true);
          }

          tables_.insert (name);
        }

      private:
        emitter& e_;
        ostream& os_;
        relational::tables& tables_;
      };

      struct class_create: relational::class_create
      {
        class_create (base const& x): base (x) {}

        virtual void
        traverse (type& c)
        {
          if (pass_ != 2)
          {
            base::traverse (c);
            return;
          }

          if (c.file () != unit.file ())
            return;

          if (!c.count ("object") || abstract (c))
            return;

          string const& name (table_name (c));

          if (tables_[pass_].count (name))
            return;

          object_columns_references ocr (e_, os_, name);
          ocr.traverse (c);

          tables_[pass_].insert (name);

          member_create mc (e_, os_, tables_[pass_]);
          mc.traverse (c);
        }
      };
      entry<class_create> class_create_;
    }
  }
}
