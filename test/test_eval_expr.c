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
	if(d->type == FLOATING) { double f = d->floating; if(f == 0.0) f = 0.0; snprintf(buf, sizeof buf, "%g", f); return strdup(buf); }
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
static void cb_delete_data(void* data, const sql_expr_eval_context* ec)
{
	(void)ec;
	if(data == NULL || is_const(data)) return;
	datum* d = data;
	if(d->type == STRING) free(d->string);
	free(d);
}

static void* cb_get_bool(void* data, const sql_expr_eval_context* ec, int* error_code)
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
			case OP_DIV: if(y == 0 || (y == -1 && x == INT64_MIN)) { *ec = 1; return NULL; } return mk_int(x / y);
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
	if(x->type == INTEGER && y->type == INTEGER) { if(y->integer == 0 || (y->integer == -1 && x->integer == INT64_MIN)) { *e = 1; return NULL; } return mk_int(x->integer % y->integer); }
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
static void* left_shift (void* a, void* b, const sql_expr_eval_context* ec, int* e){ (void)ec; int er=0; int64_t x=to_i64(a,&er), y=to_i64(b,&er); if(er){*e=1;return NULL;} return mk_int((int64_t)((uint64_t)x << (y & 63))); }
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
	cb_delete_data(*data1_p, ec);
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
/* ---- scalar functions (pure; no state) ---- */
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
/* sin / cos : one numeric arg -> float ; NULL arg -> NULL */
static void* fn_sin(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{ (void)ec; if(n != 1) { *e = 1; return NULL; } if(!p[0]) return NULL; datum* d = p[0]; if(!is_num(d)) { *e = 1; return NULL; } return mk_float(sin(as_dbl(d))); }
static void* fn_cos(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{ (void)ec; if(n != 1) { *e = 1; return NULL; } if(!p[0]) return NULL; datum* d = p[0]; if(!is_num(d)) { *e = 1; return NULL; } return mk_float(cos(as_dbl(d))); }
/* min / max : >=1 numeric args -> int if all int else float ; any NULL arg -> NULL */
static void* fn_minmax(void** p, uint32_t n, int want_max, int* e)
{
	if(n < 1) { *e = 1; return NULL; }
	int all_int = 1; double best = 0; int have = 0;
	for(uint32_t i = 0; i < n; i++)
	{
		if(!p[i]) return NULL;                       /* NULL propagates */
		datum* d = p[i]; if(!is_num(d)) { *e = 1; return NULL; }
		if(d->type != INTEGER) all_int = 0;
		double v = as_dbl(d);
		if(!have || (want_max ? v > best : v < best)) best = v;
		have = 1;
	}
	return all_int ? (void*)mk_int((int64_t)best) : (void*)mk_float(best);
}
static void* fn_min(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e){ (void)ec; return fn_minmax(p, n, 0, e); }
static void* fn_max(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e){ (void)ec; return fn_minmax(p, n, 1, e); }
/* avg : >=1 numeric args -> float ; any NULL arg -> NULL */
static void* fn_avg(void** p, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; if(n < 1) { *e = 1; return NULL; } double s = 0;
	for(uint32_t i = 0; i < n; i++){ if(!p[i]) return NULL; datum* d = p[i]; if(!is_num(d)) { *e = 1; return NULL; } s += as_dbl(d); }
	return mk_float(s / (double)n);
}

static void* call_function(const dstring* id, void** params, uint32_t params_count, const sql_expr_eval_context* ec, int* e)
{
	char nm[64]; cy_uint L = get_char_count_dstring(id);
	if(L >= sizeof nm) L = sizeof nm - 1;
	memcpy(nm, get_byte_array_dstring(id), L); nm[L] = 0;
	for(char* c = nm; *c; c++) *c = (char)tolower((unsigned char)*c);
	if(!strcmp(nm, "abs"))      return fn_abs(params, params_count, ec, e);
	if(!strcmp(nm, "length"))   return fn_length(params, params_count, ec, e);
	if(!strcmp(nm, "coalesce")) return fn_coalesce(params, params_count, ec, e);
	if(!strcmp(nm, "upper"))    return fn_upper(params, params_count, ec, e);
	if(!strcmp(nm, "lower"))    return fn_lower(params, params_count, ec, e);
	if(!strcmp(nm, "sin"))      return fn_sin(params, params_count, ec, e);
	if(!strcmp(nm, "cos"))      return fn_cos(params, params_count, ec, e);
	if(!strcmp(nm, "min"))      return fn_min(params, params_count, ec, e);
	if(!strcmp(nm, "max"))      return fn_max(params, params_count, ec, e);
	if(!strcmp(nm, "avg"))      return fn_avg(params, params_count, ec, e);
	*e = 1; return NULL;        /* unknown function -> error */
}

/* ================================================================== *
 *  TYPE INFERENCE side.
 *
 *  Every type is HEAP ALLOCATED (malloc) so that a type the library
 *  creates but fails to hand back or delete shows up as a real leak
 *  (caught by TYPE_LIVE below and by valgrind/ASan).  The one exception
 *  is the boolean type, which — as the header requires — is a single
 *  static constant that delete_type must never free.  In this datum model
 *  a boolean IS an integer (TRUE_D/FALSE_D are INTEGER 1/0), so the static
 *  bool type carries tag INT.  SQL NULL infers to the null/bottom type,
 *  represented (per the library's design) as a C NULL pointer.
 *
 *  Ownership contract every callback obeys: a type-returning callback hands
 *  back a FRESH allocation (or the static bool, or NULL) and never one of
 *  its input type pointers — the library deletes the inputs after the call,
 *  so returning an input would be a double-free, not a leak.
 * ================================================================== */
enum ty_tag { TY_INT, TY_FLOAT, TY_STRING };
typedef struct { enum ty_tag tag; } ty;

static long TYPE_LIVE = 0;                  /* outstanding malloc'd (non-bool) types */
static void* new_type(enum ty_tag t) { ty* x = malloc(sizeof *x); x->tag = t; TYPE_LIVE++; return x; }
static void* copy_type(const void* t)  { return new_type(((const ty*)t)->tag); }   /* t must be non-NULL */
static enum ty_tag ty_of(const void* t){ return ((const ty*)t)->tag; }             /* t must be non-NULL */
static int tag_num(enum ty_tag k)      { return k == TY_INT || k == TY_FLOAT; }
static int ty_num(const void* t)       { return t != NULL && tag_num(ty_of(t)); }
static int ty_is (const void* t, enum ty_tag k) { return t != NULL && ty_of(t) == k; }

static const char* type_name(const void* t)
{
	if(t == NULL) return "NULL";        /* bottom / SQL NULL */
	switch(ty_of(t)) { case TY_INT: return "INT"; case TY_FLOAT: return "FLOAT"; case TY_STRING: return "STRING"; }
	return "?";
}

/* frees a malloc'd type; leaves the static boolean type and NULL untouched */
static void cb_delete_type(void* t, const sql_expr_eval_context* ec) { (void)ec; if(t == NULL) return; free(t); TYPE_LIVE--; }

static void* get_type_for_data(void* data, const sql_expr_eval_context* ec, int* e)
{
	(void)ec; (void)e;
	if(data == NULL) return NULL;
	const datum* d = data;
	if(d->type == INTEGER)  return new_type(TY_INT);
	if(d->type == FLOATING) return new_type(TY_FLOAT);
	return new_type(TY_STRING);
}

/* common supertype for CASE / COALESCE ; always a FRESH allocation, never an input */
static void* unify_types(void* t1, void* t2, const sql_expr_eval_context* ec, int* e)
{
	(void)ec;
	if(t1 == NULL && t2 == NULL) return NULL;                 /* bottom + bottom = bottom */
	if(t1 == NULL) return copy_type(t2);
	if(t2 == NULL) return copy_type(t1);
	if(ty_of(t1) == ty_of(t2)) return new_type(ty_of(t1));
	if(ty_num(t1) && ty_num(t2)) return new_type(TY_FLOAT);   /* int + float -> float */
	*e = 1; return NULL;                                      /* numeric vs string -> incompatible */
}

/* the value model's compare() and cast() accept every combination, so these never reject.
 * (they read the input types but must NOT free them — the library owns them) */
static int can_compare_types(void* a, void* b, const sql_expr_eval_context* ec, int* e) { (void)a; (void)b; (void)ec; (void)e; return 1; }
static int can_cast_types   (void* to, void* from, const sql_expr_eval_context* ec, int* e) { (void)to; (void)from; (void)ec; (void)e; return 1; }

static void* get_type_for_sql_type(const sql_type* t, const sql_expr_eval_context* ec, int* e)
{
	(void)ec;
	switch(t->type_name)
	{
		case SQL_SMALLINT: case SQL_INT: case SQL_BIGINT: case SQL_BOOL: return new_type(TY_INT);
		case SQL_REAL: case SQL_DOUBLE: case SQL_FLOAT: case SQL_DECIMAL: case SQL_NUMERIC: return new_type(TY_FLOAT);
		case SQL_TEXT: case SQL_CHAR: case SQL_VARCHAR: case SQL_STRING: case SQL_CLOB: return new_type(TY_STRING);
		default: *e = 1; return NULL;
	}
}

/* result type of a binary/unary operator, keyed on the exec-callback pointer.
 * Result depends only on the SET of operand tags (flatten-invariant); NULL is type-transparent. */
static void* get_return_type_for_op_exec_callback(void* op, void* t1, void* t2, const sql_expr_eval_context* ec, int* e)
{
	if(op == (void*)ec->add || op == (void*)ec->sub || op == (void*)ec->mul || op == (void*)ec->div || op == (void*)ec->mod)
	{
		if(ty_is(t1, TY_STRING) || ty_is(t2, TY_STRING)) { *e = 1; return NULL; }   /* non-NULL string in arithmetic is a type error */
		return (ty_is(t1, TY_FLOAT) || ty_is(t2, TY_FLOAT)) ? new_type(TY_FLOAT) : new_type(TY_INT);
	}
	if(op == (void*)ec->left_shift || op == (void*)ec->right_shift ||
	   op == (void*)ec->bit_and || op == (void*)ec->bit_or || op == (void*)ec->bit_xor || op == (void*)ec->bit_not)
	{
		if(ty_is(t1, TY_STRING) || ty_is(t2, TY_STRING)) { *e = 1; return NULL; }
		return new_type(TY_INT);
	}
	if(op == (void*)ec->concat) return new_type(TY_STRING);          /* concat is always string-typed, NULL or not */
	if(op == (void*)ec->like)   return ec->bool_type;               /* static bool (library deletes it -> skipped) */
	*e = 1; return NULL;
}

/* return type of a function call, keyed on name + argument tags.  Mirrors the fn_* impls;
 * always returns a FRESH allocation (or the bottom type), never an input arg type. */
static void* get_return_type_for_function(const dstring* id, void** at, uint32_t n, const sql_expr_eval_context* ec, int* e)
{
	(void)ec;
	char nm[64]; cy_uint L = get_char_count_dstring(id); if(L >= sizeof nm) L = sizeof nm - 1;
	memcpy(nm, get_byte_array_dstring(id), L); nm[L] = 0;
	for(char* c = nm; *c; c++) *c = (char)tolower((unsigned char)*c);

	int any_null = 0, all_int = 1, all_num = 1;
	for(uint32_t i = 0; i < n; i++)
	{
		if(at[i] == NULL) any_null = 1;
		else { if(ty_of(at[i]) != TY_INT) all_int = 0; if(!ty_num(at[i])) all_num = 0; }
	}

	if(!strcmp(nm, "abs"))    { if(n != 1) { *e = 1; return NULL; } if(any_null) return NULL; if(!all_num) { *e = 1; return NULL; } return copy_type(at[0]); }
	if(!strcmp(nm, "length")) { if(n != 1) { *e = 1; return NULL; } if(any_null) return NULL; return new_type(TY_INT); }
	if(!strcmp(nm, "upper") || !strcmp(nm, "lower")) { if(n != 1) { *e = 1; return NULL; } if(any_null) return NULL; return new_type(TY_STRING); }
	if(!strcmp(nm, "sin") || !strcmp(nm, "cos")) { if(n != 1) { *e = 1; return NULL; } if(any_null) return NULL; if(!all_num) { *e = 1; return NULL; } return new_type(TY_FLOAT); }
	if(!strcmp(nm, "min") || !strcmp(nm, "max")) { if(n < 1) { *e = 1; return NULL; } if(any_null) return NULL; if(!all_num) { *e = 1; return NULL; } return new_type(all_int ? TY_INT : TY_FLOAT); }
	if(!strcmp(nm, "avg")) { if(n < 1) { *e = 1; return NULL; } if(any_null) return NULL; if(!all_num) { *e = 1; return NULL; } return new_type(TY_FLOAT); }
	if(!strcmp(nm, "coalesce"))
	{
		int seen = 0; enum ty_tag acc = TY_INT;              /* fold arg tags; no intermediate allocation */
		for(uint32_t i = 0; i < n; i++)
		{
			if(at[i] == NULL) continue;
			enum ty_tag ti = ty_of(at[i]);
			if(!seen) { acc = ti; seen = 1; }
			else if(acc == ti) { /* same */ }
			else if(tag_num(acc) && tag_num(ti)) acc = TY_FLOAT;
			else { *e = 1; return NULL; }
		}
		return seen ? new_type(acc) : NULL;                  /* all-NULL args -> bottom */
	}
	*e = 1; return NULL;
}

static sql_expr_eval_context make_context(void)
{
	sql_expr_eval_context ec = (sql_expr_eval_context){0};
	ec.get_bool    = cb_get_bool;
	ec.true_bool   = &TRUE_D;   ec.false_bool = &FALSE_D;   ec.unknown_bool = &UNKNOWN_D;
	ec.one_number  = &ONE_D;    ec.zero_number = &ZERO_D;   ec.minus_one_number = &MINUS_ONE_D;
	ec.add = add; ec.sub = sub; ec.mul = mul; ec.div = divf; ec.mod = mod;
	ec.compare = compare;
	ec.left_shift = left_shift; ec.right_shift = right_shift;
	ec.bit_and = bit_and; ec.bit_or = bit_or; ec.bit_xor = bit_xor; ec.bit_not = bit_not;
	ec.cast = cast;
	ec.create_number = create_number; ec.create_string = create_string;
	ec.concat = concat; ec.like = like;
	ec.delete_data = cb_delete_data;
	ec.get_sub_query = NULL; ec.next_data_from_sub_query = NULL; ec.delete_sub_query = NULL;
	ec.call_function = call_function;
	ec.get_variable = NULL;

	/* ---- type inference wiring (constant expressions only: no variable / sub-query type callbacks) ---- */
	ec.bool_type = new_type(TY_INT);              /* bool type is heap-allocated too; the library never frees it (it guards t==ec->bool_type) */
	ec.can_compare_types = can_compare_types;
	ec.get_type_for_sql_type = get_type_for_sql_type;
	ec.can_cast_types = can_cast_types;
	ec.get_return_type_for_op_exec_callback = get_return_type_for_op_exec_callback;
	ec.get_type_for_data = get_type_for_data;
	ec.unify_types = unify_types;
	ec.get_type_for_sub_query = NULL;             /* not exercised (no sub-queries) */
	ec.get_return_type_for_function = get_return_type_for_function;
	ec.get_type_for_variable = NULL;              /* not exercised (no variables) */
	ec.delete_type = cb_delete_type;
	return ec;
}

/* floats are compared with a relative tolerance: flattening rewrites a-b-c as an ADD_FLAT
 * of a+(-b)+(-c), which legitimately reassociates the sum and changes the low-order bits. */
static int approx_eq(double u, double v)
{
	if(u == v) return 1;
	if(!isfinite(u) && !isfinite(v)) return 1;                 /* both overflowed/NaN: not the thing under test */
	double d = fabs(u - v), m = fmax(1.0, fmax(fabs(u), fabs(v)));
	return d <= 1e-9 * m;
}
static int datum_equal(const void* a, const void* b)
{
	if(a == b) return 1;
	if(a == NULL || b == NULL) return 0;
	const datum* x = a; const datum* y = b;
	if(is_num(x) && is_num(y)) return approx_eq(as_dbl(x), as_dbl(y));
	if(x->type == STRING && y->type == STRING)
		return x->string_size == y->string_size && memcmp(x->string, y->string, x->string_size) == 0;
	return 0;
}

/* structural type equality for the flat-vs-unflat check (types are distinct allocations now) */
static int same_type(const void* a, const void* b) { if(a == NULL || b == NULL) return a == b; return ty_of(a) == ty_of(b); }

/* does the inferred type predict the type of the actually-evaluated value?
 * NULL value is a valid value of any type; an INT value is accepted under FLOAT (numeric widening). */
static int type_predicts_value(const void* inferred, const void* val, const sql_expr_eval_context* ec)
{
	if(val == NULL) return 1;                                   /* NULL is a member of every type */
	if(val == ec->unknown_bool) return 1;                       /* unknown_bool is the NULL/unknown sentinel: type-agnostic */
	int vt_int = 0, vt_flt = 0, vt_str = 0;
	if(val == ec->true_bool || val == ec->false_bool) vt_int = 1;  /* TRUE/FALSE are integer datums */
	else { const datum* d = val; vt_int = (d->type == INTEGER); vt_flt = (d->type == FLOATING); vt_str = (d->type == STRING); }
	if(inferred == NULL) return 0;                              /* inferred bottom but value is definite */
	switch(ty_of(inferred))
	{
		case TY_STRING: return vt_str;
		case TY_INT:    return vt_int;
		case TY_FLOAT:  return vt_flt || vt_int;                /* an int is a valid float */
	}
	return 0;
}

static void fmt_value(char* out, size_t cap, const void* r, const sql_expr_eval_context* ec)
{
	if(r == NULL)             { snprintf(out, cap, "NULL"); return; }
	if(r == ec->true_bool)    { snprintf(out, cap, "TRUE"); return; }
	if(r == ec->false_bool)   { snprintf(out, cap, "FALSE"); return; }
	if(r == ec->unknown_bool) { snprintf(out, cap, "UNKNOWN"); return; }
	const datum* d = r;
	if(d->type == INTEGER)       snprintf(out, cap, "%lld", (long long)d->integer);
	else if(d->type == FLOATING) snprintf(out, cap, "%g", d->floating);
	else                         snprintf(out, cap, "'%.*s'", (int)d->string_size, d->string);
}

/* Reads ONE sql expression from stdin, parses it, and for both the unflattened and
 * flattened forms: infers the type and evaluates it, printing the inferred types and
 * checking that flattening preserves the inferred type and the value. */
int main(void)
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

		if(sqlast1->type == EXPR)
		{
			/* unflattened: infer type, then evaluate */
			int terr1 = 0; void* t1 = infer_type_sql_expr(sqlast1->expr, &ec, &terr1);
			int verr1 = 0; void* v1 = evaluate_sql_expr(sqlast1->expr, &ec, &verr1);

			/* flatten, and print the flattened form */
			flatten_exprs_sql(sqlast1);
			make_dstring_empty(&str1);
			snprint_sql(&str1, sqlast1);
			printf("flattened " printf_dstring_format "\n\n", printf_dstring_params(&str1));

			/* flattened: infer type, then evaluate */
			int terr2 = 0; void* t2 = infer_type_sql_expr(sqlast1->expr, &ec, &terr2);
			int verr2 = 0; void* v2 = evaluate_sql_expr(sqlast1->expr, &ec, &verr2);

			char vs1[256], vs2[256]; fmt_value(vs1, sizeof vs1, v1, &ec); fmt_value(vs2, sizeof vs2, v2, &ec);

			printf("inferred type (unflattened) = %s\n", terr1 ? "TYPE-ERROR" : type_name(t1));
			printf("inferred type (flattened)   = %s\n", terr2 ? "TYPE-ERROR" : type_name(t2));
			printf("result        (unflattened) = %s\n", verr1 ? "ERROR" : vs1);
			printf("result        (flattened)   = %s\n", verr2 ? "ERROR" : vs2);

			int type_ok  = (terr1 == terr2) && (terr1 != 0 || same_type(t1, t2));
			int value_ok = (verr1 == verr2) && (verr1 != 0 || datum_equal(v1, v2));
			printf("error_code: type(unflat=%d flat=%d)  value(unflat=%d flat=%d)\n", terr1, terr2, verr1, verr2);
			printf("type  flat==unflat  : %s\n", type_ok  ? "PASS" : "FAIL");
			printf("value flat==unflat  : %s\n", value_ok ? "PASS" : "FAIL");
			if(terr1 == 0 && verr1 == 0)
				printf("type predicts value : %s\n", type_predicts_value(t1, v1, &ec) ? "PASS" : "FAIL");

			/* free everything via the library's public delete functions */
			delete_data(v1, &ec); delete_data(v2, &ec);
			delete_type(t1, &ec); delete_type(t2, &ec);
		}

		deinit_dstring(&str1);
		delete_sql(sqlast1);
	}
	else
		printf("parse error = %d\n", error1);

	cb_delete_type(ec.bool_type, &ec);   /* free the one persistent (bool) type the context owns */
	deinitialize_stream(&rs);
	return 0;
}