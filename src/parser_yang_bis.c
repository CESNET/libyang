/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include "resolve.h"
#include "common.h"
#include "parser_yang.h"
#include "parser_yang_lex.h"
#include "parser.h"

/* only syntax rules */
#define EXTENSION_ARG 0x01
#define EXTENSION_STA 0x02
#define EXTENSION_DSC 0x04
#define EXTENSION_REF 0x08
#define DISABLE_INHERIT 0
#define ENABLE_INHERIT 0x01

void yyerror(YYLTYPE *yylloc, void *scanner, ...);
/* temporary pointer for the check extension nacm 'data_node' */
/* pointer on the current parsed element 'actual' */



# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser_yang_bis.h".  */
#ifndef YY_YY_PARSER_YANG_BIS_H_INCLUDED
# define YY_YY_PARSER_YANG_BIS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNION_KEYWORD = 258,
    ANYXML_KEYWORD = 259,
    WHITESPACE = 260,
    ERROR = 261,
    EOL = 262,
    STRING = 263,
    STRINGS = 264,
    IDENTIFIER = 265,
    IDENTIFIERPREFIX = 266,
    REVISION_DATE = 267,
    TAB = 268,
    DOUBLEDOT = 269,
    URI = 270,
    INTEGER = 271,
    NON_NEGATIVE_INTEGER = 272,
    ZERO = 273,
    DECIMAL = 274,
    ARGUMENT_KEYWORD = 275,
    AUGMENT_KEYWORD = 276,
    BASE_KEYWORD = 277,
    BELONGS_TO_KEYWORD = 278,
    BIT_KEYWORD = 279,
    CASE_KEYWORD = 280,
    CHOICE_KEYWORD = 281,
    CONFIG_KEYWORD = 282,
    CONTACT_KEYWORD = 283,
    CONTAINER_KEYWORD = 284,
    DEFAULT_KEYWORD = 285,
    DESCRIPTION_KEYWORD = 286,
    ENUM_KEYWORD = 287,
    ERROR_APP_TAG_KEYWORD = 288,
    ERROR_MESSAGE_KEYWORD = 289,
    EXTENSION_KEYWORD = 290,
    DEVIATION_KEYWORD = 291,
    DEVIATE_KEYWORD = 292,
    FEATURE_KEYWORD = 293,
    FRACTION_DIGITS_KEYWORD = 294,
    GROUPING_KEYWORD = 295,
    IDENTITY_KEYWORD = 296,
    IF_FEATURE_KEYWORD = 297,
    IMPORT_KEYWORD = 298,
    INCLUDE_KEYWORD = 299,
    INPUT_KEYWORD = 300,
    KEY_KEYWORD = 301,
    LEAF_KEYWORD = 302,
    LEAF_LIST_KEYWORD = 303,
    LENGTH_KEYWORD = 304,
    LIST_KEYWORD = 305,
    MANDATORY_KEYWORD = 306,
    MAX_ELEMENTS_KEYWORD = 307,
    MIN_ELEMENTS_KEYWORD = 308,
    MODULE_KEYWORD = 309,
    MUST_KEYWORD = 310,
    NAMESPACE_KEYWORD = 311,
    NOTIFICATION_KEYWORD = 312,
    ORDERED_BY_KEYWORD = 313,
    ORGANIZATION_KEYWORD = 314,
    OUTPUT_KEYWORD = 315,
    PATH_KEYWORD = 316,
    PATTERN_KEYWORD = 317,
    POSITION_KEYWORD = 318,
    PREFIX_KEYWORD = 319,
    PRESENCE_KEYWORD = 320,
    RANGE_KEYWORD = 321,
    REFERENCE_KEYWORD = 322,
    REFINE_KEYWORD = 323,
    REQUIRE_INSTANCE_KEYWORD = 324,
    REVISION_KEYWORD = 325,
    REVISION_DATE_KEYWORD = 326,
    RPC_KEYWORD = 327,
    STATUS_KEYWORD = 328,
    SUBMODULE_KEYWORD = 329,
    TYPE_KEYWORD = 330,
    TYPEDEF_KEYWORD = 331,
    UNIQUE_KEYWORD = 332,
    UNITS_KEYWORD = 333,
    USES_KEYWORD = 334,
    VALUE_KEYWORD = 335,
    WHEN_KEYWORD = 336,
    YANG_VERSION_KEYWORD = 337,
    YIN_ELEMENT_KEYWORD = 338,
    ADD_KEYWORD = 339,
    CURRENT_KEYWORD = 340,
    DELETE_KEYWORD = 341,
    DEPRECATED_KEYWORD = 342,
    FALSE_KEYWORD = 343,
    NOT_SUPPORTED_KEYWORD = 344,
    OBSOLETE_KEYWORD = 345,
    REPLACE_KEYWORD = 346,
    SYSTEM_KEYWORD = 347,
    TRUE_KEYWORD = 348,
    UNBOUNDED_KEYWORD = 349,
    USER_KEYWORD = 350
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


  int32_t i;
  uint32_t uint;
  char *str;
  void *v;
  struct lys_module *inc;
  struct yang_type *type;
  union {
    uint32_t index;
    struct lys_node_container *container;
    struct lys_node_anyxml *anyxml;
    struct type_node node;
    struct lys_node_case *cs;
    struct lys_node_grp *grouping;
    struct type_uses uses;
    struct lys_refine *refine;
    struct lys_node_notif *notif;
    struct type_deviation *deviation;
  } nodes;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all);

#endif /* !YY_YY_PARSER_YANG_BIS_H_INCLUDED  */

/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3229

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  335
/* YYNRULES -- Number of rules.  */
#define YYNRULES  730
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1184

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   350

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     104,   105,     2,    96,     2,     2,     2,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    99,
       2,   103,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   101,     2,   102,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    97,     2,    98,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   239,   239,   240,   261,   261,   280,   282,   281,   313,
     326,   313,   335,   336,   337,   338,   341,   354,   341,   365,
     366,   372,   372,   381,   383,   383,   387,   397,   399,   422,
     423,   426,   434,   426,   451,   453,   453,   475,   476,   480,
     481,   483,   495,   506,   495,   515,   517,   518,   519,   520,
     525,   531,   533,   535,   537,   539,   547,   550,   550,   559,
     560,   560,   565,   566,   571,   578,   578,   587,   593,   634,
     637,   638,   639,   640,   641,   642,   643,   649,   650,   651,
     653,   653,   672,   673,   677,   678,   685,   692,   703,   715,
     715,   717,   718,   722,   724,   725,   726,   737,   739,   740,
     739,   743,   744,   745,   746,   763,   763,   772,   773,   778,
     797,   804,   810,   815,   821,   823,   823,   832,   834,   833,
     841,   842,   853,   859,   865,   872,   876,   874,   893,   903,
     903,   910,   913,   913,   928,   929,   935,   941,   946,   952,
     953,   959,   966,   967,   968,   975,   983,   988,   992,  1022,
    1023,  1027,  1028,  1029,  1037,  1050,  1056,  1057,  1075,  1079,
    1089,  1090,  1095,  1110,  1115,  1120,  1125,  1131,  1135,  1145,
    1146,  1151,  1151,  1169,  1170,  1173,  1185,  1195,  1196,  1201,
    1202,  1211,  1217,  1222,  1228,  1240,  1241,  1259,  1264,  1265,
    1270,  1272,  1274,  1278,  1282,  1297,  1297,  1315,  1316,  1318,
    1329,  1339,  1340,  1345,  1346,  1355,  1361,  1366,  1372,  1384,
    1385,  1402,  1404,  1406,  1408,  1410,  1410,  1417,  1418,  1423,
    1443,  1449,  1454,  1459,  1460,  1467,  1470,  1471,  1472,  1473,
    1474,  1475,  1476,  1479,  1479,  1487,  1488,  1493,  1527,  1527,
    1529,  1536,  1536,  1544,  1545,  1551,  1557,  1562,  1567,  1567,
    1572,  1572,  1580,  1580,  1587,  1594,  1587,  1614,  1642,  1642,
    1644,  1651,  1656,  1651,  1666,  1667,  1667,  1675,  1678,  1684,
    1690,  1696,  1701,  1707,  1714,  1707,  1729,  1757,  1757,  1759,
    1766,  1771,  1766,  1781,  1782,  1782,  1790,  1796,  1811,  1826,
    1838,  1844,  1849,  1855,  1862,  1855,  1887,  1929,  1929,  1931,
    1938,  1938,  1946,  1956,  1964,  1970,  1985,  2000,  2012,  2018,
    2023,  2028,  2028,  2036,  2036,  2041,  2041,  2049,  2049,  2060,
    2061,  2065,  2086,  2086,  2088,  2098,  2122,  2130,  2138,  2146,
    2154,  2162,  2162,  2172,  2173,  2176,  2177,  2178,  2179,  2180,
    2183,  2183,  2191,  2192,  2196,  2216,  2216,  2218,  2225,  2231,
    2236,  2241,  2241,  2248,  2248,  2259,  2260,  2264,  2291,  2291,
    2293,  2300,  2300,  2308,  2314,  2320,  2326,  2331,  2337,  2337,
    2353,  2354,  2358,  2393,  2393,  2395,  2402,  2408,  2413,  2418,
    2418,  2426,  2426,  2441,  2441,  2450,  2451,  2456,  2457,  2460,
    2480,  2487,  2511,  2535,  2554,  2573,  2596,  2619,  2624,  2630,
    2639,  2630,  2646,  2647,  2650,  2659,  2650,  2671,  2692,  2692,
    2694,  2701,  2710,  2715,  2720,  2720,  2728,  2728,  2738,  2739,
    2742,  2742,  2753,  2754,  2759,  2787,  2794,  2800,  2805,  2810,
    2810,  2818,  2818,  2823,  2823,  2835,  2835,  2848,  2862,  2848,
    2869,  2889,  2889,  2897,  2897,  2902,  2902,  2912,  2926,  2912,
    2933,  2933,  2943,  2944,  2949,  2976,  2983,  2989,  2994,  2999,
    2999,  3007,  3007,  3012,  3012,  3019,  3028,  3019,  3041,  3060,
    3067,  3074,  3084,  3085,  3087,  3092,  3094,  3095,  3096,  3099,
    3101,  3101,  3107,  3108,  3112,  3134,  3142,  3150,  3166,  3174,
    3181,  3188,  3199,  3211,  3211,  3217,  3218,  3222,  3244,  3252,
    3263,  3273,  3282,  3282,  3288,  3289,  3293,  3298,  3304,  3312,
    3320,  3327,  3334,  3345,  3357,  3357,  3360,  3361,  3365,  3366,
    3371,  3377,  3379,  3380,  3379,  3383,  3384,  3385,  3400,  3402,
    3403,  3402,  3406,  3407,  3408,  3423,  3425,  3427,  3428,  3449,
    3451,  3452,  3453,  3474,  3476,  3477,  3478,  3492,  3492,  3500,
    3501,  3506,  3508,  3509,  3511,  3512,  3514,  3516,  3516,  3525,
    3528,  3528,  3539,  3542,  3552,  3573,  3575,  3576,  3579,  3579,
    3598,  3598,  3607,  3607,  3616,  3619,  3621,  3623,  3624,  3626,
    3628,  3630,  3631,  3633,  3635,  3636,  3638,  3639,  3641,  3643,
    3646,  3650,  3652,  3653,  3655,  3656,  3658,  3660,  3671,  3672,
    3675,  3676,  3688,  3689,  3691,  3692,  3694,  3695,  3701,  3702,
    3705,  3706,  3707,  3733,  3734,  3737,  3738,  3739,  3742,  3742,
    3750,  3752,  3753,  3755,  3756,  3757,  3759,  3760,  3762,  3763,
    3765,  3766,  3768,  3769,  3771,  3772,  3775,  3776,  3779,  3781,
    3782,  3785,  3785,  3794,  3796,  3797,  3798,  3799,  3800,  3801,
    3802,  3804,  3805,  3806,  3807,  3808,  3809,  3810,  3811,  3812,
    3813,  3814,  3815,  3816,  3817,  3818,  3819,  3820,  3821,  3822,
    3823,  3824,  3825,  3826,  3827,  3828,  3829,  3830,  3831,  3832,
    3833,  3834,  3835,  3836,  3837,  3838,  3839,  3840,  3841,  3842,
    3843,  3844,  3845,  3846,  3847,  3848,  3849,  3850,  3851,  3852,
    3853,  3854,  3855,  3856,  3857,  3858,  3859,  3860,  3861,  3862,
    3863,  3864,  3865,  3866,  3867,  3868,  3869,  3870,  3871,  3872,
    3873,  3874,  3875,  3876,  3877,  3878,  3879,  3880,  3881,  3884,
    3893
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNION_KEYWORD", "ANYXML_KEYWORD",
  "WHITESPACE", "ERROR", "EOL", "STRING", "STRINGS", "IDENTIFIER",
  "IDENTIFIERPREFIX", "REVISION_DATE", "TAB", "DOUBLEDOT", "URI",
  "INTEGER", "NON_NEGATIVE_INTEGER", "ZERO", "DECIMAL", "ARGUMENT_KEYWORD",
  "AUGMENT_KEYWORD", "BASE_KEYWORD", "BELONGS_TO_KEYWORD", "BIT_KEYWORD",
  "CASE_KEYWORD", "CHOICE_KEYWORD", "CONFIG_KEYWORD", "CONTACT_KEYWORD",
  "CONTAINER_KEYWORD", "DEFAULT_KEYWORD", "DESCRIPTION_KEYWORD",
  "ENUM_KEYWORD", "ERROR_APP_TAG_KEYWORD", "ERROR_MESSAGE_KEYWORD",
  "EXTENSION_KEYWORD", "DEVIATION_KEYWORD", "DEVIATE_KEYWORD",
  "FEATURE_KEYWORD", "FRACTION_DIGITS_KEYWORD", "GROUPING_KEYWORD",
  "IDENTITY_KEYWORD", "IF_FEATURE_KEYWORD", "IMPORT_KEYWORD",
  "INCLUDE_KEYWORD", "INPUT_KEYWORD", "KEY_KEYWORD", "LEAF_KEYWORD",
  "LEAF_LIST_KEYWORD", "LENGTH_KEYWORD", "LIST_KEYWORD",
  "MANDATORY_KEYWORD", "MAX_ELEMENTS_KEYWORD", "MIN_ELEMENTS_KEYWORD",
  "MODULE_KEYWORD", "MUST_KEYWORD", "NAMESPACE_KEYWORD",
  "NOTIFICATION_KEYWORD", "ORDERED_BY_KEYWORD", "ORGANIZATION_KEYWORD",
  "OUTPUT_KEYWORD", "PATH_KEYWORD", "PATTERN_KEYWORD", "POSITION_KEYWORD",
  "PREFIX_KEYWORD", "PRESENCE_KEYWORD", "RANGE_KEYWORD",
  "REFERENCE_KEYWORD", "REFINE_KEYWORD", "REQUIRE_INSTANCE_KEYWORD",
  "REVISION_KEYWORD", "REVISION_DATE_KEYWORD", "RPC_KEYWORD",
  "STATUS_KEYWORD", "SUBMODULE_KEYWORD", "TYPE_KEYWORD", "TYPEDEF_KEYWORD",
  "UNIQUE_KEYWORD", "UNITS_KEYWORD", "USES_KEYWORD", "VALUE_KEYWORD",
  "WHEN_KEYWORD", "YANG_VERSION_KEYWORD", "YIN_ELEMENT_KEYWORD",
  "ADD_KEYWORD", "CURRENT_KEYWORD", "DELETE_KEYWORD", "DEPRECATED_KEYWORD",
  "FALSE_KEYWORD", "NOT_SUPPORTED_KEYWORD", "OBSOLETE_KEYWORD",
  "REPLACE_KEYWORD", "SYSTEM_KEYWORD", "TRUE_KEYWORD", "UNBOUNDED_KEYWORD",
  "USER_KEYWORD", "'+'", "'{'", "'}'", "';'", "'/'", "'['", "']'", "'='",
  "'('", "')'", "$accept", "start", "string_1", "$@1", "string_2", "$@2",
  "module_stmt", "$@3", "$@4", "module_header_stmts", "submodule_stmt",
  "$@5", "$@6", "submodule_header_stmts", "$@7", "yang_version_stmt",
  "yang_version_arg_str", "$@8", "namespace_stmt", "linkage_stmts",
  "import_stmt", "$@9", "$@10", "tmp_identifier_arg_str", "include_stmt",
  "$@11", "include_end", "revision_date_opt", "revision_date_stmt",
  "belongs_to_stmt", "$@12", "$@13", "prefix_stmt", "meta_stmts",
  "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_stmt", "$@14",
  "revision_end", "$@15", "revision_opt_stmt", "date_arg_str", "$@16",
  "body_stmts", "body_stmt", "extension_stmt", "$@17", "extension_end",
  "extension_opt_stmt", "argument_stmt", "$@18", "argument_end",
  "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@19", "$@20", "status_arg_str", "feature_stmt",
  "$@21", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@22", "identity_end", "$@23", "identity_opt_stmt",
  "base_stmt", "typedef_stmt", "$@24", "typedef_arg_str", "type_stmt",
  "$@25", "type_opt_stmt", "$@26", "type_end", "type_stmtsep",
  "type_body_stmts", "decimal_string_restrictions", "union_spec",
  "fraction_digits_stmt", "fraction_digits_arg_str", "length_stmt",
  "length_arg_str", "length_end", "message_opt_stmt", "pattern_stmt",
  "pattern_arg_str", "pattern_end", "enum_specification", "$@27",
  "enum_stmts", "enum_stmt", "enum_arg_str", "enum_end", "enum_opt_stmt",
  "value_stmt", "integer_value_arg_str", "range_stmt", "range_end",
  "path_stmt", "require_instance_stmt", "require_instance_arg_str",
  "bits_specification", "$@28", "bit_stmts", "bit_stmt", "bit_arg_str",
  "bit_end", "bit_opt_stmt", "position_stmt", "position_value_arg_str",
  "error_message_stmt", "error_app_tag_stmt", "units_stmt", "default_stmt",
  "grouping_stmt", "$@29", "grouping_end", "grouping_opt_stmt",
  "data_def_stmt", "container_stmt", "$@30", "container_end",
  "container_opt_stmt", "$@31", "$@32", "$@33", "$@34", "$@35",
  "leaf_stmt", "$@36", "$@37", "leaf_opt_stmt", "$@38", "$@39", "$@40",
  "$@41", "leaf_list_stmt", "$@42", "$@43", "leaf_list_opt_stmt", "$@44",
  "$@45", "$@46", "$@47", "list_stmt", "$@48", "$@49", "list_opt_stmt",
  "$@50", "$@51", "$@52", "$@53", "$@54", "choice_stmt", "$@55",
  "choice_end", "choice_opt_stmt", "$@56", "$@57", "short_case_case_stmt",
  "short_case_stmt", "case_stmt", "$@58", "case_end", "case_opt_stmt",
  "$@59", "$@60", "anyxml_stmt", "$@61", "anyxml_end", "anyxml_opt_stmt",
  "$@62", "$@63", "uses_stmt", "$@64", "uses_end", "uses_opt_stmt", "$@65",
  "$@66", "$@67", "refine_stmt", "$@68", "refine_end", "refine_arg_str",
  "refine_body_opt_stmts", "uses_augment_stmt", "$@69", "$@70",
  "uses_augment_arg_str", "augment_stmt", "$@71", "$@72",
  "augment_opt_stmt", "$@73", "$@74", "$@75", "augment_arg_str",
  "rpc_stmt", "$@76", "rpc_end", "rpc_opt_stmt", "$@77", "$@78", "$@79",
  "$@80", "input_stmt", "$@81", "$@82", "input_output_opt_stmt", "$@83",
  "$@84", "$@85", "output_stmt", "$@86", "$@87", "notification_stmt",
  "$@88", "notification_end", "notification_opt_stmt", "$@89", "$@90",
  "$@91", "deviation_stmt", "$@92", "$@93", "deviation_opt_stmt",
  "deviation_arg_str", "deviate_body_stmt", "deviate_stmts",
  "deviate_not_supported_stmt", "deviate_add_stmt", "$@94",
  "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt", "$@95",
  "deviate_delete_end", "deviate_delete_opt_stmt", "deviate_replace_stmt",
  "$@96", "deviate_replace_end", "deviate_replace_opt_stmt", "when_stmt",
  "$@97", "when_end", "when_opt_stmt", "config_stmt", "config_read_stmt",
  "$@98", "$@99", "config_arg_str", "mandatory_stmt",
  "mandatory_read_stmt", "$@100", "$@101", "mandatory_arg_str",
  "presence_stmt", "min_elements_stmt", "min_value_arg_str",
  "max_elements_stmt", "max_value_arg_str", "ordered_by_stmt",
  "ordered_by_arg_str", "must_stmt", "$@102", "must_end", "unique_stmt",
  "unique_arg_str", "unique_arg", "key_stmt", "key_arg_str", "$@103",
  "key_opt", "$@104", "range_arg_str", "absolute_schema_nodeid",
  "absolute_schema_nodeids", "absolute_schema_nodeid_opt",
  "descendant_schema_nodeid", "$@105", "path_arg_str", "$@106", "$@107",
  "absolute_path", "absolute_paths", "absolute_path_opt", "relative_path",
  "relative_path_part1", "relative_path_part1_opt", "descendant_path",
  "descendant_path_opt", "path_predicate", "path_equality_expr",
  "path_key_expr", "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@108", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@109", "strings", "identifier", "identifiers",
  "identifiers_ref", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,    43,   123,   125,    59,
      47,    91,    93,    61,    40,    41
};
# endif

#define YYPACT_NINF -838

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-838)))

#define YYTABLE_NINF -587

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -838,    39,  -838,  -838,   242,  -838,  -838,  -838,   113,   113,
    -838,  -838,  3042,  3042,   113,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
     -50,   113,   -28,   113,  -838,  -838,   -21,   313,   313,  -838,
    -838,   -10,  -838,  -838,     2,   462,   113,   113,   113,   113,
    -838,  -838,  -838,  -838,  -838,   130,  -838,  -838,    71,  2398,
    -838,  2490,  3042,    14,   -24,   -24,   113,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,    92,  -838,  -838,    92,  -838,  -838,  -838,
      92,   113,   113,  -838,  -838,   303,   303,  3042,   113,   113,
     113,  -838,  -838,  -838,  -838,  -838,   113,  -838,  -838,  3042,
    3042,   113,   113,   113,   113,  -838,  -838,  -838,  -838,    17,
      17,  -838,  2214,    60,    69,   313,   113,   113,  -838,  -838,
    -838,  2490,  2490,  2490,  2490,   113,  -838,  1145,  1233,    62,
    -838,  -838,   113,  -838,  -838,  -838,    78,   253,    92,    92,
      92,    92,   301,   313,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,   272,   472,   313,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  3042,    16,  3042,  3042,  3042,
      16,  3042,  3042,  3042,  3042,  3042,  3042,  3042,  3042,  3042,
    2582,   113,   313,   113,   335,  -838,  -838,  -838,  -838,   335,
     312,   313,  -838,   293,  -838,  3134,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,    91,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,   113,   113,   113,  -838,   113,   108,  -838,   113,
    -838,  -838,  -838,   330,  -838,   121,   136,   113,   339,   356,
     374,   145,   113,   391,   399,   402,   153,   157,   160,   449,
     465,  -838,   476,   113,   113,   169,  2306,   239,   301,  -838,
     313,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,   313,  -838,  -838,  -838,  -838,  -838,
     204,    92,  -838,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   411,   313,   313,
     113,  -838,  -838,   118,   525,  1382,  1266,   792,   117,    64,
     172,   628,   173,  1392,  1008,   860,  1342,   714,   113,   113,
     113,  -838,  -838,  -838,   223,   250,  -838,  -838,   217,   313,
    -838,  -838,  -838,   113,   113,   113,   113,   113,   113,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,   113,  -838,
    -838,  -838,  -838,  -838,  -838,   241,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,   113,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,   113,  -838,  -838,
    -838,  -838,  -838,   113,  -838,  -838,   260,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,   113,
    -838,  -838,  -838,   265,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,   284,   250,  -838,  -838,  -838,  -838,   113,   113,   113,
    -838,  -838,  -838,  -838,  -838,   291,   250,  -838,  -838,  -838,
    -838,  -838,  -838,   113,   113,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,   298,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    2490,   115,  2490,  -838,   113,  -838,   113,   113,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  2582,  -838,  2490,
    -838,  2490,  -838,  -838,  3042,  -838,  -838,  -838,  -838,  -838,
    -838,  2490,  -838,  -838,  -838,  -838,  -838,  3042,   531,  -838,
     313,   313,   313,  2582,  -838,  -838,  -838,  -838,  -838,    13,
     243,    10,  -838,  -838,  -838,  -838,  2674,  2674,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,   113,   113,  -838,
    -838,  -838,  -838,    92,  -838,  -838,  -838,  -838,    92,    92,
    2582,   313,  2674,  2674,  -838,  -838,  -838,    29,    92,    40,
    -838,   115,  -838,   313,   313,  -838,   313,   313,   313,   313,
     313,    92,   313,   313,   313,   313,   313,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,    92,  -838,
     313,   313,  -838,  -838,  -838,    92,  -838,  -838,  -838,    92,
    -838,  -838,  -838,  -838,  -838,    92,  -838,   313,   313,    92,
    -838,  -838,    92,  -838,    63,  -838,   313,   313,   313,   313,
     313,   313,   313,   313,   300,   304,   313,   313,   313,   313,
    -838,   113,   113,   113,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,   313,   313,   313,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,   494,  -838,   519,   522,  -838,
     551,   113,   113,    63,   113,   113,  -838,   313,   113,  -838,
     113,  -838,   113,   113,   113,  -838,   313,  -838,    63,  -838,
    -838,  -838,  3134,  -838,  -838,  -838,   559,   310,   113,   567,
     113,   113,   113,    92,   113,   113,    92,  -838,  -838,  -838,
      92,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
     580,   587,  -838,   597,  -838,  -838,  3134,    63,   136,   313,
     313,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
     313,  -838,   313,   313,    72,   313,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,   215,   215,   480,
     313,   313,   313,   176,   119,   307,   113,   325,   313,   313,
     313,    63,  -838,  -838,  -838,   334,   345,   113,   113,  -838,
     349,   419,  -838,   396,  -838,  -838,  -838,   424,  1382,   600,
     113,   113,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,    58,  -838,   326,
     267,   536,  -838,  -838,  -838,  -838,  -838,  -838,    47,    59,
    -838,   113,   113,   113,   113,   250,  -838,  -838,  -838,  -838,
     113,  -838,   113,  -838,   359,   113,   113,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  2490,  2490,  -838,
    -838,  -838,  -838,  -838,    92,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,   313,   313,   313,
    -838,    92,   372,   473,  -838,  -838,  -838,    92,    45,  2490,
    2490,  2490,  -838,   313,  2490,   313,  3042,   424,  -838,    29,
      40,   313,    92,    92,   313,   313,   113,   113,  -838,   313,
     313,   313,  -838,  3134,  -838,  -838,   376,  -838,  -838,   113,
     113,  -838,  -838,    92,  -838,   603,  -838,   606,  -838,   609,
    -838,   313,   612,  -838,   396,   613,  -838,  -838,    92,    92,
    -838,  -838,  -838,   372,  -838,  2766,  -838,   113,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,   381,  -838,  -838,  -838,
      82,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     486,  -838,   112,   270,   274,   333,    96,   367,  -838,  3134,
    -838,  -838,  -838,  -838,   113,  -838,  -838,  -838,  -838,  -838,
     113,  -838,  -838,  -838,  -838,  -838,   486,   486,   377,   243,
     390,   403,  -838,  -838,  -838,  -838,    92,  -838,  -838,  -838,
      92,  -838,  -838,   486,  -838,   113,  -838,   113,   426,   486,
    -838,   486,   413,   442,   486,   486,   438,   533,  -838,   486,
    -838,  -838,   457,  2858,   486,  -838,  -838,  -838,  2950,  -838,
     458,   486,  3134,  -838
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     636,     0,     2,     3,     0,     1,   634,   635,     0,     0,
     637,   636,     0,     0,   638,   652,     4,   651,   653,   654,
     655,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     685,   686,   687,   688,   689,   690,   691,   692,   693,   694,
     695,   696,   697,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   713,   714,
     715,   716,   717,   718,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   607,     9,   729,   636,    16,   636,
       0,   606,     0,     6,   615,   615,     5,    12,    19,   636,
     618,    10,   617,   616,    17,     0,   621,     0,     0,     0,
      28,    13,    14,    15,    28,     0,    20,     7,     0,   623,
     622,     0,     0,     0,    46,    46,     0,    22,   636,   636,
     628,   619,   636,   644,   647,   645,   649,   650,   648,   620,
     624,   646,   643,     0,   641,   604,     0,   636,    24,    26,
       0,     0,     0,    29,    30,    55,    55,     0,     8,   630,
     626,   615,   615,    27,   636,    45,   605,   636,    23,     0,
       0,     0,     0,     0,     0,    47,    48,    49,    50,    68,
      68,    42,     0,   625,     0,   613,   642,    25,    31,    34,
      35,     0,     0,     0,     0,     0,   615,     0,     0,     0,
     609,   629,   621,   608,   636,   615,     0,     0,     0,     0,
       0,     0,     0,    56,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     636,   615,    70,    71,    72,    73,    74,    75,   226,   227,
     228,   229,   230,   231,   232,    76,    77,    78,    79,   636,
     615,     0,     0,   614,   615,   615,    37,   615,    52,    53,
      51,    54,    65,    67,    57,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    69,    18,     0,   636,   636,   631,   636,     0,
      39,    36,   636,     0,   353,     0,   419,   404,   566,   636,
     317,   233,    80,   473,   465,   636,   105,   215,   115,   254,
     273,   293,   450,   420,     0,   128,   730,   612,   368,   636,
     636,    43,   630,   632,   627,    32,     0,     0,    40,    66,
     615,    59,    58,     0,   564,     0,   565,   418,     0,     0,
       0,     0,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   615,     0,   610,   611,     0,     0,    39,     0,    38,
      60,   615,   355,   354,   615,   567,   615,   319,   318,   615,
     235,   234,   615,    82,    81,   615,   615,   107,   106,   615,
     217,   216,   615,   117,   116,   615,   615,   615,   615,   452,
     451,   615,   422,   421,   131,   615,   370,   369,   615,   636,
       0,     0,    62,   357,   407,   321,   237,    84,   468,   109,
     219,   120,   257,   276,   296,   454,   424,   126,   372,    44,
     633,   615,    41,     0,     0,   405,     0,     0,     0,   466,
       0,     0,   118,   255,   274,   294,     0,     0,     0,     0,
       0,   137,   138,   136,     0,     0,   134,   135,     0,    33,
      61,    63,    64,     0,     0,     0,     0,     0,     0,   356,
     366,   367,   365,   360,   358,   363,   364,   361,     0,   412,
     413,   411,   410,   414,   416,     0,   408,   320,   329,   330,
     328,   324,   325,   335,   336,   337,   338,   331,   333,   334,
     339,   322,   326,   327,     0,   236,   246,   247,   245,   240,
     250,   248,   252,   238,   244,   243,   241,     0,    83,    87,
      88,    85,    86,     0,   469,   470,     0,   108,   112,   113,
     111,   110,   218,   221,   222,   220,   615,   615,   615,     0,
     123,   124,   122,     0,   121,   271,   272,   270,   260,   264,
     267,     0,     0,   258,   268,   269,   265,     0,     0,     0,
     291,   292,   290,   279,   283,     0,     0,   277,   286,   287,
     288,   289,   284,     0,     0,   309,   310,   308,   299,   311,
     313,   315,     0,   297,   304,   305,   306,   307,   300,   303,
     302,   453,   457,   458,   456,   455,   459,   461,   463,   636,
     636,   423,   427,   428,   426,   425,   429,   431,   433,   435,
       0,     0,     0,   127,     0,   615,     0,     0,   371,   377,
     378,   376,   375,   379,   381,   373,   522,     0,   529,     0,
      98,     0,   615,   615,     0,   615,   615,   406,   615,   615,
     615,     0,   615,   615,   615,   615,   615,     0,     0,   467,
     224,   223,   225,     0,   119,   256,   262,   615,   615,     0,
       0,     0,   275,   281,   615,   615,     0,     0,   615,   615,
     615,   295,   615,   615,   615,   615,   615,   437,   447,   615,
     615,   615,   615,     0,   636,   636,   636,   104,     0,     0,
       0,   133,     0,     0,   615,   615,   615,     0,     0,     0,
     547,     0,   514,   359,   362,   340,   415,   417,   409,   332,
     323,     0,   251,   249,   253,   239,   242,    89,   636,   636,
     636,   636,   471,   636,   474,   476,   478,   477,     0,   615,
     259,   266,   597,   636,   542,     0,   636,   598,   538,     0,
     599,   636,   636,   636,   546,     0,   615,   278,   285,     0,
     556,   557,     0,   551,     0,   568,   312,   314,   316,   298,
     301,   460,   462,   464,     0,     0,   430,   432,   434,   436,
     214,   101,   103,   102,    97,   213,   129,   403,   399,   636,
     388,   383,   636,   380,   382,   374,   636,   636,   527,   523,
     114,   636,   636,   534,   530,     0,    99,     0,     0,   535,
       0,   480,   493,     0,   502,   475,   125,   263,   540,   539,
     541,   536,   537,   545,   544,   543,   282,   559,     0,   553,
     552,   555,     0,   566,   615,   615,     0,     0,   402,     0,
     387,   526,   525,     0,   533,   532,     0,   615,   549,   548,
       0,   615,   516,   515,   615,   342,   341,   615,    91,   615,
       0,     0,   479,     0,   558,   562,     0,     0,   569,   440,
     440,   615,   139,   130,   615,   615,   385,   384,   524,   531,
     162,   100,   518,   344,     0,    90,   615,   482,   481,   615,
     495,   494,   615,   504,   503,   560,   554,   438,   448,   148,
     141,   407,   389,     0,     0,     0,     0,     0,   484,   497,
     506,     0,   441,   443,   445,     0,     0,     0,     0,   145,
       0,   142,   143,     0,   144,   146,   147,     0,   400,     0,
       0,     0,   550,   165,   166,   163,   164,   517,   519,   520,
     343,   349,   350,   348,   347,   351,   345,     0,    92,     0,
       0,     0,   561,   615,   615,   615,   439,   449,   570,     0,
     140,     0,     0,     0,     0,     0,   151,   149,   150,   615,
       0,   615,     0,   197,     0,     0,     0,   386,   397,   398,
     392,   393,   394,   391,   395,   396,   615,     0,     0,   615,
     615,   636,   636,    96,     0,   483,   485,   488,   489,   490,
     491,   492,   615,   487,   496,   498,   501,   615,   500,   505,
     615,   508,   509,   510,   511,   512,   513,   442,   444,   446,
     574,     0,     0,     0,   636,   636,   194,     0,     0,     0,
       0,     0,   615,   152,     0,   173,     0,   196,   401,     0,
       0,   390,     0,     0,   352,   346,    95,    94,    93,   486,
     499,   507,   190,     0,   577,   571,     0,   573,   581,   193,
     192,   191,   157,     0,   636,     0,   159,     0,   168,     0,
     563,   153,     0,   176,   172,     0,   200,   198,     0,     0,
     212,   211,   586,   576,   580,     0,   155,   156,   615,   160,
     615,   615,   169,   615,   615,   188,   187,   615,   177,   175,
     615,   615,   201,   615,   521,   528,   575,   578,   582,   579,
     584,   162,   158,   162,   167,   162,   179,   174,   203,   199,
     639,   583,     0,     0,     0,     0,     0,     0,   640,     0,
     585,   161,   170,   189,     0,   178,   182,   183,   181,   180,
       0,   202,   206,   207,   205,   204,   639,   639,     0,     0,
       0,     0,   602,   603,   600,   186,     0,   636,   601,   210,
       0,   636,   587,   639,   184,   185,   208,   209,     0,   639,
     588,   639,     0,     0,   639,   639,     0,     0,   596,   639,
     589,   592,     0,     0,   639,   593,   594,   591,   639,   590,
       0,   639,     0,   595
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -838,  -838,   468,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,   447,  -838,  -838,  -838,   441,
    -838,  -838,  -838,   388,  -838,  -838,  -838,   207,  -838,  -838,
    -838,  -838,  -201,   443,  -838,  -838,   -25,    87,   416,  -838,
    -838,  -838,  -838,  -838,   209,  -838,   393,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -425,  -355,  -838,
    -838,  -115,  -838,  -838,  -838,  -838,  -402,  -838,  -838,  -838,
    -838,  -838,  -299,  -337,  -838,  -838,  -516,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -837,  -838,  -838,  -838,  -838,  -838,  -838,  -470,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -424,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -432,
    -403,  -303,  -838,  -838,  -838,  -316,   159,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,   168,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,   189,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,   193,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,   203,  -838,
    -838,  -838,  -838,  -838,   213,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -283,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -251,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,
    -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -838,  -385,
    -838,  -838,  -838,  -306,   201,  -838,  -838,  -389,  -242,  -365,
    -838,  -838,  -387,  -259,  -444,  -838,  -436,  -838,   218,  -838,
    -380,  -838,  -838,  -564,  -838,  -177,  -838,  -838,  -838,  -216,
    -838,  -838,  -343,   409,  -130,  -677,  -838,  -838,  -838,  -838,
    -360,  -398,  -838,  -838,  -356,  -838,  -838,  -838,  -379,  -838,
    -838,  -838,  -451,  -838,  -838,  -838,  -645,  -416,  -838,  -838,
    -838,    -6,   -19,  -339,   231,  1139,  -838,  -838,   512,  -838,
    -838,  -838,  -838,   395,  -838,    -4,    -3,   762,   191,  -175,
    -838,   596,    -5,  -109,  -838
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    94,    99,   106,   138,     2,   100,   120,   111,
       3,   102,   124,   114,   125,   121,   160,   177,   122,   134,
     163,   216,   367,   198,   164,   217,   267,   337,   338,   137,
     209,   365,   123,   165,   185,   186,   923,   924,   189,   206,
     303,   342,   412,   433,   274,   302,   207,   241,   242,   350,
     384,   438,   521,   800,   849,   897,   984,   481,   472,   701,
     840,   688,   243,   353,   388,   440,   482,   244,   355,   394,
     543,   442,   544,   245,   454,   324,   615,   826,   427,   455,
     863,   889,   910,   911,   955,   956,  1053,   957,  1055,  1080,
     893,   958,  1057,  1083,   912,   913,  1064,   961,  1062,  1089,
    1116,  1129,  1146,   959,  1086,   914,   915,  1017,   916,   917,
    1027,   963,  1065,  1093,  1117,  1135,  1150,   925,   926,   456,
     457,   246,   354,   391,   441,   247,   248,   349,   381,   437,
     645,   646,   643,   642,   644,   249,   356,   551,   443,   657,
     552,   729,   658,   250,   357,   565,   444,   664,   566,   746,
     665,   251,   358,   582,   445,   672,   673,   668,   669,   670,
     252,   348,   378,   436,   640,   639,   497,   498,   484,   798,
     846,   895,   980,   979,   253,   343,   373,   434,   632,   633,
     254,   362,   407,   458,   696,   694,   695,   623,   829,   867,
     781,   919,   624,   827,   964,   778,   255,   345,   485,   435,
     638,   635,   636,   307,   256,   360,   403,   447,   679,   680,
     681,   682,   608,   764,   905,   887,   943,   944,   945,   609,
     765,   906,   257,   359,   400,   446,   674,   675,   676,   258,
     351,   526,   439,   314,   722,   723,   724,   725,   850,   878,
     939,   726,   851,   881,   940,   727,   853,   884,   941,   486,
     797,   843,   894,   971,   475,   697,   833,   789,   972,   476,
     699,   836,   794,   515,   569,   739,   570,   735,   571,   745,
     477,   795,   839,   589,   753,   820,   590,   750,   818,   854,
     901,  1059,   308,   309,   346,   754,   823,  1011,  1012,  1013,
    1044,  1045,  1073,  1047,  1048,  1075,  1099,  1111,  1096,  1136,
    1160,  1170,  1171,  1173,  1178,  1161,   740,   741,  1147,  1148,
     156,   199,   755,   328,   821,   107,   112,   116,   128,   129,
     149,   193,   141,   192,   297,   113,     4,   130,  1119,   153,
     174,   154,    96,    97,   330
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,   585,   453,   375,    11,    11,    95,    98,    14,   586,
      10,   549,   564,   522,   736,   779,   782,   542,    16,   161,
     162,    16,    16,   157,    16,   -21,   218,   219,   220,   221,
     732,   158,   473,   492,   491,   509,   656,    16,   531,     5,
     550,   548,   563,   578,   595,   605,   117,   104,    16,   474,
     663,   501,   513,    16,   118,    16,   622,   516,   553,   567,
     583,  -572,   732,   556,   572,   588,    16,    16,     6,   105,
       7,   503,   119,   625,     6,   109,     7,     6,   555,     7,
     110,   490,   508,   110,   119,   530,   535,   205,   547,   562,
     577,   594,   604,   331,   101,   182,   103,    10,   335,    10,
     510,   523,   742,   621,   536,   743,   115,   733,   579,   596,
     606,    10,    11,    11,    11,    11,   305,   786,     6,   483,
       7,   512,   787,    16,   151,   538,   151,   182,   791,   581,
     598,   184,    11,   792,   511,   168,   169,   517,   537,   170,
     187,   187,   580,   597,   607,   857,   981,  1014,   182,   182,
     182,   982,  1015,   136,   176,   896,   214,    11,    11,   260,
     171,   191,   172,   184,    10,    10,    10,   215,   139,   449,
     140,   196,    10,   212,   197,   264,  1124,    11,    11,    11,
      11,   329,  -586,  -586,   184,   184,   184,   213,   361,   171,
     449,   172,    10,    10,  1125,   539,   151,   151,   151,   151,
     684,    11,   685,   182,   182,   686,   369,   182,    11,   920,
     921,   262,  1043,  1110,   464,   518,   460,   927,   374,   224,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,   305,   291,   616,   184,
     184,   226,   385,   184,   227,   467,   449,     6,   182,     7,
     395,    16,   188,   188,   396,   231,   293,   397,    10,   464,
     732,   737,   233,   234,  1113,   235,  1114,   408,  1115,   304,
     527,   310,   311,   312,   922,   316,   317,   318,   319,   320,
     321,   322,   323,   325,   184,   617,   344,    10,   698,    10,
     467,   238,   332,   333,   239,   334,     8,   448,   468,   339,
     213,   182,   431,   920,   921,   182,   347,   920,   921,    16,
     336,   224,   352,   272,   728,   618,     9,     6,     6,     7,
       7,   613,   466,   110,   110,   614,   363,   364,    10,    10,
      10,   181,    11,   226,   182,    10,   227,   184,   182,   637,
       6,   184,     7,    10,   574,   450,   110,   212,    10,   464,
     265,   776,   266,   965,   233,   234,   448,   235,   649,    10,
      10,   213,   183,   654,   182,   994,   920,   921,  1121,   295,
     184,   296,  1122,  1054,   184,   993,   998,   966,   557,   558,
     467,   466,   655,   336,   173,    16,   239,   175,   468,   662,
     340,   178,   341,  1142,  1143,  1144,   671,   824,   182,   118,
     184,   825,   451,   574,   450,   930,   430,   864,   461,   470,
     479,   488,   506,   519,   524,   528,   533,   540,   545,   560,
     575,   592,   602,   938,   985,  1000,    10,   371,   960,   372,
    1130,  1123,   946,   619,   184,   683,   376,   689,   377,  1022,
     449,   448,   182,   947,    11,    11,    11,   950,   962,   268,
     269,   270,   271,   379,   700,   380,   702,  1028,   951,    11,
      11,    11,    11,    11,    11,  1131,   711,     6,   952,     7,
     127,   382,  1043,   383,    11,   974,  1074,     6,   184,     7,
     298,   953,  1110,   975,   449,   954,  -132,  1046,   386,   450,
     387,  1118,  1152,   934,  -154,   990,   389,  1005,   390,   392,
      11,   393,   539,   991,  -195,  1006,  1153,   986,   995,  1001,
     936,  1159,  -171,    11,   452,   375,   970,  1164,   329,    11,
     462,   471,   480,   489,   507,   520,   525,   529,   534,   541,
     546,   561,   576,   593,   603,    11,   987,   996,  1002,   976,
     933,   907,  1165,  1168,   329,   620,   398,  1169,   399,   908,
     902,   902,   463,    11,    11,    11,   182,  1174,  1181,   992,
     997,   126,   401,   965,   402,   135,   448,   464,   200,    11,
      11,   904,   904,   405,   410,   406,   465,   411,   166,   935,
     466,   329,   190,   208,   903,   903,   796,   966,   557,   558,
     909,   837,   184,   838,  1090,   493,   677,   678,   467,   152,
     155,   159,   483,  1067,   494,   151,   468,   151,   918,   888,
      11,   614,    11,    11,   450,   718,   841,   719,   842,   844,
     720,   845,   721,   469,   151,   495,   151,   965,   705,   496,
     448,   182,   224,   988,   999,  1003,   151,   502,   514,   499,
    1068,   717,   432,  1069,   554,   568,   584,   751,   847,   500,
     848,   966,   557,   558,   226,   466,   861,   227,   862,   182,
     973,   213,   213,   587,   865,   504,   866,   184,   231,   152,
     152,   152,   152,    10,    10,   233,   234,   876,   235,   877,
     886,   771,   772,   773,   879,   942,   880,   213,   213,   315,
     273,  1128,  1134,   858,   882,   184,   883,   989,   967,  1004,
    1078,   467,  1079,  1081,   238,  1082,  1084,   239,  1085,  1087,
    1091,  1088,  1092,  1097,  1120,   801,   802,   803,   804,  1098,
     805,  1112,  1175,  1151,   261,   150,   532,   366,     0,     0,
     808,     0,     0,   810,     0,     0,     0,     0,   812,   813,
     814,     0,     0,     0,   306,   182,     0,     0,   313,     0,
      11,     0,     0,     0,   231,     0,   464,     0,   327,   599,
       0,     0,     0,     0,     0,     0,     0,    10,    10,    10,
      12,    13,     0,     0,   600,     0,   828,     0,     0,   830,
       0,   184,     0,   831,   832,     0,     0,   467,   834,   835,
     238,     0,     0,     0,     0,     0,   224,    10,    10,    10,
      10,    10,  1032,  1033,    10,     0,    10,     0,    10,    10,
      10,     0,   601,     0,    11,     0,     0,   213,   226,   463,
       0,   227,     0,   182,    10,     0,    10,    10,    10,     0,
      10,    10,   231,     0,   464,     0,   273,   885,     0,   233,
     234,     0,   235,     0,  1056,  1058,  1060,   466,     0,  1063,
       0,   213,     0,    11,     0,     0,     0,   504,     0,   184,
       0,     0,     0,     0,   224,   467,     0,     0,   238,   928,
     931,   239,     0,   468,     0,     0,     0,     0,     0,   131,
     132,   133,     0,     0,     0,     0,   226,   463,     0,   227,
     505,   182,    11,   479,   968,     0,     0,    11,   167,     0,
     231,     0,   464,    11,    11,     0,   573,   233,   234,     0,
     235,     0,   557,   558,   770,   466,    11,    11,   559,   774,
     775,     0,     0,   179,   180,     0,     0,   184,     0,   790,
       0,     0,     0,   467,     0,     0,   238,   574,     0,   239,
       0,   468,   799,   201,   202,   203,   204,    11,    11,    11,
      11,     0,     0,     0,     0,     0,    11,     0,    11,   806,
       0,    11,    11,     0,     0,     0,   809,   222,     0,     0,
     811,     0,   151,   151,     0,     0,   815,     0,  1036,  1037,
     817,   929,   932,   819,     0,     0,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,     0,     0,     0,   480,   969,     0,     0,     0,
       0,  1049,  1050,     0,   151,   151,   151,     0,     0,   151,
    1066,     0,     0,     0,  1072,     0,     0,     0,     0,     0,
       0,     0,    10,    10,   852,   463,     0,     0,   213,   182,
       0,     0,     0,     0,     0,    10,    10,     0,     0,   855,
     464,  1077,     0,     0,     0,     0,  1100,     0,     0,     0,
     557,   558,     0,   466,   868,     0,   559,   869,     0,     0,
     213,   871,     0,    10,     0,   184,     0,     0,   152,   687,
     152,   467,     0,  -280,     0,     0,   450,     0,     0,   468,
       0,  1126,  1132,     0,     0,   327,     0,   152,   368,   152,
    1137,     0,     0,     0,     0,     0,     0,     0,     0,   152,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
      11,   327,     0,     0,     0,     0,    11,   734,   738,   744,
       0,     0,   855,     0,   749,   752,     0,     0,     0,     0,
       0,     0,     0,     0,  1155,     0,     0,     0,  1157,   224,
       0,    10,     0,    10,  1176,     0,     0,     0,   327,  1179,
     777,   780,     0,  1183,     0,   788,   225,   793,   213,   687,
       0,   226,     0,   213,   227,     0,     0,   213,     0,     0,
     228,   229,     0,   230,     0,   231,   232,     0,     0,     0,
       0,     0,   233,   234,     0,   235,     0,     0,     0,     0,
       0,     0,   236,  1127,  1133,     0,     0,     0,     0,     0,
     610,   611,   612,     0,     0,  1038,     0,   237,     0,     0,
       0,   238,     0,     0,   239,   626,   627,   628,   629,   630,
     631,     0,     0,     0,     0,     0,     0,   224,     0,     0,
     634,     0,  1042,   240,   108,     0,     0,     0,  1051,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,   226,
       0,     0,   227,  1070,  1071,     0,   641,     0,   228,   229,
     224,   230,     0,   231,   232,     0,     0,     0,     0,   647,
     233,   234,     0,   235,  1076,   648,     0,     0,     0,     0,
     236,   478,     0,   463,     0,   227,   448,   182,     0,  1094,
    1095,   653,     0,     0,     0,   237,     0,     0,   464,   238,
     194,   195,   239,   233,   234,     0,   235,   465,     0,   659,
     660,   661,     0,     0,     0,     0,     0,  1140,  1141,     0,
       0,   259,     0,   184,     0,   666,   667,     0,     0,   467,
       0,     0,     0,     0,  1158,   223,   224,   468,     0,     0,
    1162,     0,  1163,     0,   263,  1166,  1167,     0,     0,     0,
    1172,     0,     0,     0,   487,  1177,     0,     0,   226,  1180,
       0,   227,  1182,   182,     0,     0,   690,  1154,   692,   693,
     292,  1156,   231,     0,   464,     0,   224,     0,     0,   233,
     234,     0,   235,     0,     0,     0,     0,     0,     0,   294,
       0,     0,     0,   299,   300,   983,   301,   478,   226,   184,
       0,   227,     0,   182,     0,   467,  1010,  1016,   238,   463,
       0,   239,   448,   182,   464,     0,     0,     0,     0,   233,
     234,     0,   235,     0,   464,     0,     0,     0,     0,     0,
     591,     0,     0,   465,     0,   152,   152,   466,     0,   184,
       0,     0,     0,     0,     0,   449,     0,     0,     0,   184,
       0,   239,     0,   468,     0,   467,     0,  -261,     0,     0,
     450,     0,     0,   468,     0,     0,     0,     0,     0,   370,
       0,     0,     0,     0,     0,     0,  1052,   152,   152,   152,
       0,     0,   152,     0,     0,     0,     0,   788,   793,     0,
     404,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     413,     0,     0,   414,     0,   415,   822,     0,   416,     0,
       0,   417,     0,     0,   418,   419,     0,     0,   420,     0,
       0,   421,     0,     0,   422,   423,   424,   425,     0,     0,
     426,     0,     0,     0,   428,     0,     0,   429,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     459,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     856,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1145,  1149,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   822,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   937,     0,
       0,     0,     0,   856,     0,     0,     0,     0,     0,   948,
     949,     0,     0,     0,     0,   650,   651,   652,     0,     0,
       0,     0,   977,   978,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1018,  1019,  1020,  1021,     0,     0,     0,
       0,     0,  1024,     0,  1026,     0,     0,  1029,  1030,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   691,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   703,   704,     0,   706,   707,     0,   708,   709,   710,
       0,   712,   713,   714,   715,   716,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   730,   731,     0,     0,
       0,     0,     0,   747,   748,     0,     0,   756,   757,   758,
       0,   759,   760,   761,   762,   763,     0,     0,   766,   767,
     768,   769,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   783,   784,   785,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   807,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   816,  1138,     0,     0,     0,
       0,     0,  1139,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   859,   860,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   870,     0,     0,     0,
     872,     0,     0,   873,     0,     0,   874,     0,   875,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     890,     0,     0,   891,   892,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   898,     0,     0,   899,     0,
       0,   900,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1007,  1008,  1009,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1023,     0,
    1025,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1031,     0,     0,  1034,  1035,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1039,     0,     0,     0,     0,  1040,     0,     0,  1041,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1061,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1101,    15,  1102,
    1103,     0,  1104,  1105,    17,   210,  1106,     0,     0,  1107,
    1108,     0,  1109,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      15,     0,   211,     0,     0,     0,    17,   210,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    15,     0,   409,     0,   142,   143,    17,   144,
     145,     0,     0,     0,   146,   147,   148,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    15,     0,     0,     0,    16,   143,
      17,   144,   145,     0,     0,     0,   146,   147,   148,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    15,     0,     0,     0,
      16,     0,    17,   326,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    15,     0,
       0,     0,    16,     0,    17,   210,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      15,     0,     0,     0,     0,     0,    17,   210,     0,     0,
    1046,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    15,     0,     0,     0,     0,     0,    17,   210,
       0,     0,  1169,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    15,  1118,     0,     0,     0,     0,
      17,   210,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    15,     0,     0,     0,
      16,     0,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    15,     0,
       0,     0,     0,     0,    17,   210,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93
};

static const yytype_int16 yycheck[] =
{
       4,   445,   427,   346,     8,     9,    12,    13,    11,   445,
      14,   443,   444,   438,   659,   692,   693,   442,     8,    43,
      44,     8,     8,   132,     8,    23,   201,   202,   203,   204,
      17,    17,   434,   436,   436,   437,   552,     8,   440,     0,
     443,   443,   444,   445,   446,   447,    56,    97,     8,   434,
     566,   436,   437,     8,    64,     8,   458,   437,   443,   444,
     445,    14,    17,   443,   444,   445,     8,     8,     5,    97,
       7,   436,    82,   458,     5,    96,     7,     5,   443,     7,
      11,   436,   437,    11,    82,   440,   441,    70,   443,   444,
     445,   446,   447,   294,    97,    31,    99,   101,   299,   103,
     437,    37,    92,   458,   441,    95,   109,    94,   445,   446,
     447,   115,   116,   117,   118,   119,   100,    88,     5,   435,
       7,   437,    93,     8,   129,   441,   131,    31,    88,   445,
     446,    67,   136,    93,   437,   138,   139,    20,   441,   142,
     165,   166,   445,   446,   447,   822,    88,    88,    31,    31,
      31,    93,    93,    23,   157,    83,    96,   161,   162,    97,
      97,   167,    99,    67,   168,   169,   170,    98,    97,    73,
      99,   174,   176,   192,   177,    97,    80,   181,   182,   183,
     184,   290,   100,   101,    67,    67,    67,   192,    97,    97,
      73,    99,   196,   197,    98,    22,   201,   202,   203,   204,
      85,   205,    87,    31,    31,    90,    98,    31,   212,    33,
      34,   214,   100,   101,    42,    98,    98,    98,    97,     4,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   100,   240,    21,    67,
      67,    26,    97,    67,    29,    73,    73,     5,    31,     7,
      97,     8,   165,   166,    97,    40,   259,    97,   262,    42,
      17,    18,    47,    48,  1101,    50,  1103,    98,  1105,   275,
      98,   277,   278,   279,    98,   281,   282,   283,   284,   285,
     286,   287,   288,   289,    67,    68,   305,   291,   627,   293,
      73,    76,   295,   296,    79,   298,    54,    30,    81,   302,
     305,    31,    98,    33,    34,    31,   309,    33,    34,     8,
      71,     4,   315,    12,   653,    98,    74,     5,     5,     7,
       7,    98,    55,    11,    11,    75,   329,   330,   332,   333,
     334,    28,   336,    26,    31,   339,    29,    67,    31,    98,
       5,    67,     7,   347,    77,    78,    11,   366,   352,    42,
      97,   690,    99,    27,    47,    48,    30,    50,    98,   363,
     364,   366,    59,    98,    31,    98,    33,    34,    98,    97,
      67,    99,    98,  1018,    67,   939,   940,    51,    52,    53,
      73,    55,    98,    71,   153,     8,    79,   156,    81,    98,
      97,   160,    99,    16,    17,    18,    98,    97,    31,    64,
      67,    97,   427,    77,    78,    98,   409,    97,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,    98,    98,   941,   430,    97,    32,    99,
      63,    98,    98,   458,    67,   610,    97,   612,    99,   955,
      73,    30,    31,    98,   448,   449,   450,    98,    24,   218,
     219,   220,   221,    97,   629,    99,   631,    98,    39,   463,
     464,   465,   466,   467,   468,    98,   641,     5,    49,     7,
       8,    97,   100,    99,   478,   919,   100,     5,    67,     7,
       8,    62,   101,   919,    73,    66,    75,    14,    97,    78,
      99,     5,   102,   895,    75,   939,    97,   941,    99,    97,
     504,    99,    22,   939,    24,   941,   103,   939,   940,   941,
     895,    85,    32,   517,   427,   858,   919,   104,   627,   523,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   539,   939,   940,   941,   919,
     895,    61,   100,   105,   653,   458,    97,    14,    99,    69,
     887,   888,    27,   557,   558,   559,    31,   100,   100,   939,
     940,   114,    97,    27,    99,   124,    30,    42,   180,   573,
     574,   887,   888,    97,   367,    99,    51,   368,   135,   895,
      55,   690,   166,   190,   887,   888,   701,    51,    52,    53,
     889,    97,    67,    99,  1064,   436,   599,   600,    73,   131,
     132,   133,   918,  1027,   436,   610,    81,   612,   891,   860,
     614,    75,   616,   617,    78,    84,    97,    86,    99,    97,
      89,    99,    91,    98,   629,   436,   631,    27,   634,   436,
      30,    31,     4,   939,    98,   941,   641,   436,   437,   436,
    1029,   647,   411,  1030,   443,   444,   445,   666,    97,   436,
      99,    51,    52,    53,    26,    55,    97,    29,    99,    31,
     919,   666,   667,   445,    97,    65,    99,    67,    40,   201,
     202,   203,   204,   677,   678,    47,    48,    97,    50,    99,
     857,   684,   685,   686,    97,   901,    99,   692,   693,   280,
     222,  1116,  1117,   823,    97,    67,    99,   939,    98,   941,
      97,    73,    99,    97,    76,    99,    97,    79,    99,    97,
      97,    99,    99,  1073,  1112,   718,   719,   720,   721,  1075,
     723,  1100,  1173,  1139,   212,   129,    98,   332,    -1,    -1,
     733,    -1,    -1,   736,    -1,    -1,    -1,    -1,   741,   742,
     743,    -1,    -1,    -1,   276,    31,    -1,    -1,   280,    -1,
     754,    -1,    -1,    -1,    40,    -1,    42,    -1,   290,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   771,   772,   773,
       8,     9,    -1,    -1,    60,    -1,   779,    -1,    -1,   782,
      -1,    67,    -1,   786,   787,    -1,    -1,    73,   791,   792,
      76,    -1,    -1,    -1,    -1,    -1,     4,   801,   802,   803,
     804,   805,   977,   978,   808,    -1,   810,    -1,   812,   813,
     814,    -1,    98,    -1,   818,    -1,    -1,   822,    26,    27,
      -1,    29,    -1,    31,   828,    -1,   830,   831,   832,    -1,
     834,   835,    40,    -1,    42,    -1,   368,   856,    -1,    47,
      48,    -1,    50,    -1,  1019,  1020,  1021,    55,    -1,  1024,
      -1,   856,    -1,   857,    -1,    -1,    -1,    65,    -1,    67,
      -1,    -1,    -1,    -1,     4,    73,    -1,    -1,    76,   894,
     895,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,   117,
     118,   119,    -1,    -1,    -1,    -1,    26,    27,    -1,    29,
      98,    31,   896,   918,   919,    -1,    -1,   901,   136,    -1,
      40,    -1,    42,   907,   908,    -1,    46,    47,    48,    -1,
      50,    -1,    52,    53,   683,    55,   920,   921,    58,   688,
     689,    -1,    -1,   161,   162,    -1,    -1,    67,    -1,   698,
      -1,    -1,    -1,    73,    -1,    -1,    76,    77,    -1,    79,
      -1,    81,   711,   181,   182,   183,   184,   951,   952,   953,
     954,    -1,    -1,    -1,    -1,    -1,   960,    -1,   962,   728,
      -1,   965,   966,    -1,    -1,    -1,   735,   205,    -1,    -1,
     739,    -1,   977,   978,    -1,    -1,   745,    -1,   981,   982,
     749,   894,   895,   752,    -1,    -1,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,    -1,    -1,    -1,   918,   919,    -1,    -1,    -1,
      -1,  1014,  1015,    -1,  1019,  1020,  1021,    -1,    -1,  1024,
    1026,    -1,    -1,    -1,  1043,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1036,  1037,   803,    27,    -1,    -1,  1043,    31,
      -1,    -1,    -1,    -1,    -1,  1049,  1050,    -1,    -1,   818,
      42,  1054,    -1,    -1,    -1,    -1,  1075,    -1,    -1,    -1,
      52,    53,    -1,    55,   833,    -1,    58,   836,    -1,    -1,
    1075,   840,    -1,  1077,    -1,    67,    -1,    -1,   610,   611,
     612,    73,    -1,    75,    -1,    -1,    78,    -1,    -1,    81,
      -1,  1116,  1117,    -1,    -1,   627,    -1,   629,   336,   631,
    1119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   641,
      -1,    -1,    -1,    -1,  1119,    -1,    -1,    -1,    -1,    -1,
    1124,   653,    -1,    -1,    -1,    -1,  1130,   659,   660,   661,
      -1,    -1,   901,    -1,   666,   667,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1147,    -1,    -1,    -1,  1151,     4,
      -1,  1155,    -1,  1157,  1173,    -1,    -1,    -1,   690,  1178,
     692,   693,    -1,  1182,    -1,   697,    21,   699,  1173,   701,
      -1,    26,    -1,  1178,    29,    -1,    -1,  1182,    -1,    -1,
      35,    36,    -1,    38,    -1,    40,    41,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    57,  1116,  1117,    -1,    -1,    -1,    -1,    -1,
     448,   449,   450,    -1,    -1,   984,    -1,    72,    -1,    -1,
      -1,    76,    -1,    -1,    79,   463,   464,   465,   466,   467,
     468,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
     478,    -1,  1011,    98,   105,    -1,    -1,    -1,  1017,    -1,
      -1,    -1,    -1,    -1,    21,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    29,  1032,  1033,    -1,   504,    -1,    35,    36,
       4,    38,    -1,    40,    41,    -1,    -1,    -1,    -1,   517,
      47,    48,    -1,    50,  1053,   523,    -1,    -1,    -1,    -1,
      57,    25,    -1,    27,    -1,    29,    30,    31,    -1,  1068,
    1069,   539,    -1,    -1,    -1,    72,    -1,    -1,    42,    76,
     171,   172,    79,    47,    48,    -1,    50,    51,    -1,   557,
     558,   559,    -1,    -1,    -1,    -1,    -1,  1136,  1137,    -1,
      -1,    98,    -1,    67,    -1,   573,   574,    -1,    -1,    73,
      -1,    -1,    -1,    -1,  1153,   206,     4,    81,    -1,    -1,
    1159,    -1,  1161,    -1,   215,  1164,  1165,    -1,    -1,    -1,
    1169,    -1,    -1,    -1,    98,  1174,    -1,    -1,    26,  1178,
      -1,    29,  1181,    31,    -1,    -1,   614,  1146,   616,   617,
     241,  1150,    40,    -1,    42,    -1,     4,    -1,    -1,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,   260,
      -1,    -1,    -1,   264,   265,   937,   267,    25,    26,    67,
      -1,    29,    -1,    31,    -1,    73,   948,   949,    76,    27,
      -1,    79,    30,    31,    42,    -1,    -1,    -1,    -1,    47,
      48,    -1,    50,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    51,    -1,   977,   978,    55,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    67,
      -1,    79,    -1,    81,    -1,    73,    -1,    75,    -1,    -1,
      78,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,   340,
      -1,    -1,    -1,    -1,    -1,    -1,  1018,  1019,  1020,  1021,
      -1,    -1,  1024,    -1,    -1,    -1,    -1,  1029,  1030,    -1,
     361,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     371,    -1,    -1,   374,    -1,   376,   754,    -1,   379,    -1,
      -1,   382,    -1,    -1,   385,   386,    -1,    -1,   389,    -1,
      -1,   392,    -1,    -1,   395,   396,   397,   398,    -1,    -1,
     401,    -1,    -1,    -1,   405,    -1,    -1,   408,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     431,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     818,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1138,  1139,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   857,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   896,    -1,
      -1,    -1,    -1,   901,    -1,    -1,    -1,    -1,    -1,   907,
     908,    -1,    -1,    -1,    -1,   536,   537,   538,    -1,    -1,
      -1,    -1,   920,   921,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   951,   952,   953,   954,    -1,    -1,    -1,
      -1,    -1,   960,    -1,   962,    -1,    -1,   965,   966,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   615,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   632,   633,    -1,   635,   636,    -1,   638,   639,   640,
      -1,   642,   643,   644,   645,   646,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   657,   658,    -1,    -1,
      -1,    -1,    -1,   664,   665,    -1,    -1,   668,   669,   670,
      -1,   672,   673,   674,   675,   676,    -1,    -1,   679,   680,
     681,   682,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   694,   695,   696,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   729,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   746,  1124,    -1,    -1,    -1,
      -1,    -1,  1130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   824,   825,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   837,    -1,    -1,    -1,
     841,    -1,    -1,   844,    -1,    -1,   847,    -1,   849,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     861,    -1,    -1,   864,   865,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   876,    -1,    -1,   879,    -1,
      -1,   882,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   943,   944,   945,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   959,    -1,
     961,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   976,    -1,    -1,   979,   980,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   992,    -1,    -1,    -1,    -1,   997,    -1,    -1,  1000,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1022,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1078,     4,  1080,
    1081,    -1,  1083,  1084,    10,    11,  1087,    -1,    -1,  1090,
    1091,    -1,  1093,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       4,    -1,    98,    -1,    -1,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     4,    -1,    98,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    16,    17,    18,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     4,    -1,    -1,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     4,    -1,    -1,    -1,
       8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,     4,    -1,
      -1,    -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     4,     5,    -1,    -1,    -1,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     4,    -1,    -1,    -1,
       8,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,     4,    -1,
      -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   107,   112,   116,   432,     0,     5,     7,    54,    74,
     431,   431,   433,   433,   432,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   108,   417,   438,   439,   417,   109,
     113,   432,   117,   432,    97,    97,   110,   421,   421,    96,
      11,   115,   422,   431,   119,   432,   423,    56,    64,    82,
     114,   121,   124,   138,   118,   120,   121,     8,   424,   425,
     433,   433,   433,   433,   125,   125,    23,   135,   111,    97,
      99,   428,     8,     9,    11,    12,    16,    17,    18,   426,
     437,   438,   108,   435,   437,   108,   416,   439,    17,   108,
     122,    43,    44,   126,   130,   139,   139,   433,   432,   432,
     432,    97,    99,   420,   436,   420,   432,   123,   420,   433,
     433,    28,    31,    59,    67,   140,   141,   142,   143,   144,
     144,   417,   429,   427,   421,   421,   432,   432,   129,   417,
     129,   433,   433,   433,   433,    70,   145,   152,   152,   136,
      11,    98,   418,   438,    96,    98,   127,   131,   435,   435,
     435,   435,   433,   421,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    76,    79,
      98,   153,   154,   168,   173,   179,   227,   231,   232,   241,
     249,   257,   266,   280,   286,   302,   310,   328,   335,    98,
      97,   424,   432,   421,    97,    97,    99,   132,   420,   420,
     420,   420,    12,   108,   150,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   432,   421,   432,   421,    97,    99,   430,     8,   421,
     421,   421,   151,   146,   417,   100,   108,   309,   388,   389,
     417,   417,   417,   108,   339,   389,   417,   417,   417,   417,
     417,   417,   417,   417,   181,   417,    11,   108,   419,   439,
     440,   138,   432,   432,   432,   138,    71,   133,   134,   432,
      97,    99,   147,   281,   418,   303,   390,   432,   267,   233,
     155,   336,   432,   169,   228,   174,   242,   250,   258,   329,
     311,    97,   287,   432,   432,   137,   429,   128,   433,    98,
     421,    97,    99,   282,    97,   388,    97,    99,   268,    97,
      99,   234,    97,    99,   156,    97,    97,    99,   170,    97,
      99,   229,    97,    99,   175,    97,    97,    97,    97,    99,
     330,    97,    99,   312,   421,    97,    99,   288,    98,    98,
     133,   150,   148,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   184,   421,   421,
     432,    98,   420,   149,   283,   305,   269,   235,   157,   338,
     171,   230,   177,   244,   252,   260,   331,   313,    30,    73,
      78,   142,   143,   163,   180,   185,   225,   226,   289,   421,
      98,   142,   143,    27,    42,    51,    55,    73,    81,    98,
     142,   143,   164,   172,   355,   360,   365,   376,    25,   142,
     143,   163,   172,   231,   274,   304,   355,    98,   142,   143,
     164,   172,   226,   232,   241,   249,   257,   272,   273,   274,
     280,   355,   360,   365,    65,    98,   142,   143,   164,   172,
     179,   227,   231,   355,   360,   369,   376,    20,    98,   142,
     143,   158,   163,    37,   142,   143,   337,    98,   142,   143,
     164,   172,    98,   142,   143,   164,   179,   227,   231,    22,
     142,   143,   163,   176,   178,   142,   143,   164,   172,   225,
     226,   243,   246,   355,   360,   365,   376,    52,    53,    58,
     142,   143,   164,   172,   225,   251,   254,   355,   360,   370,
     372,   374,   376,    46,    77,   142,   143,   164,   172,   179,
     227,   231,   259,   355,   360,   370,   372,   374,   376,   379,
     382,    98,   142,   143,   164,   172,   179,   227,   231,    45,
      60,    98,   142,   143,   164,   172,   179,   227,   318,   325,
     433,   433,   433,    98,    75,   182,    21,    68,    98,   142,
     143,   164,   172,   293,   298,   355,   433,   433,   433,   433,
     433,   433,   284,   285,   433,   307,   308,    98,   306,   271,
     270,   433,   239,   238,   240,   236,   237,   433,   433,    98,
     421,   421,   421,   433,    98,    98,   182,   245,   248,   433,
     433,   433,    98,   182,   253,   256,   433,   433,   263,   264,
     265,    98,   261,   262,   332,   333,   334,   432,   432,   314,
     315,   316,   317,   435,    85,    87,    90,   108,   167,   435,
     433,   421,   433,   433,   291,   292,   290,   361,   419,   366,
     435,   165,   435,   421,   421,   417,   421,   421,   421,   421,
     421,   435,   421,   421,   421,   421,   421,   417,    84,    86,
      89,    91,   340,   341,   342,   343,   347,   351,   419,   247,
     421,   421,    17,    94,   108,   373,   412,    18,   108,   371,
     412,   413,    92,    95,   108,   375,   255,   421,   421,   108,
     383,   418,   108,   380,   391,   418,   421,   421,   421,   421,
     421,   421,   421,   421,   319,   326,   421,   421,   421,   421,
     420,   432,   432,   432,   420,   420,   419,   108,   301,   391,
     108,   296,   391,   421,   421,   421,    88,    93,   108,   363,
     420,    88,    93,   108,   368,   377,   167,   356,   275,   420,
     159,   432,   432,   432,   432,   432,   420,   421,   432,   420,
     432,   420,   432,   432,   432,   420,   421,   420,   384,   420,
     381,   420,   433,   392,    97,    97,   183,   299,   432,   294,
     432,   432,   432,   362,   432,   432,   367,    97,    99,   378,
     166,    97,    99,   357,    97,    99,   276,    97,    99,   160,
     344,   348,   420,   352,   385,   420,   433,   391,   390,   421,
     421,    97,    99,   186,    97,    97,    99,   295,   420,   420,
     421,   420,   421,   421,   421,   421,    97,    99,   345,    97,
      99,   349,    97,    99,   353,   418,   381,   321,   321,   187,
     421,   421,   421,   196,   358,   277,    83,   161,   421,   421,
     421,   386,   179,   227,   231,   320,   327,    61,    69,   178,
     188,   189,   200,   201,   211,   212,   214,   215,   305,   297,
      33,    34,    98,   142,   143,   223,   224,    98,   142,   143,
      98,   142,   143,   164,   172,   231,   355,   433,    98,   346,
     350,   354,   385,   322,   323,   324,    98,    98,   433,   433,
      98,    39,    49,    62,    66,   190,   191,   193,   197,   209,
      32,   203,    24,   217,   300,    27,    51,    98,   142,   143,
     226,   359,   364,   369,   370,   372,   376,   433,   433,   279,
     278,    88,    93,   108,   162,    98,   225,   226,   359,   364,
     370,   372,   376,   379,    98,   225,   226,   376,   379,    98,
     182,   225,   226,   359,   364,   370,   372,   421,   421,   421,
     108,   393,   394,   395,    88,    93,   108,   213,   433,   433,
     433,   433,   182,   421,   433,   421,   433,   216,    98,   433,
     433,   421,   435,   435,   421,   421,   432,   432,   420,   421,
     421,   421,   420,   100,   396,   397,    14,   399,   400,   432,
     432,   420,   108,   192,   412,   194,   435,   198,   435,   387,
     435,   421,   204,   435,   202,   218,   417,   217,   363,   368,
     420,   420,   418,   398,   100,   401,   420,   432,    97,    99,
     195,    97,    99,   199,    97,    99,   210,    97,    99,   205,
     203,    97,    99,   219,   420,   420,   404,   396,   400,   402,
     418,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     101,   403,   404,   196,   196,   196,   206,   220,     5,   434,
     397,    98,    98,    98,    80,    98,   142,   143,   163,   207,
      63,    98,   142,   143,   163,   221,   405,   418,   433,   433,
     434,   434,    16,    17,    18,   108,   208,   414,   415,   108,
     222,   413,   102,   103,   420,   432,   420,   432,   434,    85,
     406,   411,   434,   434,   104,   100,   434,   434,   105,    14,
     407,   408,   434,   409,   100,   408,   418,   434,   410,   418,
     434,   100,   434,   418
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   106,   107,   107,   109,   108,   110,   111,   110,   113,
     114,   112,   115,   115,   115,   115,   117,   118,   116,   119,
     119,   120,   119,   121,   123,   122,   122,   124,   125,   125,
     125,   127,   128,   126,   129,   131,   130,   132,   132,   133,
     133,   134,   136,   137,   135,   138,   139,   139,   139,   139,
     139,   140,   141,   142,   143,   144,   144,   146,   145,   147,
     148,   147,   149,   149,   149,   151,   150,   150,   152,   152,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     155,   154,   156,   156,   157,   157,   157,   157,   157,   159,
     158,   160,   160,   161,   162,   162,   162,   163,   165,   166,
     164,   167,   167,   167,   167,   169,   168,   170,   170,   171,
     171,   171,   171,   171,   172,   174,   173,   175,   176,   175,
     177,   177,   177,   177,   177,   178,   180,   179,   181,   183,
     182,   184,   185,   184,   184,   184,   184,   184,   184,   186,
     186,   187,   188,   188,   188,   188,   188,   188,   189,   189,
     189,   189,   189,   189,   190,   191,   192,   192,   193,   194,
     195,   195,   196,   196,   196,   196,   196,   197,   198,   199,
     199,   201,   200,   202,   202,   203,   204,   205,   205,   206,
     206,   206,   206,   206,   207,   208,   208,   209,   210,   210,
     211,   212,   213,   213,   213,   215,   214,   216,   216,   217,
     218,   219,   219,   220,   220,   220,   220,   220,   221,   222,
     222,   223,   224,   225,   226,   228,   227,   229,   229,   230,
     230,   230,   230,   230,   230,   230,   231,   231,   231,   231,
     231,   231,   231,   233,   232,   234,   234,   235,   236,   235,
     235,   237,   235,   235,   235,   235,   235,   235,   238,   235,
     239,   235,   240,   235,   242,   243,   241,   244,   245,   244,
     244,   246,   247,   244,   244,   248,   244,   244,   244,   244,
     244,   244,   244,   250,   251,   249,   252,   253,   252,   252,
     254,   255,   252,   252,   256,   252,   252,   252,   252,   252,
     252,   252,   252,   258,   259,   257,   260,   261,   260,   260,
     262,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   263,   260,   264,   260,   265,   260,   267,   266,   268,
     268,   269,   270,   269,   269,   269,   269,   269,   269,   269,
     269,   271,   269,   272,   272,   273,   273,   273,   273,   273,
     275,   274,   276,   276,   277,   278,   277,   277,   277,   277,
     277,   279,   277,   281,   280,   282,   282,   283,   284,   283,
     283,   285,   283,   283,   283,   283,   283,   283,   287,   286,
     288,   288,   289,   290,   289,   289,   289,   289,   289,   291,
     289,   292,   289,   294,   293,   295,   295,   296,   296,   297,
     297,   297,   297,   297,   297,   297,   297,   297,   297,   299,
     300,   298,   301,   301,   303,   304,   302,   305,   306,   305,
     305,   305,   305,   305,   307,   305,   308,   305,   309,   309,
     311,   310,   312,   312,   313,   313,   313,   313,   313,   314,
     313,   315,   313,   316,   313,   317,   313,   319,   320,   318,
     321,   322,   321,   323,   321,   324,   321,   326,   327,   325,
     329,   328,   330,   330,   331,   331,   331,   331,   331,   332,
     331,   333,   331,   334,   331,   336,   337,   335,   338,   338,
     338,   338,   339,   339,   340,   340,   341,   341,   341,   342,
     344,   343,   345,   345,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   348,   347,   349,   349,   350,   350,   350,
     350,   350,   352,   351,   353,   353,   354,   354,   354,   354,
     354,   354,   354,   354,   356,   355,   357,   357,   358,   358,
     358,   359,   361,   362,   360,   363,   363,   363,   364,   366,
     367,   365,   368,   368,   368,   369,   370,   371,   371,   372,
     373,   373,   373,   374,   375,   375,   375,   377,   376,   378,
     378,   379,   380,   380,   381,   381,   382,   384,   383,   383,
     386,   385,   385,   387,   388,   389,   390,   390,   392,   391,
     394,   393,   395,   393,   393,   396,   397,   398,   398,   399,
     400,   401,   401,   402,   403,   403,   404,   404,   405,   406,
     407,   408,   409,   409,   410,   410,   411,   412,   413,   413,
     414,   414,   415,   415,   416,   416,   417,   417,   418,   418,
     419,   419,   419,   420,   420,   421,   421,   421,   423,   422,
     424,   425,   425,   426,   426,   426,   427,   427,   428,   428,
     429,   429,   430,   430,   431,   431,   432,   432,   433,   434,
     434,   436,   435,   435,   437,   437,   437,   437,   437,   437,
     437,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   439,
     440
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     4,     0,     0,     6,     0,
       0,    15,     0,     2,     2,     2,     0,     0,    15,     0,
       2,     0,     3,     4,     0,     3,     1,     4,     0,     2,
       2,     0,     0,    11,     1,     0,     6,     1,     4,     0,
       1,     4,     0,     0,    10,     4,     0,     2,     2,     2,
       2,     4,     4,     4,     4,     0,     3,     0,     5,     1,
       0,     5,     0,     2,     2,     0,     3,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     1,     4,     0,     2,     2,     2,     2,     0,
       6,     1,     4,     4,     2,     2,     1,     4,     0,     0,
       6,     2,     2,     2,     1,     0,     5,     1,     4,     0,
       2,     2,     2,     2,     4,     0,     5,     1,     0,     5,
       0,     2,     2,     2,     2,     4,     0,     8,     1,     0,
       5,     0,     0,     4,     2,     2,     2,     2,     2,     1,
       4,     1,     1,     1,     1,     1,     1,     1,     0,     2,
       2,     2,     3,     4,     0,     4,     2,     1,     5,     1,
       1,     4,     0,     2,     2,     2,     2,     5,     1,     1,
       4,     0,     4,     0,     3,     4,     1,     1,     4,     0,
       2,     2,     2,     2,     4,     2,     1,     4,     1,     4,
       4,     4,     2,     2,     1,     0,     3,     0,     2,     5,
       1,     1,     4,     0,     2,     2,     2,     2,     4,     2,
       1,     4,     4,     4,     4,     0,     5,     1,     4,     0,
       2,     2,     2,     3,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     0,     5,     1,     4,     0,     0,     4,
       2,     0,     4,     2,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     0,     9,     0,     0,     4,
       2,     0,     0,     5,     2,     0,     4,     2,     2,     2,
       2,     2,     2,     0,     0,     9,     0,     0,     4,     2,
       0,     0,     5,     2,     0,     4,     2,     2,     2,     2,
       2,     2,     2,     0,     0,     9,     0,     0,     4,     2,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     5,     1,
       4,     0,     0,     4,     2,     2,     2,     2,     2,     2,
       2,     0,     4,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     1,     4,     0,     0,     4,     2,     2,     2,
       2,     0,     4,     0,     5,     1,     4,     0,     0,     4,
       2,     0,     4,     2,     2,     2,     2,     2,     0,     5,
       1,     4,     0,     0,     4,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     5,     1,     4,     2,     1,     0,
       3,     2,     2,     2,     2,     2,     2,     2,     2,     0,
       0,     9,     2,     1,     0,     0,     9,     0,     0,     4,
       2,     2,     2,     2,     0,     4,     0,     4,     2,     1,
       0,     5,     1,     4,     0,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     0,     8,
       0,     0,     4,     0,     4,     0,     4,     0,     0,     8,
       0,     5,     1,     4,     0,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     4,     0,     0,     9,     0,     2,
       2,     4,     2,     1,     1,     2,     1,     1,     1,     3,
       0,     4,     1,     4,     0,     2,     3,     2,     2,     2,
       2,     2,     2,     0,     4,     1,     4,     0,     2,     3,
       2,     2,     0,     4,     1,     4,     0,     3,     2,     2,
       2,     2,     2,     2,     0,     5,     1,     4,     0,     2,
       2,     4,     0,     0,     6,     2,     2,     1,     4,     0,
       0,     6,     2,     2,     1,     4,     4,     2,     1,     4,
       2,     2,     1,     4,     2,     2,     1,     0,     5,     1,
       4,     3,     2,     2,     3,     1,     3,     0,     3,     2,
       0,     4,     1,     1,     2,     2,     0,     2,     0,     3,
       0,     2,     0,     2,     1,     3,     2,     0,     2,     3,
       2,     0,     2,     2,     0,     2,     0,     6,     5,     5,
       5,     4,     0,     2,     0,     5,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     1,     1,
       2,     2,     1,     2,     4,     0,     2,     2,     0,     4,
       2,     0,     1,     0,     1,     3,     0,     5,     1,     4,
       0,     4,     2,     5,     1,     1,     0,     2,     2,     0,
       1,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, scanner, module, submodule, unres, size_arrays, read_all, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, scanner, module, submodule, unres, size_arrays, read_all); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (module);
  YYUSE (submodule);
  YYUSE (unres);
  YYUSE (size_arrays);
  YYUSE (read_all);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, module, submodule, unres, size_arrays, read_all);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , scanner, module, submodule, unres, size_arrays, read_all);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, module, submodule, unres, size_arrays, read_all); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (module);
  YYUSE (submodule);
  YYUSE (unres);
  YYUSE (size_arrays);
  YYUSE (read_all);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 129: /* tmp_identifier_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 187: /* type_stmtsep  */

      { yang_delete_type(module, ((*yyvaluep).type)); }

        break;

    case 338: /* deviation_opt_stmt  */

      { if (read_all) {
                free(((*yyvaluep).nodes).deviation);
              }
            }

        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
/* The lookahead symbol.  */
int yychar;
char *s = NULL, *tmp_s = NULL;
char rev[LY_REV_SIZE];
struct lys_module *trg = NULL;
struct lys_node *tpdf_parent = NULL, *data_node = NULL;
void *actual = NULL;
int config_inherit = 0, actual_type = 0;
int64_t cnt_val;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */

{ yylloc.last_column = 0; }


  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:

    { if (!submodule) {
                       /* update the size of the array, it can be smaller due to possible duplicities
                        * found in submodules */
                       if  (module->inc_size) {
                         module->inc = ly_realloc(module->inc, module->inc_size * sizeof *module->inc);
                         if (!module->inc) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                       if (module->imp_size) {
                         module->imp = ly_realloc(module->imp, module->imp_size * sizeof *module->imp);
                         if (!module->imp) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                     }
                   }

    break;

  case 4:

    { if (read_all) {
                      if (yyget_text(scanner)[0] == '"') {
                        s = yang_read_string(yyget_text(scanner) + 1, yyget_leng(scanner) - 2, yylloc.first_column);
                        if (!s) {
                          YYABORT;
                        }
                      } else {
                        s = calloc(1, yyget_leng(scanner) - 1);
                        if (!s) {
                          LOGMEM;
                          YYABORT;
                        }
                        memcpy(s, yyget_text(scanner) + 1, yyget_leng(scanner) - 2);
                      }
                    }
                  }

    break;

  case 7:

    { if (read_all){
                char *temp;
                if (yyget_text(scanner)[0] == '"') {
                  temp = yang_read_string(yyget_text(scanner) + 1, yyget_leng(scanner) - 2, yylloc.first_column);
                  if (!temp) {
                    YYABORT;
                  }
                  s = ly_realloc(s, strlen(temp) + strlen(s) + 1);
                  if (s) {
                    strcat(s, temp);
                    free(temp);
                  } else {
                    free(temp);
                    LOGMEM;
                    YYABORT;
                  }
                } else {
                  int length = yyget_leng(scanner) - 2 + strlen(s) + 1;
                  s = ly_realloc(s, length);
                  if (s) {
                    memcpy(s + strlen(s), yyget_text(scanner) + 1, yyget_leng(scanner) - 2);
                    s[length - 1] = '\0';
                  } else {
                    LOGMEM;
                    YYABORT;
                  }
                }
              }
            }

    break;

  case 9:

    { if (read_all) {
                                                              if (submodule) {
                                                                free(s);
                                                                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "module");
                                                                YYABORT;
                                                              }
                                                              trg = module;
                                                              yang_read_common(trg,s,MODULE_KEYWORD);
                                                              s = NULL;
                                                              config_inherit = ENABLE_INHERIT;
                                                            }
                                                          }

    break;

  case 10:

    { if (read_all && !module->ns) { LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "namespace", "module"); YYABORT; }
                                       if (read_all && !module->prefix) { LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "module"); YYABORT; }
                                     }

    break;

  case 12:

    { (yyval.i) = 0; }

    break;

  case 13:

    { if ((yyvsp[-1].i)) { LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "module"); YYABORT; } (yyval.i) = 1; }

    break;

  case 14:

    { if (read_all && yang_read_common(module, s, NAMESPACE_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 15:

    { if (read_all && yang_read_prefix(module, NULL, s, MODULE_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 16:

    { if (read_all) {
                                                                    if (!submodule) {
                                                                      free(s);
                                                                      LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "submodule");
                                                                      YYABORT;
                                                                    }
                                                                    trg = (struct lys_module *)submodule;
                                                                    yang_read_common(trg,s,MODULE_KEYWORD);
                                                                    s = NULL;
                                                                    config_inherit = ENABLE_INHERIT;
                                                                  }
                                                                }

    break;

  case 17:

    { if (read_all && !submodule->prefix) {
                                                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                                YYABORT;
                                              }
                                            }

    break;

  case 19:

    { (yyval.i) = 0; }

    break;

  case 20:

    { if ((yyvsp[-1].i)) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "submodule");
                                                  YYABORT;
                                                }
                                                (yyval.i) = 1;
                                              }

    break;

  case 21:

    { if (read_all) {
                                if (submodule->prefix) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                  YYABORT;
                                }
                              }
                            }

    break;

  case 24:

    { if (strlen(yyget_text(scanner))!=1 || yyget_text(scanner)[0]!='1') {
                                               YYABORT;
                                             }
                                           }

    break;

  case 26:

    { if (read_all) {
                 if (strlen(s)!=1 || s[0]!='1') {
                   free(s);
                   YYABORT;
                 }
                 free(s);
                 s = NULL;
               }
             }

    break;

  case 28:

    { if (read_all) {
                          if (size_arrays->imp) {
                            size_t size = (size_arrays->imp * sizeof *trg->imp) + sizeof(void*);
                            trg->imp = calloc(1, size);
                            if (!trg->imp) {
                              LOGMEM;
                              YYABORT;
                            }
                            /* set stop block for possible realloc */
                            trg->imp[size_arrays->imp].module = (void*)0x1;
                          }
                          if (size_arrays->inc) {
                            size_t size = (size_arrays->inc * sizeof *trg->inc) + sizeof(void*);
                            trg->inc = calloc(1, size);
                            if (!trg->inc) {
                              LOGMEM;
                              YYABORT;
                            }
                            /* set stop block for possible realloc */
                            trg->inc[size_arrays->inc].submodule = (void*)0x1;
                          }
                        }
                      }

    break;

  case 31:

    {
                 if (!read_all) {
                   size_arrays->imp++;
                 } else {
                   actual = &trg->imp[trg->imp_size];
                 }
             }

    break;

  case 32:

    { if (read_all) {
                                 if (yang_read_prefix(trg, actual, s, IMPORT_KEYWORD)) {YYABORT;}
                                 s=NULL;
                                 actual_type=IMPORT_KEYWORD;
                               }
                             }

    break;

  case 33:

    { if (read_all) {
                             (yyval.inc) = trg;
                             if (yang_fill_import(trg, actual, (yyvsp[-8].str))) {
                               YYABORT;
                             }
                             trg = (yyval.inc);
                             config_inherit = ENABLE_INHERIT;
                           }
                         }

    break;

  case 34:

    { (yyval.str) = s; s = NULL; }

    break;

  case 35:

    { if (read_all) {
                                                             memset(rev, 0, LY_REV_SIZE);
                                                             actual_type = INCLUDE_KEYWORD;
                                                           }
                                                           else {
                                                             size_arrays->inc++;
                                                           }
                                                         }

    break;

  case 36:

    { if (read_all) {
                                      (yyval.inc) = trg;
                                      int rc;
                                      rc = yang_fill_include(module, submodule, (yyvsp[-3].str), rev, unres);
                                      if (!rc) {
                                        s = NULL;
                                        trg = (yyval.inc);
                                        config_inherit = ENABLE_INHERIT;
                                      } else if (rc == -1) {
                                        YYABORT;
                                      }
                                    }
                                  }

    break;

  case 41:

    { if (read_all) {
                                 if (actual_type==IMPORT_KEYWORD) {
                                     memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                                 } else {                              // INCLUDE KEYWORD
                                     memcpy(rev, s, LY_REV_SIZE - 1);
                                 }
                                 free(s);
                                 s = NULL;
                               }
                             }

    break;

  case 42:

    { if (read_all) {
                                                               if (!ly_strequal(s, submodule->belongsto->name, 0)) {
                                                                 LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "belongs-to");
                                                                 free(s);
                                                                 YYABORT;
                                                               }
                                                               free(s);
                                                               s = NULL;
                                                             }
                                                           }

    break;

  case 43:

    { if (read_all) {
                                     if (yang_read_prefix(trg, NULL, s, MODULE_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }
                                 }

    break;

  case 47:

    { if (read_all && yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 48:

    { if (read_all && yang_read_common(trg, s, CONTACT_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 49:

    { if (read_all && yang_read_description(trg, NULL, s, NULL)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 50:

    { if (read_all && yang_read_reference(trg, NULL, s, NULL)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 55:

    { if (read_all && size_arrays->rev) {
                           trg->rev = calloc(size_arrays->rev, sizeof *trg->rev);
                           if (!trg->rev) {
                             LOGMEM;
                             YYABORT;
                           }
                         }
                       }

    break;

  case 57:

    { if (read_all) {
                                                     if(!(actual=yang_read_revision(trg,s))) {YYABORT;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }

    break;

  case 60:

    { actual_type = REVISION_KEYWORD; }

    break;

  case 63:

    { if (read_all && yang_read_description(trg, actual, s, "revision")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 64:

    { if (read_all && yang_read_reference(trg, actual, s, "revision")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 65:

    { if (read_all) {
                                s = strdup(yyget_text(scanner));
                                if (!s) {
                                  LOGMEM;
                                  YYABORT;
                                }
                              }
                            }

    break;

  case 67:

    { if (read_all && lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 68:

    { if (read_all) {
                       if (size_arrays->features) {
                         trg->features = calloc(size_arrays->features,sizeof *trg->features);
                         if (!trg->features) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                       if (size_arrays->ident) {
                         trg->ident = calloc(size_arrays->ident,sizeof *trg->ident);
                         if (!trg->ident) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                       if (size_arrays->augment) {
                         trg->augment = calloc(size_arrays->augment,sizeof *trg->augment);
                         if (!trg->augment) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                       if (size_arrays->tpdf) {
                         trg->tpdf = calloc(size_arrays->tpdf, sizeof *trg->tpdf);
                         if (!trg->tpdf) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                       if (size_arrays->deviation) {
                         trg->deviation = calloc(size_arrays->deviation, sizeof *trg->deviation);
                         if (!trg->deviation) {
                           LOGMEM;
                           YYABORT;
                         }
                         /* module with deviation - must be implemented (description of /ietf-yang-library:modules-state/module/deviation) */
                         module->implemented = 1;
                       }
                       actual = NULL;
                     }
                   }

    break;

  case 69:

    { actual = NULL; }

    break;

  case 73:

    { if (!read_all) { size_arrays->tpdf++; } }

    break;

  case 76:

    { if (!read_all) {
                     size_arrays->augment++;
                   } else {
                     config_inherit = ENABLE_INHERIT;
                   }
                 }

    break;

  case 79:

    { if (!read_all) { size_arrays->deviation++; } }

    break;

  case 80:

    { if (read_all) {
                                                             /* we have the following supported (hardcoded) extensions: */
                                                             /* ietf-netconf's get-filter-element-attributes */
                                                             if (!strcmp(module->ns, LY_NSNC) && !strcmp(s, "get-filter-element-attributes")) {
                                                               LOGDBG("NETCONF filter extension found");
                                                              /* NACM's default-deny-write and default-deny-all */
                                                             } else if (!strcmp(module->ns, LY_NSNACM) &&
                                                                        (!strcmp(s, "default-deny-write") || !strcmp(s, "default-deny-all"))) {
                                                               LOGDBG("NACM extension found");
                                                               /* other extensions are not supported, so inform about such an extension */
                                                             } else {
                                                               LOGWRN("Not supported \"%s\" extension statement found, ignoring.", s);
                                                             }
                                                             free(s);
                                                             s = NULL;
                                                           }
                                                         }

    break;

  case 84:

    { (yyval.uint) = 0; }

    break;

  case 85:

    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }

    break;

  case 86:

    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }

    break;

  case 87:

    { if (read_all) {
                                             free(s);
                                             s= NULL;
                                           }
                                           if ((yyvsp[-1].uint) & EXTENSION_DSC) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "description", "extension");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].uint) |= EXTENSION_DSC;
                                           (yyval.uint) = (yyvsp[-1].uint);
                                         }

    break;

  case 88:

    { if (read_all) {
                                           free(s);
                                           s = NULL;
                                         }
                                         if ((yyvsp[-1].uint) & EXTENSION_REF) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "reference", "extension");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].uint) |= EXTENSION_REF;
                                         (yyval.uint) = (yyvsp[-1].uint);
                                       }

    break;

  case 89:

    { free(s); s = NULL; }

    break;

  case 96:

    { if (read_all) {
                 if (strcmp(s, "true") && strcmp(s, "false")) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                    free(s);
                    YYABORT;
                 }
                 free(s);
                 s = NULL;
               }
             }

    break;

  case 97:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 98:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 99:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 100:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 101:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 102:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 103:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 104:

    { if (read_all) {
                 if (!strcmp(s, "current")) {
                   (yyval.i) = LYS_STATUS_CURR;
                 } else if (!strcmp(s, "obsolete")) {
                   (yyval.i) = LYS_STATUS_OBSLT;
                 } else if (!strcmp(s, "deprecated")) {
                   (yyval.i) = LYS_STATUS_DEPRC;
                 } else {
                   LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                   free(s);
                   YYABORT;
                 }
                 free(s);
                 s = NULL;
               }
             }

    break;

  case 105:

    { if (read_all) {
                                                         if (!(actual = yang_read_feature(trg, s))) {YYABORT;}
                                                         s=NULL;
                                                       } else {
                                                         size_arrays->features++;
                                                       }
                                                     }

    break;

  case 109:

    { if (read_all) {
                             if (size_arrays->node[size_arrays->next].if_features) {
                               ((struct lys_feature*)actual)->features = calloc(size_arrays->node[size_arrays->next].if_features,
                                                                                sizeof *((struct lys_feature*)actual)->features);
                               if (!((struct lys_feature*)actual)->features) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                             store_flags((struct lys_node *)actual, size_arrays->node[size_arrays->next].flags, 0);
                             size_arrays->next++;
                           } else {
                             (yyval.i) = size_arrays->size;
                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                               LOGMEM;
                               YYABORT;
                             }
                           }
                         }

    break;

  case 110:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, actual, s, unres, FEATURE_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].i)].if_features++;
                                        }
                                      }

    break;

  case 111:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].i)].flags, LYS_STATUS_MASK, "status", "feature", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         }
                                       }

    break;

  case 112:

    { if (read_all && yang_read_description(trg, actual, s, "feature")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 113:

    { if (read_all && yang_read_reference(trg, actual, s, "feature")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 115:

    { if (read_all) {
                                                           if (!(actual = yang_read_identity(trg,s))) {YYABORT;}
                                                           s = NULL;
                                                         } else {
                                                           size_arrays->ident++;
                                                         }
                                                       }

    break;

  case 118:

    { if (read_all && yang_read_base(trg, actual, (yyvsp[0].str), unres)) {
                               YYABORT;
                             }
                           }

    break;

  case 120:

    { (yyval.str) = NULL; }

    break;

  case 121:

    { if (read_all) {
                                     if ((yyvsp[-1].str)) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                                       free(s);
                                       free((yyvsp[-1].str));
                                       YYABORT;
                                     }
                                     (yyval.str) = s;
                                     s = NULL;
                                   }
                                 }

    break;

  case 122:

    { if (read_all) {
                                       if (yang_check_flags((uint16_t*)&((struct lys_ident *)actual)->flags, LYS_STATUS_MASK, "status", "identity", (yyvsp[0].i), 1)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 123:

    { if (read_all && yang_read_description(trg, actual, s, "identity")) {
                                            free((yyvsp[-1].str));
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 124:

    { if (read_all && yang_read_reference(trg, actual, s, "identity")) {
                                          free((yyvsp[-1].str));
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 126:

    { if (read_all) {
                                    if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                                      YYABORT;
                                    }
                                    actual = tpdf_parent;

                                    /* check default value */
                                    if ((yyvsp[0].nodes).node.ptr_tpdf->dflt) {
                                      if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_tpdf->dflt) == -1) {
                                        YYABORT;
                                      }
                                    }
                                  }
                                }

    break;

  case 128:

    { if (read_all) {
                                        tpdf_parent = actual;
                                        if (!(actual = yang_read_typedef(trg, actual, s))) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        actual_type = TYPEDEF_KEYWORD;
                                      }
                                    }

    break;

  case 129:

    { if (read_all && !(actual = yang_read_type(trg, actual, s, actual_type))) {
                                                       YYABORT;
                                                     }
                                                     s = NULL;
                                                   }

    break;

  case 131:

    { (yyval.nodes).node.ptr_tpdf = actual;
                        (yyval.nodes).node.flag = 0;
                      }

    break;

  case 132:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, (yyvsp[0].nodes).node.ptr_tpdf, "type", "typedef");
                       YYABORT;
                     }
                   }

    break;

  case 133:

    { if (read_all) {
                   actual = (yyvsp[-3].nodes).node.ptr_tpdf;
                   actual_type = TYPEDEF_KEYWORD;
                   (yyvsp[-3].nodes).node.flag |= LYS_TYPE_DEF;
                   (yyval.nodes) = (yyvsp[-3].nodes);
                   if (unres_schema_add_node(trg, unres, &(yyvsp[-3].nodes).node.ptr_tpdf->type, UNRES_TYPE_DER_TPDF, tpdf_parent)) {
                     YYABORT;
                   }
                 }
               }

    break;

  case 134:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 135:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 136:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 137:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 138:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 141:

    { if (read_all) {
                          (yyval.type) = actual;
                        } else {
                          (yyval.type) = NULL;
                        }
                      }

    break;

  case 144:

    { /*leafref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 145:

    { /*identityref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->flags |= LYS_TYPE_BASE;
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 146:

    { /*instance_identifier_specification */
                             if (read_all) {
                               ((struct yang_type *)actual)->base = LY_TYPE_INST;
                             }
                           }

    break;

  case 148:

    { if (read_all) {
                                         if (size_arrays->node[size_arrays->next].uni && size_arrays->node[size_arrays->next].pattern) {
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", ((struct yang_type *)actual)->type->parent->name);
                                           YYABORT;
                                         }
                                         if (size_arrays->node[size_arrays->next].pattern) {
                                           ((struct yang_type *)actual)->type->info.str.patterns = calloc(size_arrays->node[size_arrays->next].pattern, sizeof(struct lys_restr));
                                           if (!((struct yang_type *)actual)->type->info.str.patterns) {
                                             LOGMEM;
                                             YYABORT;
                                           }
                                           ((struct yang_type *)actual)->base = LY_TYPE_STRING;
                                         }
                                         if (size_arrays->node[size_arrays->next].uni) {
                                           ((struct yang_type *)actual)->type->info.uni.types = calloc(size_arrays->node[size_arrays->next].uni, sizeof(struct lys_type));
                                           if (!((struct yang_type *)actual)->type->info.uni.types) {
                                             LOGMEM;
                                             YYABORT;
                                           }
                                           ((struct yang_type *)actual)->base = LY_TYPE_UNION;
                                         }
                                         size_arrays->next++;
                                       } else {
                                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                           LOGMEM;
                                           YYABORT;
                                         }
                                         (yyval.uint) = size_arrays->size-1;
                                       }
                                     }

    break;

  case 150:

    { if (!read_all) {
                                                  size_arrays->node[(yyvsp[-1].uint)].pattern++; /* count of pattern*/
                                                }
                                              }

    break;

  case 153:

    { if (read_all) {
                                                                  actual = (yyvsp[-2].v);
                                                                } else {
                                                                  size_arrays->node[(yyvsp[-3].uint)].uni++; /* count of union*/
                                                                }
                                                              }

    break;

  case 154:

    { if (read_all) {
                         struct yang_type *typ;
                         struct lys_type *type;

                         typ = (struct yang_type *)actual;
                         (yyval.v) = actual;
                         type = &typ->type->info.uni.types[typ->type->info.uni.count++];
                         type->parent = typ->type->parent;
                         actual = type;
                         actual_type = UNION_KEYWORD;
                       }
                     }

    break;

  case 155:

    { if (read_all && yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 156:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 157:

    { if (read_all) {
                 char *endptr = NULL;
                 unsigned long val;

                 val = strtoul(s, &endptr, 10);
                 if (*endptr || s[0] == '-' || val == 0 || val > UINT32_MAX) {
                   LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "fraction-digits");
                   free(s);
                   s = NULL;
                   YYABORT;
                 }
                 (yyval.uint) = (uint32_t) val;
                 free(s);
                 s =NULL;
               }
             }

    break;

  case 158:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 159:

    { if (read_all) {
                           (yyval.v) = actual;
                           if (!(actual = yang_read_length(trg, actual, s))) {
                             YYABORT;
                           }
                           actual_type = LENGTH_KEYWORD;
                           s = NULL;
                         }
                       }

    break;

  case 162:

    { switch (actual_type) {
                           case MUST_KEYWORD:
                             (yyval.str) = "must";
                             break;
                           case LENGTH_KEYWORD:
                             (yyval.str) = "length";
                             break;
                           case PATTERN_KEYWORD:
                             (yyval.str) = "pattern";
                             break;
                           case RANGE_KEYWORD:
                             (yyval.str) = "range";
                             break;
                           }
                         }

    break;

  case 163:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 164:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 165:

    { if (read_all && yang_read_description(trg, actual, s, (yyvsp[-1].str))) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 166:

    { if (read_all && yang_read_reference(trg, actual, s, (yyvsp[-1].str))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 167:

    { actual = (yyvsp[-2].v);
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 168:

    { if (read_all) {
                            (yyval.v) = actual;
                            if (!(actual = yang_read_pattern(trg, actual, s))) {
                              YYABORT;
                            }
                            actual_type = PATTERN_KEYWORD;
                            s = NULL;
                          }
                        }

    break;

  case 171:

    { if (read_all) {
                        if (size_arrays->node[size_arrays->next].enm) {
                          ((struct yang_type *)actual)->type->info.enums.enm = calloc(size_arrays->node[size_arrays->next++].enm, sizeof(struct lys_type_enum));
                          if (!((struct yang_type *)actual)->type->info.enums.enm) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                        ((struct yang_type *)actual)->base = LY_TYPE_ENUM;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYABORT;
                        }
                      }
                    }

    break;

  case 175:

    { if (read_all) {
               if (yang_check_enum((yyvsp[-1].v), actual, &cnt_val, actual_type)) {
                 YYABORT;
               }
               actual = (yyvsp[-1].v);
               actual_type = TYPE_KEYWORD;
             } else {
               size_arrays->node[size_arrays->size-1].enm++; /* count of enum*/
             }
           }

    break;

  case 176:

    { if (read_all) {
                         (yyval.v) = actual;
                         if (!(actual = yang_read_enum(trg, actual, s))) {
                           YYABORT;
                         }
                         s = NULL;
                         actual_type = 0;
                       }
                     }

    break;

  case 180:

    { /* actual_type - it is used to check value of enum statement*/
                                if (read_all) {
                                  if (actual_type) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                    YYABORT;
                                  }
                                  actual_type = 1;
                                }
                              }

    break;

  case 181:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 182:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 183:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 184:

    { if (read_all) {
                        ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                        /* keep the highest enum value for automatic increment */
                        if ((yyvsp[-1].i) > cnt_val) {
                          cnt_val = (yyvsp[-1].i);
                        }
                        cnt_val++;
                      }
                    }

    break;

  case 185:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 186:

    { if (read_all) {
                  /* convert it to int32_t */
                  int64_t val;
                  char *endptr;

                  val = strtoll(s, &endptr, 10);
                  if (val < INT32_MIN || val > INT32_MAX || *endptr) {
                      LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "value");
                      free(s);
                      YYABORT;
                  }
                  free(s);
                  s = NULL;
                  (yyval.i) = (int32_t) val;
               }
             }

    break;

  case 187:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 192:

    { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }

    break;

  case 193:

    { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }

    break;

  case 194:

    { if (read_all) {
                  if (!strcmp(s,"true")) {
                    ((struct yang_type *)actual)->type->info.inst.req = 1;
                  } else if (!strcmp(s,"false")) {
                    ((struct yang_type *)actual)->type->info.inst.req = -1;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "require-instance");
                    free(s);
                    YYABORT;
                  }
                  free(s);
                }
              }

    break;

  case 195:

    { if (read_all) {
                        if (size_arrays->node[size_arrays->next].bit) {
                          ((struct yang_type *)actual)->type->info.bits.bit = calloc(size_arrays->node[size_arrays->next++].bit, sizeof(struct lys_type_bit));
                          if (!((struct yang_type *)actual)->type->info.bits.bit) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                        ((struct yang_type *)actual)->base = LY_TYPE_BITS;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYABORT;
                        }
                      }
                    }

    break;

  case 199:

    { if (read_all) {
                      if (yang_check_bit((yyvsp[-2].v), actual, &cnt_val, actual_type)) {
                        YYABORT;
                      }
                      actual = (yyvsp[-2].v);
                    } else {
                      size_arrays->node[size_arrays->size-1].bit++; /* count of bit*/
                    }
                  }

    break;

  case 200:

    { if (read_all) {
                                    (yyval.v) = actual;
                                    if (!(actual = yang_read_bit(trg, actual, s))) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                    actual_type = 0;
                                  }
                                }

    break;

  case 204:

    { /* actual_type - it is used to check position of bit statement*/
                                  if (read_all) {
                                    if (actual_type) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                      YYABORT;
                                    }
                                    actual_type = 1;
                                  }
                                }

    break;

  case 205:

    { if (read_all) {
                                  if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 206:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 207:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 208:

    { if (read_all) {
                           ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                           /* keep the highest position value for automatic increment */
                           if ((yyvsp[-1].uint) > cnt_val) {
                             cnt_val = (yyvsp[-1].uint);
                           }
                           cnt_val++;
                         }
                       }

    break;

  case 209:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 210:

    { if (read_all) {
                  /* convert it to uint32_t */
                  unsigned long val;
                  char *endptr;

                  val = strtoul(s, &endptr, 10);
                  if (val > UINT32_MAX || s[0] == '-' || *endptr) {
                      LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "position");
                      free(s);
                      YYABORT;
                  }
                  free(s);
                  s = NULL;
                  (yyval.uint) = (uint32_t) val;
                }
              }

    break;

  case 215:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
                                                         }
                                                       }

    break;

  case 219:

    { if (read_all) {
                               (yyval.nodes).grouping = actual;
                               actual_type = GROUPING_KEYWORD;
                               if (size_arrays->node[size_arrays->next].tpdf) {
                                 (yyval.nodes).grouping->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).grouping->tpdf);
                                 if (!(yyval.nodes).grouping->tpdf) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).grouping, size_arrays->node[size_arrays->next].flags, 0);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                           }

    break;

  case 220:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 221:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 222:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 223:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 224:

    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }

    break;

  case 225:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 233:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 237:

    { if (read_all) {
                               (yyval.nodes).container = actual;
                               actual_type = CONTAINER_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 (yyval.nodes).container->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).container->features);
                                 if (!(yyval.nodes).container->features) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 (yyval.nodes).container->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).container->must);
                                 if (!(yyval.nodes).container->must) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].tpdf) {
                                 (yyval.nodes).container->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).container->tpdf);
                                 if (!(yyval.nodes).container->tpdf) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).container, size_arrays->node[size_arrays->next].flags, config_inherit);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                           }

    break;

  case 238:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 240:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 241:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 243:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 244:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 245:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 246:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 247:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 248:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 250:

    { if (read_all) {
                                                 actual = (yyvsp[-1].nodes).container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                               }
                                             }

    break;

  case 252:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 254:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 255:

    { if (read_all) {
                                  if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                                    YYABORT;
                                  }
                                  if ((yyvsp[0].nodes).node.ptr_leaf->dflt) {
                                    if ((yyvsp[0].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE) {
                                      /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                                      LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                                      YYABORT;
                                    }
                                    if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_leaf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_leaf->dflt) == -1) {
                                      YYABORT;
                                    }
                                  }
                                }
                              }

    break;

  case 257:

    { if (read_all) {
                            (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LEAF_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).node.ptr_leaf->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_leaf->features);
                              if (!(yyval.nodes).node.ptr_leaf->features) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).node.ptr_leaf->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_leaf->must);
                              if (!(yyval.nodes).node.ptr_leaf->must) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaf, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 258:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }

    break;

  case 260:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 261:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYABORT;
                       }
                     }

    break;

  case 262:

    { if (read_all) {
                     actual = (yyvsp[-2].nodes).node.ptr_leaf;
                     actual_type = LEAF_KEYWORD;
                     (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                     if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaf)) {
                       YYABORT;
                     }
                   }
                 }

    break;

  case 263:

    { (yyval.nodes) = (yyvsp[-4].nodes);}

    break;

  case 264:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 265:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 267:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;}
                                    s = NULL;
                                  }

    break;

  case 268:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i), 0)) {
                                                 YYABORT;
                                               }
                                             }
                                           }

    break;

  case 269:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 270:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 271:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 272:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 273:

    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYABORT;}
                                                               data_node = actual;
                                                               s=NULL;
                                                             }
                                                           }

    break;

  case 274:

    { if (read_all) {
                                             if ((yyvsp[0].nodes).node.ptr_leaflist->flags & LYS_CONFIG_R) {
                                               /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                                * ignore oredering MASK - 0x7F
                                                */
                                             (yyvsp[0].nodes).node.ptr_leaflist->flags &= 0x7F;
                                           }
                                           if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                             LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                                             YYABORT;
                                           }
                                         }
                                       }

    break;

  case 276:

    { if (read_all) {
                               (yyval.nodes).node.ptr_leaflist = actual;
                               (yyval.nodes).node.flag = 0;
                               actual_type = LEAF_LIST_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 (yyval.nodes).node.ptr_leaflist->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_leaflist->features);
                                 if (!(yyval.nodes).node.ptr_leaflist->features) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 (yyval.nodes).node.ptr_leaflist->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_leaflist->must);
                                 if (!(yyval.nodes).node.ptr_leaflist->must) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaflist, size_arrays->node[size_arrays->next].flags, config_inherit);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                           }

    break;

  case 277:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 279:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 280:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYABORT;
                          }
                        }

    break;

  case 281:

    { if (read_all) {
                   actual = (yyvsp[-2].nodes).node.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                   if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER, (struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaflist)) {
                     YYABORT;
                   }
                 }
               }

    break;

  case 282:

    { (yyval.nodes) = (yyvsp[-4].nodes); }

    break;

  case 283:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 284:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 286:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 287:

    { if (read_all) {
                                              if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].nodes).node.ptr_leaflist->min = (yyvsp[0].uint);
                                              (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                              if ((yyvsp[-1].nodes).node.ptr_leaflist->max && ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max)) {
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                                                YYABORT;
                                              }
                                            }
                                          }

    break;

  case 288:

    { if (read_all) {
                                              if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "max-elements", "leaf-list");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].nodes).node.ptr_leaflist->max = (yyvsp[0].uint);
                                              (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                              if ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max) {
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "max-elements");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                                                YYABORT;
                                              }
                                            }
                                          }

    break;

  case 289:

    { if (read_all) {
                                            if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "ordered by", "leaf-list");
                                              YYABORT;
                                            }
                                            if ((yyvsp[0].i) & LYS_USERORDERED) {
                                              (yyvsp[-1].nodes).node.ptr_leaflist->flags |= LYS_USERORDERED;
                                            }
                                            (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 290:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 291:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 292:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 293:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 294:

    { if (read_all) {
                                 if ((yyvsp[0].nodes).node.ptr_list->flags & LYS_CONFIG_R) {
                                   /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                    * ignore oredering MASK - 0x7F
                                    */
                                   (yyvsp[0].nodes).node.ptr_list->flags &= 0x7F;
                                 }
                                 if (((yyvsp[0].nodes).node.ptr_list->flags & LYS_CONFIG_W) && !(yyvsp[0].nodes).node.ptr_list->keys) {
                                   LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_list, "key", "list");
                                   YYABORT;
                                 }
                                 if ((yyvsp[0].nodes).node.ptr_list->keys && yang_read_key(trg, (yyvsp[0].nodes).node.ptr_list, unres)) {
                                   YYABORT;
                                 }
                                 if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                   LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_list, "data-def statement missing.");
                                   YYABORT;
                                 }
                                 if (yang_read_unique(trg, (yyvsp[0].nodes).node.ptr_list, unres)) {
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 296:

    { if (read_all) {
                          (yyval.nodes).node.ptr_list = actual;
                          (yyval.nodes).node.flag = 0;
                          actual_type = LIST_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).node.ptr_list->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_list->features);
                            if (!(yyval.nodes).node.ptr_list->features) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].must) {
                            (yyval.nodes).node.ptr_list->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_list->must);
                            if (!(yyval.nodes).node.ptr_list->must) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].tpdf) {
                            (yyval.nodes).node.ptr_list->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_list->tpdf);
                            if (!(yyval.nodes).node.ptr_list->tpdf) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].unique) {
                            (yyval.nodes).node.ptr_list->unique = calloc(size_arrays->node[size_arrays->next].unique, sizeof *(yyval.nodes).node.ptr_list->unique);
                            if (!(yyval.nodes).node.ptr_list->unique) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).node.ptr_list, size_arrays->node[size_arrays->next].flags, config_inherit);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                      }

    break;

  case 297:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 299:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 300:

    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }

    break;

  case 302:

    { if (read_all) {
                                if ((yyvsp[-1].nodes).node.ptr_list->keys) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "key", "list");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_list->keys = (struct lys_node_leaf **)s;
                                (yyval.nodes) = (yyvsp[-1].nodes);
                                s=NULL;
                              }
                            }

    break;

  case 303:

    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }

    break;

  case 304:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 305:

    { if (read_all) {
                                         if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "min-elements", "list");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).node.ptr_list->min = (yyvsp[0].uint);
                                         (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                         if ((yyvsp[-1].nodes).node.ptr_list->max && ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max)) {
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                                           YYABORT;
                                         }
                                       }
                                     }

    break;

  case 306:

    { if (read_all) {
                                         if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "max-elements", "list");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).node.ptr_list->max = (yyvsp[0].uint);
                                         (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                         if ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max) {
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                                           YYABORT;
                                         }
                                       }
                                     }

    break;

  case 307:

    { if (read_all) {
                                       if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "ordered by", "list");
                                         YYABORT;
                                       }
                                       if ((yyvsp[0].i) & LYS_USERORDERED) {
                                         (yyvsp[-1].nodes).node.ptr_list->flags |= LYS_USERORDERED;
                                       }
                                       (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                       (yyval.nodes) = (yyvsp[-1].nodes);
                                     }
                                   }

    break;

  case 308:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 309:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 310:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 311:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }

    break;

  case 313:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 315:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 316:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 317:

    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_CHOICE,sizeof(struct lys_node_choice)))) {YYABORT;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }

    break;

  case 321:

    { if (read_all) {
                            (yyval.nodes).node.ptr_choice = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = CHOICE_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).node.ptr_choice->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_choice->features);
                              if (!(yyval.nodes).node.ptr_choice->features) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).node.ptr_choice, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 322:

    { actual = (yyvsp[-1].nodes).node.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 323:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 324:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_choice, s, unres, CHOICE_KEYWORD)) {
                                           YYABORT;
                                         }
                                         s=NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 325:

    { if (read_all) {
                                      if ((yyvsp[-1].nodes).node.flag & LYS_CHOICE_DEFAULT) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "default", "choice");
                                        free(s);
                                        YYABORT;
                                      }

                                      if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_TRUE) {
                                        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "choice");
                                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"default\" statement is forbidden on choices with \"mandatory\".");
                                        free(s);
                                        YYABORT;
                                      }

                                      if (unres_schema_add_str(trg, unres, (yyvsp[-1].nodes).node.ptr_choice, UNRES_CHOICE_DFLT, s) == -1) {
                                        free(s);
                                        YYABORT;
                                      }
                                      free(s);
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                      (yyval.nodes).node.flag |= LYS_CHOICE_DEFAULT;
                                    }
                                  }

    break;

  case 326:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 327:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 328:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 329:

    { if (read_all) {
                                          if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 330:

    { if (read_all) {
                                        if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }
                                    }

    break;

  case 331:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 332:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 340:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 344:

    { if (read_all) {
                          (yyval.nodes).cs = actual;
                          actual_type = CASE_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).cs->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).cs->features);
                            if (!(yyval.nodes).cs->features) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).cs, size_arrays->node[size_arrays->next].flags, 1);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                      }

    break;

  case 345:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 347:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 348:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 349:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 350:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 351:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 353:

    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_ANYXML,sizeof(struct lys_node_anyxml)))) {YYABORT;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }

    break;

  case 357:

    { if (read_all) {
                            (yyval.nodes).anyxml = actual;
                            actual_type = ANYXML_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).anyxml->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).anyxml->features);
                              if (!(yyval.nodes).anyxml->features) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).anyxml->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).anyxml->must);
                              if (!(yyval.nodes).anyxml->must) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).anyxml, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 358:

    { actual = (yyvsp[-1].nodes).anyxml; actual_type = ANYXML_KEYWORD; }

    break;

  case 360:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).anyxml, s, unres, ANYXML_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 361:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 363:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 364:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "anyxml", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 365:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 366:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 367:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 368:

    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_USES,sizeof(struct lys_node_uses)))) {YYABORT;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }

    break;

  case 369:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYABORT;
                        }
                      }
                    }

    break;

  case 372:

    { if (read_all) {
                          (yyval.nodes).uses.ptr_uses = actual;
                          (yyval.nodes).uses.config_inherit = config_inherit;
                          actual_type = USES_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).uses.ptr_uses->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).uses.ptr_uses->features);
                            if (!(yyval.nodes).uses.ptr_uses->features) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].refine) {
                            (yyval.nodes).uses.ptr_uses->refine = calloc(size_arrays->node[size_arrays->next].refine, sizeof *(yyval.nodes).uses.ptr_uses->refine);
                            if (!(yyval.nodes).uses.ptr_uses->refine) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].augment) {
                            (yyval.nodes).uses.ptr_uses->augment = calloc(size_arrays->node[size_arrays->next].augment, sizeof *(yyval.nodes).uses.ptr_uses->augment);
                            if (!(yyval.nodes).uses.ptr_uses->augment) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).uses.ptr_uses, size_arrays->node[size_arrays->next].flags, 0);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                      }

    break;

  case 373:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 375:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 376:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 377:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 378:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 379:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 381:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                         actual_type = USES_KEYWORD;
                                         data_node = actual;
                                         if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                           data_node = NULL;
                                         }
                                         config_inherit = (yyvsp[-1].nodes).uses.config_inherit;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].augment++;
                                       }
                                     }

    break;

  case 383:

    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 389:

    { if (read_all) {
                                  (yyval.nodes).refine = actual;
                                  actual_type = REFINE_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].must) {
                                    (yyval.nodes).refine->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).refine->must);
                                    if (!(yyval.nodes).refine->must) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                    (yyval.nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
                                  }
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 390:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 391:

    { if (read_all) {
                                             if ((yyvsp[-1].nodes).refine->target_type) {
                                               if ((yyvsp[-1].nodes).refine->target_type & LYS_CONTAINER) {
                                                 if ((yyvsp[-1].nodes).refine->mod.presence) {
                                                   LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "presence", "refine");
                                                   free(s);
                                                   YYABORT;
                                                 }
                                                 (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                                 (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                               } else {
                                                 free(s);
                                                 LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "presence", "refine");
                                                 LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                 YYABORT;
                                               }
                                             } else {
                                               (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                               (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 392:

    { if (read_all) {
                                            if ((yyvsp[-1].nodes).refine->target_type) {
                                              if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                                if ((yyvsp[-1].nodes).refine->mod.dflt) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "default", "refine");
                                                  free(s);
                                                  YYABORT;
                                                }
                                                (yyvsp[-1].nodes).refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                              } else {
                                                free(s);
                                                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                YYABORT;
                                              }
                                            } else {
                                              (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE;
                                              (yyvsp[-1].nodes).refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 393:

    { if (read_all) {
                                           if ((yyvsp[-1].nodes).refine->target_type) {
                                             if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST)) {
                                               (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST);
                                               if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).refine->flags, LYS_CONFIG_MASK, "config", "refine", (yyvsp[0].i), 1)) {
                                                 YYABORT;
                                               }
                                             } else {
                                               LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "config", "refine");
                                               LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                               YYABORT;
                                             }
                                           } else {
                                             (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST;
                                             (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                           }
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 394:

    { if (read_all) {
                                              if ((yyvsp[-1].nodes).refine->target_type) {
                                                if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML)) {
                                                  (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML);
                                                  if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).refine->flags, LYS_MAND_MASK, "mandatory", "refine", (yyvsp[0].i), 1)) {
                                                    YYABORT;
                                                  }
                                                } else {
                                                  LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "refine");
                                                  LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                  YYABORT;
                                                }
                                              } else {
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYXML;
                                                (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 395:

    { if (read_all) {
                                                 if ((yyvsp[-1].nodes).refine->target_type) {
                                                   if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     if ((yyvsp[-1].nodes).refine->flags & LYS_RFN_MINSET) {
                                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                       YYABORT;
                                                     }
                                                     (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MINSET;
                                                     (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                   } else {
                                                     LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                     LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                     YYABORT;
                                                   }
                                                 } else {
                                                   (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MINSET;
                                                   (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 396:

    { if (read_all) {
                                                 if ((yyvsp[-1].nodes).refine->target_type) {
                                                   if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     if ((yyvsp[-1].nodes).refine->flags & LYS_RFN_MAXSET) {
                                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                       YYABORT;
                                                     }
                                                     (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MAXSET;
                                                     (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                   } else {
                                                     LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                     LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                     YYABORT;
                                                   }
                                                 } else {
                                                   (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MAXSET;
                                                   (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 397:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 398:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 399:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYABORT;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 400:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYABORT;
                                          }
                                        }

    break;

  case 404:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYABORT;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 405:

    { if (read_all) {
                                       if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                         LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "augment");
                                         YYABORT;
                                       }
                                       if (unres_schema_add_node(trg, unres, actual, UNRES_AUGMENT, NULL) == -1) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 407:

    { if (read_all) {
                             (yyval.nodes).node.ptr_augment = actual;
                             (yyval.nodes).node.flag = 0;
                             actual_type = AUGMENT_KEYWORD;
                             if (size_arrays->node[size_arrays->next].if_features) {
                               (yyval.nodes).node.ptr_augment->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_augment->features);
                               if (!(yyval.nodes).node.ptr_augment->features) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                             config_inherit = DISABLE_INHERIT;
                             size_arrays->next++;
                           } else {
                             (yyval.nodes).index = size_arrays->size;
                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                               LOGMEM;
                               YYABORT;
                             }
                           }
                         }

    break;

  case 408:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 410:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 411:

    { if (read_all) {
                                      /* hack - flags is bit field, so its address is taken as a member after
                                       * 3 const char pointers in the lys_node_augment structure */
                                      if (yang_check_flags((uint16_t*)((const char **)(yyvsp[-1].nodes).node.ptr_augment + 3),
                                                           LYS_STATUS_MASK, "status", "augment", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    }
                                  }

    break;

  case 412:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 413:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 414:

    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }

    break;

  case 415:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 416:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 417:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 420:

    { if (read_all) {
                                                 if (!(actual = yang_read_node(trg, NULL, s, LYS_RPC, sizeof(struct lys_node_rpc)))) {
                                                   YYABORT;
                                                 }
                                                 data_node = actual;
                                                 s = NULL;
                                               }
                                               config_inherit = DISABLE_INHERIT;
                                             }

    break;

  case 421:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 424:

    { if (read_all) {
                         (yyval.nodes).node.ptr_rpc = actual;
                         (yyval.nodes).node.flag = 0;
                         actual_type = RPC_KEYWORD;
                         if (size_arrays->node[size_arrays->next].if_features) {
                           (yyval.nodes).node.ptr_rpc->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_rpc->features);
                           if (!(yyval.nodes).node.ptr_rpc->features) {
                             LOGMEM;
                             YYABORT;
                           }
                         }
                         if (size_arrays->node[size_arrays->next].tpdf) {
                           (yyval.nodes).node.ptr_rpc->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_rpc->tpdf);
                           if (!(yyval.nodes).node.ptr_rpc->tpdf) {
                             LOGMEM;
                             YYABORT;
                           }
                         }
                         store_flags((struct lys_node *)(yyval.nodes).node.ptr_rpc, size_arrays->node[size_arrays->next].flags, 0);
                         size_arrays->next++;
                       } else {
                         (yyval.nodes).index = size_arrays->size;
                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                           LOGMEM;
                           YYABORT;
                         }
                       }
                     }

    break;

  case 425:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 426:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 427:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 428:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 429:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 431:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 433:

    { if (read_all) {
                                 if ((yyvsp[-1].nodes).node.flag & LYS_RPC_INPUT) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "input", "rpc");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.flag |= LYS_RPC_INPUT;
                                 actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                 actual_type = RPC_KEYWORD;
                                 data_node = actual;
                               }
                             }

    break;

  case 434:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 435:

    { if (read_all) {
                                  if ((yyvsp[-1].nodes).node.flag & LYS_RPC_OUTPUT) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "output", "rpc");
                                    YYABORT;
                                  }
                                  (yyvsp[-1].nodes).node.flag |= LYS_RPC_OUTPUT;
                                  actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }
                              }

    break;

  case 436:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 437:

    { if (read_all) {
                                     s = strdup("input");
                                     if (!s) {
                                       LOGMEM;
                                       YYABORT;
                                     }
                                     if (!(actual = yang_read_node(trg, actual, s, LYS_INPUT, sizeof(struct lys_node_rpc_inout)))) {
                                      YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                   }
                                 }

    break;

  case 438:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYABORT;
                                        }
                                      }

    break;

  case 440:

    { if (read_all) {
                                  (yyval.nodes).node.ptr_inout = actual;
                                  (yyval.nodes).node.flag = 0;
                                  actual_type = INPUT_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).node.ptr_inout->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_inout->tpdf);
                                    if (!(yyval.nodes).node.ptr_inout->tpdf) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 441:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 443:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 445:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 446:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 447:

    { if (read_all) {
                                       s = strdup("output");
                                       if (!s) {
                                         LOGMEM;
                                         YYABORT;
                                       }
                                       if (!(actual = yang_read_node(trg, actual, s, LYS_OUTPUT, sizeof(struct lys_node_rpc_inout)))) {
                                        YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                     }
                                   }

    break;

  case 448:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYABORT;
                                         }
                                       }

    break;

  case 450:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 451:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 454:

    { if (read_all) {
                                  (yyval.nodes).notif = actual;
                                  actual_type = NOTIFICATION_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].if_features) {
                                    (yyval.nodes).notif->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).notif->features);
                                    if (!(yyval.nodes).notif->features) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).notif->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).notif->tpdf);
                                    if (!(yyval.nodes).notif->tpdf) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }
                                  store_flags((struct lys_node *)(yyval.nodes).notif, size_arrays->node[size_arrays->next].flags, 0);
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 455:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 456:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 457:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 458:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 459:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 461:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 463:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 465:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 466:

    { if (read_all) {
                                            if (actual_type == DEVIATION_KEYWORD) {
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                                              YYABORT;
                                            }
                                            if (yang_check_deviation(trg, actual, unres)) {
                                              YYABORT;
                                            }
                                            free((yyvsp[0].nodes).deviation);
                                          }
                                        }

    break;

  case 468:

    { if (read_all) {
                               (yyval.nodes).deviation = actual;
                               actual_type = DEVIATION_KEYWORD;
                               if (size_arrays->node[size_arrays->next].deviate) {
                                 (yyval.nodes).deviation->deviation->deviate = calloc(size_arrays->node[size_arrays->next].deviate, sizeof *(yyval.nodes).deviation->deviation->deviate);
                                 if (!(yyval.nodes).deviation->deviation->deviate) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                           }

    break;

  case 469:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 470:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 471:

    { if (read_all) {
                                                                  actual = (yyvsp[-3].nodes).deviation;
                                                                  actual_type = DEVIATE_KEYWORD;
                                                                  (yyval.nodes) = (yyvsp[-3].nodes);
                                                                } else {
                                                                  /* count of deviate statemenet */
                                                                  size_arrays->node[(yyvsp[-3].nodes).index].deviate++;
                                                                }
                                                              }

    break;

  case 474:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 480:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 484:

    { if (read_all) {
                                 (yyval.nodes).deviation = actual;
                                 actual_type = ADD_KEYWORD;
                                 if (size_arrays->node[size_arrays->next].must) {
                                    if (yang_read_deviate_must(actual, size_arrays->node[size_arrays->next].must)) {
                                      YYABORT;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].unique) {
                                    if (yang_read_deviate_unique(actual, size_arrays->node[size_arrays->next].unique)) {
                                      YYABORT;
                                    }
                                  }
                                  size_arrays->next++;
                               } else {
                                 (yyval.nodes).index = size_arrays->size;
                                 if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 485:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 486:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 487:

    { if (read_all) {
                                          struct lys_node_list *list;

                                          list = (struct lys_node_list *)(yyvsp[-1].nodes).deviation->target;
                                          if (yang_fill_unique(trg, list, &list->unique[list->unique_size], s, NULL)) {
                                            list->unique_size++;
                                            YYABORT;
                                          }
                                          list->unique_size++;
                                          free(s);
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                        }
                                      }

    break;

  case 488:

    { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 489:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 490:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 491:

    { if (read_all) {
                                                if ((yyvsp[-1].nodes).deviation->deviate->min_set) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                  YYABORT;
                                                }
                                                if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 0)) {
                                                  YYABORT;
                                                }
                                                (yyval.nodes) =  (yyvsp[-1].nodes);
                                              }
                                            }

    break;

  case 492:

    { if (read_all) {
                                                if ((yyvsp[-1].nodes).deviation->deviate->max_set) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                  YYABORT;
                                                }
                                                if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 1)) {
                                                  YYABORT;
                                                }
                                                (yyval.nodes) =  (yyvsp[-1].nodes);
                                              }
                                            }

    break;

  case 493:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 497:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = DELETE_KEYWORD;
                                    if (size_arrays->node[size_arrays->next].must) {
                                      if (yang_read_deviate_must(actual, size_arrays->node[size_arrays->next].must)) {
                                        YYABORT;
                                      }
                                    }
                                    if (size_arrays->node[size_arrays->next].unique) {
                                      if (yang_read_deviate_unique(actual, size_arrays->node[size_arrays->next].unique)) {
                                        YYABORT;
                                      }
                                    }
                                    size_arrays->next++;
                                  } else {
                                    (yyval.nodes).index = size_arrays->size;
                                    if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }
                                }

    break;

  case 498:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 499:

    { if (read_all) {
                                           if (yang_check_deviate_must(trg->ctx, (yyvsp[-2].nodes).deviation)) {
                                             YYABORT;
                                           }
                                           actual = (yyvsp[-2].nodes).deviation;
                                           actual_type = DELETE_KEYWORD;
                                           (yyval.nodes) = (yyvsp[-2].nodes);
                                         } else {
                                           size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                         }
                                       }

    break;

  case 500:

    { if (read_all) {
                                             if (yang_check_deviate_unique(trg, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           } else {
                                             size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                           }
                                         }

    break;

  case 501:

    { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 502:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 506:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }

    break;

  case 507:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 508:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 509:

    { if (read_all) {
                                               if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                 YYABORT;
                                               }
                                               s = NULL;
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }
                                           }

    break;

  case 510:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 511:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 512:

    { if (read_all) {
                                                    if ((yyvsp[-1].nodes).deviation->deviate->min_set) {
                                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviate");
                                                      YYABORT;
                                                    }
                                                    if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 0)) {
                                                      YYABORT;
                                                    }
                                                    (yyval.nodes) =  (yyvsp[-1].nodes);
                                                  }
                                                }

    break;

  case 513:

    { if (read_all) {
                                                    if ((yyvsp[-1].nodes).deviation->deviate->max_set) {
                                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviate");
                                                      YYABORT;
                                                    }
                                                    if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 1)) {
                                                      YYABORT;
                                                    }
                                                    (yyval.nodes) =  (yyvsp[-1].nodes);
                                                  }
                                                }

    break;

  case 514:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 519:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 520:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 521:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 522:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 523:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 524:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 525:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 526:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 527:

    { if (read_all) {
                  if (!strcmp(s, "true")) {
                    (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET;
                  } else if (!strcmp(s, "false")) {
                    (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "config");
                    free(s);
                    YYABORT;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 528:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 529:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 530:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 531:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 532:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 533:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 534:

    { if (read_all) {
                  if (!strcmp(s, "true")) {
                    (yyval.i) = LYS_MAND_TRUE;
                  } else if (!strcmp(s, "false")) {
                    (yyval.i) = LYS_MAND_FALSE;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "mandatory");
                    free(s);
                    YYABORT;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 536:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 537:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 538:

    { if (read_all) {
                  if (strlen(s) == 1 && s[0] == '0') {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(s, &endptr, 10);
                    if (val > UINT32_MAX || *endptr) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "min-elements");
                        free(s);
                        YYABORT;
                    }
                    (yyval.uint) = (uint32_t) val;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 539:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 540:

    { (yyval.uint) = 0; }

    break;

  case 541:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 542:

    { if (read_all) {
                  if (!strcmp(s, "unbounded")) {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(s, &endptr, 10);
                    if (val == 0 || val > UINT32_MAX || *endptr) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "max-elements");
                        free(s);
                        YYABORT;
                    }
                    (yyval.uint) = (uint32_t) val;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 543:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 544:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 545:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 546:

    { if (read_all) {
                  if (!strcmp(s, "user")) {
                    (yyval.i) = LYS_USERORDERED;
                  } else if (!strcmp(s, "system")) {
                    (yyval.i) = LYS_SYSTEMORDERED;
                  } else {
                    free(s);
                    YYABORT;
                  }
                  free(s);
                  s=NULL;
                }
              }

    break;

  case 547:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 557:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 560:

    { if (read_all) {
                                 s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                                 strcat(s," ");
                                 strcat(s, yyget_text(scanner));
                                }
                             }

    break;

  case 563:

    { if (read_all) {
                          (yyval.v) = actual;
                          if (!(actual = yang_read_range(trg, actual, s))) {
                             YYABORT;
                          }
                          actual_type = RANGE_KEYWORD;
                          s = NULL;
                        }
                      }

    break;

  case 564:

    { if (read_all) {
                                                if (s) {
                                                  s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYABORT;
                                                  }
                                                  strcat(s,"/");
                                                  strcat(s, yyget_text(scanner));
                                                } else {
                                                  s = malloc(yyget_leng(scanner) + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYABORT;
                                                  }
                                                  s[0]='/';
                                                  memcpy(s + 1, yyget_text(scanner), yyget_leng(scanner) + 1);
                                                }
                                              }
                                            }

    break;

  case 568:

    { if (read_all)  {
                                              if (s) {
                                                s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 1);
                                                if (!s) {
                                                  LOGMEM;
                                                  YYABORT;
                                                }
                                                strcat(s, yyget_text(scanner));
                                              } else {
                                                s = strdup(yyget_text(scanner));
                                                if (!s) {
                                                  LOGMEM;
                                                  YYABORT;
                                                }
                                              }
                                            }
                                          }

    break;

  case 570:

    { tmp_s = yyget_text(scanner); }

    break;

  case 571:

    { if (read_all) {
                                                     s = strdup(tmp_s);
                                                     if (!s) {
                                                       LOGMEM;
                                                       YYABORT;
                                                     }
                                                     s[strlen(s) - 1] = '\0';
                                                   }
                                                 }

    break;

  case 572:

    { tmp_s = yyget_text(scanner); }

    break;

  case 573:

    { if (read_all) {
                                           s = strdup(tmp_s);
                                           if (!s) {
                                             LOGMEM;
                                             YYABORT;
                                           }
                                           s[strlen(s) - 1] = '\0';
                                         }
                                       }

    break;

  case 597:

    { /* convert it to uint32_t */
                                                unsigned long val;

                                                val = strtoul(yyget_text(scanner), NULL, 10);
                                                if (val > UINT32_MAX) {
                                                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Converted number is very long.");
                                                    YYABORT;
                                                }
                                                (yyval.uint) = (uint32_t) val;
                                             }

    break;

  case 598:

    { (yyval.uint) = 0; }

    break;

  case 599:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 600:

    { (yyval.i) = 0; }

    break;

  case 601:

    { /* convert it to int32_t */
               int64_t val;

               val = strtoll(yyget_text(scanner), NULL, 10);
               if (val < INT32_MIN || val > INT32_MAX) {
                   LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The number is not in the correct range (INT32_MIN..INT32_MAX): \"%d\"",val);
                   YYABORT;
               }
               (yyval.i) = (int32_t) val;
             }

    break;

  case 607:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 612:

    { if (read_all) {
                 char *tmp;

                 if ((tmp = strchr(s, ':'))) {
                   *tmp = '\0';
                   /* check prefix */
                   if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYABORT;
                   }
                   /* check identifier */
                   if (lyp_check_identifier(tmp + 1, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYABORT;
                   }
                   *tmp = ':';
                 } else {
                   /* check identifier */
                   if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYABORT;
                   }
                 }
               }
             }

    break;

  case 618:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 641:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 729:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 730:

    { if (read_all) {
                                      s = strdup(yyget_text(scanner));
                                      if (!s) {
                                        LOGMEM;
                                        YYABORT;
                                      }
                                    }
                                  }

    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, scanner, module, submodule, unres, size_arrays, read_all, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, scanner, module, submodule, unres, size_arrays, read_all, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, scanner, module, submodule, unres, size_arrays, read_all);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, scanner, module, submodule, unres, size_arrays, read_all);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, scanner, module, submodule, unres, size_arrays, read_all, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, scanner, module, submodule, unres, size_arrays, read_all);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, scanner, module, submodule, unres, size_arrays, read_all);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}



void yyerror(YYLTYPE *yylloc, void *scanner, ...){

  (void)yylloc; /* unused */
  LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
}
