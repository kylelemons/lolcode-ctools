/*! \file This includes helper functions to be linked into the assembly executables */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "asmutil.h"

/*!
 * \brief Variable type
 */
typedef union _val_t {
  char *val_string;
  long val_integer;
  double val_float;
  union _val_t *val_array;
} value_t;

typedef struct {
  long var_type;
  long dim_cnt;
  long *dims;
  value_t *vals;
} variable_t;

/*!
 * \brief Allocate a new variable
 */
void *varalloc(long var_type, long dim_cnt)
{
  variable_t *nvar = (void*)malloc(sizeof(variable_t));
  nvar->var_type = var_type;
  nvar->dim_cnt = dim_cnt;
  nvar->dims = NULL;
  nvar->vals = NULL;

  fprintf(stderr, "varalloc: var@%p\n", nvar);
  return nvar;
}

void *validx(value_t *val, long index)
{
  fprintf(stderr, "validx: val@%p[%ld]\n", val, index);
  fprintf(stderr, "validx: = %p (0x%lx)\n", (val+index), val[index].val_integer);
  return (val+index);
}

void *dimalloc(value_t *val, long *sizes, long count)
{
  long d;
  if (count <= 0)
  {
    fprintf(stderr, "dimalloc: done!\n");
    return val;
  }
  if (sizes == NULL)
  {
    fprintf(stderr, "dimalloc: sizes == NULL!\n");
    exit(1);
  }
  fprintf(stderr, "dimalloc: Allocating dimension (%ld left) for %ld values\n", count, *sizes);
  val = (value_t*)realloc(val, (*sizes)*sizeof(value_t));
  if (val == NULL)
  {
    fprintf(stderr, "dimalloc: Unable to allocate new memory! %ld bytes\n", (*sizes)*sizeof(value_t));
    exit(1);
  }
  for (d = 0; d < *sizes; ++d)
  {
    val[d].val_array = dimalloc(val[d].val_array, sizes+1, count-1);
  }
  return val;
}

void vardimalloc(variable_t *var, long dim_num, long new_length)
{
  if (var == NULL)
  {
    fprintf(stderr, "vardimalloc: NULL variable!\n");
    exit(1);
  }
  if (dim_num >= var->dim_cnt)
  {
    fprintf(stderr, "vardimalloc: dim_num higher than dim_count!\n");
    exit(1);
  }
  if (var->dims == NULL) // first allocation
  {
    // allocate the dimensions for the first time
    fprintf(stderr, "vardimalloc: info: allocating variable\n");
    var->dims = (long*)calloc(var->dim_cnt, sizeof(long));
    if (var->dims == NULL)
    {
      fprintf(stderr, "vardimalloc: could not allocate memory: %ld bytes\n", var->dim_cnt * sizeof(long));
      exit(1);
    }
  }
  var->dims[dim_num] = new_length; 
  var->vals = dimalloc(var->vals, var->dims, var->dim_cnt);
  fprintf(stderr, "varalloc: var.vals@%p\n", var->vals);
}
