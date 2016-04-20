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
#define YYLAST   3032

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  335
/* YYNRULES -- Number of rules.  */
#define YYNRULES  729
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1183

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
     465,   467,   479,   490,   479,   499,   501,   502,   503,   504,
     509,   515,   517,   519,   521,   523,   531,   534,   534,   543,
     544,   544,   549,   550,   555,   562,   562,   571,   577,   618,
     621,   622,   623,   624,   625,   626,   627,   633,   634,   635,
     637,   637,   656,   657,   661,   662,   669,   676,   687,   699,
     699,   701,   702,   706,   708,   709,   710,   721,   723,   724,
     723,   727,   728,   729,   730,   747,   747,   756,   757,   762,
     781,   788,   794,   799,   805,   807,   807,   816,   818,   817,
     825,   826,   837,   843,   849,   856,   860,   858,   880,   890,
     890,   897,   900,   900,   912,   913,   919,   925,   930,   936,
     937,   943,   944,   945,   952,   960,   965,   969,   999,  1000,
    1004,  1005,  1006,  1014,  1027,  1033,  1034,  1052,  1056,  1066,
    1067,  1072,  1087,  1092,  1097,  1102,  1108,  1112,  1122,  1123,
    1128,  1128,  1146,  1147,  1150,  1162,  1172,  1173,  1178,  1179,
    1188,  1194,  1199,  1205,  1217,  1218,  1236,  1241,  1242,  1247,
    1249,  1251,  1255,  1259,  1274,  1274,  1292,  1293,  1295,  1306,
    1316,  1317,  1322,  1323,  1332,  1338,  1343,  1349,  1361,  1362,
    1377,  1379,  1381,  1383,  1385,  1385,  1392,  1393,  1398,  1418,
    1424,  1429,  1434,  1435,  1442,  1445,  1446,  1447,  1448,  1449,
    1450,  1451,  1454,  1454,  1462,  1463,  1468,  1502,  1502,  1504,
    1511,  1511,  1519,  1520,  1526,  1532,  1537,  1542,  1542,  1547,
    1547,  1555,  1555,  1562,  1569,  1562,  1593,  1621,  1621,  1623,
    1630,  1635,  1630,  1642,  1643,  1643,  1651,  1654,  1660,  1666,
    1672,  1677,  1683,  1690,  1683,  1714,  1742,  1742,  1744,  1751,
    1756,  1751,  1763,  1764,  1764,  1772,  1778,  1792,  1806,  1818,
    1824,  1829,  1835,  1842,  1835,  1867,  1908,  1908,  1910,  1917,
    1917,  1925,  1935,  1943,  1949,  1963,  1977,  1989,  1995,  2000,
    2005,  2005,  2013,  2013,  2018,  2018,  2026,  2026,  2037,  2039,
    2038,  2056,  2077,  2077,  2079,  2092,  2104,  2112,  2120,  2128,
    2137,  2146,  2146,  2156,  2157,  2160,  2161,  2162,  2163,  2164,
    2167,  2167,  2175,  2176,  2180,  2200,  2200,  2202,  2209,  2215,
    2220,  2225,  2225,  2232,  2232,  2243,  2244,  2248,  2275,  2275,
    2277,  2284,  2284,  2292,  2298,  2304,  2310,  2315,  2321,  2321,
    2337,  2338,  2342,  2377,  2377,  2379,  2386,  2392,  2397,  2402,
    2402,  2410,  2410,  2425,  2425,  2434,  2435,  2440,  2441,  2444,
    2464,  2471,  2495,  2519,  2538,  2557,  2580,  2603,  2608,  2614,
    2623,  2614,  2630,  2631,  2634,  2643,  2634,  2655,  2676,  2676,
    2678,  2685,  2694,  2699,  2704,  2704,  2712,  2712,  2722,  2723,
    2726,  2726,  2737,  2738,  2743,  2771,  2778,  2784,  2789,  2794,
    2794,  2802,  2802,  2807,  2807,  2819,  2819,  2832,  2846,  2832,
    2853,  2873,  2873,  2881,  2881,  2886,  2886,  2896,  2910,  2896,
    2917,  2917,  2927,  2931,  2936,  2963,  2970,  2976,  2981,  2986,
    2986,  2994,  2994,  2999,  2999,  3006,  3015,  3006,  3028,  3047,
    3054,  3061,  3071,  3072,  3074,  3079,  3081,  3082,  3083,  3086,
    3088,  3088,  3094,  3095,  3099,  3121,  3129,  3137,  3153,  3161,
    3168,  3175,  3186,  3198,  3198,  3204,  3205,  3209,  3231,  3239,
    3250,  3260,  3269,  3269,  3275,  3276,  3280,  3285,  3291,  3299,
    3307,  3314,  3321,  3332,  3344,  3344,  3347,  3348,  3352,  3353,
    3358,  3364,  3366,  3367,  3366,  3370,  3371,  3372,  3387,  3389,
    3390,  3389,  3393,  3394,  3395,  3410,  3412,  3414,  3415,  3436,
    3438,  3439,  3440,  3461,  3463,  3464,  3465,  3477,  3477,  3485,
    3486,  3491,  3493,  3494,  3496,  3497,  3499,  3501,  3501,  3510,
    3513,  3513,  3524,  3527,  3537,  3558,  3560,  3561,  3564,  3564,
    3583,  3583,  3592,  3592,  3601,  3604,  3606,  3608,  3609,  3611,
    3613,  3615,  3616,  3618,  3620,  3621,  3623,  3624,  3626,  3628,
    3631,  3635,  3637,  3638,  3640,  3641,  3643,  3645,  3656,  3657,
    3660,  3661,  3673,  3674,  3676,  3677,  3679,  3680,  3686,  3687,
    3690,  3691,  3692,  3718,  3719,  3722,  3723,  3724,  3727,  3727,
    3735,  3737,  3738,  3740,  3741,  3743,  3744,  3746,  3747,  3749,
    3750,  3752,  3753,  3755,  3756,  3759,  3760,  3763,  3765,  3766,
    3770,  3770,  3779,  3781,  3782,  3783,  3784,  3785,  3786,  3787,
    3789,  3790,  3791,  3792,  3793,  3794,  3795,  3796,  3797,  3798,
    3799,  3800,  3801,  3802,  3803,  3804,  3805,  3806,  3807,  3808,
    3809,  3810,  3811,  3812,  3813,  3814,  3815,  3816,  3817,  3818,
    3819,  3820,  3821,  3822,  3823,  3824,  3825,  3826,  3827,  3828,
    3829,  3830,  3831,  3832,  3833,  3834,  3835,  3836,  3837,  3838,
    3839,  3840,  3841,  3842,  3843,  3844,  3845,  3846,  3847,  3848,
    3849,  3850,  3851,  3852,  3853,  3854,  3855,  3856,  3857,  3858,
    3859,  3860,  3861,  3862,  3863,  3864,  3865,  3866,  3869,  3878
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

#define YYPACT_NINF -1012

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1012)))

#define YYTABLE_NINF -587

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1012,    59, -1012, -1012,    44, -1012, -1012, -1012,   197,   197,
   -1012, -1012,  2845,  2845,   197, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
     -18,   197,    -7,   197, -1012, -1012,     5,   413,   413, -1012,
   -1012,   110, -1012, -1012,     6,   277,   197,   197,   197,   197,
   -1012, -1012, -1012, -1012, -1012,    86, -1012, -1012,   147,   108,
   -1012,  2035,  2845,    14,   143,   143,   197, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012,   274, -1012, -1012, -1012,   274, -1012, -1012, -1012,   274,
     197,   197, -1012, -1012,   275,   275,  2845,   197,  1826,   197,
   -1012, -1012, -1012, -1012, -1012,   197, -1012, -1012,  2845,  2845,
     197,   197,   197,   197, -1012, -1012, -1012, -1012,    91,    91,
   -1012, -1012,   197,    70, -1012,   105,    64,   413,   197,   197,
   -1012, -1012, -1012,  2035,  2035,  2035,  2035,   197, -1012,   621,
    1092,    78,   350, -1012, -1012, -1012,   122,   353,   274,   274,
     274,   274,    56,   413,   197,   197,   197,   197,   197,   197,
     197,   197,   197,   197,   197,   197,   197,   197,   197,   197,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012,   316,   413, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,  2845,     8,  2845,
    2845,  2845,     8,  2845,  2845,  2845,  2845,  2845,  2845,  2845,
    2845,  2845,  2385,   197,   413,   197,   246,  1826,   197, -1012,
     246,   342,   413, -1012,   356, -1012,  2937, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   146, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   114,   197, -1012,   197,   173, -1012,   197,
   -1012, -1012, -1012,   367, -1012,   195,   186,   197,   368,   377,
     399,   219,   197,   408,   440,   444,   228,   239,   253,   445,
     448, -1012,   449,   197,   197,   256, -1012,   290,    56, -1012,
     413, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012,   413, -1012, -1012, -1012, -1012,   197,
     264,   274, -1012,   413,   413,   413,   413,   413,   413,   413,
     413,   413,   413,   413,   413,   413,   413,   697,   413,   413,
   -1012, -1012,   214,   603,   375,   532,   291,   169,   123,   313,
     976,    19,  1323,  1270,  1212,   903,   343,   197,   197,   197,
   -1012, -1012, -1012,   268,   300, -1012, -1012,   284,   413, -1012,
   -1012, -1012,   197,   197,   197,   197,   197,   197, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,   197, -1012, -1012,
   -1012, -1012, -1012, -1012,   286, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   298, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   197, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,   197, -1012, -1012, -1012,
   -1012, -1012,   197, -1012, -1012,   304, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,   197, -1012,
   -1012, -1012,   309, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
     310,   300, -1012, -1012, -1012, -1012,   197,   197,   197, -1012,
   -1012, -1012, -1012, -1012,   317,   300, -1012, -1012, -1012, -1012,
   -1012, -1012,   197,   197, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012,   348, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,  2035,
      62,  2035, -1012,   197, -1012,   197,   197, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,  2385, -1012,  2035, -1012,
    2035, -1012, -1012,  2845, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012,  2035, -1012, -1012, -1012, -1012, -1012,  2845,   213, -1012,
     413,   413,   413,  2385, -1012, -1012, -1012, -1012, -1012,    30,
     189,    15, -1012, -1012, -1012, -1012,  2477,  2477, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,   197,   197, -1012,
   -1012, -1012, -1012,   274, -1012, -1012, -1012, -1012,   274,   274,
    2385,   413,  2477,  2477, -1012, -1012, -1012,    52,   274,    58,
   -1012,    62, -1012,   413,   413, -1012,   413,   413,   413,   413,
     413,   274,   413,   413,   413,   413,   413, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,   274, -1012,
     413,   413, -1012, -1012, -1012,   274, -1012, -1012, -1012,   274,
   -1012, -1012, -1012, -1012, -1012,   274, -1012,   413,   413,   274,
   -1012, -1012,   274, -1012,    51, -1012,   413,   413,   413,   413,
     413,   413,   413,   413,   372,   378,   413,   413,   413,   413,
   -1012,   197,   197,   197, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   413,   413,   413, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   459, -1012,   489,   518, -1012,
     519,   197,   197,    51,   197,   197, -1012,   413,   197, -1012,
     197, -1012,   197,   197,   197, -1012,   413, -1012,    51, -1012,
   -1012, -1012,  2937, -1012, -1012, -1012,   527,   395,   197,   534,
     197,   197,   197,   274,   197,   197,   274, -1012, -1012, -1012,
     274, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
     538,   542, -1012,   549, -1012, -1012,  2937,    51,   186,   413,
     413, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
     413, -1012,   413,   413,   115,   413, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,   261,   261,  1053,
     413,   413,   150,   231,  1155,   197,   359,   413,   413,   413,
      51, -1012, -1012, -1012,   403,   411,   197,   197, -1012,   416,
     548, -1012,   382, -1012, -1012, -1012,   469,   375,   658,   197,
     197, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,    79, -1012,   551,   163,
     937, -1012, -1012, -1012, -1012, -1012, -1012,    26,    92, -1012,
     197,   197,   197,   197,   300, -1012, -1012, -1012, -1012,   197,
   -1012,   197, -1012,   424,   197,   197, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,  2035,  2035, -1012, -1012,
   -1012, -1012, -1012,   274, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,   413,   413,   413, -1012,
     274,   397,   524, -1012, -1012, -1012,   274,   191,  2035,  2035,
    2035, -1012,   413,  2035,   413,  2845,   469, -1012,    52,    58,
     413,   274,   274,   413,   413,   197,   197, -1012,   413,   413,
     413, -1012,  2937, -1012, -1012,   450, -1012, -1012,   197,   197,
   -1012, -1012,   274, -1012,   556, -1012,   566, -1012,   567, -1012,
     413,   578, -1012,   382,   597, -1012, -1012,   274,   274, -1012,
   -1012, -1012,   397, -1012,  2569, -1012,   197, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   454, -1012, -1012, -1012,   138,
     413,   413,   413,   413,   413,   413,   413,   413,   413,   544,
   -1012,   292,   216,   221,   437,   314,   659, -1012,  2937, -1012,
   -1012, -1012, -1012,   197, -1012, -1012, -1012, -1012, -1012,   197,
   -1012, -1012, -1012, -1012, -1012,   544,   544,   360,   189,   458,
     461, -1012, -1012, -1012, -1012,   274, -1012, -1012, -1012,   274,
   -1012, -1012,   544, -1012,   197, -1012,   197,   490,   544, -1012,
     544,   472,   493,   544,   544,   484,   576, -1012,   544, -1012,
   -1012,   494,  2661,   544, -1012, -1012, -1012,  2753, -1012,   498,
     544,  2937, -1012
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     635,     0,     2,     3,     0,     1,   633,   634,     0,     0,
     636,   635,     0,     0,   637,   651,     4,   650,   652,   653,
     654,   655,   656,   657,   658,   659,   660,   661,   662,   663,
     664,   665,   666,   667,   668,   669,   670,   671,   672,   673,
     674,   675,   676,   677,   678,   679,   680,   681,   682,   683,
     684,   685,   686,   687,   688,   689,   690,   691,   692,   693,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   607,     9,   728,   635,    16,   635,
       0,   606,     0,     6,   615,   615,     5,    12,    19,   635,
     618,    10,   617,   616,    17,     0,   621,     0,     0,     0,
      28,    13,    14,    15,    28,     0,    20,     7,     0,   623,
     622,     0,     0,     0,    46,    46,     0,    22,   635,   635,
     627,   619,   635,   620,   643,   646,   644,   648,   649,   647,
     642,     0,   640,   645,   604,     0,   635,    24,    26,     0,
       0,     0,    29,    30,    55,    55,     0,     8,   629,   625,
     615,   615,    27,   635,    45,   605,   635,    23,     0,     0,
       0,     0,     0,     0,    47,    48,    49,    50,    68,    68,
      42,   609,   621,     0,   608,   624,     0,   613,   641,    25,
      31,    34,    35,     0,     0,     0,     0,     0,   615,     0,
       0,     0,     0,   628,   635,   615,     0,     0,     0,     0,
       0,     0,     0,    56,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     635,   615,    70,    71,    72,    73,    74,    75,   225,   226,
     227,   228,   229,   230,   231,    76,    77,    78,    79,   635,
     615,   635,   635,   630,     0,   614,   615,   615,    37,   615,
      52,    53,    51,    54,    65,    67,    57,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    69,    18,     0,   629,   631,   635,
       0,    39,    36,   635,     0,   353,     0,   419,   404,   566,
     635,   316,   232,    80,   473,   465,   635,   105,   214,   115,
     253,   272,   292,   450,   420,     0,   128,   729,   612,   368,
     635,   635,    43,     0,   626,    32,     0,     0,    40,    66,
     615,    59,    58,     0,   564,     0,   565,   418,     0,     0,
       0,     0,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   615,     0,   610,   611,     0,   635,    39,     0,    38,
      60,   615,   355,   354,   615,   567,   615,   318,   317,   615,
     234,   233,   615,    82,    81,   615,   615,   107,   106,   615,
     216,   215,   615,   117,   116,   615,   615,   615,   615,   452,
     451,   615,   422,   421,   131,   615,   370,   369,   615,   632,
       0,     0,    62,   357,   407,   321,   236,    84,   468,   109,
     218,   120,   256,   275,   295,   454,   424,   126,   372,    44,
     615,    41,     0,     0,   405,   319,     0,     0,   466,     0,
       0,   118,   254,   273,   293,     0,     0,     0,     0,     0,
     137,   138,   136,     0,     0,   134,   135,     0,    33,    61,
      63,    64,     0,     0,     0,     0,     0,     0,   356,   366,
     367,   365,   360,   358,   363,   364,   361,     0,   412,   413,
     411,   410,   414,   416,     0,   408,   329,   330,   328,   324,
     325,   335,   336,   337,   338,     0,   331,   333,   334,   339,
     322,   326,   327,     0,   235,   245,   246,   244,   239,   249,
     247,   251,   237,   243,   242,   240,     0,    83,    87,    88,
      85,    86,     0,   469,   470,     0,   108,   112,   113,   111,
     110,   217,   220,   221,   219,   615,   615,   615,     0,   123,
     124,   122,     0,   121,   270,   271,   269,   259,   263,   266,
       0,     0,   257,   267,   268,   264,     0,     0,     0,   290,
     291,   289,   278,   282,     0,     0,   276,   285,   286,   287,
     288,   283,     0,     0,   308,   309,   307,   298,   310,   312,
     314,     0,   296,   303,   304,   305,   306,   299,   302,   301,
     453,   457,   458,   456,   455,   459,   461,   463,   635,   635,
     423,   427,   428,   426,   425,   429,   431,   433,   435,     0,
       0,     0,   127,     0,   615,     0,     0,   371,   377,   378,
     376,   375,   379,   381,   373,   522,     0,   529,     0,    98,
       0,   615,   615,     0,   615,   615,   406,   615,   320,   615,
     615,     0,   615,   615,   615,   615,   615,     0,     0,   467,
     223,   222,   224,     0,   119,   255,   261,   615,   615,     0,
       0,     0,   274,   280,   615,   615,     0,     0,   615,   615,
     615,   294,   615,   615,   615,   615,   615,   437,   447,   615,
     615,   615,   615,     0,   635,   635,   635,   104,     0,     0,
       0,   133,     0,     0,   615,   615,   615,     0,     0,     0,
     547,     0,   514,   359,   362,   340,   415,   417,   409,   332,
     323,     0,   250,   248,   252,   238,   241,    89,   635,   635,
     635,   635,   471,   635,   474,   476,   478,   477,     0,   615,
     258,   265,   597,   635,   542,     0,   635,   598,   538,     0,
     599,   635,   635,   635,   546,     0,   615,   277,   284,     0,
     556,   557,     0,   551,     0,   568,   311,   313,   315,   297,
     300,   460,   462,   464,     0,     0,   430,   432,   434,   436,
     213,   101,   103,   102,    97,   212,   129,   403,   399,   635,
     388,   383,   635,   380,   382,   374,   635,   635,   527,   523,
     114,   635,   635,   534,   530,     0,    99,     0,     0,   535,
       0,   480,   493,     0,   502,   475,   125,   262,   540,   539,
     541,   536,   537,   545,   544,   543,   281,   559,     0,   553,
     552,   555,     0,   566,   615,   615,     0,     0,   402,     0,
     387,   526,   525,     0,   533,   532,     0,   615,   549,   548,
       0,   615,   516,   515,   615,   342,   341,   615,    91,   615,
       0,     0,   479,     0,   558,   562,     0,     0,   569,   440,
     440,   615,   139,   130,   615,   615,   385,   384,   524,   531,
     161,   100,   518,   344,     0,    90,   615,   482,   481,   615,
     495,   494,   615,   504,   503,   560,   554,   438,   448,   147,
     407,   389,     0,     0,     0,     0,     0,   484,   497,   506,
       0,   441,   443,   445,     0,     0,     0,     0,   144,     0,
     141,   142,     0,   143,   145,   146,     0,   400,     0,     0,
       0,   550,   164,   165,   162,   163,   517,   519,   520,   343,
     349,   350,   348,   347,   351,   345,     0,    92,     0,     0,
       0,   561,   615,   615,   615,   439,   449,   570,     0,   140,
       0,     0,     0,     0,     0,   150,   148,   149,   615,     0,
     615,     0,   196,     0,     0,     0,   386,   397,   398,   392,
     393,   394,   391,   395,   396,   615,     0,     0,   615,   615,
     635,   635,    96,     0,   483,   485,   488,   489,   490,   491,
     492,   615,   487,   496,   498,   501,   615,   500,   505,   615,
     508,   509,   510,   511,   512,   513,   442,   444,   446,   574,
       0,     0,     0,   635,   635,   193,     0,     0,     0,     0,
       0,   615,   151,     0,   172,     0,   195,   401,     0,     0,
     390,     0,     0,   352,   346,    95,    94,    93,   486,   499,
     507,   189,     0,   577,   571,     0,   573,   581,   192,   191,
     190,   156,     0,   635,     0,   158,     0,   167,     0,   563,
     152,     0,   175,   171,     0,   199,   197,     0,     0,   211,
     210,   586,   576,   580,     0,   154,   155,   615,   159,   615,
     615,   168,   615,   615,   187,   186,   615,   176,   174,   615,
     615,   200,   615,   521,   528,   575,   578,   582,   579,   584,
     161,   157,   161,   166,   161,   178,   173,   202,   198,   638,
     583,     0,     0,     0,     0,     0,     0,   639,     0,   585,
     160,   169,   188,     0,   177,   181,   182,   180,   179,     0,
     201,   205,   206,   204,   203,   638,   638,     0,     0,     0,
       0,   602,   603,   600,   185,     0,   635,   601,   209,     0,
     635,   587,   638,   183,   184,   207,   208,     0,   638,   588,
     638,     0,     0,   638,   638,     0,     0,   596,   638,   589,
     592,     0,     0,   638,   593,   594,   591,   638,   590,     0,
     638,     0,   595
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1012, -1012,   616, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   487, -1012, -1012, -1012,   483,
   -1012, -1012, -1012,   429, -1012, -1012, -1012,   260, -1012, -1012,
   -1012, -1012,  -162,   485, -1012, -1012,    -6,    45,   467, -1012,
   -1012, -1012, -1012, -1012,   299, -1012,   463, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,  -424,  -362, -1012,
   -1012,   -41, -1012, -1012, -1012, -1012,  -400, -1012, -1012, -1012,
   -1012, -1012,  -217,  -315, -1012, -1012,  -545, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1011,
   -1012, -1012, -1012, -1012, -1012, -1012,  -384, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
    -346, -1012, -1012, -1012, -1012, -1012, -1012, -1012,  -415,  -423,
    -303, -1012, -1012, -1012,  -317,   251, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012,   252, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012,   263, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
     271, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,   272, -1012,
   -1012, -1012, -1012, -1012,   273, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,  -195,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,  -161, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,  -381,
   -1012, -1012, -1012,  -236,   -45, -1012, -1012,  -316,  -235,  -370,
   -1012, -1012,  -305,  -204,  -412, -1012,  -407, -1012,   285, -1012,
    -418, -1012, -1012,  -675, -1012,  -126, -1012, -1012, -1012,  -166,
   -1012, -1012,  -339,   453,   -87,  -678, -1012, -1012, -1012, -1012,
    -335,  -369, -1012, -1012,  -331, -1012, -1012, -1012,  -355, -1012,
   -1012, -1012,  -427, -1012, -1012, -1012,  -648,  -392, -1012, -1012,
   -1012,   -11,  -115,  -605,   631,  1308, -1012, -1012,   559, -1012,
   -1012, -1012, -1012,   462, -1012,    -4,    -3,   635,   204,    10,
   -1012, -1012,  -101,  -123, -1012
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    94,    99,   106,   138,     2,   100,   120,   111,
       3,   102,   124,   114,   125,   121,   159,   176,   122,   134,
     162,   216,   367,   200,   163,   217,   269,   337,   338,   137,
     211,   365,   123,   164,   184,   185,   922,   923,   188,   208,
     304,   342,   412,   432,   276,   303,   209,   241,   242,   350,
     384,   437,   520,   800,   849,   896,   983,   480,   471,   701,
     840,   688,   243,   353,   388,   439,   481,   244,   355,   394,
     542,   441,   543,   245,   453,   325,   614,   826,   427,   454,
     863,   909,   910,   954,   955,  1052,   956,  1054,  1079,   892,
     957,  1056,  1082,   911,   912,  1063,   960,  1061,  1088,  1115,
    1128,  1145,   958,  1085,   913,   914,  1016,   915,   916,  1026,
     962,  1064,  1092,  1116,  1134,  1149,   924,   925,   455,   456,
     246,   354,   391,   440,   247,   248,   349,   381,   436,   645,
     646,   643,   642,   644,   249,   356,   550,   442,   657,   551,
     729,   658,   250,   357,   564,   443,   664,   565,   746,   665,
     251,   358,   581,   444,   672,   673,   668,   669,   670,   252,
     348,   378,   495,   435,   640,   639,   496,   497,   483,   798,
     846,   894,   979,   978,   253,   343,   373,   433,   631,   632,
     254,   362,   407,   457,   696,   694,   695,   622,   829,   867,
     781,   918,   623,   827,   963,   778,   255,   345,   484,   434,
     637,   634,   635,   308,   256,   360,   403,   446,   679,   680,
     681,   682,   607,   764,   904,   887,   942,   943,   944,   608,
     765,   905,   257,   359,   400,   445,   674,   675,   676,   258,
     351,   525,   438,   315,   722,   723,   724,   725,   850,   878,
     938,   726,   851,   881,   939,   727,   853,   884,   940,   485,
     797,   843,   893,   970,   474,   697,   833,   789,   971,   475,
     699,   836,   794,   514,   568,   739,   569,   735,   570,   745,
     476,   795,   839,   588,   753,   820,   589,   750,   818,   854,
     900,  1058,   309,   310,   346,   754,   823,  1010,  1011,  1012,
    1043,  1044,  1072,  1046,  1047,  1074,  1098,  1110,  1095,  1135,
    1159,  1169,  1170,  1172,  1177,  1160,   740,   741,  1146,  1147,
     155,   201,   755,   329,   821,   107,   112,   116,   128,   129,
     143,   195,   141,   193,   263,   113,     4,   130,  1118,   151,
     173,   152,    96,    97,   331
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    95,    98,   452,    11,    11,   656,   375,    14,   156,
      10,   736,   490,   521,   779,   782,    16,   541,   515,   549,
     663,   698,    16,    16,   555,   571,   587,   548,   563,   -21,
     153,   157,   584,   472,    16,   489,   508,   585,    16,   530,
    -572,   538,   547,   562,   577,   594,   604,   732,   728,     6,
     181,     7,   473,   192,   500,   512,     6,   621,     7,     5,
      16,   552,   566,   582,    16,   502,    16,   194,   274,     6,
      16,     7,   554,   488,   507,   110,   624,   529,   534,   104,
     546,   561,   576,   593,   603,   776,   183,    16,   119,  1112,
     105,  1113,   448,  1114,   101,   620,   103,    10,     8,    10,
      16,   109,   153,   153,   153,   153,   115,   742,   306,   136,
     743,    10,    11,    11,    11,    11,   142,   482,     9,   511,
       6,   509,     7,   537,   733,   535,   110,   580,   597,   578,
     595,   605,    11,   510,   332,   167,   168,   536,   335,   169,
     786,   579,   596,   606,   857,   787,   791,   684,   170,   685,
     171,   792,   686,   175,   181,   190,    11,    11,   186,   186,
     522,   207,   215,    10,    10,    10,   117,   980,   213,   330,
     198,    10,   981,   199,   118,   260,    11,    11,    11,    11,
    1013,   181,   192,   919,   920,  1014,   160,   161,    11,   516,
     183,   344,   119,   447,    10,    10,   194,    16,   895,    16,
     181,   214,     6,    11,     7,   194,   732,   737,   732,   187,
     187,   264,   366,   218,   219,   220,   221,   183,   465,   266,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,   183,   293,  -586,  -586,
     573,   449,   448,   361,   139,   181,   140,   181,   921,   919,
     920,     6,   181,     7,   919,   920,   295,   110,   297,   298,
      10,   993,   181,   992,   997,   224,   305,   517,   311,   312,
     313,   369,   317,   318,   319,   320,   321,   322,   323,   324,
     326,   183,     6,   183,     7,   127,   306,   226,   183,    10,
     227,    10,   374,    10,    10,   224,   334,   718,   183,   719,
     339,   231,   720,   180,   721,   615,   181,   347,   233,   234,
     118,   235,   459,   352,  1120,   181,   385,   226,   462,  1121,
     227,     6,   181,     7,   299,   395,   463,   363,   364,   926,
      10,   231,    11,   463,   182,    10,   396,   238,   233,   234,
     239,   235,   183,    10,   181,   181,   465,     6,    10,     7,
     397,   183,   616,   110,   408,   463,   503,   466,   183,    10,
      10,   336,   430,   409,   466,   467,   612,   238,    16,  1053,
     239,   170,   467,   171,   181,   613,  1141,  1142,  1143,   224,
     183,   183,   617,   231,   636,   463,   466,   448,   598,   504,
     501,   513,  1042,  1109,  1123,   999,   638,   553,   567,   583,
     477,   226,   649,   599,   227,    10,   181,   654,   655,  1021,
     183,   526,  1124,   336,   959,   662,   466,   463,     6,   238,
       7,   450,   233,   234,   110,   235,   460,   469,   478,   486,
     505,   518,   523,   527,   532,   539,   544,   559,   574,   591,
     601,   600,   183,    11,    11,    11,   671,   261,   448,   262,
     267,   618,   268,   340,   239,   341,   467,   937,    11,    11,
      11,    11,    11,    11,   371,   376,   372,   377,   181,   824,
     919,   920,   451,    11,   379,   825,   380,   461,   470,   479,
     487,   506,   519,   524,   528,   533,   540,   545,   560,   575,
     592,   602,   864,   961,   933,   969,   382,  1042,   383,    11,
     975,   945,   619,   330,   183,   386,   973,   387,   153,   946,
     153,   974,    11,   935,   949,   986,   995,  1001,    11,   375,
     991,   996,  1027,   985,   994,  1000,   989,   153,  1004,   153,
     330,   990,   932,  1005,    11,  1122,   224,   389,  1045,   390,
     153,   392,   398,   393,   399,   401,   405,   402,   406,  1117,
    1073,   751,    11,    11,    11,  1109,   837,   477,   838,   462,
    1151,   227,   447,   181,  1152,   194,   194,   330,    11,    11,
     903,   903,   901,   901,   463,  1158,  1163,   934,   964,   233,
     234,   447,   235,   464,   902,   902,   841,   950,   842,  1167,
    1168,   194,   194,  1164,  1173,   677,   678,   951,  1180,   183,
     482,   126,   965,   556,   557,   466,   465,   135,   202,    11,
     952,    11,    11,   467,   953,   844,   847,   845,   848,   683,
     165,   689,   705,  -153,   861,   224,   862,   410,   573,   449,
     462,   865,   189,   866,   181,   876,   717,   877,   700,   879,
     702,   880,   225,    12,    13,   463,   882,   226,   883,   984,
     227,   711,   210,  1077,   464,  1078,   228,   229,   465,   230,
     796,   231,   232,  1080,  1083,  1081,  1084,   411,   233,   234,
     183,   235,   908,    10,    10,  1086,   466,  1087,   236,  1089,
    1066,   771,   772,   773,   467,   964,   491,   492,   447,   181,
     181,  1127,  1133,   237,  1090,   917,  1091,   238,   493,   888,
     239,   468,   987,   988,  1002,  1003,   494,   498,   499,   965,
     556,   557,  1067,   465,   972,   801,   802,   803,   804,   240,
     805,   194,  1129,   503,  1068,   183,   183,   447,   181,   586,
     808,   886,   448,   810,   941,   316,   858,  1096,   812,   813,
     814,   885,  1119,  1097,  1111,  1174,  1150,   150,   154,   158,
      11,   212,   131,   132,   133,   194,   966,  1130,     0,   333,
       0,     0,     0,     0,   183,     0,     0,    10,    10,    10,
     448,   166,  -132,     0,     0,   449,   828,     0,     0,   830,
       0,     0,   172,   831,   832,     0,   174,     0,   834,   835,
     177,     0,     0,     0,     0,   178,   179,    10,    10,    10,
      10,    10,     0,     0,    10,     0,    10,     0,    10,    10,
      10,     0,     0,     0,    11,   203,   204,   205,   206,   150,
     150,   150,   150,     0,    10,     0,    10,    10,    10,     0,
      10,    10,     0,     0,     0,     0,     0,     0,   275,     0,
       0,     0,   222,     0,     0,     0,     0,     0,     0,   270,
     271,   272,   273,    11,     0,     0,     0,     0,     0,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   153,   153,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   927,   930,     0,
       0,    11,     0,     0,   307,     0,    11,     0,   314,     0,
       0,     0,    11,    11,     0,     0,     0,   224,   328,     0,
       0,   478,   967,     0,     0,    11,    11,   153,   153,   153,
       0,     0,   153,     0,     0,     0,     0,  1071,     0,   226,
       0,     0,   227,     0,   181,     0,     0,     0,   928,   931,
       0,   194,     0,   231,     0,   463,    11,    11,    11,    11,
     233,   234,     0,   235,     0,    11,     0,    11,     0,  1099,
      11,    11,   479,   968,   964,     0,     0,   447,     0,     0,
     183,   368,     0,   194,     0,     0,   466,  1035,  1036,   238,
     224,     0,   239,     0,   275,     0,  1031,  1032,   965,   556,
     557,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   590,   226,  1136,     0,   227,     0,   181,     0,     0,
    1048,  1049,   613,     0,  1065,   449,   231,   194,     0,     0,
       0,     0,     0,   233,   234,     0,   235,     0,  1055,  1057,
    1059,    10,    10,  1062,     0,   998,     0,     0,     0,     0,
       0,     0,   431,   183,    10,    10,     0,     0,     0,   466,
    1076,     0,   238,     0,     0,   239,     0,  1175,     6,     0,
       7,     0,  1178,     0,   110,     0,  1182,     0,     0,     0,
       0,   194,    10,     0,   531,   538,   194,  -194,     0,     0,
     194,     0,   609,   610,   611,  -170,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   224,   625,   626,   627,
     628,   629,   630,     0,     0,     0,     0,     0,     0,  1125,
    1131,     0,   633,   225,   906,     0,     0,     0,   226,    11,
       0,   227,   907,     0,     0,    11,     0,   228,   229,     0,
     230,     0,   231,   232,     0,     0,     0,     0,   641,   233,
     234,     0,   235,  1154,     0,     0,     0,  1156,     0,   236,
      10,   647,    10,     0,     0,     0,     0,   648,     0,   224,
    1126,  1132,     0,     0,   237,     0,     0,     0,   238,     0,
       0,   239,     0,   653,     0,     0,     0,     0,     0,     0,
       0,   226,     0,     0,   227,     0,   181,     0,     0,     0,
     259,   659,   660,   661,     0,     0,     0,   463,     0,     0,
       0,     0,   233,   234,     0,   235,     0,   666,   667,     0,
       0,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,     0,   183,     0,     0,   150,   687,   150,   466,     0,
       0,     0,     0,     0,   239,     0,   467,     0,   226,   462,
       0,   227,   328,   181,   150,     0,   150,     0,   690,     0,
     692,   693,   231,   929,   463,     0,     0,   150,   572,   233,
     234,     0,   235,     0,   556,   557,     0,   465,     0,   328,
     558,     0,     0,     0,     0,   734,   738,   744,     0,   183,
       0,     0,   749,   752,     0,   466,     0,     0,   238,   573,
       0,   239,     0,   467,     0,     0,     0,   462,     0,     0,
       0,   181,     0,     0,     0,     0,   328,     0,   777,   780,
       0,     0,   463,   788,   770,   793,     0,   687,     0,   774,
     775,     0,   556,   557,     0,   465,     0,     0,   558,   790,
       0,     0,     0,     0,     0,     0,     0,   183,     0,  1139,
    1140,     0,   799,   466,     0,  -279,     0,     0,   449,     0,
     462,   467,     0,   447,   181,     0,  1157,     0,     0,   806,
       0,     0,  1161,     0,  1162,   463,   809,  1165,  1166,     0,
     811,     0,  1171,     0,   464,     0,   815,  1176,   465,     0,
     817,  1179,     0,   819,  1181,     0,     0,     0,     0,   822,
     183,     0,     0,     0,     0,     0,   466,     0,  -260,     0,
       0,   449,     0,     0,   467,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   852,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   855,
       0,     0,     0,   856,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   868,     0,     0,   869,     0,     0,
       0,   871,     0,     0,     0,     0,     0,     0,   196,   197,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   822,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,   265,     0,     0,     0,     0,     0,     0,
     936,   855,     0,     0,     0,   856,     0,     0,     0,     0,
       0,   947,   948,     0,     0,     0,     0,     0,     0,   294,
       0,     0,   982,     0,   976,   977,     0,     0,     0,     0,
       0,     0,     0,  1009,  1015,     0,     0,     0,   296,     0,
       0,     0,     0,     0,   300,   301,     0,   302,     0,     0,
       0,     0,     0,     0,     0,  1017,  1018,  1019,  1020,     0,
       0,     0,   150,   150,  1023,     0,  1025,     0,     0,  1028,
    1029,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1037,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1051,   150,   150,   150,     0,     0,   150,
       0,  1041,     0,     0,   788,   793,     0,  1050,   370,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1069,  1070,     0,     0,     0,     0,     0,   404,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   413,
       0,     0,   414,  1075,   415,     0,     0,   416,     0,     0,
     417,     0,     0,   418,   419,     0,     0,   420,  1093,  1094,
     421,     0,     0,   422,   423,   424,   425,     0,     0,   426,
       0,     0,     0,   428,     0,     0,   429,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   458,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1144,  1148,     0,     0,     0,  1137,     0,
       0,     0,     0,     0,  1138,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1153,     0,     0,     0,
    1155,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,     6,     0,     7,     0,     0,    17,   191,     0,     0,
       0,     0,     0,   650,   651,   652,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,   691,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   703,
     704,     0,   706,   707,     0,   708,     0,   709,   710,     0,
     712,   713,   714,   715,   716,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   730,   731,     0,     0,     0,
       0,     0,   747,   748,     0,     0,   756,   757,   758,     0,
     759,   760,   761,   762,   763,     0,     0,   766,   767,   768,
     769,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   783,   784,   785,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   807,     0,    15,
       0,     0,     0,    16,   144,    17,   145,   146,     0,     0,
       0,   147,   148,   149,   816,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,   859,   860,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   870,     0,     0,     0,   872,
       0,     0,   873,     0,     0,   874,     0,   875,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   889,
       0,     0,   890,   891,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   897,     0,     0,   898,     0,     0,
     899,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1006,  1007,  1008,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1022,     0,  1024,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1030,     0,     0,  1033,  1034,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1038,
       0,     0,     0,     0,  1039,     0,     0,  1040,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1060,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1100,     0,  1101,  1102,    15,
    1103,  1104,     0,    16,  1105,    17,   327,  1106,  1107,     0,
    1108,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    15,     0,     0,     0,    16,     0,    17,   191,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    15,     0,     0,     0,     0,     0,    17,
     191,     0,     0,  1045,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    15,     0,     0,     0,     0,
       0,    17,   191,     0,     0,  1168,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    15,  1117,     0,
       0,     0,     0,    17,   191,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    15,
       0,     0,     0,    16,     0,    17,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    15,     0,     0,     0,     0,     0,    17,   191,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93
};

static const yytype_int16 yycheck[] =
{
       4,    12,    13,   427,     8,     9,   551,   346,    11,   132,
      14,   659,   435,   437,   692,   693,     8,   441,   436,   442,
     565,   626,     8,     8,   442,   443,   444,   442,   443,    23,
     131,    17,   444,   433,     8,   435,   436,   444,     8,   439,
      14,    22,   442,   443,   444,   445,   446,    17,   653,     5,
      31,     7,   433,   168,   435,   436,     5,   457,     7,     0,
       8,   442,   443,   444,     8,   435,     8,   168,    12,     5,
       8,     7,   442,   435,   436,    11,   457,   439,   440,    97,
     442,   443,   444,   445,   446,   690,    67,     8,    82,  1100,
      97,  1102,    73,  1104,    97,   457,    99,   101,    54,   103,
       8,    96,   203,   204,   205,   206,   109,    92,   100,    23,
      95,   115,   116,   117,   118,   119,     8,   434,    74,   436,
       5,   436,     7,   440,    94,   440,    11,   444,   445,   444,
     445,   446,   136,   436,   296,   138,   139,   440,   300,   142,
      88,   444,   445,   446,   822,    93,    88,    85,    97,    87,
      99,    93,    90,   156,    31,   166,   160,   161,   164,   165,
      37,    70,    98,   167,   168,   169,    56,    88,    98,   292,
     173,   175,    93,   176,    64,    97,   180,   181,   182,   183,
      88,    31,   297,    33,    34,    93,    43,    44,   192,    20,
      67,   306,    82,    30,   198,   199,   297,     8,    83,     8,
      31,    96,     5,   207,     7,   306,    17,    18,    17,   164,
     165,   214,    98,   203,   204,   205,   206,    67,    55,    97,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,    67,   240,   100,   101,
      77,    78,    73,    97,    97,    31,    99,    31,    98,    33,
      34,     5,    31,     7,    33,    34,   259,    11,   261,   262,
     264,    98,    31,   938,   939,     4,   277,    98,   279,   280,
     281,    98,   283,   284,   285,   286,   287,   288,   289,   290,
     291,    67,     5,    67,     7,     8,   100,    26,    67,   293,
      29,   295,    97,   297,   298,     4,   299,    84,    67,    86,
     303,    40,    89,    28,    91,    21,    31,   310,    47,    48,
      64,    50,    98,   316,    98,    31,    97,    26,    27,    98,
      29,     5,    31,     7,     8,    97,    42,   330,   331,    98,
     334,    40,   336,    42,    59,   339,    97,    76,    47,    48,
      79,    50,    67,   347,    31,    31,    55,     5,   352,     7,
      97,    67,    68,    11,    98,    42,    65,    73,    67,   363,
     364,    71,    98,   366,    73,    81,    98,    76,     8,  1017,
      79,    97,    81,    99,    31,    75,    16,    17,    18,     4,
      67,    67,    98,    40,    98,    42,    73,    73,    45,    98,
     435,   436,   100,   101,    80,   940,    98,   442,   443,   444,
      25,    26,    98,    60,    29,   409,    31,    98,    98,   954,
      67,    98,    98,    71,    32,    98,    73,    42,     5,    76,
       7,   427,    47,    48,    11,    50,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,    98,    67,   447,   448,   449,    98,    97,    73,    99,
      97,   457,    99,    97,    79,    99,    81,    98,   462,   463,
     464,   465,   466,   467,    97,    97,    99,    99,    31,    97,
      33,    34,   427,   477,    97,    97,    99,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,    97,    24,   894,   918,    97,   100,    99,   503,
     918,    98,   457,   626,    67,    97,   918,    99,   609,    98,
     611,   918,   516,   894,    98,   938,   939,   940,   522,   858,
     938,   939,    98,   938,   939,   940,   938,   628,   940,   630,
     653,   938,   894,   940,   538,    98,     4,    97,    14,    99,
     641,    97,    97,    99,    99,    97,    97,    99,    99,     5,
     100,   666,   556,   557,   558,   101,    97,    25,    99,    27,
     102,    29,    30,    31,   103,   666,   667,   690,   572,   573,
     887,   888,   887,   888,    42,    85,   104,   894,    27,    47,
      48,    30,    50,    51,   887,   888,    97,    39,    99,   105,
      14,   692,   693,   100,   100,   598,   599,    49,   100,    67,
     917,   114,    51,    52,    53,    73,    55,   124,   179,   613,
      62,   615,   616,    81,    66,    97,    97,    99,    99,   609,
     135,   611,   633,    75,    97,     4,    99,   367,    77,    78,
      27,    97,   165,    99,    31,    97,   647,    99,   628,    97,
     630,    99,    21,     8,     9,    42,    97,    26,    99,    98,
      29,   641,   189,    97,    51,    99,    35,    36,    55,    38,
     701,    40,    41,    97,    97,    99,    99,   368,    47,    48,
      67,    50,   889,   677,   678,    97,    73,    99,    57,  1063,
    1026,   684,   685,   686,    81,    27,   435,   435,    30,    31,
      31,  1115,  1116,    72,    97,   890,    99,    76,   435,   860,
      79,    98,   938,   938,   940,   940,   435,   435,   435,    51,
      52,    53,  1028,    55,   918,   718,   719,   720,   721,    98,
     723,   822,    63,    65,  1029,    67,    67,    30,    31,   444,
     733,   857,    73,   736,   900,   282,   823,  1072,   741,   742,
     743,   856,  1111,  1074,  1099,  1172,  1138,   131,   132,   133,
     754,   192,   117,   118,   119,   856,    98,    98,    -1,   297,
      -1,    -1,    -1,    -1,    67,    -1,    -1,   771,   772,   773,
      73,   136,    75,    -1,    -1,    78,   779,    -1,    -1,   782,
      -1,    -1,   151,   786,   787,    -1,   155,    -1,   791,   792,
     159,    -1,    -1,    -1,    -1,   160,   161,   801,   802,   803,
     804,   805,    -1,    -1,   808,    -1,   810,    -1,   812,   813,
     814,    -1,    -1,    -1,   818,   180,   181,   182,   183,   203,
     204,   205,   206,    -1,   828,    -1,   830,   831,   832,    -1,
     834,   835,    -1,    -1,    -1,    -1,    -1,    -1,   222,    -1,
      -1,    -1,   207,    -1,    -1,    -1,    -1,    -1,    -1,   218,
     219,   220,   221,   857,    -1,    -1,    -1,    -1,    -1,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   976,   977,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   893,   894,    -1,
      -1,   895,    -1,    -1,   278,    -1,   900,    -1,   282,    -1,
      -1,    -1,   906,   907,    -1,    -1,    -1,     4,   292,    -1,
      -1,   917,   918,    -1,    -1,   919,   920,  1018,  1019,  1020,
      -1,    -1,  1023,    -1,    -1,    -1,    -1,  1042,    -1,    26,
      -1,    -1,    29,    -1,    31,    -1,    -1,    -1,   893,   894,
      -1,  1042,    -1,    40,    -1,    42,   950,   951,   952,   953,
      47,    48,    -1,    50,    -1,   959,    -1,   961,    -1,  1074,
     964,   965,   917,   918,    27,    -1,    -1,    30,    -1,    -1,
      67,   336,    -1,  1074,    -1,    -1,    73,   980,   981,    76,
       4,    -1,    79,    -1,   368,    -1,   976,   977,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    26,  1118,    -1,    29,    -1,    31,    -1,    -1,
    1013,  1014,    75,    -1,  1025,    78,    40,  1118,    -1,    -1,
      -1,    -1,    -1,    47,    48,    -1,    50,    -1,  1018,  1019,
    1020,  1035,  1036,  1023,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,   411,    67,  1048,  1049,    -1,    -1,    -1,    73,
    1053,    -1,    76,    -1,    -1,    79,    -1,  1172,     5,    -1,
       7,    -1,  1177,    -1,    11,    -1,  1181,    -1,    -1,    -1,
      -1,  1172,  1076,    -1,    98,    22,  1177,    24,    -1,    -1,
    1181,    -1,   447,   448,   449,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,   462,   463,   464,
     465,   466,   467,    -1,    -1,    -1,    -1,    -1,    -1,  1115,
    1116,    -1,   477,    21,    61,    -1,    -1,    -1,    26,  1123,
      -1,    29,    69,    -1,    -1,  1129,    -1,    35,    36,    -1,
      38,    -1,    40,    41,    -1,    -1,    -1,    -1,   503,    47,
      48,    -1,    50,  1146,    -1,    -1,    -1,  1150,    -1,    57,
    1154,   516,  1156,    -1,    -1,    -1,    -1,   522,    -1,     4,
    1115,  1116,    -1,    -1,    72,    -1,    -1,    -1,    76,    -1,
      -1,    79,    -1,   538,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    29,    -1,    31,    -1,    -1,    -1,
      98,   556,   557,   558,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    47,    48,    -1,    50,    -1,   572,   573,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,   609,   610,   611,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    81,    -1,    26,    27,
      -1,    29,   626,    31,   628,    -1,   630,    -1,   613,    -1,
     615,   616,    40,    98,    42,    -1,    -1,   641,    46,    47,
      48,    -1,    50,    -1,    52,    53,    -1,    55,    -1,   653,
      58,    -1,    -1,    -1,    -1,   659,   660,   661,    -1,    67,
      -1,    -1,   666,   667,    -1,    73,    -1,    -1,    76,    77,
      -1,    79,    -1,    81,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,   690,    -1,   692,   693,
      -1,    -1,    42,   697,   683,   699,    -1,   701,    -1,   688,
     689,    -1,    52,    53,    -1,    55,    -1,    -1,    58,   698,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,  1135,
    1136,    -1,   711,    73,    -1,    75,    -1,    -1,    78,    -1,
      27,    81,    -1,    30,    31,    -1,  1152,    -1,    -1,   728,
      -1,    -1,  1158,    -1,  1160,    42,   735,  1163,  1164,    -1,
     739,    -1,  1168,    -1,    51,    -1,   745,  1173,    55,    -1,
     749,  1177,    -1,   752,  1180,    -1,    -1,    -1,    -1,   754,
      67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    75,    -1,
      -1,    78,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   803,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   818,
      -1,    -1,    -1,   818,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   833,    -1,    -1,   836,    -1,    -1,
      -1,   840,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   857,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   208,    -1,    -1,    -1,
      -1,    -1,    -1,   215,    -1,    -1,    -1,    -1,    -1,    -1,
     895,   900,    -1,    -1,    -1,   900,    -1,    -1,    -1,    -1,
      -1,   906,   907,    -1,    -1,    -1,    -1,    -1,    -1,   241,
      -1,    -1,   936,    -1,   919,   920,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   947,   948,    -1,    -1,    -1,   260,    -1,
      -1,    -1,    -1,    -1,   266,   267,    -1,   269,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   950,   951,   952,   953,    -1,
      -1,    -1,   976,   977,   959,    -1,   961,    -1,    -1,   964,
     965,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   983,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1017,  1018,  1019,  1020,    -1,    -1,  1023,
      -1,  1010,    -1,    -1,  1028,  1029,    -1,  1016,   340,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1031,  1032,    -1,    -1,    -1,    -1,    -1,   361,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   371,
      -1,    -1,   374,  1052,   376,    -1,    -1,   379,    -1,    -1,
     382,    -1,    -1,   385,   386,    -1,    -1,   389,  1067,  1068,
     392,    -1,    -1,   395,   396,   397,   398,    -1,    -1,   401,
      -1,    -1,    -1,   405,    -1,    -1,   408,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   430,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1137,  1138,    -1,    -1,    -1,  1123,    -1,
      -1,    -1,    -1,    -1,  1129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1145,    -1,    -1,    -1,
    1149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,     5,    -1,     7,    -1,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,   535,   536,   537,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,   614,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   631,
     632,    -1,   634,   635,    -1,   637,    -1,   639,   640,    -1,
     642,   643,   644,   645,   646,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   657,   658,    -1,    -1,    -1,
      -1,    -1,   664,   665,    -1,    -1,   668,   669,   670,    -1,
     672,   673,   674,   675,   676,    -1,    -1,   679,   680,   681,
     682,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   694,   695,   696,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   729,    -1,     4,
      -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,   746,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,   824,   825,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   837,    -1,    -1,    -1,   841,
      -1,    -1,   844,    -1,    -1,   847,    -1,   849,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   861,
      -1,    -1,   864,   865,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   876,    -1,    -1,   879,    -1,    -1,
     882,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     942,   943,   944,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   958,    -1,   960,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   975,    -1,    -1,   978,   979,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   991,
      -1,    -1,    -1,    -1,   996,    -1,    -1,   999,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1021,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1077,    -1,  1079,  1080,     4,
    1082,  1083,    -1,     8,  1086,    10,    11,  1089,  1090,    -1,
    1092,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     4,    -1,    -1,    -1,     8,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
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
      91,    92,    93,    94,    95,     4,    -1,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,     4,     5,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     4,
      -1,    -1,    -1,     8,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95
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
      99,   428,     8,   426,     9,    11,    12,    16,    17,    18,
     108,   435,   437,   438,   108,   416,   439,    17,   108,   122,
      43,    44,   126,   130,   139,   139,   433,   432,   432,   432,
      97,    99,   420,   436,   420,   432,   123,   420,   433,   433,
      28,    31,    59,    67,   140,   141,   142,   143,   144,   144,
     417,    11,   418,   429,   438,   427,   421,   421,   432,   432,
     129,   417,   129,   433,   433,   433,   433,    70,   145,   152,
     152,   136,   424,    98,    96,    98,   127,   131,   435,   435,
     435,   435,   433,   421,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    76,    79,
      98,   153,   154,   168,   173,   179,   226,   230,   231,   240,
     248,   256,   265,   280,   286,   302,   310,   328,   335,    98,
      97,    97,    99,   430,   432,   421,    97,    97,    99,   132,
     420,   420,   420,   420,    12,   108,   150,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   432,   421,   432,   421,   432,   432,     8,
     421,   421,   421,   151,   146,   417,   100,   108,   309,   388,
     389,   417,   417,   417,   108,   339,   389,   417,   417,   417,
     417,   417,   417,   417,   417,   181,   417,    11,   108,   419,
     439,   440,   138,   429,   432,   138,    71,   133,   134,   432,
      97,    99,   147,   281,   418,   303,   390,   432,   266,   232,
     155,   336,   432,   169,   227,   174,   241,   249,   257,   329,
     311,    97,   287,   432,   432,   137,    98,   128,   433,    98,
     421,    97,    99,   282,    97,   388,    97,    99,   267,    97,
      99,   233,    97,    99,   156,    97,    97,    99,   170,    97,
      99,   228,    97,    99,   175,    97,    97,    97,    97,    99,
     330,    97,    99,   312,   421,    97,    99,   288,    98,   432,
     133,   150,   148,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   184,   421,   421,
      98,   420,   149,   283,   305,   269,   234,   157,   338,   171,
     229,   177,   243,   251,   259,   331,   313,    30,    73,    78,
     142,   143,   163,   180,   185,   224,   225,   289,   421,    98,
     142,   143,    27,    42,    51,    55,    73,    81,    98,   142,
     143,   164,   172,   355,   360,   365,   376,    25,   142,   143,
     163,   172,   230,   274,   304,   355,   142,   143,   164,   172,
     225,   231,   240,   248,   256,   268,   272,   273,   274,   280,
     355,   360,   365,    65,    98,   142,   143,   164,   172,   179,
     226,   230,   355,   360,   369,   376,    20,    98,   142,   143,
     158,   163,    37,   142,   143,   337,    98,   142,   143,   164,
     172,    98,   142,   143,   164,   179,   226,   230,    22,   142,
     143,   163,   176,   178,   142,   143,   164,   172,   224,   225,
     242,   245,   355,   360,   365,   376,    52,    53,    58,   142,
     143,   164,   172,   224,   250,   253,   355,   360,   370,   372,
     374,   376,    46,    77,   142,   143,   164,   172,   179,   226,
     230,   258,   355,   360,   370,   372,   374,   376,   379,   382,
      98,   142,   143,   164,   172,   179,   226,   230,    45,    60,
      98,   142,   143,   164,   172,   179,   226,   318,   325,   433,
     433,   433,    98,    75,   182,    21,    68,    98,   142,   143,
     164,   172,   293,   298,   355,   433,   433,   433,   433,   433,
     433,   284,   285,   433,   307,   308,    98,   306,    98,   271,
     270,   433,   238,   237,   239,   235,   236,   433,   433,    98,
     421,   421,   421,   433,    98,    98,   182,   244,   247,   433,
     433,   433,    98,   182,   252,   255,   433,   433,   262,   263,
     264,    98,   260,   261,   332,   333,   334,   432,   432,   314,
     315,   316,   317,   435,    85,    87,    90,   108,   167,   435,
     433,   421,   433,   433,   291,   292,   290,   361,   419,   366,
     435,   165,   435,   421,   421,   417,   421,   421,   421,   421,
     421,   435,   421,   421,   421,   421,   421,   417,    84,    86,
      89,    91,   340,   341,   342,   343,   347,   351,   419,   246,
     421,   421,    17,    94,   108,   373,   412,    18,   108,   371,
     412,   413,    92,    95,   108,   375,   254,   421,   421,   108,
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
      99,   349,    97,    99,   353,   418,   381,   321,   321,   421,
     421,   421,   195,   358,   277,    83,   161,   421,   421,   421,
     386,   179,   226,   230,   320,   327,    61,    69,   178,   187,
     188,   199,   200,   210,   211,   213,   214,   305,   297,    33,
      34,    98,   142,   143,   222,   223,    98,   142,   143,    98,
     142,   143,   164,   172,   230,   355,   433,    98,   346,   350,
     354,   385,   322,   323,   324,    98,    98,   433,   433,    98,
      39,    49,    62,    66,   189,   190,   192,   196,   208,    32,
     202,    24,   216,   300,    27,    51,    98,   142,   143,   225,
     359,   364,   369,   370,   372,   376,   433,   433,   279,   278,
      88,    93,   108,   162,    98,   224,   225,   359,   364,   370,
     372,   376,   379,    98,   224,   225,   376,   379,    98,   182,
     224,   225,   359,   364,   370,   372,   421,   421,   421,   108,
     393,   394,   395,    88,    93,   108,   212,   433,   433,   433,
     433,   182,   421,   433,   421,   433,   215,    98,   433,   433,
     421,   435,   435,   421,   421,   432,   432,   420,   421,   421,
     421,   420,   100,   396,   397,    14,   399,   400,   432,   432,
     420,   108,   191,   412,   193,   435,   197,   435,   387,   435,
     421,   203,   435,   201,   217,   417,   216,   363,   368,   420,
     420,   418,   398,   100,   401,   420,   432,    97,    99,   194,
      97,    99,   198,    97,    99,   209,    97,    99,   204,   202,
      97,    99,   218,   420,   420,   404,   396,   400,   402,   418,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   101,
     403,   404,   195,   195,   195,   205,   219,     5,   434,   397,
      98,    98,    98,    80,    98,   142,   143,   163,   206,    63,
      98,   142,   143,   163,   220,   405,   418,   433,   433,   434,
     434,    16,    17,    18,   108,   207,   414,   415,   108,   221,
     413,   102,   103,   420,   432,   420,   432,   434,    85,   406,
     411,   434,   434,   104,   100,   434,   434,   105,    14,   407,
     408,   434,   409,   100,   408,   418,   434,   410,   418,   434,
     100,   434,   418
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
     186,   187,   187,   187,   187,   187,   187,   188,   188,   188,
     188,   188,   188,   189,   190,   191,   191,   192,   193,   194,
     194,   195,   195,   195,   195,   195,   196,   197,   198,   198,
     200,   199,   201,   201,   202,   203,   204,   204,   205,   205,
     205,   205,   205,   206,   207,   207,   208,   209,   209,   210,
     211,   212,   212,   212,   214,   213,   215,   215,   216,   217,
     218,   218,   219,   219,   219,   219,   219,   220,   221,   221,
     222,   223,   224,   225,   227,   226,   228,   228,   229,   229,
     229,   229,   229,   229,   229,   230,   230,   230,   230,   230,
     230,   230,   232,   231,   233,   233,   234,   235,   234,   234,
     236,   234,   234,   234,   234,   234,   234,   237,   234,   238,
     234,   239,   234,   241,   242,   240,   243,   244,   243,   243,
     245,   246,   243,   243,   247,   243,   243,   243,   243,   243,
     243,   243,   249,   250,   248,   251,   252,   251,   251,   253,
     254,   251,   251,   255,   251,   251,   251,   251,   251,   251,
     251,   251,   257,   258,   256,   259,   260,   259,   259,   261,
     259,   259,   259,   259,   259,   259,   259,   259,   259,   259,
     262,   259,   263,   259,   264,   259,   266,   265,   267,   268,
     267,   269,   270,   269,   269,   269,   269,   269,   269,   269,
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
     424,   425,   425,   426,   426,   427,   427,   428,   428,   429,
     429,   430,   430,   431,   431,   432,   432,   433,   434,   434,
     436,   435,   435,   437,   437,   437,   437,   437,   437,   437,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   439,   440
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
       4,     1,     1,     1,     1,     1,     1,     0,     2,     2,
       2,     3,     4,     0,     4,     2,     1,     5,     1,     1,
       4,     0,     2,     2,     2,     2,     5,     1,     1,     4,
       0,     4,     0,     3,     4,     1,     1,     4,     0,     2,
       2,     2,     2,     4,     2,     1,     4,     1,     4,     4,
       4,     2,     2,     1,     0,     3,     0,     2,     5,     1,
       1,     4,     0,     2,     2,     2,     2,     4,     2,     1,
       4,     4,     4,     4,     0,     5,     1,     4,     0,     2,
       2,     2,     3,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     0,     5,     1,     4,     0,     0,     4,     2,
       0,     4,     2,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     0,     9,     0,     0,     4,     2,
       0,     0,     5,     2,     0,     4,     2,     2,     2,     2,
       2,     2,     0,     0,     9,     0,     0,     4,     2,     0,
       0,     5,     2,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     0,     0,     9,     0,     0,     4,     2,     0,
       4,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     5,     1,     0,
       5,     0,     0,     4,     2,     2,     2,     2,     2,     2,
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
       2,     0,     1,     0,     3,     0,     5,     1,     4,     0,
       3,     2,     5,     1,     1,     0,     2,     2,     0,     1,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
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

    case 269: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

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
                       YYABORT;
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
               }
             }

    break;

  case 28:

    { if (read_all) {
                          if (size_arrays->imp) {
                            trg->imp = calloc(size_arrays->imp, sizeof *trg->imp);
                            if (!trg->imp) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->inc) {
                            trg->inc = calloc(size_arrays->inc, sizeof *trg->inc);
                            if (!trg->inc) {
                              LOGMEM;
                              YYABORT;
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
                                      if (yang_fill_include(module, submodule, (yyvsp[-3].str), rev, size_arrays->inc, unres)) {
                                        YYABORT;
                                      }
                                      size_arrays->inc++;
                                      s = NULL;
                                      trg = (yyval.inc);
                                      config_inherit = ENABLE_INHERIT;
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

    { if (read_all && yang_read_base(module, actual, (yyvsp[0].str), unres)) {
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
                                    if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DER,(struct lys_node *) (yyvsp[-3].v))) {
                                      YYABORT;
                                    }
                                    actual = (yyvsp[-3].v);

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
                                        (yyval.v) = actual;
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

  case 143:

    { /*leafref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 144:

    { /*identityref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->flags |= LYS_TYPE_BASE;
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 145:

    { /*instance_identifier_specification */
                             if (read_all) {
                               ((struct yang_type *)actual)->base = LY_TYPE_INST;
                             }
                           }

    break;

  case 147:

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

  case 149:

    { if (!read_all) {
                                                  size_arrays->node[(yyvsp[-1].uint)].pattern++; /* count of pattern*/
                                                }
                                              }

    break;

  case 152:

    { if (read_all) {
                                                                  actual = (yyvsp[-2].v);
                                                                } else {
                                                                  size_arrays->node[(yyvsp[-3].uint)].uni++; /* count of union*/
                                                                }
                                                              }

    break;

  case 153:

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

  case 154:

    { if (read_all && yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 155:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 156:

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

  case 157:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 158:

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

  case 161:

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

  case 162:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 163:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 164:

    { if (read_all && yang_read_description(trg, actual, s, (yyvsp[-1].str))) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 165:

    { if (read_all && yang_read_reference(trg, actual, s, (yyvsp[-1].str))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 166:

    { actual = (yyvsp[-2].v);
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 167:

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

  case 170:

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

  case 174:

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

  case 175:

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

  case 179:

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

  case 180:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 181:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 182:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 183:

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

  case 184:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 185:

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

  case 186:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 191:

    { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }

    break;

  case 192:

    { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }

    break;

  case 193:

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

  case 194:

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

  case 198:

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

  case 199:

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

  case 203:

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

  case 204:

    { if (read_all) {
                                  if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 205:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 206:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 207:

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

  case 208:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 209:

    { /* convert it to uint32_t */
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

    break;

  case 214:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
                                                         }
                                                       }

    break;

  case 218:

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

  case 219:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 220:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 221:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 222:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 223:

    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }

    break;

  case 224:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 232:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 236:

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

  case 237:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 239:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 240:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 242:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 243:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 244:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 245:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 246:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 247:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 249:

    { if (read_all) {
                                                 actual = (yyvsp[-1].nodes).container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                               }
                                             }

    break;

  case 251:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 253:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 254:

    { if (read_all) {
                                  if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                                    YYABORT;
                                  } else {
                                      if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *) (yyvsp[0].nodes).node.ptr_leaf->parent)) {
                                        YYABORT;
                                      }
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

  case 256:

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

  case 257:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }

    break;

  case 259:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 260:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYABORT;
                       }
                     }

    break;

  case 261:

    { if (read_all) {
                     actual = (yyvsp[-2].nodes).node.ptr_leaf;
                     actual_type = LEAF_KEYWORD;
                     (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                   }
                 }

    break;

  case 262:

    { (yyval.nodes) = (yyvsp[-4].nodes);}

    break;

  case 263:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 264:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 266:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;}
                                    s = NULL;
                                  }

    break;

  case 267:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i), 0)) {
                                                 YYABORT;
                                               }
                                             }
                                           }

    break;

  case 268:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 269:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 270:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 271:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 272:

    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYABORT;}
                                                               data_node = actual;
                                                               s=NULL;
                                                             }
                                                           }

    break;

  case 273:

    { if (read_all) {
                                             if ((yyvsp[0].nodes).node.ptr_leaflist->flags & LYS_CONFIG_R) {
                                               /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                                * ignore oredering MASK - 0x7F
                                                */
                                             (yyvsp[0].nodes).node.ptr_leaflist->flags &= 0x7F;
                                           }
                                           if ((yyvsp[0].nodes).node.ptr_leaflist->max && (yyvsp[0].nodes).node.ptr_leaflist->min > (yyvsp[0].nodes).node.ptr_leaflist->max) {
                                             LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                                             YYABORT;
                                           }
                                           if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                             LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                                             YYABORT;
                                           } else {
                                             if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER,
                                                                      (struct lys_node *) (yyvsp[0].nodes).node.ptr_leaflist->parent)) {
                                               YYABORT;
                                             }
                                           }
                                         }
                                       }

    break;

  case 275:

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

  case 276:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 278:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 279:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYABORT;
                          }
                        }

    break;

  case 280:

    { if (read_all) {
                   actual = (yyvsp[-2].nodes).node.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                 }
               }

    break;

  case 281:

    { (yyval.nodes) = (yyvsp[-4].nodes); }

    break;

  case 282:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 283:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 285:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 286:

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
                                              }
                                            }
                                          }

    break;

  case 287:

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
                                              }
                                            }
                                          }

    break;

  case 288:

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

  case 289:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 290:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 291:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 292:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 293:

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

  case 295:

    { if (read_all) {
                          (yyval.nodes).node.ptr_list = actual;
                          (yyval.nodes).node.flag = 0;
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

  case 296:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 298:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 299:

    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }

    break;

  case 301:

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

  case 302:

    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }

    break;

  case 303:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 304:

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
                                         }
                                       }
                                     }

    break;

  case 305:

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
                                         }
                                       }
                                     }

    break;

  case 306:

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

  case 307:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 308:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 309:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 310:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }

    break;

  case 312:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 314:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 315:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 316:

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

  case 319:

    { if (read_all) {
                              if ((yyvsp[0].nodes).choice.s && ((yyvsp[0].nodes).choice.ptr_choice->flags & LYS_MAND_TRUE)) {
                                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "choice");
                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"default\" statement is forbidden on choices with \"mandatory\".");
                                YYABORT;
                              }
                              /* link default with the case */
                              if ((yyvsp[0].nodes).choice.s) {
                                if (unres_schema_add_str(trg, unres, (yyvsp[0].nodes).choice.ptr_choice, UNRES_CHOICE_DFLT, (yyvsp[0].nodes).choice.s) == -1) {
                                  YYABORT;
                                }
                                free((yyvsp[0].nodes).choice.s);
                              }
                            }
                          }

    break;

  case 321:

    { if (read_all) {
                            (yyval.nodes).choice.ptr_choice = actual;
                            (yyval.nodes).choice.s = NULL;
                            actual_type = CHOICE_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).choice.ptr_choice->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).choice.ptr_choice->features);
                              if (!(yyval.nodes).choice.ptr_choice->features) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).choice.ptr_choice, size_arrays->node[size_arrays->next].flags, config_inherit);
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

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 323:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 324:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).choice.ptr_choice,s, unres, CHOICE_KEYWORD)) {
                                           if ((yyvsp[-1].nodes).choice.s) {
                                             free((yyvsp[-1].nodes).choice.s);
                                           }
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
                                      if ((yyvsp[-1].nodes).choice.s) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).choice.ptr_choice, "default", "choice");
                                        free((yyvsp[-1].nodes).choice.s);
                                        free(s);
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).choice.s = s;
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
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
                                          if (yang_read_description(trg, (yyvsp[-1].nodes).choice.ptr_choice, s, "choice")) {
                                            free((yyvsp[-1].nodes).choice.s);
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 330:

    { if (read_all) {
                                        if (yang_read_reference(trg, (yyvsp[-1].nodes).choice.ptr_choice, s, "choice")) {
                                          free((yyvsp[-1].nodes).choice.s);
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }
                                    }

    break;

  case 331:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
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
                              if (!(yyval.nodes).anyxml->features || !(yyval.nodes).anyxml->must) {
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
                          store_flags((struct lys_node *)(yyval.nodes).uses.ptr_uses, size_arrays->node[size_arrays->next].flags, config_inherit);
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
                                    (yyval.nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
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

  case 452:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

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

                    val = strtoul(yyget_text(scanner), &endptr, 10);
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

                    val = strtoul(yyget_text(scanner), &endptr, 10);
                    if (val > UINT32_MAX || *endptr) {
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

    { if (!strcmp(s, "user")) {
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

  case 640:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 728:

    { if (read_all) {
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
