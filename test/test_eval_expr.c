/*
	THIS IS AI GENERATED TEST BENCH FOR NON SUB-QUERY CONTAINING SQL-EXPRESSIONS
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#include <cutlery/stream.h>
#include <cutlery/stream_for_file_descriptor.h>
#include <cutlery/stream_for_dstring.h>

#include <sqltoast/sqltoast.h>
#include <sqltoast/sql_expression_eval.h>

/* ------------------------------------------------------------------ *
 *  Value model: every "data pointer" the evaluator passes around is a
 *  datum* (or C NULL for SQL NULL, or one of the static constants).
 * ------------------------------------------------------------------ */
typedef enum data_type data_type;
enum data_type { INTEGER, FLOATING, STRING };

typedef struct datum datum;
struct datum
{
	data_type type;
	union
	{
		int64_t integer;
		double  floating;
		struct { char* string; uint64_t string_size; };
	};
};

/* The constants the evaluator needs. They are distinguished by *address*,
 * so they must be distinct static objects (UNKNOWN and FALSE share a value
 * but must NOT share an address). delete_data() is a no-op on all of them. */
static datum TRUE_D      = { .type = INTEGER, .integer =  1 };
static datum FALSE_D     = { .type = INTEGER, .integer =  0 };
static datum UNKNOWN_D   = { .type = INTEGER, .integer =  0 };
static datum ONE_D       = { .type = INTEGER, .integer =  1 };
static datum ZERO_D      = { .type = INTEGER, .integer =  0 };
static datum MINUS_ONE_D = { .type = INTEGER, .integer = -1 };

static int is_const(const void* d)
{
	return d == &TRUE_D || d == &FALSE_D || d == &UNKNOWN_D ||
	       d == &ONE_D   || d == &ZERO_D  || d == &MINUS_ONE_D;
}

/* ---- allocators ---- */
static datum* mk_int(int64_t v)   { datum* d = malloc(sizeof *d); d->type = INTEGER;  d->integer  = v; return d; }
static datum* mk_float(double v)  { datum* d = malloc(sizeof *d); d->type = FLOATING; d->floating = v; return d; }
static datum* mk_str_take(char* s, uint64_t n) { datum* d = malloc(sizeof *d); d->type = STRING; d->string = s; d->string_size = n; return d; }

/* text representation of a datum (caller frees) — used by concat / like / cast-to-text */
static char* to_text(const datum* d)
{
	char buf[64];
	if(d->type == INTEGER)  { snprintf(buf, sizeof buf, "%lld", (long long)d->integer); return strdup(buf); }
	if(d->type == FLOATING) { snprintf(buf, sizeof buf, "%g", d->floating); return strdup(buf); }
	char* s = malloc(d->string_size + 1); memcpy(s, d->string, d->string_size); s[d->string_size] = 0; return s;
}

static int    is_num(const datum* d) { return d->type == INTEGER || d->type == FLOATING; }
static double  as_dbl(const datum* d) { return d->type == INTEGER ? (double)d->integer : d->floating; }

/* deep copy — functions must return a *fresh* value because evaluate_sql_expr
 * frees every argument it passed in after the call returns. */
static void* datum_dup(const datum* d)
{
	if(d == NULL) return NULL;
	if(d->type == STRING) { char* s = malloc(d->string_size + 1); memcpy(s, d->string, d->string_size); s[d->string_size] = 0; return mk_str_take(s, d->string_size); }
	return d->type == INTEGER ? (void*)mk_int(d->integer) : (void*)mk_float(d->floating);
}

/* ------------------------------------------------------------------ *
 *  Callbacks. EVERY callback returns NULL (and sets *ec) on error, as
 *  the header requires.
 * ------------------------------------------------------------------ */
static void delete_data(void* data, const sql_expr_eval_context* ec)
{
	(void)ec;
	if(data == NULL || is_const(data)) return;
	datum* d = data;
	if(d->type == STRING) free(d->string);
	free(d);
}

static void* get_bool(void* data, const sql_expr_eval_context* ec, int* error_code)
{
	(void)error_code;
	datum* d = data;                 /* evaluator guarantees data is a real value here */
	if(is_num(d)) return as_dbl(d) != 0 ? ec->true_bool : ec->false_bool;
	/* string truthiness: coerce leading numeric prefix */
	return strtod(d->string, NULL) != 0 ? ec->true_bool : ec->false_bool;
}

/* shared arithmetic: integer-by-integer stays integer, anything with a
 * float promotes to float; non-numeric or divide-by-zero -> error. */
typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } arith_op;
static void* arith(const datum* a, const datum* b, arith_op op, int* ec)
{
	if(!is_num(a) || !is_num(b)) { *ec = 1; return NULL; }
	if(a->type == INTEGER && b->type == INTEGER)
	{
		int64_t x = a->integer, y = b->integer;
		switch(op)
		{
			case OP_ADD: return mk_int(x + y);
			case OP_SUB: return mk_int(x - y);
			case OP_MUL: return mk_int(x * y);
			case OP_DIV: if(y == 0) { *ec = 1; return NULL; } return mk_int(x / y);
		}
	}
	double x = as_dbl(a), y = as_dbl(b);
	switch(op)
	{
		case OP_ADD: return mk_float(x + y);
		case OP_SUB: return mk_float(x - y);
		case OP_MUL: return mk_float(x * y);
		case OP_DIV: if(y == 0) { *ec = 1; return NULL; } return mk_float(x / y);
	}
	*ec = 1; return NULL;
}
static void* add(void* a, void* b, const sql_expr_eval_context* ec, int* e) { (void)ec; return arith(a, b, OP_ADD, e); }
static void* sub(void* a, void* b, const sql_expr_eval_context* ec, int* e) { (void)ec; return arith(a, b, OP_SUB, e); }
static void* mul(void* a, void* b, const sql_expr_eval_context* ec, int* e) { (void)ec; return arith(a, b, OP_MUL, e); }
static void* divf(void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; return arith(a, b, OP_DIV, e); }
static void* mod(void* a, void* b, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; const datum* x = a; const datum* y = b;
	if(!is_num(x) || !is_num(y)) { *e = 1; return NULL; }
	if(x->type == INTEGER && y->type == INTEGER) { if(y->integer == 0) { *e = 1; return NULL; } return mk_int(x->integer % y->integer); }
	double bb = as_dbl(y); if(bb == 0) { *e = 1; return NULL; } return mk_float(fmod(as_dbl(x), bb));
}

static int compare(void* a, void* b, const sql_expr_eval_context* ec, int* error_code)
{
	(void)ec; (void)error_code; const datum* x = a; const datum* y = b;
	if(is_num(x) && is_num(y)) { double u = as_dbl(x), v = as_dbl(y); return (u > v) - (u < v); }
	if(x->type == STRING && y->type == STRING)
	{
		uint64_t n = x->string_size < y->string_size ? x->string_size : y->string_size;
		int c = memcmp(x->string, y->string, n);
		if(c) return c > 0 ? 1 : -1;
		return (x->string_size > y->string_size) - (x->string_size < y->string_size);
	}
	return x->type == STRING ? 1 : -1;     /* numbers sort before strings */
}

static int64_t to_i64(const datum* d, int* e) { if(d->type == INTEGER) return d->integer; if(d->type == FLOATING) return (int64_t)d->floating; *e = 1; return 0; }
static void* left_shift (void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er), y=to_i64(b,&er); if(er){*e=1;return NULL;} return mk_int(x << (y & 63)); }
static void* right_shift(void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er), y=to_i64(b,&er); if(er){*e=1;return NULL;} return mk_int(x >> (y & 63)); }
static void* bit_and(void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er), y=to_i64(b,&er); if(er){*e=1;return NULL;} return mk_int(x & y); }
static void* bit_or (void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er), y=to_i64(b,&er); if(er){*e=1;return NULL;} return mk_int(x | y); }
static void* bit_xor(void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er), y=to_i64(b,&er); if(er){*e=1;return NULL;} return mk_int(x ^ y); }
static void* bit_not(void* a,           const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er);              if(er){*e=1;return NULL;} return mk_int(~x); }

static void* cast(void* data, const sql_type* to_type, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; const datum* d = data;
	switch(to_type->type_name)
	{
		case SQL_SMALLINT: case SQL_INT: case SQL_BIGINT: case SQL_BOOL:
			if(d->type == INTEGER)  return mk_int(d->integer);
			if(d->type == FLOATING) return mk_int((int64_t)d->floating);
			return mk_int(strtoll(d->string, NULL, 10));
		case SQL_REAL: case SQL_DOUBLE: case SQL_FLOAT: case SQL_DECIMAL: case SQL_NUMERIC:
			if(d->type == INTEGER)  return mk_float((double)d->integer);
			if(d->type == FLOATING) return mk_float(d->floating);
			return mk_float(strtod(d->string, NULL));
		case SQL_TEXT: case SQL_CHAR: case SQL_VARCHAR: case SQL_STRING: case SQL_CLOB:
		{ char* s = to_text(d); return mk_str_take(s, strlen(s)); }
		default: *e = 1; return NULL;     /* unsupported target type */
	}
}

static void* create_number(const dstring* data_bytes, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; (void)e;
	char buf[128]; cy_uint n = get_char_count_dstring(data_bytes);
	if(n >= sizeof buf) n = sizeof buf - 1;
	memcpy(buf, get_byte_array_dstring(data_bytes), n); buf[n] = 0;
	if(strpbrk(buf, ".eE")) return mk_float(strtod(buf, NULL));
	return mk_int(strtoll(buf, NULL, 10));
}
static void* create_string(const dstring* data_bytes, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; (void)e;
	cy_uint n = get_char_count_dstring(data_bytes);
	char* s = malloc(n + 1); memcpy(s, get_byte_array_dstring(data_bytes), n); s[n] = 0;
	return mk_str_take(s, n);
}

/* concat appends data2 into *data1_p, replacing it; does NOT free data2;
 * on error leaves *data1_p a valid (deletable) pointer. */
static void concat(void** data1_p, void* data2, const sql_expr_eval_context* ec, int* e)
{
	(void)e;
	char* ta = to_text(*data1_p); char* tb = to_text(data2);
	uint64_t na = strlen(ta), nb = strlen(tb);
	char* r = malloc(na + nb + 1); memcpy(r, ta, na); memcpy(r + na, tb, nb + 1);
	free(ta); free(tb);
	delete_data(*data1_p, ec);
	*data1_p = mk_str_take(r, na + nb);
}

/* SQL LIKE: % = any run, _ = exactly one char (case-sensitive, ANSI style) */
static int like_match(const char* s, const char* p)
{
	const char* star = NULL; const char* ss = NULL;
	while(*s)
	{
		if(*p == '_' || *p == *s) { s++; p++; }
		else if(*p == '%')        { star = p++; ss = s; }
		else if(star)             { p = star + 1; s = ++ss; }
		else return 0;
	}
	while(*p == '%') p++;
	return *p == 0;
}
static void* like(void* str_p, void* pattern_p, const sql_expr_eval_context* ec, int* e)
{
	(void)e;
	char* s = to_text(str_p); char* p = to_text(pattern_p);
	int m = like_match(s, p); free(s); free(p);
	return m ? ec->true_bool : ec->false_bool;
}

/* ---- a few scalar functions (extend as you like) ---- */
static void* fn_abs(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; if(n != 1) { *e = 1; return NULL; } if(!p[0]) return NULL;
	datum* d = p[0];
	if(d->type == INTEGER)  return mk_int(d->integer < 0 ? -d->integer : d->integer);
	if(d->type == FLOATING) return mk_float(d->floating < 0 ? -d->floating : d->floating);
	*e = 1; return NULL;
}
static void* fn_length(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; if(n != 1) { *e = 1; return NULL; } if(!p[0]) return NULL;
	char* t = to_text(p[0]); int64_t L = (int64_t)strlen(t); free(t); return mk_int(L);
}
static void* fn_coalesce(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; (void)e; for(uint32_t i = 0; i < n; i++) if(p[i]) return datum_dup(p[i]); return NULL;
}
static void* fn_upper(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; if(n != 1) { *e = 1; return NULL; } if(!p[0]) return NULL;
	char* t = to_text(p[0]); for(char* c = t; *c; c++) *c = (char)toupper((unsigned char)*c); return mk_str_take(t, strlen(t));
}
static void* fn_lower(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; if(n != 1) { *e = 1; return NULL; } if(!p[0]) return NULL;
	char* t = to_text(p[0]); for(char* c = t; *c; c++) *c = (char)tolower((unsigned char)*c); return mk_str_take(t, strlen(t));
}
static void* call_function(const dstring* id, void** params, uint32_t params_count, const sql_expr_eval_context* ec, int* e)
{
	(void)params_count; (void)ec;
	char nm[64]; cy_uint L = get_char_count_dstring(id);
	if(L >= sizeof nm) L = sizeof nm - 1;
	memcpy(nm, get_byte_array_dstring(id), L);
	nm[L] = 0;
	for(char* c = nm; *c; c++) *c = (char)tolower((unsigned char)*c);
	if(!strcmp(nm, "abs"))      return fn_abs(params, params_count, ec, e);
	if(!strcmp(nm, "length"))   return fn_length(params, params_count, ec, e);
	if(!strcmp(nm, "coalesce")) return fn_coalesce(params, params_count, ec, e);
	if(!strcmp(nm, "upper"))    return fn_upper(params, params_count, ec, e);
	if(!strcmp(nm, "lower"))    return fn_lower(params, params_count, ec, e);
	*e = 1; return NULL;        /* unknown function -> error (never return NULL without setting *e) */
}

static sql_expr_eval_context make_context(void)
{
	sql_expr_eval_context ec = (sql_expr_eval_context){0};
	ec.get_bool    = get_bool;
	ec.true_bool   = &TRUE_D;   ec.false_bool = &FALSE_D;   ec.unknown_bool = &UNKNOWN_D;
	ec.one_number  = &ONE_D;    ec.zero_number = &ZERO_D;   ec.minus_one_number = &MINUS_ONE_D;
	ec.add = add; ec.sub = sub; ec.mul = mul; ec.div = divf; ec.mod = mod;
	ec.compare = compare;
	ec.left_shift = left_shift; ec.right_shift = right_shift;
	ec.bit_and = bit_and; ec.bit_or = bit_or; ec.bit_xor = bit_xor; ec.bit_not = bit_not;
	ec.cast = cast;
	ec.create_number = create_number; ec.create_string = create_string;
	ec.concat = concat; ec.like = like;
	ec.delete_data = delete_data;
	ec.get_sub_query = NULL; ec.next_data_from_sub_query = NULL; ec.delete_sub_query = NULL;
	ec.call_function = call_function;
	ec.get_variable = NULL;
	return ec;
}

/* equality of two results, for the unflattened-vs-flattened cross-check.
 * Handles NULL, the shared constants (by identity), and value equality. */
static int datum_equal(const void* a, const void* b)
{
	if(a == b) return 1;                 /* same pointer: both NULL, or same constant */
	if(a == NULL || b == NULL) return 0;
	const datum* x = a; const datum* y = b;
	if(is_num(x) && is_num(y)) return as_dbl(x) == as_dbl(y);
	if(x->type == STRING && y->type == STRING)
		return x->string_size == y->string_size && memcmp(x->string, y->string, x->string_size) == 0;
	return 0;
}

static void print_result(const char* label, const void* r, const sql_expr_eval_context* ec)
{
	if(r == NULL)                 { printf("%s = NULL\n", label); return; }
	if(r == ec->true_bool)        { printf("%s = TRUE\n", label); return; }
	if(r == ec->false_bool)       { printf("%s = FALSE\n", label); return; }
	if(r == ec->unknown_bool)     { printf("%s = UNKNOWN\n", label); return; }
	const datum* d = r;
	if(d->type == INTEGER)  printf("%s = %lld\n", label, (long long)d->integer);
	else if(d->type == FLOATING) printf("%s = %g\n", label, d->floating);
	else printf("%s = '%.*s'\n", label, (int)d->string_size, d->string);
}

int main()
{
	sql_expr_eval_context ec = make_context();

	stream rs;
	initialize_stream_for_fd(&rs, 0);

	int error1 = 0;
	sql* sqlast1 = parse_sql(&rs, &error1);

	if(sqlast1)
	{
		dstring str1;
		init_empty_dstring(&str1, 0);

		make_dstring_empty(&str1);
		snprint_sql(&str1, sqlast1);
		printf("unflattened " printf_dstring_format "\n\n", printf_dstring_params(&str1));

		int error_code1 = 0; void* res1 = NULL;
		if(sqlast1->type == EXPR)
			res1 = evaluate_sql_expr(sqlast1->expr, &ec, &error_code1);

		flatten_exprs_sql(sqlast1);

		make_dstring_empty(&str1);
		snprint_sql(&str1, sqlast1);
		printf("flattened " printf_dstring_format "\n\n", printf_dstring_params(&str1));

		int error_code2 = 0; void* res2 = NULL;
		if(sqlast1->type == EXPR)
			res2 = evaluate_sql_expr(sqlast1->expr, &ec, &error_code2);

		if(sqlast1->type == EXPR)
		{
			print_result("result (unflattened)", res1, &ec);
			print_result("result (flattened)  ", res2, &ec);

			int consistent = (error_code1 == error_code2) &&
			                 ((error_code1 != 0) || datum_equal(res1, res2));
			printf("error_code: unflattened=%d flattened=%d\n", error_code1, error_code2);
			printf("flatten-consistency: %s\n", consistent ? "PASS" : "FAIL");

			ec.delete_data(res1, &ec);
			ec.delete_data(res2, &ec);
		}

		deinit_dstring(&str1);
		delete_sql(sqlast1);
	}
	else
		printf("parse error = %d\n", error1);

	deinitialize_stream(&rs);
	return 0;
}