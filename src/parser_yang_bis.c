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
    USER_KEYWORD = 350,
    ACTION_KEYWORD = 351,
    MODIFIER_KEYWORD = 352,
    ANYDATA_KEYWORD = 353
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
  char ch;
  struct lys_module *inc;
  struct yang_type *type;
  union {
    uint32_t index;
    struct lys_node_container *container;
    struct lys_node_anydata *anydata;
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
#define YYLAST   3251

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  344
/* YYNRULES -- Number of rules.  */
#define YYNRULES  764
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1229

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   353

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
     107,   108,     2,    99,     2,     2,     2,   103,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   102,
       2,   106,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   104,     2,   105,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   100,     2,   101,     2,     2,     2,     2,
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
      95,    96,    97,    98
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   255,   255,   256,   277,   277,   296,   298,   297,   329,
     342,   329,   351,   352,   360,   361,   364,   377,   364,   388,
     389,   395,   395,   404,   406,   408,   431,   432,   435,   435,
     455,   456,   462,   473,   484,   493,   495,   495,   518,   519,
     523,   524,   535,   546,   555,   567,   578,   567,   587,   589,
     590,   591,   592,   597,   603,   605,   607,   609,   611,   619,
     622,   622,   631,   632,   632,   637,   638,   643,   650,   650,
     659,   665,   706,   709,   710,   711,   712,   713,   714,   715,
     721,   722,   723,   725,   725,   744,   745,   749,   750,   757,
     764,   775,   787,   787,   789,   790,   794,   796,   797,   798,
     809,   811,   812,   811,   815,   816,   817,   818,   835,   835,
     844,   845,   850,   871,   884,   890,   895,   901,   903,   903,
     912,   913,   918,   958,   967,   976,   982,   987,   993,   997,
     995,  1012,  1022,  1022,  1029,  1032,  1032,  1050,  1051,  1057,
    1063,  1068,  1074,  1075,  1081,  1086,  1087,  1088,  1095,  1103,
    1108,  1112,  1142,  1143,  1147,  1148,  1149,  1157,  1170,  1176,
    1177,  1195,  1199,  1209,  1210,  1215,  1227,  1232,  1237,  1242,
    1248,  1257,  1270,  1271,  1275,  1276,  1288,  1293,  1298,  1303,
    1309,  1322,  1322,  1341,  1342,  1345,  1357,  1367,  1368,  1373,
    1397,  1406,  1415,  1421,  1426,  1432,  1444,  1445,  1463,  1468,
    1469,  1474,  1476,  1478,  1482,  1486,  1501,  1501,  1520,  1521,
    1523,  1534,  1544,  1545,  1550,  1574,  1583,  1592,  1598,  1603,
    1609,  1621,  1622,  1637,  1639,  1641,  1643,  1645,  1645,  1652,
    1653,  1658,  1680,  1686,  1691,  1696,  1697,  1704,  1705,  1708,
    1709,  1710,  1711,  1712,  1713,  1714,  1715,  1718,  1718,  1726,
    1727,  1732,  1768,  1768,  1770,  1777,  1777,  1785,  1786,  1792,
    1798,  1803,  1808,  1808,  1813,  1813,  1818,  1818,  1826,  1826,
    1833,  1840,  1833,  1858,  1888,  1888,  1890,  1897,  1902,  1897,
    1915,  1916,  1916,  1924,  1927,  1933,  1939,  1945,  1950,  1956,
    1963,  1956,  1998,  2039,  2039,  2041,  2048,  2053,  2048,  2066,
    2083,  2084,  2084,  2092,  2098,  2112,  2126,  2138,  2144,  2149,
    2155,  2162,  2155,  2187,  2232,  2232,  2234,  2241,  2241,  2249,
    2262,  2270,  2276,  2290,  2304,  2316,  2322,  2327,  2332,  2332,
    2340,  2340,  2345,  2345,  2350,  2350,  2358,  2358,  2369,  2371,
    2370,  2388,  2411,  2411,  2413,  2426,  2438,  2446,  2454,  2462,
    2471,  2480,  2480,  2490,  2491,  2494,  2495,  2496,  2497,  2498,
    2499,  2502,  2502,  2510,  2511,  2515,  2537,  2537,  2539,  2546,
    2552,  2557,  2562,  2562,  2569,  2569,  2581,  2581,  2593,  2594,
    2598,  2627,  2627,  2633,  2640,  2640,  2648,  2655,  2662,  2669,
    2674,  2680,  2680,  2696,  2697,  2701,  2738,  2738,  2740,  2747,
    2753,  2758,  2763,  2763,  2771,  2771,  2786,  2786,  2795,  2796,
    2801,  2802,  2805,  2855,  2862,  2886,  2906,  2925,  2944,  2967,
    2990,  2995,  3001,  3010,  3001,  3017,  3018,  3021,  3030,  3021,
    3042,  3063,  3063,  3065,  3072,  3081,  3086,  3091,  3091,  3099,
    3099,  3107,  3107,  3117,  3118,  3121,  3121,  3132,  3132,  3143,
    3144,  3149,  3179,  3186,  3192,  3197,  3202,  3202,  3210,  3210,
    3215,  3215,  3227,  3227,  3240,  3254,  3240,  3261,  3292,  3292,
    3300,  3300,  3308,  3308,  3313,  3313,  3323,  3337,  3323,  3344,
    3344,  3354,  3358,  3363,  3403,  3403,  3411,  3418,  3424,  3429,
    3434,  3434,  3442,  3442,  3447,  3447,  3454,  3463,  3454,  3477,
    3497,  3505,  3513,  3523,  3524,  3526,  3531,  3533,  3534,  3535,
    3538,  3540,  3540,  3546,  3547,  3551,  3578,  3586,  3594,  3610,
    3620,  3627,  3634,  3645,  3657,  3657,  3663,  3664,  3684,  3711,
    3719,  3730,  3740,  3751,  3751,  3757,  3758,  3762,  3779,  3789,
    3797,  3807,  3814,  3821,  3832,  3844,  3844,  3847,  3848,  3852,
    3853,  3858,  3864,  3866,  3867,  3866,  3870,  3871,  3872,  3887,
    3889,  3890,  3889,  3893,  3894,  3895,  3910,  3912,  3914,  3915,
    3936,  3938,  3939,  3940,  3961,  3963,  3964,  3965,  3977,  3977,
    3985,  3986,  3991,  3993,  3994,  3996,  3997,  3999,  4001,  4001,
    4010,  4013,  4013,  4024,  4027,  4037,  4058,  4060,  4061,  4064,
    4064,  4083,  4083,  4092,  4092,  4101,  4104,  4106,  4108,  4109,
    4111,  4113,  4115,  4116,  4118,  4120,  4121,  4123,  4124,  4126,
    4128,  4131,  4135,  4137,  4138,  4140,  4141,  4143,  4145,  4156,
    4157,  4160,  4161,  4173,  4174,  4176,  4177,  4179,  4180,  4186,
    4187,  4190,  4191,  4192,  4218,  4219,  4222,  4223,  4224,  4227,
    4227,  4235,  4237,  4238,  4240,  4241,  4242,  4244,  4245,  4247,
    4248,  4250,  4251,  4253,  4254,  4256,  4257,  4260,  4261,  4264,
    4266,  4267,  4270,  4270,  4279,  4281,  4282,  4283,  4284,  4285,
    4286,  4287,  4289,  4290,  4291,  4292,  4293,  4294,  4295,  4296,
    4297,  4298,  4299,  4300,  4301,  4302,  4303,  4304,  4305,  4306,
    4307,  4308,  4309,  4310,  4311,  4312,  4313,  4314,  4315,  4316,
    4317,  4318,  4319,  4320,  4321,  4322,  4323,  4324,  4325,  4326,
    4327,  4328,  4329,  4330,  4331,  4332,  4333,  4334,  4335,  4336,
    4337,  4338,  4339,  4340,  4341,  4342,  4343,  4344,  4345,  4346,
    4347,  4348,  4349,  4350,  4351,  4352,  4353,  4354,  4355,  4356,
    4357,  4358,  4359,  4360,  4361,  4362,  4363,  4364,  4365,  4366,
    4367,  4368,  4369,  4372,  4381
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
  "USER_KEYWORD", "ACTION_KEYWORD", "MODIFIER_KEYWORD", "ANYDATA_KEYWORD",
  "'+'", "'{'", "'}'", "';'", "'/'", "'['", "']'", "'='", "'('", "')'",
  "$accept", "start", "string_1", "$@1", "string_2", "$@2", "module_stmt",
  "$@3", "$@4", "module_header_stmts", "submodule_stmt", "$@5", "$@6",
  "submodule_header_stmts", "$@7", "yang_version_stmt", "namespace_stmt",
  "linkage_stmts", "import_stmt", "$@8", "import_opt_stmt",
  "tmp_identifier_arg_str", "include_stmt", "$@9", "include_end",
  "include_opt_stmt", "revision_date_stmt", "belongs_to_stmt", "$@10",
  "$@11", "prefix_stmt", "meta_stmts", "organization_stmt", "contact_stmt",
  "description_stmt", "reference_stmt", "revision_stmts", "revision_stmt",
  "$@12", "revision_end", "$@13", "revision_opt_stmt", "date_arg_str",
  "$@14", "body_stmts", "body_stmt", "extension_stmt", "$@15",
  "extension_end", "extension_opt_stmt", "argument_stmt", "$@16",
  "argument_end", "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@17", "$@18", "status_arg_str", "feature_stmt",
  "$@19", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@20", "identity_end", "identity_opt_stmt",
  "base_stmt", "typedef_stmt", "$@21", "typedef_arg_str", "type_stmt",
  "$@22", "type_opt_stmt", "$@23", "type_end", "type_stmtsep",
  "type_body_stmts", "decimal_string_restrictions", "union_spec",
  "fraction_digits_stmt", "fraction_digits_arg_str", "length_stmt",
  "length_arg_str", "length_end", "message_opt_stmt", "pattern_stmt",
  "pattern_arg_str", "pattern_end", "pattern_opt_stmt", "modifier_stmt",
  "enum_specification", "$@24", "enum_stmts", "enum_stmt", "enum_arg_str",
  "enum_end", "enum_opt_stmt", "value_stmt", "integer_value_arg_str",
  "range_stmt", "range_end", "path_stmt", "require_instance_stmt",
  "require_instance_arg_str", "bits_specification", "$@25", "bit_stmts",
  "bit_stmt", "bit_arg_str", "bit_end", "bit_opt_stmt", "position_stmt",
  "position_value_arg_str", "error_message_stmt", "error_app_tag_stmt",
  "units_stmt", "default_stmt", "grouping_stmt", "$@26", "grouping_end",
  "grouping_opt_stmt", "data_def_stmt", "container_stmt", "$@27",
  "container_end", "container_opt_stmt", "$@28", "$@29", "$@30", "$@31",
  "$@32", "$@33", "leaf_stmt", "$@34", "$@35", "leaf_opt_stmt", "$@36",
  "$@37", "$@38", "$@39", "leaf_list_stmt", "$@40", "$@41",
  "leaf_list_opt_stmt", "$@42", "$@43", "$@44", "$@45", "list_stmt",
  "$@46", "$@47", "list_opt_stmt", "$@48", "$@49", "$@50", "$@51", "$@52",
  "$@53", "choice_stmt", "$@54", "choice_end", "$@55", "choice_opt_stmt",
  "$@56", "$@57", "short_case_case_stmt", "short_case_stmt", "case_stmt",
  "$@58", "case_end", "case_opt_stmt", "$@59", "$@60", "anyxml_stmt",
  "$@61", "anydata_stmt", "$@62", "anyxml_end", "anyxml_opt_stmt", "$@63",
  "$@64", "uses_stmt", "$@65", "uses_end", "uses_opt_stmt", "$@66", "$@67",
  "$@68", "refine_stmt", "$@69", "refine_end", "refine_arg_str",
  "refine_body_opt_stmts", "uses_augment_stmt", "$@70", "$@71",
  "uses_augment_arg_str", "augment_stmt", "$@72", "$@73",
  "augment_opt_stmt", "$@74", "$@75", "$@76", "$@77", "augment_arg_str",
  "action_stmt", "$@78", "rpc_stmt", "$@79", "rpc_end", "rpc_opt_stmt",
  "$@80", "$@81", "$@82", "$@83", "input_stmt", "$@84", "$@85",
  "input_output_opt_stmt", "$@86", "$@87", "$@88", "$@89", "output_stmt",
  "$@90", "$@91", "notification_stmt", "$@92", "notification_end",
  "notification_opt_stmt", "$@93", "$@94", "$@95", "$@96",
  "deviation_stmt", "$@97", "$@98", "deviation_opt_stmt",
  "deviation_arg_str", "deviate_body_stmt", "deviate_stmts",
  "deviate_not_supported_stmt", "deviate_add_stmt", "$@99",
  "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt",
  "$@100", "deviate_delete_end", "deviate_delete_opt_stmt",
  "deviate_replace_stmt", "$@101", "deviate_replace_end",
  "deviate_replace_opt_stmt", "when_stmt", "$@102", "when_end",
  "when_opt_stmt", "config_stmt", "config_read_stmt", "$@103", "$@104",
  "config_arg_str", "mandatory_stmt", "mandatory_read_stmt", "$@105",
  "$@106", "mandatory_arg_str", "presence_stmt", "min_elements_stmt",
  "min_value_arg_str", "max_elements_stmt", "max_value_arg_str",
  "ordered_by_stmt", "ordered_by_arg_str", "must_stmt", "$@107",
  "must_end", "unique_stmt", "unique_arg_str", "unique_arg", "key_stmt",
  "key_arg_str", "$@108", "key_opt", "$@109", "range_arg_str",
  "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@110",
  "path_arg_str", "$@111", "$@112", "absolute_path", "absolute_paths",
  "absolute_path_opt", "relative_path", "relative_path_part1",
  "relative_path_part1_opt", "descendant_path", "descendant_path_opt",
  "path_predicate", "path_equality_expr", "path_key_expr",
  "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@113", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@114", "strings", "identifier", "identifiers",
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
     345,   346,   347,   348,   349,   350,   351,   352,   353,    43,
     123,   125,    59,    47,    91,    93,    61,    40,    41
};
# endif

#define YYPACT_NINF -946

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-946)))

#define YYTABLE_NINF -618

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -946,    21,  -946,  -946,   247,  -946,  -946,  -946,   124,   124,
    -946,  -946,  3058,  3058,   124,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,   -76,   124,   -71,   124,  -946,  -946,   -37,
     115,   115,  -946,  -946,   392,  -946,  -946,   -10,   306,   124,
     124,   124,   124,  -946,  -946,  -946,  -946,  -946,    60,  -946,
    -946,   112,  2025,  -946,  2393,  3058,  2393,   118,   118,   124,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,   165,  -946,  -946,   165,
    -946,   165,   124,   124,  -946,  -946,   421,   421,  3058,   124,
    2488,   124,  -946,  -946,  -946,  -946,  -946,   124,  -946,  3058,
    3058,   124,   124,   124,   124,  -946,  -946,  -946,  -946,    43,
      43,  -946,  -946,   124,    69,  -946,   106,    37,   115,   124,
    -946,  -946,  -946,  2393,  2393,  2393,  2393,   124,  -946,   363,
     577,   116,   269,  -946,  -946,  -946,   138,   272,   165,   165,
     165,   165,   141,   115,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,   323,   115,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  3058,
       3,  3058,  3058,  3058,     3,  3058,  3058,  3058,  3058,  3058,
    3058,  3058,  3058,  3058,  2583,  3058,   124,   115,   124,    59,
    2488,   124,  -946,   115,   115,   115,  -946,   281,  -946,  3153,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,   142,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,   147,   124,   219,
     232,   124,  -946,  -946,  -946,   305,  -946,   151,   155,   124,
     365,   375,   385,   164,   124,   425,   428,   434,   185,   195,
     200,   452,   455,  -946,   461,   124,   124,   305,   207,  -946,
     124,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
     115,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,   115,  -946,  -946,  -946,  -946,  -946,
     124,   141,   115,  -946,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   133,   115,
     115,   165,    54,   669,  1379,  1074,   262,   140,   287,   333,
    1322,   108,  1108,  1545,  1670,   871,   634,   124,   124,   124,
    -946,  -946,  -946,   215,   244,  -946,  -946,   284,  -946,  -946,
    -946,  -946,   124,   124,   124,   124,   124,   124,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,   124,   124,  -946,
    -946,  -946,  -946,  -946,  -946,   222,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,   235,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,   124,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
     124,  -946,  -946,  -946,  -946,  -946,   124,  -946,  -946,   238,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,   124,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,   241,   244,  -946,  -946,
    -946,  -946,   124,   124,   124,  -946,  -946,  -946,  -946,  -946,
    -946,   252,   244,  -946,  -946,  -946,  -946,  -946,  -946,   124,
     124,  -946,  -946,  -946,  -946,  -946,  -946,  -946,   255,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  2393,   111,
    2393,  -946,   124,  -946,   124,   124,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  2393,  -946,  2393,  -946,  2393,
    -946,  -946,  3058,  3058,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  2393,  -946,  -946,  -946,  -946,  -946,  -946,  3058,
     113,  -946,   115,   115,   115,   115,  2583,  -946,  -946,  -946,
    -946,    42,    61,    12,  -946,  -946,  -946,  -946,  2678,  2678,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,   124,   124,  -946,  -946,  -946,  -946,   165,  -946,  -946,
    -946,  -946,   165,   165,  2583,   115,  2678,  2678,  -946,  -946,
    -946,    49,   165,    52,  -946,   111,  -946,   115,   115,  -946,
    -946,   115,   115,   115,   115,   115,   115,   165,   115,   115,
     115,   115,   115,   115,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,   165,  -946,   115,   115,  -946,
    -946,  -946,   165,  -946,  -946,  -946,   165,  -946,  -946,  -946,
    -946,  -946,   165,  -946,   115,   115,   165,  -946,  -946,   165,
    -946,    25,  -946,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   277,   282,   115,   115,   115,   115,  -946,
     124,   124,   124,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,   115,   115,   115,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,   499,  -946,   500,   505,   455,  -946,
     520,   124,   124,    25,   124,   124,  -946,   115,   124,  -946,
     124,  -946,   124,   124,   124,  -946,   115,  -946,    25,  -946,
    -946,  -946,  3153,  -946,  -946,  -946,   533,   294,   124,   544,
     124,   124,   124,   165,   124,   124,   165,  -946,  -946,  -946,
     165,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,   552,   559,  -946,   562,  -946,  -946,  3153,    25,   155,
     115,   115,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,   115,  -946,   115,   115,   177,   115,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,   383,   383,
     237,   115,   115,   115,   206,   301,   590,   124,   314,   115,
     115,   115,    25,  -946,  -946,  -946,   336,  -946,   340,   124,
     124,  -946,   356,   611,  -946,   431,  -946,  -946,  -946,   436,
    1379,  1079,   124,   124,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,    53,
    -946,   748,   114,   295,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,   179,    55,  -946,   124,   124,   124,   124,   244,  -946,
    -946,  -946,  -946,   124,  -946,   124,  -946,   381,   124,   124,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    2393,  2393,  -946,  -946,  -946,  -946,  -946,   165,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
     115,   115,   115,   115,  -946,   165,   373,   472,  -946,  -946,
    -946,   165,    95,  2393,  2393,  2393,  -946,   115,  2393,   115,
    3058,   436,  -946,    49,    52,   115,   165,   165,   115,   115,
     124,   124,  -946,   115,   115,   115,  -946,  3153,  -946,  -946,
     388,  -946,  -946,   124,   124,  -946,  -946,   165,  -946,   580,
    -946,   581,  -946,   593,  -946,   115,   612,  -946,   431,   625,
    -946,  -946,   165,   165,  -946,  -946,  -946,   373,  -946,  2773,
    -946,   124,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
     362,  -946,  -946,  -946,    -5,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   509,  -946,    48,   391,   152,   412,
     313,   377,  -946,  3153,  -946,  -946,   124,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,   124,  -946,  -946,  -946,  -946,  -946,
    -946,   124,  -946,  -946,  -946,  -946,  -946,  -946,   509,   509,
    2393,   239,    61,   414,   423,   165,  -946,  -946,  -946,  -946,
     165,  -946,  -946,  -946,   165,  -946,  -946,   509,  -946,  -946,
     124,  -946,   124,   435,   509,  -946,   509,   449,   459,   509,
     509,   430,   551,  -946,   509,  -946,  -946,   473,  2868,   509,
    -946,  -946,  -946,  2963,  -946,   476,   509,  3153,  -946
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     667,     0,     2,     3,     0,     1,   665,   666,     0,     0,
     668,   667,     0,     0,   669,   683,     4,   682,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   695,
     696,   697,   698,   699,   700,   701,   702,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   719,   720,   721,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   732,   733,   734,   735,
     736,   737,   738,   739,   740,   741,   742,   743,   744,   745,
     746,   747,   748,   749,   750,   751,   752,   753,   754,   755,
     756,   757,   758,   759,   760,   761,   762,   638,     9,   763,
     667,    16,   667,     0,   637,     0,     6,   646,   646,     5,
      12,    19,   667,   649,    10,   648,   647,    17,     0,   652,
       0,     0,     0,    25,    13,    14,    15,    25,     0,    20,
       7,     0,   654,   653,     0,     0,     0,    49,    49,     0,
      22,   667,   667,   659,   650,   667,   675,   678,   676,   680,
     681,   679,   651,   655,   677,   674,     0,   672,   635,     0,
     667,     0,     0,     0,    26,    27,    58,    58,     0,     8,
     661,   657,   646,   646,    24,   667,    48,   636,    23,     0,
       0,     0,     0,     0,     0,    50,    51,    52,    53,    71,
      71,    45,   640,   652,     0,   639,   656,     0,   644,   673,
      28,    35,    36,     0,     0,     0,     0,     0,   646,     0,
       0,     0,     0,   660,   667,   646,     0,     0,     0,     0,
       0,     0,     0,    59,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   667,   646,    73,    74,    75,    76,    77,    78,   239,
     240,   241,   242,   243,   244,   245,   246,    79,    80,    81,
      82,   667,   646,   667,   667,   662,     0,   645,   646,   646,
      38,   646,    55,    56,    54,    57,    68,    70,    60,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    72,    18,     0,
     661,   663,   667,    30,    40,    37,   667,     0,   374,     0,
     444,   427,   597,   667,   336,   247,    83,   504,   496,   667,
     108,   227,   118,   270,   289,   310,   479,   447,     0,   131,
     764,   643,   391,   667,   667,   376,    46,     0,   658,     0,
       0,    69,   646,    62,    61,     0,   595,     0,   596,   443,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,   646,     0,   641,   642,     0,     0,   667,
       0,   646,    34,    31,    32,    33,    39,    43,    41,    42,
      63,   646,   378,   375,   646,   598,   646,   338,   337,   646,
     249,   248,   646,    85,    84,   646,   646,   110,   109,   646,
     229,   228,   646,   120,   119,   646,   646,   646,   646,   481,
     480,   646,   449,   448,   134,   646,   393,   392,   377,   646,
     664,     0,    29,    65,   380,   430,   341,   251,    87,   499,
     112,   231,   122,   273,   292,   313,   483,   451,   129,   395,
      47,     0,     0,     0,   428,   339,     0,     0,   497,     0,
       0,     0,   271,   290,   311,     0,     0,     0,     0,     0,
     140,   141,   139,     0,     0,   137,   138,     0,    44,    64,
      66,    67,     0,     0,     0,     0,     0,     0,   379,   389,
     390,   388,   383,   381,   386,   387,   384,     0,     0,   435,
     436,   434,   433,   437,   441,     0,   439,   431,   349,   350,
     348,   344,   345,   355,   356,   357,   358,     0,   351,   353,
     354,   359,   360,   342,   346,   347,     0,   250,   260,   261,
     259,   254,   266,   262,   268,   264,   252,   258,   257,   255,
       0,    86,    90,    91,    88,    89,     0,   500,   501,     0,
     111,   115,   116,   114,   113,   230,   233,   234,   232,   646,
     646,   646,   646,     0,   121,   126,   127,   125,   124,   123,
     287,   288,   286,   276,   280,   283,     0,     0,   274,   284,
     285,   281,     0,     0,     0,   308,   309,   307,   295,   300,
     299,     0,     0,   293,   303,   304,   305,   306,   301,     0,
       0,   326,   327,   325,   316,   328,   330,   334,     0,   332,
     314,   321,   322,   323,   324,   317,   320,   319,   482,   488,
     489,   487,   486,   490,   492,   494,   484,   667,   667,   450,
     454,   455,   453,   452,   456,   458,   460,   462,     0,     0,
       0,   130,     0,   646,     0,     0,   394,   400,   401,   399,
     398,   402,   404,   396,   553,     0,   560,     0,   101,     0,
     646,   646,     0,     0,   646,   646,   429,   646,   646,   340,
     646,   646,     0,   646,   646,   646,   646,   646,   646,     0,
       0,   498,   236,   235,   237,   238,     0,   272,   278,   646,
     646,     0,     0,     0,   291,   297,   646,   646,     0,     0,
     646,   646,   646,   312,   646,   646,   646,   646,   646,   646,
     646,   464,   476,   646,   646,   646,   646,     0,   667,   667,
     667,   107,     0,     0,     0,   136,     0,     0,   646,   646,
     646,     0,     0,     0,   578,     0,   545,   382,   385,   361,
     445,   438,   442,   440,   432,   352,   343,     0,   267,   263,
     269,   265,   253,   256,    92,   667,   667,   667,   667,   502,
     667,   505,   507,   509,   508,     0,   646,   275,   282,   628,
     667,   573,     0,   667,   629,   569,     0,   630,   667,   667,
     667,   577,     0,   646,   294,   302,     0,   587,   588,     0,
     582,     0,   599,   329,   331,   335,   333,   315,   318,   491,
     493,   495,   485,     0,     0,   457,   459,   461,   463,   226,
     104,   106,   105,   100,   225,   132,   426,   422,   667,   411,
     406,   667,   403,   405,   397,   667,   667,   558,   554,   117,
     667,   667,   565,   561,     0,   102,     0,     0,     0,   566,
       0,   511,   524,     0,   533,   506,   128,   279,   571,   570,
     572,   567,   568,   576,   575,   574,   298,   590,     0,   584,
     583,   586,     0,   597,   646,   646,     0,     0,   425,     0,
     410,   557,   556,     0,   564,   563,     0,   646,   580,   579,
       0,   646,   547,   546,   646,   363,   362,   446,   646,    94,
     646,     0,     0,   510,     0,   589,   593,     0,     0,   600,
     467,   467,   646,   142,   133,   646,   646,   408,   407,   555,
     562,   165,   103,   549,   365,     0,    93,   646,   513,   512,
     646,   526,   525,   646,   535,   534,   591,   585,   465,   477,
     151,   144,   430,   412,     0,     0,     0,     0,     0,   515,
     528,   537,     0,   470,   472,   474,     0,   468,     0,     0,
       0,   148,     0,   145,   146,     0,   147,   149,   150,     0,
     423,     0,     0,     0,   581,   168,   169,   166,   167,   548,
     550,   551,   364,   370,   371,   369,   368,   372,   366,     0,
      95,     0,     0,     0,   592,   646,   646,   646,   466,   646,
     478,   601,     0,   143,     0,     0,     0,     0,     0,   154,
     152,   153,   646,     0,   646,     0,   208,     0,     0,     0,
     409,   420,   421,   415,   416,   417,   414,   418,   419,   646,
       0,     0,   646,   646,   667,   667,    99,     0,   514,   516,
     519,   520,   521,   522,   523,   646,   518,   527,   529,   532,
     646,   531,   536,   646,   539,   540,   541,   542,   543,   544,
     471,   473,   475,   469,   605,     0,     0,     0,   667,   667,
     205,     0,     0,     0,     0,     0,   646,   155,     0,   183,
       0,   207,   424,     0,     0,   413,     0,     0,   373,   367,
      98,    97,    96,   517,   530,   538,   201,     0,   608,   602,
       0,   604,   612,   204,   203,   202,   160,     0,   667,     0,
     162,     0,   171,     0,   594,   156,     0,   186,   182,     0,
     211,   209,     0,     0,   224,   223,   617,   607,   611,     0,
     158,   159,   646,   163,   646,   646,   172,   646,   646,   199,
     198,   646,   187,   185,   646,   646,   212,   646,   552,   559,
     606,   609,   613,   610,   615,   165,   161,   174,   170,   165,
     189,   184,   214,   210,   670,   614,     0,     0,     0,     0,
       0,     0,   671,     0,   616,   164,     0,   173,   178,   179,
     175,   176,   177,   200,     0,   188,   193,   194,   192,   190,
     191,     0,   213,   218,   219,   217,   215,   216,   670,   670,
       0,     0,     0,     0,     0,     0,   633,   634,   631,   197,
       0,   667,   632,   222,     0,   667,   618,   670,   180,   195,
     196,   220,   221,     0,   670,   619,   670,     0,     0,   670,
     670,     0,     0,   627,   670,   620,   623,     0,     0,   670,
     624,   625,   622,   670,   621,     0,   670,     0,   626
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -946,  -946,   583,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,   458,  -946,   453,  -946,  -946,
    -946,   403,  -946,  -946,  -946,  -946,   250,  -946,  -946,  -946,
    -283,   446,  -946,  -946,    51,   867,   420,  -946,  -946,  -946,
    -946,  -946,   170,  -946,   418,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -435,  -359,  -946,  -946,  -121,
    -946,  -946,  -946,  -946,  -418,  -946,  -946,  -946,  -946,  -309,
    -345,  -946,  -946,  -541,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -945,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -484,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -438,  -946,  -946,  -946,  -946,  -946,  -522,  -519,  -413,  -430,
    -322,  -946,  -946,  -946,  -373,   191,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,   194,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,   198,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,   202,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
     203,  -946,  -946,  -946,  -946,  -946,   210,  -946,   221,  -946,
     300,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -271,  -946,  -946,  -946,  -946,  -946,  -366,
    -946,  -946,  -946,  -156,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -207,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,
    -946,  -946,  -946,  -946,  -946,  -946,  -946,  -946,  -378,  -946,
    -946,  -946,  -576,    70,  -946,  -946,  -364,  -281,  -394,  -946,
    -946,  -361,  -243,  -440,  -946,  -407,  -946,   259,  -946,  -400,
    -946,  -946,  -462,  -946,  -179,  -946,  -946,  -946,  -211,  -946,
    -946,  -342,   438,  -127,  -698,  -946,  -946,  -946,  -946,  -377,
    -415,  -946,  -946,  -372,  -946,  -946,  -946,  -396,  -946,  -946,
    -946,  -479,  -946,  -946,  -946,  -673,  -442,  -946,  -946,  -946,
     -11,  -153,  -669,   687,  1271,  -946,  -946,   548,  -946,  -946,
    -946,  -946,   443,  -946,    -4,   276,   584,   283,   223,  -946,
     613,    40,  -126,  -946
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,   102,   109,   141,     2,   103,   123,   114,
       3,   105,   127,   117,   128,   124,   125,   137,   164,   216,
     339,   200,   165,   217,   271,   340,   372,   140,   211,   368,
     126,   166,   185,   186,   955,   956,   189,   208,   307,   344,
     423,   442,   278,   306,   209,   242,   243,   352,   394,   447,
     534,   830,   880,   928,  1017,   491,   481,   725,   870,   712,
     244,   355,   398,   449,   492,   245,   357,   404,   451,   559,
     246,   463,   328,   633,   856,   438,   464,   894,   920,   942,
     943,   988,   989,  1087,   990,  1089,  1114,   924,   991,  1091,
    1117,  1148,  1160,   944,   945,  1098,   994,  1096,  1123,  1150,
    1170,  1190,   992,  1120,   946,   947,  1051,   948,   949,  1061,
     996,  1099,  1127,  1151,  1177,  1194,   957,   958,   465,   466,
     247,   356,   401,   450,   248,   249,   351,   391,   446,   667,
     668,   664,   666,   663,   665,   250,   358,   566,   452,   679,
     567,   756,   680,   251,   359,   581,   453,   686,   582,   773,
     687,   252,   360,   598,   454,   695,   696,   690,   691,   694,
     692,   253,   350,   388,   507,   445,   661,   660,   508,   509,
     494,   827,   876,   926,  1013,  1012,   254,   345,   255,   367,
     383,   443,   650,   651,   256,   364,   417,   467,   720,   718,
     719,   641,   859,   898,   810,   951,   642,   857,   997,   807,
     257,   347,   495,   444,   658,   654,   657,   655,   311,   496,
     828,   258,   362,   413,   456,   703,   704,   705,   706,   626,
     793,   936,   918,   979,   975,   976,   977,   627,   794,   938,
     259,   361,   410,   455,   700,   697,   698,   699,   260,   353,
     539,   448,   318,   749,   750,   751,   752,   881,   909,   971,
     753,   882,   912,   972,   754,   884,   915,   973,   497,   826,
     873,   925,  1004,   484,   721,   863,   818,  1005,   485,   723,
     866,   823,   528,   585,   766,   586,   762,   587,   772,   937,
     824,   869,   606,   780,   850,   607,   777,   848,   885,   932,
    1093,   312,   313,   348,   781,   853,  1045,  1046,  1047,  1078,
    1079,  1107,  1081,  1082,  1109,  1133,  1145,  1130,  1178,  1205,
    1215,  1216,  1218,  1223,  1206,   767,   768,  1191,  1192,   159,
     201,   782,   332,   851,   110,   115,   119,   131,   132,   152,
     196,   144,   194,   265,   116,     4,   133,  1153,   156,   175,
     157,    99,   100,   334
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    98,   101,   462,    11,    11,   385,   755,   763,   160,
      10,    16,   535,   -21,   602,   502,   336,   193,   808,   811,
      16,     5,   565,   580,   107,   482,   678,   501,   521,   108,
       6,   544,     7,   558,   563,   578,   594,   612,   623,   564,
     579,   685,     6,   486,     7,   805,   529,   603,   113,   640,
      16,   515,   571,   588,   605,   616,   373,    16,   570,   759,
      16,    16,   112,    16,     6,   483,     7,   513,   526,    16,
     113,   493,   122,   524,   568,   583,   600,   551,   759,   764,
     525,   597,   615,   139,   552,   182,   500,   520,   599,   643,
     543,   548,   557,   562,   577,   593,   611,   622,  -617,  -617,
      10,   522,    10,    16,   769,   549,   309,   770,   639,   595,
     613,   624,   759,   207,    10,    11,    11,    11,    11,    16,
       6,   184,     7,   121,   523,   172,   113,   173,   550,     6,
     553,     7,   596,   614,   625,    11,   760,   815,   215,   182,
     820,  1014,   816,  1048,   457,   821,  1015,   193,  1049,    16,
     473,  1077,  1144,   276,   888,   469,   346,   191,    11,    11,
     530,   162,   163,   457,   182,    10,    10,    10,   333,   475,
     213,   182,   154,    10,   154,   184,   154,    11,    11,    11,
      11,   476,     6,   182,     7,   952,   953,    16,   113,    11,
    1147,   590,   459,  -603,  1149,    10,   708,   745,   709,   746,
     184,   710,   747,    11,   748,   214,   458,   184,  -135,   554,
     195,   459,   142,   458,   143,  1027,   262,   187,   187,   184,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,   182,   268,   952,
     953,   531,   363,   154,   154,   154,   154,    16,   369,  1156,
     182,   384,     6,  1157,     7,  1186,  1187,  1188,   309,   553,
     927,  -206,    10,   182,   395,   172,   224,   173,   308,  -181,
     314,   315,   316,   184,   320,   321,   322,   323,   324,   325,
     326,   327,   329,   121,   335,   405,   184,    14,   226,   472,
     370,   227,    10,   182,    10,   406,    10,    10,   939,   184,
     407,     8,   231,   370,   473,   634,   940,   954,   419,   233,
     234,     6,   235,     7,   130,   182,   631,   475,   182,   632,
     371,     9,   998,   656,   536,   457,   473,   516,     6,   184,
       7,   302,   182,   376,    10,   476,   659,    10,   238,   671,
     195,   239,   677,   477,   182,    10,   999,   572,   573,   195,
      10,   184,   635,   684,   184,   473,   693,   476,   488,   161,
     240,    10,    10,   517,   182,   477,    11,   224,   184,   263,
     632,   264,   269,   459,   270,   473,   104,   854,   106,  1088,
     184,   342,   855,   343,   225,   636,   458,   224,   118,   226,
     374,   378,   227,  1164,   895,  1021,  1032,  1036,   228,   229,
     184,   230,   959,   231,   232,   381,   476,   382,   182,   226,
     233,   234,   227,   235,  1165,   970,    10,   169,   170,   473,
     236,   171,   182,   231,   952,   953,   218,   219,   220,   221,
     233,   234,  1033,   235,   540,   237,   177,   978,   475,   238,
    1171,   980,   239,   182,   184,   952,   953,  1056,   120,   181,
     458,   199,   182,    11,    11,    11,   121,   983,   184,   238,
     995,   240,   239,   993,   241,   386,  1144,   387,    11,    11,
      11,    11,    11,    11,   122,   389,  1077,   390,  1172,   184,
     183,   240,  1062,    11,    11,   392,  1080,   393,   184,   460,
     266,  1108,  1155,   470,   479,   489,   498,   518,   532,   537,
     541,   546,   555,   560,   575,   591,   609,   620,   966,  1026,
    1031,  1007,    11,  1163,  1152,   514,   527,   296,   637,  1196,
    1204,  1003,   569,   584,   601,   396,    11,   397,   399,  1197,
     400,  1023,    11,  1038,   402,   778,   403,   298,  1213,   300,
     301,  1020,  1029,  1035,  1008,   935,   935,   385,   968,    11,
     333,  1009,   408,   967,   409,   411,  1209,   412,  1019,  1028,
    1034,   415,  1210,   416,  1024,  1214,  1039,   965,    11,    11,
      11,  1025,  1030,   933,   933,   129,  1219,   493,   338,  1226,
     138,   224,   341,   202,   167,    11,    11,   190,   333,   349,
     377,   441,    12,    13,   224,   354,   934,   934,   225,   867,
     871,   868,   872,   226,   825,   874,   227,   875,   210,   365,
     366,   941,   228,   229,  1124,   230,   226,   231,   232,   227,
     878,   182,   879,  1101,   233,   234,  1161,   235,    11,  1162,
      11,    11,   473,   892,   236,   893,   503,   233,   234,   504,
     235,   729,   730,   505,   896,   420,   897,   506,   510,   237,
     984,   950,   907,   238,   908,   511,   239,   184,   744,   910,
     985,   911,   913,   476,   914,   182,   512,   418,   154,   239,
     154,   477,   877,   986,   231,   240,   473,   987,   261,   617,
    1112,  1115,  1113,  1116,   919,   154,  -157,   154,   240,   154,
    1022,   962,  1037,  1118,   618,  1119,   472,    10,    10,  1102,
     182,   184,   154,  1103,   134,   135,   136,   476,  1006,   917,
     238,   473,  1121,   604,  1122,  1168,  1175,   155,   158,   155,
     474,   974,   319,   168,   475,  1125,   889,  1126,   195,   195,
    1131,  1154,  1169,  1176,   916,   619,   184,  1132,  1146,  1220,
    1195,   212,   476,   337,     0,   153,   179,   180,     0,     0,
     477,     0,     0,     0,     0,     0,   195,   195,     0,     0,
       0,     0,     0,     0,     0,   203,   204,   205,   206,     0,
     478,     0,     0,     0,     0,   998,     0,    11,   457,     0,
       0,     0,     0,     0,     0,     0,   155,   155,   155,   155,
       0,   222,     0,     0,     0,     0,    10,    10,    10,   999,
     572,   573,     0,   475,     0,   277,     0,     0,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   590,   459,    10,    10,    10,
      10,    10,     0,     0,    10,     0,    10,     0,    10,    10,
      10,     0,     0,   174,    11,     0,   176,     0,   178,  1018,
       0,   707,     0,   713,    10,     0,    10,    10,    10,     0,
      10,    10,     0,   310,     0,     0,     0,   317,   722,     0,
     724,     0,   726,     0,     0,   224,     0,   331,     0,     0,
       0,     0,     0,     0,    11,   737,     0,     0,     0,     0,
       0,     0,   195,   701,   702,     0,     0,   226,     0,     0,
     227,     0,   182,     0,     0,   272,   273,   274,   275,     0,
       0,   231,     0,   473,     0,     0,     0,     0,   233,   234,
       0,   235,     0,    11,  1106,     0,   475,   195,    11,     0,
       0,     0,     0,     0,     0,    11,    11,     0,   184,     0,
       0,     0,     0,     0,   476,     0,     0,   238,    11,    11,
     239,     0,     0,     0,   421,     0,  1134,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   240,
       0,     0,   608,     0,     0,     0,   960,   963,     0,     0,
      11,    11,    11,    11,   800,   801,   802,     0,     0,    11,
       0,    11,     0,     0,    11,    11,     0,     0,     0,     0,
    1179,   489,  1001,     0,   277,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   831,   832,   833,   834,     0,   835,     0,     0,     0,
       0,     0,     0,   188,   188,     0,   838,     0,     0,   840,
       0,   628,   629,   630,   842,   843,   844,     0,     0,  1100,
     154,   154,     0,     0,     0,     0,   644,   645,   646,   647,
     648,   649,     0,     0,     0,  1221,    10,    10,     0,     0,
    1224,   652,   653,     0,  1228,     0,     0,     0,   224,    10,
      10,     0,     0,     0,   858,     0,     0,   860,     0,     0,
       0,   861,   862,   154,   154,   154,   864,   865,   154,   487,
     662,   472,     0,   227,   457,   182,   998,    10,     0,   457,
     182,     0,     0,     0,   669,     0,   473,   195,     0,     0,
     670,   233,   234,     0,   235,   474,     0,     0,   468,     0,
     999,   572,   573,     0,   475,   472,     0,   676,   457,   182,
       0,   184,     0,     0,   516,     0,   184,   476,     0,   195,
     473,     0,    11,     0,     0,   477,   681,   682,   683,   474,
      11,     0,     0,   475,     0,     0,     0,    11,     0,     0,
       0,     0,   240,   688,   689,   184,     0,     0,     0,     0,
    1000,   476,     0,  -277,     0,     0,   459,     0,     0,   477,
       0,     0,     0,   195,     0,     0,    10,     0,    10,  1158,
       0,  1166,  1173,     0,     0,     0,   375,   379,     0,     0,
       0,   155,   711,   155,     0,     0,   714,     0,   716,   717,
     154,     0,     0,     0,     0,     0,     0,     0,   155,     0,
     155,     0,   155,  1066,  1067,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   155,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   195,   331,
       0,     0,     0,   195,   761,   765,   771,   195,     0,     0,
       0,   776,   779,     0,     0,     0,  1090,  1092,  1094,     0,
       0,  1097,     0,     0,     0,     0,     0,     0,     0,     0,
    1070,  1071,     0,     0,     0,     0,     0,   331,     0,   806,
     809,     0,     0,     0,   817,   461,   822,     0,   711,   471,
     480,   490,   499,   519,   533,   538,   542,   547,   556,   561,
     576,   592,   610,   621,  1083,  1084,   224,     0,     0,     0,
       0,     0,     0,     0,   638,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,     0,
       0,   227,     0,   182,     0,     0,     0,     0,     0,     0,
       0,     0,   231,     0,  1111,   852,     0,     0,     0,   233,
     234,     0,   235,     0,     0,     0,     0,     0,     0,   111,
       0,     0,     0,   224,     0,     0,     0,     0,     0,   184,
       0,     0,     0,     0,   799,   476,     0,     0,   238,   803,
     804,   239,     0,  1185,   487,   226,     0,     0,   227,   819,
     182,     0,     0,     0,     0,     0,     0,     0,   488,     0,
     240,   473,     0,   545,   829,     0,   233,   234,     0,   235,
       0,     0,   887,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   836,   197,   198,     0,   184,     0,     0,   839,
       0,     0,   458,   841,     0,     0,     0,     0,   239,   845,
     477,  1183,  1184,   847,     0,     0,   849,  1200,     0,     0,
       0,  1202,   852,     0,     0,   488,     0,   240,     0,   223,
    1203,     0,     0,     0,     0,     0,   267,  1207,     0,  1208,
       0,     0,  1211,  1212,     0,     0,     0,  1217,     0,     0,
       0,     0,  1222,     0,     0,     0,  1225,     0,     0,  1227,
       0,   969,     0,   297,     0,     0,   887,     0,     0,     0,
     883,     0,     0,   981,   982,     0,     0,     0,     0,     0,
       0,     0,     0,   299,     0,   886,  1010,  1011,     0,   303,
     304,     0,   305,     0,     0,     0,     0,     0,     0,     0,
     899,     0,  1016,   900,     0,     0,     0,   902,     0,     0,
       0,     0,     0,     0,  1044,  1050,     0,     0,  1052,  1053,
    1054,  1055,   472,     0,     0,   457,   182,  1058,     0,  1060,
       0,     0,  1063,  1064,     0,     0,     0,   473,     0,     0,
       0,     0,     0,   155,   155,     0,     0,   572,   573,     0,
     475,     0,     0,   574,     0,     0,     0,     0,     0,     0,
       0,     0,   184,   380,     0,     0,     0,     0,   476,   886,
    -296,     0,     0,   459,     0,     0,   477,     0,     0,     0,
       0,     0,     0,     0,   414,  1086,   155,   155,   155,     0,
       0,   155,   422,     0,     0,     0,   817,   822,     0,     0,
       0,     0,   424,     0,     0,   425,     0,   426,     0,     0,
     427,     0,     0,   428,     0,     0,   429,   430,     0,     0,
     431,     0,     0,   432,   224,     0,   433,   434,   435,   436,
       0,     0,   437,     0,     0,     0,   439,     0,     0,     0,
     440,     0,     0,     0,     0,     0,   226,   472,     0,   227,
       0,   182,     0,     0,  1072,     0,     0,     0,     0,     0,
     231,     0,   473,     0,     0,     0,   589,   233,   234,     0,
     235,     0,   572,   573,     0,   475,     0,     0,   574,     0,
       0,     0,  1076,     0,     0,     0,     0,   184,  1085,     0,
    1180,     0,     0,   476,     0,     0,   238,   590,  1181,   239,
       0,   477,     0,  1104,  1105,  1182,     0,     0,     0,     0,
       0,     0,     0,   155,  1189,  1193,   488,     0,   240,     0,
       0,     0,     0,     0,  1110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1128,
    1129,     0,   961,   964,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   490,  1002,     0,
     672,   673,   674,   675,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1198,     0,     0,     0,     0,  1199,     0,     0,
       0,  1201,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   715,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   727,   728,     0,     0,   731,   732,     0,   733,   734,
       0,   735,   736,     0,   738,   739,   740,   741,   742,   743,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     757,   758,     0,     0,     0,     0,     0,   774,   775,     0,
       0,   783,   784,   785,     0,   786,   787,   788,   789,   790,
     791,   792,     0,     0,   795,   796,   797,   798,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   812,
     813,   814,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1159,     0,  1167,  1174,     0,
       0,     0,     0,     0,     0,     0,     0,   837,     0,    15,
       0,     0,     0,   145,   146,    17,   147,   148,     0,     0,
       0,   149,   150,   151,   846,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,     0,   890,   891,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   901,     0,
       0,     0,   903,     0,     0,   904,     0,     0,     0,   905,
       0,   906,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   921,     0,     0,   922,   923,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   929,     0,
       0,   930,     0,     0,   931,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1040,  1041,  1042,     0,
    1043,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1057,     0,  1059,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1065,     0,     0,  1068,  1069,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1073,     0,     0,     0,
       0,  1074,     0,     0,  1075,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1095,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1135,     0,  1136,  1137,     0,  1138,  1139,
       0,     0,  1140,     0,     0,  1141,  1142,    15,  1143,     0,
       0,    16,   146,    17,   147,   148,     0,     0,     0,   149,
     150,   151,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    15,     6,     0,     7,     0,     0,    17,   192,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    15,     0,     0,
       0,    16,     0,    17,   330,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    15,     0,     0,     0,    16,     0,    17,   192,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    15,     0,     0,
       0,     0,     0,    17,   192,     0,     0,  1080,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    15,     0,     0,     0,     0,     0,    17,   192,
       0,     0,  1214,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    15,  1152,     0,
       0,     0,     0,    17,   192,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    15,     0,     0,     0,    16,     0,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    15,     0,     0,
       0,     0,     0,    17,   192,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96
};

static const yytype_int16 yycheck[] =
{
       4,    12,    13,   438,     8,     9,   348,   676,   681,   135,
      14,     8,   447,    23,   454,   445,   299,   170,   716,   717,
       8,     0,   452,   453,   100,   443,   567,   445,   446,   100,
       5,   449,     7,   451,   452,   453,   454,   455,   456,   452,
     453,   582,     5,   443,     7,   714,   446,   454,    11,   467,
       8,   445,   452,   453,   454,   455,   339,     8,   452,    17,
       8,     8,    99,     8,     5,   443,     7,   445,   446,     8,
      11,   444,    82,   446,   452,   453,   454,   450,    17,    18,
     446,   454,   455,    23,   450,    31,   445,   446,   454,   467,
     449,   450,   451,   452,   453,   454,   455,   456,   103,   104,
     104,   446,   106,     8,    92,   450,   103,    95,   467,   454,
     455,   456,    17,    70,   118,   119,   120,   121,   122,     8,
       5,    67,     7,    64,   446,   100,    11,   102,   450,     5,
      22,     7,   454,   455,   456,   139,    94,    88,   101,    31,
      88,    88,    93,    88,    30,    93,    93,   300,    93,     8,
      42,   103,   104,    12,   852,   101,   309,   168,   162,   163,
      20,    43,    44,    30,    31,   169,   170,   171,   294,    55,
     101,    31,   132,   177,   134,    67,   136,   181,   182,   183,
     184,    73,     5,    31,     7,    33,    34,     8,    11,   193,
    1135,    77,    78,    14,  1139,   199,    85,    84,    87,    86,
      67,    90,    89,   207,    91,    99,    73,    67,    75,   101,
     170,    78,   100,    73,   102,   101,   100,   166,   167,    67,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,    31,   100,    33,
      34,   101,   100,   203,   204,   205,   206,     8,   101,    97,
      31,   100,     5,   101,     7,    16,    17,    18,   103,    22,
      83,    24,   266,    31,   100,   100,     4,   102,   279,    32,
     281,   282,   283,    67,   285,   286,   287,   288,   289,   290,
     291,   292,   293,    64,   295,   100,    67,    11,    26,    27,
      71,    29,   296,    31,   298,   100,   300,   301,    61,    67,
     100,    54,    40,    71,    42,    21,    69,   101,   101,    47,
      48,     5,    50,     7,     8,    31,   101,    55,    31,    75,
     101,    74,    27,   101,    37,    30,    42,    65,     5,    67,
       7,     8,    31,   101,   338,    73,   101,   341,    76,   101,
     300,    79,   101,    81,    31,   349,    51,    52,    53,   309,
     354,    67,    68,   101,    67,    42,   101,    73,    96,   136,
      98,   365,   366,   101,    31,    81,   370,     4,    67,   100,
      75,   102,   100,    78,   102,    42,   100,   100,   102,  1052,
      67,   100,   100,   102,    21,   101,    73,     4,   112,    26,
     339,   340,    29,    80,   100,   971,   101,   973,    35,    36,
      67,    38,   101,    40,    41,   100,    73,   102,    31,    26,
      47,    48,    29,    50,   101,   101,   420,   141,   142,    42,
      57,   145,    31,    40,    33,    34,   203,   204,   205,   206,
      47,    48,   973,    50,   101,    72,   160,   101,    55,    76,
      63,   101,    79,    31,    67,    33,    34,   988,    56,    28,
      73,   175,    31,   457,   458,   459,    64,   101,    67,    76,
      24,    98,    79,    32,   101,   100,   104,   102,   472,   473,
     474,   475,   476,   477,    82,   100,   103,   102,   101,    67,
      59,    98,   101,   487,   488,   100,    14,   102,    67,   438,
     214,   103,   101,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   926,   971,
     972,   951,   516,   101,     5,   445,   446,   241,   467,   105,
      85,   951,   452,   453,   454,   100,   530,   102,   100,   106,
     102,   971,   536,   973,   100,   688,   102,   261,   108,   263,
     264,   971,   972,   973,   951,   918,   919,   889,   926,   553,
     676,   951,   100,   926,   102,   100,   107,   102,   971,   972,
     973,   100,   103,   102,   971,    14,   973,   926,   572,   573,
     574,   971,   972,   918,   919,   117,   103,   950,   302,   103,
     127,     4,   306,   180,   138,   589,   590,   167,   714,   313,
     340,   421,     8,     9,     4,   319,   918,   919,    21,   100,
     100,   102,   102,    26,   725,   100,    29,   102,   190,   333,
     334,   920,    35,    36,  1098,    38,    26,    40,    41,    29,
     100,    31,   102,  1061,    47,    48,  1148,    50,   632,  1148,
     634,   635,    42,   100,    57,   102,   445,    47,    48,   445,
      50,   652,   653,   445,   100,   369,   102,   445,   445,    72,
      39,   922,   100,    76,   102,   445,    79,    67,   669,   100,
      49,   102,   100,    73,   102,    31,   445,   367,   628,    79,
     630,    81,   828,    62,    40,    98,    42,    66,   101,    45,
     100,   100,   102,   102,   891,   645,    75,   647,    98,   649,
     971,   101,   973,   100,    60,   102,    27,   701,   702,  1063,
      31,    67,   662,  1064,   120,   121,   122,    73,   951,   888,
      76,    42,   100,   454,   102,  1150,  1151,   134,   135,   136,
      51,   932,   284,   139,    55,   100,   853,   102,   688,   689,
    1107,  1146,  1150,  1151,   887,   101,    67,  1109,  1134,  1218,
    1182,   193,    73,   300,    -1,   132,   162,   163,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,   716,   717,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,   182,   183,   184,    -1,
     101,    -1,    -1,    -1,    -1,    27,    -1,   781,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   203,   204,   205,   206,
      -1,   207,    -1,    -1,    -1,    -1,   800,   801,   802,    51,
      52,    53,    -1,    55,    -1,   222,    -1,    -1,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,    77,    78,   831,   832,   833,
     834,   835,    -1,    -1,   838,    -1,   840,    -1,   842,   843,
     844,    -1,    -1,   156,   848,    -1,   159,    -1,   161,   101,
      -1,   628,    -1,   630,   858,    -1,   860,   861,   862,    -1,
     864,   865,    -1,   280,    -1,    -1,    -1,   284,   645,    -1,
     647,    -1,   649,    -1,    -1,     4,    -1,   294,    -1,    -1,
      -1,    -1,    -1,    -1,   888,   662,    -1,    -1,    -1,    -1,
      -1,    -1,   852,   617,   618,    -1,    -1,    26,    -1,    -1,
      29,    -1,    31,    -1,    -1,   218,   219,   220,   221,    -1,
      -1,    40,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,
      -1,    50,    -1,   927,  1077,    -1,    55,   887,   932,    -1,
      -1,    -1,    -1,    -1,    -1,   939,   940,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    76,   952,   953,
      79,    -1,    -1,    -1,   370,    -1,  1109,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,   101,    -1,    -1,    -1,   925,   926,    -1,    -1,
     984,   985,   986,   987,   708,   709,   710,    -1,    -1,   993,
      -1,   995,    -1,    -1,   998,   999,    -1,    -1,    -1,    -1,
    1153,   950,   951,    -1,   421,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   745,   746,   747,   748,    -1,   750,    -1,    -1,    -1,
      -1,    -1,    -1,   166,   167,    -1,   760,    -1,    -1,   763,
      -1,   457,   458,   459,   768,   769,   770,    -1,    -1,  1060,
    1010,  1011,    -1,    -1,    -1,    -1,   472,   473,   474,   475,
     476,   477,    -1,    -1,    -1,  1218,  1070,  1071,    -1,    -1,
    1223,   487,   488,    -1,  1227,    -1,    -1,    -1,     4,  1083,
    1084,    -1,    -1,    -1,   808,    -1,    -1,   811,    -1,    -1,
      -1,   815,   816,  1053,  1054,  1055,   820,   821,  1058,    25,
     516,    27,    -1,    29,    30,    31,    27,  1111,    -1,    30,
      31,    -1,    -1,    -1,   530,    -1,    42,  1077,    -1,    -1,
     536,    47,    48,    -1,    50,    51,    -1,    -1,   441,    -1,
      51,    52,    53,    -1,    55,    27,    -1,   553,    30,    31,
      -1,    67,    -1,    -1,    65,    -1,    67,    73,    -1,  1109,
      42,    -1,  1156,    -1,    -1,    81,   572,   573,   574,    51,
    1164,    -1,    -1,    55,    -1,    -1,    -1,  1171,    -1,    -1,
      -1,    -1,    98,   589,   590,    67,    -1,    -1,    -1,    -1,
     101,    73,    -1,    75,    -1,    -1,    78,    -1,    -1,    81,
      -1,    -1,    -1,  1153,    -1,    -1,  1200,    -1,  1202,  1148,
      -1,  1150,  1151,    -1,    -1,    -1,   339,   340,    -1,    -1,
      -1,   628,   629,   630,    -1,    -1,   632,    -1,   634,   635,
    1180,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   645,    -1,
     647,    -1,   649,  1010,  1011,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   662,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1218,   676,
      -1,    -1,    -1,  1223,   681,   682,   683,  1227,    -1,    -1,
      -1,   688,   689,    -1,    -1,    -1,  1053,  1054,  1055,    -1,
      -1,  1058,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1014,  1015,    -1,    -1,    -1,    -1,    -1,   714,    -1,   716,
     717,    -1,    -1,    -1,   721,   438,   723,    -1,   725,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,  1048,  1049,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   467,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,  1088,   781,    -1,    -1,    -1,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,   707,    73,    -1,    -1,    76,   712,
     713,    79,    -1,  1180,    25,    26,    -1,    -1,    29,   722,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    -1,
      98,    42,    -1,   101,   737,    -1,    47,    48,    -1,    50,
      -1,    -1,   848,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   755,   172,   173,    -1,    67,    -1,    -1,   762,
      -1,    -1,    73,   766,    -1,    -1,    -1,    -1,    79,   772,
      81,  1178,  1179,   776,    -1,    -1,   779,  1191,    -1,    -1,
      -1,  1195,   888,    -1,    -1,    96,    -1,    98,    -1,   208,
    1197,    -1,    -1,    -1,    -1,    -1,   215,  1204,    -1,  1206,
      -1,    -1,  1209,  1210,    -1,    -1,    -1,  1214,    -1,    -1,
      -1,    -1,  1219,    -1,    -1,    -1,  1223,    -1,    -1,  1226,
      -1,   927,    -1,   242,    -1,    -1,   932,    -1,    -1,    -1,
     833,    -1,    -1,   939,   940,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   262,    -1,   848,   952,   953,    -1,   268,
     269,    -1,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     863,    -1,   969,   866,    -1,    -1,    -1,   870,    -1,    -1,
      -1,    -1,    -1,    -1,   981,   982,    -1,    -1,   984,   985,
     986,   987,    27,    -1,    -1,    30,    31,   993,    -1,   995,
      -1,    -1,   998,   999,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,  1010,  1011,    -1,    -1,    52,    53,    -1,
      55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,   342,    -1,    -1,    -1,    -1,    73,   932,
      75,    -1,    -1,    78,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   363,  1052,  1053,  1054,  1055,    -1,
      -1,  1058,   371,    -1,    -1,    -1,  1063,  1064,    -1,    -1,
      -1,    -1,   381,    -1,    -1,   384,    -1,   386,    -1,    -1,
     389,    -1,    -1,   392,    -1,    -1,   395,   396,    -1,    -1,
     399,    -1,    -1,   402,     4,    -1,   405,   406,   407,   408,
      -1,    -1,   411,    -1,    -1,    -1,   415,    -1,    -1,    -1,
     419,    -1,    -1,    -1,    -1,    -1,    26,    27,    -1,    29,
      -1,    31,    -1,    -1,  1017,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    42,    -1,    -1,    -1,    46,    47,    48,    -1,
      50,    -1,    52,    53,    -1,    55,    -1,    -1,    58,    -1,
      -1,    -1,  1045,    -1,    -1,    -1,    -1,    67,  1051,    -1,
    1156,    -1,    -1,    73,    -1,    -1,    76,    77,  1164,    79,
      -1,    81,    -1,  1066,  1067,  1171,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1180,  1181,  1182,    96,    -1,    98,    -1,
      -1,    -1,    -1,    -1,  1087,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1102,
    1103,    -1,   925,   926,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   950,   951,    -1,
     549,   550,   551,   552,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1185,    -1,    -1,    -1,    -1,  1190,    -1,    -1,
      -1,  1194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   633,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   650,   651,    -1,    -1,   654,   655,    -1,   657,   658,
      -1,   660,   661,    -1,   663,   664,   665,   666,   667,   668,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     679,   680,    -1,    -1,    -1,    -1,    -1,   686,   687,    -1,
      -1,   690,   691,   692,    -1,   694,   695,   696,   697,   698,
     699,   700,    -1,    -1,   703,   704,   705,   706,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   718,
     719,   720,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1148,    -1,  1150,  1151,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   756,    -1,     4,
      -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,   773,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    -1,   854,   855,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   867,    -1,
      -1,    -1,   871,    -1,    -1,   874,    -1,    -1,    -1,   878,
      -1,   880,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   892,    -1,    -1,   895,   896,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   907,    -1,
      -1,   910,    -1,    -1,   913,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   975,   976,   977,    -1,
     979,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   992,    -1,   994,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1009,    -1,    -1,  1012,  1013,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1025,    -1,    -1,    -1,
      -1,  1030,    -1,    -1,  1033,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1056,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1112,    -1,  1114,  1115,    -1,  1117,  1118,
      -1,    -1,  1121,    -1,    -1,  1124,  1125,     4,  1127,    -1,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,     5,    -1,     7,    -1,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,    -1,    -1,
      -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,     8,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,    -1,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,     5,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,     8,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,    -1,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   110,   115,   119,   444,     0,     5,     7,    54,    74,
     443,   443,   445,   445,   444,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   111,   429,   450,
     451,   429,   112,   116,   444,   120,   444,   100,   100,   113,
     433,   433,    99,    11,   118,   434,   443,   122,   444,   435,
      56,    64,    82,   117,   124,   125,   139,   121,   123,   124,
       8,   436,   437,   445,   445,   445,   445,   126,   126,    23,
     136,   114,   100,   102,   440,     8,     9,    11,    12,    16,
      17,    18,   438,   449,   450,   111,   447,   449,   111,   428,
     451,   447,    43,    44,   127,   131,   140,   140,   445,   444,
     444,   444,   100,   102,   432,   448,   432,   444,   432,   445,
     445,    28,    31,    59,    67,   141,   142,   143,   144,   145,
     145,   429,    11,   430,   441,   450,   439,   433,   433,   444,
     130,   429,   130,   445,   445,   445,   445,    70,   146,   153,
     153,   137,   436,   101,    99,   101,   128,   132,   447,   447,
     447,   447,   445,   433,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    76,    79,
      98,   101,   154,   155,   169,   174,   179,   229,   233,   234,
     244,   252,   260,   270,   285,   287,   293,   309,   320,   339,
     347,   101,   100,   100,   102,   442,   444,   433,   100,   100,
     102,   133,   432,   432,   432,   432,    12,   111,   151,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   444,   433,   444,   433,
     444,   444,     8,   433,   433,   433,   152,   147,   429,   103,
     111,   317,   400,   401,   429,   429,   429,   111,   351,   401,
     429,   429,   429,   429,   429,   429,   429,   429,   181,   429,
      11,   111,   431,   451,   452,   429,   139,   441,   444,   129,
     134,   444,   100,   102,   148,   286,   430,   310,   402,   444,
     271,   235,   156,   348,   444,   170,   230,   175,   245,   253,
     261,   340,   321,   100,   294,   444,   444,   288,   138,   101,
      71,   101,   135,   139,   143,   144,   101,   135,   143,   144,
     433,   100,   102,   289,   100,   400,   100,   102,   272,   100,
     102,   236,   100,   102,   157,   100,   100,   102,   171,   100,
     102,   231,   100,   102,   176,   100,   100,   100,   100,   102,
     341,   100,   102,   322,   433,   100,   102,   295,   289,   101,
     444,   445,   433,   149,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   184,   433,
     433,   151,   150,   290,   312,   274,   237,   158,   350,   172,
     232,   177,   247,   255,   263,   342,   323,    30,    73,    78,
     143,   144,   164,   180,   185,   227,   228,   296,   432,   101,
     143,   144,    27,    42,    51,    55,    73,    81,   101,   143,
     144,   165,   173,   367,   372,   377,   388,    25,    96,   143,
     144,   164,   173,   233,   279,   311,   318,   367,   143,   144,
     165,   173,   228,   234,   244,   252,   260,   273,   277,   278,
     279,   285,   287,   367,   372,   377,    65,   101,   143,   144,
     165,   173,   179,   229,   233,   318,   367,   372,   381,   388,
      20,   101,   143,   144,   159,   164,    37,   143,   144,   349,
     101,   143,   144,   165,   173,   101,   143,   144,   165,   179,
     229,   233,   318,    22,   101,   143,   144,   165,   173,   178,
     143,   144,   165,   173,   227,   228,   246,   249,   367,   372,
     377,   388,    52,    53,    58,   143,   144,   165,   173,   227,
     228,   254,   257,   367,   372,   382,   384,   386,   388,    46,
      77,   143,   144,   165,   173,   179,   229,   233,   262,   318,
     367,   372,   382,   384,   386,   388,   391,   394,   101,   143,
     144,   165,   173,   179,   229,   233,   388,    45,    60,   101,
     143,   144,   165,   173,   179,   229,   328,   336,   445,   445,
     445,   101,    75,   182,    21,    68,   101,   143,   144,   165,
     173,   300,   305,   367,   445,   445,   445,   445,   445,   445,
     291,   292,   445,   445,   314,   316,   101,   315,   313,   101,
     276,   275,   445,   242,   240,   243,   241,   238,   239,   445,
     445,   101,   433,   433,   433,   433,   445,   101,   182,   248,
     251,   445,   445,   445,   101,   182,   256,   259,   445,   445,
     266,   267,   269,   101,   268,   264,   265,   344,   345,   346,
     343,   444,   444,   324,   325,   326,   327,   447,    85,    87,
      90,   111,   168,   447,   445,   433,   445,   445,   298,   299,
     297,   373,   447,   378,   447,   166,   447,   433,   433,   429,
     429,   433,   433,   433,   433,   433,   433,   447,   433,   433,
     433,   433,   433,   433,   429,    84,    86,    89,    91,   352,
     353,   354,   355,   359,   363,   431,   250,   433,   433,    17,
      94,   111,   385,   424,    18,   111,   383,   424,   425,    92,
      95,   111,   387,   258,   433,   433,   111,   395,   430,   111,
     392,   403,   430,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   329,   337,   433,   433,   433,   433,   432,
     444,   444,   444,   432,   432,   431,   111,   308,   403,   111,
     303,   403,   433,   433,   433,    88,    93,   111,   375,   432,
      88,    93,   111,   380,   389,   168,   368,   280,   319,   432,
     160,   444,   444,   444,   444,   444,   432,   433,   444,   432,
     444,   432,   444,   444,   444,   432,   433,   432,   396,   432,
     393,   432,   445,   404,   100,   100,   183,   306,   444,   301,
     444,   444,   444,   374,   444,   444,   379,   100,   102,   390,
     167,   100,   102,   369,   100,   102,   281,   322,   100,   102,
     161,   356,   360,   432,   364,   397,   432,   445,   403,   402,
     433,   433,   100,   102,   186,   100,   100,   102,   302,   432,
     432,   433,   432,   433,   433,   433,   433,   100,   102,   357,
     100,   102,   361,   100,   102,   365,   430,   393,   331,   331,
     187,   433,   433,   433,   196,   370,   282,    83,   162,   433,
     433,   433,   398,   179,   229,   233,   330,   388,   338,    61,
      69,   178,   188,   189,   202,   203,   213,   214,   216,   217,
     312,   304,    33,    34,   101,   143,   144,   225,   226,   101,
     143,   144,   101,   143,   144,   165,   173,   233,   367,   445,
     101,   358,   362,   366,   397,   333,   334,   335,   101,   332,
     101,   445,   445,   101,    39,    49,    62,    66,   190,   191,
     193,   197,   211,    32,   205,    24,   219,   307,    27,    51,
     101,   143,   144,   228,   371,   376,   381,   382,   384,   388,
     445,   445,   284,   283,    88,    93,   111,   163,   101,   227,
     228,   371,   376,   382,   384,   388,   391,   101,   227,   228,
     388,   391,   101,   182,   227,   228,   371,   376,   382,   384,
     433,   433,   433,   433,   111,   405,   406,   407,    88,    93,
     111,   215,   445,   445,   445,   445,   182,   433,   445,   433,
     445,   218,   101,   445,   445,   433,   447,   447,   433,   433,
     444,   444,   432,   433,   433,   433,   432,   103,   408,   409,
      14,   411,   412,   444,   444,   432,   111,   192,   424,   194,
     447,   198,   447,   399,   447,   433,   206,   447,   204,   220,
     429,   219,   375,   380,   432,   432,   430,   410,   103,   413,
     432,   444,   100,   102,   195,   100,   102,   199,   100,   102,
     212,   100,   102,   207,   205,   100,   102,   221,   432,   432,
     416,   408,   412,   414,   430,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   104,   415,   416,   196,   200,   196,
     208,   222,     5,   446,   409,   101,    97,   101,   143,   144,
     201,   225,   226,   101,    80,   101,   143,   144,   164,   173,
     209,    63,   101,   143,   144,   164,   173,   223,   417,   430,
     445,   445,   445,   446,   446,   447,    16,    17,    18,   111,
     210,   426,   427,   111,   224,   425,   105,   106,   432,   432,
     444,   432,   444,   446,    85,   418,   423,   446,   446,   107,
     103,   446,   446,   108,    14,   419,   420,   446,   421,   103,
     420,   430,   446,   422,   430,   446,   103,   446,   430
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   109,   110,   110,   112,   111,   113,   114,   113,   116,
     117,   115,   118,   118,   118,   118,   120,   121,   119,   122,
     122,   123,   122,   124,   125,   126,   126,   126,   128,   127,
     129,   129,   129,   129,   129,   130,   132,   131,   133,   133,
     134,   134,   134,   134,   135,   137,   138,   136,   139,   140,
     140,   140,   140,   140,   141,   142,   143,   144,   145,   145,
     147,   146,   148,   149,   148,   150,   150,   150,   152,   151,
     151,   153,   153,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   156,   155,   157,   157,   158,   158,   158,
     158,   158,   160,   159,   161,   161,   162,   163,   163,   163,
     164,   166,   167,   165,   168,   168,   168,   168,   170,   169,
     171,   171,   172,   172,   172,   172,   172,   173,   175,   174,
     176,   176,   177,   177,   177,   177,   177,   177,   178,   180,
     179,   181,   183,   182,   184,   185,   184,   184,   184,   184,
     184,   184,   186,   186,   187,   188,   188,   188,   188,   188,
     188,   189,   189,   189,   189,   189,   189,   190,   191,   192,
     192,   193,   194,   195,   195,   196,   196,   196,   196,   196,
     197,   198,   199,   199,   200,   200,   200,   200,   200,   200,
     201,   203,   202,   204,   204,   205,   206,   207,   207,   208,
     208,   208,   208,   208,   208,   209,   210,   210,   211,   212,
     212,   213,   214,   215,   215,   215,   217,   216,   218,   218,
     219,   220,   221,   221,   222,   222,   222,   222,   222,   222,
     223,   224,   224,   225,   226,   227,   228,   230,   229,   231,
     231,   232,   232,   232,   232,   232,   232,   232,   232,   233,
     233,   233,   233,   233,   233,   233,   233,   235,   234,   236,
     236,   237,   238,   237,   237,   239,   237,   237,   237,   237,
     237,   237,   240,   237,   241,   237,   242,   237,   243,   237,
     245,   246,   244,   247,   248,   247,   247,   249,   250,   247,
     247,   251,   247,   247,   247,   247,   247,   247,   247,   253,
     254,   252,   255,   256,   255,   255,   257,   258,   255,   255,
     255,   259,   255,   255,   255,   255,   255,   255,   255,   255,
     261,   262,   260,   263,   264,   263,   263,   265,   263,   263,
     263,   263,   263,   263,   263,   263,   263,   263,   266,   263,
     267,   263,   268,   263,   269,   263,   271,   270,   272,   273,
     272,   274,   275,   274,   274,   274,   274,   274,   274,   274,
     274,   276,   274,   277,   277,   278,   278,   278,   278,   278,
     278,   280,   279,   281,   281,   282,   283,   282,   282,   282,
     282,   282,   284,   282,   286,   285,   288,   287,   289,   289,
     290,   291,   290,   290,   292,   290,   290,   290,   290,   290,
     290,   294,   293,   295,   295,   296,   297,   296,   296,   296,
     296,   296,   298,   296,   299,   296,   301,   300,   302,   302,
     303,   303,   304,   304,   304,   304,   304,   304,   304,   304,
     304,   304,   306,   307,   305,   308,   308,   310,   311,   309,
     312,   313,   312,   312,   312,   312,   312,   314,   312,   315,
     312,   316,   312,   317,   317,   319,   318,   321,   320,   322,
     322,   323,   323,   323,   323,   323,   324,   323,   325,   323,
     326,   323,   327,   323,   329,   330,   328,   331,   332,   331,
     333,   331,   334,   331,   335,   331,   337,   338,   336,   340,
     339,   341,   341,   342,   343,   342,   342,   342,   342,   342,
     344,   342,   345,   342,   346,   342,   348,   349,   347,   350,
     350,   350,   350,   351,   351,   352,   352,   353,   353,   353,
     354,   356,   355,   357,   357,   358,   358,   358,   358,   358,
     358,   358,   358,   358,   360,   359,   361,   361,   362,   362,
     362,   362,   362,   364,   363,   365,   365,   366,   366,   366,
     366,   366,   366,   366,   366,   368,   367,   369,   369,   370,
     370,   370,   371,   373,   374,   372,   375,   375,   375,   376,
     378,   379,   377,   380,   380,   380,   381,   382,   383,   383,
     384,   385,   385,   385,   386,   387,   387,   387,   389,   388,
     390,   390,   391,   392,   392,   393,   393,   394,   396,   395,
     395,   398,   397,   397,   399,   400,   401,   402,   402,   404,
     403,   406,   405,   407,   405,   405,   408,   409,   410,   410,
     411,   412,   413,   413,   414,   415,   415,   416,   416,   417,
     418,   419,   420,   421,   421,   422,   422,   423,   424,   425,
     425,   426,   426,   427,   427,   428,   428,   429,   429,   430,
     430,   431,   431,   431,   432,   432,   433,   433,   433,   435,
     434,   436,   437,   437,   438,   438,   438,   439,   439,   440,
     440,   441,   441,   442,   442,   443,   443,   444,   444,   445,
     446,   446,   448,   447,   447,   449,   449,   449,   449,   449,
     449,   449,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   450,   450,   451,   452
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     4,     0,     0,     6,     0,
       0,    15,     0,     2,     2,     2,     0,     0,    15,     0,
       2,     0,     3,     4,     4,     0,     2,     2,     0,     9,
       0,     2,     2,     2,     2,     1,     0,     6,     1,     4,
       0,     2,     2,     2,     4,     0,     0,    10,     4,     0,
       2,     2,     2,     2,     4,     4,     4,     4,     0,     3,
       0,     5,     1,     0,     5,     0,     2,     2,     0,     3,
       1,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     5,     1,     4,     0,     2,     2,
       2,     2,     0,     6,     1,     4,     4,     2,     2,     1,
       4,     0,     0,     6,     2,     2,     2,     1,     0,     5,
       1,     4,     0,     2,     2,     2,     2,     4,     0,     5,
       1,     4,     0,     2,     2,     2,     2,     2,     4,     0,
       8,     1,     0,     5,     0,     0,     4,     2,     2,     2,
       2,     2,     1,     4,     1,     1,     1,     1,     1,     1,
       1,     0,     2,     2,     2,     3,     4,     0,     4,     2,
       1,     5,     1,     1,     4,     0,     2,     2,     2,     2,
       5,     1,     1,     4,     0,     2,     2,     2,     2,     2,
       4,     0,     4,     0,     3,     4,     1,     1,     4,     0,
       2,     2,     2,     2,     2,     4,     2,     1,     4,     1,
       4,     4,     4,     2,     2,     1,     0,     3,     0,     2,
       5,     1,     1,     4,     0,     2,     2,     2,     2,     2,
       4,     2,     1,     4,     4,     4,     4,     0,     5,     1,
       4,     0,     2,     2,     2,     3,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     5,     1,
       4,     0,     0,     4,     2,     0,     4,     2,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     0,     9,     0,     0,     4,     2,     0,     0,     5,
       2,     0,     4,     2,     2,     2,     2,     2,     2,     0,
       0,     9,     0,     0,     4,     2,     0,     0,     5,     2,
       2,     0,     4,     2,     2,     2,     2,     2,     2,     2,
       0,     0,     9,     0,     0,     4,     2,     0,     4,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     5,     1,     0,
       5,     0,     0,     4,     2,     2,     2,     2,     2,     2,
       2,     0,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     5,     0,     5,     1,     4,
       0,     0,     4,     2,     0,     4,     2,     2,     2,     2,
       2,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     5,     1,     4,
       2,     1,     0,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     0,     0,     9,     2,     1,     0,     0,     9,
       0,     0,     4,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     2,     1,     0,     5,     0,     5,     1,
       4,     0,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     0,     8,     0,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     0,     8,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
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
       4,     2,     0,     1,     0,     1,     3,     0,     5,     1,
       4,     0,     3,     2,     5,     1,     1,     0,     2,     2,
       0,     1,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1
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
          case 130: /* tmp_identifier_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 187: /* type_stmtsep  */

      { if (read_all) {
                yang_delete_type(module, ((*yyvaluep).type));
              }
            }

        break;

    case 198: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 274: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

        break;

    case 350: /* deviation_opt_stmt  */

      { if (read_all) {
                ly_set_free(((*yyvaluep).nodes).deviation->dflt_check);
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
struct lys_include inc;
struct lys_module *trg = NULL;
struct lys_node *tpdf_parent = NULL, *data_node = NULL;
void *actual = NULL;
int config_inherit = 0, actual_type = 0;
int64_t cnt_val;
void *yang_type = NULL;


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
                        s = yang_read_string(yyget_text(scanner) + 1, yyget_leng(scanner) - 2, yylloc.first_column, (trg) ? trg->version : 0);
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
                  temp = yang_read_string(yyget_text(scanner) + 1, yyget_leng(scanner) - 2, yylloc.first_column, (trg) ? trg->version : 0);
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

    { if (read_all) {
                                               if (yang_check_version(module, submodule, s, (yyvsp[-1].i))) {
                                                 YYABORT;
                                               }
                                               (yyval.i) = 1;
                                               s = NULL;
                                             }
                                           }

    break;

  case 14:

    { if (read_all && yang_read_common(module, s, NAMESPACE_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 15:

    { if (read_all && yang_read_prefix(module, NULL, s)) {YYABORT;} s=NULL; }

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

  case 25:

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

  case 28:

    {
                 if (!read_all) {
                   size_arrays->imp++;
                 } else {
                   actual = &trg->imp[trg->imp_size];
                   actual_type=IMPORT_KEYWORD;
                 }
             }

    break;

  case 29:

    { if (read_all) {
                             (yyval.inc) = trg;
                             if (yang_fill_import(trg, actual, (yyvsp[-6].str))) {
                               YYABORT;
                             }
                             trg = (yyval.inc);
                             config_inherit = ENABLE_INHERIT;
                           }
                         }

    break;

  case 30:

    { (yyval.i) = 0; }

    break;

  case 31:

    { if (read_all && yang_read_prefix(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                   (yyval.i) = (yyvsp[-1].i);
                                 }

    break;

  case 32:

    { if (read_all) {
                                          if (trg->version != 2) {
                                            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "description");
                                          }
                                          if (yang_read_description(trg, actual, s, "import")) {
                                            YYABORT;
                                          }
                                        }
                                        s = NULL;
                                        (yyval.i) = (yyvsp[-1].i);
                                      }

    break;

  case 33:

    { if (read_all) {
                                        if (trg->version != 2) {
                                          LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "reference");
                                        }
                                        if (yang_read_reference(trg, actual, s, "import")) {
                                          YYABORT;
                                        }
                                      }
                                      s = NULL;
                                      (yyval.i) = (yyvsp[-1].i);
                                    }

    break;

  case 34:

    { if (read_all) {
                                            if ((yyvsp[-1].i)) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "import");
                                              YYABORT;
                                            }
                                          }
                                          (yyval.i) = 1;
                                        }

    break;

  case 35:

    { (yyval.str) = s; s = NULL; }

    break;

  case 36:

    { if (read_all) {
                                                             memset(&inc, 0, sizeof inc);
                                                             actual = &inc;
                                                             actual_type = INCLUDE_KEYWORD;
                                                           }
                                                           else {
                                                             size_arrays->inc++;
                                                           }
                                                         }

    break;

  case 37:

    { if (read_all) {
                                      (yyval.inc) = trg;
                                      int rc;
                                      rc = yang_fill_include(module, submodule, (yyvsp[-3].str), actual, unres);
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

  case 40:

    { (yyval.i) = 0; }

    break;

  case 41:

    { if (read_all) {
                                           if (trg->version != 2) {
                                             LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "description");
                                           }
                                           if (yang_read_description(trg, actual, s, "import")) {
                                            YYABORT;
                                           }
                                         }
                                         s = NULL;
                                         (yyval.i) = (yyvsp[-1].i);
                                       }

    break;

  case 42:

    { if (read_all) {
                                         if (trg->version != 2) {
                                           LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "reference");
                                         }
                                         if (yang_read_reference(trg, actual, s, "import")) {
                                          YYABORT;
                                         }
                                       }
                                       s = NULL;
                                       (yyval.i) = (yyvsp[-1].i);
                                     }

    break;

  case 43:

    { if (read_all) {
                                             if ((yyvsp[-1].i)) {
                                               LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "include");
                                               YYABORT;
                                             }
                                           }
                                           (yyval.i) = 1;
                                         }

    break;

  case 44:

    { if (read_all) {
                                 if (actual_type==IMPORT_KEYWORD) {
                                     memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                                 } else {                              // INCLUDE KEYWORD
                                     memcpy(((struct lys_include *)actual)->rev, s, LY_REV_SIZE-1);
                                 }
                                 free(s);
                                 s = NULL;
                               }
                             }

    break;

  case 45:

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

  case 46:

    { if (read_all) {
                                     if (yang_read_prefix(trg, NULL, s)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }
                                 }

    break;

  case 50:

    { if (read_all && yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 51:

    { if (read_all && yang_read_common(trg, s, CONTACT_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 52:

    { if (read_all && yang_read_description(trg, NULL, s, NULL)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 53:

    { if (read_all && yang_read_reference(trg, NULL, s, NULL)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 58:

    { if (read_all && size_arrays->rev) {
                           trg->rev = calloc(size_arrays->rev, sizeof *trg->rev);
                           if (!trg->rev) {
                             LOGMEM;
                             YYABORT;
                           }
                         }
                       }

    break;

  case 60:

    { if (read_all) {
                                                     if(!(actual=yang_read_revision(trg,s))) {YYABORT;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }

    break;

  case 63:

    { actual_type = REVISION_KEYWORD; }

    break;

  case 66:

    { if (read_all && yang_read_description(trg, actual, s, "revision")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 67:

    { if (read_all && yang_read_reference(trg, actual, s, "revision")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 68:

    { if (read_all) {
                                s = strdup(yyget_text(scanner));
                                if (!s) {
                                  LOGMEM;
                                  YYABORT;
                                }
                              }
                            }

    break;

  case 70:

    { if (read_all && lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 71:

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

  case 72:

    { actual = NULL; }

    break;

  case 76:

    { if (!read_all) { size_arrays->tpdf++; } }

    break;

  case 79:

    { if (!read_all) {
                     size_arrays->augment++;
                   } else {
                     config_inherit = ENABLE_INHERIT;
                   }
                 }

    break;

  case 82:

    { if (!read_all) { size_arrays->deviation++; } }

    break;

  case 83:

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

  case 87:

    { (yyval.uint) = 0; }

    break;

  case 88:

    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }

    break;

  case 89:

    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }

    break;

  case 90:

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

  case 91:

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

  case 92:

    { free(s); s = NULL; }

    break;

  case 99:

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

  case 100:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 101:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 102:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 103:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 104:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 105:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 106:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 107:

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

  case 108:

    { if (read_all) {
                                                         if (!(actual = yang_read_feature(trg, s))) {YYABORT;}
                                                         s=NULL;
                                                       } else {
                                                         size_arrays->features++;
                                                       }
                                                     }

    break;

  case 112:

    { if (read_all) {
                             if (size_arrays->node[size_arrays->next].if_features) {
                               ((struct lys_feature*)actual)->iffeature = calloc(size_arrays->node[size_arrays->next].if_features,
                                                                                sizeof *((struct lys_feature*)actual)->iffeature);
                               if (!((struct lys_feature*)actual)->iffeature) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }
                             if (store_flags((struct lys_node *)actual, size_arrays->node[size_arrays->next].flags, 0)) {
                                 YYABORT;
                             }
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

  case 113:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, actual, s, unres, FEATURE_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                          /* check for circular dependencies */
                                          if (((struct lys_feature *)actual)->iffeature_size) {
                                            if (unres_schema_add_node(module, unres, (struct lys_feature *)actual, UNRES_FEATURE, NULL) == -1) {
                                                YYABORT;
                                            }
                                          }
                                        } else {
                                          size_arrays->node[(yyvsp[-1].i)].if_features++;
                                        }
                                      }

    break;

  case 114:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].i)].flags, LYS_STATUS_MASK, "status", "feature", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         }
                                       }

    break;

  case 115:

    { if (read_all && yang_read_description(trg, actual, s, "feature")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 116:

    { if (read_all && yang_read_reference(trg, actual, s, "feature")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 118:

    { if (read_all) {
                                                           if (!(actual = yang_read_identity(trg,s))) {YYABORT;}
                                                           s = NULL;
                                                         } else {
                                                           size_arrays->ident++;
                                                         }
                                                       }

    break;

  case 122:

    { if (read_all) {
                                  struct lys_ident *ident = actual;

                                  if (size_arrays->node[size_arrays->next].base > 1 && (trg->version < 2)) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                                    YYABORT;
                                  }

                                  if ((trg->version < 2) && size_arrays->node[size_arrays->next].if_features) {
                                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "identity");
                                    YYABORT;
                                  }

                                  if (size_arrays->node[size_arrays->next].base) {
                                    ident->base_size = 0;
                                    ident->base = calloc(size_arrays->node[size_arrays->next].base, sizeof *ident->base);
                                    if (!ident->base) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }

                                  if (size_arrays->node[size_arrays->next].if_features) {
                                      ident->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *ident->iffeature);
                                      if (!ident->iffeature) {
                                          LOGMEM;
                                          YYABORT;
                                      }
                                  }

                                  ident->flags |= size_arrays->node[size_arrays->next].flags;
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

  case 123:

    { if (read_all) {
                                     if (yang_read_base(trg, actual, s, unres)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   } else {
                                     size_arrays->node[(yyvsp[-1].i)].base++;
                                   }
                                 }

    break;

  case 124:

    { if (read_all) {
                                           if (yang_read_if_feature(trg, actual, s, unres, IDENTITY_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s=NULL;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].i)].if_features++;
                                         }
                                       }

    break;

  case 125:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].i)].flags, LYS_STATUS_MASK, "status", "identity", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 126:

    { if (read_all && yang_read_description(trg, actual, s, "identity")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 127:

    { if (read_all && yang_read_reference(trg, actual, s, "identity")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 129:

    { if (read_all) {
                                    if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                                      YYABORT;
                                    }
                                    actual = tpdf_parent;

                                    /* check default value */
                                    if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_tpdf->dflt) == -1) {
                                      YYABORT;
                                    }
                                  }
                                }

    break;

  case 131:

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

  case 132:

    { if (read_all && !(actual = yang_read_type(trg, actual, s, actual_type))) {
                                                       YYABORT;
                                                     }
                                                     s = NULL;
                                                   }

    break;

  case 134:

    { (yyval.nodes).node.ptr_tpdf = actual;
                        (yyval.nodes).node.flag = 0;
                      }

    break;

  case 135:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, (yyvsp[0].nodes).node.ptr_tpdf, "type", "typedef");
                       YYABORT;
                     }
                   }

    break;

  case 136:

    { if (read_all) {
                   actual = (yyvsp[-3].nodes).node.ptr_tpdf;
                   actual_type = TYPEDEF_KEYWORD;
                   (yyvsp[-3].nodes).node.flag |= LYS_TYPE_DEF;
                   (yyval.nodes) = (yyvsp[-3].nodes);
                   if (unres_schema_add_node(trg, unres, &(yyvsp[-3].nodes).node.ptr_tpdf->type, UNRES_TYPE_DER_TPDF, tpdf_parent) == -1) {
                     lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-3].nodes).node.ptr_tpdf->type.der)->name);
                     free((yyvsp[-3].nodes).node.ptr_tpdf->type.der);
                     (yyvsp[-3].nodes).node.ptr_tpdf->type.der = NULL;
                     YYABORT;
                   }
                 }
               }

    break;

  case 137:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 138:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 139:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 140:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 141:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 144:

    { if (read_all) {
                          (yyval.type) = actual;
                        }
                      }

    break;

  case 147:

    { /*leafref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 148:

    { /*identityref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->flags |= LYS_TYPE_BASE;
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 149:

    { /*instance_identifier_specification */
                             if (read_all) {
                               ((struct yang_type *)actual)->base = LY_TYPE_INST;
                             }
                           }

    break;

  case 151:

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

  case 153:

    { if (!read_all) {
                                                  size_arrays->node[(yyvsp[-1].uint)].pattern++; /* count of pattern*/
                                                }
                                              }

    break;

  case 156:

    { if (read_all) {
                                                                  actual = (yyvsp[-2].v);
                                                                } else {
                                                                  size_arrays->node[(yyvsp[-3].uint)].uni++; /* count of union*/
                                                                }
                                                              }

    break;

  case 157:

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

  case 158:

    { if (read_all && yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 159:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 160:

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

  case 161:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 162:

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

  case 165:

    { switch (actual_type) {
                           case MUST_KEYWORD:
                             (yyval.str) = "must";
                             break;
                           case LENGTH_KEYWORD:
                             (yyval.str) = "length";
                             break;
                           case RANGE_KEYWORD:
                             (yyval.str) = "range";
                             break;
                           }
                         }

    break;

  case 166:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 167:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 168:

    { if (read_all && yang_read_description(trg, actual, s, (yyvsp[-1].str))) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 169:

    { if (read_all && yang_read_reference(trg, actual, s, (yyvsp[-1].str))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 170:

    { if (read_all) {
                                                                          if (yang_read_pattern(trg, actual, (yyvsp[-2].str), (yyvsp[-1].ch))) {
                                                                            YYABORT;
                                                                          }
                                                                          actual = yang_type;
                                                                          actual_type = TYPE_KEYWORD;
                                                                        }
                                                                      }

    break;

  case 171:

    { if (read_all) {
                            struct yang_type *tmp = (struct yang_type *)actual;

                            yang_type = actual;
                            actual = &tmp->type->info.str.patterns[tmp->type->info.str.pat_count];
                            tmp->type->info.str.pat_count++;
                            (yyval.str) = s;
                            s = NULL;
                          } else {
                            (yyval.str) = NULL;
                          }
                        }

    break;

  case 172:

    { (yyval.ch) = 0x06; }

    break;

  case 173:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 174:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 175:

    { if (read_all) {
                                        if (trg->version < 2) {
                                          LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "modifier");
                                          YYABORT;
                                        }
                                        if ((yyvsp[-1].ch) != 0x06) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "modifier", "pattern");
                                          YYABORT;
                                        }
                                        (yyval.ch) = (yyvsp[0].ch);
                                      }
                                    }

    break;

  case 176:

    { if (read_all && yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 177:

    { if (read_all && yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 178:

    { if (read_all && yang_read_description(trg, actual, s, "pattern")) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 179:

    { if (read_all && yang_read_reference(trg, actual, s, "pattern")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 180:

    { if (read_all) {
                                                       if (!strcmp(s, "invert-match")) {
                                                         (yyval.ch) = 0x15;
                                                         free(s);
                                                         s = NULL;
                                                       } else {
                                                         LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                                                         free(s);
                                                         YYABORT;
                                                       }
                                                     }
                                                   }

    break;

  case 181:

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
                        cnt_val = size_arrays->size; /* hack store index of node array */
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYABORT;
                        }
                      }
                    }

    break;

  case 185:

    { if (read_all) {
               if (yang_check_enum(yang_type, actual, &cnt_val, actual_type)) {
                 YYABORT;
               }
               actual = yang_type;
               actual_type = TYPE_KEYWORD;
             } else {
               size_arrays->node[cnt_val].enm++; /* count of enum*/
             }
           }

    break;

  case 186:

    { if (read_all) {
                         yang_type = actual;
                         if (!(actual = yang_read_enum(trg, actual, s))) {
                           YYABORT;
                         }
                         s = NULL;
                         actual_type = 0;
                       }
                     }

    break;

  case 189:

    { if (read_all) {
                              if ((trg->version < 2) && size_arrays->node[size_arrays->next].if_features) {
                                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "enum");
                                YYABORT;
                              }

                              if (size_arrays->node[size_arrays->next].if_features) {
                                ((struct lys_type_enum *)actual)->iffeature = calloc(size_arrays->node[size_arrays->next].if_features,
                                                                                     sizeof *((struct lys_type_enum *)actual)->iffeature);
                                if (!((struct lys_type_enum *)actual)->iffeature) {
                                  LOGMEM;
                                  YYABORT;
                                }
                              }

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

  case 190:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, yang_type, s, unres, ENUM_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].i)].if_features++;
                                     }
                                   }

    break;

  case 191:

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

  case 192:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 193:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 194:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 195:

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

  case 196:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 197:

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

  case 198:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 203:

    { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }

    break;

  case 204:

    { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }

    break;

  case 205:

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

  case 206:

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
                        cnt_val = size_arrays->size; /* hack store index of node array */
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYABORT;
                        }
                      }
                    }

    break;

  case 210:

    { if (read_all) {
                      if (yang_check_bit(yang_type, actual, &cnt_val, actual_type)) {
                        YYABORT;
                      }
                      actual = yang_type;
                    } else {
                      size_arrays->node[cnt_val].bit++; /* count of bit*/
                    }
                  }

    break;

  case 211:

    { if (read_all) {
                                    yang_type = actual;
                                    if (!(actual = yang_read_bit(trg, actual, s))) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                    actual_type = 0;
                                  }
                                }

    break;

  case 214:

    { if (read_all) {
                              if ((trg->version < 2) && size_arrays->node[size_arrays->next].if_features) {
                                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "bit");
                                YYABORT;
                              }

                              if (size_arrays->node[size_arrays->next].if_features) {
                                ((struct lys_type_bit *)actual)->iffeature = calloc(size_arrays->node[size_arrays->next].if_features,
                                                                                     sizeof *((struct lys_type_bit *)actual)->iffeature);
                                if (!((struct lys_type_bit *)actual)->iffeature) {
                                  LOGMEM;
                                  YYABORT;
                                }
                              }

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

  case 215:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, yang_type, s, unres, BIT_KEYWORD)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].i)].if_features++;
                                    }
                                  }

    break;

  case 216:

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

  case 217:

    { if (read_all) {
                                  if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 218:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 219:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 220:

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

  case 221:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 222:

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

  case 227:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
                                                         }
                                                       }

    break;

  case 231:

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
                               if (store_flags((struct lys_node *)(yyval.nodes).grouping, size_arrays->node[size_arrays->next].flags, 0)) {
                                   YYABORT;
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

  case 232:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 233:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 234:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 235:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 236:

    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }

    break;

  case 237:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 238:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 247:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 251:

    { if (read_all) {
                               (yyval.nodes).container = actual;
                               actual_type = CONTAINER_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 (yyval.nodes).container->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).container->iffeature);
                                 if (!(yyval.nodes).container->iffeature) {
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
                               if (store_flags((struct lys_node *)(yyval.nodes).container, size_arrays->node[size_arrays->next].flags, config_inherit)) {
                                   YYABORT;
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

  case 252:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 254:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 255:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 257:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 258:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 259:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 260:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 261:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 262:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 264:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 266:

    { if (read_all) {
                                                 actual = (yyvsp[-1].nodes).container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                               }
                                             }

    break;

  case 268:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 270:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 271:

    { if (read_all) {
                                  if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                                    YYABORT;
                                  }
                                  if ((yyvsp[0].nodes).node.ptr_leaf->dflt && ((yyvsp[0].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE)) {
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

    break;

  case 273:

    { if (read_all) {
                            (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LEAF_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).node.ptr_leaf->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_leaf->iffeature);
                              if (!(yyval.nodes).node.ptr_leaf->iffeature) {
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
                            if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaf, size_arrays->node[size_arrays->next].flags, config_inherit)) {
                                YYABORT;
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

  case 274:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }

    break;

  case 276:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 277:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYABORT;
                       }
                     }

    break;

  case 278:

    { if (read_all) {
                     actual = (yyvsp[-2].nodes).node.ptr_leaf;
                     actual_type = LEAF_KEYWORD;
                     (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                     if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaf) == -1) {
                       lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-2].nodes).node.ptr_leaf->type.der)->name);
                       free((yyvsp[-2].nodes).node.ptr_leaf->type.der);
                       (yyvsp[-2].nodes).node.ptr_leaf->type.der = NULL;
                       YYABORT;
                     }
                   }
                 }

    break;

  case 279:

    { (yyval.nodes) = (yyvsp[-4].nodes);}

    break;

  case 280:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 281:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 283:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;}
                                    s = NULL;
                                  }

    break;

  case 284:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i), 0)) {
                                                 YYABORT;
                                               }
                                             }
                                           }

    break;

  case 285:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 286:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 287:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 288:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 289:

    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYABORT;}
                                                               data_node = actual;
                                                               s=NULL;
                                                             }
                                                           }

    break;

  case 290:

    { if (read_all) {
                                             int i;

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
                                             }
                                             if ((yyvsp[0].nodes).node.ptr_leaflist->dflt_size && (yyvsp[0].nodes).node.ptr_leaflist->min) {
                                               LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "min-elements", "leaf-list");
                                               LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                                                      "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
                                               YYABORT;
                                             }

                                             /* check default value (if not defined, there still could be some restrictions
                                              * that need to be checked against a default value from a derived type) */
                                             for (i = 0; i < (yyvsp[0].nodes).node.ptr_leaflist->dflt_size; i++) {
                                               if (unres_schema_add_str(module, unres, &(yyvsp[0].nodes).node.ptr_leaflist->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_leaflist->dflt[i]) == -1) {
                                                 YYABORT;
                                               }
                                             }
                                           }
                                         }

    break;

  case 292:

    { if (read_all) {
                               (yyval.nodes).node.ptr_leaflist = actual;
                               (yyval.nodes).node.flag = 0;
                               actual_type = LEAF_LIST_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 (yyval.nodes).node.ptr_leaflist->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_leaflist->iffeature);
                                 if (!(yyval.nodes).node.ptr_leaflist->iffeature) {
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
                               if (size_arrays->node[size_arrays->next].dflt) {
                                 if (trg->version < 2) {
                                   LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
                                   YYABORT;
                                 }
                                 (yyval.nodes).node.ptr_leaflist->dflt = calloc(size_arrays->node[size_arrays->next].dflt, sizeof *(yyval.nodes).node.ptr_leaflist->dflt);
                                 if (!(yyval.nodes).node.ptr_leaflist->dflt) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                               if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaflist, size_arrays->node[size_arrays->next].flags, config_inherit)) {
                                   YYABORT;
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

  case 293:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 295:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 296:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYABORT;
                          }
                        }

    break;

  case 297:

    { if (read_all) {
                   actual = (yyvsp[-2].nodes).node.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                   if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER, (struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaflist) == -1) {
                     lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-2].nodes).node.ptr_leaflist->type.der)->name);
                     free((yyvsp[-2].nodes).node.ptr_leaflist->type.der);
                     (yyvsp[-2].nodes).node.ptr_leaflist->type.der = NULL;
                     YYABORT;
                   }
                 }
               }

    break;

  case 298:

    { (yyval.nodes) = (yyvsp[-4].nodes); }

    break;

  case 299:

    { if (read_all) {
                                         int i;

                                         /* check for duplicity */
                                         for (i = 0; i < (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size; i++) {
                                           if (ly_strequal((yyvsp[-1].nodes).node.ptr_leaflist->dflt[i], s, 0)) {
                                             LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "default");
                                             LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", s);
                                             free(s);
                                             YYABORT;
                                           }
                                         }
                                         (yyvsp[-1].nodes).node.ptr_leaflist->dflt[(yyvsp[-1].nodes).node.ptr_leaflist->dflt_size++] = lydict_insert_zc(module->ctx, s);
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].dflt++;
                                       }
                                     }

    break;

  case 300:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 301:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 303:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 304:

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

  case 305:

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

  case 306:

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

  case 307:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 308:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 309:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 310:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 311:

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

  case 313:

    { if (read_all) {
                          (yyval.nodes).node.ptr_list = actual;
                          (yyval.nodes).node.flag = 0;
                          actual_type = LIST_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).node.ptr_list->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_list->iffeature);
                            if (!(yyval.nodes).node.ptr_list->iffeature) {
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
                          (yyval.nodes).node.ptr_list->keys_size = size_arrays->node[size_arrays->next].keys;
                          if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_list, size_arrays->node[size_arrays->next].flags, config_inherit)) {
                              YYABORT;
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

  case 314:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 316:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 317:

    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }

    break;

  case 319:

    { if (read_all) {
                                if ((yyvsp[-1].nodes).node.ptr_list->keys) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "key", "list");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_list->keys = (struct lys_node_leaf **)s;
                                (yyval.nodes) = (yyvsp[-1].nodes);
                                s=NULL;
                              } else {
                                /* list has keys */
                                size_arrays->node[(yyvsp[-1].nodes).index].keys = 1;
                              }
                            }

    break;

  case 320:

    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }

    break;

  case 321:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 322:

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

  case 323:

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

  case 324:

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

  case 325:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 326:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 327:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 328:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }

    break;

  case 330:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 332:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 334:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 335:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 336:

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

  case 339:

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

  case 341:

    { if (read_all) {
                            (yyval.nodes).choice.ptr_choice = actual;
                            (yyval.nodes).choice.s = NULL;
                            actual_type = CHOICE_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).choice.ptr_choice->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).choice.ptr_choice->iffeature);
                              if (!(yyval.nodes).choice.ptr_choice->iffeature) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (store_flags((struct lys_node *)(yyval.nodes).choice.ptr_choice, size_arrays->node[size_arrays->next].flags, config_inherit)) {
                                YYABORT;
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

  case 342:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 343:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 344:

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

  case 345:

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

  case 346:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 347:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 348:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 349:

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

  case 350:

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

  case 351:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 352:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 361:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 365:

    { if (read_all) {
                          (yyval.nodes).cs = actual;
                          actual_type = CASE_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).cs->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).cs->iffeature);
                            if (!(yyval.nodes).cs->iffeature) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (store_flags((struct lys_node *)(yyval.nodes).cs, size_arrays->node[size_arrays->next].flags, 1)) {
                              YYABORT;
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

  case 366:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 368:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 369:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 370:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 371:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 372:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 374:

    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_ANYXML,sizeof(struct lys_node_anydata)))) {YYABORT;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                     actual_type = ANYXML_KEYWORD;
                                                   }

    break;

  case 376:

    { if (read_all) {
                                                         if (!(actual = yang_read_node(trg, actual, s, LYS_ANYDATA, sizeof(struct lys_node_anydata)))) {YYABORT;}
                                                         data_node = actual;
                                                         if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                           data_node = NULL;
                                                         }
                                                         s = NULL;
                                                       }
                                                       actual_type = ANYDATA_KEYWORD;
                                                     }

    break;

  case 380:

    { if (read_all) {
                            (yyval.nodes).node.ptr_anydata = actual;
                            (yyval.nodes).node.flag = actual_type;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).node.ptr_anydata->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_anydata->iffeature);
                              if (!(yyval.nodes).node.ptr_anydata->iffeature) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).node.ptr_anydata->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_anydata->must);
                              if (!(yyval.nodes).node.ptr_anydata->must) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_anydata, size_arrays->node[size_arrays->next].flags, config_inherit)) {
                                YYABORT;
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

  case 381:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                   actual_type = (yyvsp[-1].nodes).node.flag;
                                 }
                               }

    break;

  case 383:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, unres, (yyvsp[-1].nodes).node.flag)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 384:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                   actual_type = (yyvsp[-1].nodes).node.flag;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 386:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config",
                                                               (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 387:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory",
                                                                  (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 388:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status",
                                                               (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 389:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 390:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 391:

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

  case 392:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYABORT;
                        }
                      }
                    }

    break;

  case 395:

    { if (read_all) {
                          (yyval.nodes).uses.ptr_uses = actual;
                          (yyval.nodes).uses.config_inherit = config_inherit;
                          actual_type = USES_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).uses.ptr_uses->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).uses.ptr_uses->iffeature);
                            if (!(yyval.nodes).uses.ptr_uses->iffeature) {
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
                          if (store_flags((struct lys_node *)(yyval.nodes).uses.ptr_uses, size_arrays->node[size_arrays->next].flags, 0)) {
                              YYABORT;
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

  case 396:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 398:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 399:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 400:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 401:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 402:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 404:

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

  case 406:

    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 412:

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
                                  if (size_arrays->node[size_arrays->next].dflt) {
                                    (yyval.nodes).refine->dflt = calloc(size_arrays->node[size_arrays->next].dflt, sizeof *(yyval.nodes).refine->dflt);
                                    if (!(yyval.nodes).refine->dflt) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                    if (size_arrays->node[size_arrays->next].dflt > 1) {
                                      if (trg->version < 2) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "default", "refine");
                                        YYABORT;
                                      }
                                      (yyval.nodes).refine->target_type = LYS_LEAFLIST;
                                    } else {
                                      if ((yyval.nodes).refine->target_type) {
                                        if (trg->version < 2) {
                                          (yyval.nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                        } else {
                                          /* YANG 1.1 */
                                          (yyval.nodes).refine->target_type &= (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE);
                                        }
                                      } else {
                                        if (trg->version < 2) {
                                          (yyval.nodes).refine->target_type = LYS_LEAF | LYS_CHOICE;
                                        } else {
                                          /* YANG 1.1 */
                                          (yyval.nodes).refine->target_type = LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE;
                                        }
                                      }
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

  case 413:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 414:

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

  case 415:

    { if (read_all) {
                                            int i;

                                            (yyvsp[-1].nodes).refine->dflt[(yyvsp[-1].nodes).refine->dflt_size] = lydict_insert_zc(trg->ctx, s);
                                            /* check for duplicity */
                                            for (i = 0; i < (yyvsp[-1].nodes).refine->dflt_size; ++i) {
                                                if (ly_strequal((yyvsp[-1].nodes).refine->dflt[i], (yyvsp[-1].nodes).refine->dflt[(yyvsp[-1].nodes).refine->dflt_size], 1)) {
                                                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, (yyvsp[-1].nodes).refine->dflt[(yyvsp[-1].nodes).refine->dflt_size], "default");
                                                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", (yyvsp[-1].nodes).refine->dflt[(yyvsp[-1].nodes).refine->dflt_size]);
                                                    (yyvsp[-1].nodes).refine->dflt_size++;
                                                    YYABORT;
                                                }
                                            }
                                            (yyvsp[-1].nodes).refine->dflt_size++;
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].dflt++;
                                          }
                                        }

    break;

  case 416:

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

  case 417:

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

  case 418:

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

  case 419:

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

  case 420:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 421:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 422:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYABORT;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 423:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYABORT;
                                          }
                                        }

    break;

  case 427:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYABORT;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 428:

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

  case 430:

    { if (read_all) {
                             (yyval.nodes).node.ptr_augment = actual;
                             (yyval.nodes).node.flag = 0;
                             actual_type = AUGMENT_KEYWORD;
                             if (size_arrays->node[size_arrays->next].if_features) {
                               (yyval.nodes).node.ptr_augment->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_augment->iffeature);
                               if (!(yyval.nodes).node.ptr_augment->iffeature) {
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

  case 431:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 433:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 434:

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

  case 435:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 436:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 437:

    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }

    break;

  case 438:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 439:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                      data_node = actual;
                                    }
                                  }

    break;

  case 440:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 441:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 442:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 445:

    { if (read_all) {
                                                 if (!(actual = yang_read_action(trg, actual, s))) {
                                                   YYABORT;
                                                 }
                                                 data_node = actual;
                                                 s = NULL;
                                               }
                                               config_inherit = DISABLE_INHERIT;
                                             }

    break;

  case 446:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 447:

    { if (read_all) {
                                                 if (!(actual = yang_read_node(trg, NULL, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                                   YYABORT;
                                                 }
                                                 data_node = actual;
                                                 s = NULL;
                                               }
                                               config_inherit = DISABLE_INHERIT;
                                             }

    break;

  case 448:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 451:

    { if (read_all) {
                         (yyval.nodes).node.ptr_rpc = actual;
                         (yyval.nodes).node.flag = 0;
                         actual_type = RPC_KEYWORD;
                         if (size_arrays->node[size_arrays->next].if_features) {
                           (yyval.nodes).node.ptr_rpc->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_rpc->iffeature);
                           if (!(yyval.nodes).node.ptr_rpc->iffeature) {
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
                         if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_rpc, size_arrays->node[size_arrays->next].flags, 0)) {
                             YYABORT;
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

  case 452:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 453:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 454:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 455:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 456:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 458:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 460:

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

  case 461:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 462:

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

  case 463:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 464:

    { if (read_all) {
                                     s = strdup("input");
                                     if (!s) {
                                       LOGMEM;
                                       YYABORT;
                                     }
                                     if (!(actual = yang_read_node(trg, actual, s, LYS_INPUT, sizeof(struct lys_node_inout)))) {
                                      YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                   }
                                 }

    break;

  case 465:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYABORT;
                                        }
                                      }

    break;

  case 467:

    { if (read_all) {
                                  (yyval.nodes).node.ptr_inout = actual;
                                  (yyval.nodes).node.flag = 0;
                                  actual_type = INPUT_KEYWORD;
                                  if (trg->version < 2 && size_arrays->node[size_arrays->next].must) {
                                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "must");
                                    YYABORT;
                                  }
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).node.ptr_inout->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_inout->tpdf);
                                    if (!(yyval.nodes).node.ptr_inout->tpdf) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].must) {
                                    (yyval.nodes).node.ptr_inout->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_inout->must);
                                    if (!(yyval.nodes).node.ptr_inout->must) {
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

  case 468:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).node.ptr_inout;
                                         actual_type = INPUT_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                       }
                                     }

    break;

  case 470:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 472:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 474:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 475:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 476:

    { if (read_all) {
                                       s = strdup("output");
                                       if (!s) {
                                         LOGMEM;
                                         YYABORT;
                                       }
                                       if (!(actual = yang_read_node(trg, actual, s, LYS_OUTPUT, sizeof(struct lys_node_inout)))) {
                                        YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                     }
                                   }

    break;

  case 477:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYABORT;
                                         }
                                       }

    break;

  case 479:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 480:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 481:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 483:

    { if (read_all) {
                                  (yyval.nodes).notif = actual;
                                  actual_type = NOTIFICATION_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].if_features) {
                                    (yyval.nodes).notif->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).notif->iffeature);
                                    if (!(yyval.nodes).notif->iffeature) {
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
                                  if (trg->version < 2 && size_arrays->node[size_arrays->next].must) {
                                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "must");
                                    YYABORT;
                                  }
                                  if (size_arrays->node[size_arrays->next].must) {
                                    (yyval.nodes).notif->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).notif->must);
                                    if (!(yyval.nodes).notif->must) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }
                                  if (store_flags((struct lys_node *)(yyval.nodes).notif, size_arrays->node[size_arrays->next].flags, 0)) {
                                      YYABORT;
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

  case 484:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).notif;
                                      actual_type = NOTIFICATION_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 486:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 487:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 488:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 489:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 490:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 492:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 494:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 496:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 497:

    { if (read_all) {
                                            if (actual_type == DEVIATION_KEYWORD) {
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                                              YYABORT;
                                            }
                                            if (yang_check_deviation(trg, (yyvsp[0].nodes).deviation->dflt_check, unres)) {
                                              YYABORT;
                                            }
                                            ly_set_free((yyvsp[0].nodes).deviation->dflt_check);
                                            free((yyvsp[0].nodes).deviation);
                                          }
                                        }

    break;

  case 499:

    { if (read_all) {
                               (yyval.nodes).deviation = actual;
                               actual_type = DEVIATION_KEYWORD;
                               if (size_arrays->node[size_arrays->next].deviate) {
                                 (yyval.nodes).deviation->deviation->deviate = calloc(size_arrays->node[size_arrays->next].deviate, sizeof *(yyval.nodes).deviation->deviation->deviate);
                                 if (!(yyval.nodes).deviation->deviation->deviate) {
                                   LOGMEM;
                                   ly_set_free((yyval.nodes).deviation->dflt_check);
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

  case 500:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 501:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 502:

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

  case 505:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 511:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 515:

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
                                  if (size_arrays->node[size_arrays->next].dflt) {
                                    if (yang_read_deviate_default(trg, actual, size_arrays->node[size_arrays->next].dflt)) {
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

  case 516:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 517:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 518:

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

  case 519:

    { if (read_all) {
                                           if (yang_fill_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].dflt++;
                                         }
                                       }

    break;

  case 520:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 521:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 522:

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

  case 523:

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

  case 524:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 527:

    { if (read_all) {
              struct lys_node_leaflist *llist;
              int i,j;

              if ((yyvsp[-1].nodes).deviation->deviate->dflt_size && (yyvsp[-1].nodes).deviation->target->nodetype == LYS_LEAFLIST) {
                /* consolidate the final list in the target after removing items from it */
                llist = (struct lys_node_leaflist *)(yyvsp[-1].nodes).deviation->target;
                for (i = j = 0; j < llist->dflt_size; j++) {
                  llist->dflt[i] = llist->dflt[j];
                  if (llist->dflt[i]) {
                    i++;
                  }
                }
                llist->dflt_size = i + 1;
              }
            }
          }

    break;

  case 528:

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
                                    if (size_arrays->node[size_arrays->next].dflt) {
                                      if (yang_read_deviate_default(trg, actual, size_arrays->node[size_arrays->next].dflt)) {
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

  case 529:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 530:

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

  case 531:

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

  case 532:

    { if (read_all) {
                                              if (yang_fill_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            } else {
                                              size_arrays->node[(yyvsp[-1].nodes).index].dflt++;
                                            }
                                          }

    break;

  case 533:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 537:

    { if (read_all) {
                                         (yyval.nodes).deviation = actual;
                                         actual_type = REPLACE_KEYWORD;
                                         if (size_arrays->node[size_arrays->next].dflt) {
                                           if (yang_read_deviate_default(trg, actual, size_arrays->node[size_arrays->next].dflt)) {
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

  case 538:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target) == -1) {
                                              lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-2].nodes).deviation->deviate->type->der)->name);
                                              free((yyvsp[-2].nodes).deviation->deviate->type->der);
                                              (yyvsp[-2].nodes).deviation->deviate->type->der = NULL;
                                              YYABORT;
                                            }
                                            ly_set_add((yyvsp[-2].nodes).deviation->dflt_check, (yyvsp[-2].nodes).deviation->target, 0);
                                          }
                                        }

    break;

  case 539:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 540:

    { if (read_all) {
                                               if (yang_fill_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                 YYABORT;
                                               }
                                               s = NULL;
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].dflt++;
                                             }
                                           }

    break;

  case 541:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 542:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 543:

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

  case 544:

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

  case 545:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 550:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 551:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 552:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 553:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 554:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 555:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 556:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 557:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 558:

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

  case 559:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 560:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 561:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 562:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 563:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 564:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 565:

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

  case 567:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 568:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 569:

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

  case 570:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 571:

    { (yyval.uint) = 0; }

    break;

  case 572:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 573:

    { if (read_all) {
                  if (!strcmp(s, "unbounded")) {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(s, &endptr, 10);
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

  case 574:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 575:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 576:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 577:

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

  case 578:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 588:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 591:

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

  case 594:

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

  case 595:

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

  case 599:

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

  case 601:

    { tmp_s = yyget_text(scanner); }

    break;

  case 602:

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

  case 603:

    { tmp_s = yyget_text(scanner); }

    break;

  case 604:

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

  case 628:

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

  case 629:

    { (yyval.uint) = 0; }

    break;

  case 630:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 631:

    { (yyval.i) = 0; }

    break;

  case 632:

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

  case 638:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 643:

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

  case 649:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 672:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 763:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 764:

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
