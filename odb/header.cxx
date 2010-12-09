// file      : odb/header.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2010 Code Synthesis Tools CC
// license   : GNU GPL v3; see accompanying LICENSE file

#include <odb/common.hxx>
#include <odb/header.hxx>

namespace
{
  struct data_member: traversal::data_member, context
  {
    data_member (context& c)
        : context (c)
    {
    }

    virtual void
    traverse (semantics::data_member& m)
    {
      if (m.count ("transient"))
        return;

      string const& name (public_name (m));
      string const& type (m.type ().fq_name (m.belongs ().hint ()));

      os << "static " << type << "&" << endl
         << name << " (value_type&);"
         << endl;

      os << "static const " << type << "&" << endl
         << name << " (const value_type&);"
         << endl;
    }
  };

  struct class_: traversal::class_, context
  {
    class_ (context& c)
        : context (c), member_ (c)
    {
      member_names_ >> member_;
    }

    virtual void
    traverse (type& c)
    {
      if (c.file () != unit.file ())
        return;

      if (!comp_value (c))
        return;

      string const& type (c.fq_name ());

      os << "// " << c.name () << endl
         << "//" << endl;

      os << "template <>" << endl
         << "class access::value_traits< " << type << " >"
         << "{"
         << "public:" << endl;

      // value_type
      //
      os << "typedef " << type << " value_type;"
         << endl;

      names (c, member_names_);

      os << "};";
    }

  private:
    data_member member_;
    traversal::names member_names_;
  };
}

void
generate_header (context& ctx)
{
  ctx.os << "#include <memory>" << endl
         << "#include <cstddef>" << endl // std::size_t
         << endl;

  ctx.os << "#include <odb/core.hxx>" << endl
         << "#include <odb/traits.hxx>" << endl
         << "#include <odb/pointer-traits.hxx>" << endl;

  // In case of a boost TR1 implementation, we cannot distinguish
  // between the boost::shared_ptr and std::tr1::shared_ptr usage since
  // the latter is just a using-declaration for the former. To resolve
  // this we will include TR1 traits if the Boost TR1 header is included.
  //
  if (ctx.unit.count ("tr1-pointer-used") &&
      ctx.unit.get<bool> ("tr1-pointer-used"))
  {
    ctx.os << "#include <odb/tr1/pointer-traits.hxx>" << endl;
  }
  else if (ctx.unit.count ("boost-pointer-used") &&
           ctx.unit.get<bool> ("boost-pointer-used"))
  {
    ctx.os << "#ifdef BOOST_TR1_MEMORY_HPP_INCLUDED" << endl
           << "#  include <odb/tr1/pointer-traits.hxx>" << endl
           << "#endif" << endl;
  }

  ctx.os << "#include <odb/container-traits.hxx>" << endl;

  if (ctx.options.generate_query ())
    ctx.os << "#include <odb/result.hxx>" << endl;

  ctx.os << endl;


  /*
  traversal::unit unit;
  traversal::defines unit_defines;
  traversal::namespace_ ns;
  class_ c (ctx);

  unit >> unit_defines >> ns;
  unit_defines >> c;

  traversal::defines ns_defines;

  ns >> ns_defines >> ns;
  ns_defines >> c;

  ctx.os << "namespace odb"
         << "{";

  unit.dispatch (ctx.unit);

  ctx.os << "}";
  */
}
