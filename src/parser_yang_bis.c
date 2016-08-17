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
    MODIFIER_KEYWORD = 352
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
#define YYLAST   3170

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  108
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  342
/* YYNRULES -- Number of rules.  */
#define YYNRULES  759
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1221

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   352

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
     106,   107,     2,    98,     2,     2,     2,   102,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   101,
       2,   105,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   103,     2,   104,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    99,     2,   100,     2,     2,     2,     2,
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
      95,    96,    97
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   254,   254,   255,   276,   276,   295,   297,   296,   328,
     341,   328,   350,   351,   359,   360,   363,   376,   363,   387,
     388,   394,   394,   403,   405,   407,   430,   431,   434,   434,
     454,   455,   461,   472,   483,   492,   494,   494,   517,   518,
     522,   523,   534,   545,   554,   566,   577,   566,   586,   588,
     589,   590,   591,   596,   602,   604,   606,   608,   610,   618,
     621,   621,   630,   631,   631,   636,   637,   642,   649,   649,
     658,   664,   705,   708,   709,   710,   711,   712,   713,   714,
     720,   721,   722,   724,   724,   743,   744,   748,   749,   756,
     763,   774,   786,   786,   788,   789,   793,   795,   796,   797,
     808,   810,   811,   810,   814,   815,   816,   817,   834,   834,
     843,   844,   849,   868,   881,   887,   892,   898,   900,   900,
     909,   910,   915,   955,   964,   973,   979,   984,   990,   994,
     992,  1009,  1019,  1019,  1026,  1029,  1029,  1044,  1045,  1051,
    1057,  1062,  1068,  1069,  1075,  1080,  1081,  1082,  1089,  1097,
    1102,  1106,  1136,  1137,  1141,  1142,  1143,  1151,  1164,  1170,
    1171,  1189,  1193,  1203,  1204,  1209,  1221,  1226,  1231,  1236,
    1242,  1251,  1264,  1265,  1269,  1270,  1282,  1287,  1292,  1297,
    1303,  1316,  1316,  1335,  1336,  1339,  1351,  1361,  1362,  1367,
    1391,  1400,  1409,  1415,  1420,  1426,  1438,  1439,  1457,  1462,
    1463,  1468,  1470,  1472,  1476,  1480,  1495,  1495,  1514,  1515,
    1517,  1528,  1538,  1539,  1544,  1568,  1577,  1586,  1592,  1597,
    1603,  1615,  1616,  1631,  1633,  1635,  1637,  1639,  1639,  1646,
    1647,  1652,  1672,  1678,  1683,  1688,  1689,  1696,  1697,  1700,
    1701,  1702,  1703,  1704,  1705,  1706,  1709,  1709,  1717,  1718,
    1723,  1757,  1757,  1759,  1766,  1766,  1774,  1775,  1781,  1787,
    1792,  1797,  1797,  1802,  1802,  1807,  1807,  1815,  1815,  1822,
    1829,  1822,  1847,  1875,  1875,  1877,  1884,  1889,  1884,  1899,
    1900,  1900,  1908,  1911,  1917,  1923,  1929,  1934,  1940,  1947,
    1940,  1982,  2021,  2021,  2023,  2030,  2035,  2030,  2045,  2062,
    2063,  2063,  2071,  2077,  2091,  2105,  2117,  2123,  2128,  2134,
    2141,  2134,  2166,  2209,  2209,  2211,  2218,  2218,  2226,  2239,
    2247,  2253,  2267,  2281,  2293,  2299,  2304,  2309,  2309,  2317,
    2317,  2322,  2322,  2327,  2327,  2335,  2335,  2346,  2348,  2347,
    2365,  2386,  2386,  2388,  2401,  2413,  2421,  2429,  2437,  2446,
    2455,  2455,  2465,  2466,  2469,  2470,  2471,  2472,  2473,  2476,
    2476,  2484,  2485,  2489,  2509,  2509,  2511,  2518,  2524,  2529,
    2534,  2534,  2541,  2541,  2552,  2553,  2557,  2584,  2584,  2586,
    2593,  2593,  2601,  2607,  2613,  2619,  2624,  2630,  2630,  2646,
    2647,  2651,  2686,  2686,  2688,  2695,  2701,  2706,  2711,  2711,
    2719,  2719,  2734,  2734,  2743,  2744,  2749,  2750,  2753,  2803,
    2810,  2834,  2854,  2873,  2892,  2915,  2938,  2943,  2949,  2958,
    2949,  2965,  2966,  2969,  2978,  2969,  2990,  3011,  3011,  3013,
    3020,  3029,  3034,  3039,  3039,  3047,  3047,  3055,  3055,  3065,
    3066,  3069,  3069,  3080,  3080,  3091,  3092,  3097,  3125,  3132,
    3138,  3143,  3148,  3148,  3156,  3156,  3161,  3161,  3173,  3173,
    3186,  3200,  3186,  3207,  3238,  3238,  3246,  3246,  3254,  3254,
    3259,  3259,  3269,  3283,  3269,  3290,  3290,  3300,  3304,  3309,
    3347,  3347,  3355,  3362,  3368,  3373,  3378,  3378,  3386,  3386,
    3391,  3391,  3398,  3407,  3398,  3421,  3441,  3449,  3457,  3467,
    3468,  3470,  3475,  3477,  3478,  3479,  3482,  3484,  3484,  3490,
    3491,  3495,  3522,  3530,  3538,  3554,  3564,  3571,  3578,  3589,
    3601,  3601,  3607,  3608,  3628,  3655,  3663,  3674,  3684,  3695,
    3695,  3701,  3702,  3706,  3723,  3730,  3738,  3748,  3755,  3762,
    3773,  3785,  3785,  3788,  3789,  3793,  3794,  3799,  3805,  3807,
    3808,  3807,  3811,  3812,  3813,  3828,  3830,  3831,  3830,  3834,
    3835,  3836,  3851,  3853,  3855,  3856,  3877,  3879,  3880,  3881,
    3902,  3904,  3905,  3906,  3918,  3918,  3926,  3927,  3932,  3934,
    3935,  3937,  3938,  3940,  3942,  3942,  3951,  3954,  3954,  3965,
    3968,  3978,  3999,  4001,  4002,  4005,  4005,  4024,  4024,  4033,
    4033,  4042,  4045,  4047,  4049,  4050,  4052,  4054,  4056,  4057,
    4059,  4061,  4062,  4064,  4065,  4067,  4069,  4072,  4076,  4078,
    4079,  4081,  4082,  4084,  4086,  4097,  4098,  4101,  4102,  4114,
    4115,  4117,  4118,  4120,  4121,  4127,  4128,  4131,  4132,  4133,
    4159,  4160,  4163,  4164,  4165,  4168,  4168,  4176,  4178,  4179,
    4181,  4182,  4183,  4185,  4186,  4188,  4189,  4191,  4192,  4194,
    4195,  4197,  4198,  4201,  4202,  4205,  4207,  4208,  4211,  4211,
    4220,  4222,  4223,  4224,  4225,  4226,  4227,  4228,  4230,  4231,
    4232,  4233,  4234,  4235,  4236,  4237,  4238,  4239,  4240,  4241,
    4242,  4243,  4244,  4245,  4246,  4247,  4248,  4249,  4250,  4251,
    4252,  4253,  4254,  4255,  4256,  4257,  4258,  4259,  4260,  4261,
    4262,  4263,  4264,  4265,  4266,  4267,  4268,  4269,  4270,  4271,
    4272,  4273,  4274,  4275,  4276,  4277,  4278,  4279,  4280,  4281,
    4282,  4283,  4284,  4285,  4286,  4287,  4288,  4289,  4290,  4291,
    4292,  4293,  4294,  4295,  4296,  4297,  4298,  4299,  4300,  4301,
    4302,  4303,  4304,  4305,  4306,  4307,  4308,  4309,  4312,  4321
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
  "USER_KEYWORD", "ACTION_KEYWORD", "MODIFIER_KEYWORD", "'+'", "'{'",
  "'}'", "';'", "'/'", "'['", "']'", "'='", "'('", "')'", "$accept",
  "start", "string_1", "$@1", "string_2", "$@2", "module_stmt", "$@3",
  "$@4", "module_header_stmts", "submodule_stmt", "$@5", "$@6",
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
  "$@61", "anyxml_end", "anyxml_opt_stmt", "$@62", "$@63", "uses_stmt",
  "$@64", "uses_end", "uses_opt_stmt", "$@65", "$@66", "$@67",
  "refine_stmt", "$@68", "refine_end", "refine_arg_str",
  "refine_body_opt_stmts", "uses_augment_stmt", "$@69", "$@70",
  "uses_augment_arg_str", "augment_stmt", "$@71", "$@72",
  "augment_opt_stmt", "$@73", "$@74", "$@75", "$@76", "augment_arg_str",
  "action_stmt", "$@77", "rpc_stmt", "$@78", "rpc_end", "rpc_opt_stmt",
  "$@79", "$@80", "$@81", "$@82", "input_stmt", "$@83", "$@84",
  "input_output_opt_stmt", "$@85", "$@86", "$@87", "$@88", "output_stmt",
  "$@89", "$@90", "notification_stmt", "$@91", "notification_end",
  "notification_opt_stmt", "$@92", "$@93", "$@94", "$@95",
  "deviation_stmt", "$@96", "$@97", "deviation_opt_stmt",
  "deviation_arg_str", "deviate_body_stmt", "deviate_stmts",
  "deviate_not_supported_stmt", "deviate_add_stmt", "$@98",
  "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt", "$@99",
  "deviate_delete_end", "deviate_delete_opt_stmt", "deviate_replace_stmt",
  "$@100", "deviate_replace_end", "deviate_replace_opt_stmt", "when_stmt",
  "$@101", "when_end", "when_opt_stmt", "config_stmt", "config_read_stmt",
  "$@102", "$@103", "config_arg_str", "mandatory_stmt",
  "mandatory_read_stmt", "$@104", "$@105", "mandatory_arg_str",
  "presence_stmt", "min_elements_stmt", "min_value_arg_str",
  "max_elements_stmt", "max_value_arg_str", "ordered_by_stmt",
  "ordered_by_arg_str", "must_stmt", "$@106", "must_end", "unique_stmt",
  "unique_arg_str", "unique_arg", "key_stmt", "key_arg_str", "$@107",
  "key_opt", "$@108", "range_arg_str", "absolute_schema_nodeid",
  "absolute_schema_nodeids", "absolute_schema_nodeid_opt",
  "descendant_schema_nodeid", "$@109", "path_arg_str", "$@110", "$@111",
  "absolute_path", "absolute_paths", "absolute_path_opt", "relative_path",
  "relative_path_part1", "relative_path_part1_opt", "descendant_path",
  "descendant_path_opt", "path_predicate", "path_equality_expr",
  "path_key_expr", "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@112", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@113", "strings", "identifier", "identifiers",
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
     345,   346,   347,   348,   349,   350,   351,   352,    43,   123,
     125,    59,    47,    91,    93,    61,    40,    41
};
# endif

#define YYPACT_NINF -925

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-925)))

#define YYTABLE_NINF -614

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -925,    25,  -925,  -925,   117,  -925,  -925,  -925,   102,   102,
    -925,  -925,  2979,  2979,   102,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,   -61,   102,    27,   102,  -925,  -925,    54,   448,
     448,  -925,  -925,   153,  -925,  -925,    28,   477,   102,   102,
     102,   102,  -925,  -925,  -925,  -925,  -925,   151,  -925,  -925,
     -31,  2227,  -925,  2321,  2979,  2321,    10,    10,   102,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,   220,  -925,  -925,   220,  -925,
     220,   102,   102,  -925,  -925,   337,   337,  2979,   102,  2415,
     102,  -925,  -925,  -925,  -925,  -925,   102,  -925,  2979,  2979,
     102,   102,   102,   102,  -925,  -925,  -925,  -925,   172,   172,
    -925,  -925,   102,   112,  -925,   152,    60,   448,   102,  -925,
    -925,  -925,  2321,  2321,  2321,  2321,   102,  -925,  1503,  1559,
     155,   238,  -925,  -925,  -925,   165,   291,   220,   220,   220,
     220,   100,   448,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,   512,   448,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  2979,    33,  2979,  2979,
    2979,    33,  2979,  2979,  2979,  2979,  2979,  2979,  2979,  2979,
    2979,  2509,   102,   448,   102,   199,  2415,   102,  -925,   448,
     448,   448,  -925,   336,  -925,  3073,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,   171,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,   196,   102,   264,   367,   102,  -925,  -925,  -925,
     355,  -925,   190,   204,   102,   422,   438,   465,   231,   102,
     472,   483,   487,   235,   239,   244,   500,   504,  -925,   513,
     102,   102,   212,  -925,   102,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,   448,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,   448,  -925,
    -925,  -925,  -925,   102,   100,   448,  -925,   448,   448,   448,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   448,
     448,   227,   448,   448,   220,   116,   856,  1309,   579,   967,
     108,   315,   280,   402,   214,  1633,  1616,  1064,   645,   455,
     102,   102,   102,  -925,  -925,  -925,   226,   258,  -925,  -925,
     594,  -925,  -925,  -925,  -925,   102,   102,   102,   102,   102,
     102,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
     102,   102,  -925,  -925,  -925,  -925,  -925,  -925,   248,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
     249,  -925,  -925,  -925,  -925,  -925,  -925,  -925,   102,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,   102,  -925,  -925,  -925,  -925,  -925,   102,  -925,
    -925,   254,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,   102,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,   266,   258,
    -925,  -925,  -925,  -925,   102,   102,   102,  -925,  -925,  -925,
    -925,  -925,  -925,   278,   258,  -925,  -925,  -925,  -925,  -925,
    -925,   102,   102,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
     289,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    2321,    51,  2321,  -925,   102,  -925,   102,   102,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  2321,  -925,  2321,
    -925,  2321,  -925,  -925,  2979,  2979,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  2321,  -925,  -925,  -925,  -925,  -925,
    -925,  2979,   302,  -925,   448,   448,   448,   448,  2509,  -925,
    -925,  -925,  -925,    12,   182,     3,  -925,  -925,  -925,  -925,
    2603,  2603,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,   102,   102,  -925,  -925,  -925,  -925,   220,
    -925,  -925,  -925,  -925,   220,   220,  2509,   448,  2603,  2603,
    -925,  -925,  -925,    52,   220,    58,  -925,    51,  -925,   448,
     448,  -925,  -925,   448,   448,   448,   448,   448,   448,   220,
     448,   448,   448,   448,   448,   448,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,   220,  -925,   448,
     448,  -925,  -925,  -925,   220,  -925,  -925,  -925,   220,  -925,
    -925,  -925,  -925,  -925,   220,  -925,   448,   448,   220,  -925,
    -925,   220,  -925,    43,  -925,   448,   448,   448,   448,   448,
     448,   448,   448,   448,   448,   259,   312,   448,   448,   448,
     448,  -925,   102,   102,   102,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,   448,   448,   448,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,   517,  -925,   518,   536,
     504,  -925,   543,   102,   102,    43,   102,   102,  -925,   448,
     102,  -925,   102,  -925,   102,   102,   102,  -925,   448,  -925,
      43,  -925,  -925,  -925,  3073,  -925,  -925,  -925,   544,   330,
     102,   552,   102,   102,   102,   220,   102,   102,   220,  -925,
    -925,  -925,   220,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,   564,   565,  -925,   583,  -925,  -925,  3073,
      43,   204,   448,   448,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,   448,  -925,   448,   448,   156,   448,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
     973,   973,   469,   448,   448,   448,   213,   217,   294,   102,
     301,   448,   448,   448,    43,  -925,  -925,  -925,   339,  -925,
     358,   102,   102,  -925,   368,   421,  -925,   404,  -925,  -925,
    -925,   449,  1309,   332,   102,   102,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,    61,  -925,   706,   118,   162,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,   129,    94,  -925,   102,   102,   102,   102,
     258,  -925,  -925,  -925,  -925,   102,  -925,   102,  -925,   379,
     102,   102,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  2321,  2321,  -925,  -925,  -925,  -925,  -925,   220,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,   448,   448,   448,   448,  -925,   220,   378,   474,
    -925,  -925,  -925,   220,   103,  2321,  2321,  2321,  -925,   448,
    2321,   448,  2979,   449,  -925,    52,    58,   448,   220,   220,
     448,   448,   102,   102,  -925,   448,   448,   448,  -925,  3073,
    -925,  -925,   397,  -925,  -925,   102,   102,  -925,  -925,   220,
    -925,   587,  -925,   597,  -925,   604,  -925,   448,   621,  -925,
     404,   624,  -925,  -925,   220,   220,  -925,  -925,  -925,   378,
    -925,  2697,  -925,   102,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,   400,  -925,  -925,  -925,   -22,   448,   448,   448,
     448,   448,   448,   448,   448,   448,   501,  -925,    83,   343,
     218,   374,   224,   138,  -925,  3073,  -925,  -925,   102,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,   102,  -925,  -925,  -925,
    -925,  -925,  -925,   102,  -925,  -925,  -925,  -925,  -925,  -925,
     501,   501,  2321,   573,   182,   403,   407,   220,  -925,  -925,
    -925,  -925,   220,  -925,  -925,  -925,   220,  -925,  -925,   501,
    -925,  -925,   102,  -925,   102,   447,   501,  -925,   501,   427,
     444,   501,   501,   433,   537,  -925,   501,  -925,  -925,   461,
    2791,   501,  -925,  -925,  -925,  2885,  -925,   468,   501,  3073,
    -925
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     663,     0,     2,     3,     0,     1,   661,   662,     0,     0,
     664,   663,     0,     0,   665,   679,     4,   678,   680,   681,
     682,   683,   684,   685,   686,   687,   688,   689,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   699,   700,   701,
     702,   703,   704,   705,   706,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   717,   718,   719,   720,   721,
     722,   723,   724,   725,   726,   727,   728,   729,   730,   731,
     732,   733,   734,   735,   736,   737,   738,   739,   740,   741,
     742,   743,   744,   745,   746,   747,   748,   749,   750,   751,
     752,   753,   754,   755,   756,   757,   634,     9,   758,   663,
      16,   663,     0,   633,     0,     6,   642,   642,     5,    12,
      19,   663,   645,    10,   644,   643,    17,     0,   648,     0,
       0,     0,    25,    13,    14,    15,    25,     0,    20,     7,
       0,   650,   649,     0,     0,     0,    49,    49,     0,    22,
     663,   663,   655,   646,   663,   671,   674,   672,   676,   677,
     675,   647,   651,   673,   670,     0,   668,   631,     0,   663,
       0,     0,     0,    26,    27,    58,    58,     0,     8,   657,
     653,   642,   642,    24,   663,    48,   632,    23,     0,     0,
       0,     0,     0,     0,    50,    51,    52,    53,    71,    71,
      45,   636,   648,     0,   635,   652,     0,   640,   669,    28,
      35,    36,     0,     0,     0,     0,     0,   642,     0,     0,
       0,     0,   656,   663,   642,     0,     0,     0,     0,     0,
       0,     0,    59,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   663,
     642,    73,    74,    75,    76,    77,    78,   239,   240,   241,
     242,   243,   244,   245,    79,    80,    81,    82,   663,   642,
     663,   663,   658,     0,   641,   642,   642,    38,   642,    55,
      56,    54,    57,    68,    70,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    72,    18,     0,   657,   659,   663,    30,
      40,    37,   663,     0,   372,     0,   440,   423,   593,   663,
     335,   246,    83,   500,   492,   663,   108,   227,   118,   269,
     288,   309,   475,   443,     0,   131,   759,   639,   387,   663,
     663,    46,     0,   654,     0,     0,    69,   642,    62,    61,
       0,   591,     0,   592,   439,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,     0,   642,     0,
     637,   638,     0,   663,     0,   642,    34,    31,    32,    33,
      39,    43,    41,    42,    63,   642,   374,   373,   642,   594,
     642,   337,   336,   642,   248,   247,   642,    85,    84,   642,
     642,   110,   109,   642,   229,   228,   642,   120,   119,   642,
     642,   642,   642,   477,   476,   642,   445,   444,   134,   642,
     389,   388,   642,   660,     0,    29,    65,   376,   426,   340,
     250,    87,   495,   112,   231,   122,   272,   291,   312,   479,
     447,   129,   391,    47,     0,     0,     0,   424,   338,     0,
       0,   493,     0,     0,     0,   270,   289,   310,     0,     0,
       0,     0,     0,   140,   141,   139,     0,     0,   137,   138,
       0,    44,    64,    66,    67,     0,     0,     0,     0,     0,
       0,   375,   385,   386,   384,   379,   377,   382,   383,   380,
       0,     0,   431,   432,   430,   429,   433,   437,     0,   435,
     427,   348,   349,   347,   343,   344,   354,   355,   356,   357,
       0,   350,   352,   353,   358,   341,   345,   346,     0,   249,
     259,   260,   258,   253,   265,   261,   267,   263,   251,   257,
     256,   254,     0,    86,    90,    91,    88,    89,     0,   496,
     497,     0,   111,   115,   116,   114,   113,   230,   233,   234,
     232,   642,   642,   642,   642,     0,   121,   126,   127,   125,
     124,   123,   286,   287,   285,   275,   279,   282,     0,     0,
     273,   283,   284,   280,     0,     0,     0,   307,   308,   306,
     294,   299,   298,     0,     0,   292,   302,   303,   304,   305,
     300,     0,     0,   325,   326,   324,   315,   327,   329,   333,
       0,   331,   313,   320,   321,   322,   323,   316,   319,   318,
     478,   484,   485,   483,   482,   486,   488,   490,   480,   663,
     663,   446,   450,   451,   449,   448,   452,   454,   456,   458,
       0,     0,     0,   130,     0,   642,     0,     0,   390,   396,
     397,   395,   394,   398,   400,   392,   549,     0,   556,     0,
     101,     0,   642,   642,     0,     0,   642,   642,   425,   642,
     642,   339,   642,   642,     0,   642,   642,   642,   642,   642,
     642,     0,     0,   494,   236,   235,   237,   238,     0,   271,
     277,   642,   642,     0,     0,     0,   290,   296,   642,   642,
       0,     0,   642,   642,   642,   311,   642,   642,   642,   642,
     642,   642,   642,   460,   472,   642,   642,   642,   642,     0,
     663,   663,   663,   107,     0,     0,     0,   136,     0,     0,
     642,   642,   642,     0,     0,     0,   574,     0,   541,   378,
     381,   359,   441,   434,   438,   436,   428,   351,   342,     0,
     266,   262,   268,   264,   252,   255,    92,   663,   663,   663,
     663,   498,   663,   501,   503,   505,   504,     0,   642,   274,
     281,   624,   663,   569,     0,   663,   625,   565,     0,   626,
     663,   663,   663,   573,     0,   642,   293,   301,     0,   583,
     584,     0,   578,     0,   595,   328,   330,   334,   332,   314,
     317,   487,   489,   491,   481,     0,     0,   453,   455,   457,
     459,   226,   104,   106,   105,   100,   225,   132,   422,   418,
     663,   407,   402,   663,   399,   401,   393,   663,   663,   554,
     550,   117,   663,   663,   561,   557,     0,   102,     0,     0,
       0,   562,     0,   507,   520,     0,   529,   502,   128,   278,
     567,   566,   568,   563,   564,   572,   571,   570,   297,   586,
       0,   580,   579,   582,     0,   593,   642,   642,     0,     0,
     421,     0,   406,   553,   552,     0,   560,   559,     0,   642,
     576,   575,     0,   642,   543,   542,   642,   361,   360,   442,
     642,    94,   642,     0,     0,   506,     0,   585,   589,     0,
       0,   596,   463,   463,   642,   142,   133,   642,   642,   404,
     403,   551,   558,   165,   103,   545,   363,     0,    93,   642,
     509,   508,   642,   522,   521,   642,   531,   530,   587,   581,
     461,   473,   151,   144,   426,   408,     0,     0,     0,     0,
       0,   511,   524,   533,     0,   466,   468,   470,     0,   464,
       0,     0,     0,   148,     0,   145,   146,     0,   147,   149,
     150,     0,   419,     0,     0,     0,   577,   168,   169,   166,
     167,   544,   546,   547,   362,   368,   369,   367,   366,   370,
     364,     0,    95,     0,     0,     0,   588,   642,   642,   642,
     462,   642,   474,   597,     0,   143,     0,     0,     0,     0,
       0,   154,   152,   153,   642,     0,   642,     0,   208,     0,
       0,     0,   405,   416,   417,   411,   412,   413,   410,   414,
     415,   642,     0,     0,   642,   642,   663,   663,    99,     0,
     510,   512,   515,   516,   517,   518,   519,   642,   514,   523,
     525,   528,   642,   527,   532,   642,   535,   536,   537,   538,
     539,   540,   467,   469,   471,   465,   601,     0,     0,     0,
     663,   663,   205,     0,     0,     0,     0,     0,   642,   155,
       0,   183,     0,   207,   420,     0,     0,   409,     0,     0,
     371,   365,    98,    97,    96,   513,   526,   534,   201,     0,
     604,   598,     0,   600,   608,   204,   203,   202,   160,     0,
     663,     0,   162,     0,   171,     0,   590,   156,     0,   186,
     182,     0,   211,   209,     0,     0,   224,   223,   613,   603,
     607,     0,   158,   159,   642,   163,   642,   642,   172,   642,
     642,   199,   198,   642,   187,   185,   642,   642,   212,   642,
     548,   555,   602,   605,   609,   606,   611,   165,   161,   174,
     170,   165,   189,   184,   214,   210,   666,   610,     0,     0,
       0,     0,     0,     0,   667,     0,   612,   164,     0,   173,
     178,   179,   175,   176,   177,   200,     0,   188,   193,   194,
     192,   190,   191,     0,   213,   218,   219,   217,   215,   216,
     666,   666,     0,     0,     0,     0,     0,     0,   629,   630,
     627,   197,     0,   663,   628,   222,     0,   663,   614,   666,
     180,   195,   196,   220,   221,     0,   666,   615,   666,     0,
       0,   666,   666,     0,     0,   623,   666,   616,   619,     0,
       0,   666,   620,   621,   618,   666,   617,     0,   666,     0,
     622
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -925,  -925,   574,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,   441,  -925,   476,  -925,  -925,
    -925,   396,  -925,  -925,  -925,  -925,   252,  -925,  -925,  -925,
    -262,   470,  -925,  -925,   981,  1020,   432,  -925,  -925,  -925,
    -925,  -925,   233,  -925,   439,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -428,  -360,  -925,  -925,   -86,
    -925,  -925,  -925,  -925,  -402,  -925,  -925,  -925,  -925,  -280,
    -421,  -925,  -925,  -535,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -924,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -452,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -405,  -925,  -925,  -925,  -925,  -925,  -472,  -463,  -415,  -429,
    -316,  -925,  -925,  -925,  -318,   240,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,   245,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,   253,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,   261,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
     272,  -925,  -925,  -925,  -925,  -925,   275,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -212,  -925,  -925,  -925,  -925,  -925,  -140,  -925,  -925,
    -925,  -104,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -164,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,  -925,
    -925,  -925,  -925,  -925,  -925,  -925,  -342,  -925,  -925,  -925,
    -409,   234,  -925,  -925,  -329,  -352,  -283,  -925,  -925,  -328,
    -216,  -398,  -925,  -391,  -925,   282,  -925,  -384,  -925,  -925,
    -703,  -925,  -150,  -925,  -925,  -925,  -193,  -925,  -925,  -337,
     454,  -108,  -694,  -925,  -925,  -925,  -925,  -361,  -399,  -925,
    -925,  -358,  -925,  -925,  -925,  -382,  -925,  -925,  -925,  -464,
    -925,  -925,  -925,  -665,  -427,  -925,  -925,  -925,   -11,  -137,
    -633,   546,  1113,  -925,  -925,   556,  -925,  -925,  -925,  -925,
     453,  -925,    -4,   142,   189,    63,  -128,  -925,   619,  -112,
    -121,  -925
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    96,   101,   108,   140,     2,   102,   122,   113,
       3,   104,   126,   116,   127,   123,   124,   136,   163,   215,
     334,   199,   164,   216,   268,   335,   366,   139,   210,   362,
     125,   165,   184,   185,   947,   948,   188,   207,   303,   339,
     416,   435,   275,   302,   208,   240,   241,   347,   388,   440,
     526,   822,   872,   920,  1009,   484,   474,   717,   862,   704,
     242,   350,   392,   442,   485,   243,   352,   398,   444,   551,
     244,   456,   324,   625,   848,   431,   457,   886,   912,   934,
     935,   980,   981,  1079,   982,  1081,  1106,   916,   983,  1083,
    1109,  1140,  1152,   936,   937,  1090,   986,  1088,  1115,  1142,
    1162,  1182,   984,  1112,   938,   939,  1043,   940,   941,  1053,
     988,  1091,  1119,  1143,  1169,  1186,   949,   950,   458,   459,
     245,   351,   395,   443,   246,   247,   346,   385,   439,   659,
     660,   656,   658,   655,   657,   248,   353,   558,   445,   671,
     559,   748,   672,   249,   354,   573,   446,   678,   574,   765,
     679,   250,   355,   590,   447,   687,   688,   682,   683,   686,
     684,   251,   345,   382,   500,   438,   653,   652,   501,   502,
     487,   819,   868,   918,  1005,  1004,   252,   340,   377,   436,
     642,   643,   253,   359,   411,   460,   712,   710,   711,   633,
     851,   890,   802,   943,   634,   849,   989,   799,   254,   342,
     488,   437,   650,   646,   649,   647,   307,   489,   820,   255,
     357,   407,   449,   695,   696,   697,   698,   618,   785,   928,
     910,   971,   967,   968,   969,   619,   786,   930,   256,   356,
     404,   448,   692,   689,   690,   691,   257,   348,   531,   441,
     314,   741,   742,   743,   744,   873,   901,   963,   745,   874,
     904,   964,   746,   876,   907,   965,   490,   818,   865,   917,
     996,   477,   713,   855,   810,   997,   478,   715,   858,   815,
     520,   577,   758,   578,   754,   579,   764,   929,   816,   861,
     598,   772,   842,   599,   769,   840,   877,   924,  1085,   308,
     309,   343,   773,   845,  1037,  1038,  1039,  1070,  1071,  1099,
    1073,  1074,  1101,  1125,  1137,  1122,  1170,  1197,  1207,  1208,
    1210,  1215,  1198,   759,   760,  1183,  1184,   158,   200,   774,
     328,   843,   109,   114,   118,   130,   131,   151,   195,   143,
     193,   262,   115,     4,   132,  1145,   155,   174,   156,    98,
      99,   330
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    97,   100,   455,    11,    11,   379,   160,   755,   495,
      10,    16,   527,   159,   800,   803,   557,   572,   514,   153,
      16,   153,   541,   153,   670,     5,   587,   605,   616,   751,
     556,   571,   192,   331,   475,   747,   494,   513,   106,   677,
     536,    16,   550,   555,   570,   586,   604,   615,     6,   594,
       7,   -21,   479,   161,   162,   521,   595,   194,   632,    16,
      16,   563,   580,   597,   608,     6,    16,     7,   141,    16,
     142,   112,   367,   797,   217,   218,   219,   220,   493,   512,
    -613,  -613,   535,   540,   549,   554,   569,   585,   603,   614,
     153,   153,   153,   153,   476,   761,   505,   518,   762,    10,
     631,    10,    16,   560,   575,   592,   752,     6,    16,     7,
     121,    16,   273,    10,    11,    11,    11,    11,   635,   486,
     751,   516,     6,   515,     7,   543,   107,   542,   522,   589,
     607,   588,   606,   617,    11,   305,   700,    16,   701,   181,
     807,   702,   171,  -599,   172,   808,   812,   181,   450,  1006,
     880,   813,   111,    14,  1007,   507,   190,    11,    11,   192,
     214,     6,   562,     7,    10,    10,    10,   112,   341,   181,
     329,     8,    10,   468,   138,   183,    11,    11,    11,    11,
     466,   451,  1040,   183,   194,  1069,  1136,  1041,    11,   990,
      16,     9,   450,   194,    10,   582,   452,    12,    13,   751,
     756,  1163,    11,  1139,     6,   183,     7,  1141,   523,   119,
     112,   451,   212,   991,   564,   565,   462,   120,  1019,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,   121,   545,   624,  1164,   919,
     452,   103,   206,   105,   181,   181,   944,   945,   181,   181,
     213,   944,   945,   117,   259,   181,   466,   450,   181,    10,
    1018,  1023,  1024,   120,   265,   304,   466,   310,   311,   312,
     358,   316,   317,   318,   319,   320,   321,   322,   323,   325,
     183,   183,   168,   169,   183,   183,   170,   469,    10,   378,
      10,   183,    10,    10,   183,   181,   363,   451,   223,   517,
     451,   176,  -135,   544,  1156,   452,   305,   591,   133,   134,
     135,   181,   412,   946,   546,  1148,   198,   951,  1149,   171,
     225,   172,   466,   226,  1157,   181,   623,   167,   120,    10,
     389,   183,    10,   624,   399,   364,   466,   260,   400,   261,
      10,   232,   233,   401,   234,    10,   181,   183,   648,   651,
     178,   179,   528,   469,   663,   263,    10,    10,   846,   990,
      11,   183,   450,   181,   365,   180,   669,   469,   181,   202,
     203,   204,   205,   238,   181,   470,   944,   945,   676,  1080,
     532,   292,   183,   991,   564,   565,   737,   468,   738,   685,
     266,   739,   267,   740,   954,   221,   182,   508,   181,   183,
     294,   962,   296,   297,   183,   181,   223,   944,   945,    10,
     183,   847,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   225,   887,
    1025,   226,   992,   181,   183,   337,   985,   338,   364,   970,
     333,   183,   230,  1147,   336,  1048,    11,    11,    11,   232,
     233,   344,   234,     6,   375,     7,   376,   349,   972,   112,
     976,    11,    11,    11,    11,    11,    11,   370,   975,   183,
     977,   360,   361,   987,  1155,   469,    11,    11,   237,  1054,
    1069,   238,     6,   978,     7,   129,   181,   979,  1072,   925,
     925,   545,   699,  -206,   705,   230,  -157,   466,   481,  1100,
     609,  -181,   537,  1136,    11,   413,  1144,  1188,   153,   714,
     153,   716,  1189,   718,   995,   610,   958,     6,    11,     7,
     298,   380,   183,   381,    11,   153,   729,   153,   469,   153,
     931,   237,  1196,  1201,  1012,  1021,  1027,   383,   932,   384,
    1205,    11,   153,   770,   379,   999,  1202,   329,  1011,  1020,
    1026,  1206,  1000,   414,  1013,   611,  1028,   128,   957,  1001,
      11,    11,    11,  1211,   386,  1015,   387,  1030,   194,   194,
    1218,   390,  1016,   391,  1031,   201,   960,    11,    11,  1017,
    1022,    16,   393,   223,   394,   329,   396,   371,   397,  1178,
    1179,  1180,   927,   927,   926,   926,   194,   194,   189,   402,
     959,   403,   137,   405,   480,   406,   465,   166,   226,   450,
     181,  1014,   409,  1029,   410,   626,   859,   863,   860,   864,
      11,   466,    11,    11,   486,   181,   232,   233,   209,   234,
     467,   817,   933,   721,   722,   866,   466,   867,  1116,   620,
     621,   622,   870,   884,   871,   885,   183,   434,  1093,   223,
     736,   888,   469,   889,   636,   637,   638,   639,   640,   641,
     470,   183,   627,   899,   902,   900,   903,   469,  1153,   644,
     645,   225,   506,   519,   226,   470,   181,  1154,   496,   561,
     576,   593,   905,   497,   906,   230,  1104,   466,  1105,    10,
      10,   498,   232,   233,   628,   234,  1107,   654,  1108,   499,
     468,   173,   942,  1110,   175,  1111,   177,   154,   157,   154,
     503,   661,   183,   504,  1160,  1167,   869,   662,   469,   911,
    1113,   237,  1114,  1117,   238,  1118,  1094,   998,  1095,   596,
     909,   966,   194,   990,   668,   315,   450,   881,  1123,  1146,
    1161,  1168,   908,  1124,  1138,   600,  1212,  1187,   211,   332,
     152,   693,   694,   673,   674,   675,     0,   991,   564,   565,
       0,   468,     0,   269,   270,   271,   272,   194,     0,    11,
     680,   681,     0,     0,     0,     0,   154,   154,   154,   154,
       0,     0,     0,   582,   452,     0,     0,     0,    10,    10,
      10,     0,     0,     0,     0,   274,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1010,     0,     0,     0,
       0,     0,     0,   706,     0,   708,   709,     0,     0,    10,
      10,    10,    10,    10,     0,     0,    10,     0,    10,     0,
      10,    10,    10,     0,     0,     0,    11,     0,     0,     0,
       0,     0,   792,   793,   794,     0,    10,     0,    10,    10,
      10,   306,    10,    10,     0,   313,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,     0,     0,     0,  1058,  1059,    11,     0,     0,   823,
     824,   825,   826,   465,   827,     0,     0,   181,     0,     0,
     153,   153,     0,     0,   830,     0,     0,   832,   466,     0,
       0,     0,   834,   835,   836,     0,     0,   467,     0,     0,
       0,   468,     0,     0,     0,    11,     0,  1082,  1084,  1086,
      11,     0,  1089,   183,     0,     0,     0,    11,    11,   469,
       0,     0,  1098,   153,   153,   153,     0,   470,   153,     0,
      11,    11,   850,     0,     0,   852,     0,     0,     0,   853,
     854,     0,     0,     0,   856,   857,   471,   194,     0,     0,
       0,     0,   844,     0,  1126,     0,     0,     0,     0,     0,
       0,   223,    11,    11,    11,    11,     0,   223,     0,     0,
     461,    11,     0,    11,     0,     0,    11,    11,   274,   194,
       0,     0,     0,   225,   465,     0,   226,     0,   181,   225,
       0,     0,   226,     0,     0,     0,     0,   230,  1171,   466,
       0,     0,     0,   230,   232,   233,     0,   234,     0,     0,
     232,   233,   468,   234,     0,     0,     0,     0,   468,   879,
       0,     0,   508,   194,   183,     0,     0,     0,     0,     0,
     469,  1092,     0,   237,  1177,     0,   238,     0,   470,   237,
       0,     0,   238,     0,     0,     0,     0,     0,    10,    10,
     153,     0,     0,   481,     0,     0,     0,   509,   223,   844,
       0,    10,    10,  1213,     0,     0,     0,     0,  1216,     0,
       0,     0,  1220,     0,     0,     0,     0,     0,     0,     0,
     225,   465,     0,   226,     0,   181,     0,     0,   194,    10,
       0,     0,     0,   194,   230,     0,   466,   194,   961,     0,
     581,   232,   233,   879,   234,     0,   564,   565,     0,   468,
     973,   974,   566,     0,     0,     0,     0,     0,     0,     0,
       0,   183,     0,  1002,  1003,     0,     0,   469,     0,     0,
     237,   582,     0,   238,    11,   470,   186,   186,  1062,  1063,
       0,     0,    11,     0,     0,     0,     0,     0,     0,    11,
     481,     0,     0,     0,     0,  1044,  1045,  1046,  1047,     0,
       0,     0,     0,     0,  1050,     0,  1052,     0,     0,  1055,
    1056,     0,  1075,  1076,     0,   187,   187,     0,    10,     0,
      10,     0,     0,     0,   154,   703,   154,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   154,     0,   154,     0,   154,     0,     0,     0,     0,
     110,     0,  1103,     0,     0,     0,     0,     0,   154,     0,
       0,     0,     0,  1175,  1176,     0,     0,     0,     0,     0,
       0,     0,   327,     0,     0,   791,     0,   753,   757,   763,
     795,   796,  1195,     0,   768,   771,     0,     0,     0,  1199,
     811,  1200,     0,     0,  1203,  1204,     0,     0,     0,  1209,
       0,     0,     0,     0,  1214,   821,     0,     0,  1217,     0,
     327,  1219,   798,   801,   196,   197,     0,   809,     0,   814,
       0,   703,     0,   828,     0,     0,     0,     0,     0,     0,
     831,     0,     0,     0,   833,     0,     0,     0,     0,     0,
     837,     0,     0,   223,   839,   368,   372,   841,     0,     0,
     222,     0,     0,     0,     0,  1192,     0,   264,     0,  1194,
       0,     0,     0,     0,   480,   225,     0,  1172,   226,     0,
     181,     0,     0,     0,     0,  1173,     0,     0,     0,     0,
       0,   466,  1174,   293,   369,   373,   232,   233,     0,   234,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   875,   295,     0,     0,     0,   183,     0,   299,   300,
       0,   301,   451,     0,     0,     0,   878,     0,   238,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   891,     0,     0,   892,   481,     0,     0,   894,     0,
       0,     0,   453,     0,     0,     0,   463,   472,   482,   491,
     510,   524,   529,   533,   538,   547,   552,   567,   583,   601,
     612,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   629,     0,     0,     0,     0,     0,     0,     0,     0,
     374,   454,     0,     0,     0,   464,   473,   483,   492,   511,
     525,   530,   534,   539,   548,   553,   568,   584,   602,   613,
     878,   408,     0,     0,     0,     0,     0,     0,   415,     0,
     630,     0,     0,     0,     0,     0,     0,     0,   417,     0,
       0,   418,     0,   419,     0,     0,   420,     0,     0,   421,
       0,     0,   422,   423,     0,     0,   424,   223,     0,   425,
       0,     0,   426,   427,   428,   429,     0,     0,   430,     0,
       0,     0,   432,     0,   224,   433,     0,     0,     0,   225,
       0,     0,   226,     0,     0,  1008,     0,     0,   227,   228,
       0,   229,     0,   230,   231,     0,     0,  1036,  1042,     0,
     232,   233,     0,   234,     0,  1064,     0,     0,     0,     0,
     235,     0,     0,   223,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   236,   154,   154,     0,   237,
     224,     0,   238,  1068,     0,   225,     0,     0,   226,  1077,
       0,     0,     0,     0,   227,   228,     0,   229,     0,   230,
     231,     0,     0,   239,  1096,  1097,   232,   233,     0,   234,
       0,     0,     0,     0,     0,     0,   235,     0,  1078,   154,
     154,   154,     0,     0,   154,  1102,     0,     0,     0,   809,
     814,   236,     0,     0,     0,   237,     0,     0,   238,     0,
    1120,  1121,     0,   465,     0,     0,   450,   181,     0,     0,
       0,     0,     0,     0,   664,   665,   666,   667,   466,   258,
     465,     0,     0,   450,   181,     0,     0,     0,   564,   565,
       0,   468,     0,     0,   566,   466,     0,     0,     0,     0,
       0,     0,     0,   183,   467,     0,     0,     0,   468,   469,
       0,  -295,     0,     0,   452,     0,     0,   470,     0,     0,
     183,     0,     0,     0,     0,     0,   469,     0,  -276,     0,
       0,   452,     0,     0,   470,     0,     0,     0,     0,     0,
       0,     0,     0,  1190,     0,     0,     0,     0,  1191,     0,
       0,     0,  1193,     0,     0,     0,     0,     0,   707,     0,
       0,     0,     0,     0,     0,     0,   154,  1181,  1185,     0,
       0,     0,     0,     0,     0,   719,   720,     0,     0,   723,
     724,     0,   725,   726,     0,   727,   728,     0,   730,   731,
     732,   733,   734,   735,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   749,   750,     0,     0,     0,     0,
       0,   766,   767,     0,     0,   775,   776,   777,     0,   778,
     779,   780,   781,   782,   783,   784,     0,     0,   787,   788,
     789,   790,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   804,   805,   806,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   829,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   838,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   952,   955,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   482,   993,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   953,   956,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   882,
     883,     0,   483,   994,     0,     0,     0,     0,     0,     0,
       0,     0,   893,     0,     0,     0,   895,     0,     0,   896,
       0,     0,     0,   897,     0,   898,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   913,     0,     0,
     914,   915,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   921,     0,     0,   922,     0,     0,   923,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1032,  1033,  1034,     0,  1035,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1049,     0,  1051,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1057,     0,     0,  1060,  1061,     0,
       0,  1150,     0,  1158,  1165,     0,     0,     0,     0,     0,
    1065,     0,     0,     0,     0,  1066,     0,     0,  1067,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1151,  1087,  1159,  1166,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1127,     0,  1128,
    1129,     0,  1130,  1131,     0,     0,  1132,     0,     0,  1133,
    1134,    15,  1135,     0,     0,   144,   145,    17,   146,   147,
       0,     0,     0,   148,   149,   150,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    15,     0,     0,     0,    16,
     145,    17,   146,   147,     0,     0,     0,   148,   149,   150,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    15,
       6,     0,     7,     0,     0,    17,   191,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    15,     0,     0,     0,    16,     0,    17,
     326,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    15,     0,     0,
       0,    16,     0,    17,   191,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    15,     0,     0,     0,     0,     0,    17,   191,     0,
       0,  1072,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    15,     0,     0,     0,     0,
       0,    17,   191,     0,     0,  1206,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    15,
    1144,     0,     0,     0,     0,    17,   191,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    15,     0,     0,     0,    16,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    15,     0,     0,
       0,     0,     0,    17,   191,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95
};

static const yytype_int16 yycheck[] =
{
       4,    12,    13,   431,     8,     9,   343,   135,   673,   438,
      14,     8,   440,   134,   708,   709,   445,   446,   439,   131,
       8,   133,   443,   135,   559,     0,   447,   448,   449,    17,
     445,   446,   169,   295,   436,   668,   438,   439,    99,   574,
     442,     8,   444,   445,   446,   447,   448,   449,     5,   447,
       7,    23,   436,    43,    44,   439,   447,   169,   460,     8,
       8,   445,   446,   447,   448,     5,     8,     7,    99,     8,
     101,    11,   334,   706,   202,   203,   204,   205,   438,   439,
     102,   103,   442,   443,   444,   445,   446,   447,   448,   449,
     202,   203,   204,   205,   436,    92,   438,   439,    95,   103,
     460,   105,     8,   445,   446,   447,    94,     5,     8,     7,
      82,     8,    12,   117,   118,   119,   120,   121,   460,   437,
      17,   439,     5,   439,     7,   443,    99,   443,    20,   447,
     448,   447,   448,   449,   138,   102,    85,     8,    87,    31,
      88,    90,    99,    14,   101,    93,    88,    31,    30,    88,
     844,    93,    98,    11,    93,   438,   167,   161,   162,   296,
     100,     5,   445,     7,   168,   169,   170,    11,   305,    31,
     291,    54,   176,    55,    23,    67,   180,   181,   182,   183,
      42,    73,    88,    67,   296,   102,   103,    93,   192,    27,
       8,    74,    30,   305,   198,    77,    78,     8,     9,    17,
      18,    63,   206,  1127,     5,    67,     7,  1131,   100,    56,
      11,    73,   100,    51,    52,    53,   100,    64,   100,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,    82,    22,    75,   100,    83,
      78,    99,    70,   101,    31,    31,    33,    34,    31,    31,
      98,    33,    34,   111,    99,    31,    42,    30,    31,   263,
     963,   964,   100,    64,    99,   276,    42,   278,   279,   280,
      99,   282,   283,   284,   285,   286,   287,   288,   289,   290,
      67,    67,   140,   141,    67,    67,   144,    73,   292,    99,
     294,    67,   296,   297,    67,    31,   100,    73,     4,   439,
      73,   159,    75,   443,    80,    78,   102,   447,   119,   120,
     121,    31,   100,   100,   100,    97,   174,   100,   100,    99,
      26,   101,    42,    29,   100,    31,   100,   138,    64,   333,
      99,    67,   336,    75,    99,    71,    42,    99,    99,   101,
     344,    47,    48,    99,    50,   349,    31,    67,   100,   100,
     161,   162,    37,    73,   100,   213,   360,   361,    99,    27,
     364,    67,    30,    31,   100,    28,   100,    73,    31,   180,
     181,   182,   183,    79,    31,    81,    33,    34,   100,  1044,
     100,   239,    67,    51,    52,    53,    84,    55,    86,   100,
      99,    89,   101,    91,   100,   206,    59,    65,    31,    67,
     258,   100,   260,   261,    67,    31,     4,    33,    34,   413,
      67,    99,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,    26,    99,
     965,    29,   100,    31,    67,    99,    32,   101,    71,   100,
     298,    67,    40,   100,   302,   980,   450,   451,   452,    47,
      48,   309,    50,     5,    99,     7,   101,   315,   100,    11,
      39,   465,   466,   467,   468,   469,   470,   100,   100,    67,
      49,   329,   330,    24,   100,    73,   480,   481,    76,   100,
     102,    79,     5,    62,     7,     8,    31,    66,    14,   910,
     911,    22,   620,    24,   622,    40,    75,    42,    96,   102,
      45,    32,   100,   103,   508,   363,     5,   104,   620,   637,
     622,   639,   105,   641,   943,    60,   918,     5,   522,     7,
       8,    99,    67,   101,   528,   637,   654,   639,    73,   641,
      61,    76,    85,   106,   963,   964,   965,    99,    69,   101,
     107,   545,   654,   680,   881,   943,   102,   668,   963,   964,
     965,    14,   943,   364,   963,   100,   965,   116,   918,   943,
     564,   565,   566,   102,    99,   963,   101,   965,   680,   681,
     102,    99,   963,   101,   965,   179,   918,   581,   582,   963,
     964,     8,    99,     4,   101,   706,    99,   335,   101,    16,
      17,    18,   910,   911,   910,   911,   708,   709,   166,    99,
     918,   101,   126,    99,    25,   101,    27,   137,    29,    30,
      31,   963,    99,   965,   101,    21,    99,    99,   101,   101,
     624,    42,   626,   627,   942,    31,    47,    48,   189,    50,
      51,   717,   912,   644,   645,    99,    42,   101,  1090,   450,
     451,   452,    99,    99,   101,   101,    67,   414,  1053,     4,
     661,    99,    73,   101,   465,   466,   467,   468,   469,   470,
      81,    67,    68,    99,    99,   101,   101,    73,  1140,   480,
     481,    26,   438,   439,    29,    81,    31,  1140,   438,   445,
     446,   447,    99,   438,   101,    40,    99,    42,   101,   693,
     694,   438,    47,    48,   100,    50,    99,   508,   101,   438,
      55,   155,   914,    99,   158,   101,   160,   133,   134,   135,
     438,   522,    67,   438,  1142,  1143,   820,   528,    73,   883,
      99,    76,   101,    99,    79,   101,  1055,   943,  1056,   447,
     880,   924,   844,    27,   545,   281,    30,   845,  1099,  1138,
    1142,  1143,   879,  1101,  1126,   100,  1210,  1174,   192,   296,
     131,   609,   610,   564,   565,   566,    -1,    51,    52,    53,
      -1,    55,    -1,   217,   218,   219,   220,   879,    -1,   773,
     581,   582,    -1,    -1,    -1,    -1,   202,   203,   204,   205,
      -1,    -1,    -1,    77,    78,    -1,    -1,    -1,   792,   793,
     794,    -1,    -1,    -1,    -1,   221,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,   624,    -1,   626,   627,    -1,    -1,   823,
     824,   825,   826,   827,    -1,    -1,   830,    -1,   832,    -1,
     834,   835,   836,    -1,    -1,    -1,   840,    -1,    -1,    -1,
      -1,    -1,   700,   701,   702,    -1,   850,    -1,   852,   853,
     854,   277,   856,   857,    -1,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   291,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1002,  1003,   880,    -1,    -1,   737,
     738,   739,   740,    27,   742,    -1,    -1,    31,    -1,    -1,
    1002,  1003,    -1,    -1,   752,    -1,    -1,   755,    42,    -1,
      -1,    -1,   760,   761,   762,    -1,    -1,    51,    -1,    -1,
      -1,    55,    -1,    -1,    -1,   919,    -1,  1045,  1046,  1047,
     924,    -1,  1050,    67,    -1,    -1,    -1,   931,   932,    73,
      -1,    -1,  1069,  1045,  1046,  1047,    -1,    81,  1050,    -1,
     944,   945,   800,    -1,    -1,   803,    -1,    -1,    -1,   807,
     808,    -1,    -1,    -1,   812,   813,   100,  1069,    -1,    -1,
      -1,    -1,   773,    -1,  1101,    -1,    -1,    -1,    -1,    -1,
      -1,     4,   976,   977,   978,   979,    -1,     4,    -1,    -1,
     434,   985,    -1,   987,    -1,    -1,   990,   991,   414,  1101,
      -1,    -1,    -1,    26,    27,    -1,    29,    -1,    31,    26,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    40,  1145,    42,
      -1,    -1,    -1,    40,    47,    48,    -1,    50,    -1,    -1,
      47,    48,    55,    50,    -1,    -1,    -1,    -1,    55,   840,
      -1,    -1,    65,  1145,    67,    -1,    -1,    -1,    -1,    -1,
      73,  1052,    -1,    76,  1172,    -1,    79,    -1,    81,    76,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,  1062,  1063,
    1172,    -1,    -1,    96,    -1,    -1,    -1,   100,     4,   880,
      -1,  1075,  1076,  1210,    -1,    -1,    -1,    -1,  1215,    -1,
      -1,    -1,  1219,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    27,    -1,    29,    -1,    31,    -1,    -1,  1210,  1103,
      -1,    -1,    -1,  1215,    40,    -1,    42,  1219,   919,    -1,
      46,    47,    48,   924,    50,    -1,    52,    53,    -1,    55,
     931,   932,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,   944,   945,    -1,    -1,    73,    -1,    -1,
      76,    77,    -1,    79,  1148,    81,   165,   166,  1006,  1007,
      -1,    -1,  1156,    -1,    -1,    -1,    -1,    -1,    -1,  1163,
      96,    -1,    -1,    -1,    -1,   976,   977,   978,   979,    -1,
      -1,    -1,    -1,    -1,   985,    -1,   987,    -1,    -1,   990,
     991,    -1,  1040,  1041,    -1,   165,   166,    -1,  1192,    -1,
    1194,    -1,    -1,    -1,   620,   621,   622,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   637,    -1,   639,    -1,   641,    -1,    -1,    -1,    -1,
     107,    -1,  1080,    -1,    -1,    -1,    -1,    -1,   654,    -1,
      -1,    -1,    -1,  1170,  1171,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   668,    -1,    -1,   699,    -1,   673,   674,   675,
     704,   705,  1189,    -1,   680,   681,    -1,    -1,    -1,  1196,
     714,  1198,    -1,    -1,  1201,  1202,    -1,    -1,    -1,  1206,
      -1,    -1,    -1,    -1,  1211,   729,    -1,    -1,  1215,    -1,
     706,  1218,   708,   709,   171,   172,    -1,   713,    -1,   715,
      -1,   717,    -1,   747,    -1,    -1,    -1,    -1,    -1,    -1,
     754,    -1,    -1,    -1,   758,    -1,    -1,    -1,    -1,    -1,
     764,    -1,    -1,     4,   768,   334,   335,   771,    -1,    -1,
     207,    -1,    -1,    -1,    -1,  1183,    -1,   214,    -1,  1187,
      -1,    -1,    -1,    -1,    25,    26,    -1,  1148,    29,    -1,
      31,    -1,    -1,    -1,    -1,  1156,    -1,    -1,    -1,    -1,
      -1,    42,  1163,   240,   334,   335,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   825,   259,    -1,    -1,    -1,    67,    -1,   265,   266,
      -1,   268,    73,    -1,    -1,    -1,   840,    -1,    79,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   855,    -1,    -1,   858,    96,    -1,    -1,   862,    -1,
      -1,    -1,   431,    -1,    -1,    -1,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   460,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     337,   431,    -1,    -1,    -1,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     924,   358,    -1,    -1,    -1,    -1,    -1,    -1,   365,    -1,
     460,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   375,    -1,
      -1,   378,    -1,   380,    -1,    -1,   383,    -1,    -1,   386,
      -1,    -1,   389,   390,    -1,    -1,   393,     4,    -1,   396,
      -1,    -1,   399,   400,   401,   402,    -1,    -1,   405,    -1,
      -1,    -1,   409,    -1,    21,   412,    -1,    -1,    -1,    26,
      -1,    -1,    29,    -1,    -1,   961,    -1,    -1,    35,    36,
      -1,    38,    -1,    40,    41,    -1,    -1,   973,   974,    -1,
      47,    48,    -1,    50,    -1,  1009,    -1,    -1,    -1,    -1,
      57,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,  1002,  1003,    -1,    76,
      21,    -1,    79,  1037,    -1,    26,    -1,    -1,    29,  1043,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    -1,    40,
      41,    -1,    -1,   100,  1058,  1059,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,  1044,  1045,
    1046,  1047,    -1,    -1,  1050,  1079,    -1,    -1,    -1,  1055,
    1056,    72,    -1,    -1,    -1,    76,    -1,    -1,    79,    -1,
    1094,  1095,    -1,    27,    -1,    -1,    30,    31,    -1,    -1,
      -1,    -1,    -1,    -1,   541,   542,   543,   544,    42,   100,
      27,    -1,    -1,    30,    31,    -1,    -1,    -1,    52,    53,
      -1,    55,    -1,    -1,    58,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    51,    -1,    -1,    -1,    55,    73,
      -1,    75,    -1,    -1,    78,    -1,    -1,    81,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    75,    -1,
      -1,    78,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1177,    -1,    -1,    -1,    -1,  1182,    -1,
      -1,    -1,  1186,    -1,    -1,    -1,    -1,    -1,   625,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1172,  1173,  1174,    -1,
      -1,    -1,    -1,    -1,    -1,   642,   643,    -1,    -1,   646,
     647,    -1,   649,   650,    -1,   652,   653,    -1,   655,   656,
     657,   658,   659,   660,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   671,   672,    -1,    -1,    -1,    -1,
      -1,   678,   679,    -1,    -1,   682,   683,   684,    -1,   686,
     687,   688,   689,   690,   691,   692,    -1,    -1,   695,   696,
     697,   698,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   710,   711,   712,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   748,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   765,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   917,   918,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   942,   943,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   917,   918,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   846,
     847,    -1,   942,   943,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   859,    -1,    -1,    -1,   863,    -1,    -1,   866,
      -1,    -1,    -1,   870,    -1,   872,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   884,    -1,    -1,
     887,   888,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   899,    -1,    -1,   902,    -1,    -1,   905,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     967,   968,   969,    -1,   971,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   984,    -1,   986,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1001,    -1,    -1,  1004,  1005,    -1,
      -1,  1140,    -1,  1142,  1143,    -1,    -1,    -1,    -1,    -1,
    1017,    -1,    -1,    -1,    -1,  1022,    -1,    -1,  1025,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1140,  1048,  1142,  1143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1104,    -1,  1106,
    1107,    -1,  1109,  1110,    -1,    -1,  1113,    -1,    -1,  1116,
    1117,     4,  1119,    -1,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,     4,    -1,    -1,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    16,    17,    18,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,     4,
       5,    -1,     7,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,     4,    -1,    -1,    -1,     8,    -1,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,     4,    -1,    -1,
      -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,     4,    -1,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,     4,
       5,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,     4,    -1,    -1,    -1,     8,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,     4,    -1,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   109,   114,   118,   441,     0,     5,     7,    54,    74,
     440,   440,   442,   442,   441,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,   110,   426,   447,   448,
     426,   111,   115,   441,   119,   441,    99,    99,   112,   430,
     430,    98,    11,   117,   431,   440,   121,   441,   432,    56,
      64,    82,   116,   123,   124,   138,   120,   122,   123,     8,
     433,   434,   442,   442,   442,   442,   125,   125,    23,   135,
     113,    99,   101,   437,     8,     9,    11,    12,    16,    17,
      18,   435,   446,   447,   110,   444,   446,   110,   425,   448,
     444,    43,    44,   126,   130,   139,   139,   442,   441,   441,
     441,    99,   101,   429,   445,   429,   441,   429,   442,   442,
      28,    31,    59,    67,   140,   141,   142,   143,   144,   144,
     426,    11,   427,   438,   447,   436,   430,   430,   441,   129,
     426,   129,   442,   442,   442,   442,    70,   145,   152,   152,
     136,   433,   100,    98,   100,   127,   131,   444,   444,   444,
     444,   442,   430,     4,    21,    26,    29,    35,    36,    38,
      40,    41,    47,    48,    50,    57,    72,    76,    79,   100,
     153,   154,   168,   173,   178,   228,   232,   233,   243,   251,
     259,   269,   284,   290,   306,   317,   336,   344,   100,    99,
      99,   101,   439,   441,   430,    99,    99,   101,   132,   429,
     429,   429,   429,    12,   110,   150,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   441,   430,   441,   430,   441,   441,     8,   430,
     430,   430,   151,   146,   426,   102,   110,   314,   397,   398,
     426,   426,   426,   110,   348,   398,   426,   426,   426,   426,
     426,   426,   426,   426,   180,   426,    11,   110,   428,   448,
     449,   138,   438,   441,   128,   133,   441,    99,   101,   147,
     285,   427,   307,   399,   441,   270,   234,   155,   345,   441,
     169,   229,   174,   244,   252,   260,   337,   318,    99,   291,
     441,   441,   137,   100,    71,   100,   134,   138,   142,   143,
     100,   134,   142,   143,   430,    99,   101,   286,    99,   397,
      99,   101,   271,    99,   101,   235,    99,   101,   156,    99,
      99,   101,   170,    99,   101,   230,    99,   101,   175,    99,
      99,    99,    99,   101,   338,    99,   101,   319,   430,    99,
     101,   292,   100,   441,   442,   430,   148,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   183,   430,   430,   150,   149,   287,   309,   273,   236,
     157,   347,   171,   231,   176,   246,   254,   262,   339,   320,
      30,    73,    78,   142,   143,   163,   179,   184,   226,   227,
     293,   429,   100,   142,   143,    27,    42,    51,    55,    73,
      81,   100,   142,   143,   164,   172,   364,   369,   374,   385,
      25,    96,   142,   143,   163,   172,   232,   278,   308,   315,
     364,   142,   143,   164,   172,   227,   233,   243,   251,   259,
     272,   276,   277,   278,   284,   364,   369,   374,    65,   100,
     142,   143,   164,   172,   178,   228,   232,   315,   364,   369,
     378,   385,    20,   100,   142,   143,   158,   163,    37,   142,
     143,   346,   100,   142,   143,   164,   172,   100,   142,   143,
     164,   178,   228,   232,   315,    22,   100,   142,   143,   164,
     172,   177,   142,   143,   164,   172,   226,   227,   245,   248,
     364,   369,   374,   385,    52,    53,    58,   142,   143,   164,
     172,   226,   227,   253,   256,   364,   369,   379,   381,   383,
     385,    46,    77,   142,   143,   164,   172,   178,   228,   232,
     261,   315,   364,   369,   379,   381,   383,   385,   388,   391,
     100,   142,   143,   164,   172,   178,   228,   232,   385,    45,
      60,   100,   142,   143,   164,   172,   178,   228,   325,   333,
     442,   442,   442,   100,    75,   181,    21,    68,   100,   142,
     143,   164,   172,   297,   302,   364,   442,   442,   442,   442,
     442,   442,   288,   289,   442,   442,   311,   313,   100,   312,
     310,   100,   275,   274,   442,   241,   239,   242,   240,   237,
     238,   442,   442,   100,   430,   430,   430,   430,   442,   100,
     181,   247,   250,   442,   442,   442,   100,   181,   255,   258,
     442,   442,   265,   266,   268,   100,   267,   263,   264,   341,
     342,   343,   340,   441,   441,   321,   322,   323,   324,   444,
      85,    87,    90,   110,   167,   444,   442,   430,   442,   442,
     295,   296,   294,   370,   444,   375,   444,   165,   444,   430,
     430,   426,   426,   430,   430,   430,   430,   430,   430,   444,
     430,   430,   430,   430,   430,   430,   426,    84,    86,    89,
      91,   349,   350,   351,   352,   356,   360,   428,   249,   430,
     430,    17,    94,   110,   382,   421,    18,   110,   380,   421,
     422,    92,    95,   110,   384,   257,   430,   430,   110,   392,
     427,   110,   389,   400,   427,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   326,   334,   430,   430,   430,
     430,   429,   441,   441,   441,   429,   429,   428,   110,   305,
     400,   110,   300,   400,   430,   430,   430,    88,    93,   110,
     372,   429,    88,    93,   110,   377,   386,   167,   365,   279,
     316,   429,   159,   441,   441,   441,   441,   441,   429,   430,
     441,   429,   441,   429,   441,   441,   441,   429,   430,   429,
     393,   429,   390,   429,   442,   401,    99,    99,   182,   303,
     441,   298,   441,   441,   441,   371,   441,   441,   376,    99,
     101,   387,   166,    99,   101,   366,    99,   101,   280,   319,
      99,   101,   160,   353,   357,   429,   361,   394,   429,   442,
     400,   399,   430,   430,    99,   101,   185,    99,    99,   101,
     299,   429,   429,   430,   429,   430,   430,   430,   430,    99,
     101,   354,    99,   101,   358,    99,   101,   362,   427,   390,
     328,   328,   186,   430,   430,   430,   195,   367,   281,    83,
     161,   430,   430,   430,   395,   178,   228,   232,   327,   385,
     335,    61,    69,   177,   187,   188,   201,   202,   212,   213,
     215,   216,   309,   301,    33,    34,   100,   142,   143,   224,
     225,   100,   142,   143,   100,   142,   143,   164,   172,   232,
     364,   442,   100,   355,   359,   363,   394,   330,   331,   332,
     100,   329,   100,   442,   442,   100,    39,    49,    62,    66,
     189,   190,   192,   196,   210,    32,   204,    24,   218,   304,
      27,    51,   100,   142,   143,   227,   368,   373,   378,   379,
     381,   385,   442,   442,   283,   282,    88,    93,   110,   162,
     100,   226,   227,   368,   373,   379,   381,   385,   388,   100,
     226,   227,   385,   388,   100,   181,   226,   227,   368,   373,
     379,   381,   430,   430,   430,   430,   110,   402,   403,   404,
      88,    93,   110,   214,   442,   442,   442,   442,   181,   430,
     442,   430,   442,   217,   100,   442,   442,   430,   444,   444,
     430,   430,   441,   441,   429,   430,   430,   430,   429,   102,
     405,   406,    14,   408,   409,   441,   441,   429,   110,   191,
     421,   193,   444,   197,   444,   396,   444,   430,   205,   444,
     203,   219,   426,   218,   372,   377,   429,   429,   427,   407,
     102,   410,   429,   441,    99,   101,   194,    99,   101,   198,
      99,   101,   211,    99,   101,   206,   204,    99,   101,   220,
     429,   429,   413,   405,   409,   411,   427,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   103,   412,   413,   195,
     199,   195,   207,   221,     5,   443,   406,   100,    97,   100,
     142,   143,   200,   224,   225,   100,    80,   100,   142,   143,
     163,   172,   208,    63,   100,   142,   143,   163,   172,   222,
     414,   427,   442,   442,   442,   443,   443,   444,    16,    17,
      18,   110,   209,   423,   424,   110,   223,   422,   104,   105,
     429,   429,   441,   429,   441,   443,    85,   415,   420,   443,
     443,   106,   102,   443,   443,   107,    14,   416,   417,   443,
     418,   102,   417,   427,   443,   419,   427,   443,   102,   443,
     427
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   108,   109,   109,   111,   110,   112,   113,   112,   115,
     116,   114,   117,   117,   117,   117,   119,   120,   118,   121,
     121,   122,   121,   123,   124,   125,   125,   125,   127,   126,
     128,   128,   128,   128,   128,   129,   131,   130,   132,   132,
     133,   133,   133,   133,   134,   136,   137,   135,   138,   139,
     139,   139,   139,   139,   140,   141,   142,   143,   144,   144,
     146,   145,   147,   148,   147,   149,   149,   149,   151,   150,
     150,   152,   152,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   155,   154,   156,   156,   157,   157,   157,
     157,   157,   159,   158,   160,   160,   161,   162,   162,   162,
     163,   165,   166,   164,   167,   167,   167,   167,   169,   168,
     170,   170,   171,   171,   171,   171,   171,   172,   174,   173,
     175,   175,   176,   176,   176,   176,   176,   176,   177,   179,
     178,   180,   182,   181,   183,   184,   183,   183,   183,   183,
     183,   183,   185,   185,   186,   187,   187,   187,   187,   187,
     187,   188,   188,   188,   188,   188,   188,   189,   190,   191,
     191,   192,   193,   194,   194,   195,   195,   195,   195,   195,
     196,   197,   198,   198,   199,   199,   199,   199,   199,   199,
     200,   202,   201,   203,   203,   204,   205,   206,   206,   207,
     207,   207,   207,   207,   207,   208,   209,   209,   210,   211,
     211,   212,   213,   214,   214,   214,   216,   215,   217,   217,
     218,   219,   220,   220,   221,   221,   221,   221,   221,   221,
     222,   223,   223,   224,   225,   226,   227,   229,   228,   230,
     230,   231,   231,   231,   231,   231,   231,   231,   231,   232,
     232,   232,   232,   232,   232,   232,   234,   233,   235,   235,
     236,   237,   236,   236,   238,   236,   236,   236,   236,   236,
     236,   239,   236,   240,   236,   241,   236,   242,   236,   244,
     245,   243,   246,   247,   246,   246,   248,   249,   246,   246,
     250,   246,   246,   246,   246,   246,   246,   246,   252,   253,
     251,   254,   255,   254,   254,   256,   257,   254,   254,   254,
     258,   254,   254,   254,   254,   254,   254,   254,   254,   260,
     261,   259,   262,   263,   262,   262,   264,   262,   262,   262,
     262,   262,   262,   262,   262,   262,   262,   265,   262,   266,
     262,   267,   262,   268,   262,   270,   269,   271,   272,   271,
     273,   274,   273,   273,   273,   273,   273,   273,   273,   273,
     275,   273,   276,   276,   277,   277,   277,   277,   277,   279,
     278,   280,   280,   281,   282,   281,   281,   281,   281,   281,
     283,   281,   285,   284,   286,   286,   287,   288,   287,   287,
     289,   287,   287,   287,   287,   287,   287,   291,   290,   292,
     292,   293,   294,   293,   293,   293,   293,   293,   295,   293,
     296,   293,   298,   297,   299,   299,   300,   300,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   303,   304,
     302,   305,   305,   307,   308,   306,   309,   310,   309,   309,
     309,   309,   309,   311,   309,   312,   309,   313,   309,   314,
     314,   316,   315,   318,   317,   319,   319,   320,   320,   320,
     320,   320,   321,   320,   322,   320,   323,   320,   324,   320,
     326,   327,   325,   328,   329,   328,   330,   328,   331,   328,
     332,   328,   334,   335,   333,   337,   336,   338,   338,   339,
     340,   339,   339,   339,   339,   339,   341,   339,   342,   339,
     343,   339,   345,   346,   344,   347,   347,   347,   347,   348,
     348,   349,   349,   350,   350,   350,   351,   353,   352,   354,
     354,   355,   355,   355,   355,   355,   355,   355,   355,   355,
     357,   356,   358,   358,   359,   359,   359,   359,   359,   361,
     360,   362,   362,   363,   363,   363,   363,   363,   363,   363,
     363,   365,   364,   366,   366,   367,   367,   367,   368,   370,
     371,   369,   372,   372,   372,   373,   375,   376,   374,   377,
     377,   377,   378,   379,   380,   380,   381,   382,   382,   382,
     383,   384,   384,   384,   386,   385,   387,   387,   388,   389,
     389,   390,   390,   391,   393,   392,   392,   395,   394,   394,
     396,   397,   398,   399,   399,   401,   400,   403,   402,   404,
     402,   402,   405,   406,   407,   407,   408,   409,   410,   410,
     411,   412,   412,   413,   413,   414,   415,   416,   417,   418,
     418,   419,   419,   420,   421,   422,   422,   423,   423,   424,
     424,   425,   425,   426,   426,   427,   427,   428,   428,   428,
     429,   429,   430,   430,   430,   432,   431,   433,   434,   434,
     435,   435,   435,   436,   436,   437,   437,   438,   438,   439,
     439,   440,   440,   441,   441,   442,   443,   443,   445,   444,
     444,   446,   446,   446,   446,   446,   446,   446,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   448,   449
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
       1,     1,     1,     1,     1,     1,     0,     5,     1,     4,
       0,     0,     4,     2,     0,     4,     2,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       0,     9,     0,     0,     4,     2,     0,     0,     5,     2,
       0,     4,     2,     2,     2,     2,     2,     2,     0,     0,
       9,     0,     0,     4,     2,     0,     0,     5,     2,     2,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     0,
       0,     9,     0,     0,     4,     2,     0,     4,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     5,     1,     0,     5,
       0,     0,     4,     2,     2,     2,     2,     2,     2,     2,
       0,     4,     1,     1,     1,     1,     1,     1,     1,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     5,     1,     4,     0,     0,     4,     2,
       0,     4,     2,     2,     2,     2,     2,     0,     5,     1,
       4,     0,     0,     4,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     5,     1,     4,     2,     1,     0,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     0,     0,
       9,     2,     1,     0,     0,     9,     0,     0,     4,     2,
       2,     2,     2,     0,     4,     0,     4,     0,     4,     2,
       1,     0,     5,     0,     5,     1,     4,     0,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     0,     8,     0,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     0,     8,     0,     5,     1,     4,     0,
       0,     4,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     0,     9,     0,     2,     2,     4,     2,
       1,     1,     2,     1,     1,     1,     3,     0,     4,     1,
       4,     0,     2,     3,     2,     2,     2,     2,     2,     2,
       0,     4,     1,     4,     0,     2,     3,     2,     2,     0,
       4,     1,     4,     0,     3,     2,     2,     2,     2,     2,
       2,     0,     5,     1,     4,     0,     2,     2,     4,     0,
       0,     6,     2,     2,     1,     4,     0,     0,     6,     2,
       2,     1,     4,     4,     2,     1,     4,     2,     2,     1,
       4,     2,     2,     1,     0,     5,     1,     4,     3,     2,
       2,     3,     1,     3,     0,     3,     2,     0,     4,     1,
       1,     2,     2,     0,     2,     0,     3,     0,     2,     0,
       2,     1,     3,     2,     0,     2,     3,     2,     0,     2,
       2,     0,     2,     0,     6,     5,     5,     5,     4,     0,
       2,     0,     5,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     1,     1,     1,     2,     2,     1,
       2,     4,     0,     2,     2,     0,     4,     2,     0,     1,
       0,     1,     3,     0,     5,     1,     4,     0,     3,     2,
       5,     1,     1,     0,     2,     2,     0,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
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

    case 186: /* type_stmtsep  */

      { if (read_all) {
                yang_delete_type(module, ((*yyvaluep).type));
              }
            }

        break;

    case 197: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 273: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

        break;

    case 347: /* deviation_opt_stmt  */

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

  case 246:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 250:

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

  case 251:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 253:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 254:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 256:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 257:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 258:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 259:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 260:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 261:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 263:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 265:

    { if (read_all) {
                                                 actual = (yyvsp[-1].nodes).container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                               }
                                             }

    break;

  case 267:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 269:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 270:

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

  case 272:

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

  case 273:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }

    break;

  case 275:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 276:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYABORT;
                       }
                     }

    break;

  case 277:

    { if (read_all) {
                     actual = (yyvsp[-2].nodes).node.ptr_leaf;
                     actual_type = LEAF_KEYWORD;
                     (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                     if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaf) == -1) {
                       YYABORT;
                     }
                   }
                 }

    break;

  case 278:

    { (yyval.nodes) = (yyvsp[-4].nodes);}

    break;

  case 279:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 280:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 282:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;}
                                    s = NULL;
                                  }

    break;

  case 283:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i), 0)) {
                                                 YYABORT;
                                               }
                                             }
                                           }

    break;

  case 284:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 285:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 286:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 287:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 288:

    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYABORT;}
                                                               data_node = actual;
                                                               s=NULL;
                                                             }
                                                           }

    break;

  case 289:

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

  case 291:

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

  case 292:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 294:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 295:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYABORT;
                          }
                        }

    break;

  case 296:

    { if (read_all) {
                   actual = (yyvsp[-2].nodes).node.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                   if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER, (struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaflist) == -1) {
                     YYABORT;
                   }
                 }
               }

    break;

  case 297:

    { (yyval.nodes) = (yyvsp[-4].nodes); }

    break;

  case 298:

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

  case 299:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 300:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 302:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 303:

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

  case 304:

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

  case 305:

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

  case 306:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 307:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 308:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 309:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 310:

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

  case 312:

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

  case 313:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 315:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 316:

    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }

    break;

  case 318:

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

  case 319:

    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }

    break;

  case 320:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 321:

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

  case 322:

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

  case 323:

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

  case 324:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 325:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 326:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 327:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }

    break;

  case 329:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 331:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 333:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 334:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 335:

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

  case 338:

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

  case 340:

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

  case 341:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 342:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 343:

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

  case 344:

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

  case 345:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 346:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 347:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 348:

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

  case 349:

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

  case 350:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 351:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 359:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 363:

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

  case 364:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 366:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 367:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 368:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 369:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 370:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 372:

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

  case 376:

    { if (read_all) {
                            (yyval.nodes).anyxml = actual;
                            actual_type = ANYXML_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).anyxml->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).anyxml->iffeature);
                              if (!(yyval.nodes).anyxml->iffeature) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).anyxml->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).anyxml->must);
                              if (!(yyval.nodes).anyxml->iffeature || !(yyval.nodes).anyxml->must) {
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

  case 377:

    { actual = (yyvsp[-1].nodes).anyxml; actual_type = ANYXML_KEYWORD; }

    break;

  case 379:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).anyxml, s, unres, ANYXML_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 380:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 382:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 383:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "anyxml", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 384:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 385:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 386:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 387:

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

  case 388:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYABORT;
                        }
                      }
                    }

    break;

  case 391:

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

  case 392:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 394:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 395:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 396:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 397:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 398:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 400:

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

  case 402:

    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 408:

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

  case 409:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 410:

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

  case 411:

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

  case 412:

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

  case 413:

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

  case 414:

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

  case 415:

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

  case 416:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 417:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 418:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYABORT;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 419:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYABORT;
                                          }
                                        }

    break;

  case 423:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYABORT;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 424:

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

  case 426:

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

  case 427:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 429:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 430:

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

  case 431:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 432:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 433:

    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }

    break;

  case 434:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 435:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                      data_node = actual;
                                    }
                                  }

    break;

  case 436:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

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

  case 441:

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

  case 442:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 443:

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

  case 444:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 447:

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

  case 448:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 449:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 450:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 451:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 452:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 454:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 456:

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

  case 457:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 458:

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

  case 459:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 460:

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

  case 461:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYABORT;
                                        }
                                      }

    break;

  case 463:

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

  case 464:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).node.ptr_inout;
                                         actual_type = INPUT_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                       }
                                     }

    break;

  case 466:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 468:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 470:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 471:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 472:

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

  case 473:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYABORT;
                                         }
                                       }

    break;

  case 475:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 476:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 477:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 479:

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

  case 480:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).notif;
                                      actual_type = NOTIFICATION_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 482:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 483:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 484:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 485:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 486:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 488:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 490:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 492:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 493:

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

  case 495:

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

  case 496:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 497:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 498:

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

  case 501:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 507:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 511:

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

  case 512:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 513:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 514:

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

  case 515:

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

  case 516:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 517:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 518:

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

  case 519:

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

  case 520:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 523:

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

  case 524:

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

  case 525:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 526:

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

  case 527:

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

  case 528:

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

  case 529:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 533:

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

  case 534:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target) == -1) {
                                              YYABORT;
                                            }
                                            ly_set_add((yyvsp[-2].nodes).deviation->dflt_check, (yyvsp[-2].nodes).deviation->target, 0);
                                          }
                                        }

    break;

  case 535:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 536:

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

  case 537:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 538:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 539:

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

  case 540:

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

  case 541:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 546:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 547:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 548:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 549:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 550:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 551:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 552:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 553:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 554:

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

  case 555:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 556:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 557:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 558:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 559:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 560:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 561:

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

  case 563:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 564:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 565:

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

  case 566:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 567:

    { (yyval.uint) = 0; }

    break;

  case 568:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 569:

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

  case 570:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 571:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 572:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 573:

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

  case 574:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 584:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 587:

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

  case 590:

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

  case 591:

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

  case 595:

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

  case 597:

    { tmp_s = yyget_text(scanner); }

    break;

  case 598:

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

  case 599:

    { tmp_s = yyget_text(scanner); }

    break;

  case 600:

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

  case 624:

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

  case 625:

    { (yyval.uint) = 0; }

    break;

  case 626:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 627:

    { (yyval.i) = 0; }

    break;

  case 628:

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

  case 634:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 639:

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

  case 645:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 668:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 758:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 759:

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
