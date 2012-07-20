// file      : odb/diagnostics.hxx
// copyright : Copyright (c) 2009-2012 Code Synthesis Tools CC
// license   : GNU GPL v3; see accompanying LICENSE file

#ifndef ODB_DIAGNOSTICS_HXX
#define ODB_DIAGNOSTICS_HXX

#include <odb/gcc-fwd.hxx>

#include <cstddef>
#include <iostream>

#include <cutl/fs/path.hxx>

using std::endl;

std::ostream&
error (cutl::fs::path const&, std::size_t line, std::size_t clmn);

std::ostream&
warn (cutl::fs::path const&, std::size_t line, std::size_t clmn);

std::ostream&
info (cutl::fs::path const&, std::size_t line, std::size_t clmn);

std::ostream&
error (location_t);

std::ostream&
warn (location_t);

std::ostream&
info (location_t);

//
//
class cxx_lexer;

std::ostream&
error (cxx_lexer&);

std::ostream&
warn (cxx_lexer&);

std::ostream&
info (cxx_lexer&);

// location_t macro wrappers.
//
cutl::fs::path
location_file (location_t);

std::size_t
location_line (location_t);

std::size_t
location_column (location_t);

#endif // ODB_DIAGNOSTICS_HXX
