/*  FCALGS: Formal Concept Analysis (FCA) Algorithms
 *  Copyright (C) 2007
 *  Jan Outrata, <jan.outrata@upol.cz>
 *  Vilem Vychodil, <vilem.vychodil@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  Iterative Greedy Concepts on Demand from Essential Elements
 *  (IterEss) algorithm
 *  Copyright (C) 2013
 *  Jan Outrata, <jan.outrata@upol.cz>
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define STATS


int lastCol;


/*
void afficherBinaire(unsigned long int nombre, unsigned long int maxnb, int nbCol) {
    //if (nombre > 1) {
    if (maxnb > 1) {
        afficherBinaire(nombre / 2, maxnb/2, nbCol);
    }
    
    printf("%d,", nombre % 2);
}
*/


// get the n-th bit of a unsigned long int
int getBit(unsigned long int nombre, int n) {
    return (nombre >> n) & 1;
}

int min(int a, int b) {
  if(a<b) return a;
  return b;
}

void afficherBinaire(unsigned long int nombre, int nbCol, int first) {
  for(int i=0; i<min(64, nbCol); i++) {
    if(first) {
      first = 0;
    } else {
      printf(",");
    }
    printf("%d", getBit(nombre, 63-i));
  }
}


/* Basic data types */

typedef enum {LESS = -1, EQUAL, GREATER, INCOMPARABLE} order_t;

typedef long int count_t;
typedef char small_count_t;

/* Error handling */

typedef enum {ERROR_NOERROR = 0, ERROR_MEM_ALLOC, ERROR_VALUE, ERROR_FILE_OPEN} error_t;

#define ERROR_MSG_MAXLENGTH 1024

typedef void (error_func_t) (error_t type, char *msg);

char program_name[ERROR_MSG_MAXLENGTH] = "";
error_func_t *error_func = NULL;

void
set_error (char *_program_name, error_func_t *func)
{
	strcpy (program_name, _program_name);
	error_func = func;
}

void
error (error_t type, char *msg, ...)
{
	char _msg[ERROR_MSG_MAXLENGTH];
	va_list args;

	if (error_func) {
		strcpy (_msg, program_name);
		strcat (_msg, ": ");
		strncat (_msg, msg, ERROR_MSG_MAXLENGTH - strlen(program_name) - 3);

		va_start (args, msg);
		msg = _msg;
		vsprintf (_msg, msg, args);
		error_func (type, _msg);
		va_end (args);
	}
}

/* Counters and times */

#ifdef STATS
typedef struct {
  count_t *counters;
  char **counter_texts;
	int ncounters;
  struct timeval *times;
  char **time_texts;
	int ntimes;
	int time_pos;
} stats_t;

#define STATS_INITIALIZER {NULL, NULL, 0, NULL, NULL, 0, 0}

#define COMMA ,
#define STATS_DATA stats_t *stats
#define STATS_DATA_PARAM stats_t *stats
#define STATS_DATA_ARG stats
#define STATS_DATA_ARG_P(p) p stats
#define INC_COUNTER_i(i, c) stats[i].counters[c]++
#define INC_COUNTER(c) INC_COUNTER_i (0, c)
#define ADD_COUNTER_i(i, c, v) stats[i].counters[c] += v
#define ADD_COUNTER(c, v) ADD_COUNTER_i (0, c, v)
#define SET_COUNTER_i(i, c, v) stats[i].counters[c] = v
#define SET_COUNTER(c, v) SET_COUNTER_i (0, c, v)
#define COUNTER_i(i, c) stats[i].counters[c]
#define COUNTER(c) COUNTER_i (0, c)
#define SET_COUNTER_TEXT_i(i, c, t) stats[i].counter_texts[c] = t
#define SET_COUNTER_TEXT(c, t) SET_COUNTER_TEXT_i (0, c, t)
#define COUNTER_TEXT_i(i, c) stats[i].counter_texts[c]
#define COUNTER_TEXT(c) COUNTER_TEXT_i (0, c)
#define COUNTERS_i(i) stats[i].ncounters
#define COUNTERS COUNTERS_i (0)
#define SAVE_TIME_i(i, t)                                     \
  if (stats[i].time_pos < stats[i].ntimes) {                  \
		gettimeofday (&stats[i].times[stats[i].time_pos], NULL);  \
		stats[i].time_texts[stats[i].time_pos++] = t;             \
  }
#define SAVE_TIME(t) SAVE_TIME_i (0, t)
#define TIME_i(i, t) stats[i].times[t]
#define TIME(t) TIME_i (0, t)
#define TIME_TEXT_i(i, t) stats[i].time_texts[t]
#define TIME_TEXT(t) TIME_TEXT_i (0, t)
#define TIME_POS_i(i) stats[i].time_pos
#define TIME_POS TIME_POS_i (0)
#define INIT_STATS(n, c, t) init_stats (&stats, n, c, t)
#define FREE_STATS free_stats (&stats)

#else
#define COMMA
#define STATS_DATA
#define STATS_DATA_PARAM
#define STATS_DATA_ARG
#define STATS_DATA_ARG_P(p)
#define INC_COUNTER_i(i, c)
#define INC_COUNTER(c)
#define ADD_COUNTER_i(i, c, v)
#define ADD_COUNTER(c, v)
#define SET_COUNTER_i(i, c, v)
#define SET_COUNTER(c, v)
#define SET_COUNTER_TEXT_i(i, c, t)
#define SET_COUNTER_TEXT(c, t)
#define SAVE_TIME_i(i, t)
#define SAVE_TIME(t)
#define INIT_STATS(n, c, t)
#define FREE_STATS
#endif

#ifdef STATS
  void
    init_stats (stats_t **stats_p, int count, int counters, int times)
  {
    STATS_DATA;

    stats = *stats_p = (stats_t *) malloc (sizeof (stats_t) * count);
    stats[0].counters = (count_t *) malloc (sizeof (count_t) * count * counters);
    memset (stats[0].counters, 0, sizeof (count_t) * count * counters);
    stats[0].counter_texts = (char **) malloc (sizeof (char *) * count * counters);
    memset (stats[0].counter_texts, 0, sizeof (char *) * count * counters);
    stats[0].times = (struct timeval *) malloc (sizeof (struct timeval) * count * times);
    stats[0].time_texts = (char **) malloc (sizeof (char *) * count * times);
    for (count--; count > 0; count--) {
      stats[count].counters = stats[0].counters + count * counters;
      stats[count].counter_texts = stats[0].counter_texts;
      stats[count].times = stats[0].times + count * times;
      stats[count].time_texts = stats[0].time_texts;
      stats[count].time_pos = 0;
      stats[count].ncounters = counters;
      stats[count].ntimes = times;
    }
    stats[0].time_pos = 0;
    stats[0].ncounters = counters;
    stats[0].ntimes = times;
  }

void
free_stats (stats_t **stats_p)
{
	STATS_DATA = *stats_p;

	free (stats[0].counters);
	free (stats[0].counter_texts);
	free (stats[0].times);
	free (stats[0].time_texts);
	free (stats);
	*stats_p = NULL;
}
#endif

/* Input/Output interface */

#define IO_BUFFER_BLOCK	1048576

typedef enum {IO_MARK_CONCEPT_HELP = -7,
							IO_MARK_CONCEPT_UPPER,
							IO_MARK_CONCEPT_LOWER,
							IO_MARK_CONCEPT_MODIFIED,
							IO_SEP_OA,
							IO_SEP_CLASS,
							IO_END_SET} io_char_t;

typedef enum {INPUT_LABELS = 1, INPUT_LABELS_ONLY = 1 << 1, OUTPUT_ALL = 1 << 8} io_flags_t;

typedef struct {
  count_t *write_ptr;
	count_t *read_ptr;
	int write_left;
	int read_left;
  int size;
  count_t *base;
} io_buffer_t;

struct io_struc;

typedef void (io_func_t) (struct io_struc *io, void *data, io_flags_t flags);

typedef struct io_struc {
	io_buffer_t buffer;
	io_func_t *func;
	void *func_data;
} io_t;

#define IO_INITIALIZER {{NULL, NULL, 0, 0, 0, NULL}, NULL, NULL}

#define IO_WRITE(_io, _value)                                       \
	{                                                                 \
		if (!(_io)->buffer.write_left)                                  \
			alloc_io_buffer (_io, (_io)->buffer.size + IO_BUFFER_BLOCK);  \
		*(_io)->buffer.write_ptr = _value;                              \
		(_io)->buffer.write_ptr++;                                      \
		(_io)->buffer.write_left--;                                     \
		(_io)->buffer.read_left++;                                      \
	}

#define IO_READ(_io, _var)                      \
	{                                             \
    _var = *(_io)->buffer.read_ptr;             \
    (_io)->buffer.read_ptr++;                   \
    (_io)->buffer.read_left--;                  \
  }

#define IO_FUNC(_io, _flags)                      \
	{                                               \
    if ((_io)->func)                              \
      (_io)->func (_io, (_io)->func_data, _flags);  \
  }

void
reset_io (io_t *io)
{
  io->buffer.write_ptr = io->buffer.read_ptr = io->buffer.base;
  io->buffer.write_left = io->buffer.size;
  io->buffer.read_left = 0;
}

void
init_io (io_t *io, io_func_t *func, void *func_data)
{
  io->buffer.base = NULL;
  io->buffer.size = 0;
  reset_io (io);
  io->func = func;
  io->func_data = func_data;
}

void
free_io (io_t *io)
{
  if (io->buffer.base) free (io->buffer.base);
  init_io (io, NULL, NULL);
}

io_buffer_t *
alloc_io_buffer (io_t *io, int size)
{
  count_t *old_base;

  if (io->buffer.base) {
    old_base = io->buffer.base;
    io->buffer.base = (count_t *) realloc (io->buffer.base, sizeof (count_t) * size);
    if (io->buffer.base && (io->buffer.base != old_base)) {
      io->buffer.write_ptr += io->buffer.base - old_base;
      io->buffer.read_ptr += io->buffer.base - old_base;
    }
  }
  else
    io->buffer.base = io->buffer.write_ptr = io->buffer.read_ptr = (count_t *) malloc (sizeof (count_t) * size);
  if (!io->buffer.base) {
    error (ERROR_MEM_ALLOC, "cannot allocate I/O buffer");
    return NULL;
  }
  io->buffer.write_left += size - io->buffer.size;
  io->buffer.size = size;

  return &io->buffer;
}

/* File input */

int
get_next_number_from_file (FILE *file, count_t *value)
{
	int ch = ' ';

	*value = IO_END_SET;

	while ((ch != EOF) && ((ch < '0') || (ch > '9'))) {
		ch = fgetc (file);
		if (ch == '\n')
	    return 1;
		if (ch == '|') {
			*value = IO_SEP_CLASS;
			return 1;
		}
	}

	if (ch == EOF)
		return 0;

	*value = 0;
	while ((ch >= '0') && (ch <= '9')) {
		*value *= 10;
		*value += ch - '0';
		ch = fgetc (file);
	}

	ungetc (ch, file);

	return 1;
}

void
read_file_to_io (io_t *io, FILE *file, io_flags_t flags)
{
  count_t value = 0, last_attribute = -1, last_object = -1;
  small_count_t last_class = -1, classes = 0;
  int labels;

	IO_WRITE (io, last_object + 1);
	if (!(flags & INPUT_LABELS_ONLY))
		IO_WRITE (io, last_attribute + 1);
	if (flags & (INPUT_LABELS | INPUT_LABELS_ONLY))
		IO_WRITE (io, classes);

	labels = flags & INPUT_LABELS_ONLY;

  while (get_next_number_from_file (file, &value)) {
		if (value == IO_END_SET) {
			last_object++;
			IO_WRITE (io, IO_END_SET);
			if (last_class >= classes) classes = last_class + 1;
			last_class = -1;
			labels = flags & INPUT_LABELS_ONLY;
		}
		else if (value == IO_SEP_CLASS) {
			labels = 1;
			if ((flags & INPUT_LABELS) && (!(flags & INPUT_LABELS_ONLY)))
				IO_WRITE (io, IO_SEP_CLASS);
		} else
			if (labels) {
				last_class++;
				if (flags & (INPUT_LABELS | INPUT_LABELS_ONLY))
					IO_WRITE (io, value);
			} else {
				if (value > last_attribute)
					last_attribute = value;
				IO_WRITE (io, value);
			}
  }
  if (value >= 0) {
		last_object++;
		IO_WRITE (io, IO_END_SET);
		if (last_class >= classes) classes = last_class + 1;
  }

	*io->buffer.read_ptr = last_object + 1;
	if (!(flags & INPUT_LABELS_ONLY)) {
		*(io->buffer.read_ptr + 1) = last_attribute + 1;
		if (flags & INPUT_LABELS)
			*(io->buffer.read_ptr + 2) = classes;
	} else
		*(io->buffer.read_ptr + 1) = classes;
}

/* File output */

#define OUTPUT_BLOCK 1024

void
print_error (error_t type, char *msg)
{
	fprintf (stderr, "%s\n", msg);
}

void
write_file_from_io (io_t *io, FILE *file, io_flags_t flags)
{
  int first = 1;
  count_t value;

	if ((io->buffer.read_left < OUTPUT_BLOCK) && (!(flags & OUTPUT_ALL)))
		return;

	while (io->buffer.read_left) {
		IO_READ (io, value);
		if (value == IO_END_SET) {
			fprintf (file, "\n");
			first = 1;
		}	else {
			if (!first)
				fprintf (file, " ");
			switch (value) {
			case IO_SEP_CLASS:
			case IO_SEP_OA:
				fprintf (file, "|");
				break;
			case IO_MARK_CONCEPT_MODIFIED:
				fprintf (file, "*");
				break;
			case IO_MARK_CONCEPT_LOWER:
				fprintf (file, "<");
				break;
			case IO_MARK_CONCEPT_UPPER:
				fprintf (file, ">");
				break;
			case IO_MARK_CONCEPT_HELP:
				fprintf (file, "-");
				break;
			default:
				fprintf (file, "%li", value);
			}
			first = 0;
		}
	}
	reset_io (io);
}

/* Formal context */

typedef unsigned long int data_t;

#define BIT		((data_t) 1)
#define ZERO	((data_t) 0)
#define FULL ((data_t) -1)

#define ARCHBIT		((sizeof (data_t) * 8) - 1)

#define DT_SIZE_A (sizeof (data_t) * context->dt_count_a)
#define DT_SIZE_O (sizeof (data_t *) * (context->objects + 1))
#define DT_SIZE_Ox(x) (sizeof (data_t *) * (x + 1))

typedef enum {CONTEXT_SORT_COLS = 1, CONTEXT_SORT_ROWS = 1 << 1} context_sort_t;

struct context_struc;

typedef struct context_struc {
  count_t attributes;
  count_t objects;
  count_t dt_count_a;
  count_t table_entries;
  data_t *bitarray;
  data_t **rows;
  count_t *supps;
  count_t *attrib_numbers;
  count_t *object_numbers;
} context_t;

#define CONTEXT_INITIALIZER {0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

context_t *qsort_context = NULL;

void
create_context (context_t *context, count_t objects, count_t attributes)
{
  count_t i;

  context->attributes = attributes;
  context->objects = objects;
  context->dt_count_a = (attributes / (ARCHBIT + 1)) + 1;

  context->bitarray = (data_t *) malloc (sizeof (data_t) * context->dt_count_a * objects);
  if (! context->bitarray)
		error (ERROR_MEM_ALLOC, "cannot allocate context");
  memset (context->bitarray, 0, sizeof (data_t) * context->dt_count_a * objects);
  context->rows = (data_t **) malloc (sizeof (data_t *) * objects);
  for (i = 0; i < objects; i++)
    context->rows [i] = context->bitarray + i * context->dt_count_a;

  context->supps = (count_t *) malloc (sizeof (count_t) * attributes);
  memset (context->supps, 0, sizeof (count_t) * attributes);

  context->table_entries = 0;

  context->attrib_numbers = (count_t *) malloc (sizeof(count_t) * attributes);
  for (i = 0; i < attributes; i ++)
    context->attrib_numbers [i] = i;
  context->object_numbers = (count_t *) malloc (sizeof(count_t) * objects);
  for (i = 0; i < objects; i ++)
    context->object_numbers [i] = i;
}

void
destroy_context (context_t *context)
{
  if (context->bitarray) free (context->bitarray);
  if (context->rows) free (context->rows);
  if (context->supps) free (context->supps);
  if (context->attrib_numbers) free (context->attrib_numbers);
  if (context->object_numbers) free (context->object_numbers);
}

void
copy_context (context_t *to_context, context_t *from_context)
{
  create_context(to_context, from_context->objects, from_context->attributes);

  memcpy (to_context->bitarray, from_context->bitarray, sizeof (data_t) * to_context->dt_count_a * to_context->objects);
  memcpy (to_context->supps, from_context->supps, sizeof (count_t) * to_context->attributes);
  to_context->table_entries = from_context->table_entries;
  memcpy (to_context->attrib_numbers, from_context->attrib_numbers, sizeof(count_t) * to_context->attributes);
  memcpy (to_context->object_numbers, from_context->object_numbers, sizeof(count_t) * to_context->objects);
}

void
input_context_from_io (context_t *context, io_t *io, count_t attr_offset)
{
  count_t object, value;

	IO_FUNC (io, 0);

	IO_READ (io, object);
	IO_READ (io, value);
	create_context (context, object, value);

	object = 0;
	while (io->buffer.read_left) {
		IO_READ (io, value);
		if (value == IO_END_SET) {
			object++;
			continue;
		}
		value -= attr_offset;
		if (value < 0) {
			error (ERROR_VALUE, "invalid input value %li (minimum value is %li)", value + attr_offset, attr_offset);
		  continue;
		}
    if(lastCol < value) {
      lastCol = value; 
    }
    
		context->rows [object] [value / (ARCHBIT + 1)] |= (BIT << (ARCHBIT - (value % (ARCHBIT + 1))));
		context->supps [value] ++;
		context->table_entries++;
	}
}

void
output_context_to_io (context_t *context, io_t *io)
{
  count_t i, j, total;
  int k;

  for (i = 0; i < context->objects; i++) {
    for (total = j = 0; j < context->dt_count_a; j++)
      for (k = ARCHBIT; k >= 0; k--, total++) {
				if (total >= context->attributes)
					goto skipout;
				if (context->rows[i][j] & (BIT << k))
					IO_WRITE (io, context->attrib_numbers[total]);
      }
  skipout:
    IO_WRITE (io, IO_END_SET);
  }

	IO_FUNC (io, OUTPUT_ALL);
}

int
context_attrib_numbers_compar (const void *a, const void *b)
{
  count_t x, y;

  x = qsort_context->supps [*(count_t const *)a];
  y = qsort_context->supps [*(count_t const *)b];
  return (x < y) ? LESS : ((x > y) ? GREATER : EQUAL);
}

int
context_object_numbers_compar (const void *a, const void *b)
{
  count_t i;

  for (i = 0; i < qsort_context->dt_count_a; i ++)
    if (qsort_context->rows [*(count_t const *)a] [i] < qsort_context->rows [*(count_t const *)b] [i])
      return LESS;
    else if (qsort_context->rows [*(count_t const *)a] [i] > qsort_context->rows [*(count_t const *)b] [i])
      return GREATER;
  return EQUAL;
}

int
context_objects_compar (const void *a, const void *b)
{
  count_t i;

  for (i = 0; i < qsort_context->dt_count_a; i ++)
    if (((data_t const *)a)[i] < ((data_t const *)b)[i])
      return LESS;
    else if (((data_t const *)a)[i] > ((data_t const *)b)[i])
      return GREATER;
  return EQUAL;
}

void
reorder_attribs_in_context (context_t *context, count_t *attrib_numbers)
{
  data_t *bitarray;
  count_t *supps;
  count_t i, o, total;
  int j;

  bitarray = (data_t *) malloc (sizeof (data_t) * context->dt_count_a * context->objects);
  memcpy (bitarray, context->bitarray, sizeof (data_t) * context->dt_count_a * context->objects);
  memset (context->bitarray, 0, sizeof (data_t) * context->dt_count_a * context->objects);
  supps = (count_t *) malloc (sizeof (count_t) * context->attributes);
  memcpy (supps, context->supps, sizeof (count_t) * context->attributes);

  for (total = i = 0; i < context->dt_count_a; i++)
    for (j = ARCHBIT; j >= 0; j--, total++) {
      if (total >= context->attributes)
        goto skipout;

      for (o = 0; o < context->objects; o ++)
        if (bitarray [o * context->dt_count_a + attrib_numbers [total] / (ARCHBIT + 1)] & (BIT << (ARCHBIT - (attrib_numbers [total] % (ARCHBIT + 1)))))
          context->rows [o] [i] |= (BIT << j);
      context->supps [total] = supps [attrib_numbers [total]];
    }
skipout:
  if (context->attrib_numbers != attrib_numbers)
    memcpy (context->attrib_numbers, attrib_numbers, sizeof(count_t) * context->attributes);

  free (bitarray);
  free (supps);
}

void
sort_context (context_t *context, int sort_by)
{
  if (sort_by & CONTEXT_SORT_COLS) {
    qsort_context = context;
    qsort (context->attrib_numbers, context->attributes, sizeof(count_t), context_attrib_numbers_compar);
    reorder_attribs_in_context (context, context->attrib_numbers);
  }

  if (sort_by & CONTEXT_SORT_ROWS) {
    qsort_context = context;
    qsort (context->object_numbers, context->objects, sizeof(count_t), context_object_numbers_compar);
    qsort (context->bitarray, context->objects, sizeof (data_t) * context->dt_count_a, context_objects_compar);
  }
}

void remove_low_support_attribs_from_context(context_t *context, count_t min_support)
{
  context_t new_context;
  count_t attribs, i, ii, o, total, ttotal;
  int j, jj;

  for (attribs = i = 0; i < context->attributes; i ++)
    if (context->supps [i] >= min_support)
      attribs ++;

  create_context (&new_context, context->objects, attribs);

  for (total = ttotal = i = ii = 0, jj = ARCHBIT; i < context->dt_count_a; i++)
    for (j = ARCHBIT; j >= 0; j--, total++) {
      if (total >= context->attributes)
        goto skipout;

      if (context->supps [total] < min_support)
        continue;

      for (o = 0; o < context->objects; o ++)
        if (context->rows [o] [i] & (BIT << j))
          new_context.rows [o] [ii] |= (BIT << jj);
      new_context.supps [ttotal] = context->supps [total];
      new_context.table_entries += context->supps [total];
      new_context.attrib_numbers [ttotal] = context->attrib_numbers [total];

      ttotal ++;
      if (--jj < 0) { ii ++; jj = ARCHBIT; }
    }
skipout:
  memcpy (new_context.object_numbers, context->object_numbers, sizeof(count_t) * context->objects);

  destroy_context(context);
  *context = new_context;
}

void
transpose_context(context_t *context)
{
  context_t new_context;
  count_t o, i, total;
  int j;

  create_context(&new_context, context->attributes, context->objects);

  for (total = i = 0; i < context->dt_count_a; i++)
    for (j = ARCHBIT; j >= 0; j--, total++) {
      if (total >= context->attributes)
        goto skipout;

      for (o = 0; o < context->objects; o++)
        if (context->rows [o] [i] & (BIT << j)) {
          new_context.rows [total] [o / (ARCHBIT + 1)] |=
            (BIT << (ARCHBIT - (o % (ARCHBIT + 1))));
          new_context.supps [o] ++;
        }
    }
skipout:
  memcpy (new_context.object_numbers, context->attrib_numbers, sizeof(count_t) * context->attributes);
  memcpy (new_context.attrib_numbers, context->object_numbers, sizeof(count_t) * context->objects);

  destroy_context(context);
  *context = new_context;
}

void
compute_ess_context(context_t *ess_context, context_t *context)
{
//printf("compute_ess_context...\n");
  context_t transp_context, transp_ess_context, *tmp_context,
    *tmp_ess_context;
  count_t o, i, p, total;
  int j, n;
  int geq, leq;

  copy_context(ess_context, context);
  copy_context(&transp_context, context);
  transpose_context (&transp_context);
  copy_context(&transp_ess_context, &transp_context);

  tmp_context = context;
  tmp_ess_context = ess_context;
  for (n = 0; n < 2; n ++) {
    for (o = 0; o < tmp_context->objects; o ++)
      for (p = o + 1; p < tmp_context->objects; p ++) {
        geq = leq = 1;
        for (i = 0; (geq || leq) && (i < tmp_context->dt_count_a); i ++) {
          if (geq && (~tmp_context->rows [o] [i] & tmp_context->rows [p] [i]))
            geq = 0;
          if (leq && (tmp_context->rows [o] [i] & ~tmp_context->rows [p] [i]))
            leq = 0;
        }
        if (geq && !leq) {
//printf("geq && !leq\n");
          for (i = 0; i < tmp_context->dt_count_a; i ++)
            tmp_ess_context->rows [o] [i] &= ~tmp_context->rows [p] [i];
        }
        if (leq && !geq) {
        
//printf("leq && !geq\n");
          for (i = 0; i < tmp_context->dt_count_a; i ++)
            tmp_ess_context->rows [p] [i] &= ~tmp_context->rows [o] [i];
        }
      }
    tmp_context = &transp_context;
    tmp_ess_context = &transp_ess_context;
  }

  destroy_context (&transp_context);
  transpose_context (&transp_ess_context);

  memset (ess_context->supps, 0, sizeof (count_t) * context->attributes);
  ess_context->table_entries = 0;
  for (o = 0; o < context->objects; o++) {
    for (total = i = 0; i < context->dt_count_a; i++) {
      ess_context->rows [o] [i] &= transp_ess_context.rows [o] [i];
      for (j = ARCHBIT; j >= 0; j--, total++) {
        if (total >= context->attributes)
          goto skipout;

        if (ess_context->rows [o] [i] & (BIT << j)) {
          ess_context->supps[total]++;
          ess_context->table_entries++;
        }
      }
    }
  skipout:
    ;
  }

  destroy_context (&transp_ess_context);
}

void
_transform_objects_to_context(objects, from_objects, context, from_context)
data_t **objects, **from_objects;
context_t *context, *from_context;
{
  count_t o;

  *objects = NULL;
  for (o = (count_t)*from_objects, from_objects ++; o; from_objects ++, o --)
    if (((*from_objects - *from_context->rows) / from_context->dt_count_a) < context->objects) {
      *objects = (data_t *)((count_t)*objects + 1);
      objects[(count_t)*objects] = context->rows [(*from_objects - *from_context->rows) / from_context->dt_count_a];
    }
}

count_t
compute_table_entries_percent (count_t table_entries, int percent)
{
	return ((percent / 10) * table_entries / 10 + (percent % 10) * table_entries / 100);
}

/* Formal concept and concepts */

#ifdef STATS
enum {CONCEPTS = 0, CLOSURES, FAIL_SUPPORT, NEXT_CONCEPTS_COUNTER};
#endif

typedef enum {CONCEPT_NEW = 1, CONCEPT_MODIFIED = 1 << 1, CONCEPT_LOWER = 1 << 2,
							CONCEPT_UPPER = 1 << 3, CONCEPT_HELP = 1 << 4,
							CONCEPT_SAVE_ATTRIBS_ONLY = 1 << 8, CONCEPT_SAVE_OBJECTS_ONLY = 1 << 9}
	concept_flags_t;

typedef struct {
  data_t *start;
  data_t *iter;
  data_t *end;
} concept_list_t;

typedef void (concept_save_func_t) (data_t *intent, data_t **extent,
																		context_t *context, void *data,
																		concept_flags_t flags);

typedef void (concept_info_func_t) (data_t *intent, data_t **extent,
																		context_t *context, concept_flags_t flags
																		COMMA STATS_DATA_PARAM);

typedef struct {
	count_t min_support;
	concept_save_func_t *save_func;
	void *save_data;
	concept_info_func_t *info_func;
	concept_flags_t flags;
} concepts_t;

#define CONCEPTS_INITIALIZER {0, NULL, NULL, NULL, 0}

#define SAVE_CONCEPT(_intent, _extent, _context, _concepts, _flags)			\
	{																																			\
		if ((_concepts)->save_func || (_concepts)->info_func) {							\
			if ((_concepts)->save_func)																				\
				(_concepts)->save_func (_intent, _extent, _context, (_concepts)->save_data, (_concepts)->flags | _flags); \
			if ((_concepts)->info_func)																				\
				(_concepts)->info_func (_intent, _extent, _context, (_concepts)->flags | _flags COMMA STATS_DATA_ARG); \
		}																																		\
	}

void compute_closure_from_attrs (data_t *intent, data_t **extent, data_t *attrs,
																 context_t *context);

void compute_closure_from_objects (data_t *intent, data_t **extent, data_t **objects,
																	 context_t *context);

data_t upto_bit [ARCHBIT + 1];

void
init_concepts (concepts_t *concepts, count_t min_support,
							 concept_save_func_t *save_func, void *save_data,
							 concept_info_func_t *info_func, concept_flags_t flags)
{
	concepts->min_support = min_support;
	concepts->save_func = save_func;
	concepts->save_data = save_data;
	concepts->info_func = info_func;
	concepts->flags = flags;
}

void
free_concepts (concepts_t *concepts)
{
}

void
compute_closure_plus_attr (data_t *intent, data_t **extent, data_t **prev_extent, data_t *prev_intent, context_t *context, count_t attr_dt, data_t attr_mask, count_t min_support)
{
  count_t i, o;
  data_t data = 0;

  memset (intent, 0xFF, DT_SIZE_A);

  if (prev_extent) {
    for (o = i = (count_t)*prev_extent, prev_extent ++; o; prev_extent ++, o --) {
      if ((*prev_extent) [attr_dt] & attr_mask) {
        extent[(count_t)++data] = *prev_extent;
      }
      else if (--i < min_support) {
        *extent = (data_t *)data;
        return;
      }
    }
    *extent = (data_t *)data;
    for (i = 0; i < context->dt_count_a; i ++, intent ++) {
      data = *intent;
      if (data && (prev_intent [i] ^ FULL))
        for (o = (count_t)*extent; data && o; o--)
          data &= extent[o] [i];
      *intent = data;
    }
  }
  else {
    *extent = (data_t *)context->objects;
    for (o = 0; o < context->objects; o ++) {
      extent[o + 1] = context->rows[o];
      for (i = 0; i < context->dt_count_a; i ++)
        intent [i] &= context->rows [o] [i];
    }
  }
}

void
compute_extent_from_attrs (extent, attrs, context)
data_t **extent;
data_t *attrs;
context_t *context;
{
  count_t o, i;
  data_t data = 0;

  for (o = 0; o < context->objects; o ++) {
    for (i = 0; i < context->dt_count_a; i ++)
      if (attrs [i] & ~(context->rows [o] [i]))
        goto skip;
    extent[(count_t)++data] = context->rows [o];
  skip:
    ;
  }
  *extent = (data_t *)data;
}

void
compute_intent_from_objects (intent, objects, context)
data_t *intent;
data_t **objects;
context_t *context;
{
  count_t o, i;
  data_t data;

  memset (intent, 0xFF, DT_SIZE_A);
  for (i = 0; i < context->dt_count_a; i ++, intent ++) {
    data = *intent;
    if (data)
      for (o = (count_t)*objects; data && o; o--)
        data &= objects [o] [i];
    *intent = data;
  }
}

__inline void
compute_closure_from_attrs (intent, extent, attrs, context)
data_t *intent, *attrs;
data_t **extent;
context_t *context;
{
  compute_extent_from_attrs (extent, attrs, context);
  compute_intent_from_objects (intent, extent, context);
}

__inline void
compute_closure_from_objects (intent, extent, objects, context)
data_t *intent;
data_t **extent, **objects;
context_t *context;
{
  compute_intent_from_objects (intent, objects, context);
  compute_extent_from_attrs (extent, intent, context);
}

void
compute_attribute_concepts(data_t *intents, context_t *context, count_t min_support COMMA STATS_DATA_PARAM)
{
  data_t *intent_largest, *intent;
  data_t **extent_largest, **extent;
  count_t j, total;
  int i;

  intent_largest = (data_t *) malloc (DT_SIZE_A + DT_SIZE_O);
  extent_largest = (data_t **)(intent_largest + context->dt_count_a);

  compute_closure_plus_attr (intent_largest, extent_largest, NULL, NULL, context, 0, 0, min_support);
  INC_COUNTER(CLOSURES);

  intent = intents;
  extent = (data_t **)(intent + context->dt_count_a);

  for (total = j = 0; j < context->dt_count_a; j ++) {
    for (i = ARCHBIT; i >= 0; i --) {
      if (total >= context->attributes)
        return;

      compute_closure_plus_attr (intent, extent, extent_largest, intent_largest, context,
                                 j, BIT << i, min_support);
      INC_COUNTER(CLOSURES);

      intent = (data_t *)(extent + context->objects + 1);
      extent = (data_t **)(intent + context->dt_count_a);
      total ++;
    }
  }

  free (intent_largest);
}

void
output_attributes_to_io (data_t *set, context_t *context, io_t *io)
{
	count_t j, c;
	data_t i;

	for (c = j = 0; j < context->dt_count_a; j ++) {
		for (i = BIT << ARCHBIT; i; i >>= 1) {
	    if (set [j] & i) {
				IO_WRITE (io, context->attrib_numbers[c]);
	    }
	    c ++;
	    if (c >= context->attributes)
	      goto out;
		}
	}
out: ;
}

void
output_objects_to_io (data_t **set, context_t *context, io_t *io)
{
  count_t o;

  for (o = (count_t)*set, set++; o; set ++, o --)
		IO_WRITE (io, context->object_numbers[(count_t)(*set - *context->rows) / context->dt_count_a]);
}

void
save_concept_to_list (data_t *intent, data_t **extent, context_t *context,
											concept_list_t *list, concept_flags_t flags)
{
  count_t length;

  if ((list->iter + context->dt_count_a + (count_t)*extent + 1) > list->end) {
		length = list->iter - list->start;
		list->start = (data_t *) realloc (list->start, sizeof (data_t) * (length << 1));
		list->iter = list->start + length;
		list->end = list->start + (length << 1);
  }

	if (!(flags & CONCEPT_SAVE_OBJECTS_ONLY)) {
		memcpy (list->iter, intent, DT_SIZE_A);
		list->iter += context->dt_count_a;
	}
	if (!(flags & CONCEPT_SAVE_ATTRIBS_ONLY)) {
		memcpy (list->iter, extent, DT_SIZE_Ox((count_t)*extent));
		list->iter += (count_t)*extent + 1;
	}
}

/* Boolean factors as formal concepts */

#ifdef STATS
enum {FACTORS = 0, UNCOVERED = 2, OVERCOVERED, NEXT_FACTORS_COUNTER};
#endif

typedef struct {
	int threshold;
	concepts_t concepts;
} factors_t;

void
init_factors (factors_t *factors, int threshold)
{
	factors->threshold = threshold;
}

void
compute_closure_masked_size (data_t *intent, data_t **extent, context_t *context,
														 context_t *mask_context, count_t *size, count_t *over_size)
{
  count_t o, i, count, over_count;
  int j;
  data_t cell;

  *size = 0;
	if (over_size) *over_size = 0;
  for (o = (count_t)*extent, extent++; o; extent ++, o --) {
		for (count = over_count = i = 0; i < context->dt_count_a; i ++) {
			cell = intent [i] & mask_context->rows [(*extent - *context->rows) / context->dt_count_a] [i];
			for (j = 0; (j <= ARCHBIT) && cell; j ++) {
				if (cell & BIT) {
					if (context->rows [(*extent - *context->rows) / context->dt_count_a] [i] & (BIT << j))
						count ++;
					else
						over_count ++;
				}
				cell >>= BIT;
			}
		}
		*size += count;
		if (over_size) *over_size += over_count;
  }
}

void
output_factor_to_io (data_t *intent, data_t **extent, context_t *context,
										 io_t *io, concept_flags_t flags)
{
	if (!(flags & CONCEPT_SAVE_OBJECTS_ONLY))
		output_attributes_to_io (intent, context, io);
  if (!(flags & CONCEPT_SAVE_ATTRIBS_ONLY)) {
		if (!(flags & CONCEPT_SAVE_OBJECTS_ONLY))
			IO_WRITE (io, IO_SEP_OA);
		output_objects_to_io (extent, context, io);
  }
	IO_WRITE (io, IO_END_SET);

	IO_FUNC (io, 0);
}

/* IterEss algorithm */

#ifdef STATS
enum {ESS_CONTEXTS = NEXT_FACTORS_COUNTER};
#endif

int ess_iters = 1;

void
find_factors (context_t *context, factors_t *factors COMMA STATS_DATA_PARAM)
{
  ess_iters = 0;
  
  context_t new_context, **contexts;
  concept_list_t _factor_concepts[2], *factor_concepts = _factor_concepts, *new_factor_concepts = _factor_concepts + 1, *tmp_factor_concepts;
  int contexts_length = 8;
  struct {
    data_t *start;
    data_t *iter;
  } attribute_intents;
  data_t *intent, *new_intent, *best_intent, *tmp_intent,
    *intent_parent_intent, *intent_new_parent_intent, *intent_best_parent_intent,
    *extent_parent_intent, *extent_new_parent_intent, *extent_best_parent_intent,
    *factor_intent, *intent_factor_parent_intent, *extent_factor_parent_intent;
  data_t **extent, **new_extent, **best_extent, **tmp_extent,
    **intent_parent_extent, **intent_new_parent_extent, **intent_best_parent_extent,
    **extent_parent_extent, **extent_new_parent_extent, **extent_best_parent_extent,
    **factor_extent, **intent_factor_parent_extent, **extent_factor_parent_extent, **factor_concepts_extent, **objects;
  count_t size, new_size, best_size, factor_size;
  count_t j, o, total, c, last_c;
  int i;
  data_t *factor_concept = NULL;
  count_t table_entries_percent = 0;

	SET_COUNTER_TEXT (ESS_CONTEXTS, "Ess contexts");

  table_entries_percent = compute_table_entries_percent (context->table_entries, 100 - factors->threshold);

  contexts = (context_t **) malloc (sizeof (context_t *) * contexts_length);
  *contexts = context;
  c = 0;
  int nbInit = contexts[c]->table_entries;
  //printf("contexts[%d]->table_entries = %d\n", c, contexts[c]->table_entries);
  do {
    c++;
    if (c == contexts_length) {
      contexts_length <<= 1;
      contexts = (context_t **) realloc (contexts, sizeof (context_t *) * contexts_length);
    }
    contexts[c] = (context_t *) malloc (sizeof (context_t));
    compute_ess_context(contexts[c], contexts[c - 1]);
    //printf("contexts[%d]->table_entries = %d\n", c, contexts[c]->table_entries);
  } while (((ess_iters && (c < ess_iters)) || !ess_iters) && (contexts[c]->table_entries != contexts[c - 1]->table_entries));



  //exit(0);
  if ((contexts[c]->table_entries == contexts[c - 1]->table_entries) && (c > 1))
    c--;
  last_c = c;
  //exit(0);
  SAVE_TIME("Ess contexts time");
  SET_COUNTER(ESS_CONTEXTS, last_c);


  fprintf( stderr, "Iteress: Simplified %d ones to %d ones !\n", nbInit, contexts[c]->table_entries);

  for (int o = 0; o < context->objects; o++) {
    int first = 1;
    for (int i = 0; i < context->dt_count_a; i++) {
      afficherBinaire(contexts[c]->rows[o][i], lastCol+1 - 64*i, first);
      first = 0;
    }
    printf("\n");
  }
	
  exit(0);

  attribute_intents.start = attribute_intents.iter = (data_t *) malloc ((DT_SIZE_A + DT_SIZE_O) * (context->attributes + 12) + DT_SIZE_O);
  intent = (data_t *)(attribute_intents.start + (context->dt_count_a + context->objects + 1) * context->attributes);
  extent = (data_t **)(intent + context->dt_count_a);
  new_intent = (data_t *)(extent + context->objects + 1);
  new_extent = (data_t **)(new_intent + context->dt_count_a);
  best_intent = (data_t *)(new_extent + context->objects + 1);
  best_extent = (data_t **)(best_intent + context->dt_count_a);
  intent_parent_intent = (data_t *)(best_extent + context->objects + 1);
  intent_parent_extent = (data_t **)(intent_parent_intent + context->dt_count_a);
  intent_new_parent_intent = (data_t *)(intent_parent_extent + context->objects + 1);
  intent_new_parent_extent = (data_t **)(intent_new_parent_intent + context->dt_count_a);
  intent_best_parent_intent = (data_t *)(intent_new_parent_extent + context->objects + 1);
  intent_best_parent_extent = (data_t **)(intent_best_parent_intent + context->dt_count_a);
  extent_parent_intent = (data_t *)(intent_best_parent_extent + context->objects + 1);
  extent_parent_extent = (data_t **)(extent_parent_intent + context->dt_count_a);
  extent_new_parent_intent = (data_t *)(extent_parent_extent + context->objects + 1);
  extent_new_parent_extent = (data_t **)(extent_new_parent_intent + context->dt_count_a);
  extent_best_parent_intent = (data_t *)(extent_new_parent_extent + context->objects + 1);
  extent_best_parent_extent = (data_t **)(extent_best_parent_intent + context->dt_count_a);
  factor_intent = (data_t *)(extent_best_parent_extent + context->objects + 1);
  factor_extent = (data_t **)(factor_intent + context->dt_count_a);
  intent_factor_parent_intent = (data_t *)(factor_extent + context->objects + 1);
  intent_factor_parent_extent = (data_t **)(intent_factor_parent_intent + context->dt_count_a);
  extent_factor_parent_intent = (data_t *)(intent_factor_parent_extent + context->objects + 1);
  extent_factor_parent_extent = (data_t **)(extent_factor_parent_intent + context->dt_count_a);
  objects = (data_t **)(extent_factor_parent_extent + context->objects + 1);

  factor_concepts->start = factor_concepts->iter = (data_t *) malloc (DT_SIZE_A + DT_SIZE_O);
  factor_concepts->end = factor_concepts->start + context->dt_count_a + context->objects + 1;
  compute_closure_plus_attr (factor_concepts->start, (data_t **)(factor_concepts->start + context->dt_count_a), NULL, NULL, context, 0, 0, factors->concepts.min_support);
  memset (factor_concepts->start, 0xFF, DT_SIZE_A);
  factor_concepts->start[context->dt_count_a - 1] &= ~BIT;
  new_factor_concepts->start = new_factor_concepts->iter = (data_t *) malloc ((DT_SIZE_A + DT_SIZE_O) * context->attributes);
  new_factor_concepts->end = new_factor_concepts->start + (context->dt_count_a + context->objects + 1) * context->attributes;

  SET_COUNTER(FACTORS, 0);
  SET_COUNTER(UNCOVERED, context->table_entries);

  while (c >= 0) {
    if (!c)
      SAVE_TIME("Ess factors time");

    copy_context(&new_context, contexts[c]);

    if (c == last_c)
      compute_attribute_concepts(attribute_intents.start, contexts[c], factors->concepts.min_support COMMA STATS_DATA_ARG);
    else {
      compute_closure_plus_attr (attribute_intents.start, (data_t **)(attribute_intents.start + context->dt_count_a), NULL, NULL, contexts[c], 0, 0, factors->concepts.min_support);
      INC_COUNTER(CLOSURES);
    }

    while (1) {
      factor_size = 0;

      for (factor_concepts->iter = factor_concepts->start; factor_concepts->iter < factor_concepts->end; factor_concepts->iter = (data_t *)(factor_concepts_extent + (count_t)*factor_concepts_extent + 1)) {
        factor_concepts_extent = (data_t **)(factor_concepts->iter + context->dt_count_a);

        if ((c != last_c) && (factor_concepts->iter [context->dt_count_a - 1] & BIT))
          continue;

        if (c == last_c)
          memset(intent, 0, DT_SIZE_A + DT_SIZE_O);
        else {
          memcpy(intent, attribute_intents.start, DT_SIZE_A);
          memcpy(extent, (data_t *)factor_concepts_extent, DT_SIZE_Ox((count_t)*factor_concepts_extent));
        }
        size = best_size = 0;
        attribute_intents.iter = attribute_intents.start;

        while (1) {
          for (total = j = 0; j < context->dt_count_a; j ++) {
            for (i = ARCHBIT; i >= 0; i --) {
              if (total >= context->attributes)
                goto skipout;
              if (intent [j] & (BIT << i))
                goto skip;

              if (!(factor_concepts->iter [j] & (BIT << i)))
                goto skip;

              if (!size && (c == last_c)) {
                memcpy (new_intent, attribute_intents.iter, DT_SIZE_A + DT_SIZE_O);
                attribute_intents.iter += context->dt_count_a + context->objects + 1;
              } else {
                compute_closure_plus_attr (new_intent, new_extent, extent, intent,
                                           contexts[c], j, BIT << i, factors->concepts.min_support);
                INC_COUNTER(CLOSURES);
              }

              if (c) {
                compute_closure_from_attrs (intent_new_parent_intent, intent_new_parent_extent, new_intent, contexts[c - 1]);
                INC_COUNTER(CLOSURES);
                _transform_objects_to_context (objects, new_extent, contexts[c - 1], contexts[c]);
                compute_closure_from_objects (extent_new_parent_intent, extent_new_parent_extent, objects, contexts[c - 1]);
                INC_COUNTER(CLOSURES);

                compute_closure_masked_size (intent_new_parent_intent, extent_new_parent_extent, contexts[c - 1], &new_context, &new_size, NULL);
              } else
                compute_closure_masked_size (new_intent, new_extent, context, &new_context, &new_size, NULL);

              if (new_size > best_size) {
                best_size = new_size;
                tmp_intent = new_intent; new_intent = best_intent; best_intent = tmp_intent;
                tmp_extent = new_extent; new_extent = best_extent; best_extent = tmp_extent;
                if (c) {
                  tmp_intent = intent_new_parent_intent; intent_new_parent_intent = intent_best_parent_intent; intent_best_parent_intent = tmp_intent;
                  tmp_extent = intent_new_parent_extent; intent_new_parent_extent = intent_best_parent_extent; intent_best_parent_extent = tmp_extent;
                  tmp_intent = extent_new_parent_intent; extent_new_parent_intent = extent_best_parent_intent; extent_best_parent_intent = tmp_intent;
                  tmp_extent = extent_new_parent_extent; extent_new_parent_extent = extent_best_parent_extent; extent_best_parent_extent = tmp_extent;
                }
              }
            skip:
              total ++;
            }
          }
        skipout:
          if (best_size > size) {
            size = best_size;
            tmp_intent = intent; intent = best_intent; best_intent = tmp_intent;
            tmp_extent = extent; extent = best_extent; best_extent = tmp_extent;

            if (c) {
              tmp_intent = intent_parent_intent; intent_parent_intent = intent_best_parent_intent; intent_best_parent_intent = tmp_intent;
              tmp_extent = intent_parent_extent; intent_parent_extent = intent_best_parent_extent; intent_best_parent_extent = tmp_extent;
              tmp_intent = extent_parent_intent; extent_parent_intent = extent_best_parent_intent; extent_best_parent_intent = tmp_intent;
              tmp_extent = extent_parent_extent; extent_parent_extent = extent_best_parent_extent; extent_best_parent_extent = tmp_extent;
            }
          }
          else
            break;
        }

        if (size > factor_size) {
          factor_size = size;
          tmp_intent = factor_intent; factor_intent = intent; intent = tmp_intent;
          tmp_extent = factor_extent; factor_extent = extent; extent = tmp_extent;

          if (c) {
            tmp_intent = intent_factor_parent_intent; intent_factor_parent_intent = intent_parent_intent; intent_parent_intent = tmp_intent;
            tmp_extent = intent_factor_parent_extent; intent_factor_parent_extent = intent_parent_extent; intent_parent_extent = tmp_extent;
            tmp_intent = extent_factor_parent_intent; extent_factor_parent_intent = extent_parent_intent; extent_parent_intent = tmp_intent;
            tmp_extent = extent_factor_parent_extent; extent_factor_parent_extent = extent_parent_extent; extent_parent_extent = tmp_extent;
          }

          factor_concept = factor_concepts->iter;
        }
      }

      if (! factor_size)
        break;

      new_context.table_entries -= factor_size;
      if (c)
        save_concept_to_list (extent_factor_parent_intent, intent_factor_parent_extent, contexts[c - 1], new_factor_concepts, 0);
      else {
        ADD_COUNTER(UNCOVERED, -factor_size);
        SAVE_CONCEPT (factor_intent, factor_extent, context, &factors->concepts, CONCEPT_NEW);
        INC_COUNTER(FACTORS);
      }

      if (c) {
        if (!new_context.table_entries)
          break;
      } else
        if (new_context.table_entries <= table_entries_percent)
          break;

      if (c != last_c)
        factor_concept [context->dt_count_a - 1] |= BIT;

      if (c) {
        tmp_intent = factor_intent;
        factor_intent = intent_factor_parent_intent;
        tmp_extent = factor_extent;
        factor_extent = extent_factor_parent_extent;
        context = contexts[c - 1];
      }
      for (o = (count_t)*factor_extent; o; o --) {
        for (i = 0; i < context->dt_count_a; i ++) {
          new_context.rows [(factor_extent [o] - *context->rows) / context->dt_count_a] [i] &= ~factor_intent [i];
        }
      }
      if (c) {
        factor_intent = tmp_intent;
        factor_extent = tmp_extent;
        context = *contexts;
      }
    }

    new_factor_concepts->end = new_factor_concepts->iter;

    tmp_factor_concepts = factor_concepts; factor_concepts = new_factor_concepts; new_factor_concepts = tmp_factor_concepts;
    factor_concepts->iter = factor_concepts->start;
    new_factor_concepts->iter = new_factor_concepts->start;

    c--;
  }
}

void
find_factors_help (int descr)
{
  if (!descr)
		fprintf (stderr, " [-ness_iters]");
  else
		fprintf (stderr,
						 "-ness_iters    maximal number of Ess context iterations, ess_iters = 0+, 0 = while changes, default 1\n\n");
}

int
find_factors_process_args (int argc, char **argv)
{
  int index = 0;

  if (argv[index][0] == '-')
		switch (argv[index][1]) {
		case 'n':
			ess_iters = atoi (argv [index++] + 2);
		}

  return index;
}

/* CLI prints */

typedef enum {VERBOSITY_NONE = 0, VERBOSITY_OUTPUT, VERBOSITY_STATS,
							VERBOSITY_OUTPUT_STATS, VERBOSITY_INFO,
							VERBOSITY_OUTPUT_INFO} verbosity_t;

int threshold = 100;
count_t min_support = 0;
concept_flags_t factor_output = CONCEPT_SAVE_ATTRIBS_ONLY;
context_sort_t context_sort = 0;

verbosity_t verbosity = VERBOSITY_OUTPUT;
FILE *input_file = NULL;
FILE *output_file = NULL;
count_t attr_offset = 0;

void
print_context_info (context_t *context, char *name)
{
  fprintf (stderr, "%s objects: %li\n%s attributes: %li\n%s table entries: %li\n%s fill ratio: %.3f%%\n", name, context->objects, name, context->attributes, name, context->table_entries, name, (double) 100 * context->table_entries / (context->objects * context->attributes));
}

#ifdef STATS
void
print_stats (STATS_DATA_PARAM)
{
  int i, usec_less;

	for (i = 0; i < COUNTERS; i++)
	  if (COUNTER_TEXT (i))
			fprintf (stderr, "%s: %li\n", COUNTER_TEXT (i), COUNTER (i));

	for (i = 1; i < TIME_POS; i++) {
	  usec_less = (TIME (i).tv_usec < TIME (i - 1).tv_usec) ? 1 : 0;
	  fprintf (stderr, "%s: %li.%06li s\n", TIME_TEXT (i), TIME (i).tv_sec - TIME (i - 1).tv_sec - usec_less, usec_less * 1000000 + TIME (i).tv_usec - TIME (i - 1).tv_usec);
	}
	i--;
	usec_less = (TIME (i).tv_usec < TIME (0).tv_usec) ? 1 : 0;
	fprintf (stderr, "total time: %li.%06li s\n", TIME (i).tv_sec - TIME (0).tv_sec - usec_less, usec_less * 1000000 + TIME (i).tv_usec - TIME (0).tv_usec);
}
#endif

/* CLI Boolean factors as formal concepts finding main, prints and
 * help */

void
print_factor_info (data_t *intent, data_t **extent, context_t *context,
									 concept_flags_t flags COMMA STATS_DATA_PARAM)
{
#ifndef STATS
	static count_t i = 1;
#endif

#ifdef STATS
	fprintf (stderr, "factor %li:\terrors: %li\tcovered: %.3f%%", COUNTER (FACTORS)+1, COUNTER (UNCOVERED), 100 - (double) 100 * COUNTER (UNCOVERED) / context->table_entries);
	if (COUNTER (OVERCOVERED))
	  fprintf (stderr, ", overcovered %li = %.3f%%", COUNTER (OVERCOVERED), (double) 100 * COUNTER (OVERCOVERED) / (context->objects * context->attributes - context->table_entries));
	fprintf (stderr, "\n");
#else
	fprintf (stderr, "factor %li\n", i++);
#endif
}

void
main_help ()
{
  fprintf (stderr, "%s [-Vverbosity] [-attr_offset] [-Tthreshold] [-Smin_support] [-Oa|-Oo|-Of] [-sc|-sr|-sb]", program_name);
  find_factors_help (0);
  fprintf (stderr, " [input_filename [output_filename]]\n");
  fprintf (stderr, "\n"
					 "-Vverbosity    verbosity level, verbosity = 0-5, 0 = no output, 1 = factors output, 2 = counters & time stderr output, 3 = 1 + 2, 4 = 2 + info stderr output, 5 = 1 + 4, default 1\n\n"
					 "-attr_offset   number of the first attribute in input, attr_offset = 0+, default 0\n\n");
  fprintf (stderr,
					 "-Tthreshold    sufficient factorization threshold in %%, threshold = 0-100, default 100\n\n"
					 "-Smin_support  minimal support of factors, min_support = 0+, default 0\n\n"
					 "-Oa|-Oo|-Of    output attributes | objects | factors (attributes | objects), default -Oa\n\n"
					 "-sc|-sr|-sb    sort columns by support | rows lexicaly | both prior to computation\n\n");
  find_factors_help (1);
  fprintf (stderr,
					 "input_filename input context data file name\n");
  exit (1);
}

int
main_process_args (int argc, char **argv)
{
  int index = 0;

  if (argv[index][0] == '-')
		switch (argv[index][1]) {
		case 'T':
			threshold = atoi (argv [index++] + 2);
			break;
		case 'S':
			min_support = atoi (argv [index++] + 2);
			break;
		case 'O':
			switch (argv[index++][2]) {
			case 'a': factor_output = CONCEPT_SAVE_ATTRIBS_ONLY; break;
			case 'o': factor_output = CONCEPT_SAVE_OBJECTS_ONLY; break;
			case 'f': factor_output = 0; break;
			default: return 0;
			}
			break;
		case 's':
			switch (argv[index++][2]) {
			case 'c': context_sort = CONTEXT_SORT_COLS; break;
			case 'r': context_sort = CONTEXT_SORT_ROWS; break;
			case 'b': context_sort = CONTEXT_SORT_COLS | CONTEXT_SORT_ROWS; break;
			default: return 0;
			}
			break;
		default:
			index += find_factors_process_args (argc - index, &argv[index]);
		}

  return index;
}

void
main_program ()
{
	context_t context;
	io_t io;
	factors_t factors;

	STATS_DATA;

	INIT_STATS (1, 6, 10);
  SAVE_TIME ("");

	init_io (&io, (io_func_t *)read_file_to_io, input_file);
  input_context_from_io (&context, &io, attr_offset);


	if (verbosity >= VERBOSITY_INFO) {
		print_context_info (&context, "input");
	}

	SAVE_TIME ("input read time");

  if (context_sort)
		sort_context (&context, context_sort);

  if (min_support) {
		remove_low_support_attribs_from_context (&context, min_support);
		if (verbosity >= VERBOSITY_INFO)
			print_context_info (&context, "preprocessed");
  }

  SAVE_TIME("preprocessing time");

	SET_COUNTER_TEXT (FACTORS, "factors");
  SET_COUNTER_TEXT (CLOSURES, "closures");
  SET_COUNTER_TEXT (UNCOVERED, "uncovered entries");
  SET_COUNTER_TEXT (OVERCOVERED, "overcovered entries");

	free_io (&io);
	init_io (&io, (io_func_t *)write_file_from_io, output_file);

	init_factors (&factors, threshold);
	if (verbosity % 2)
		init_concepts (&factors.concepts, min_support, (concept_save_func_t *)output_factor_to_io, &io, (verbosity >= VERBOSITY_INFO) ? (concept_info_func_t *)print_factor_info : NULL, factor_output);
	else
		init_concepts (&factors.concepts, min_support, NULL, NULL, (verbosity >= VERBOSITY_INFO) ? (concept_info_func_t *)print_factor_info : NULL, 0);

  find_factors (&context, &factors COMMA STATS_DATA_ARG);

	IO_FUNC (&io, OUTPUT_ALL);

  SAVE_TIME ("find time");

	free_concepts (&factors.concepts);
	free_io (&io);

#ifdef STATS
	if (verbosity >= VERBOSITY_STATS)
		print_stats (STATS_DATA_ARG);
#endif

	FREE_STATS;

	destroy_context (&context);
}

/* CLI main */

int main (int argc, char **argv)
{
  lastCol = 0;
  int index = 1;
  int shift;
  int input_file_stdin = 0;

	set_error (argv[0], (error_func_t *)print_error);

  input_file = stdin;
  output_file = stdout;

  while (index < argc) {
		if (argv[index][0] == '-') {
			switch (argv[index][1]) {
			case 0:
				input_file_stdin = 1;
				index++;
				break;
			case 'V':
				verbosity = atoi (argv [index++] + 2);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				attr_offset = atoi (argv [index++] + 1);
				break;
			default:
				if (!(shift = main_process_args (argc - index, &argv[index]))) {
					main_help ();
					index++;
				} else
					index += shift;
			}
		} else {
			if ((input_file == stdin) && !input_file_stdin)
				input_file = fopen (argv [index++], "rb");
			else if (output_file == stdout) {
				if (input_file)
					output_file = fopen (argv [index++], "wb");
			} else {
				if (!(shift = main_process_args (argc - index, &argv[index]))) {
					main_help();
					index++;
				} else
					index += shift;
			}
		}
  }

  if (!input_file) {
		error (ERROR_FILE_OPEN, "cannot open input data stream");
	  return ERROR_FILE_OPEN;
  }
  if (!output_file) {
		error (ERROR_FILE_OPEN, "cannot open output data stream");
	  return ERROR_FILE_OPEN;
	}

  main_program ();

  fclose (input_file);
  fclose (output_file);

  return 0;
}
