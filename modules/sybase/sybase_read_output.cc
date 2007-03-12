/*
  sybase_read_output.cc

  Sybase DB layer for QORE
  uses Sybase OpenClient C library

  Qore Programming language

  Copyright (C) 2007

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <qore/config.h>
#include <qore/support.h>
#include <qore/Exception.h>

#include <assert.h>
#include <qore/minitest.hpp>

#include "sybase_read_output.h"

//------------------------------------------------------------------------------
QoreNode* convert_sybase_output_to_Qore(const sybase_command_wrapper& wrapper, ExceptionSink* xsink)
{
  // TBD
  return 0;
}

#ifdef DEBUG
#  include "tests/sybase_read_output_tests.cc"
#endif

// EOF

