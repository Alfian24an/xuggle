/*
 * This file is part of Xuggler.
 * 
 * Xuggler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * Xuggler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with Xuggler.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include <cxxtest/TestSuite.h>

// These macros exists for backwards comability reasons; we used
//  to use the Template Unit Testing framework and old code
//  used this form.  This is no deprecated and you should
//  use the TS_* macros.
#define VS_TUT_ENSURE(__msg, __expr) \
  TS_ASSERT(__expr)

#define VS_TUT_ENSURE_EQUALS(msg, __a, __b) \
  TS_ASSERT_EQUALS(__a, __b)

#define VS_TUT_ENSURE_DISTANCE(msg, __a, __b, __delta) \
  TS_ASSERT_DELTA(__a, __b, __delta)

#endif // ! __TEST_UTILS_H__
