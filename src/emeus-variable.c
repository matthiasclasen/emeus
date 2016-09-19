/* emeus-variable.c: A symbolic value
 *
 * Copyright 2016  Endless
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "emeus-variable-private.h"

#include <math.h>
#include <float.h>

static void
dummy_variable_init (Variable *v)
{
  v->is_external = FALSE;
  v->is_pivotable = FALSE;
  v->is_restricted = TRUE;
}

static void
objective_variable_init (Variable *v)
{
  v->is_external = FALSE;
  v->is_pivotable = FALSE;
  v->is_restricted = FALSE;
}

static void
slack_variable_init (Variable *v)
{
  v->is_external = FALSE;
  v->is_pivotable = TRUE;
  v->is_restricted = TRUE;
}

static void
regular_variable_init (Variable *v)
{
  v->is_external = TRUE;
  v->is_pivotable = FALSE;
  v->is_restricted = FALSE;
}

Variable *
variable_new (SimplexSolver *solver,
              VariableType   type)
{
  Variable *res = g_slice_new (Variable);

  res->solver = solver;
  res->type = type;
  res->ref_count = 1;

  switch (type)
    {
    case VARIABLE_DUMMY:
      dummy_variable_init (res);
      break;

    case VARIABLE_OBJECTIVE:
      objective_variable_init (res);
      break;

    case VARIABLE_SLACK:
      slack_variable_init (res);
      break;

    case VARIABLE_REGULAR:
      regular_variable_init (res);
      break;
    }

  return res;
}

static void
variable_free (Variable *variable)
{
  g_slice_free (Variable, variable);
}

Variable *
variable_ref (Variable *variable)
{
  variable->ref_count += 1;

  return variable;
}

void
variable_unref (Variable *variable)
{
  variable->ref_count -= 1;
  if (variable->ref_count == 0)
    variable_free (variable);
}

void
variable_set_value (Variable *variable,
                    double value)
{
  variable->value = value;
}
