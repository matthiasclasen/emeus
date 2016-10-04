/* emeus-expression-private.c: A set of terms and a constant
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

#include "emeus-expression-private.h"
#include "emeus-variable-private.h"
#include "emeus-simplex-solver-private.h"
#include "emeus-utils-private.h"

#include <glib.h>
#include <math.h>
#include <float.h>

static Term *
term_new (Variable *variable,
          double coefficient)
{
  Term *t = g_slice_new (Term);

  t->variable = variable_ref (variable);
  t->coefficient = coefficient;

  return t;
}

static void
term_free (Term *term)
{
  if (term == NULL)
    return;

  variable_unref (term->variable);

  g_slice_free (Term, term);
}

static int
sort_by_variable_id (gconstpointer a,
                     gconstpointer b)
{
  const Term *ta = a;
  const Term *tb = b;

  if (ta == tb)
    return 0;

  if (ta == NULL)
    return -1;

  if (tb == NULL)
    return 1;

  if (ta->variable == tb->variable)
    return 0;

  return (tb->variable->id_ - ta->variable->id_);
}

static void
expression_add_term (Expression *expression,
                     Term *term)
{
  if (expression->terms == NULL)
    expression->terms = g_hash_table_new_full (NULL, NULL, NULL, (GDestroyNotify) term_free);

  expression->ordered_terms = g_list_insert_sorted (expression->ordered_terms, term, sort_by_variable_id);
  g_hash_table_insert (expression->terms, term->variable, term);
}

static Expression *
expression_new_full (SimplexSolver *solver,
                     Variable *variable,
                     double coefficient,
                     double constant)
{
  Expression *res = g_slice_new (Expression);

  res->solver = solver;
  res->constant = constant;
  res->terms = NULL;
  res->ordered_terms = NULL;
  res->ref_count = 1;

  if (variable != NULL)
    expression_add_term (res, term_new (variable, coefficient));

  return res;
}

Expression *
expression_new (SimplexSolver *solver,
                double constant)
{
  return expression_new_full (solver, NULL, 0.0, constant);
}

Expression *
expression_new_from_variable (Variable *variable)
{
  return expression_new_full (variable->solver, variable, 1.0, 0.0);
}

Expression *
expression_new_from_constant (double constant)
{
  return expression_new_full (NULL, NULL, 0.0, constant);
}

Expression *
expression_clone (Expression *expression)
{
  Expression *clone = expression_new_full (expression->solver,
                                           NULL, 0.0,
                                           expression->constant);
  GHashTableIter iter;
  gpointer value_p;

  if (expression->terms == NULL)
    return clone;

  g_hash_table_iter_init (&iter, expression->terms);
  while (g_hash_table_iter_next (&iter, NULL, &value_p))
    {
      Term *t = value_p;

      expression_add_term (clone, term_new (t->variable, t->coefficient));
    }

  return clone;
}

Expression *
expression_ref (Expression *expression)
{
  if (expression == NULL)
    return NULL;

  expression->ref_count += 1;

  return expression;
}

void
expression_unref (Expression *expression)
{
  if (expression == NULL)
    return;

  expression->ref_count -= 1;

  if (expression->ref_count == 0)
    {
      if (expression->terms != NULL)
        g_hash_table_unref (expression->terms);

      g_slice_free (Expression, expression);
    }
}

void
expression_set_constant (Expression *expression,
                         double constant)
{
  expression->constant = constant;
}

void
expression_add_variable (Expression *expression,
                         Variable *variable,
                         double coefficient,
                         Variable *subject)
{
  if (expression->terms != NULL)
    {
      Term *t = g_hash_table_lookup (expression->terms, variable);

      if (t != NULL)
        {
          double new_coefficient = term_get_coefficient (t) + coefficient;

          if (approx_val (new_coefficient, 0.0))
            expression_remove_variable (expression, variable, subject);
          else
            t->coefficient = new_coefficient;

          return;
        }
    }

  if (!approx_val (coefficient, 0.0))
    {
      expression_add_term (expression, term_new (variable, coefficient));

      if (expression->solver != NULL)
        simplex_solver_note_added_variable (expression->solver, variable, subject);
    }
}

void
expression_remove_variable (Expression *expression,
                            Variable *variable,
                            Variable *subject)
{
  Term *term;

  if (expression->terms == NULL)
    return;

  term = g_hash_table_lookup (expression->terms, variable);
  if (term == NULL)
    return;

  variable_ref (variable);

  if (subject != NULL)
    variable_ref (subject);

  if (expression->solver != NULL)
    simplex_solver_note_removed_variable (expression->solver, variable, subject);

  expression->ordered_terms = g_list_remove (expression->ordered_terms, term);
  g_hash_table_remove (expression->terms, variable);

  if (subject != NULL)
    variable_unref (subject);

  variable_unref (variable);
}

bool
expression_has_variable (Expression *expression,
                         Variable *variable)
{
  if (expression->terms == NULL)
    return false;

  return g_hash_table_lookup (expression->terms, variable) != NULL;
}

void
expression_set_variable (Expression *expression,
                         Variable *variable,
                         double coefficient)
{
  if (expression->terms != NULL)
    {
      Term *t = g_hash_table_lookup (expression->terms, variable);

      if (t != NULL)
        {
          t->coefficient = coefficient;
          return;
        }
    }

  expression_add_term (expression, term_new (variable, coefficient));
}

void
expression_add_expression (Expression *a,
                           Expression *b,
                           double n,
                           Variable *subject)
{
  GHashTableIter iter;
  gpointer value_p;

  a->constant += (n * b->constant);

  if (b->terms == NULL)
    return;

  g_hash_table_iter_init (&iter, b->terms);
  while (g_hash_table_iter_next (&iter, NULL, &value_p))
    {
      Term *t = value_p;

      expression_add_variable (a, t->variable, n * t->coefficient, subject);
    }
}

double
expression_get_coefficient (const Expression *expression,
                            Variable *variable)
{
  Term *t;

  if (expression->terms == NULL)
    return 0.0;

  t = g_hash_table_lookup (expression->terms, variable);
  if (t == NULL)
    return 0.0;

  return term_get_coefficient (t);
}

double
expression_get_value (const Expression *expression)
{
  GHashTableIter iter;
  gpointer value_p;
  double res;

  res = expression->constant;

  if (expression->terms == NULL)
    return res;

  g_hash_table_iter_init (&iter, expression->terms);
  while (g_hash_table_iter_next (&iter, NULL, &value_p))
    {
      const Term *t = value_p;

      res += (t->coefficient * variable_get_value (t->variable));
    }

  return res;
}

void
expression_terms_foreach (Expression *expression,
                          ExpressionForeachTermFunc func,
                          gpointer data)
{
  GList *l;

  if (expression->terms == NULL)
    return;

  for (l = g_list_last (expression->ordered_terms); l != NULL; l = l->prev)
    {
      Term *t = l->data;

      g_assert (t != NULL && t->variable != NULL);

      if (!func (t, data))
        break;
    }
}

Expression *
expression_plus (Expression *expression,
                 double constant)
{
  Expression *e = expression_new (expression->solver, constant);

  expression_add_expression (expression, e, 1.0, NULL);

  expression_unref (e);

  return expression;
}

Expression *
expression_plus_variable (Expression *expression,
                          Variable *variable)
{
  Expression *e = expression_new_from_variable (variable);

  expression_add_expression (expression, e, 1.0, NULL);

  expression_unref (e);

  return expression;
}

Expression *
expression_times (Expression *expression,
                  double multiplier)
{
  GHashTableIter iter;
  gpointer value_p;

  expression->constant *= multiplier;

  if (expression->terms == NULL)
    return expression;

  g_hash_table_iter_init (&iter, expression->terms);
  while (g_hash_table_iter_next (&iter, NULL, &value_p))
    {
      Term *t = value_p;

      t->coefficient *= multiplier;
    }

  return expression;
}

void
expression_change_subject (Expression *expression,
                           Variable *old_subject,
                           Variable *new_subject)
{
  expression_set_variable (expression,
                           old_subject,
                           expression_new_subject (expression, new_subject));
}

double
expression_new_subject (Expression *expression,
                        Variable *subject)
{
  double reciprocal = 1.0;
  Term *term;

  g_assert (!expression_is_constant (expression));

  term = g_hash_table_lookup (expression->terms, subject);
  g_assert (term != NULL);
  g_assert (term->coefficient != 0.0);

  reciprocal = 1.0 / term->coefficient;

  expression->ordered_terms = g_list_remove (expression->ordered_terms, term);
  g_hash_table_remove (expression->terms, subject);

  expression_times (expression, -reciprocal);

  return reciprocal;
}

void
expression_substitute_out (Expression *expression,
                           Variable *out_var,
                           Expression *expr,
                           Variable *subject)
{
  if (expression->terms == NULL)
    return;

  double multiplier = expression_get_coefficient (expression, out_var);

  expression_remove_variable (expression, out_var, NULL);

  expression->constant = expression->constant + multiplier * expr->constant;

  GHashTableIter iter;
  gpointer value_p;
  g_hash_table_iter_init (&iter, expr->terms);
  while (g_hash_table_iter_next (&iter, NULL, &value_p))
    {
      Variable *clv = term_get_variable (value_p);
      double coeff = term_get_coefficient (value_p);

      double old_coefficient = expression_get_coefficient (expression, clv);

      if (old_coefficient > 0.0)
        {
          double new_coefficient = old_coefficient + multiplier * coeff;

          if (approx_val (new_coefficient, 0.0))
            expression_remove_variable (expression, clv, subject);
          else
            expression_set_variable (expression, clv, new_coefficient);
        }
      else
        {
          expression_set_variable (expression, clv, multiplier * coeff);

          if (expression->solver)
            simplex_solver_note_added_variable (expression->solver, clv, subject);
        }
    }
}

Variable *
expression_get_pivotable_variable (Expression *expression)
{
  GHashTableIter iter;
  gpointer key_p;

  if (expression->terms == NULL)
    {
      g_critical ("Expression %p is a constant", expression);
      return NULL;
    }

  g_hash_table_iter_init (&iter, expression->terms);
  while (g_hash_table_iter_next (&iter, &key_p, NULL))
    {
      if (variable_is_pivotable (key_p))
        return key_p;
    }

  return NULL;
}

static int
sort_by_variable_name (gconstpointer a,
                       gconstpointer b)
{
  const Variable *va = a;
  const Variable *vb = b;

  if (va == vb)
    return 0;

  return g_strcmp0 (va->name, vb->name);
}

char *
expression_to_string (const Expression *expression)
{
  GString *buf;
  GList *keys, *l;
  bool needs_plus = false;

  if (expression == NULL)
    return g_strdup ("<null>");

  buf = g_string_new (NULL);

  if (!approx_val (expression->constant, 0.0) || expression->terms == NULL)
    {
      g_string_append_printf (buf, "%g", expression->constant);
      needs_plus = true;
    }

  if (expression->terms == NULL)
    return g_string_free (buf, FALSE);

  keys = g_hash_table_get_keys (expression->terms);
  keys = g_list_sort (keys, sort_by_variable_name);

  for (l = keys; l != NULL; l = l->next)
    {
      Term *t = g_hash_table_lookup (expression->terms, l->data);
      Variable *clv = term_get_variable (t);
      double coeff = term_get_coefficient (t);
      char *str = variable_to_string (clv);

      if (needs_plus)
        g_string_append (buf, " + ");

      if (approx_val (coeff, 1.0))
        g_string_append_printf (buf, "%s", str);
      else
        g_string_append_printf (buf, "%g * %s", coeff, str);

      g_free (str);

      if (!needs_plus)
        needs_plus = true;
    }

  return g_string_free (buf, FALSE);
}
