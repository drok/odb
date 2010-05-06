// file      : odb/parser.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2010 Code Synthesis Tools CC
// license   : GNU GPL v2; see accompanying LICENSE file

#ifndef ODB_PARSER_HXX
#define ODB_PARSER_HXX

#include <odb/gcc.hxx>

#include <memory>  // std::auto_ptr

#include <odb/pragma.hxx>
#include <odb/options.hxx>
#include <odb/semantics/unit.hxx>

class parser
{
public:
  class failed {};

  parser (options const&, loc_pragmas const&, decl_pragmas const&);

  std::auto_ptr<semantics::unit>
  parse (tree global_scope, semantics::path const& main_file);

private:
  parser (parser const&);

  parser&
  operator= (parser const&);

private:
  class impl;
  std::auto_ptr<impl> impl_;
};

#endif // ODB_PARSER_HXX