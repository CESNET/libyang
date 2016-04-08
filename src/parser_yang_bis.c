/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

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
char *s, *tmp_s;
char rev[LY_REV_SIZE];
struct lys_module *trg;
/* temporary pointer for the check extension nacm */
struct lys_node *data_node;
/* pointer on the current parsed element*/
void *actual;
int config_inherit;
int actual_type;
int64_t cnt_val;



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
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{


  int32_t i;
  uint32_t uint;
  char *str;
  void *v;
  struct lys_module *inc;
  union {
    uint32_t index;
    struct lys_node_container *container;
    struct lys_node_anyxml *anyxml;
    struct type_choice choice;
    struct type_node node;
    struct lys_node_case *cs;
    struct lys_node_grp *grouping;
    struct type_uses uses;
    struct lys_refine *refine;
    struct lys_node_notif *notif;
    struct type_deviation *deviation;
  } nodes;


};
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
#define YYLAST   3160

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  336
/* YYNRULES -- Number of rules.  */
#define YYNRULES  730
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1185

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
       0,   244,   244,   245,   251,   251,   270,   272,   271,   303,
     315,   303,   324,   325,   326,   327,   330,   342,   330,   353,
     354,   360,   360,   369,   371,   371,   375,   383,   385,   408,
     409,   412,   420,   412,   437,   439,   439,   459,   460,   464,
     465,   467,   469,   478,   489,   478,   498,   500,   501,   502,
     503,   508,   514,   516,   518,   520,   522,   530,   533,   533,
     542,   543,   543,   548,   549,   554,   561,   561,   570,   576,
     617,   620,   621,   622,   623,   624,   625,   626,   632,   633,
     634,   636,   636,   655,   656,   660,   661,   668,   675,   686,
     698,   698,   700,   701,   705,   707,   708,   709,   720,   722,
     723,   722,   726,   727,   728,   729,   746,   746,   755,   756,
     761,   780,   787,   793,   798,   804,   806,   806,   815,   817,
     816,   824,   825,   836,   842,   848,   855,   859,   857,   879,
     889,   889,   896,   899,   899,   911,   912,   918,   924,   929,
     935,   936,   942,   943,   944,   951,   959,   964,   968,   998,
     999,  1003,  1004,  1005,  1013,  1026,  1032,  1033,  1051,  1055,
    1065,  1066,  1071,  1086,  1091,  1096,  1101,  1107,  1111,  1121,
    1122,  1127,  1127,  1145,  1146,  1149,  1161,  1171,  1172,  1177,
    1178,  1187,  1193,  1198,  1204,  1216,  1217,  1235,  1240,  1241,
    1246,  1248,  1250,  1254,  1258,  1273,  1273,  1291,  1292,  1294,
    1305,  1315,  1316,  1321,  1322,  1331,  1337,  1342,  1348,  1360,
    1361,  1376,  1378,  1380,  1382,  1384,  1384,  1391,  1392,  1397,
    1417,  1423,  1428,  1433,  1434,  1441,  1444,  1445,  1446,  1447,
    1448,  1449,  1450,  1453,  1453,  1461,  1462,  1467,  1501,  1501,
    1503,  1510,  1510,  1518,  1519,  1525,  1531,  1536,  1541,  1541,
    1546,  1546,  1554,  1554,  1561,  1568,  1561,  1593,  1621,  1621,
    1623,  1630,  1635,  1630,  1642,  1643,  1643,  1651,  1654,  1660,
    1666,  1672,  1677,  1683,  1690,  1683,  1714,  1742,  1742,  1744,
    1751,  1756,  1751,  1763,  1764,  1764,  1772,  1778,  1792,  1806,
    1818,  1824,  1829,  1835,  1842,  1835,  1867,  1908,  1908,  1910,
    1917,  1917,  1925,  1935,  1943,  1949,  1963,  1977,  1989,  1995,
    2000,  2005,  2005,  2013,  2013,  2018,  2018,  2026,  2026,  2037,
    2039,  2038,  2056,  2077,  2077,  2079,  2092,  2104,  2112,  2120,
    2128,  2137,  2146,  2146,  2156,  2157,  2160,  2161,  2162,  2163,
    2164,  2167,  2167,  2175,  2176,  2180,  2200,  2200,  2202,  2209,
    2215,  2220,  2225,  2225,  2232,  2232,  2243,  2244,  2248,  2275,
    2275,  2277,  2284,  2284,  2292,  2298,  2304,  2310,  2315,  2321,
    2321,  2337,  2338,  2342,  2377,  2377,  2379,  2386,  2392,  2397,
    2402,  2402,  2410,  2410,  2425,  2425,  2434,  2435,  2440,  2441,
    2444,  2464,  2471,  2495,  2519,  2538,  2557,  2582,  2607,  2612,
    2618,  2627,  2618,  2634,  2635,  2638,  2647,  2638,  2659,  2680,
    2680,  2682,  2689,  2695,  2700,  2705,  2705,  2713,  2713,  2723,
    2724,  2727,  2727,  2738,  2739,  2744,  2772,  2779,  2785,  2790,
    2795,  2795,  2803,  2803,  2808,  2808,  2820,  2820,  2833,  2841,
    2833,  2848,  2868,  2868,  2876,  2876,  2881,  2881,  2891,  2899,
    2891,  2906,  2906,  2916,  2920,  2925,  2952,  2959,  2965,  2970,
    2975,  2975,  2983,  2983,  2988,  2988,  2995,  3004,  2995,  3017,
    3036,  3043,  3050,  3060,  3061,  3063,  3068,  3070,  3071,  3072,
    3075,  3077,  3077,  3083,  3084,  3088,  3110,  3118,  3126,  3142,
    3150,  3157,  3164,  3175,  3187,  3187,  3193,  3194,  3198,  3220,
    3228,  3239,  3249,  3258,  3258,  3264,  3265,  3269,  3274,  3280,
    3288,  3296,  3303,  3310,  3321,  3333,  3333,  3336,  3337,  3341,
    3342,  3347,  3353,  3355,  3356,  3355,  3359,  3360,  3361,  3376,
    3378,  3379,  3378,  3382,  3383,  3384,  3399,  3401,  3403,  3404,
    3425,  3427,  3428,  3429,  3450,  3452,  3453,  3454,  3466,  3466,
    3474,  3475,  3480,  3482,  3483,  3485,  3486,  3488,  3490,  3490,
    3499,  3502,  3502,  3513,  3516,  3526,  3547,  3549,  3550,  3553,
    3553,  3572,  3572,  3581,  3581,  3590,  3593,  3595,  3597,  3598,
    3600,  3602,  3604,  3605,  3607,  3609,  3610,  3612,  3613,  3615,
    3617,  3620,  3624,  3626,  3627,  3629,  3630,  3632,  3634,  3645,
    3646,  3649,  3650,  3662,  3663,  3665,  3666,  3668,  3669,  3675,
    3676,  3679,  3680,  3681,  3707,  3708,  3711,  3712,  3713,  3716,
    3716,  3724,  3726,  3727,  3729,  3730,  3732,  3733,  3735,  3736,
    3738,  3739,  3741,  3742,  3744,  3745,  3748,  3749,  3752,  3754,
    3755,  3759,  3759,  3768,  3770,  3771,  3772,  3773,  3774,  3775,
    3776,  3778,  3779,  3780,  3781,  3782,  3783,  3784,  3785,  3786,
    3787,  3788,  3789,  3790,  3791,  3792,  3793,  3794,  3795,  3796,
    3797,  3798,  3799,  3800,  3801,  3802,  3803,  3804,  3805,  3806,
    3807,  3808,  3809,  3810,  3811,  3812,  3813,  3814,  3815,  3816,
    3817,  3818,  3819,  3820,  3821,  3822,  3823,  3824,  3825,  3826,
    3827,  3828,  3829,  3830,  3831,  3832,  3833,  3834,  3835,  3836,
    3837,  3838,  3839,  3840,  3841,  3842,  3843,  3844,  3845,  3846,
    3847,  3848,  3849,  3850,  3851,  3852,  3853,  3854,  3855,  3858,
    3867
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
  "revision_date", "belongs_to_stmt", "$@12", "$@13", "prefix_stmt",
  "meta_stmts", "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_stmt", "$@14",
  "revision_end", "$@15", "revision_opt_stmt", "date_arg_str", "$@16",
  "body_stmts", "body_stmt", "extension_stmt", "$@17", "extension_end",
  "extension_opt_stmt", "argument_stmt", "$@18", "argument_end",
  "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@19", "$@20", "status_arg_str", "feature_stmt",
  "$@21", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@22", "identity_end", "$@23", "identity_opt_stmt",
  "base_stmt", "typedef_stmt", "$@24", "typedef_arg_str", "type_stmt",
  "$@25", "type_opt_stmt", "$@26", "type_end", "type_body_stmts",
  "decimal_string_restrictions", "union_spec", "fraction_digits_stmt",
  "fraction_digits_arg_str", "length_stmt", "length_arg_str", "length_end",
  "message_opt_stmt", "pattern_stmt", "pattern_arg_str", "pattern_end",
  "enum_specification", "$@27", "enum_stmts", "enum_stmt", "enum_arg_str",
  "enum_end", "enum_opt_stmt", "value_stmt", "integer_value_arg_str",
  "range_stmt", "range_end", "path_stmt", "require_instance_stmt",
  "require_instance_arg_str", "bits_specification", "$@28", "bit_stmts",
  "bit_stmt", "bit_arg_str", "bit_end", "bit_opt_stmt", "position_stmt",
  "position_value_arg_str", "error_message_stmt", "error_app_tag_stmt",
  "units_stmt", "default_stmt", "grouping_stmt", "$@29", "grouping_end",
  "grouping_opt_stmt", "data_def_stmt", "container_stmt", "$@30",
  "container_end", "container_opt_stmt", "$@31", "$@32", "$@33", "$@34",
  "$@35", "leaf_stmt", "$@36", "$@37", "leaf_opt_stmt", "$@38", "$@39",
  "$@40", "$@41", "leaf_list_stmt", "$@42", "$@43", "leaf_list_opt_stmt",
  "$@44", "$@45", "$@46", "$@47", "list_stmt", "$@48", "$@49",
  "list_opt_stmt", "$@50", "$@51", "$@52", "$@53", "$@54", "choice_stmt",
  "$@55", "choice_end", "$@56", "choice_opt_stmt", "$@57", "$@58",
  "short_case_case_stmt", "short_case_stmt", "case_stmt", "$@59",
  "case_end", "case_opt_stmt", "$@60", "$@61", "anyxml_stmt", "$@62",
  "anyxml_end", "anyxml_opt_stmt", "$@63", "$@64", "uses_stmt", "$@65",
  "uses_end", "uses_opt_stmt", "$@66", "$@67", "$@68", "refine_stmt",
  "$@69", "refine_end", "refine_arg_str", "refine_body_opt_stmts",
  "uses_augment_stmt", "$@70", "$@71", "uses_augment_arg_str",
  "augment_stmt", "$@72", "$@73", "augment_opt_stmt", "$@74", "$@75",
  "$@76", "augment_arg_str", "rpc_stmt", "$@77", "rpc_end", "rpc_opt_stmt",
  "$@78", "$@79", "$@80", "$@81", "input_stmt", "$@82", "$@83",
  "input_output_opt_stmt", "$@84", "$@85", "$@86", "output_stmt", "$@87",
  "$@88", "notification_stmt", "$@89", "notification_end",
  "notification_opt_stmt", "$@90", "$@91", "$@92", "deviation_stmt",
  "$@93", "$@94", "deviation_opt_stmt", "deviation_arg_str",
  "deviate_body_stmt", "deviate_stmts", "deviate_not_supported_stmt",
  "deviate_add_stmt", "$@95", "deviate_add_end", "deviate_add_opt_stmt",
  "deviate_delete_stmt", "$@96", "deviate_delete_end",
  "deviate_delete_opt_stmt", "deviate_replace_stmt", "$@97",
  "deviate_replace_end", "deviate_replace_opt_stmt", "when_stmt", "$@98",
  "when_end", "when_opt_stmt", "config_stmt", "config_read_stmt", "$@99",
  "$@100", "config_arg_str", "mandatory_stmt", "mandatory_read_stmt",
  "$@101", "$@102", "mandatory_arg_str", "presence_stmt",
  "min_elements_stmt", "min_value_arg_str", "max_elements_stmt",
  "max_value_arg_str", "ordered_by_stmt", "ordered_by_arg_str",
  "must_stmt", "$@103", "must_end", "unique_stmt", "unique_arg_str",
  "unique_arg", "key_stmt", "key_arg_str", "$@104", "key_opt", "$@105",
  "range_arg_str", "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@106",
  "path_arg_str", "$@107", "$@108", "absolute_path", "absolute_paths",
  "absolute_path_opt", "relative_path", "relative_path_part1",
  "relative_path_part1_opt", "descendant_path", "descendant_path_opt",
  "path_predicate", "path_equality_expr", "path_key_expr",
  "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@109", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@110", "strings", "identifier", "identifiers",
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

#define YYPACT_NINF -804

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-804)))

#define YYTABLE_NINF -588

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -804,    25,  -804,  -804,   145,  -804,  -804,  -804,   285,   285,
    -804,  -804,  2973,  2973,   285,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
     -47,   285,   -28,   285,  -804,  -804,   -14,   329,   329,  -804,
    -804,    52,  -804,  -804,    64,   403,   285,   285,   285,   285,
    -804,  -804,  -804,  -804,  -804,    70,  -804,  -804,   211,    92,
    -804,  2329,  2973,    31,   218,   218,   285,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,   245,  -804,  -804,  -804,   245,  -804,  -804,  -804,   245,
     285,   285,  -804,  -804,    60,    60,  2973,   285,  2421,   285,
    -804,  -804,  -804,  -804,  -804,   285,  -804,  -804,  2973,  2973,
     285,   285,   285,   285,  -804,  -804,  -804,  -804,   102,   102,
    -804,  -804,   285,    24,  -804,    34,    50,   329,   285,   285,
    -804,  -804,  -804,  2329,  2329,  2329,  2329,   285,  -804,   960,
    1270,    41,   297,  -804,  -804,  -804,    96,   328,   245,   245,
     245,   245,    12,   329,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,   426,   329,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  2973,     9,  2973,
    2973,  2973,     9,  2973,  2973,  2973,  2973,  2973,  2973,  2973,
    2973,  2973,  2513,   285,   329,   285,   179,  2421,   285,  -804,
     179,   178,   329,  -804,   342,  -804,  3065,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,   104,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,   112,   285,  -804,   285,   148,  -804,   285,
    -804,  -804,  -804,   369,  -804,   153,   166,   285,   384,   400,
     429,   156,   285,   440,   447,   450,   173,   212,   225,   451,
     464,  -804,   467,   285,   285,   185,  -804,   216,   319,  -804,
     329,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,   329,  -804,  -804,  -804,  -804,   285,
     243,  -804,  -804,  -804,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   417,   329,
     329,  -804,    26,    23,   326,  1372,   885,   325,   184,   169,
     213,   591,   266,  1493,  1385,  1620,   409,   590,   285,   285,
     285,  -804,  -804,  -804,   257,   299,  -804,  -804,   316,   329,
    -804,  -804,  -804,  -804,   285,   285,   285,   285,   285,   285,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,   285,
    -804,  -804,  -804,  -804,  -804,  -804,   284,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,   288,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,   285,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,   285,  -804,
    -804,  -804,  -804,  -804,   285,  -804,  -804,   293,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
     285,  -804,  -804,  -804,   302,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,   305,   299,  -804,  -804,  -804,  -804,   285,   285,
     285,  -804,  -804,  -804,  -804,  -804,   320,   299,  -804,  -804,
    -804,  -804,  -804,  -804,   285,   285,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,   321,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  2329,    -1,  2329,  -804,   285,  -804,   285,   285,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  2513,  -804,
    2329,  -804,  2329,  -804,  -804,  2973,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  2329,  -804,  -804,  -804,  -804,  -804,  2973,
     331,  -804,   329,   329,   329,  2513,  -804,  -804,  -804,  -804,
    -804,    68,   190,     6,  -804,  -804,  -804,  -804,  2605,  2605,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,   285,
     285,  -804,  -804,  -804,  -804,   245,  -804,  -804,  -804,  -804,
     245,   245,  2513,   329,  2605,  2605,  -804,  -804,  -804,    38,
     245,    87,  -804,    -1,  -804,   329,   329,  -804,   329,   329,
     329,   329,   329,   245,   329,   329,   329,   329,   329,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
     245,  -804,   329,   329,  -804,  -804,  -804,   245,  -804,  -804,
    -804,   245,  -804,  -804,  -804,  -804,  -804,   245,  -804,   329,
     329,   245,  -804,  -804,   245,  -804,    26,  -804,   329,   329,
     329,   329,   329,   329,   329,   329,   333,   339,   329,   329,
     329,   329,  -804,   285,   285,   285,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,   329,   329,   329,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,   481,  -804,   484,
     490,  -804,   509,   285,   285,    26,   285,   285,  -804,   329,
     285,  -804,   285,  -804,   285,   285,   285,  -804,   329,  -804,
      26,  -804,  -804,  -804,  3065,  -804,  -804,  -804,   513,   353,
     285,   527,   285,   285,   285,   245,   285,   285,   245,  -804,
    -804,  -804,   245,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,   528,   537,  -804,   550,  -804,  -804,  3065,    26,
     166,   329,   329,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,   329,  -804,   329,   329,    61,   329,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,   512,
     512,   675,   329,   329,   214,   114,  1091,   285,   344,   329,
     329,   329,    26,  -804,  -804,  -804,   360,   373,   285,   285,
    -804,   376,   543,  -804,   363,  -804,  -804,  -804,   443,  1372,
     187,   285,   285,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,    99,  -804,
     268,   186,   402,  -804,  -804,  -804,  -804,  -804,  -804,   160,
     109,  -804,   285,   285,   285,   285,   299,  -804,  -804,  -804,
    -804,   285,  -804,   285,  -804,   380,   285,   285,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  2329,  2329,
    -804,  -804,  -804,  -804,  -804,   245,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,   329,   329,
     329,  -804,   245,   387,   412,  -804,  -804,  -804,   245,   132,
    2329,  2329,  2329,  -804,   329,  2329,   329,  2973,   443,  -804,
      38,    87,   329,   245,   245,   329,   329,   285,   285,  -804,
     329,   329,   329,  -804,  3065,  -804,  -804,   396,  -804,  -804,
     285,   285,  -804,  -804,   245,  -804,   554,  -804,   557,  -804,
     562,  -804,   329,   563,  -804,   363,   580,  -804,  -804,   245,
     245,  -804,  -804,  -804,   387,  -804,  2697,  -804,   285,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,   420,  -804,  -804,
    -804,   249,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   517,  -804,   261,   271,   345,   354,   478,   406,  -804,
    3065,  -804,  -804,  -804,  -804,   285,  -804,  -804,  -804,  -804,
    -804,   285,  -804,  -804,  -804,  -804,  -804,   517,   517,   298,
     190,   425,   437,  -804,  -804,  -804,  -804,   245,  -804,  -804,
    -804,   245,  -804,  -804,   517,  -804,   285,  -804,   285,   458,
     517,  -804,   517,   453,   465,   517,   517,   469,   570,  -804,
     517,  -804,  -804,   496,  2789,   517,  -804,  -804,  -804,  2881,
    -804,   507,   517,  3065,  -804
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
     725,   726,   727,   728,   608,     9,   729,   636,    16,   636,
       0,   607,     0,     6,   616,   616,     5,    12,    19,   636,
     619,    10,   618,   617,    17,     0,   622,     0,     0,     0,
      28,    13,    14,    15,    28,     0,    20,     7,     0,   624,
     623,     0,     0,     0,    47,    47,     0,    22,   636,   636,
     628,   620,   636,   621,   644,   647,   645,   649,   650,   648,
     643,     0,   641,   646,   605,     0,   636,    24,    26,     0,
       0,     0,    29,    30,    56,    56,     0,     8,   630,   626,
     616,   616,    27,   636,    46,   606,   636,    23,     0,     0,
       0,     0,     0,     0,    48,    49,    50,    51,    69,    69,
      43,   610,   622,     0,   609,   625,     0,   614,   642,    25,
      31,    34,    35,     0,     0,     0,     0,     0,   616,     0,
       0,     0,     0,   629,   636,   616,     0,     0,     0,     0,
       0,     0,     0,    57,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     636,   616,    71,    72,    73,    74,    75,    76,   226,   227,
     228,   229,   230,   231,   232,    77,    78,    79,    80,   636,
     616,   636,   636,   631,     0,   615,   616,   616,    37,   616,
      53,    54,    52,    55,    66,    68,    58,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    70,    18,     0,   630,   632,   636,
       0,    39,    36,   636,     0,   354,     0,   420,   405,   567,
     636,   317,   233,    81,   474,   466,   636,   106,   215,   116,
     254,   273,   293,   451,   421,     0,   129,   730,   613,   369,
     636,   636,    44,     0,   627,    32,     0,     0,    40,    67,
     616,    60,    59,     0,   565,     0,   566,   419,     0,     0,
       0,     0,   473,     0,     0,     0,     0,     0,     0,     0,
       0,   616,     0,   611,   612,     0,   636,    39,     0,    38,
      61,   616,   356,   355,   616,   568,   616,   319,   318,   616,
     235,   234,   616,    83,    82,   616,   616,   108,   107,   616,
     217,   216,   616,   118,   117,   616,   616,   616,   616,   453,
     452,   616,   423,   422,   132,   616,   371,   370,   616,   633,
       0,    42,   636,    63,   358,   408,   322,   237,    85,   469,
     110,   219,   121,   257,   276,   296,   455,   425,   127,   373,
      45,   616,     0,     0,     0,   406,   320,     0,     0,   467,
       0,     0,   119,   255,   274,   294,     0,     0,     0,     0,
       0,   138,   139,   137,     0,     0,   135,   136,     0,    33,
      41,    62,    64,    65,     0,     0,     0,     0,     0,     0,
     357,   367,   368,   366,   361,   359,   364,   365,   362,     0,
     413,   414,   412,   411,   415,   417,     0,   409,   330,   331,
     329,   325,   326,   336,   337,   338,   339,     0,   332,   334,
     335,   340,   323,   327,   328,     0,   236,   246,   247,   245,
     240,   250,   248,   252,   238,   244,   243,   241,     0,    84,
      88,    89,    86,    87,     0,   470,   471,     0,   109,   113,
     114,   112,   111,   218,   221,   222,   220,   616,   616,   616,
       0,   124,   125,   123,     0,   122,   271,   272,   270,   260,
     264,   267,     0,     0,   258,   268,   269,   265,     0,     0,
       0,   291,   292,   290,   279,   283,     0,     0,   277,   286,
     287,   288,   289,   284,     0,     0,   309,   310,   308,   299,
     311,   313,   315,     0,   297,   304,   305,   306,   307,   300,
     303,   302,   454,   458,   459,   457,   456,   460,   462,   464,
     636,   636,   424,   428,   429,   427,   426,   430,   432,   434,
     436,     0,     0,     0,   128,     0,   616,     0,     0,   372,
     378,   379,   377,   376,   380,   382,   374,   523,     0,   530,
       0,    99,     0,   616,   616,     0,   616,   616,   407,   616,
     321,   616,   616,     0,   616,   616,   616,   616,   616,     0,
       0,   468,   224,   223,   225,     0,   120,   256,   262,   616,
     616,     0,     0,     0,   275,   281,   616,   616,     0,     0,
     616,   616,   616,   295,   616,   616,   616,   616,   616,   438,
     448,   616,   616,   616,   616,     0,   636,   636,   636,   105,
       0,     0,     0,   134,     0,     0,   616,   616,   616,     0,
       0,     0,   548,     0,   515,   360,   363,   341,   416,   418,
     410,   333,   324,     0,   251,   249,   253,   239,   242,    90,
     636,   636,   636,   636,   472,   636,   475,   477,   479,   478,
       0,   616,   259,   266,   598,   636,   543,     0,   636,   599,
     539,     0,   600,   636,   636,   636,   547,     0,   616,   278,
     285,     0,   557,   558,     0,   552,     0,   569,   312,   314,
     316,   298,   301,   461,   463,   465,     0,     0,   431,   433,
     435,   437,   214,   102,   104,   103,    98,   213,   130,   404,
     400,   636,   389,   384,   636,   381,   383,   375,   636,   636,
     528,   524,   115,   636,   636,   535,   531,     0,   100,     0,
       0,   536,     0,   481,   494,     0,   503,   476,   126,   263,
     541,   540,   542,   537,   538,   546,   545,   544,   282,   560,
       0,   554,   553,   556,     0,   567,   616,   616,     0,     0,
     403,     0,   388,   527,   526,     0,   534,   533,     0,   616,
     550,   549,     0,   616,   517,   516,   616,   343,   342,   616,
      92,   616,     0,     0,   480,     0,   559,   563,     0,     0,
     570,   441,   441,   616,   140,   131,   616,   616,   386,   385,
     525,   532,   162,   101,   519,   345,     0,    91,   616,   483,
     482,   616,   496,   495,   616,   505,   504,   561,   555,   439,
     449,   148,   408,   390,     0,     0,     0,     0,     0,   485,
     498,   507,     0,   442,   444,   446,     0,     0,     0,     0,
     145,     0,   142,   143,     0,   144,   146,   147,     0,   401,
       0,     0,     0,   551,   165,   166,   163,   164,   518,   520,
     521,   344,   350,   351,   349,   348,   352,   346,     0,    93,
       0,     0,     0,   562,   616,   616,   616,   440,   450,   571,
       0,   141,     0,     0,     0,     0,     0,   151,   149,   150,
     616,     0,   616,     0,   197,     0,     0,     0,   387,   398,
     399,   393,   394,   395,   392,   396,   397,   616,     0,     0,
     616,   616,   636,   636,    97,     0,   484,   486,   489,   490,
     491,   492,   493,   616,   488,   497,   499,   502,   616,   501,
     506,   616,   509,   510,   511,   512,   513,   514,   443,   445,
     447,   575,     0,     0,     0,   636,   636,   194,     0,     0,
       0,     0,     0,   616,   152,     0,   173,     0,   196,   402,
       0,     0,   391,     0,     0,   353,   347,    96,    95,    94,
     487,   500,   508,   190,     0,   578,   572,     0,   574,   582,
     193,   192,   191,   157,     0,   636,     0,   159,     0,   168,
       0,   564,   153,     0,   176,   172,     0,   200,   198,     0,
       0,   212,   211,   587,   577,   581,     0,   155,   156,   616,
     160,   616,   616,   169,   616,   616,   188,   187,   616,   177,
     175,   616,   616,   201,   616,   522,   529,   576,   579,   583,
     580,   585,   162,   158,   162,   167,   162,   179,   174,   203,
     199,   639,   584,     0,     0,     0,     0,     0,     0,   640,
       0,   586,   161,   170,   189,     0,   178,   182,   183,   181,
     180,     0,   202,   206,   207,   205,   204,   639,   639,     0,
       0,     0,     0,   603,   604,   601,   186,     0,   636,   602,
     210,     0,   636,   588,   639,   184,   185,   208,   209,     0,
     639,   589,   639,     0,     0,   639,   639,     0,     0,   597,
     639,   590,   593,     0,     0,   639,   594,   595,   592,   639,
     591,     0,   639,     0,   596
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -804,  -804,   468,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,   476,  -804,  -804,  -804,   491,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,   252,  -804,  -804,
    -804,  -804,  -804,   -87,   494,  -804,  -804,   823,   919,   463,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,   427,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -426,  -366,
    -804,  -804,   -70,  -804,  -804,  -804,  -804,  -402,  -804,  -804,
    -804,  -804,  -804,  -248,  -304,  -804,  -804,  -540,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -803,  -804,  -804,  -804,  -804,  -804,  -804,  -410,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -363,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -407,
    -417,  -286,  -804,  -804,  -804,  -317,   232,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,   233,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,   242,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,   251,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,   258,
    -804,  -804,  -804,  -804,  -804,   259,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -211,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -169,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,  -804,
    -385,  -804,  -804,  -804,  -365,  -119,  -804,  -804,  -334,  -300,
    -396,  -804,  -804,  -333,  -220,  -434,  -804,  -427,  -804,   256,
    -804,  -422,  -804,  -804,  -570,  -804,  -157,  -804,  -804,  -804,
    -199,  -804,  -804,  -343,   422,  -120,  -666,  -804,  -804,  -804,
    -804,  -368,  -405,  -804,  -804,  -367,  -804,  -804,  -804,  -391,
    -804,  -804,  -804,  -459,  -804,  -804,  -804,  -655,  -424,  -804,
    -804,  -804,   -12,  -115,  -545,   493,  1240,  -804,  -804,   529,
    -804,  -804,  -804,  -804,   428,  -804,    -4,    -3,   637,    54,
    -141,  -804,  -804,  -101,  -123,  -804
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    94,    99,   106,   138,     2,   100,   120,   111,
       3,   102,   124,   114,   125,   121,   159,   176,   122,   134,
     162,   216,   367,   200,   163,   217,   269,   337,   338,   412,
     137,   211,   365,   123,   164,   184,   185,   924,   925,   188,
     208,   304,   342,   413,   433,   276,   303,   209,   241,   242,
     350,   384,   438,   522,   802,   851,   898,   985,   482,   473,
     703,   842,   690,   243,   353,   388,   440,   483,   244,   355,
     394,   544,   442,   545,   245,   454,   325,   616,   828,   428,
     455,   865,   911,   912,   956,   957,  1054,   958,  1056,  1081,
     894,   959,  1058,  1084,   913,   914,  1065,   962,  1063,  1090,
    1117,  1130,  1147,   960,  1087,   915,   916,  1018,   917,   918,
    1028,   964,  1066,  1094,  1118,  1136,  1151,   926,   927,   456,
     457,   246,   354,   391,   441,   247,   248,   349,   381,   437,
     647,   648,   645,   644,   646,   249,   356,   552,   443,   659,
     553,   731,   660,   250,   357,   566,   444,   666,   567,   748,
     667,   251,   358,   583,   445,   674,   675,   670,   671,   672,
     252,   348,   378,   497,   436,   642,   641,   498,   499,   485,
     800,   848,   896,   981,   980,   253,   343,   373,   434,   633,
     634,   254,   362,   407,   458,   698,   696,   697,   624,   831,
     869,   783,   920,   625,   829,   965,   780,   255,   345,   486,
     435,   639,   636,   637,   308,   256,   360,   403,   447,   681,
     682,   683,   684,   609,   766,   906,   889,   944,   945,   946,
     610,   767,   907,   257,   359,   400,   446,   676,   677,   678,
     258,   351,   527,   439,   315,   724,   725,   726,   727,   852,
     880,   940,   728,   853,   883,   941,   729,   855,   886,   942,
     487,   799,   845,   895,   972,   476,   699,   835,   791,   973,
     477,   701,   838,   796,   516,   570,   741,   571,   737,   572,
     747,   478,   797,   841,   590,   755,   822,   591,   752,   820,
     856,   902,  1060,   309,   310,   346,   756,   825,  1012,  1013,
    1014,  1045,  1046,  1074,  1048,  1049,  1076,  1100,  1112,  1097,
    1137,  1161,  1171,  1172,  1174,  1179,  1162,   742,   743,  1148,
    1149,   155,    95,   757,   329,   823,   107,   112,   116,   128,
     129,   143,   195,   141,   193,   263,   113,     4,   130,  1120,
     151,   173,   152,    96,    97,   331
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    98,   453,   375,    11,    11,   738,    16,    14,   156,
      10,   586,   523,   658,    16,   517,   543,    16,   587,   492,
      16,   557,   573,   589,   274,     5,   551,   665,   781,   784,
     153,     6,   474,     7,   491,   510,   550,   565,   532,    16,
     504,   549,   564,   579,   596,   606,    16,   556,   157,   475,
     104,   502,   514,   192,   181,     6,   623,     7,   554,   568,
     584,   110,   218,   219,   220,   221,     6,   194,     7,   105,
     490,   509,   110,   626,   531,   536,    16,   548,   563,   578,
     595,   605,   109,   700,   686,   734,   687,   -21,   180,   688,
     183,   181,   622,   136,   101,    16,   103,    10,   744,    10,
     142,   745,   153,   153,   153,   153,   115,    16,   117,   306,
     730,    10,    11,    11,    11,    11,   118,    16,   484,   182,
     513,   461,   213,   170,   539,   171,   788,   183,   582,   599,
     214,   789,    11,   511,   119,   167,   168,   537,   260,   169,
      16,   580,   597,   607,   897,   181,   119,   778,   215,   734,
       6,   512,     7,   175,   190,   538,    11,    11,   859,   581,
     598,   608,   735,    10,    10,    10,   201,   202,    16,   330,
     198,    10,   207,   199,  -573,   793,    11,    11,    11,    11,
     794,   183,   192,     6,     6,     7,     7,   982,    11,   110,
     110,   344,   983,   266,    10,    10,   194,  1015,    16,     8,
     181,   361,  1016,    11,   518,   194,   524,   734,   739,   332,
     366,   264,   928,   335,   966,   181,   448,   448,   181,     9,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,   183,   293,   967,   558,
     559,   467,   467,   118,   181,   181,   369,   921,   922,   336,
     374,   183,   505,   385,   183,   465,   295,   449,   297,   298,
      10,   160,   161,   575,   450,   305,   306,   311,   312,   313,
     395,   317,   318,   319,   320,   321,   322,   323,   324,   326,
     183,   183,   519,   408,   995,   968,   468,   336,   540,    10,
       6,    10,     7,    10,    10,   966,   334,   181,   448,  1114,
     339,  1115,   181,  1116,   921,   922,    16,   347,   139,   396,
     140,   528,   923,   352,  1143,  1144,  1145,   503,   515,   967,
     558,   559,   397,   467,   555,   569,   585,   363,   364,   224,
      10,   411,    11,   183,     6,    10,     7,   617,   183,   449,
     110,   431,   170,    10,   171,   575,   450,   181,    10,  -587,
    -587,   226,   464,   464,   227,   614,   181,   181,   465,    10,
      10,  1044,  1111,   409,  1055,   231,   986,   465,   465,  1122,
     994,   999,   233,   234,   615,   235,   181,   466,   921,   922,
     467,   467,   638,   183,   618,   181,   640,   921,   922,   468,
     505,   651,   183,   183,   261,   961,   262,   469,   468,   468,
     656,   238,  1001,   657,   239,    10,   469,   469,     6,   432,
       7,   127,   183,   224,   619,   720,  1023,   721,   664,   673,
     722,   183,   723,   506,   470,   267,  1047,   268,    10,   966,
     826,     6,   448,     7,   299,   226,   827,   181,   227,   340,
     181,   341,   939,  1123,    11,    11,    11,   448,   181,   231,
     866,   465,  1124,   967,   558,   559,   233,   234,   947,   235,
      11,    11,    11,    11,    11,    11,   371,   963,   372,  1131,
     685,   948,   691,   183,   951,    11,   183,   615,  1029,   449,
     450,   376,   468,   377,   183,   238,   975,  1044,   239,   702,
     449,   704,  -133,   976,   935,   450,  1075,   379,   977,   380,
    1000,    11,   713,   971,  1132,   330,   991,   592,  1006,   181,
     153,   937,   153,   992,    11,  1007,   224,   375,   993,   998,
      11,  1111,  1119,   988,   997,  1003,   382,  1153,   383,   153,
     934,   153,   330,   987,   996,  1002,    11,   386,   226,   387,
    1154,   227,   153,  1160,   389,   183,   390,   392,   398,   393,
     399,   449,   231,   753,    11,    11,    11,  1165,  1125,   233,
     234,   401,   235,   402,   405,  1166,   406,   194,   194,   330,
      11,    11,   905,   905,  1169,   989,  1126,  1004,   839,   936,
     840,   843,   952,   844,  1170,   903,   903,   846,   238,   847,
     126,   239,   953,   194,   194,   224,  1175,   679,   680,   150,
     154,   158,   484,   904,   904,   954,   849,  1182,   850,   955,
     863,    11,   864,    11,    11,   135,   210,   226,  -154,   410,
     227,   181,   181,   707,   867,   878,   868,   879,   189,   165,
     231,   231,   465,   798,   881,   600,   882,   719,   233,   234,
     990,   235,  1005,   910,   172,    12,    13,   884,   174,   885,
     601,  1079,   177,  1080,  1082,  1091,  1083,   183,   183,  1085,
    1088,  1086,  1089,   468,   468,  1068,   238,   238,   493,   494,
     239,   150,   150,   150,   150,    10,    10,  1092,   495,  1093,
       6,   919,     7,   773,   774,   775,   110,   496,   602,   533,
     275,  1129,  1135,   890,   500,   501,  1069,   540,  1070,  -195,
     974,   588,   888,   943,   316,   860,  1098,  -171,  1121,  1099,
    1113,   270,   271,   272,   273,  1176,  1152,   803,   804,   805,
     806,   212,   807,   194,     0,   333,     0,     0,     0,     0,
       0,     0,   810,     0,     0,   812,   908,     0,     0,     0,
     814,   815,   816,   887,   909,     0,   307,     0,     0,     0,
     314,     0,    11,     0,   131,   132,   133,   194,     0,     0,
     328,     0,     0,     0,     0,     0,     0,     0,     0,    10,
      10,    10,     0,   166,     0,     0,     0,     0,   830,     0,
       0,   832,     0,     0,     0,   833,   834,     0,     0,     0,
     836,   837,     0,     0,     0,     0,     0,   178,   179,    10,
      10,    10,    10,    10,     0,     0,    10,     0,    10,     0,
      10,    10,    10,     0,     0,     0,    11,   203,   204,   205,
     206,     0,     0,     0,     0,     0,    10,     0,    10,    10,
      10,     0,    10,    10,     0,     0,     0,  1033,  1034,     0,
       0,     0,     0,     0,   222,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,     0,     0,     0,     0,
       0,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   153,   153,  1057,
    1059,  1061,     0,     0,  1064,     0,     0,     0,     0,   224,
       0,     0,     0,    11,     0,     0,     0,     0,    11,     0,
       0,     0,     0,     0,    11,    11,     0,     0,     0,     0,
     479,     0,   464,     0,   227,   448,   181,    11,    11,   153,
     153,   153,     0,     0,   153,   460,     0,   465,     0,  1073,
       0,     0,   233,   234,     0,   235,   466,     0,     0,     0,
       0,     0,     0,   194,     0,     0,     0,     0,    11,    11,
      11,    11,   183,     0,     0,     0,     0,    11,   468,    11,
       0,  1101,    11,    11,   224,     0,   469,     0,     0,     0,
       0,     0,     0,   368,     0,   194,     0,     0,     0,  1037,
    1038,   225,     0,     0,     0,     0,   226,   186,   186,   227,
       0,     0,     0,     0,     0,   228,   229,     0,   230,     0,
     231,   232,     0,     0,     0,  1138,     0,   233,   234,     0,
     235,     0,  1050,  1051,     0,  1067,     0,   236,     0,   194,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   237,    10,    10,     0,   238,     0,     0,   239,
       0,     0,     0,     0,     0,     0,    10,    10,     0,     0,
       0,     0,  1078,     0,     0,     0,     0,     0,   240,  1177,
       0,     0,     0,     0,  1180,     0,     0,     0,  1184,     0,
       0,     0,     0,   194,    10,     0,     0,     0,   194,   150,
     689,   150,   194,   187,   187,   611,   612,   613,     0,     0,
       0,     0,     0,     0,     0,   224,   328,     0,   150,     0,
     150,   627,   628,   629,   630,   631,   632,     0,     0,     0,
       0,   150,     0,     0,     0,     0,   635,   226,     0,     0,
     227,    11,   181,   328,     0,     0,     0,    11,     0,   736,
     740,   746,     0,   465,     0,     0,   751,   754,   233,   234,
       0,   235,   643,     0,     0,  1156,     0,     0,     0,  1158,
       0,     0,    10,     0,    10,   649,     0,     0,   183,     0,
     328,   650,   779,   782,   468,     0,     0,   790,     0,   795,
     239,   689,   469,     0,     0,     0,     0,   655,   772,     0,
       0,     0,     0,   776,   777,     0,     0,     0,     0,   931,
       0,  1141,  1142,   792,     0,   661,   662,   663,     0,     0,
       0,     0,     0,     0,     0,     0,   801,     0,  1159,     0,
       0,   668,   669,     0,  1163,     0,  1164,     0,     0,  1167,
    1168,     0,     0,   808,  1173,     0,     0,     0,     0,  1178,
     811,     0,     0,  1181,   813,     0,  1183,     0,     0,     0,
     817,     0,     0,     0,   819,     0,     0,   821,     0,     0,
       0,   451,   692,     0,   694,   695,   462,   471,   480,   488,
     507,   520,   525,   529,   534,   541,   546,   561,   576,   593,
     603,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,   620,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   225,     0,     0,     0,     0,   226,     0,   854,   227,
       0,     0,     0,     0,     0,   228,   229,     0,   230,     0,
     231,   232,     0,   857,     0,     0,     0,   233,   234,     0,
     235,     0,     0,     0,     0,     0,     0,   236,   870,     0,
       0,   871,     0,     0,     0,   873,     0,     0,     0,     0,
       0,     0,   237,     0,     0,   108,   238,   452,     0,   239,
       0,     0,   463,   472,   481,   489,   508,   521,   526,   530,
     535,   542,   547,   562,   577,   594,   604,     0,   259,     0,
       0,     0,     0,     0,     0,     0,   224,   621,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   824,     0,   857,     0,   479,   226,     0,
       0,   227,     0,   181,     0,     0,   984,     0,     0,     0,
     196,   197,   464,     0,   465,     0,   181,  1011,  1017,   233,
     234,     0,   235,     0,     0,     0,     0,   465,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   558,   559,   183,
     467,     0,     0,   560,     0,   449,   150,   150,   223,     0,
       0,   239,   183,   469,     0,   265,     0,   858,   468,     0,
    -280,     0,     0,   450,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1039,     0,
       0,   294,     0,     0,     0,     0,     0,  1053,   150,   150,
     150,     0,     0,   150,     0,     0,   824,     0,   790,   795,
     296,     0,     0,     0,     0,  1043,   300,   301,     0,   302,
       0,  1052,     0,     0,     0,     0,     0,     0,     0,     0,
     464,     0,     0,   448,   181,     0,  1071,  1072,     0,     0,
       0,     0,     0,     0,   938,   465,     0,     0,     0,   858,
       0,     0,     0,     0,   466,   949,   950,  1077,   467,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   978,   979,
     183,     0,  1095,  1096,     0,     0,   468,     0,  -261,     0,
       0,   450,     0,     0,   469,     0,     0,     0,     0,     0,
     370,     0,     0,     0,     0,     0,     0,     0,     0,  1019,
    1020,  1021,  1022,     0,     0,     0,     0,     0,  1025,     0,
    1027,   404,     0,  1030,  1031,     0,     0,  1146,  1150,     0,
       0,   414,     0,     0,   415,     0,   416,     0,     0,   417,
       0,     0,   418,     0,   224,   419,   420,     0,     0,   421,
       0,     0,   422,     0,     0,   423,   424,   425,   426,     0,
    1155,   427,     0,     0,  1157,   429,   226,   464,   430,   227,
       0,   181,     0,     0,     0,     0,     0,     0,     0,     0,
     231,     0,   465,     0,     0,     0,   574,   233,   234,     0,
     235,   459,   558,   559,     0,   467,     0,     0,   560,     0,
       0,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,   468,     0,     0,   238,   575,     0,   239,
       0,   469,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   929,   932,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   480,   969,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1139,     0,     0,     0,     0,     0,  1140,     0,
       0,     0,     0,     0,     0,     0,     0,   652,   653,   654,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   930,   933,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   481,   970,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   693,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   705,   706,     0,   708,   709,     0,   710,
       0,   711,   712,     0,   714,   715,   716,   717,   718,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   732,
     733,     0,     0,     0,     0,     0,   749,   750,     0,     0,
     758,   759,   760,     0,   761,   762,   763,   764,   765,     0,
       0,   768,   769,   770,   771,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   785,   786,   787,     0,
    1127,  1133,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   809,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   818,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1128,  1134,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   861,   862,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   872,
       0,     0,     0,   874,     0,     0,   875,     0,     0,   876,
       0,   877,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   891,     0,     0,   892,   893,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   899,     0,
       0,   900,     0,     0,   901,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1008,  1009,  1010,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1024,     0,  1026,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1032,     0,     0,
    1035,  1036,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1040,     0,     0,     0,     0,  1041,     0,
       0,  1042,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1062,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1102,
       0,  1103,  1104,     0,  1105,  1106,     0,     0,  1107,     0,
       0,  1108,  1109,    15,  1110,     0,     0,    16,   144,    17,
     145,   146,     0,     0,     0,   147,   148,   149,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    15,     6,     0,     7,     0,
       0,    17,   191,     0,     0,     0,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    15,     0,     0,
       0,    16,     0,    17,   327,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    15,
       0,     0,     0,    16,     0,    17,   191,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    15,     0,     0,     0,     0,     0,    17,   191,     0,
       0,  1047,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    15,     0,     0,     0,     0,     0,    17,
     191,     0,     0,  1170,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    15,  1119,     0,     0,     0,
       0,    17,   191,     0,     0,     0,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    15,     0,     0,
       0,    16,     0,    17,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    15,
       0,     0,     0,     0,     0,    17,   191,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93
};

static const yytype_int16 yycheck[] =
{
       4,    13,   428,   346,     8,     9,   661,     8,    11,   132,
      14,   445,   438,   553,     8,   437,   442,     8,   445,   436,
       8,   443,   444,   445,    12,     0,   443,   567,   694,   695,
     131,     5,   434,     7,   436,   437,   443,   444,   440,     8,
     436,   443,   444,   445,   446,   447,     8,   443,    17,   434,
      97,   436,   437,   168,    31,     5,   458,     7,   443,   444,
     445,    11,   203,   204,   205,   206,     5,   168,     7,    97,
     436,   437,    11,   458,   440,   441,     8,   443,   444,   445,
     446,   447,    96,   628,    85,    17,    87,    23,    28,    90,
      67,    31,   458,    23,    97,     8,    99,   101,    92,   103,
       8,    95,   203,   204,   205,   206,   109,     8,    56,   100,
     655,   115,   116,   117,   118,   119,    64,     8,   435,    59,
     437,    98,    98,    97,   441,    99,    88,    67,   445,   446,
      96,    93,   136,   437,    82,   138,   139,   441,    97,   142,
       8,   445,   446,   447,    83,    31,    82,   692,    98,    17,
       5,   437,     7,   156,   166,   441,   160,   161,   824,   445,
     446,   447,    94,   167,   168,   169,   178,   179,     8,   292,
     173,   175,    70,   176,    14,    88,   180,   181,   182,   183,
      93,    67,   297,     5,     5,     7,     7,    88,   192,    11,
      11,   306,    93,    97,   198,   199,   297,    88,     8,    54,
      31,    97,    93,   207,    20,   306,    37,    17,    18,   296,
      98,   214,    98,   300,    27,    31,    30,    30,    31,    74,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,    67,   240,    51,    52,
      53,    55,    55,    64,    31,    31,    98,    33,    34,    71,
      97,    67,    65,    97,    67,    42,   259,    73,   261,   262,
     264,    43,    44,    77,    78,   277,   100,   279,   280,   281,
      97,   283,   284,   285,   286,   287,   288,   289,   290,   291,
      67,    67,    98,    98,    98,    98,    73,    71,    22,   293,
       5,   295,     7,   297,   298,    27,   299,    31,    30,  1102,
     303,  1104,    31,  1106,    33,    34,     8,   310,    97,    97,
      99,    98,    98,   316,    16,    17,    18,   436,   437,    51,
      52,    53,    97,    55,   443,   444,   445,   330,   331,     4,
     334,    12,   336,    67,     5,   339,     7,    21,    67,    73,
      11,    98,    97,   347,    99,    77,    78,    31,   352,   100,
     101,    26,    27,    27,    29,    98,    31,    31,    42,   363,
     364,   100,   101,   366,  1019,    40,    98,    42,    42,    98,
     940,   941,    47,    48,    75,    50,    31,    51,    33,    34,
      55,    55,    98,    67,    68,    31,    98,    33,    34,    73,
      65,    98,    67,    67,    97,    32,    99,    81,    73,    73,
      98,    76,   942,    98,    79,   409,    81,    81,     5,   412,
       7,     8,    67,     4,    98,    84,   956,    86,    98,    98,
      89,    67,    91,    98,    98,    97,    14,    99,   432,    27,
      97,     5,    30,     7,     8,    26,    97,    31,    29,    97,
      31,    99,    98,    98,   448,   449,   450,    30,    31,    40,
      97,    42,    98,    51,    52,    53,    47,    48,    98,    50,
     464,   465,   466,   467,   468,   469,    97,    24,    99,    63,
     611,    98,   613,    67,    98,   479,    67,    75,    98,    73,
      78,    97,    73,    99,    67,    76,   920,   100,    79,   630,
      73,   632,    75,   920,   896,    78,   100,    97,   920,    99,
      98,   505,   643,   920,    98,   628,   940,    98,   942,    31,
     611,   896,   613,   940,   518,   942,     4,   860,   940,   941,
     524,   101,     5,   940,   941,   942,    97,   102,    99,   630,
     896,   632,   655,   940,   941,   942,   540,    97,    26,    99,
     103,    29,   643,    85,    97,    67,    99,    97,    97,    99,
      99,    73,    40,   668,   558,   559,   560,   104,    80,    47,
      48,    97,    50,    99,    97,   100,    99,   668,   669,   692,
     574,   575,   889,   890,   105,   940,    98,   942,    97,   896,
      99,    97,    39,    99,    14,   889,   890,    97,    76,    99,
     114,    79,    49,   694,   695,     4,   100,   600,   601,   131,
     132,   133,   919,   889,   890,    62,    97,   100,    99,    66,
      97,   615,    99,   617,   618,   124,   189,    26,    75,   367,
      29,    31,    31,   635,    97,    97,    99,    99,   165,   135,
      40,    40,    42,   703,    97,    45,    99,   649,    47,    48,
     940,    50,   942,   891,   151,     8,     9,    97,   155,    99,
      60,    97,   159,    99,    97,  1065,    99,    67,    67,    97,
      97,    99,    99,    73,    73,  1028,    76,    76,   436,   436,
      79,   203,   204,   205,   206,   679,   680,    97,   436,    99,
       5,   892,     7,   686,   687,   688,    11,   436,    98,    98,
     222,  1117,  1118,   862,   436,   436,  1030,    22,  1031,    24,
     920,   445,   859,   902,   282,   825,  1074,    32,  1113,  1076,
    1101,   218,   219,   220,   221,  1174,  1140,   720,   721,   722,
     723,   192,   725,   824,    -1,   297,    -1,    -1,    -1,    -1,
      -1,    -1,   735,    -1,    -1,   738,    61,    -1,    -1,    -1,
     743,   744,   745,   858,    69,    -1,   278,    -1,    -1,    -1,
     282,    -1,   756,    -1,   117,   118,   119,   858,    -1,    -1,
     292,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   773,
     774,   775,    -1,   136,    -1,    -1,    -1,    -1,   781,    -1,
      -1,   784,    -1,    -1,    -1,   788,   789,    -1,    -1,    -1,
     793,   794,    -1,    -1,    -1,    -1,    -1,   160,   161,   803,
     804,   805,   806,   807,    -1,    -1,   810,    -1,   812,    -1,
     814,   815,   816,    -1,    -1,    -1,   820,   180,   181,   182,
     183,    -1,    -1,    -1,    -1,    -1,   830,    -1,   832,   833,
     834,    -1,   836,   837,    -1,    -1,    -1,   978,   979,    -1,
      -1,    -1,    -1,    -1,   207,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   859,    -1,    -1,    -1,    -1,
      -1,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   978,   979,  1020,
    1021,  1022,    -1,    -1,  1025,    -1,    -1,    -1,    -1,     4,
      -1,    -1,    -1,   897,    -1,    -1,    -1,    -1,   902,    -1,
      -1,    -1,    -1,    -1,   908,   909,    -1,    -1,    -1,    -1,
      25,    -1,    27,    -1,    29,    30,    31,   921,   922,  1020,
    1021,  1022,    -1,    -1,  1025,   432,    -1,    42,    -1,  1044,
      -1,    -1,    47,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,  1044,    -1,    -1,    -1,    -1,   952,   953,
     954,   955,    67,    -1,    -1,    -1,    -1,   961,    73,   963,
      -1,  1076,   966,   967,     4,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,   336,    -1,  1076,    -1,    -1,    -1,   982,
     983,    21,    -1,    -1,    -1,    -1,    26,   164,   165,    29,
      -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,    -1,
      40,    41,    -1,    -1,    -1,  1120,    -1,    47,    48,    -1,
      50,    -1,  1015,  1016,    -1,  1027,    -1,    57,    -1,  1120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,  1037,  1038,    -1,    76,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,  1050,  1051,    -1,    -1,
      -1,    -1,  1055,    -1,    -1,    -1,    -1,    -1,    98,  1174,
      -1,    -1,    -1,    -1,  1179,    -1,    -1,    -1,  1183,    -1,
      -1,    -1,    -1,  1174,  1078,    -1,    -1,    -1,  1179,   611,
     612,   613,  1183,   164,   165,   448,   449,   450,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,   628,    -1,   630,    -1,
     632,   464,   465,   466,   467,   468,   469,    -1,    -1,    -1,
      -1,   643,    -1,    -1,    -1,    -1,   479,    26,    -1,    -1,
      29,  1125,    31,   655,    -1,    -1,    -1,  1131,    -1,   661,
     662,   663,    -1,    42,    -1,    -1,   668,   669,    47,    48,
      -1,    50,   505,    -1,    -1,  1148,    -1,    -1,    -1,  1152,
      -1,    -1,  1156,    -1,  1158,   518,    -1,    -1,    67,    -1,
     692,   524,   694,   695,    73,    -1,    -1,   699,    -1,   701,
      79,   703,    81,    -1,    -1,    -1,    -1,   540,   685,    -1,
      -1,    -1,    -1,   690,   691,    -1,    -1,    -1,    -1,    98,
      -1,  1137,  1138,   700,    -1,   558,   559,   560,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   713,    -1,  1154,    -1,
      -1,   574,   575,    -1,  1160,    -1,  1162,    -1,    -1,  1165,
    1166,    -1,    -1,   730,  1170,    -1,    -1,    -1,    -1,  1175,
     737,    -1,    -1,  1179,   741,    -1,  1182,    -1,    -1,    -1,
     747,    -1,    -1,    -1,   751,    -1,    -1,   754,    -1,    -1,
      -1,   428,   615,    -1,   617,   618,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      -1,   458,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    -1,    -1,    -1,    -1,    26,    -1,   805,    29,
      -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,    -1,
      40,    41,    -1,   820,    -1,    -1,    -1,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    57,   835,    -1,
      -1,   838,    -1,    -1,    -1,   842,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,   105,    76,   428,    -1,    79,
      -1,    -1,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,   458,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   756,    -1,   902,    -1,    25,    26,    -1,
      -1,    29,    -1,    31,    -1,    -1,   938,    -1,    -1,    -1,
     170,   171,    27,    -1,    42,    -1,    31,   949,   950,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,    67,
      55,    -1,    -1,    58,    -1,    73,   978,   979,   208,    -1,
      -1,    79,    67,    81,    -1,   215,    -1,   820,    73,    -1,
      75,    -1,    -1,    78,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   985,    -1,
      -1,   241,    -1,    -1,    -1,    -1,    -1,  1019,  1020,  1021,
    1022,    -1,    -1,  1025,    -1,    -1,   859,    -1,  1030,  1031,
     260,    -1,    -1,    -1,    -1,  1012,   266,   267,    -1,   269,
      -1,  1018,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    30,    31,    -1,  1033,  1034,    -1,    -1,
      -1,    -1,    -1,    -1,   897,    42,    -1,    -1,    -1,   902,
      -1,    -1,    -1,    -1,    51,   908,   909,  1054,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   921,   922,
      67,    -1,  1069,  1070,    -1,    -1,    73,    -1,    75,    -1,
      -1,    78,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
     340,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   952,
     953,   954,   955,    -1,    -1,    -1,    -1,    -1,   961,    -1,
     963,   361,    -1,   966,   967,    -1,    -1,  1139,  1140,    -1,
      -1,   371,    -1,    -1,   374,    -1,   376,    -1,    -1,   379,
      -1,    -1,   382,    -1,     4,   385,   386,    -1,    -1,   389,
      -1,    -1,   392,    -1,    -1,   395,   396,   397,   398,    -1,
    1147,   401,    -1,    -1,  1151,   405,    26,    27,   408,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    42,    -1,    -1,    -1,    46,    47,    48,    -1,
      50,   431,    52,    53,    -1,    55,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    76,    77,    -1,    79,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   895,   896,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   919,   920,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1125,    -1,    -1,    -1,    -1,    -1,  1131,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   537,   538,   539,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   895,   896,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   919,   920,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   616,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   633,   634,    -1,   636,   637,    -1,   639,
      -1,   641,   642,    -1,   644,   645,   646,   647,   648,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   659,
     660,    -1,    -1,    -1,    -1,    -1,   666,   667,    -1,    -1,
     670,   671,   672,    -1,   674,   675,   676,   677,   678,    -1,
      -1,   681,   682,   683,   684,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   696,   697,   698,    -1,
    1117,  1118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   731,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   748,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1117,  1118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   826,   827,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   839,
      -1,    -1,    -1,   843,    -1,    -1,   846,    -1,    -1,   849,
      -1,   851,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   863,    -1,    -1,   866,   867,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   878,    -1,
      -1,   881,    -1,    -1,   884,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   944,   945,   946,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     960,    -1,   962,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   977,    -1,    -1,
     980,   981,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   993,    -1,    -1,    -1,    -1,   998,    -1,
      -1,  1001,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1023,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1079,
      -1,  1081,  1082,    -1,  1084,  1085,    -1,    -1,  1088,    -1,
      -1,  1091,  1092,     4,  1094,    -1,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    16,    17,    18,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,     4,     5,    -1,     7,    -1,
      -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,     4,    -1,    -1,
      -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     4,
      -1,    -1,    -1,     8,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     4,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,     4,     5,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,     4,    -1,    -1,
      -1,     8,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     4,
      -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   107,   112,   116,   433,     0,     5,     7,    54,    74,
     432,   432,   434,   434,   433,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   108,   418,   439,   440,   418,   109,
     113,   433,   117,   433,    97,    97,   110,   422,   422,    96,
      11,   115,   423,   432,   119,   433,   424,    56,    64,    82,
     114,   121,   124,   139,   118,   120,   121,     8,   425,   426,
     434,   434,   434,   434,   125,   125,    23,   136,   111,    97,
      99,   429,     8,   427,     9,    11,    12,    16,    17,    18,
     108,   436,   438,   439,   108,   417,   440,    17,   108,   122,
      43,    44,   126,   130,   140,   140,   434,   433,   433,   433,
      97,    99,   421,   437,   421,   433,   123,   421,   434,   434,
      28,    31,    59,    67,   141,   142,   143,   144,   145,   145,
     418,    11,   419,   430,   439,   428,   422,   422,   433,   433,
     129,   418,   418,   434,   434,   434,   434,    70,   146,   153,
     153,   137,   425,    98,    96,    98,   127,   131,   436,   436,
     436,   436,   434,   422,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    76,    79,
      98,   154,   155,   169,   174,   180,   227,   231,   232,   241,
     249,   257,   266,   281,   287,   303,   311,   329,   336,    98,
      97,    97,    99,   431,   433,   422,    97,    97,    99,   132,
     421,   421,   421,   421,    12,   108,   151,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   433,   422,   433,   422,   433,   433,     8,
     422,   422,   422,   152,   147,   418,   100,   108,   310,   389,
     390,   418,   418,   418,   108,   340,   390,   418,   418,   418,
     418,   418,   418,   418,   418,   182,   418,    11,   108,   420,
     440,   441,   139,   430,   433,   139,    71,   133,   134,   433,
      97,    99,   148,   282,   419,   304,   391,   433,   267,   233,
     156,   337,   433,   170,   228,   175,   242,   250,   258,   330,
     312,    97,   288,   433,   433,   138,    98,   128,   434,    98,
     422,    97,    99,   283,    97,   389,    97,    99,   268,    97,
      99,   234,    97,    99,   157,    97,    97,    99,   171,    97,
      99,   229,    97,    99,   176,    97,    97,    97,    97,    99,
     331,    97,    99,   313,   422,    97,    99,   289,    98,   433,
     133,    12,   135,   149,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   185,   422,
     422,    98,   433,   150,   284,   306,   270,   235,   158,   339,
     172,   230,   178,   244,   252,   260,   332,   314,    30,    73,
      78,   143,   144,   164,   181,   186,   225,   226,   290,   422,
     421,    98,   143,   144,    27,    42,    51,    55,    73,    81,
      98,   143,   144,   165,   173,   356,   361,   366,   377,    25,
     143,   144,   164,   173,   231,   275,   305,   356,   143,   144,
     165,   173,   226,   232,   241,   249,   257,   269,   273,   274,
     275,   281,   356,   361,   366,    65,    98,   143,   144,   165,
     173,   180,   227,   231,   356,   361,   370,   377,    20,    98,
     143,   144,   159,   164,    37,   143,   144,   338,    98,   143,
     144,   165,   173,    98,   143,   144,   165,   180,   227,   231,
      22,   143,   144,   164,   177,   179,   143,   144,   165,   173,
     225,   226,   243,   246,   356,   361,   366,   377,    52,    53,
      58,   143,   144,   165,   173,   225,   251,   254,   356,   361,
     371,   373,   375,   377,    46,    77,   143,   144,   165,   173,
     180,   227,   231,   259,   356,   361,   371,   373,   375,   377,
     380,   383,    98,   143,   144,   165,   173,   180,   227,   231,
      45,    60,    98,   143,   144,   165,   173,   180,   227,   319,
     326,   434,   434,   434,    98,    75,   183,    21,    68,    98,
     143,   144,   165,   173,   294,   299,   356,   434,   434,   434,
     434,   434,   434,   285,   286,   434,   308,   309,    98,   307,
      98,   272,   271,   434,   239,   238,   240,   236,   237,   434,
     434,    98,   422,   422,   422,   434,    98,    98,   183,   245,
     248,   434,   434,   434,    98,   183,   253,   256,   434,   434,
     263,   264,   265,    98,   261,   262,   333,   334,   335,   433,
     433,   315,   316,   317,   318,   436,    85,    87,    90,   108,
     168,   436,   434,   422,   434,   434,   292,   293,   291,   362,
     420,   367,   436,   166,   436,   422,   422,   418,   422,   422,
     422,   422,   422,   436,   422,   422,   422,   422,   422,   418,
      84,    86,    89,    91,   341,   342,   343,   344,   348,   352,
     420,   247,   422,   422,    17,    94,   108,   374,   413,    18,
     108,   372,   413,   414,    92,    95,   108,   376,   255,   422,
     422,   108,   384,   419,   108,   381,   392,   419,   422,   422,
     422,   422,   422,   422,   422,   422,   320,   327,   422,   422,
     422,   422,   421,   433,   433,   433,   421,   421,   420,   108,
     302,   392,   108,   297,   392,   422,   422,   422,    88,    93,
     108,   364,   421,    88,    93,   108,   369,   378,   168,   357,
     276,   421,   160,   433,   433,   433,   433,   433,   421,   422,
     433,   421,   433,   421,   433,   433,   433,   421,   422,   421,
     385,   421,   382,   421,   434,   393,    97,    97,   184,   300,
     433,   295,   433,   433,   433,   363,   433,   433,   368,    97,
      99,   379,   167,    97,    99,   358,    97,    99,   277,    97,
      99,   161,   345,   349,   421,   353,   386,   421,   434,   392,
     391,   422,   422,    97,    99,   187,    97,    97,    99,   296,
     421,   421,   422,   421,   422,   422,   422,   422,    97,    99,
     346,    97,    99,   350,    97,    99,   354,   419,   382,   322,
     322,   422,   422,   422,   196,   359,   278,    83,   162,   422,
     422,   422,   387,   180,   227,   231,   321,   328,    61,    69,
     179,   188,   189,   200,   201,   211,   212,   214,   215,   306,
     298,    33,    34,    98,   143,   144,   223,   224,    98,   143,
     144,    98,   143,   144,   165,   173,   231,   356,   434,    98,
     347,   351,   355,   386,   323,   324,   325,    98,    98,   434,
     434,    98,    39,    49,    62,    66,   190,   191,   193,   197,
     209,    32,   203,    24,   217,   301,    27,    51,    98,   143,
     144,   226,   360,   365,   370,   371,   373,   377,   434,   434,
     280,   279,    88,    93,   108,   163,    98,   225,   226,   360,
     365,   371,   373,   377,   380,    98,   225,   226,   377,   380,
      98,   183,   225,   226,   360,   365,   371,   373,   422,   422,
     422,   108,   394,   395,   396,    88,    93,   108,   213,   434,
     434,   434,   434,   183,   422,   434,   422,   434,   216,    98,
     434,   434,   422,   436,   436,   422,   422,   433,   433,   421,
     422,   422,   422,   421,   100,   397,   398,    14,   400,   401,
     433,   433,   421,   108,   192,   413,   194,   436,   198,   436,
     388,   436,   422,   204,   436,   202,   218,   418,   217,   364,
     369,   421,   421,   419,   399,   100,   402,   421,   433,    97,
      99,   195,    97,    99,   199,    97,    99,   210,    97,    99,
     205,   203,    97,    99,   219,   421,   421,   405,   397,   401,
     403,   419,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   101,   404,   405,   196,   196,   196,   206,   220,     5,
     435,   398,    98,    98,    98,    80,    98,   143,   144,   164,
     207,    63,    98,   143,   144,   164,   221,   406,   419,   434,
     434,   435,   435,    16,    17,    18,   108,   208,   415,   416,
     108,   222,   414,   102,   103,   421,   433,   421,   433,   435,
      85,   407,   412,   435,   435,   104,   100,   435,   435,   105,
      14,   408,   409,   435,   410,   100,   409,   419,   435,   411,
     419,   435,   100,   435,   419
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   106,   107,   107,   109,   108,   110,   111,   110,   113,
     114,   112,   115,   115,   115,   115,   117,   118,   116,   119,
     119,   120,   119,   121,   123,   122,   122,   124,   125,   125,
     125,   127,   128,   126,   129,   131,   130,   132,   132,   133,
     133,   134,   135,   137,   138,   136,   139,   140,   140,   140,
     140,   140,   141,   142,   143,   144,   145,   145,   147,   146,
     148,   149,   148,   150,   150,   150,   152,   151,   151,   153,
     153,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   156,   155,   157,   157,   158,   158,   158,   158,   158,
     160,   159,   161,   161,   162,   163,   163,   163,   164,   166,
     167,   165,   168,   168,   168,   168,   170,   169,   171,   171,
     172,   172,   172,   172,   172,   173,   175,   174,   176,   177,
     176,   178,   178,   178,   178,   178,   179,   181,   180,   182,
     184,   183,   185,   186,   185,   185,   185,   185,   185,   185,
     187,   187,   188,   188,   188,   188,   188,   188,   189,   189,
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
     269,   268,   270,   271,   270,   270,   270,   270,   270,   270,
     270,   270,   272,   270,   273,   273,   274,   274,   274,   274,
     274,   276,   275,   277,   277,   278,   279,   278,   278,   278,
     278,   278,   280,   278,   282,   281,   283,   283,   284,   285,
     284,   284,   286,   284,   284,   284,   284,   284,   284,   288,
     287,   289,   289,   290,   291,   290,   290,   290,   290,   290,
     292,   290,   293,   290,   295,   294,   296,   296,   297,   297,
     298,   298,   298,   298,   298,   298,   298,   298,   298,   298,
     300,   301,   299,   302,   302,   304,   305,   303,   306,   307,
     306,   306,   306,   306,   306,   308,   306,   309,   306,   310,
     310,   312,   311,   313,   313,   314,   314,   314,   314,   314,
     315,   314,   316,   314,   317,   314,   318,   314,   320,   321,
     319,   322,   323,   322,   324,   322,   325,   322,   327,   328,
     326,   330,   329,   331,   331,   332,   332,   332,   332,   332,
     333,   332,   334,   332,   335,   332,   337,   338,   336,   339,
     339,   339,   339,   340,   340,   341,   341,   342,   342,   342,
     343,   345,   344,   346,   346,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   349,   348,   350,   350,   351,   351,
     351,   351,   351,   353,   352,   354,   354,   355,   355,   355,
     355,   355,   355,   355,   355,   357,   356,   358,   358,   359,
     359,   359,   360,   362,   363,   361,   364,   364,   364,   365,
     367,   368,   366,   369,   369,   369,   370,   371,   372,   372,
     373,   374,   374,   374,   375,   376,   376,   376,   378,   377,
     379,   379,   380,   381,   381,   382,   382,   383,   385,   384,
     384,   387,   386,   386,   388,   389,   390,   391,   391,   393,
     392,   395,   394,   396,   394,   394,   397,   398,   399,   399,
     400,   401,   402,   402,   403,   404,   404,   405,   405,   406,
     407,   408,   409,   410,   410,   411,   411,   412,   413,   414,
     414,   415,   415,   416,   416,   417,   417,   418,   418,   419,
     419,   420,   420,   420,   421,   421,   422,   422,   422,   424,
     423,   425,   426,   426,   427,   427,   428,   428,   429,   429,
     430,   430,   431,   431,   432,   432,   433,   433,   434,   435,
     435,   437,   436,   436,   438,   438,   438,   438,   438,   438,
     438,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   440,
     441
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     4,     0,     0,     6,     0,
       0,    15,     0,     2,     2,     2,     0,     0,    15,     0,
       2,     0,     3,     4,     0,     3,     1,     4,     0,     2,
       2,     0,     0,    11,     1,     0,     6,     1,     4,     0,
       1,     5,     1,     0,     0,    10,     4,     0,     2,     2,
       2,     2,     4,     4,     4,     4,     0,     3,     0,     5,
       1,     0,     5,     0,     2,     2,     0,     3,     1,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     2,     2,     2,     2,
       0,     6,     1,     4,     4,     2,     2,     1,     4,     0,
       0,     6,     2,     2,     2,     1,     0,     5,     1,     4,
       0,     2,     2,     2,     2,     4,     0,     5,     1,     0,
       5,     0,     2,     2,     2,     2,     4,     0,     8,     1,
       0,     5,     0,     0,     4,     2,     2,     2,     2,     2,
       1,     4,     1,     1,     1,     1,     1,     1,     0,     2,
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
       0,     5,     0,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     0,     4,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     5,     1,     4,     0,     0,
       4,     2,     0,     4,     2,     2,     2,     2,     2,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     5,     1,     4,     2,     1,
       0,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     0,     9,     2,     1,     0,     0,     9,     0,     0,
       4,     2,     2,     2,     2,     0,     4,     0,     4,     2,
       1,     0,     5,     1,     4,     0,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     0,
       8,     0,     0,     4,     0,     4,     0,     4,     0,     0,
       8,     0,     5,     1,     4,     0,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     0,     9,     0,
       2,     2,     4,     2,     1,     1,     2,     1,     1,     1,
       3,     0,     4,     1,     4,     0,     2,     3,     2,     2,
       2,     2,     2,     2,     0,     4,     1,     4,     0,     2,
       3,     2,     2,     0,     4,     1,     4,     0,     3,     2,
       2,     2,     2,     2,     2,     0,     5,     1,     4,     0,
       2,     2,     4,     0,     0,     6,     2,     2,     1,     4,
       0,     0,     6,     2,     2,     1,     4,     4,     2,     1,
       4,     2,     2,     1,     4,     2,     2,     1,     0,     5,
       1,     4,     3,     2,     2,     3,     1,     3,     0,     3,
       2,     0,     4,     1,     1,     2,     2,     0,     2,     0,
       3,     0,     2,     0,     2,     1,     3,     2,     0,     2,
       3,     2,     0,     2,     2,     0,     2,     0,     6,     5,
       5,     5,     4,     0,     2,     0,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     2,     2,     1,     2,     4,     0,     2,     2,     0,
       4,     2,     0,     1,     0,     3,     0,     5,     1,     4,
       0,     3,     2,     5,     1,     1,     0,     2,     2,     0,
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

    case 270: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

        break;

    case 339: /* deviation_opt_stmt  */

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

    { if (read_all && lyp_propagate_submodule(module, submodule)) {
                       YYERROR;
                     }
                   }

    break;

  case 4:

    { if (read_all) {
                      if (yyget_text(scanner)[0] == '"') {
                        s = yang_read_string(yyget_text(scanner) + 1, yyget_leng(scanner) - 2, yylloc.first_column);
                        if (!s) {
                          YYERROR;
                        }
                      } else {
                        s = calloc(1, yyget_leng(scanner) - 1);
                        if (!s) {
                          LOGMEM;
                          YYERROR;
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
                    YYERROR;
                  }
                  s = ly_realloc(s, strlen(temp) + strlen(s) + 1);
                  if (s) {
                    strcat(s, temp);
                    free(temp);
                  } else {
                    free(temp);
                    LOGMEM;
                    YYERROR;
                  }
                } else {
                  int length = yyget_leng(scanner) - 2 + strlen(s) + 1;
                  s = ly_realloc(s, length);
                  if (s) {
                    memcpy(s + strlen(s), yyget_text(scanner) + 1, yyget_leng(scanner) - 2);
                    s[length - 1] = '\0';
                  } else {
                    LOGMEM;
                    YYERROR;
                  }
                }
              }
            }

    break;

  case 9:

    { if (read_all) {
                                                              if (submodule) {
                                                                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "module");
                                                                YYERROR;
                                                              }
                                                              trg = module;
                                                              yang_read_common(trg,s,MODULE_KEYWORD);
                                                              s = NULL;
                                                              config_inherit = ENABLE_INHERIT;
                                                            }
                                                          }

    break;

  case 10:

    { if (read_all && !module->ns) { LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "namespace", "module"); YYERROR; }
                                       if (read_all && !module->prefix) { LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "module"); YYERROR; }
                                     }

    break;

  case 12:

    { (yyval.i) = 0; }

    break;

  case 13:

    { if ((yyvsp[-1].i)) { LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "module"); YYERROR; } (yyval.i) = 1; }

    break;

  case 14:

    { if (read_all && yang_read_common(module, s, NAMESPACE_KEYWORD)) {YYERROR;} s=NULL; }

    break;

  case 15:

    { if (read_all && yang_read_prefix(module, NULL, s, MODULE_KEYWORD)) {YYERROR;} s=NULL; }

    break;

  case 16:

    { if (read_all) {
                                                                    if (!submodule) {
                                                                      LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "submodule");
                                                                      YYERROR;
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
                                                YYERROR;
                                              }
                                            }

    break;

  case 19:

    { (yyval.i) = 0; }

    break;

  case 20:

    { if ((yyvsp[-1].i)) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "submodule");
                                                  YYERROR;
                                                }
                                                (yyval.i) = 1;
                                              }

    break;

  case 21:

    { if (read_all) {
                                if (submodule->prefix) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                  YYERROR;
                                }
                              }
                            }

    break;

  case 24:

    { if (strlen(yyget_text(scanner))!=1 || yyget_text(scanner)[0]!='1') {
                                               YYERROR;
                                             }
                                           }

    break;

  case 26:

    { if (read_all) {
                 if (strlen(s)!=1 || s[0]!='1') {
                   free(s);
                   YYERROR;
                 }
               }
             }

    break;

  case 28:

    { if (read_all) {
                          if (size_arrays->imp) {
                            trg->imp = calloc(size_arrays->imp, sizeof *trg->imp);
                            if (!trg->imp) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->inc) {
                            trg->inc = calloc(size_arrays->inc, sizeof *trg->inc);
                            if (!trg->inc) {
                              LOGMEM;
                              YYERROR;
                            }
                            trg->inc_size = size_arrays->inc;
                            size_arrays->inc = 0;
                            /* trg->inc_size can be updated by the included submodules,
                             * so we will use size_arrays->inc here, trg->inc_size stores the
                             * target size of the array
                             */
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
                                 if (yang_read_prefix(trg, actual, s, IMPORT_KEYWORD)) {YYERROR;}
                                 s=NULL;
                                 actual_type=IMPORT_KEYWORD;
                               }
                             }

    break;

  case 33:

    { if (read_all) {
                             (yyval.inc) = trg;
                             if (yang_fill_import(trg, actual, (yyvsp[-8].str))) {
                               YYERROR;
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
                                      if (yang_fill_include(module, submodule, s, rev, size_arrays->inc, unres)) {
                                        YYERROR;
                                      }
                                      size_arrays->inc++;
                                      s = NULL;
                                      trg = (yyval.inc);
                                      config_inherit = ENABLE_INHERIT;
                                    }
                                  }

    break;

  case 42:

    { if (read_all) {
                                 if (actual_type==IMPORT_KEYWORD) {
                                     memcpy(((struct lys_import *)actual)->rev, yyget_text(scanner), LY_REV_SIZE-1);
                                 } else {                              // INCLUDE KEYWORD
                                     memcpy(rev, yyget_text(scanner), LY_REV_SIZE - 1);
                                 }
                               }
                             }

    break;

  case 43:

    { if (read_all) {
                                                               if (!ly_strequal(s, submodule->belongsto->name, 0)) {
                                                                 LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "belongs-to");
                                                                 free(s);
                                                                 YYERROR;
                                                               }
                                                               free(s);
                                                               s = NULL;
                                                             }
                                                           }

    break;

  case 44:

    { if (read_all) {
                                     if (yang_read_prefix(trg, NULL, s, MODULE_KEYWORD)) {
                                       YYERROR;
                                     }
                                     s = NULL;
                                   }
                                 }

    break;

  case 48:

    { if (read_all && yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {YYERROR;} s=NULL; }

    break;

  case 49:

    { if (read_all && yang_read_common(trg, s, CONTACT_KEYWORD)) {YYERROR;} s=NULL; }

    break;

  case 50:

    { if (read_all && yang_read_description(trg, NULL, s, NULL)) {
                                     YYERROR;
                                   }
                                   s = NULL;
                                 }

    break;

  case 51:

    { if (read_all && yang_read_reference(trg, NULL, s, NULL)) {
                                   YYERROR;
                                 }
                                 s=NULL;
                               }

    break;

  case 56:

    { if (read_all && size_arrays->rev) {
                           trg->rev = calloc(size_arrays->rev, sizeof *trg->rev);
                           if (!trg->rev) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                       }

    break;

  case 58:

    { if (read_all) {
                                                     if(!(actual=yang_read_revision(trg,s))) {YYERROR;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }

    break;

  case 61:

    { actual_type = REVISION_KEYWORD; }

    break;

  case 64:

    { if (read_all && yang_read_description(trg, actual, s, "revision")) {
                                            YYERROR;
                                          }
                                          s = NULL;
                                        }

    break;

  case 65:

    { if (read_all && yang_read_reference(trg, actual, s, "revision")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }

    break;

  case 66:

    { if (read_all) {
                                s = strdup(yyget_text(scanner));
                                if (!s) {
                                  LOGMEM;
                                  YYERROR;
                                }
                              }
                            }

    break;

  case 68:

    { if (read_all && lyp_check_date(s)) {
                   free(s);
                   YYERROR;
               }
             }

    break;

  case 69:

    { if (read_all) {
                       if (size_arrays->features) {
                         trg->features = calloc(size_arrays->features,sizeof *trg->features);
                         if (!trg->features) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->ident) {
                         trg->ident = calloc(size_arrays->ident,sizeof *trg->ident);
                         if (!trg->ident) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->augment) {
                         trg->augment = calloc(size_arrays->augment,sizeof *trg->augment);
                         if (!trg->augment) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->tpdf) {
                         trg->tpdf = calloc(size_arrays->tpdf, sizeof *trg->tpdf);
                         if (!trg->tpdf) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->deviation) {
                         trg->deviation = calloc(size_arrays->deviation, sizeof *trg->deviation);
                         if (!trg->deviation) {
                           LOGMEM;
                           YYERROR;
                         }
                         /* module with deviation - must be implemented (description of /ietf-yang-library:modules-state/module/deviation) */
                         module->implemented = 1;
                       }
                       actual = NULL;
                     }
                   }

    break;

  case 70:

    { actual = NULL; }

    break;

  case 74:

    { if (!read_all) { size_arrays->tpdf++; } }

    break;

  case 77:

    { if (!read_all) {
                     size_arrays->augment++;
                   } else {
                     config_inherit = ENABLE_INHERIT;
                   }
                 }

    break;

  case 80:

    { if (!read_all) { size_arrays->deviation++; } }

    break;

  case 81:

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

  case 85:

    { (yyval.uint) = 0; }

    break;

  case 86:

    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYERROR;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }

    break;

  case 87:

    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYERROR;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }

    break;

  case 88:

    { if (read_all) {
                                             free(s);
                                             s= NULL;
                                           }
                                           if ((yyvsp[-1].uint) & EXTENSION_DSC) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "description", "extension");
                                             YYERROR;
                                           }
                                           (yyvsp[-1].uint) |= EXTENSION_DSC;
                                           (yyval.uint) = (yyvsp[-1].uint);
                                         }

    break;

  case 89:

    { if (read_all) {
                                           free(s);
                                           s = NULL;
                                         }
                                         if ((yyvsp[-1].uint) & EXTENSION_REF) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "reference", "extension");
                                           YYERROR;
                                         }
                                         (yyvsp[-1].uint) |= EXTENSION_REF;
                                         (yyval.uint) = (yyvsp[-1].uint);
                                       }

    break;

  case 90:

    { free(s); s = NULL; }

    break;

  case 97:

    { if (read_all) {
                 if (strcmp(s, "true") && strcmp(s, "false")) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                    free(s);
                    YYERROR;
                 }
                 free(s);
                 s = NULL;
               }
             }

    break;

  case 98:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 99:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 100:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 101:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 102:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 103:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 104:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 105:

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
                   YYERROR;
                 }
                 free(s);
                 s = NULL;
               }
             }

    break;

  case 106:

    { if (read_all) {
                                                         if (!(actual = yang_read_feature(trg, s))) {YYERROR;}
                                                         s=NULL;
                                                       } else {
                                                         size_arrays->features++;
                                                       }
                                                     }

    break;

  case 110:

    { if (read_all) {
                             if (size_arrays->node[size_arrays->next].if_features) {
                               ((struct lys_feature*)actual)->features = calloc(size_arrays->node[size_arrays->next].if_features,
                                                                                sizeof *((struct lys_feature*)actual)->features);
                               if (!((struct lys_feature*)actual)->features) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                             store_flags((struct lys_node *)actual, size_arrays->node[size_arrays->next].flags, 0);
                             size_arrays->next++;
                           } else {
                             (yyval.i) = size_arrays->size;
                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                               LOGMEM;
                               YYERROR;
                             }
                           }
                         }

    break;

  case 111:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, actual, s, unres, FEATURE_KEYWORD)) {YYERROR;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].i)].if_features++;
                                        }
                                      }

    break;

  case 112:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].i)].flags, LYS_STATUS_MASK, "status", "feature", (yyvsp[0].i))) {
                                             YYERROR;
                                           }
                                         }
                                       }

    break;

  case 113:

    { if (read_all && yang_read_description(trg, actual, s, "feature")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }

    break;

  case 114:

    { if (read_all && yang_read_reference(trg, actual, s, "feature")) {
                                         YYERROR;
                                       }
                                       s = NULL;
                                     }

    break;

  case 116:

    { if (read_all) {
                                                           if (!(actual = yang_read_identity(trg,s))) {YYERROR;}
                                                           s = NULL;
                                                         } else {
                                                           size_arrays->ident++;
                                                         }
                                                       }

    break;

  case 119:

    { if (read_all && yang_read_base(module, actual, (yyvsp[0].str), unres)) {
                               YYERROR;
                             }
                           }

    break;

  case 121:

    { (yyval.str) = NULL; }

    break;

  case 122:

    { if (read_all) {
                                     if ((yyvsp[-1].str)) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                                       free(s);
                                       free((yyvsp[-1].str));
                                       YYERROR;
                                     }
                                     (yyval.str) = s;
                                     s = NULL;
                                   }
                                 }

    break;

  case 123:

    { if (read_all) {
                                       if (yang_check_flags(&((struct lys_ident *)actual)->flags, LYS_STATUS_MASK, "status", "identity", (yyvsp[0].i))) {
                                         YYERROR;
                                       }
                                     }
                                   }

    break;

  case 124:

    { if (read_all && yang_read_description(trg, actual, s, "identity")) {
                                            free((yyvsp[-1].str));
                                            YYERROR;
                                          }
                                          s = NULL;
                                        }

    break;

  case 125:

    { if (read_all && yang_read_reference(trg, actual, s, "identity")) {
                                          free((yyvsp[-1].str));
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }

    break;

  case 127:

    { if (read_all) {
                                    if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                                      YYERROR;
                                    }
                                    if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DER,(struct lys_node *) (yyvsp[-3].v))) {
                                      YYERROR;
                                    }
                                    actual = (yyvsp[-3].v);

                                    /* check default value */
                                    if ((yyvsp[0].nodes).node.ptr_tpdf->dflt) {
                                      if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_tpdf->dflt) == -1) {
                                        YYERROR;
                                      }
                                    }
                                  }
                                }

    break;

  case 129:

    { if (read_all) {
                                        (yyval.v) = actual;
                                        if (!(actual = yang_read_typedef(trg, actual, s))) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                        actual_type = TYPEDEF_KEYWORD;
                                      }
                                    }

    break;

  case 130:

    { if (read_all && !(actual = yang_read_type(trg, actual, s, actual_type))) {
                                                       YYERROR;
                                                     }
                                                     s = NULL;
                                                   }

    break;

  case 132:

    { (yyval.nodes).node.ptr_tpdf = actual;
                        (yyval.nodes).node.flag = 0;
                      }

    break;

  case 133:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, (yyvsp[0].nodes).node.ptr_tpdf, "type", "typedef");
                       YYERROR;
                     }
                   }

    break;

  case 134:

    { if (read_all) {
                   actual = (yyvsp[-3].nodes).node.ptr_tpdf;
                   actual_type = TYPEDEF_KEYWORD;
                   (yyvsp[-3].nodes).node.flag |= LYS_TYPE_DEF;
                   (yyval.nodes) = (yyvsp[-3].nodes);
                 }
               }

    break;

  case 135:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYERROR;} s = NULL; }

    break;

  case 136:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYERROR;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 137:

    { if (read_all) {
                                   if (yang_check_flags(&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i))) {
                                     YYERROR;
                                   }
                                 }
                               }

    break;

  case 138:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 139:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                      YYERROR;
                                    }
                                    s = NULL;
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
                                           YYERROR;
                                         }
                                         if (size_arrays->node[size_arrays->next].pattern) {
                                           ((struct yang_type *)actual)->type->info.str.patterns = calloc(size_arrays->node[size_arrays->next].pattern, sizeof(struct lys_restr));
                                           if (!((struct yang_type *)actual)->type->info.str.patterns) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           ((struct yang_type *)actual)->base = LY_TYPE_STRING;
                                         }
                                         if (size_arrays->node[size_arrays->next].uni) {
                                           ((struct yang_type *)actual)->type->info.uni.types = calloc(size_arrays->node[size_arrays->next].uni, sizeof(struct lys_type));
                                           if (!((struct yang_type *)actual)->type->info.uni.types) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           ((struct yang_type *)actual)->base = LY_TYPE_UNION;
                                         }
                                         size_arrays->next++;
                                       } else {
                                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                           LOGMEM;
                                           YYERROR;
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
                                  YYERROR;
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
                   YYERROR;
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
                             YYERROR;
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
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }

    break;

  case 164:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }

    break;

  case 165:

    { if (read_all && yang_read_description(trg, actual, s, (yyvsp[-1].str))) {
                                           YYERROR;
                                          }
                                          s = NULL;
                                        }

    break;

  case 166:

    { if (read_all && yang_read_reference(trg, actual, s, (yyvsp[-1].str))) {
                                         YYERROR;
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
                              YYERROR;
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
                            YYERROR;
                          }
                        }
                        ((struct yang_type *)actual)->base = LY_TYPE_ENUM;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYERROR;
                        }
                      }
                    }

    break;

  case 175:

    { if (read_all) {
               if (yang_check_enum((yyvsp[-1].v), actual, &cnt_val, actual_type)) {
                 YYERROR;
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
                           YYERROR;
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
                                    YYERROR;
                                  }
                                  actual_type = 1;
                                }
                              }

    break;

  case 181:

    { if (read_all) {
                                   if (yang_check_flags(&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i))) {
                                     YYERROR;
                                   }
                                 }
                               }

    break;

  case 182:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 183:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYERROR;
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
                      YYERROR;
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
                    YYERROR;
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
                            YYERROR;
                          }
                        }
                        ((struct yang_type *)actual)->base = LY_TYPE_BITS;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYERROR;
                        }
                      }
                    }

    break;

  case 199:

    { if (read_all) {
                      if (yang_check_bit((yyvsp[-2].v), actual, &cnt_val, actual_type)) {
                        YYERROR;
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
                                      YYERROR;
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
                                      YYERROR;
                                    }
                                    actual_type = 1;
                                  }
                                }

    break;

  case 205:

    { if (read_all) {
                                  if (yang_check_flags(&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i))) {
                                    YYERROR;
                                  }
                                }
                              }

    break;

  case 206:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYERROR;
                                     }
                                     s = NULL;
                                   }

    break;

  case 207:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYERROR;
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

    { /* convert it to uint32_t */
                unsigned long val;
                char *endptr;

                val = strtoul(s, &endptr, 10);
                if (val > UINT32_MAX || s[0] == '-' || *endptr) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "position");
                    free(s);
                    YYERROR;
                }
                free(s);
                s = NULL;
                (yyval.uint) = (uint32_t) val;
              }

    break;

  case 215:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYERROR;}
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
                                   YYERROR;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).grouping, size_arrays->node[size_arrays->next].flags, 0);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }

    break;

  case 220:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i))) {
                                              YYERROR;
                                            }
                                          }
                                        }

    break;

  case 221:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYERROR;
                                          }
                                          s = NULL;
                                        }

    break;

  case 222:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYERROR;
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
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYERROR;}
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
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 (yyval.nodes).container->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).container->must);
                                 if (!(yyval.nodes).container->must) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].tpdf) {
                                 (yyval.nodes).container->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).container->tpdf);
                                 if (!(yyval.nodes).container->tpdf) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).container, size_arrays->node[size_arrays->next].flags, config_inherit);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }

    break;

  case 238:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 240:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYERROR;}
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

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYERROR;} s=NULL; }

    break;

  case 244:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }

    break;

  case 245:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }

    break;

  case 246:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }

    break;

  case 247:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYERROR;
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
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 255:

    { if (read_all) {
                                  if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                                    YYERROR;
                                  } else {
                                      if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *) (yyvsp[0].nodes).node.ptr_leaf->parent)) {
                                        (yyvsp[0].nodes).node.ptr_leaf->type.der = NULL;
                                        YYERROR;
                                      }
                                  }
                                  if ((yyvsp[0].nodes).node.ptr_leaf->dflt) {
                                    if ((yyvsp[0].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE) {
                                      /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                                      LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                                      YYERROR;
                                    }
                                    if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_leaf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_leaf->dflt) == -1) {
                                      YYERROR;
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
                                YYERROR;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).node.ptr_leaf->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_leaf->must);
                              if (!(yyval.nodes).node.ptr_leaf->must) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaf, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }

    break;

  case 258:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }

    break;

  case 260:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYERROR;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 261:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYERROR;
                       }
                     }

    break;

  case 262:

    { if (read_all) {
                     actual = (yyvsp[-2].nodes).node.ptr_leaf;
                     actual_type = LEAF_KEYWORD;
                     (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                   }
                 }

    break;

  case 263:

    { (yyval.nodes) = (yyvsp[-4].nodes);}

    break;

  case 264:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYERROR;} s = NULL; }

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

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYERROR;}
                                    s = NULL;
                                  }

    break;

  case 268:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i))) {
                                                 YYERROR;
                                               }
                                             }
                                           }

    break;

  case 269:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }

    break;

  case 270:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        }
                                      }

    break;

  case 271:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }

    break;

  case 272:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 273:

    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYERROR;}
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
                                           if ((yyvsp[0].nodes).node.ptr_leaflist->max && (yyvsp[0].nodes).node.ptr_leaflist->min > (yyvsp[0].nodes).node.ptr_leaflist->max) {
                                             LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                                             YYERROR;
                                           }
                                           if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                             LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                                             YYERROR;
                                           } else {
                                             if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER,
                                                                      (struct lys_node *) (yyvsp[0].nodes).node.ptr_leaflist->parent)) {
                                               YYERROR;
                                             }
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
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 (yyval.nodes).node.ptr_leaflist->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_leaflist->must);
                                 if (!(yyval.nodes).node.ptr_leaflist->must) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaflist, size_arrays->node[size_arrays->next].flags, config_inherit);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }

    break;

  case 277:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 279:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYERROR;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 280:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYERROR;
                          }
                        }

    break;

  case 281:

    { if (read_all) {
                   actual = (yyvsp[-2].nodes).node.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                 }
               }

    break;

  case 282:

    { (yyval.nodes) = (yyvsp[-4].nodes); }

    break;

  case 283:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYERROR;} s = NULL; }

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
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }

    break;

  case 287:

    { if (read_all) {
                                              if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                                                YYERROR;
                                              }
                                              (yyvsp[-1].nodes).node.ptr_leaflist->min = (yyvsp[0].uint);
                                              (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                              if ((yyvsp[-1].nodes).node.ptr_leaflist->max && ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max)) {
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                                              }
                                            }
                                          }

    break;

  case 288:

    { if (read_all) {
                                              if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "max-elements", "leaf-list");
                                                YYERROR;
                                              }
                                              (yyvsp[-1].nodes).node.ptr_leaflist->max = (yyvsp[0].uint);
                                              (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                              if ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max) {
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "max-elements");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                                              }
                                            }
                                          }

    break;

  case 289:

    { if (read_all) {
                                            if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "ordered by", "leaf-list");
                                              YYERROR;
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
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }

    break;

  case 291:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }

    break;

  case 292:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }

    break;

  case 293:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYERROR;}
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
                                   YYERROR;
                                 }
                                 if ((yyvsp[0].nodes).node.ptr_list->keys && yang_read_key(trg, (yyvsp[0].nodes).node.ptr_list, unres)) {
                                   YYERROR;
                                 }
                                 if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                   LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_list, "data-def statement missing.");
                                   YYERROR;
                                 }
                                 if (yang_read_unique(trg, (yyvsp[0].nodes).node.ptr_list, unres)) {
                                   YYERROR;
                                 }
                               }
                             }

    break;

  case 296:

    { if (read_all) {
                          (yyval.nodes).node.ptr_list = actual;
                          (yyval.nodes).node.flag = 0;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).node.ptr_list->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_list->features);
                            if (!(yyval.nodes).node.ptr_list->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].must) {
                            (yyval.nodes).node.ptr_list->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_list->must);
                            if (!(yyval.nodes).node.ptr_list->must) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].tpdf) {
                            (yyval.nodes).node.ptr_list->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_list->tpdf);
                            if (!(yyval.nodes).node.ptr_list->tpdf) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].unique) {
                            (yyval.nodes).node.ptr_list->unique = calloc(size_arrays->node[size_arrays->next].unique, sizeof *(yyval.nodes).node.ptr_list->unique);
                            if (!(yyval.nodes).node.ptr_list->unique) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).node.ptr_list, size_arrays->node[size_arrays->next].flags, config_inherit);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                      }

    break;

  case 297:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 299:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYERROR;}
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
                                  YYERROR;
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
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }

    break;

  case 305:

    { if (read_all) {
                                         if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "min-elements", "list");
                                           YYERROR;
                                         }
                                         (yyvsp[-1].nodes).node.ptr_list->min = (yyvsp[0].uint);
                                         (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                         if ((yyvsp[-1].nodes).node.ptr_list->max && ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max)) {
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                                         }
                                       }
                                     }

    break;

  case 306:

    { if (read_all) {
                                         if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "max-elements", "list");
                                           YYERROR;
                                         }
                                         (yyvsp[-1].nodes).node.ptr_list->max = (yyvsp[0].uint);
                                         (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                         if ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max) {
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                                         }
                                       }
                                     }

    break;

  case 307:

    { if (read_all) {
                                       if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "ordered by", "list");
                                         YYERROR;
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
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }

    break;

  case 309:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 310:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYERROR;
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
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_CHOICE,sizeof(struct lys_node_choice)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }

    break;

  case 320:

    { if (read_all) {
                              if ((yyvsp[0].nodes).choice.s && ((yyvsp[0].nodes).choice.ptr_choice->flags & LYS_MAND_TRUE)) {
                                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "choice");
                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"default\" statement is forbidden on choices with \"mandatory\".");
                                YYERROR;
                              }
                              /* link default with the case */
                              if ((yyvsp[0].nodes).choice.s) {
                                if (unres_schema_add_str(trg, unres, (yyvsp[0].nodes).choice.ptr_choice, UNRES_CHOICE_DFLT, (yyvsp[0].nodes).choice.s) == -1) {
                                  YYERROR;
                                }
                                free((yyvsp[0].nodes).choice.s);
                              }
                            }
                          }

    break;

  case 322:

    { if (read_all) {
                            (yyval.nodes).choice.ptr_choice = actual;
                            (yyval.nodes).choice.s = NULL;
                            actual_type = CHOICE_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).choice.ptr_choice->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).choice.ptr_choice->features);
                              if (!(yyval.nodes).choice.ptr_choice->features) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).choice.ptr_choice, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }

    break;

  case 323:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 324:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 325:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).choice.ptr_choice,s, unres, CHOICE_KEYWORD)) {
                                           if ((yyvsp[-1].nodes).choice.s) {
                                             free((yyvsp[-1].nodes).choice.s);
                                           }
                                           YYERROR;
                                         }
                                         s=NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 326:

    { if (read_all) {
                                      if ((yyvsp[-1].nodes).choice.s) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).choice.ptr_choice, "default", "choice");
                                        free((yyvsp[-1].nodes).choice.s);
                                        free(s);
                                        YYERROR;
                                      }
                                      (yyvsp[-1].nodes).choice.s = s;
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 327:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i))) {
                                             YYERROR;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 328:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i))) {
                                        YYERROR;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 329:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 330:

    { if (read_all) {
                                          if (yang_read_description(trg, (yyvsp[-1].nodes).choice.ptr_choice, s, "choice")) {
                                            free((yyvsp[-1].nodes).choice.s);
                                            YYERROR;
                                          }
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 331:

    { if (read_all) {
                                        if (yang_read_reference(trg, (yyvsp[-1].nodes).choice.ptr_choice, s, "choice")) {
                                          free((yyvsp[-1].nodes).choice.s);
                                          YYERROR;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }
                                    }

    break;

  case 332:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 333:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 341:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 345:

    { if (read_all) {
                          (yyval.nodes).cs = actual;
                          actual_type = CASE_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).cs->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).cs->features);
                            if (!(yyval.nodes).cs->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).cs, size_arrays->node[size_arrays->next].flags, 1);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                      }

    break;

  case 346:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 348:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 349:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }

    break;

  case 350:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 351:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }

    break;

  case 352:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 354:

    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_ANYXML,sizeof(struct lys_node_anyxml)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }

    break;

  case 358:

    { if (read_all) {
                            (yyval.nodes).anyxml = actual;
                            actual_type = ANYXML_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).anyxml->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).anyxml->features);
                              if (!(yyval.nodes).anyxml->features) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).anyxml->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).anyxml->must);
                              if (!(yyval.nodes).anyxml->features || !(yyval.nodes).anyxml->must) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).anyxml, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }

    break;

  case 359:

    { actual = (yyvsp[-1].nodes).anyxml; actual_type = ANYXML_KEYWORD; }

    break;

  case 361:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).anyxml, s, unres, ANYXML_KEYWORD)) {YYERROR;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 362:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 364:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "anyxml", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        }
                                      }

    break;

  case 365:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "anyxml", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }

    break;

  case 366:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "anyxml", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        }
                                      }

    break;

  case 367:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }

    break;

  case 368:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 369:

    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_USES,sizeof(struct lys_node_uses)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }

    break;

  case 370:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYERROR;
                        }
                      }
                    }

    break;

  case 373:

    { if (read_all) {
                          (yyval.nodes).uses.ptr_uses = actual;
                          (yyval.nodes).uses.config_inherit = config_inherit;
                          actual_type = USES_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).uses.ptr_uses->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).uses.ptr_uses->features);
                            if (!(yyval.nodes).uses.ptr_uses->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].refine) {
                            (yyval.nodes).uses.ptr_uses->refine = calloc(size_arrays->node[size_arrays->next].refine, sizeof *(yyval.nodes).uses.ptr_uses->refine);
                            if (!(yyval.nodes).uses.ptr_uses->refine) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].augment) {
                            (yyval.nodes).uses.ptr_uses->augment = calloc(size_arrays->node[size_arrays->next].augment, sizeof *(yyval.nodes).uses.ptr_uses->augment);
                            if (!(yyval.nodes).uses.ptr_uses->augment) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).uses.ptr_uses, size_arrays->node[size_arrays->next].flags, config_inherit);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                      }

    break;

  case 374:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 376:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 377:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }

    break;

  case 378:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 379:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }

    break;

  case 380:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 382:

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

  case 384:

    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYERROR;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 390:

    { if (read_all) {
                                  (yyval.nodes).refine = actual;
                                  actual_type = REFINE_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].must) {
                                    (yyval.nodes).refine->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).refine->must);
                                    if (!(yyval.nodes).refine->must) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                    (yyval.nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
                                  }
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYERROR;
                                  }
                                }
                              }

    break;

  case 391:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 392:

    { if (read_all) {
                                             if ((yyvsp[-1].nodes).refine->target_type) {
                                               if ((yyvsp[-1].nodes).refine->target_type & LYS_CONTAINER) {
                                                 if ((yyvsp[-1].nodes).refine->mod.presence) {
                                                   LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "presence", "refine");
                                                   free(s);
                                                   YYERROR;
                                                 }
                                                 (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                                 (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                               } else {
                                                 free(s);
                                                 LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "presence", "refine");
                                                 LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                 YYERROR;
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

  case 393:

    { if (read_all) {
                                            if ((yyvsp[-1].nodes).refine->target_type) {
                                              if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                                if ((yyvsp[-1].nodes).refine->mod.dflt) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "default", "refine");
                                                  free(s);
                                                  YYERROR;
                                                }
                                                (yyvsp[-1].nodes).refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                              } else {
                                                free(s);
                                                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                YYERROR;
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

  case 394:

    { if (read_all) {
                                           if ((yyvsp[-1].nodes).refine->target_type) {
                                             if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST)) {
                                               (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST);
                                               if (yang_check_flags(&(yyvsp[-1].nodes).refine->flags, LYS_CONFIG_MASK, "config", "refine", (yyvsp[0].i))) {
                                                 YYERROR;
                                               }
                                             } else {
                                               LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "config", "refine");
                                               LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                               YYERROR;
                                             }
                                           } else {
                                             (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST;
                                             (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                           }
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 395:

    { if (read_all) {
                                              if ((yyvsp[-1].nodes).refine->target_type) {
                                                if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML)) {
                                                  (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML);
                                                  if (yang_check_flags(&(yyvsp[-1].nodes).refine->flags, LYS_MAND_MASK, "mandatory", "refine", (yyvsp[0].i))) {
                                                    YYERROR;
                                                  }
                                                } else {
                                                  LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "refine");
                                                  LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                  YYERROR;
                                                }
                                              } else {
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYXML;
                                                (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
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
                                                     /* magic - bit 3 in flags means min set */
                                                     if ((yyvsp[-1].nodes).refine->flags & 0x04) {
                                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                       YYERROR;
                                                     }
                                                     (yyvsp[-1].nodes).refine->flags |= 0x04;
                                                     (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                   } else {
                                                     LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                     LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                     YYERROR;
                                                   }
                                                 } else {
                                                   (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   /* magic - bit 3 in flags means min set */
                                                   (yyvsp[-1].nodes).refine->flags |= 0x04;
                                                   (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 397:

    { if (read_all) {
                                                 if ((yyvsp[-1].nodes).refine->target_type) {
                                                   if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     /* magic - bit 4 in flags means max set */
                                                     if ((yyvsp[-1].nodes).refine->flags & 0x08) {
                                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                       YYERROR;
                                                     }
                                                     (yyvsp[-1].nodes).refine->flags |= 0x08;
                                                     (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                   } else {
                                                     LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                     LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                     YYERROR;
                                                   }
                                                 } else {
                                                   (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   /* magic - bit 4 in flags means max set */
                                                   (yyvsp[-1].nodes).refine->flags |= 0x08;
                                                   (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 398:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                            }

    break;

  case 399:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                          }

    break;

  case 400:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYERROR;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 401:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYERROR;
                                          }
                                        }

    break;

  case 405:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYERROR;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 406:

    { if (read_all) {
                                       if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                         LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "augment");
                                         YYERROR;
                                       }
                                       if (unres_schema_add_node(trg, unres, actual, UNRES_AUGMENT, NULL) == -1) {
                                         YYERROR;
                                       }
                                     }
                                   }

    break;

  case 408:

    { if (read_all) {
                             (yyval.nodes).node.ptr_augment = actual;
                             (yyval.nodes).node.flag = 0;
                             actual_type = AUGMENT_KEYWORD;
                             if (size_arrays->node[size_arrays->next].if_features) {
                               (yyval.nodes).node.ptr_augment->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_augment->features);
                               if (!(yyval.nodes).node.ptr_augment->features) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                             config_inherit = DISABLE_INHERIT;
                             size_arrays->next++;
                           } else {
                             (yyval.nodes).index = size_arrays->size;
                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                               LOGMEM;
                               YYERROR;
                             }
                           }
                         }

    break;

  case 409:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 411:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYERROR;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 412:

    { if (read_all) {
                                      if (yang_check_flags(&(yyvsp[-1].nodes).node.ptr_augment->flags, LYS_STATUS_MASK, "status", "augment", (yyvsp[0].i))) {
                                        YYERROR;
                                      }
                                    }
                                  }

    break;

  case 413:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }

    break;

  case 414:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYERROR;
                                       }
                                       s = NULL;
                                     }

    break;

  case 415:

    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }

    break;

  case 416:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 417:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 418:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 421:

    { if (read_all) {
                                                 if (!(actual = yang_read_node(trg, NULL, s, LYS_RPC, sizeof(struct lys_node_rpc)))) {
                                                   YYERROR;
                                                 }
                                                 data_node = actual;
                                                 s = NULL;
                                               }
                                               config_inherit = DISABLE_INHERIT;
                                             }

    break;

  case 422:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 425:

    { if (read_all) {
                         (yyval.nodes).node.ptr_rpc = actual;
                         (yyval.nodes).node.flag = 0;
                         actual_type = RPC_KEYWORD;
                         if (size_arrays->node[size_arrays->next].if_features) {
                           (yyval.nodes).node.ptr_rpc->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_rpc->features);
                           if (!(yyval.nodes).node.ptr_rpc->features) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                         if (size_arrays->node[size_arrays->next].tpdf) {
                           (yyval.nodes).node.ptr_rpc->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_rpc->tpdf);
                           if (!(yyval.nodes).node.ptr_rpc->tpdf) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                         store_flags((struct lys_node *)(yyval.nodes).node.ptr_rpc, size_arrays->node[size_arrays->next].flags, 0);
                         size_arrays->next++;
                       } else {
                         (yyval.nodes).index = size_arrays->size;
                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                     }

    break;

  case 426:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYERROR;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 427:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i))) {
                                         YYERROR;
                                       }
                                     }
                                   }

    break;

  case 428:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYERROR;
                                     }
                                     s = NULL;
                                   }

    break;

  case 429:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYERROR;
                                   }
                                   s = NULL;
                                 }

    break;

  case 430:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 432:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 434:

    { if (read_all) {
                                 if ((yyvsp[-1].nodes).node.flag & LYS_RPC_INPUT) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "input", "rpc");
                                   YYERROR;
                                 }
                                 (yyvsp[-1].nodes).node.flag |= LYS_RPC_INPUT;
                                 actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                 actual_type = RPC_KEYWORD;
                                 data_node = actual;
                               }
                             }

    break;

  case 435:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 436:

    { if (read_all) {
                                  if ((yyvsp[-1].nodes).node.flag & LYS_RPC_OUTPUT) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "output", "rpc");
                                    YYERROR;
                                  }
                                  (yyvsp[-1].nodes).node.flag |= LYS_RPC_OUTPUT;
                                  actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }
                              }

    break;

  case 437:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 438:

    { if (read_all) {
                                     if (!(actual = yang_read_node(trg, actual, NULL, LYS_INPUT, sizeof(struct lys_node_rpc_inout)))) {
                                      YYERROR;
                                     }
                                     data_node = actual;
                                   }
                                 }

    break;

  case 439:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYERROR;
                                        }
                                      }

    break;

  case 441:

    { if (read_all) {
                                  (yyval.nodes).node.ptr_inout = actual;
                                  (yyval.nodes).node.flag = 0;
                                  actual_type = INPUT_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).node.ptr_inout->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_inout->tpdf);
                                    if (!(yyval.nodes).node.ptr_inout->tpdf) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYERROR;
                                  }
                                }
                              }

    break;

  case 442:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 444:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 446:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 447:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 448:

    { if (read_all) {
                                       if (!(actual = yang_read_node(trg, actual, NULL, LYS_OUTPUT, sizeof(struct lys_node_rpc_inout)))) {
                                        YYERROR;
                                       }
                                       data_node = actual;
                                     }
                                   }

    break;

  case 449:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYERROR;
                                         }
                                       }

    break;

  case 451:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYERROR;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 452:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 453:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 455:

    { if (read_all) {
                                  (yyval.nodes).notif = actual;
                                  actual_type = NOTIFICATION_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].if_features) {
                                    (yyval.nodes).notif->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).notif->features);
                                    if (!(yyval.nodes).notif->features) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).notif->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).notif->tpdf);
                                    if (!(yyval.nodes).notif->tpdf) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  store_flags((struct lys_node *)(yyval.nodes).notif, size_arrays->node[size_arrays->next].flags, 0);
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYERROR;
                                  }
                                }
                              }

    break;

  case 456:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYERROR;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 457:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i))) {
                                                  YYERROR;
                                                }
                                              }
                                            }

    break;

  case 458:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                            }

    break;

  case 459:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                          }

    break;

  case 460:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 462:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 464:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 466:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYERROR;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 467:

    { if (read_all) {
                                            if (actual_type == DEVIATION_KEYWORD) {
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                                              YYERROR;
                                            }
                                            if (yang_check_deviation(trg, actual, unres)) {
                                              YYERROR;
                                            }
                                            free((yyvsp[0].nodes).deviation);
                                          }
                                        }

    break;

  case 469:

    { if (read_all) {
                               (yyval.nodes).deviation = actual;
                               actual_type = DEVIATION_KEYWORD;
                               if (size_arrays->node[size_arrays->next].deviate) {
                                 (yyval.nodes).deviation->deviation->deviate = calloc(size_arrays->node[size_arrays->next].deviate, sizeof *(yyval.nodes).deviation->deviation->deviate);
                                 if (!(yyval.nodes).deviation->deviation->deviate) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }

    break;

  case 470:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             free((yyvsp[-1].nodes).deviation);
                                             YYERROR;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 471:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           free((yyvsp[-1].nodes).deviation);
                                           YYERROR;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 472:

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

  case 475:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYERROR;
                     }
                   }

    break;

  case 481:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYERROR;
                                       }
                                     }

    break;

  case 485:

    { if (read_all) {
                                 (yyval.nodes).deviation = actual;
                                 actual_type = ADD_KEYWORD;
                                 if (size_arrays->node[size_arrays->next].must) {
                                    if (yang_read_deviate_must(actual, size_arrays->node[size_arrays->next].must)) {
                                      YYERROR;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].unique) {
                                    if (yang_read_deviate_unique(actual, size_arrays->node[size_arrays->next].unique)) {
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                               } else {
                                 (yyval.nodes).index = size_arrays->size;
                                 if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                             }

    break;

  case 486:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 487:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 488:

    { if (read_all) {
                                          struct lys_node_list *list;

                                          list = (struct lys_node_list *)(yyvsp[-1].nodes).deviation->target;
                                          if (yang_fill_unique(trg, list, &list->unique[list->unique_size], s, NULL)) {
                                            list->unique_size++;
                                            YYERROR;
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

  case 489:

    { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 490:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 491:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 492:

    { if (read_all) {
                                                if ((yyvsp[-1].nodes).deviation->deviate->min_set) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                  YYERROR;
                                                }
                                                if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 0)) {
                                                  YYERROR;
                                                }
                                                (yyval.nodes) =  (yyvsp[-1].nodes);
                                              }
                                            }

    break;

  case 493:

    { if (read_all) {
                                                if ((yyvsp[-1].nodes).deviation->deviate->max_set) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                  YYERROR;
                                                }
                                                if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 1)) {
                                                  YYERROR;
                                                }
                                                (yyval.nodes) =  (yyvsp[-1].nodes);
                                              }
                                            }

    break;

  case 494:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYERROR;
                                             }
                                           }

    break;

  case 498:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = DELETE_KEYWORD;
                                    if (size_arrays->node[size_arrays->next].must) {
                                      if (yang_read_deviate_must(actual, size_arrays->node[size_arrays->next].must)) {
                                        YYERROR;
                                      }
                                    }
                                    if (size_arrays->node[size_arrays->next].unique) {
                                      if (yang_read_deviate_unique(actual, size_arrays->node[size_arrays->next].unique)) {
                                        YYERROR;
                                      }
                                    }
                                    size_arrays->next++;
                                  } else {
                                    (yyval.nodes).index = size_arrays->size;
                                    if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                }

    break;

  case 499:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 500:

    { if (read_all) {
                                           if (yang_check_deviate_must(trg->ctx, (yyvsp[-2].nodes).deviation)) {
                                             YYERROR;
                                           }
                                           actual = (yyvsp[-2].nodes).deviation;
                                           actual_type = DELETE_KEYWORD;
                                           (yyval.nodes) = (yyvsp[-2].nodes);
                                         } else {
                                           size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                         }
                                       }

    break;

  case 501:

    { if (read_all) {
                                             if (yang_check_deviate_unique(trg, (yyvsp[-1].nodes).deviation, s)) {
                                               YYERROR;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           } else {
                                             size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                           }
                                         }

    break;

  case 502:

    { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 503:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYERROR;
                                               }
                                             }

    break;

  case 507:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }

    break;

  case 508:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target)) {
                                              YYERROR;
                                            }
                                          }
                                        }

    break;

  case 509:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYERROR;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 510:

    { if (read_all) {
                                               if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                 YYERROR;
                                               }
                                               s = NULL;
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }
                                           }

    break;

  case 511:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYERROR;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 512:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYERROR;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 513:

    { if (read_all) {
                                                    if ((yyvsp[-1].nodes).deviation->deviate->min_set) {
                                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviate");
                                                      YYERROR;
                                                    }
                                                    if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 0)) {
                                                      YYERROR;
                                                    }
                                                    (yyval.nodes) =  (yyvsp[-1].nodes);
                                                  }
                                                }

    break;

  case 514:

    { if (read_all) {
                                                    if ((yyvsp[-1].nodes).deviation->deviate->max_set) {
                                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviate");
                                                      YYERROR;
                                                    }
                                                    if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 1)) {
                                                      YYERROR;
                                                    }
                                                    (yyval.nodes) =  (yyvsp[-1].nodes);
                                                  }
                                                }

    break;

  case 515:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYERROR;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 520:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }

    break;

  case 521:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }

    break;

  case 522:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 523:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 524:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 525:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 526:

    { (yyval.i) = LYS_CONFIG_W; }

    break;

  case 527:

    { (yyval.i) = LYS_CONFIG_R; }

    break;

  case 528:

    { if (read_all) {
                  if (!strcmp(s, "true")) {
                    (yyval.i) = LYS_CONFIG_W;
                  } else if (!strcmp(s, "false")) {
                    (yyval.i) = LYS_CONFIG_R;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "config");
                    free(s);
                    YYERROR;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 529:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 530:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 531:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 532:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 533:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 534:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 535:

    { if (read_all) {
                  if (!strcmp(s, "true")) {
                    (yyval.i) = LYS_MAND_TRUE;
                  } else if (!strcmp(s, "false")) {
                    (yyval.i) = LYS_MAND_FALSE;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "mandatory");
                    free(s);
                    YYERROR;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 537:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 538:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 539:

    { if (read_all) {
                  if (strlen(s) == 1 && s[0] == '0') {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(yyget_text(scanner), &endptr, 10);
                    if (val > UINT32_MAX || *endptr) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "min-elements");
                        free(s);
                        YYERROR;
                    }
                    (yyval.uint) = (uint32_t) val;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 540:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 541:

    { (yyval.uint) = 0; }

    break;

  case 542:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 543:

    { if (read_all) {
                  if (!strcmp(s, "unbounded")) {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(yyget_text(scanner), &endptr, 10);
                    if (val > UINT32_MAX || *endptr) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "max-elements");
                        free(s);
                        YYERROR;
                    }
                    (yyval.uint) = (uint32_t) val;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 544:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 545:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 546:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 547:

    { if (!strcmp(s, "user")) {
                  (yyval.i) = LYS_USERORDERED;
                } else if (!strcmp(s, "system")) {
                  (yyval.i) = LYS_SYSTEMORDERED;
                } else {
                  free(s);
                  YYERROR;
                }
                free(s);
                s=NULL;
              }

    break;

  case 548:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYERROR;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 558:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                             }

    break;

  case 561:

    { if (read_all) {
                                 s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                 if (!s) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                                 strcat(s," ");
                                 strcat(s, yyget_text(scanner));
                                }
                             }

    break;

  case 564:

    { if (read_all) {
                          (yyval.v) = actual;
                          if (!(actual = yang_read_range(trg, actual, s))) {
                             YYERROR;
                          }
                          actual_type = RANGE_KEYWORD;
                          s = NULL;
                        }
                      }

    break;

  case 565:

    { if (read_all) {
                                                if (s) {
                                                  s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYERROR;
                                                  }
                                                  strcat(s,"/");
                                                  strcat(s, yyget_text(scanner));
                                                } else {
                                                  s = malloc(yyget_leng(scanner) + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYERROR;
                                                  }
                                                  s[0]='/';
                                                  memcpy(s + 1, yyget_text(scanner), yyget_leng(scanner) + 1);
                                                }
                                              }
                                            }

    break;

  case 569:

    { if (read_all)  {
                                              if (s) {
                                                s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 1);
                                                if (!s) {
                                                  LOGMEM;
                                                  YYERROR;
                                                }
                                                strcat(s, yyget_text(scanner));
                                              } else {
                                                s = strdup(yyget_text(scanner));
                                                if (!s) {
                                                  LOGMEM;
                                                  YYERROR;
                                                }
                                              }
                                            }
                                          }

    break;

  case 571:

    { tmp_s = yyget_text(scanner); }

    break;

  case 572:

    { if (read_all) {
                                                     s = strdup(tmp_s);
                                                     if (!s) {
                                                       LOGMEM;
                                                       YYERROR;
                                                     }
                                                     s[strlen(s) - 1] = '\0';
                                                   }
                                                 }

    break;

  case 573:

    { tmp_s = yyget_text(scanner); }

    break;

  case 574:

    { if (read_all) {
                                           s = strdup(tmp_s);
                                           if (!s) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           s[strlen(s) - 1] = '\0';
                                         }
                                       }

    break;

  case 598:

    { /* convert it to uint32_t */
                                                unsigned long val;

                                                val = strtoul(yyget_text(scanner), NULL, 10);
                                                if (val > UINT32_MAX) {
                                                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Converted number is very long.");
                                                    YYERROR;
                                                }
                                                (yyval.uint) = (uint32_t) val;
                                             }

    break;

  case 599:

    { (yyval.uint) = 0; }

    break;

  case 600:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 601:

    { (yyval.i) = 0; }

    break;

  case 602:

    { /* convert it to int32_t */
               int64_t val;

               val = strtoll(yyget_text(scanner), NULL, 10);
               if (val < INT32_MIN || val > INT32_MAX) {
                   LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The number is not in the correct range (INT32_MIN..INT32_MAX): \"%d\"",val);
                   YYERROR;
               }
               (yyval.i) = (int32_t) val;
             }

    break;

  case 608:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYERROR;
                }
              }

    break;

  case 613:

    { if (read_all) {
                 char *tmp;

                 if ((tmp = strchr(s, ':'))) {
                   *tmp = '\0';
                   /* check prefix */
                   if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYERROR;
                   }
                   /* check identifier */
                   if (lyp_check_identifier(tmp + 1, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYERROR;
                   }
                   *tmp = ':';
                 } else {
                   /* check identifier */
                   if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYERROR;
                   }
                 }
               }
             }

    break;

  case 619:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYERROR;
                                       }
                                     }
                                   }

    break;

  case 641:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYERROR;
                    }
                  }
                }

    break;

  case 729:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }

    break;

  case 730:

    { if (read_all) {
                                      s = strdup(yyget_text(scanner));
                                      if (!s) {
                                        LOGMEM;
                                        YYERROR;
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
