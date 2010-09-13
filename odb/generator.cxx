// file      : cli/generator.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2010 Code Synthesis Tools CC
// license   : GNU GPL v3; see accompanying LICENSE file

#include <cctype>  // std::toupper, std::is{alpha,upper,lower}
#include <string>
#include <memory>  // std::auto_ptr
#include <fstream>
#include <iostream>

#include <cutl/fs/auto-remove.hxx>

#include <cutl/compiler/code-stream.hxx>
#include <cutl/compiler/cxx-indenter.hxx>

#include <odb/version.hxx>
#include <odb/context.hxx>
#include <odb/generator.hxx>

#include <odb/tracer/header.hxx>
#include <odb/tracer/inline.hxx>
#include <odb/tracer/source.hxx>

#include <odb/mysql/context.hxx>
#include <odb/mysql/schema.hxx>
#include <odb/mysql/header.hxx>
#include <odb/mysql/inline.hxx>
#include <odb/mysql/source.hxx>

using namespace std;
using namespace cutl;

using semantics::path;

namespace
{
  static char const file_header[] =
    "// This file was generated by ODB, object-relational mapping (ORM)\n"
    "// compiler for C++.\n"
    "//\n\n";

  string
  make_guard (string const& file, context& ctx)
  {
    string g (file);

    // Split words, e.g., "FooBar" to "Foo_Bar" and convert everything
    // to upper case.
    //
    string r;
    for (string::size_type i (0), n (g.size ()); i < n - 1; ++i)
    {
      char c1 (g[i]);
      char c2 (g[i + 1]);

      r += toupper (c1);

      if (isalpha (c1) && isalpha (c2) && islower (c1) && isupper (c2))
        r += "_";
    }
    r += toupper (g[g.size () - 1]);

    return ctx.escape (r);
  }

  void
  open (ifstream& ifs, string const& path)
  {
    ifs.open (path.c_str (), ios_base::in | ios_base::binary);

    if (!ifs.is_open ())
    {
      cerr << path << ": error: unable to open in read mode" << endl;
      throw generator::failed ();
    }
  }

  void
  append (ostream& os, vector<string> const& text, string const& file)
  {
    for (vector<string>::const_iterator i (text.begin ());
         i != text.end (); ++i)
    {
      os << *i << endl;
    }

    if (!file.empty ())
    {
      ifstream ifs;
      open (ifs, file);
      os << ifs.rdbuf ();
    }
  }
}

generator::
generator ()
{
}

void generator::
generate (options const& ops, semantics::unit& unit, path const& p)
{
  try
  {
    path file (p.leaf ());
    string base (file.base ().string ());

    fs::auto_removes auto_rm;

    // C++ output.
    //
    string hxx_name (base + ops.odb_file_suffix () + ops.hxx_suffix ());
    string ixx_name (base + ops.odb_file_suffix () + ops.ixx_suffix ());
    string cxx_name (base + ops.odb_file_suffix () + ops.cxx_suffix ());
    string sql_name (base + ops.sql_suffix ());

    path hxx_path (hxx_name);
    path ixx_path (ixx_name);
    path cxx_path (cxx_name);
    path sql_path (sql_name);

    if (!ops.output_dir ().empty ())
    {
      path dir (ops.output_dir ());
      hxx_path = dir / hxx_path;
      ixx_path = dir / ixx_path;
      cxx_path = dir / cxx_path;
      sql_path = dir / sql_path;
    }

    //
    //
    ofstream hxx (hxx_path.string ().c_str ());

    if (!hxx.is_open ())
    {
      cerr << "error: unable to open '" << hxx_path << "' in write mode"
           << endl;
      throw failed ();
    }

    auto_rm.add (hxx_path);

    //
    //
    ofstream ixx (ixx_path.string ().c_str ());

    if (!ixx.is_open ())
    {
      cerr << "error: unable to open '" << ixx_path << "' in write mode"
           << endl;
      throw failed ();
    }

    auto_rm.add (ixx_path);

    //
    //
    ofstream cxx (cxx_path.string ().c_str ());

    if (!cxx.is_open ())
    {
      cerr << "error: unable to open '" << cxx_path << "' in write mode"
           << endl;
      throw failed ();
    }

    auto_rm.add (cxx_path);

    //
    //
    ofstream sql;

    if (ops.generate_schema ())
    {
      sql.open (sql_path.string ().c_str (), ios_base::out);

      if (!sql.is_open ())
      {
        cerr << "error: unable to open '" << sql_path << "' in write mode"
             << endl;
        throw failed ();
      }

      auto_rm.add (sql_path);
    }

    // Print C++ headers.
    //
    hxx << file_header;
    ixx << file_header;
    cxx << file_header;

    typedef compiler::ostream_filter<compiler::cxx_indenter, char> cxx_filter;

    // Include settings.
    //
    bool br (ops.include_with_brackets ());
    string ip (ops.include_prefix ());
    string gp (ops.guard_prefix ());

    if (!ip.empty () && ip[ip.size () - 1] != '/')
      ip.append ("/");

    if (!gp.empty () && gp[gp.size () - 1] != '_')
      gp.append ("_");

    // HXX
    //
    {
      cxx_filter filt (hxx);
      auto_ptr<context> ctx;

      switch (ops.database ())
      {
      case database::mysql:
        {
          ctx.reset (new mysql::context (hxx, unit, ops));
          break;
        }
      case database::tracer:
        {
          ctx.reset (new context (hxx, unit, ops));
          break;
        }
      }

      string guard (make_guard (gp + hxx_name, *ctx));

      hxx << "#ifndef " << guard << endl
          << "#define " << guard << endl
          << endl;

      // Copy prologue.
      //
      hxx << "// Begin prologue." << endl
          << "//" << endl;
      append (hxx, ops.hxx_prologue (), ops.hxx_prologue_file ());
      hxx << "//" << endl
          << "// End prologue." << endl
          << endl;

      // Version check.
      //
      hxx << "#include <odb/version.hxx>" << endl
          << endl
          << "#if (ODB_VERSION != " << ODB_VERSION << "UL)" << endl
          << "#error ODB runtime version mismatch" << endl
          << "#endif" << endl
          << endl;

      hxx << "#include <odb/pre.hxx>" << endl
          << endl;

      hxx << "#include " << (br ? '<' : '"') << ip << file <<
        (br ? '>' : '"') << endl
          << endl;

      switch (ops.database ())
      {
      case database::mysql:
        {
          mysql::generate_header (static_cast<mysql::context&> (*ctx.get ()));
          break;
        }
      case database::tracer:
        {
          tracer::generate_header (*ctx);
          break;
        }
      }

      hxx << "#include " << (br ? '<' : '"') << ip << ixx_name <<
        (br ? '>' : '"') << endl
          << endl;

      hxx << "#include <odb/post.hxx>" << endl
          << endl;

      // Copy epilogue.
      //
      hxx << "// Begin epilogue." << endl
          << "//" << endl;
      append (hxx, ops.hxx_epilogue (), ops.hxx_epilogue_file ());
      hxx << "//" << endl
          << "// End epilogue." << endl
          << endl;

      hxx << "#endif // " << guard << endl;
    }

    // IXX
    //
    {
      cxx_filter filt (ixx);

      // Copy prologue.
      //
      ixx << "// Begin prologue." << endl
          << "//" << endl;
      append (ixx, ops.ixx_prologue (), ops.ixx_prologue_file ());
      ixx << "//" << endl
          << "// End prologue." << endl
          << endl;

      switch (ops.database ())
      {
      case database::mysql:
        {
          mysql::context ctx (ixx, unit, ops);
          mysql::generate_inline (ctx);
          break;
        }
      case database::tracer:
        {
          context ctx (ixx, unit, ops);
          tracer::generate_inline (ctx);
          break;
        }
      }

      // Copy epilogue.
      //
      ixx << "// Begin epilogue." << endl
          << "//" << endl;
      append (ixx, ops.ixx_epilogue (), ops.ixx_epilogue_file ());
      ixx << "//" << endl
          << "// End epilogue." << endl;
    }

    // CXX
    //
    {
      cxx_filter filt (cxx);

      // Copy prologue.
      //
      cxx << "// Begin prologue." << endl
          << "//" << endl;
      append (cxx, ops.cxx_prologue (), ops.cxx_prologue_file ());
      cxx << "//" << endl
          << "// End prologue." << endl
          << endl;

      cxx << "#include <odb/pre.hxx>" << endl
          << endl;

      cxx << "#include " << (br ? '<' : '"') << ip << hxx_name <<
        (br ? '>' : '"') << endl
          << endl;

      switch (ops.database ())
      {
      case database::mysql:
        {
          mysql::context ctx (cxx, unit, ops);
          mysql::generate_source (ctx);
          break;
        }
      case database::tracer:
        {
          context ctx (cxx, unit, ops);
          tracer::generate_source (ctx);
          break;
        }
      }

      cxx << "#include <odb/post.hxx>" << endl
          << endl;

      // Copy epilogue.
      //
      cxx << "// Begin epilogue." << endl
          << "//" << endl;
      append (cxx, ops.cxx_epilogue (), ops.cxx_epilogue_file ());
      cxx << "//" << endl
          << "// End epilogue." << endl;
    }

    // SQL
    //
    if (ops.generate_schema ())
    {
      // Copy prologue.
      //
      append (sql, ops.sql_prologue (), ops.sql_prologue_file ());

      switch (ops.database ())
      {
      case database::mysql:
        {
          mysql::context ctx (sql, unit, ops);
          mysql::generate_schema (ctx);
          break;
        }
      case database::tracer:
        {
          cerr << "error: the tracer database does not have schema" << endl;
          throw failed ();
        }
      }

      // Copy epilogue.
      //
      append (sql, ops.sql_epilogue (), ops.sql_epilogue_file ());
    }

    auto_rm.cancel ();
  }
  catch (const generation_failed&)
  {
    // Code generation failed. Diagnostics has already been issued.
    //
    throw failed ();
  }
  catch (semantics::invalid_path const& e)
  {
    cerr << "error: '" << e.path () << "' is not a valid filesystem path"
         << endl;
    throw failed ();
  }
  catch (fs::error const&)
  {
    // Auto-removal of generated files failed. Ignore it.
    //
    throw failed ();
  }
}
