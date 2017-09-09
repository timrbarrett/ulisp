//uLisp.h
#ifndef ULISP_H
#define ULISP_H

//#include <Arduino.h>
#include <setjmp.h>
#include <SPI.h>

// Compile options

#define checkoverflow
// #define resetautorun
#define printfreespace
#define serialmonitor
// #define printgcs

/* must
 * 	my test if my sda scl definitions and associated code work.
 *
 * may
 *   get the flash image save working.
 *
 */

class uLisp {
	
	typedef unsigned int symbol_t;

	typedef struct sobject {
		union {
			struct {
				sobject *car;
				sobject *cdr;
			};
			struct {
				unsigned int type;
				union {
					symbol_t name;
					int integer;
				};
			};
		};
	} object;

	typedef object *(*fn_ptr_type)(object *, object *);
	
	typedef struct {
		const char *string;
		fn_ptr_type fptr;
		int min;
		int max;
	} tbl_entry_t;

	typedef char (*gfun_t)();
	typedef void (*pfun_t)(char);
	
	public:
	
		// setup workspace	
		void initworkspace();
		void initenv();

		void randomseed(long micros);
		void error(PGM_P p);

// C Macros

#define nil                NULL
#define car(x)             (((object *) (x))->car)
#define cdr(x)             (((object *) (x))->cdr)

#define first(x)           (((object *) (x))->car)
#define second(x)          (car(cdr(x)))
#define cddr(x)            (cdr(cdr(x)))
#define third(x)           (car(cdr(cdr(x))))

#define push(x, y)         ((y) = cons((x),(y)))
#define pop(y)             ((y) = cdr(y))

#define numberp(x)         ((x)->type == NUMBER)
#define symbolp(x)         ((x)->type == SYMBOL)
#define stringp(x)         ((x)->type == STRING)
#define characterp(x)      ((x)->type == CHARACTER)
#define streamp(x)         ((x)->type == STREAM)

#define mark(x)            (car(x) = (object *)(((unsigned int)(car(x))) | MARKBIT))
#define unmark(x)          (car(x) = (object *)(((unsigned int)(car(x))) & ~MARKBIT))
#define marked(x)          ((((unsigned int)(car(x))) & MARKBIT) != 0)
#define MARKBIT            1

#define setflag(x)         (Flags = Flags | 1<<(x))
#define clrflag(x)         (Flags = Flags & ~(1<<(x)))
#define tstflag(x)         (Flags & 1<<(x))

// Constants

static const int TRACEMAX = 3; // Number of traced functions
enum type { ZERO=0, SYMBOL=2, NUMBER=4, STREAM=6, CHARACTER=8, STRING=10, PAIR=12 };  // STRING and PAIR must be last
enum token { UNUSED, BRA, KET, QUO, DOT };
enum stream { SERIALSTREAM, I2CSTREAM, SPISTREAM };

enum function { SYMBOLS, NIL, TEE, NOTHING, AMPREST, LAMBDA, LET, LETSTAR, CLOSURE, SPECIAL_FORMS, QUOTE,
DEFUN, DEFVAR, SETQ, LOOP, PUSH, POP, INCF, DECF, SETF, DOLIST, DOTIMES, TRACE, UNTRACE, FORMILLIS,
WITHSERIAL, WITHI2C, WITHSPI, TAIL_FORMS, PROGN, RETURN, IF, COND, WHEN, UNLESS, AND, OR, FUNCTIONS, NOT,
NULLFN, CONS, ATOM, LISTP, CONSP, NUMBERP, SYMBOLP, STREAMP, EQ, CAR, FIRST, CDR, REST, CAAR, CADR,
SECOND, CDAR, CDDR, CAAAR, CAADR, CADAR, CADDR, THIRD, CDAAR, CDADR, CDDAR, CDDDR, LENGTH, LIST, REVERSE,
NTH, ASSOC, MEMBER, APPLY, FUNCALL, APPEND, MAPC, MAPCAR, ADD, SUBTRACT, MULTIPLY, DIVIDE, MOD, ONEPLUS,
ONEMINUS, ABS, RANDOM, MAX, MIN, NUMEQ, LESS, LESSEQ, GREATER, GREATEREQ, NOTEQ, PLUSP, MINUSP, ZEROP,
ODDP, EVENP, CHAR, CHARCODE, CODECHAR, CHARACTERP, STRINGP, STRINGEQ, STRINGLESS, STRINGGREATER, SORT,
STRINGFN, CONCATENATE, SUBSEQ, READFROMSTRING, PRINCTOSTRING, PRIN1TOSTRING, LOGAND, LOGIOR, LOGXOR,
LOGNOT, ASH, LOGBITP, EVAL, GLOBALS, LOCALS, MAKUNBOUND, BREAK, READ, PRIN1, PRINT, PRINC, TERPRI,
READBYTE, READLINE, WRITEBYTE, WRITESTRING, WRITELINE, RESTARTI2C, GC, ROOM, SAVEIMAGE, LOADIMAGE, CLS,
PINMODE, DIGITALREAD, DIGITALWRITE, ANALOGREAD, ANALOGWRITE, DELAY, MILLIS, NOTE, EDIT, PPRINT, ENDFUNCTIONS };


// Workspace - sizes in bytes
#define WORDALIGNED __attribute__((aligned (2)))
#define BUFFERSIZE 18

#if defined(__AVR_ATmega328P__)
#define WORKSPACESIZE 316               /* Cells (4*bytes) */
#define IMAGEDATASIZE 254               /* Cells */
#define SYMBOLTABLESIZE BUFFERSIZE      /* Bytes - no long symbols */

#elif defined(__AVR_ATmega2560__)
#define WORKSPACESIZE 1216              /* Cells (4*bytes) */
#define IMAGEDATASIZE 893               /* Cells */
#define SYMBOLTABLESIZE 512             /* Bytes */

#elif defined(__AVR_ATmega1284P__)
#define WORKSPACESIZE 2816              /* Cells (4*bytes) */
#define IMAGEDATASIZE 893               /* Cells */
#define SYMBOLTABLESIZE 512             /* Bytes */

#else 
	// arduino 101?
#define WORKSPACESIZE 2816              /* Cells (4*bytes) */
#define IMAGEDATASIZE 893               /* Cells */
#define SYMBOLTABLESIZE 512             /* Bytes */

#endif

object Workspace[WORKSPACESIZE] WORDALIGNED;
char SymbolTable[SYMBOLTABLESIZE];

// Global variables

jmp_buf exception;
unsigned int Freespace = 0;
object *Freelist;
char *SymbolTop = SymbolTable;
/*extern*/ uint8_t _end;
unsigned int I2CCount;
unsigned int TraceFn[TRACEMAX];
unsigned int TraceDepth[TRACEMAX];

object *GlobalEnv;
object *GCStack = NULL;
object *GlobalString;
int GlobalStringIndex = 0;
char BreakLevel = 0;
char LastChar = 0;
char LastPrint = 0;
char PrintReadably = 1;

// Flags
enum flag { RETURNFLAG, ESCAPE, EXITEDITOR };
volatile char Flags;
/*

	

 // Forward references
object *tee;
object *tf_progn (object *form, object *env);
object *eval (object *form, object *env);
object *read ();
void repl(object *env);
void printobject (object *form, pfun_t pfun);
char *lookupbuiltin (symbol_t name);
int lookupfn (symbol_t name);
int builtin (char* n);
void Display (char c);
	*/
	
	private:
	
		// ulisp allocations
		object* myalloc();
		void myfree(object* obj);
		
		// make each type of object Lines:196-231
		object* number (int n);
		object *character (char c);
		object *cons (object *arg1, object *arg2);
		object *symbol (symbol_t name);
		object *stream (unsigned char streamtype, unsigned char address);
		
		// garbage collection - Lines:235-282
		void markobject (object *obj);
		void sweep();
		void gc (object *form, object *env);
		
		// compact image lines: - Lines:286-331
		void movepointer (object *from, object *to);
		int compactimage (object **arg);
		
		// Save-image and load-image

		/*
typedef struct {
  unsigned int eval;
  unsigned int datasize;
  unsigned int globalenv;
  unsigned int gcstack;
  #if SYMBOLTABLESIZE > BUFFERSIZE
  unsigned int symboltop;
  char table[SYMBOLTABLESIZE];
  #endif
  object data[IMAGEDATASIZE/4];
} struct_image;

struct_image EEMEM image;

		// save-image and load-image - Lines:335-391
		int saveimage (object *arg);
		int loadimage();
		void autorunimage();
		*/
		// error handling - Lines:395-409
		// error is in public above
		void error2 (object *symbol, PGM_P string);
		
		// tracing - Lines:413-440
		boolean tracing(symbol_t name);
		void trace(symbol_t name);
		void untrace(symbol_t name);
		
		// helper functions - Lines:445-510
		boolean consp (object *x);
		boolean atom (object *x);
		boolean listp (object *x);
		int toradix40 (char ch);
		int fromradix40 (int n);
		int pack40 (char *buffer);
		boolean valid40 (char *buffer);
		int digitvalue (char d);
		char* name (object *obj);
		int integer( object *obj);
		int fromchar(object *obj);
		int istream(object *obj);
		int issymbol(object *obj, symbol_t n);
		int eq(object *arg1, object *arg2);
		int listlength (object *list);
		
	// Association lists - Lines:514-537
		object *assoc (object *key, object *list);
		object *delassoc (object *key, object **alist);
		
	// String utilities - Lines:514-537
		void indent (int spaces);
		void buildstring (char ch, int *chars, object **head);
		object *readstring (char delim, gfun_t gfun);
		int stringlength (object *form);
		char nthchar (object *string, int n);
		
	// Lookup variable in environment
		object *value (symbol_t n, object *env);
		object *findvalue (object *var, object *env);
		object *findtwin (object *var, object *env); 
		void dropframe (int tc, object **env); 

	// Handling closures
		object *closure (object *fname, object *state, object *function, object *args, object **env);
		object *apply (object *function, object *args, object **env);

	// In-place operations
		object **place (object *args, object *env);

	// Checked car and cdr
		inline object *carx (object *arg);
		inline object *cdrx (object *arg);

	// I2C interface
		uint8_t const TWI_SDA_PIN = 18;
		uint8_t const TWI_SCL_PIN = 19;
		uint32_t const F_TWI = 400000L;  // Hardware I2C clock in Hz
		uint8_t const TWSR_MTX_DATA_ACK = 0x28;
		uint8_t const TWSR_MTX_ADR_ACK = 0x18;
		uint8_t const TWSR_MRX_ADR_ACK = 0x40;
		uint8_t const TWSR_START = 0x08;
		uint8_t const TWSR_REP_START = 0x10;
		uint8_t const I2C_READ = 1;
		uint8_t const I2C_WRITE = 0;

		void I2Cinit(bool enablePullup);
		uint8_t I2Cread();
		bool I2Cwrite(uint8_t data);
		bool I2Cstart(uint8_t address, uint8_t read);
		bool I2Crestart(uint8_t address, uint8_t read); 
		void I2Cstop(uint8_t read);
		
		inline char spiread ();
		inline char serial1read ();
		//inline char serial2read ();
		//inline char serial3read ();
		gfun_t gstreamfun (object *args);
		inline void spiwrite (char c);
		inline void serial1write (char c);
		//inline void serial2write (char c) 
		//inline void serial3write (char c)
		pfun_t pstreamfun (object *args);
		
	// Check pins
		void checkanalogread (int pin);
		void checkanalogwrite (int pin);
		
	// Note
		//const uint8_t scale[] PROGMEM = {239,226,213,201,190,179,169,160,151,142,134,127};
		//void playnote (int pin, int note, int octave);
		//void nonote (int pin);
		
	// Special forms
		object *sp_quote (object *args, object *env);
		object *sp_defun (object *args, object *env);
		object *sp_defvar (object *args, object *env); 
		object *sp_setq (object *args, object *env);
		object *sp_loop (object *args, object *env);
		object *sp_push (object *args, object *env);
		object *sp_pop (object *args, object *env);
		object *sp_incf (object *args, object *env);
		object *sp_decf (object *args, object *env);
		object *sp_setf (object *args, object *env);
		object *sp_dolist (object *args, object *env);
		object *sp_dotimes (object *args, object *env);
		object *sp_trace (object *args, object *env);
		object *sp_untrace (object *args, object *env);
		object *sp_formillis (object *args, object *env);
		object *sp_withserial (object *args, object *env);
		object *sp_withi2c (object *args, object *env);
		object *sp_withspi (object *args, object *env);

	// Tail-recursive forms
		object *tf_progn (object *args, object *env);
		object *tf_return (object *args, object *env);
		object *tf_if (object *args, object *env) ;
		object *tf_cond (object *args, object *env) ;
		object *tf_when (object *args, object *env) ;
		object *tf_unless (object *args, object *env);
		object *tf_and (object *args, object *env);
		object *tf_or (object *args, object *env) ;
		
	// Core functions
		object *fn_not (object *args, object *env);
		object *fn_cons (object *args, object *env);
		object *fn_atom (object *args, object *env);
		object *fn_listp (object *args, object *env);
		object *fn_consp (object *args, object *env);
		object *fn_numberp (object *args, object *env);
		object *fn_symbolp (object *args, object *env);
		object *fn_streamp (object *args, object *env);
		object *fn_eq (object *args, object *env);
		
	// List functions
		object *fn_car (object *args, object *env);
		object *fn_cdr (object *args, object *env);
		object *fn_caar (object *args, object *env);
		object *fn_cadr (object *args, object *env);
		object *fn_cdar (object *args, object *env);
		object *fn_cddr (object *args, object *env);
		object *fn_caaar (object *args, object *env);
		object *fn_caadr (object *args, object *env);
		object *fn_cadar (object *args, object *env);
		object *fn_caddr (object *args, object *env);
		object *fn_cdaar (object *args, object *env);
		object *fn_cdadr (object *args, object *env);
		object *fn_cddar (object *args, object *env);
		object *fn_cdddr (object *args, object *env);
		object *fn_length (object *args, object *env);
		object *fn_list (object *args, object *env);
		object *fn_reverse (object *args, object *env);
		object *fn_nth (object *args, object *env);
		object *fn_assoc (object *args, object *env);
		object *fn_member (object *args, object *env);
		object *fn_apply (object *args, object *env);
		object *fn_funcall (object *args, object *env);
		object *fn_append (object *args, object *env);
		object *fn_mapc (object *args, object *env);
		object *fn_mapcar (object *args, object *env);
		
	// Arithmetic functions
		object *fn_add (object *args, object *env);
		object *fn_subtract (object *args, object *env);
		object *fn_multiply (object *args, object *env);
		object *fn_divide (object *args, object *env);
		object *fn_mod (object *args, object *env);
		object *fn_oneplus (object *args, object *env);
		object *fn_oneminus (object *args, object *env);
		object *fn_abs (object *args, object *env);
		object *fn_random (object *args, object *env);
		object *fn_max (object *args, object *env);
		object *fn_min (object *args, object *env);
		
	// Arithmetic comparisons
		object *fn_numeq (object *args, object *env);
		object *fn_less (object *args, object *env);
		object *fn_lesseq (object *args, object *env);
		object *fn_greater (object *args, object *env);
		object *fn_greatereq (object *args, object *env);
		object *fn_noteq (object *args, object *env);
		object *fn_plusp (object *args, object *env);
		object *fn_minusp (object *args, object *env);
		object *fn_zerop (object *args, object *env);
		object *fn_oddp (object *args, object *env);
		object *fn_evenp (object *args, object *env);
		
	// Characters
		object *fn_char (object *args, object *env);
		object *fn_charcode (object *args, object *env);
		object *fn_codechar (object *args, object *env);
		object *fn_characterp (object *args, object *env);
		
	// Strings
		object *fn_stringp (object *args, object *env);
		bool stringcompare (object *args, bool lt, bool gt, bool eq);
		object *fn_stringeq (object *args, object *env);
		object *fn_stringless (object *args, object *env);
		object *fn_stringgreater (object *args, object *env);
		object *fn_sort (object *args, object *env);
		object *fn_stringfn (object *args, object *env);
		object *fn_concatenate (object *args, object *env);
		object *fn_subseq (object *args, object *env);
		char gstr ();
		object *fn_readfromstring (object *args, object *env);
		void pstr (char c);
		object *fn_princtostring (object *args, object *env);
		object *fn_prin1tostring (object *args, object *env);
		
	// Bitwise operators
		object *fn_logand (object *args, object *env);
		object *fn_logior (object *args, object *env);
		object *fn_logxor (object *args, object *env);
		object *fn_lognot (object *args, object *env);
		object *fn_ash (object *args, object *env);
		object *fn_logbitp (object *args, object *env);
		
	// System functions
		object *fn_eval (object *args, object *env);
		object *fn_globals (object *args, object *env);
		object *fn_locals (object *args, object *env);
		object *fn_makunbound (object *args, object *env);
		object *fn_break (object *args, object *env);
		object *fn_read (object *args, object *env);
		object *fn_prin1 (object *args, object *env);

object *fn_print (object *args, object *env);
object *fn_princ (object *args, object *env) ;
object *fn_terpri (object *args, object *env);
object *fn_readbyte (object *args, object *env);
object *fn_readline (object *args, object *env) ;
object *fn_writebyte (object *args, object *env);
object *fn_writestring (object *args, object *env);
object *fn_writeline (object *args, object *env) ;
object *fn_restarti2c (object *args, object *env);
object *fn_gc (object *obj, object *env) ;
object *fn_room (object *args, object *env);
object *fn_saveimage (object *args, object *env) ;
object *fn_loadimage (object *args, object *env) ;
object *fn_cls(object *args, object *env) ;
// Arduino procedures

object *fn_pinmode (object *args, object *env);
object *fn_digitalread (object *args, object *env);
object *fn_digitalwrite (object *args, object *env);

object *fn_analogread (object *args, object *env);
object *fn_analogwrite (object *args, object *env);
object *fn_delay (object *args, object *env);
object *fn_millis (object *args, object *env);
object *fn_note (object *args, object *env);
// Tree Editor

object *fn_edit (object *args, object *env);
object *edit (object *fun);
// Pretty printer

const int PPINDENT = 2;
const int PPWIDTH = 80;

int atomwidth (object *obj);
boolean quoted (object *obj);
int subwidth (object *obj, int w);
int subwidthlist (object *form, int w);
void superprint (object *form, int lm) ;
static const int ppspecials = 8;
const char ppspecial[ppspecials] PROGMEM = { IF, SETQ, TEE, LET, LETSTAR, LAMBDA, WHEN, UNLESS };

void supersub (object *form, int lm, int super) ;
object *fn_pprint (object *args, object *env);
// Insert your own function definitions here

// Built-in procedure names - stored in PROGMEM

const char string0[8] PROGMEM = "symbols";
const char string1[4] PROGMEM = "nil";
const char string2[2] PROGMEM = "t";
const char string3[8] PROGMEM = "nothing";
const char string4[6] PROGMEM = "&rest";
const char string5[7] PROGMEM = "lambda";
const char string6[4] PROGMEM = "let";
const char string7[5] PROGMEM = "let*";
const char string8[8] PROGMEM = "closure";
const char string9[14] PROGMEM = "special_forms";
const char string10[6] PROGMEM = "quote";
const char string11[6] PROGMEM = "defun";
const char string12[7] PROGMEM = "defvar";
const char string13[5] PROGMEM = "setq";
const char string14[5] PROGMEM = "loop";
const char string15[5] PROGMEM = "push";
const char string16[4] PROGMEM = "pop";
const char string17[5] PROGMEM = "incf";
const char string18[5] PROGMEM = "decf";
const char string19[5] PROGMEM = "setf";
const char string20[7] PROGMEM = "dolist";
const char string21[8] PROGMEM = "dotimes";
const char string22[6] PROGMEM = "trace";
const char string23[8] PROGMEM = "untrace";
const char string24[11] PROGMEM = "for-millis";
const char string25[12] PROGMEM = "with-serial";
const char string26[9] PROGMEM = "with-i2c";
const char string27[9] PROGMEM = "with-spi";
const char string28[11] PROGMEM = "tail_forms";
const char string29[6] PROGMEM = "progn";
const char string30[7] PROGMEM = "return";
const char string31[3] PROGMEM = "if";
const char string32[5] PROGMEM = "cond";
const char string33[5] PROGMEM = "when";
const char string34[7] PROGMEM = "unless";
const char string35[4] PROGMEM = "and";
const char string36[3] PROGMEM = "or";
const char string37[10] PROGMEM = "functions";
const char string38[4] PROGMEM = "not";
const char string39[5] PROGMEM = "null";
const char string40[5] PROGMEM = "cons";
const char string41[5] PROGMEM = "atom";
const char string42[6] PROGMEM = "listp";
const char string43[6] PROGMEM = "consp";
const char string44[8] PROGMEM = "numberp";
const char string45[8] PROGMEM = "symbolp";
const char string46[8] PROGMEM = "streamp";
const char string47[3] PROGMEM = "eq";
const char string48[4] PROGMEM = "car";
const char string49[6] PROGMEM = "first";
const char string50[4] PROGMEM = "cdr";
const char string51[5] PROGMEM = "rest";
const char string52[5] PROGMEM = "caar";
const char string53[5] PROGMEM = "cadr";
const char string54[7] PROGMEM = "second";
const char string55[5] PROGMEM = "cdar";
const char string56[5] PROGMEM = "cddr";
const char string57[6] PROGMEM = "caaar";
const char string58[6] PROGMEM = "caadr";
const char string59[6] PROGMEM = "cadar";
const char string60[6] PROGMEM = "caddr";
const char string61[6] PROGMEM = "third";
const char string62[6] PROGMEM = "cdaar";
const char string63[6] PROGMEM = "cdadr";
const char string64[6] PROGMEM = "cddar";
const char string65[6] PROGMEM = "cdddr";
const char string66[7] PROGMEM = "length";
const char string67[5] PROGMEM = "list";
const char string68[8] PROGMEM = "reverse";
const char string69[4] PROGMEM = "nth";
const char string70[6] PROGMEM = "assoc";
const char string71[7] PROGMEM = "member";
const char string72[6] PROGMEM = "apply";
const char string73[8] PROGMEM = "funcall";
const char string74[8] PROGMEM = "append";
const char string75[5] PROGMEM = "mapc";
const char string76[7] PROGMEM = "mapcar";
const char string77[2] PROGMEM = "+";
const char string78[2] PROGMEM = "-";
const char string79[2] PROGMEM = "*";
const char string80[2] PROGMEM = "/";
const char string81[4] PROGMEM = "mod";
const char string82[3] PROGMEM = "1+";
const char string83[3] PROGMEM = "1-";
const char string84[4] PROGMEM = "abs";
const char string85[7] PROGMEM = "random";
const char string86[4] PROGMEM = "max";
const char string87[4] PROGMEM = "min";
const char string88[2] PROGMEM = "=";
const char string89[2] PROGMEM = "<";
const char string90[3] PROGMEM = "<=";
const char string91[2] PROGMEM = ">";
const char string92[3] PROGMEM = ">=";
const char string93[3] PROGMEM = "/=";
const char string94[6] PROGMEM = "plusp";
const char string95[7] PROGMEM = "minusp";
const char string96[6] PROGMEM = "zerop";
const char string97[5] PROGMEM = "oddp";
const char string98[6] PROGMEM = "evenp";
const char string99[5] PROGMEM = "char";
const char string100[10] PROGMEM = "char-code";
const char string101[10] PROGMEM = "code-char";
const char string102[11] PROGMEM = "characterp";
const char string103[8] PROGMEM = "stringp";
const char string104[8] PROGMEM = "string=";
const char string105[8] PROGMEM = "string<";
const char string106[8] PROGMEM = "string>";
const char string107[5] PROGMEM = "sort";
const char string108[7] PROGMEM = "string";
const char string109[12] PROGMEM = "concatenate";
const char string110[7] PROGMEM = "subseq";
const char string111[17] PROGMEM = "read-from-string";
const char string112[18] PROGMEM = "princ-to-string";
const char string113[18] PROGMEM = "prin1-to-string";
const char string114[7] PROGMEM = "logand";
const char string115[7] PROGMEM = "logior";
const char string116[7] PROGMEM = "logxor";
const char string117[7] PROGMEM = "lognot";
const char string118[4] PROGMEM = "ash";
const char string119[8] PROGMEM = "logbitp";
const char string120[5] PROGMEM = "eval";
const char string121[8] PROGMEM = "globals";
const char string122[7] PROGMEM = "locals";
const char string123[11] PROGMEM = "makunbound";
const char string124[6] PROGMEM = "break";
const char string125[5] PROGMEM = "read";
const char string126[6] PROGMEM = "prin1";
const char string127[6] PROGMEM = "print";
const char string128[6] PROGMEM = "princ";
const char string129[7] PROGMEM = "terpri";
const char string130[10] PROGMEM = "read-byte";
const char string131[10] PROGMEM = "read-line";
const char string132[12] PROGMEM = "write-byte";
const char string133[13] PROGMEM = "write-string";
const char string134[11] PROGMEM = "write-line";
const char string135[12] PROGMEM = "restart-i2c";
const char string136[3] PROGMEM = "gc";
const char string137[5] PROGMEM = "room";
const char string138[11] PROGMEM = "save-image";
const char string139[11] PROGMEM = "load-image";
const char string140[4] PROGMEM = "cls";
const char string141[8] PROGMEM = "pinmode";
const char string142[12] PROGMEM = "digitalread";
const char string143[13] PROGMEM = "digitalwrite";
const char string144[11] PROGMEM = "analogread";
const char string145[12] PROGMEM = "analogwrite";
const char string146[6] PROGMEM = "delay";
const char string147[7] PROGMEM = "millis";
const char string148[5] PROGMEM = "note";
const char string149[5] PROGMEM = "edit";
const char string150[7] PROGMEM = "pprint";

const tbl_entry_t lookup_table[152] PROGMEM = {
  { string0, NULL, NIL, NIL },
  { string1, NULL, 0, 0 },
  { string2, NULL, 1, 0 },
  { string3, NULL, 1, 0 },
  { string4, NULL, 1, 0 },
  { string5, NULL, 0, 127 },
  { string6, NULL, 0, 127 },
  { string7, NULL, 0, 127 },
  { string8, NULL, 0, 127 },
  { string9, NULL, NIL, NIL },
  { string10, sp_quote, 1, 1 },
  { string11, sp_defun, 0, 127 },
  { string12, sp_defvar, 2, 2 },
  { string13, sp_setq, 2, 2 },
  { string14, sp_loop, 0, 127 },
  { string15, sp_push, 2, 2 },
  { string16, sp_pop, 1, 1 },
  { string17, sp_incf, 1, 2 },
  { string18, sp_decf, 1, 2 },
  { string19, sp_setf, 2, 2 },
  { string20, sp_dolist, 1, 127 },
  { string21, sp_dotimes, 1, 127 },
  { string22, sp_trace, 0, 1 },
  { string23, sp_untrace, 0, 1 },
  { string24, sp_formillis, 1, 127 },
  { string25, sp_withserial, 1, 127 },
  { string26, sp_withi2c, 1, 127 },
  { string27, sp_withspi, 1, 127 },
  { string28, NULL, NIL, NIL },
  { string29, tf_progn, 0, 127 },
  { string30, tf_return, 0, 127 },
  { string31, tf_if, 2, 3 },
  { string32, tf_cond, 0, 127 },
  { string33, tf_when, 1, 127 },
  { string34, tf_unless, 1, 127 },
  { string35, tf_and, 0, 127 },
  { string36, tf_or, 0, 127 },
  { string37, NULL, NIL, NIL },
  { string38, fn_not, 1, 1 },
  { string39, fn_not, 1, 1 },
  { string40, fn_cons, 2, 2 },
  { string41, fn_atom, 1, 1 },
  { string42, fn_listp, 1, 1 },
  { string43, fn_consp, 1, 1 },
  { string44, fn_numberp, 1, 1 },
  { string45, fn_symbolp, 1, 1 },
  { string46, fn_streamp, 1, 1 },
  { string47, fn_eq, 2, 2 },
  { string48, fn_car, 1, 1 },
  { string49, fn_car, 1, 1 },
  { string50, fn_cdr, 1, 1 },
  { string51, fn_cdr, 1, 1 },
  { string52, fn_caar, 1, 1 },
  { string53, fn_cadr, 1, 1 },
  { string54, fn_cadr, 1, 1 },
  { string55, fn_cdar, 1, 1 },
  { string56, fn_cddr, 1, 1 },
  { string57, fn_caaar, 1, 1 },
  { string58, fn_caadr, 1, 1 },
  { string59, fn_cadar, 1, 1 },
  { string60, fn_caddr, 1, 1 },
  { string61, fn_caddr, 1, 1 },
  { string62, fn_cdaar, 1, 1 },
  { string63, fn_cdadr, 1, 1 },
  { string64, fn_cddar, 1, 1 },
  { string65, fn_cdddr, 1, 1 },
  { string66, fn_length, 1, 1 },
  { string67, fn_list, 0, 127 },
  { string68, fn_reverse, 1, 1 },
  { string69, fn_nth, 2, 2 },
  { string70, fn_assoc, 2, 2 },
  { string71, fn_member, 2, 2 },
  { string72, fn_apply, 2, 127 },
  { string73, fn_funcall, 1, 127 },
  { string74, fn_append, 0, 127 },
  { string75, fn_mapc, 2, 3 },
  { string76, fn_mapcar, 2, 3 },
  { string77, fn_add, 0, 127 },
  { string78, fn_subtract, 1, 127 },
  { string79, fn_multiply, 0, 127 },
  { string80, fn_divide, 2, 127 },
  { string81, fn_mod, 2, 2 },
  { string82, fn_oneplus, 1, 1 },
  { string83, fn_oneminus, 1, 1 },
  { string84, fn_abs, 1, 1 },
  { string85, fn_random, 1, 1 },
  { string86, fn_max, 1, 127 },
  { string87, fn_min, 1, 127 },
  { string88, fn_numeq, 1, 127 },
  { string89, fn_less, 1, 127 },
  { string90, fn_lesseq, 1, 127 },
  { string91, fn_greater, 1, 127 },
  { string92, fn_greatereq, 1, 127 },
  { string93, fn_noteq, 1, 127 },
  { string94, fn_plusp, 1, 1 },
  { string95, fn_minusp, 1, 1 },
  { string96, fn_zerop, 1, 1 },
  { string97, fn_oddp, 1, 1 },
  { string98, fn_evenp, 1, 1 },
  { string99, fn_char, 2, 2 },
  { string100, fn_charcode, 1, 1 },
  { string101, fn_codechar, 1, 1 },
  { string102, fn_characterp, 1, 1 },
  { string103, fn_stringp, 1, 1 },
  { string104, fn_stringeq, 2, 2 },
  { string105, fn_stringless, 2, 2 },
  { string106, fn_stringgreater, 2, 2 },
  { string107, fn_sort, 2, 2 },
  { string108, fn_stringfn, 1, 1 },
  { string109, fn_concatenate, 1, 127 },
  { string110, fn_subseq, 2, 3 },
  { string111, fn_readfromstring, 1, 1 },
  { string112, fn_princtostring, 1, 1 },
  { string113, fn_prin1tostring, 1, 1 },
  { string114, fn_logand, 0, 127 },
  { string115, fn_logior, 0, 127 },
  { string116, fn_logxor, 0, 127 },
  { string117, fn_lognot, 1, 1 },
  { string118, fn_ash, 2, 2 },
  { string119, fn_logbitp, 2, 2 },
  { string120, fn_eval, 1, 1 },
  { string121, fn_globals, 0, 0 },
  { string122, fn_locals, 0, 0 },
  { string123, fn_makunbound, 1, 1 },
  { string124, fn_break, 0, 0 },
  { string125, fn_read, 0, 0 },
  { string126, fn_prin1, 1, 1 },
  { string127, fn_print, 1, 1 },
  { string128, fn_princ, 1, 1 },
  { string129, fn_terpri, 0, 0 },
  { string130, fn_readbyte, 0, 2 },
  { string131, fn_readline, 0, 1 },
  { string132, fn_writebyte, 1, 2 },
  { string133, fn_writestring, 1, 2 },
  { string134, fn_writeline, 1, 2 },
  { string135, fn_restarti2c, 1, 2 },
  { string136, fn_gc, 0, 0 },
  { string137, fn_room, 0, 0 },
  { string138, fn_saveimage, 0, 1 },
  { string139, fn_loadimage, 0, 0 },
  { string140, fn_cls, 0, 0 },
  { string141, fn_pinmode, 2, 2 },
  { string142, fn_digitalread, 1, 1 },
  { string143, fn_digitalwrite, 2, 2 },
  { string144, fn_analogread, 1, 1 },
  { string145, fn_analogwrite, 2, 2 },
  { string146, fn_delay, 1, 1 },
  { string147, fn_millis, 0, 0 },
  { string148, fn_note, 0, 3 },
  { string149, fn_edit, 1, 1 },
  { string150, fn_pprint, 1, 1 }
};

// Table lookup functions

int builtin (char* n);
int longsymbol (char *buffer);
int lookupfn (symbol_t name) ;
int lookupmin (symbol_t name);
int lookupmax (symbol_t name);
char *lookupbuiltin (symbol_t name);
char *lookupsymbol (symbol_t name);
void deletesymbol (symbol_t name);
void testescape ();
// Main evaluator

object *eval (object *form, object *env) ;
// Print functions

void pserial (char c);
const char ControlCodes[250] PROGMEM = "Null\0SOH\0STX\0ETX\0EOT\0ENQ\0ACK\0Bell\0Backspace\0Tab\0Newline\0VT\0"
"Page\0Return\0SO\0SI\0DLE\0DC1\0DC2\0DC3\0DC4\0NAK\0SYN\0ETB\0CAN\0EM\0SUB\0Escape\0FS\0GS\0RS\0US\0Space\0";

void pcharacter (char c, pfun_t pfun) ;
void pstring (char *s, pfun_t pfun);
void printstring (object *form, pfun_t pfun) ;
void pfstring (PGM_P s, pfun_t pfun) ;
void pint (int i, pfun_t pfun) ;
inline void pln (pfun_t pfun);
void pfl (pfun_t pfun);
void printobject(object *form, pfun_t pfun);
// Read functions

char gserial ();
object *nextitem (gfun_t gfun);
object *readrest (gfun_t gfun);
object *read (gfun_t gfun) ;
// Setup

//public void initenv();
// Read/Evaluate/Print loop

void repl (object *env);

};

#endif
