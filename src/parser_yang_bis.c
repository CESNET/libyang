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
  char **p_str;
  void *v;
  char ch;
  struct lys_module *inc;
  struct yang_type *type;
  union {
    uint32_t index;
    struct lys_node_container *container;
    struct lys_node_anydata *anydata;
    struct type_node node;
    struct lys_node_case *cs;
    struct lys_node_grp *grouping;
    struct lys_refine *refine;
    struct lys_node_notif *notif;
    struct type_deviation *deviation;
    struct lys_node_uses *uses;
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
#define YYLAST   3393

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  339
/* YYNRULES -- Number of rules.  */
#define YYNRULES  765
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1235

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
       0,   269,   269,   270,   290,   317,   320,   322,   321,   345,
     358,   345,   367,   368,   376,   377,   380,   393,   380,   404,
     405,   413,   413,   422,   424,   426,   449,   450,   453,   453,
     473,   474,   480,   491,   502,   511,   513,   513,   536,   537,
     541,   542,   553,   564,   573,   585,   596,   585,   605,   607,
     608,   609,   610,   615,   621,   623,   625,   627,   629,   637,
     640,   640,   649,   650,   650,   655,   656,   661,   668,   668,
     677,   683,   722,   725,   726,   727,   728,   729,   730,   731,
     735,   736,   737,   739,   739,   758,   759,   763,   764,   771,
     778,   789,   801,   801,   803,   804,   808,   810,   811,   812,
     823,   825,   826,   825,   829,   830,   831,   832,   849,   849,
     858,   859,   864,   885,   898,   904,   909,   915,   917,   917,
     926,   927,   932,   972,   981,   990,   996,  1001,  1007,  1009,
    1020,  1043,  1046,  1053,  1054,  1060,  1066,  1071,  1078,  1080,
    1088,  1089,  1094,  1095,  1096,  1099,  1134,  1138,  1146,  1154,
    1155,  1159,  1160,  1161,  1169,  1182,  1188,  1189,  1208,  1212,
    1222,  1223,  1228,  1240,  1245,  1250,  1255,  1261,  1270,  1283,
    1284,  1288,  1289,  1301,  1306,  1311,  1316,  1322,  1335,  1335,
    1354,  1355,  1358,  1370,  1380,  1381,  1386,  1410,  1419,  1428,
    1434,  1439,  1445,  1457,  1458,  1476,  1481,  1482,  1487,  1489,
    1491,  1492,  1493,  1509,  1509,  1528,  1529,  1531,  1542,  1552,
    1553,  1558,  1582,  1591,  1600,  1606,  1611,  1617,  1629,  1630,
    1648,  1650,  1652,  1654,  1656,  1656,  1663,  1669,  1674,  1696,
    1702,  1707,  1712,  1713,  1720,  1721,  1722,  1733,  1734,  1735,
    1736,  1737,  1738,  1739,  1740,  1743,  1743,  1751,  1757,  1767,
    1803,  1803,  1805,  1812,  1812,  1820,  1821,  1827,  1833,  1838,
    1843,  1843,  1848,  1848,  1853,  1853,  1864,  1864,  1872,  1872,
    1879,  1910,  1918,  1948,  1948,  1950,  1957,  1957,  1964,  1965,
    1965,  1973,  1976,  1982,  1988,  1994,  1999,  2004,  2014,  2058,
    2099,  2099,  2101,  2108,  2108,  2115,  2135,  2136,  2136,  2144,
    2150,  2165,  2180,  2192,  2198,  2203,  2209,  2216,  2209,  2251,
    2296,  2296,  2298,  2305,  2305,  2313,  2327,  2335,  2341,  2356,
    2371,  2383,  2389,  2394,  2399,  2399,  2407,  2407,  2412,  2412,
    2417,  2417,  2428,  2428,  2436,  2436,  2447,  2453,  2464,  2487,
    2487,  2489,  2499,  2523,  2531,  2539,  2547,  2555,  2563,  2563,
    2573,  2574,  2577,  2578,  2579,  2580,  2581,  2582,  2583,  2590,
    2590,  2598,  2604,  2615,  2637,  2637,  2639,  2646,  2652,  2657,
    2662,  2662,  2669,  2669,  2681,  2681,  2693,  2699,  2710,  2739,
    2739,  2745,  2752,  2752,  2760,  2767,  2774,  2781,  2786,  2792,
    2792,  2813,  2814,  2818,  2854,  2854,  2856,  2863,  2869,  2874,
    2879,  2879,  2887,  2887,  2901,  2901,  2911,  2912,  2917,  2918,
    2921,  2989,  2996,  3005,  3029,  3049,  3068,  3087,  3110,  3133,
    3138,  3144,  3153,  3144,  3168,  3169,  3172,  3181,  3172,  3199,
    3220,  3220,  3222,  3229,  3238,  3243,  3248,  3248,  3256,  3256,
    3264,  3264,  3276,  3276,  3286,  3287,  3290,  3301,  3303,  3314,
    3316,  3317,  3322,  3352,  3359,  3365,  3370,  3375,  3375,  3383,
    3383,  3388,  3388,  3400,  3400,  3413,  3427,  3413,  3441,  3472,
    3472,  3480,  3480,  3488,  3488,  3493,  3493,  3503,  3517,  3503,
    3531,  3541,  3543,  3544,  3556,  3596,  3596,  3604,  3611,  3617,
    3622,  3627,  3627,  3635,  3635,  3640,  3640,  3647,  3647,  3685,
    3705,  3713,  3721,  3731,  3732,  3734,  3739,  3741,  3742,  3743,
    3746,  3748,  3748,  3754,  3755,  3766,  3793,  3801,  3809,  3825,
    3835,  3842,  3849,  3860,  3872,  3872,  3878,  3879,  3904,  3931,
    3939,  3950,  3960,  3971,  3971,  3977,  3981,  3982,  3994,  4011,
    4015,  4023,  4033,  4040,  4047,  4058,  4070,  4070,  4073,  4074,
    4078,  4079,  4084,  4090,  4092,  4093,  4092,  4096,  4097,  4098,
    4113,  4115,  4116,  4115,  4119,  4120,  4121,  4136,  4138,  4140,
    4141,  4163,  4165,  4166,  4167,  4189,  4191,  4192,  4193,  4207,
    4207,  4215,  4216,  4221,  4223,  4224,  4226,  4227,  4229,  4231,
    4231,  4240,  4243,  4243,  4254,  4257,  4267,  4288,  4290,  4291,
    4294,  4294,  4313,  4313,  4322,  4322,  4331,  4334,  4336,  4338,
    4339,  4341,  4343,  4345,  4346,  4348,  4350,  4351,  4353,  4354,
    4356,  4358,  4361,  4365,  4367,  4368,  4370,  4371,  4373,  4375,
    4386,  4387,  4390,  4391,  4403,  4404,  4406,  4407,  4409,  4410,
    4416,  4417,  4420,  4421,  4422,  4448,  4449,  4452,  4453,  4454,
    4457,  4457,  4465,  4467,  4468,  4470,  4471,  4472,  4474,  4475,
    4477,  4478,  4480,  4481,  4483,  4484,  4486,  4487,  4490,  4491,
    4494,  4496,  4497,  4500,  4500,  4509,  4511,  4512,  4513,  4514,
    4515,  4516,  4517,  4519,  4520,  4521,  4522,  4523,  4524,  4525,
    4526,  4527,  4528,  4529,  4530,  4531,  4532,  4533,  4534,  4535,
    4536,  4537,  4538,  4539,  4540,  4541,  4542,  4543,  4544,  4545,
    4546,  4547,  4548,  4549,  4550,  4551,  4552,  4553,  4554,  4555,
    4556,  4557,  4558,  4559,  4560,  4561,  4562,  4563,  4564,  4565,
    4566,  4567,  4568,  4569,  4570,  4571,  4572,  4573,  4574,  4575,
    4576,  4577,  4578,  4579,  4580,  4581,  4582,  4583,  4584,  4585,
    4586,  4587,  4588,  4589,  4590,  4591,  4592,  4593,  4594,  4595,
    4596,  4597,  4598,  4599,  4602,  4611
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
  "$accept", "start", "tmp_string", "string_1", "string_2", "$@1",
  "module_stmt", "$@2", "$@3", "module_header_stmts", "submodule_stmt",
  "$@4", "$@5", "submodule_header_stmts", "$@6", "yang_version_stmt",
  "namespace_stmt", "linkage_stmts", "import_stmt", "$@7",
  "import_opt_stmt", "tmp_identifier_arg_str", "include_stmt", "$@8",
  "include_end", "include_opt_stmt", "revision_date_stmt",
  "belongs_to_stmt", "$@9", "$@10", "prefix_stmt", "meta_stmts",
  "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_stmt", "$@11",
  "revision_end", "$@12", "revision_opt_stmt", "date_arg_str", "$@13",
  "body_stmts", "body_stmt", "extension_stmt", "$@14", "extension_end",
  "extension_opt_stmt", "argument_stmt", "$@15", "argument_end",
  "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@16", "$@17", "status_arg_str", "feature_stmt",
  "$@18", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@19", "identity_end", "identity_opt_stmt",
  "base_stmt", "typedef_arg_str", "typedef_stmt", "type_opt_stmt",
  "type_stmt", "type_arg_str", "type_end", "type_body_stmts",
  "some_restrictions", "union_spec", "fraction_digits_stmt",
  "fraction_digits_arg_str", "length_stmt", "length_arg_str", "length_end",
  "message_opt_stmt", "pattern_stmt", "pattern_arg_str", "pattern_end",
  "pattern_opt_stmt", "modifier_stmt", "enum_specification", "$@20",
  "enum_stmts", "enum_stmt", "enum_arg_str", "enum_end", "enum_opt_stmt",
  "value_stmt", "integer_value_arg_str", "range_stmt", "range_end",
  "path_stmt", "require_instance_stmt", "require_instance_arg_str",
  "bits_specification", "$@21", "bit_stmts", "bit_stmt", "bit_arg_str",
  "bit_end", "bit_opt_stmt", "position_stmt", "position_value_arg_str",
  "error_message_stmt", "error_app_tag_stmt", "units_stmt", "default_stmt",
  "grouping_stmt", "$@22", "grouping_end", "grouping_opt_stmt",
  "data_def_stmt", "container_stmt", "$@23", "container_end",
  "container_opt_stmt", "$@24", "$@25", "$@26", "$@27", "$@28", "$@29",
  "$@30", "leaf_stmt", "leaf_arg_str", "leaf_opt_stmt", "$@31", "$@32",
  "$@33", "leaf_list_arg_str", "leaf_list_stmt", "leaf_list_opt_stmt",
  "$@34", "$@35", "$@36", "list_stmt", "$@37", "$@38", "list_opt_stmt",
  "$@39", "$@40", "$@41", "$@42", "$@43", "$@44", "$@45", "choice_stmt",
  "$@46", "choice_end", "choice_opt_stmt", "$@47", "$@48",
  "short_case_case_stmt", "short_case_stmt", "case_stmt", "$@49",
  "case_end", "case_opt_stmt", "$@50", "$@51", "anyxml_stmt", "$@52",
  "anydata_stmt", "$@53", "anyxml_end", "anyxml_opt_stmt", "$@54", "$@55",
  "uses_stmt", "$@56", "uses_end", "uses_opt_stmt", "$@57", "$@58", "$@59",
  "refine_stmt", "$@60", "refine_end", "refine_arg_str",
  "refine_body_opt_stmts", "uses_augment_stmt", "$@61", "$@62",
  "uses_augment_arg_str", "augment_stmt", "$@63", "$@64",
  "augment_opt_stmt", "$@65", "$@66", "$@67", "$@68", "$@69",
  "augment_arg_str", "action_arg_str", "action_stmt", "rpc_arg_str",
  "rpc_stmt", "rpc_end", "rpc_opt_stmt", "$@70", "$@71", "$@72", "$@73",
  "input_stmt", "$@74", "$@75", "input_output_opt_stmt", "$@76", "$@77",
  "$@78", "$@79", "output_stmt", "$@80", "$@81", "notification_arg_str",
  "notification_stmt", "notification_end", "notification_opt_stmt", "$@82",
  "$@83", "$@84", "$@85", "deviation_stmt", "$@86", "deviation_opt_stmt",
  "deviation_arg_str", "deviate_body_stmt", "deviate_stmts",
  "deviate_not_supported_stmt", "deviate_add_stmt", "$@87",
  "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt", "$@88",
  "deviate_delete_end", "deviate_delete_opt_stmt", "deviate_replace_stmt",
  "$@89", "deviate_replace_stmtsep", "deviate_replace_end",
  "deviate_replace_opt_stmt", "when_stmt", "$@90", "when_end",
  "when_opt_stmt", "config_stmt", "config_read_stmt", "$@91", "$@92",
  "config_arg_str", "mandatory_stmt", "mandatory_read_stmt", "$@93",
  "$@94", "mandatory_arg_str", "presence_stmt", "min_elements_stmt",
  "min_value_arg_str", "max_elements_stmt", "max_value_arg_str",
  "ordered_by_stmt", "ordered_by_arg_str", "must_stmt", "$@95", "must_end",
  "unique_stmt", "unique_arg_str", "unique_arg", "key_stmt", "key_arg_str",
  "$@96", "key_opt", "$@97", "range_arg_str", "absolute_schema_nodeid",
  "absolute_schema_nodeids", "absolute_schema_nodeid_opt",
  "descendant_schema_nodeid", "$@98", "path_arg_str", "$@99", "$@100",
  "absolute_path", "absolute_paths", "absolute_path_opt", "relative_path",
  "relative_path_part1", "relative_path_part1_opt", "descendant_path",
  "descendant_path_opt", "path_predicate", "path_equality_expr",
  "path_key_expr", "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@101", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@102", "strings", "identifier", "identifiers",
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

#define YYPACT_NINF -1007

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1007)))

#define YYTABLE_NINF -619

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1007,    58, -1007, -1007,   205, -1007, -1007, -1007,    62,    62,
   -1007, -1007,  3200,  3200,    62, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,    62,   -59,    62,    23,    47, -1007, -1007,
   -1007,   476,   476,   245, -1007,   120, -1007, -1007,    -1, -1007,
      62,    62,    62,    62, -1007, -1007, -1007, -1007, -1007,   111,
   -1007, -1007,    45,  2535, -1007,  2630,  3200,  2630,    41,    41,
      62, -1007,    62, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,   162, -1007, -1007,
     162, -1007,   162,    62,    62, -1007, -1007,   505,   505,  3200,
      62,    62,    62, -1007, -1007, -1007, -1007, -1007,    62, -1007,
    3200,  3200,    62,    62,    62,    62, -1007, -1007, -1007, -1007,
      72,    72, -1007,  2345,    50,    43,   476,    62, -1007, -1007,
   -1007,  2630,  2630,  2630,  2630,    62, -1007,   444,  1241,    61,
   -1007, -1007,    62, -1007, -1007, -1007,    86,   184,   162,   162,
     162,   162,    88,   476,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,   324,   267,   476, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,  3200,    17,  3200,
    3200,  3200,    17,  3200,  3200,  3200,  3200,  3200,  3200,  3200,
    3200,  3200,  2725,  3200,    62,   476,    62,   361, -1007, -1007,
   -1007, -1007,   476,   476,   476, -1007,   364, -1007,  3295, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007,    89, -1007,   118, -1007, -1007,   388, -1007,   393,
   -1007,   144, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
      62,    62,    62,   230,   455,    62, -1007, -1007, -1007,   397,
   -1007,   155,   160,    62,   408,   446,   467,   167,    62,   468,
     501,   512, -1007, -1007,   171, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007,   519,    62,    62,   397,   198,  2440,    62, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,   476, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,   476,   476, -1007,   476,   476,   476, -1007,
   -1007, -1007, -1007, -1007, -1007,    88,   476, -1007,   476,   476,
     476,   476,   476,   476,   476,   476,   476,  1280,  1076,   476,
    1447,   914,   176,   476,   476,    62,   162,   247,   567,  1734,
     953,  1412,   172,   239,   465,  1012,   215,    62,    62,    62,
      62,    62,    62,    62,    62,    62, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,    62,    62,
      62, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,  1670, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007,    62, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007,   262, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,    62,    62,
   -1007, -1007, -1007, -1007, -1007, -1007,   199, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,    62,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007,    62, -1007, -1007, -1007, -1007, -1007,
      62, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,    62, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007,  2630,  2630, -1007,  2630,
   -1007,  2725,  2630,  2630, -1007, -1007, -1007,    32,    94,    37,
   -1007, -1007, -1007,    62,    62, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007,   202, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,    62,    62, -1007,
   -1007, -1007, -1007,   100,   476,    62,    62, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,  3200,  3200, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,  2630, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,  3200,   471,   476,   476,   476,
     476,   476,  2725,    60,   162,   162,    82, -1007,   100,   531,
   -1007,   162, -1007,   476,   476,   476, -1007, -1007, -1007,   162,
   -1007, -1007, -1007,   162, -1007, -1007, -1007, -1007, -1007,   162,
     476,   476,   476,  2820,  2820, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,   476,   476,   476,   476,   185,   246,   476,
     476,   476,   476, -1007, -1007, -1007, -1007,   162,  2820,  2820,
   -1007, -1007, -1007,   476,   476, -1007,   393, -1007,   476,   476,
     476,   476,   476,   476,   476,   162,   476,   476,   476,   476,
     476,   476,   476, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007,   162, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,   535, -1007, -1007, -1007, -1007,
   -1007,   536,    62, -1007,    62, -1007,    62,    62,    62, -1007,
     162, -1007, -1007,   162, -1007,    39, -1007,   476,   476,   476,
     476,   476,   476,   476, -1007, -1007,    62,    62,    62, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007,   476,   476,   476,   539,
   -1007, -1007,   549,    62,    62,    39,    62,    62, -1007,    62,
      62,   162,    62,    62,   162, -1007, -1007, -1007,   162,   547,
   -1007, -1007, -1007, -1007,    39, -1007, -1007, -1007,  3295, -1007,
     476,   476,   259,    62,   550,    62, -1007, -1007, -1007, -1007,
   -1007, -1007,   555,   556, -1007,   564, -1007, -1007,   476, -1007,
     231,   478, -1007,   330, -1007,   343,   476, -1007, -1007,  3295,
      39,   160,   335,   335, -1007, -1007, -1007, -1007,   476,    54,
     476, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
     311, -1007,    62,    62,    62,    62,    62,    62, -1007,   296,
   -1007, -1007, -1007, -1007, -1007, -1007,    62, -1007,    62, -1007,
     350, -1007, -1007, -1007, -1007, -1007,   275, -1007,   286,   476,
     476,  1480,    62,   287,   476,   476, -1007,   476,    62,    62,
   -1007, -1007, -1007, -1007, -1007,    35,  2630,   149,  2630,  2630,
      84, -1007,   476,  2630,   476,  3200,   343, -1007, -1007, -1007,
      39, -1007, -1007, -1007, -1007, -1007, -1007,  1734,   377, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007,    95, -1007,   164,   250,
     710,  2630,  2630, -1007,   162, -1007,   569, -1007, -1007,   162,
     303,   396,   570, -1007,   573, -1007, -1007, -1007, -1007,   162,
     476,   574, -1007,   330,   577, -1007, -1007, -1007,   476,   476,
     476,   476,   317,    62,    62, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007,   162, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007,   162,   162, -1007,    62, -1007, -1007,
   -1007, -1007,  3295, -1007, -1007,   320, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007,    62,    62, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,    60,    82,   476,   476,   476,
      62,    62, -1007,   476,   476,   476, -1007, -1007,   476,   476,
   -1007,   303, -1007,  2915,   476,   476,   476,   476,   476,   476,
     476,   162,   162,   346,   323, -1007, -1007, -1007,    96,   319,
     405,   342,   404, -1007, -1007, -1007,   426, -1007,   101,    62,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,    62, -1007, -1007,
   -1007, -1007, -1007, -1007,    62, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007,  3295, -1007,  2630,    97,    94,   426,   426,   162,
   -1007, -1007, -1007, -1007,   162, -1007, -1007, -1007,   162, -1007,
     329,   331, -1007, -1007,    62, -1007,    62, -1007,   426,   355,
     426, -1007,   426,   338,   360,   426,   426,   341,   475, -1007,
     426, -1007, -1007,   395,  3010,   426, -1007, -1007, -1007,  3105,
   -1007,   410,   426,  3295, -1007
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     668,     0,     2,     3,     0,     1,   666,   667,     0,     0,
     669,   668,     0,     0,   670,   684,     4,   683,   685,   686,
     687,   688,   689,   690,   691,   692,   693,   694,   695,   696,
     697,   698,   699,   700,   701,   702,   703,   704,   705,   706,
     707,   708,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,   721,   722,   723,   724,   725,   726,
     727,   728,   729,   730,   731,   732,   733,   734,   735,   736,
     737,   738,   739,   740,   741,   742,   743,   744,   745,   746,
     747,   748,   749,   750,   751,   752,   753,   754,   755,   756,
     757,   758,   759,   760,   761,   762,   763,   668,   639,     9,
     764,   668,    16,     6,     0,   638,     0,     5,   647,   647,
     668,    12,    19,     0,   650,    10,   649,   648,    17,     7,
     653,     0,     0,     0,    25,    13,    14,    15,    25,     0,
      20,   668,     0,   655,   654,     0,     0,     0,    49,    49,
       0,    22,     8,   668,   660,   651,   668,   676,   679,   677,
     681,   682,   680,   652,   668,   678,   675,     0,   673,   636,
       0,   668,     0,     0,     0,    26,    27,    58,    58,     0,
     662,   658,   656,   647,   647,    24,   668,    48,   637,    23,
       0,     0,     0,     0,     0,     0,    50,    51,    52,    53,
      71,    71,    45,     0,   657,     0,   645,   674,    28,    35,
      36,     0,     0,     0,     0,     0,   647,     0,     0,     0,
     641,   661,   653,   640,   668,   647,     0,     0,     0,     0,
       0,     0,     0,    59,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   668,   647,    73,    74,    75,    76,    77,    78,   237,
     238,   239,   240,   241,   242,   243,   244,    79,    80,    81,
      82,   668,   647,     0,     0,   646,   647,   647,    38,   647,
      55,    56,    54,    57,    68,    70,    60,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    72,    18,     0,   668,   668,
     663,   668,    30,    40,    37,   668,     0,   372,     0,   445,
     426,   598,   668,   334,   245,    83,   504,   497,   668,   108,
     224,   118,     0,   271,     0,   287,   306,     0,   480,     0,
     448,     0,   129,   765,   644,   389,   668,   668,   374,    46,
     662,   664,   659,     0,     0,    69,   647,    62,    61,     0,
     596,     0,   597,   444,     0,     0,     0,     0,   503,     0,
       0,     0,   647,   647,     0,   647,   482,   481,   647,   450,
     449,   647,     0,   642,   643,     0,     0,     0,     0,   647,
      34,    31,    32,    33,    39,    43,    41,    42,    63,   647,
     376,   373,   647,   599,   647,   336,   335,   647,   247,   246,
     647,    85,    84,   647,   647,   110,   109,   647,   226,   225,
     647,   120,   119,   272,   289,   647,   484,   452,   131,   647,
     391,   390,   375,   647,   668,     0,    29,    65,   378,   429,
     338,   249,    87,   499,   112,   228,   122,     0,     0,   309,
       0,     0,     0,   393,    47,   665,     0,     0,     0,   427,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,   285,   286,   284,
     275,   276,   278,   281,   273,   282,   283,   279,     0,     0,
       0,   288,   304,   305,   303,   292,   293,   296,   295,   290,
     299,   300,   301,   302,   297,   307,   483,   489,   490,   488,
     487,   491,   493,   495,   485,   668,   668,   451,   455,   456,
     454,   453,   457,   459,   461,   463,     0,   130,   136,   137,
     135,   647,   133,   134,     0,    44,    64,    66,    67,   377,
     387,   388,   386,   381,   379,   384,   385,   382,     0,     0,
     434,   435,   433,   432,   436,   442,     0,   438,   440,   430,
     337,   346,   347,   345,   341,   342,   352,   353,   354,   355,
     358,   348,   350,   351,   356,   357,   339,   343,   344,     0,
     248,   258,   259,   257,   252,   266,   260,   268,   262,   264,
     250,   256,   255,   253,     0,    86,    90,    91,    88,    89,
       0,   498,   500,   501,   111,   115,   116,   114,   113,   227,
     230,   231,   229,   647,   647,   647,   647,   647,     0,   121,
     126,   127,   125,   124,   123,   554,     0,     0,   561,     0,
     101,     0,     0,     0,   647,   647,   647,     0,     0,     0,
     647,   647,   647,     0,     0,   322,   323,   321,   312,   324,
     326,   332,     0,   328,   330,   310,   317,   318,   319,   320,
     313,   316,   315,   647,   647,   647,   647,   465,   477,   647,
     647,   647,   647,     0,   132,     0,     0,   392,   398,   399,
     397,   396,   400,   402,   394,   647,   647,     0,     0,   647,
     647,   428,   647,   647,   647,   647,   647,     0,   647,   647,
     647,   647,   647,   647,   647,     0,     0,   233,   232,   234,
     235,   236,     0,     0,     0,     0,     0,   579,     0,     0,
     139,     0,   546,   277,   274,   280,   629,   668,   574,     0,
     668,   630,   570,     0,   631,   668,   668,   668,   578,     0,
     294,   291,   298,     0,     0,   647,   647,   647,   308,   647,
     647,   647,   647,   492,   494,   496,   486,     0,     0,   458,
     460,   462,   464,   668,   668,   668,   107,     0,     0,     0,
     647,   647,   647,   380,   383,   359,     0,   446,   437,   443,
     439,   441,   431,   349,   340,     0,   267,   261,   269,   263,
     265,   251,   254,    92,   668,   668,   668,   668,   502,   668,
     505,   507,   509,   508,     0,   668,   668,   559,   555,   223,
     117,   668,   668,   566,   562,     0,   102,   647,   140,   138,
     222,     0,   572,   571,   573,   568,   569,   577,   576,   575,
       0,   588,   589,     0,   583,     0,   600,   325,   327,   333,
     329,   331,   311,   314,   647,   647,   104,   106,   105,   100,
     425,   421,   668,   409,   404,   668,   401,   403,   395,     0,
     447,   567,     0,   511,   524,     0,   533,   506,   128,   558,
     557,     0,   565,   564,     0,   647,   581,   580,     0,   145,
     647,   548,   547,   591,     0,   585,   584,   587,     0,   598,
     468,   468,     0,   424,     0,   408,   647,   361,   360,   647,
      94,   647,     0,     0,   510,     0,   556,   563,   162,   103,
       0,   142,   143,     0,   144,     0,   550,   590,   594,     0,
       0,   601,   466,   478,   647,   647,   406,   405,   363,     0,
      93,   647,   513,   512,   647,   526,   525,   647,   536,   534,
       0,   141,     0,     0,     0,     0,     0,     0,   148,     0,
     151,   149,   150,   647,   147,   146,     0,   647,     0,   205,
       0,   592,   586,   471,   473,   475,     0,   469,     0,   429,
     410,     0,     0,     0,   515,   528,   538,   535,     0,     0,
     582,   165,   166,   163,   164,     0,     0,   602,     0,     0,
       0,   647,   152,     0,   180,     0,   204,   549,   551,   552,
       0,   647,   647,   647,   467,   647,   479,   422,     0,   362,
     368,   369,   367,   366,   370,   364,     0,    95,     0,     0,
       0,     0,     0,   157,     0,   668,     0,   159,   606,     0,
       0,     0,     0,   168,     0,   595,   668,   668,   202,     0,
     153,     0,   183,   179,     0,   208,   206,   593,   472,   474,
     476,   470,     0,     0,     0,   407,   419,   420,   412,   414,
     415,   416,   413,   417,   418,   647,   647,   647,   668,   668,
      99,     0,   514,   516,   519,   520,   521,   522,   523,   647,
     518,   527,   529,   532,   647,   531,   537,   647,   540,   541,
     542,   543,   544,   545,     0,     0,   155,   156,   647,   160,
     647,   198,     0,   609,   603,     0,   605,   613,   647,   169,
     647,   647,   196,   195,   201,   200,   199,   647,   184,   182,
     647,   647,   209,   647,   423,     0,     0,   411,   371,   365,
      98,    97,    96,   517,   530,   539,   221,   220,   162,   158,
     618,   608,   612,     0,   171,   167,   162,   186,   181,   211,
     207,     0,     0,     0,   607,   610,   614,   611,   616,     0,
       0,     0,     0,   553,   560,   161,   671,   615,     0,     0,
     170,   175,   176,   172,   173,   174,   197,     0,   185,   190,
     191,   189,   187,   188,     0,   210,   215,   216,   214,   212,
     213,   672,     0,   617,     0,     0,     0,   671,   671,     0,
     634,   635,   632,   194,     0,   668,   633,   219,     0,   668,
       0,     0,   177,   192,   193,   217,   218,   619,   671,     0,
     671,   620,   671,     0,     0,   671,   671,     0,     0,   628,
     671,   621,   624,     0,     0,   671,   625,   626,   623,   671,
     622,     0,   671,     0,   627
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1007, -1007, -1007,   593, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,   406, -1007,   401, -1007, -1007,
   -1007,   344, -1007, -1007, -1007, -1007,   193, -1007, -1007, -1007,
    -288,   420, -1007, -1007,  -130,   734,   375, -1007, -1007, -1007,
   -1007, -1007,   124, -1007,   382, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,  -441,  -368, -1007, -1007,  -132,
   -1007, -1007, -1007, -1007,  -420, -1007, -1007, -1007, -1007,  -324,
   -1007,  -362, -1007,  -435, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1006, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,  -445, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,  -384, -1007, -1007,
   -1007, -1007, -1007,  -552,  -534,  -419,  -403,  -286, -1007, -1007,
   -1007,  -398,   170, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,   175, -1007, -1007, -1007, -1007, -1007, -1007,
     178, -1007, -1007, -1007, -1007,   182, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007,   196, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007,   197, -1007, -1007, -1007, -1007, -1007,
     209, -1007,   210, -1007,   288, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,  -292, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007,  -360, -1007, -1007,   -88, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007,  -198, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007,  -389, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007, -1007,
   -1007, -1007, -1007,  -374, -1007, -1007, -1007,  -846,  -182, -1007,
   -1007,  -414,  -558,   232, -1007, -1007,  -412,  -296,  -489, -1007,
    -480, -1007,   212, -1007,  -424, -1007, -1007,  -761, -1007,  -205,
   -1007, -1007, -1007,  -281, -1007, -1007,  -350,   430,  -166,  -726,
   -1007, -1007, -1007, -1007,  -416,  -442, -1007, -1007,  -415, -1007,
   -1007, -1007,  -429, -1007, -1007, -1007,  -504, -1007, -1007, -1007,
    -615,  -465, -1007, -1007, -1007,   112,  -137,  -592,   546,  1250,
   -1007, -1007,   513, -1007, -1007, -1007, -1007,   386, -1007,    -4,
      -3,   460,   602,    -6, -1007,   594,   154,  -110, -1007
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   107,   131,     2,   104,   124,   115,
       3,   106,   128,   118,   129,   125,   126,   138,   165,   216,
     343,   198,   166,   217,   269,   344,   380,   141,   209,   376,
     127,   167,   186,   187,   971,   972,   190,   206,   306,   348,
     427,   447,   276,   305,   207,   242,   243,   356,   402,   452,
     588,   852,   891,   963,  1061,   542,   469,   708,   868,   757,
     244,   359,   406,   454,   543,   245,   361,   412,   456,   614,
     331,   246,   442,   471,   709,   809,   900,   901,   939,   940,
    1014,   941,  1016,  1090,   930,   942,  1022,  1100,  1149,  1163,
     902,   903,  1033,   947,  1031,  1109,  1151,  1173,  1194,   943,
    1103,   944,   945,  1029,   904,   905,   986,   949,  1034,  1113,
    1152,  1180,  1198,   973,   974,   472,   473,   247,   360,   409,
     455,   248,   249,   355,   399,   451,   693,   694,   689,   691,
     692,   688,   690,   250,   322,   437,   625,   624,   626,   324,
     251,   438,   631,   630,   632,   252,   364,   642,   495,   741,
     742,   735,   736,   739,   740,   737,   253,   354,   396,   450,
     686,   685,   561,   562,   545,   849,   888,   961,  1057,  1056,
     254,   349,   255,   375,   391,   448,   675,   676,   256,   372,
     421,   524,   762,   760,   761,   672,   884,   917,   844,   998,
     673,   882,  1042,   841,   257,   351,   546,   449,   684,   679,
     682,   683,   680,   310,   766,   547,   329,   258,   370,   441,
     659,   660,   661,   662,   514,   747,   956,   912,   995,   991,
     992,   993,   515,   748,   958,   327,   259,   367,   440,   656,
     653,   654,   655,   260,   357,   453,   317,   788,   789,   790,
     791,   892,   923,  1008,   792,   893,   926,  1009,   793,   895,
     966,   929,  1010,   549,   811,   872,   950,  1050,   475,   703,
     861,   798,  1051,   476,   706,   864,   804,   582,   491,   723,
     492,   719,   493,   729,   957,   805,   867,   651,   824,   876,
     652,   821,   874,   907,   990,  1024,   311,   312,   352,   825,
     879,  1019,  1020,  1021,  1093,  1094,  1131,  1096,  1097,  1133,
    1147,  1157,  1144,  1187,  1211,  1221,  1222,  1224,  1229,  1212,
     724,   725,  1195,  1196,   160,   199,   826,   335,   877,   111,
     116,   120,   132,   133,   153,   194,   145,   193,   300,   117,
       4,   134,  1182,   157,   176,   158,   100,   101,   337
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,   520,   393,   486,    11,    11,   647,   521,    14,   339,
      10,   589,   720,   477,   494,   648,   504,   470,   485,   487,
     500,   511,   -21,   522,   537,    16,   161,   583,   533,   710,
     554,   574,   842,   845,   598,   488,   613,   188,   188,   523,
      16,   108,   503,    16,     6,    16,     7,   555,     6,   716,
       7,   544,   716,   577,   114,   381,   212,   605,     5,     6,
     548,     7,   579,   474,   489,   114,   607,     6,    16,     7,
     484,   650,   499,   510,   534,   638,   566,   580,   501,   512,
     532,   123,   553,   573,   163,   164,   597,   602,   612,   575,
      16,   578,    16,   603,   103,   606,    16,   641,   105,    10,
     274,    10,    16,    16,   671,    16,   644,   113,    16,    10,
     794,   716,   721,  1190,  1191,  1192,    11,    11,    11,    11,
     308,   645,  1143,   109,    99,   102,   717,   637,   142,   726,
    1150,   162,   727,   639,   140,   643,    11,   962,    10,   173,
     170,   174,   205,   171,   215,   143,   110,   144,   795,   214,
     674,   172,   910,   796,   502,   513,   670,    16,   178,    11,
      11,   262,  1065,  -604,  1080,   576,    10,    10,    10,   604,
     801,   350,  1026,   197,    10,   802,   121,  1027,    11,    11,
      11,    11,   336,  1058,   122,   753,   266,   754,  1059,   362,
     755,  1043,   584,    10,   458,   218,   219,   220,   221,  -618,
    -618,    11,   123,   183,  1092,  1156,   458,   183,    11,   640,
       6,   264,     7,   382,   386,  1044,   478,   479,   363,   461,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,   608,   294,   185,
     212,   634,   464,   185,   371,   516,   183,  1070,  1075,   516,
       6,   463,     7,   119,   464,   392,   490,   459,   296,     8,
      10,   183,   173,   308,   174,  1062,   535,   403,   567,   581,
     183,   415,     6,   585,     7,   301,   590,   517,   183,     9,
     458,   192,   185,   665,   267,   834,   268,   155,   462,   155,
      10,   155,    10,   183,   122,   340,   341,   185,   342,   423,
     681,   378,   345,   738,   459,   461,   185,   467,   482,   353,
     497,   508,   518,   646,   185,   358,   609,   527,   530,   540,
     551,   571,   586,   592,   595,   600,   610,   634,   464,   185,
     666,   379,   931,   373,   374,   462,    10,    10,    10,   224,
     591,    10,   183,   465,   968,   969,   835,   213,   526,    10,
     183,  1071,   968,   969,    10,   155,   155,   155,   155,   914,
    1015,   226,   946,   667,   227,   635,     6,   948,     7,    10,
      10,   463,   114,   183,    11,   231,   994,   183,   185,   968,
     969,   183,   233,   234,   459,   235,   185,   996,  1007,   307,
     461,   313,   314,   315,   668,   319,   320,   321,   323,   325,
     326,   328,   330,   332,  1043,   338,  1092,   458,   183,   185,
    1095,   238,   970,   185,   239,   516,  1159,   185,  1114,   459,
    1160,   445,  1167,  1132,   298,   122,   299,  1156,  1044,   478,
     479,  1181,   461,   240,  1207,   183,   183,  1208,   968,   969,
    1210,    10,   569,  1168,   185,  1215,   459,  1155,   224,  1219,
    1066,   987,  1081,    11,    11,    11,    11,    11,    11,    11,
      11,    11,   213,  1216,   346,   225,   347,  1174,    12,    13,
     226,   185,   185,   227,    11,    11,    11,   516,  1045,   228,
     229,     6,   230,     7,   231,   232,   183,   114,   365,  1220,
     366,   233,   234,   368,   235,   369,   183,   389,  1225,   390,
     608,   236,   657,   658,   981,  1175,  1166,   459,   394,  1053,
     395,   336,    11,  1232,   955,   955,   237,   932,  1054,  1067,
     238,  1082,   185,   239,   130,   200,   378,   933,  1068,   139,
    1083,   213,   185,   182,    11,    11,   183,   385,   462,   934,
     935,  1003,   240,   191,   936,   241,   397,   937,   398,   446,
     953,   953,     6,  -154,     7,   784,   384,   785,   114,   168,
     786,   393,   787,  1004,   184,    11,   594,   400,   404,   401,
     405,  -203,   185,   208,  1055,  1077,   806,   938,  1048,  -178,
      11,   135,   136,   137,  1069,  1074,    11,  1005,  1110,  1063,
    1072,  1078,   336,  1002,   457,  1049,   822,  1164,   183,   544,
     169,   407,  1036,   408,    11,  1064,  1073,  1079,   548,   459,
     704,   705,   410,   707,   411,  1165,   711,   712,   460,   419,
     556,   420,   461,   180,   181,   557,   954,   954,   558,    11,
      11,   807,   559,   808,   185,   865,   870,   866,   871,   886,
     462,   887,   201,   202,   203,   204,   560,   563,   465,   889,
     915,   890,   916,    10,    10,   921,   924,   922,   925,   564,
     565,    11,    11,   422,   927,   222,   928,   997,   529,  1088,
    1098,  1089,  1099,  1101,  1107,  1102,  1108,  1111,   850,  1112,
     536,   775,   568,   913,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,  1141,  1052,   175,  1142,   952,   177,   649,   179,  1037,
    1171,  1178,   318,   911,   812,  1145,  1183,   814,  1146,  1158,
    1226,  1199,   816,   817,   818,   263,   377,   154,   156,   159,
     156,  1172,  1179,     0,     0,     0,     0,  1043,     0,     0,
     458,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     836,   837,   838,     0,     0,     0,     0,     0,     0,     0,
       0,  1044,   478,   479,   270,   271,   272,   273,     0,     0,
     155,   155,   951,   155,     0,     0,   155,   155,     0,     0,
       0,   853,   854,   855,   856,   463,   857,     0,   464,   765,
     767,     0,   859,   860,   156,   156,   156,   156,   862,   863,
       0,     0,     0,     0,     0,     0,     0,   783,    10,     0,
      10,  1076,    10,    10,    10,   275,     0,     0,     0,     0,
     988,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1000,    10,    10,    10,     0,     0,     0,   425,   883,
       0,   155,   885,     0,     0,     0,     0,     0,     0,    10,
      10,    10,    10,    10,     0,    10,    10,     0,    10,    10,
       0,     0,     0,     0,     0,     0,     0,   540,  1046,     0,
      11,   309,     0,     0,     0,   316,     0,     0,     0,    10,
       0,    10,     0,     0,     0,   334,     0,   213,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   189,   189,     0,     0,     0,    11,     0,     0,     0,
       0,     0,   213,   213,     0,     0,     0,   615,   616,   617,
     618,   619,   620,   621,   622,   623,     0,     0,    11,    11,
      11,    11,    11,    11,     0,     0,     0,     0,   627,   628,
     629,     0,    11,     0,    11,   183,     0,     0,     0,     0,
       0,     0,     0,     0,   231,  1130,   459,   224,    11,   505,
       0,     0,     0,     0,    11,    11,     0,     0,     0,     0,
    1017,     0,  1023,  1025,   506,     0,   663,  1032,   538,   226,
     457,   185,   227,   458,   183,     0,    11,   462,     0,     0,
     238,     0,   525,     0,     0,   459,  1148,     0,   677,   678,
     233,   234,     0,   235,   460,  1084,  1085,     0,     0,     0,
       0,     0,  1087,     0,     0,   507,   224,     0,   275,  1161,
     185,  1169,  1176,  1104,  1105,     0,   462,     0,     0,   687,
       0,     0,   213,     0,   465,     0,     0,     0,   226,    11,
      11,   227,     0,   183,   695,  1188,     0,     0,     0,     0,
     696,   240,   231,     0,   550,  1120,  1121,     0,     0,   233,
     234,     0,   235,   213,     0,     0,     0,     0,   702,   236,
       0,     0,     0,     0,     0,     0,     0,   383,   387,   185,
       0,     0,     0,    10,     0,   462,     0,  1227,   238,     0,
       0,   239,  1230,   733,   734,     0,  1234,  1035,     0,     0,
      10,    10,     0,   457,     0,     0,   458,   183,   539,     0,
     240,     0,     0,   599,     0,     0,    10,    10,   459,     0,
       0,     0,     0,     0,     0,   758,   759,     0,   478,   479,
     155,   461,   155,   155,   480,     0,     0,   155,     0,     0,
       0,     0,     0,   185,     0,     0,     0,     0,     0,   462,
       0,   463,     0,     0,   464,    11,     0,   465,     0,     0,
       0,     0,     0,    11,     0,   155,   155,     0,     0,     0,
      11,   468,   483,     0,   498,   509,   519,   481,  1189,     0,
       0,   528,   531,   541,   552,   572,   587,   593,   596,   601,
     611,     0,  1204,     0,     0,     0,  1206,     0,     0,     0,
      10,     0,    10,     0,     0,     0,     0,     0,     0,   156,
     156,     0,   156,     0,   334,   156,   156,     0,     0,     0,
     718,   722,   728,     0,     0,     0,     0,     0,     0,   636,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   224,   213,     0,     0,     0,
     799,   800,     0,     0,     0,     0,   756,   810,   669,     0,
       0,     0,   225,     0,     0,   813,     0,   226,     0,   815,
     227,     0,     0,     0,     0,   819,   228,   229,     0,   230,
     156,   231,   232,     0,     0,   878,     0,   213,   233,   234,
       0,   235,     0,     0,     0,   334,   797,     0,   236,   803,
       0,   756,     0,   839,     0,     0,     0,   457,     0,     0,
     458,   183,     0,   237,     0,     0,     0,   238,     0,     0,
     239,   851,   459,     0,     0,     0,   820,   823,     0,     0,
       0,   460,     0,     0,   909,   461,   213,     0,   155,   240,
     858,     0,   261,     0,     0,     0,     0,   185,     0,     0,
       0,   840,   843,   462,     0,   463,     0,     0,   464,   112,
       0,   465,     0,     0,     0,     0,   873,     0,     0,   875,
     878,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,   466,     0,   213,     0,     0,     0,   213,     0,     0,
       0,     0,   975,   976,   977,   978,   979,   980,     0,     0,
       0,   894,     0,     0,     0,     0,   983,   896,   985,     0,
     897,     0,     0,     0,   899,     0,   224,     0,     0,     0,
     908,     0,  1006,   195,   196,     0,     0,     0,  1011,  1012,
       0,     0,     0,     0,     0,     0,     0,     0,   226,   457,
       0,   227,     0,   183,     0,     0,     0,     0,     0,     0,
     909,   224,   231,     0,   459,     0,   223,     0,     0,   233,
     234,     0,   235,     0,     0,   265,     0,   461,     0,   236,
       0,     0,     0,   226,     0,     0,   227,   569,   183,   185,
       0,     0,     0,     0,   224,   462,     0,   231,   238,   459,
       0,   239,   295,   465,   233,   234,     0,   235,     0,     0,
       0,     0,   461,  1115,  1116,     0,   226,     0,   539,   227,
     240,   183,   297,   570,   185,     0,   302,   303,     0,   304,
     462,     0,   459,   238,     0,     0,   239,   233,   234,     0,
     235,     0,     0,     0,     0,     0,   908,     0,     0,     0,
       0,     0,     0,     0,     0,   240,     0,   185,   496,     0,
       0,     0,     0,   462,     0,     0,     0,     0,     0,   239,
    1086,   465,     0,     0,     0,  1091,     0,     0,  1013,   156,
    1018,   156,   156,  1028,     0,  1106,   156,     0,   240,     0,
       0,   999,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   388,     0,     0,  1060,
       0,     0,     0,     0,   156,   156,     0,  1122,     0,     0,
       0,     0,   413,   414,     0,   416,     0,     0,   417,  1184,
       0,   418,     0,     0,     0,     0,     0,  1185,     0,   426,
    1126,  1127,     0,     0,  1186,     0,     0,     0,     0,   428,
       0,     0,   429,     0,   430,     0,     0,   431,     0,     0,
     432,     0,     0,   433,   434,     0,     0,   435,     0,     0,
     436,     0,     0,     0,     0,   439,     0,     0,     0,   443,
       0,     0,     0,   444,   224,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   989,     0,     0,  1153,  1154,     0,
       0,     0,     0,     0,     0,  1001,   226,   457,     0,   227,
       0,   183,     0,     0,     0,     0,     0,     0,   797,   803,
     231,     0,   459,     0,     0,     0,   633,   233,   234,     0,
     235,     0,   478,   479,     0,   461,     0,   236,   480,     0,
       0,   541,  1047,     0,     0,  1202,     0,   185,   224,     0,
    1203,     0,     0,   462,  1205,     0,   238,   634,     0,   239,
       0,   465,     0,     0,     0,     0,     0,     0,     0,   538,
     226,     0,     0,   227,     0,   183,   539,     0,   240,     0,
       0,   664,     0,     0,     0,     0,   459,   156,  1193,  1197,
       0,   233,   234,     0,   235,     0,     0,     0,     0,  1200,
    1201,   236,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   185,     0,     0,     0,     0,     0,   516,     0,     0,
    1209,     0,  1213,   239,  1214,   465,     0,  1217,  1218,     0,
       0,     0,  1223,     0,     0,     0,     0,  1228,     0,     0,
     539,  1231,   240,     0,  1233,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   697,   698,   699,   700,   701,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   713,   714,   715,     0,     0,     0,
     730,   731,   732,  1162,     0,  1170,  1177,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   743,   744,   745,   746,     0,     0,   749,
     750,   751,   752,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   763,   764,     0,     0,   768,
     769,     0,   770,   771,   772,   773,   774,     0,   776,   777,
     778,   779,   780,   781,   782,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   827,   828,   829,     0,   830,
     831,   832,   833,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     846,   847,   848,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   869,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   880,   881,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   898,     0,     0,     0,     0,
     906,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   918,     0,     0,   919,
       0,   920,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   959,   960,     0,     0,     0,     0,
       0,   964,     0,     0,   965,     0,     0,   967,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   982,     0,     0,     0,   984,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1030,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1038,  1039,  1040,     0,  1041,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1117,  1118,  1119,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1123,
       0,     0,     0,     0,  1124,     0,     0,  1125,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1128,     0,
    1129,     0,     0,     0,     0,     0,     0,     0,  1134,    15,
    1135,  1136,     0,     0,     0,    17,   210,  1137,     0,     0,
    1138,  1139,     0,  1140,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,   211,     0,     0,     0,
      17,   210,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,   424,     0,   146,   147,    17,   148,   149,     0,     0,
       0,   150,   151,   152,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,   147,
      17,   148,   149,     0,     0,     0,   150,   151,   152,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,    16,     0,    17,   333,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,     0,
      17,   210,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,     0,     0,    17,   210,     0,     0,  1095,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,     0,     0,
      17,   210,     0,     0,  1220,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
    1181,     0,     0,     0,     0,    17,   210,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,     0,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,     0,     0,    17,   210,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96
};

static const yytype_int16 yycheck[] =
{
       4,   442,   352,   438,     8,     9,   495,   442,    11,   297,
      14,   452,   627,   437,   438,   495,   440,   437,   438,   438,
     440,   441,    23,   442,   448,     8,   136,   451,   448,   621,
     450,   451,   758,   759,   454,   438,   456,   167,   168,   442,
       8,   100,   440,     8,     5,     8,     7,   450,     5,    17,
       7,   449,    17,   451,    11,   343,   193,   455,     0,     5,
     449,     7,   451,   437,   438,    11,   455,     5,     8,     7,
     438,   495,   440,   441,   448,   495,   450,   451,   440,   441,
     448,    82,   450,   451,    43,    44,   454,   455,   456,   451,
       8,   451,     8,   455,    97,   455,     8,   495,   101,   103,
      12,   105,     8,     8,   524,     8,   495,   110,     8,   113,
     702,    17,    18,    16,    17,    18,   120,   121,   122,   123,
     103,   495,  1128,   100,    12,    13,    94,   495,   131,    92,
    1136,   137,    95,   495,    23,   495,   140,    83,   142,   100,
     143,   102,    70,   146,   101,   100,    99,   102,    88,    99,
     524,   154,   878,    93,   440,   441,   524,     8,   161,   163,
     164,   100,  1008,    14,  1010,   451,   170,   171,   172,   455,
      88,   308,    88,   176,   178,    93,    56,    93,   182,   183,
     184,   185,   292,    88,    64,    85,   100,    87,    93,   100,
      90,    27,    20,   197,    30,   201,   202,   203,   204,   103,
     104,   205,    82,    31,   103,   104,    30,    31,   212,   495,
       5,   214,     7,   343,   344,    51,    52,    53,   100,    55,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,    22,   241,    67,
     377,    77,    78,    67,   100,    73,    31,  1008,  1009,    73,
       5,    75,     7,     8,    78,   100,   438,    42,   261,    54,
     264,    31,   100,   103,   102,   101,   448,   100,   450,   451,
      31,   100,     5,   101,     7,     8,    37,   101,    31,    74,
      30,   169,    67,    21,   100,   100,   102,   133,    73,   135,
     294,   137,   296,    31,    64,   298,   299,    67,   301,   101,
     101,    71,   305,   101,    42,    55,    67,   437,   438,   312,
     440,   441,   442,   495,    67,   318,   101,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,    77,    78,    67,
      68,   101,   101,   336,   337,    73,   340,   341,   342,     4,
     101,   345,    31,    81,    33,    34,   100,   193,   101,   353,
      31,   101,    33,    34,   358,   201,   202,   203,   204,   100,
     975,    26,    32,   101,    29,   495,     5,    24,     7,   373,
     374,    75,    11,    31,   378,    40,   101,    31,    67,    33,
      34,    31,    47,    48,    42,    50,    67,   101,   101,   277,
      55,   279,   280,   281,   524,   283,   284,   285,   286,   287,
     288,   289,   290,   291,    27,   293,   103,    30,    31,    67,
      14,    76,   101,    67,    79,    73,    97,    67,   101,    42,
     101,   424,    80,   103,   100,    64,   102,   104,    51,    52,
      53,     5,    55,    98,   105,    31,    31,   106,    33,    34,
      85,   445,    65,   101,    67,   107,    42,   101,     4,   108,
    1008,   101,  1010,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   308,   103,   100,    21,   102,    63,     8,     9,
      26,    67,    67,    29,   478,   479,   480,    73,   101,    35,
      36,     5,    38,     7,    40,    41,    31,    11,   100,    14,
     102,    47,    48,   100,    50,   102,    31,   100,   103,   102,
      22,    57,   505,   506,   939,   101,   101,    42,   100,   998,
     102,   621,   516,   103,   912,   913,    72,    39,   998,  1008,
      76,  1010,    67,    79,   118,   181,    71,    49,  1008,   128,
    1010,   377,    67,    28,   538,   539,    31,   344,    73,    61,
      62,   961,    98,   168,    66,   101,   100,    69,   102,   425,
     912,   913,     5,    75,     7,    84,   101,    86,    11,   139,
      89,   911,    91,   961,    59,   569,   101,   100,   100,   102,
     102,    24,    67,   191,   998,  1010,   708,   901,   998,    32,
     584,   121,   122,   123,  1008,  1009,   590,   961,  1033,  1008,
    1009,  1010,   702,   961,    27,   998,   733,  1149,    31,   997,
     140,   100,   986,   102,   608,  1008,  1009,  1010,   997,    42,
     616,   617,   100,   619,   102,  1149,   622,   623,    51,   100,
     450,   102,    55,   163,   164,   450,   912,   913,   450,   633,
     634,   100,   450,   102,    67,   100,   100,   102,   102,   100,
      73,   102,   182,   183,   184,   185,   450,   450,    81,   100,
     100,   102,   102,   657,   658,   100,   100,   102,   102,   450,
     450,   665,   666,   375,   100,   205,   102,   959,   101,   100,
     100,   102,   102,   100,   100,   102,   102,   100,   766,   102,
     448,   687,   450,   881,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,  1115,   998,   157,  1116,   910,   160,   495,   162,   990,
    1151,  1152,   282,   879,   717,  1131,  1158,   720,  1133,  1148,
    1224,  1186,   725,   726,   727,   212,   340,   133,   135,   136,
     137,  1151,  1152,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     753,   754,   755,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    53,   218,   219,   220,   221,    -1,    -1,
     616,   617,   909,   619,    -1,    -1,   622,   623,    -1,    -1,
      -1,   784,   785,   786,   787,    75,   789,    -1,    78,   677,
     678,    -1,   795,   796,   201,   202,   203,   204,   801,   802,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   695,   812,    -1,
     814,   101,   816,   817,   818,   222,    -1,    -1,    -1,    -1,
     950,   825,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   961,   836,   837,   838,    -1,    -1,    -1,   378,   842,
      -1,   687,   845,    -1,    -1,    -1,    -1,    -1,    -1,   853,
     854,   855,   856,   857,    -1,   859,   860,    -1,   862,   863,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   997,   998,    -1,
     874,   278,    -1,    -1,    -1,   282,    -1,    -1,    -1,   883,
      -1,   885,    -1,    -1,    -1,   292,    -1,   733,   734,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   167,   168,    -1,    -1,    -1,   910,    -1,    -1,    -1,
      -1,    -1,   758,   759,    -1,    -1,    -1,   457,   458,   459,
     460,   461,   462,   463,   464,   465,    -1,    -1,   932,   933,
     934,   935,   936,   937,    -1,    -1,    -1,    -1,   478,   479,
     480,    -1,   946,    -1,   948,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,  1092,    42,     4,   962,    45,
      -1,    -1,    -1,    -1,   968,   969,    -1,    -1,    -1,    -1,
     976,    -1,   978,   979,    60,    -1,   516,   983,    25,    26,
      27,    67,    29,    30,    31,    -1,   990,    73,    -1,    -1,
      76,    -1,   446,    -1,    -1,    42,  1133,    -1,   538,   539,
      47,    48,    -1,    50,    51,  1011,  1012,    -1,    -1,    -1,
      -1,    -1,  1015,    -1,    -1,   101,     4,    -1,   425,  1149,
      67,  1151,  1152,  1026,  1027,    -1,    73,    -1,    -1,   569,
      -1,    -1,   878,    -1,    81,    -1,    -1,    -1,    26,  1043,
    1044,    29,    -1,    31,   584,  1182,    -1,    -1,    -1,    -1,
     590,    98,    40,    -1,   101,  1058,  1059,    -1,    -1,    47,
      48,    -1,    50,   909,    -1,    -1,    -1,    -1,   608,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,   344,    67,
      -1,    -1,    -1,  1087,    -1,    73,    -1,  1224,    76,    -1,
      -1,    79,  1229,   633,   634,    -1,  1233,   985,    -1,    -1,
    1104,  1105,    -1,    27,    -1,    -1,    30,    31,    96,    -1,
      98,    -1,    -1,   101,    -1,    -1,  1120,  1121,    42,    -1,
      -1,    -1,    -1,    -1,    -1,   665,   666,    -1,    52,    53,
     976,    55,   978,   979,    58,    -1,    -1,   983,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    78,  1159,    -1,    81,    -1,    -1,
      -1,    -1,    -1,  1167,    -1,  1011,  1012,    -1,    -1,    -1,
    1174,   437,   438,    -1,   440,   441,   442,   101,  1184,    -1,
      -1,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,    -1,  1195,    -1,    -1,    -1,  1199,    -1,    -1,    -1,
    1204,    -1,  1206,    -1,    -1,    -1,    -1,    -1,    -1,   616,
     617,    -1,   619,    -1,   621,   622,   623,    -1,    -1,    -1,
     627,   628,   629,    -1,    -1,    -1,    -1,    -1,    -1,   495,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,  1092,    -1,    -1,    -1,
     704,   705,    -1,    -1,    -1,    -1,   663,   711,   524,    -1,
      -1,    -1,    21,    -1,    -1,   719,    -1,    26,    -1,   723,
      29,    -1,    -1,    -1,    -1,   729,    35,    36,    -1,    38,
     687,    40,    41,    -1,    -1,   825,    -1,  1133,    47,    48,
      -1,    50,    -1,    -1,    -1,   702,   703,    -1,    57,   706,
      -1,   708,    -1,   757,    -1,    -1,    -1,    27,    -1,    -1,
      30,    31,    -1,    72,    -1,    -1,    -1,    76,    -1,    -1,
      79,   775,    42,    -1,    -1,    -1,   733,   734,    -1,    -1,
      -1,    51,    -1,    -1,   874,    55,  1182,    -1,  1184,    98,
     794,    -1,   101,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,   758,   759,    73,    -1,    75,    -1,    -1,    78,   109,
      -1,    81,    -1,    -1,    -1,    -1,   820,    -1,    -1,   823,
     910,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1224,    -1,
      -1,   101,    -1,  1229,    -1,    -1,    -1,  1233,    -1,    -1,
      -1,    -1,   932,   933,   934,   935,   936,   937,    -1,    -1,
      -1,   855,    -1,    -1,    -1,    -1,   946,   861,   948,    -1,
     864,    -1,    -1,    -1,   868,    -1,     4,    -1,    -1,    -1,
     874,    -1,   962,   173,   174,    -1,    -1,    -1,   968,   969,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,
      -1,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
     990,     4,    40,    -1,    42,    -1,   206,    -1,    -1,    47,
      48,    -1,    50,    -1,    -1,   215,    -1,    55,    -1,    57,
      -1,    -1,    -1,    26,    -1,    -1,    29,    65,    31,    67,
      -1,    -1,    -1,    -1,     4,    73,    -1,    40,    76,    42,
      -1,    79,   242,    81,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    55,  1043,  1044,    -1,    26,    -1,    96,    29,
      98,    31,   262,   101,    67,    -1,   266,   267,    -1,   269,
      73,    -1,    42,    76,    -1,    -1,    79,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,   990,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    67,   101,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    79,
    1014,    81,    -1,    -1,    -1,  1019,    -1,    -1,   975,   976,
     977,   978,   979,   980,    -1,  1029,   983,    -1,    98,    -1,
      -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   346,    -1,    -1,  1006,
      -1,    -1,    -1,    -1,  1011,  1012,    -1,  1061,    -1,    -1,
      -1,    -1,   362,   363,    -1,   365,    -1,    -1,   368,  1159,
      -1,   371,    -1,    -1,    -1,    -1,    -1,  1167,    -1,   379,
    1084,  1085,    -1,    -1,  1174,    -1,    -1,    -1,    -1,   389,
      -1,    -1,   392,    -1,   394,    -1,    -1,   397,    -1,    -1,
     400,    -1,    -1,   403,   404,    -1,    -1,   407,    -1,    -1,
     410,    -1,    -1,    -1,    -1,   415,    -1,    -1,    -1,   419,
      -1,    -1,    -1,   423,     4,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   950,    -1,    -1,  1141,  1142,    -1,
      -1,    -1,    -1,    -1,    -1,   961,    26,    27,    -1,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,  1115,  1116,
      40,    -1,    42,    -1,    -1,    -1,    46,    47,    48,    -1,
      50,    -1,    52,    53,    -1,    55,    -1,    57,    58,    -1,
      -1,   997,   998,    -1,    -1,  1189,    -1,    67,     4,    -1,
    1194,    -1,    -1,    73,  1198,    -1,    76,    77,    -1,    79,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    -1,    31,    96,    -1,    98,    -1,
      -1,   521,    -1,    -1,    -1,    -1,    42,  1184,  1185,  1186,
      -1,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,  1187,
    1188,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
    1208,    -1,  1210,    79,  1212,    81,    -1,  1215,  1216,    -1,
      -1,    -1,  1220,    -1,    -1,    -1,    -1,  1225,    -1,    -1,
      96,  1229,    98,    -1,  1232,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   603,   604,   605,   606,   607,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   624,   625,   626,    -1,    -1,    -1,
     630,   631,   632,  1149,    -1,  1151,  1152,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   653,   654,   655,   656,    -1,    -1,   659,
     660,   661,   662,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   675,   676,    -1,    -1,   679,
     680,    -1,   682,   683,   684,   685,   686,    -1,   688,   689,
     690,   691,   692,   693,   694,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   735,   736,   737,    -1,   739,
     740,   741,   742,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     760,   761,   762,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   807,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   834,   835,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   865,    -1,    -1,    -1,    -1,
     870,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   886,    -1,    -1,   889,
      -1,   891,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   914,   915,    -1,    -1,    -1,    -1,
      -1,   921,    -1,    -1,   924,    -1,    -1,   927,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   943,    -1,    -1,    -1,   947,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   981,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   991,   992,   993,    -1,   995,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1055,  1056,  1057,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1069,
      -1,    -1,    -1,    -1,  1074,    -1,    -1,  1077,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1088,    -1,
    1090,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1098,     4,
    1100,  1101,    -1,    -1,    -1,    10,    11,  1107,    -1,    -1,
    1110,  1111,    -1,  1113,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,    -1,   101,    -1,    -1,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
      -1,   101,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,    -1,    -1,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
      -1,    -1,    -1,     8,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,    -1,    -1,    -1,     8,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
      -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    14,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,    -1,    -1,    -1,    -1,    -1,
      10,    11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
       5,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,    -1,    -1,    -1,     8,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
      -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   110,   115,   119,   439,     0,     5,     7,    54,    74,
     438,   438,   440,   440,   439,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   111,   112,   424,
     445,   446,   424,   439,   116,   439,   120,   113,   100,   100,
      99,   428,   428,   439,    11,   118,   429,   438,   122,     8,
     430,    56,    64,    82,   117,   124,   125,   139,   121,   123,
     124,   114,   431,   432,   440,   440,   440,   440,   126,   126,
      23,   136,   439,   100,   102,   435,     8,     9,    11,    12,
      16,    17,    18,   433,   444,   445,   112,   442,   444,   112,
     423,   446,   442,    43,    44,   127,   131,   140,   140,   440,
     439,   439,   439,   100,   102,   427,   443,   427,   439,   427,
     440,   440,    28,    31,    59,    67,   141,   142,   143,   144,
     145,   145,   424,   436,   434,   428,   428,   439,   130,   424,
     130,   440,   440,   440,   440,    70,   146,   153,   153,   137,
      11,   101,   425,   445,    99,   101,   128,   132,   442,   442,
     442,   442,   440,   428,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    76,    79,
      98,   101,   154,   155,   169,   174,   180,   226,   230,   231,
     242,   249,   254,   265,   279,   281,   287,   303,   316,   335,
     342,   101,   100,   431,   439,   428,   100,   100,   102,   133,
     427,   427,   427,   427,    12,   112,   151,   440,   440,   440,
     440,   440,   440,   440,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   440,   439,   428,   439,   428,   100,   102,
     437,     8,   428,   428,   428,   152,   147,   424,   103,   112,
     312,   395,   396,   424,   424,   424,   112,   345,   396,   424,
     424,   424,   243,   424,   248,   424,   424,   334,   424,   315,
     424,   179,   424,    11,   112,   426,   446,   447,   424,   139,
     439,   439,   439,   129,   134,   439,   100,   102,   148,   280,
     425,   304,   397,   439,   266,   232,   156,   343,   439,   170,
     227,   175,   100,   100,   255,   100,   102,   336,   100,   102,
     317,   100,   288,   439,   439,   282,   138,   436,    71,   101,
     135,   139,   143,   144,   101,   135,   143,   144,   428,   100,
     102,   283,   100,   395,   100,   102,   267,   100,   102,   233,
     100,   102,   157,   100,   100,   102,   171,   100,   102,   228,
     100,   102,   176,   428,   428,   100,   428,   428,   428,   100,
     102,   289,   283,   101,   101,   440,   428,   149,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   244,   250,   428,
     337,   318,   181,   428,   428,   439,   151,   150,   284,   306,
     268,   234,   158,   344,   172,   229,   177,    27,    30,    42,
      51,    55,    73,    75,    78,    81,   101,   143,   144,   165,
     173,   182,   224,   225,   362,   367,   372,   383,    52,    53,
      58,   101,   143,   144,   165,   173,   182,   224,   225,   362,
     367,   377,   379,   381,   383,   257,   101,   143,   144,   165,
     173,   180,   226,   230,   383,    45,    60,   101,   143,   144,
     165,   173,   180,   226,   323,   331,    73,   101,   143,   144,
     164,   182,   224,   225,   290,   427,   101,   143,   144,   101,
     143,   144,   165,   173,   362,   367,   372,   383,    25,    96,
     143,   144,   164,   173,   230,   273,   305,   314,   335,   362,
     101,   143,   144,   165,   173,   225,   231,   242,   249,   254,
     265,   271,   272,   273,   279,   281,   362,   367,   372,    65,
     101,   143,   144,   165,   173,   180,   226,   230,   314,   335,
     362,   367,   376,   383,    20,   101,   143,   144,   159,   164,
      37,   101,   143,   144,   101,   143,   144,   165,   173,   101,
     143,   144,   165,   180,   226,   230,   314,   335,    22,   101,
     143,   144,   165,   173,   178,   440,   440,   440,   440,   440,
     440,   440,   440,   440,   246,   245,   247,   440,   440,   440,
     252,   251,   253,    46,    77,   143,   144,   165,   173,   180,
     226,   230,   256,   314,   335,   362,   367,   377,   379,   381,
     383,   386,   389,   339,   340,   341,   338,   439,   439,   319,
     320,   321,   322,   440,   428,    21,    68,   101,   143,   144,
     165,   173,   294,   299,   362,   285,   286,   440,   440,   308,
     311,   101,   309,   310,   307,   270,   269,   440,   240,   237,
     241,   238,   239,   235,   236,   440,   440,   428,   428,   428,
     428,   428,   440,   368,   442,   442,   373,   442,   166,   183,
     426,   442,   442,   428,   428,   428,    17,    94,   112,   380,
     419,    18,   112,   378,   419,   420,    92,    95,   112,   382,
     428,   428,   428,   440,   440,   260,   261,   264,   101,   262,
     263,   258,   259,   428,   428,   428,   428,   324,   332,   428,
     428,   428,   428,    85,    87,    90,   112,   168,   440,   440,
     292,   293,   291,   428,   428,   424,   313,   424,   428,   428,
     428,   428,   428,   428,   428,   442,   428,   428,   428,   428,
     428,   428,   428,   424,    84,    86,    89,    91,   346,   347,
     348,   349,   353,   357,   426,    88,    93,   112,   370,   427,
     427,    88,    93,   112,   375,   384,   168,   100,   102,   184,
     427,   363,   439,   427,   439,   427,   439,   439,   439,   427,
     112,   390,   425,   112,   387,   398,   425,   428,   428,   428,
     428,   428,   428,   428,   100,   100,   439,   439,   439,   427,
     112,   302,   398,   112,   297,   398,   428,   428,   428,   274,
     317,   427,   160,   439,   439,   439,   439,   439,   427,   439,
     439,   369,   439,   439,   374,   100,   102,   385,   167,   428,
     100,   102,   364,   427,   391,   427,   388,   427,   440,   399,
     428,   428,   300,   439,   295,   439,   100,   102,   275,   100,
     102,   161,   350,   354,   427,   358,   427,   427,   428,   427,
     185,   186,   199,   200,   213,   214,   428,   392,   427,   440,
     398,   397,   326,   326,   100,   100,   102,   296,   428,   428,
     428,   100,   102,   351,   100,   102,   355,   100,   102,   360,
     193,   101,    39,    49,    61,    62,    66,    69,   178,   187,
     188,   190,   194,   208,   210,   211,    32,   202,    24,   216,
     365,   425,   388,   180,   226,   230,   325,   383,   333,   428,
     428,   276,    83,   162,   428,   428,   359,   428,    33,    34,
     101,   143,   144,   222,   223,   440,   440,   440,   440,   440,
     440,   182,   428,   440,   428,   440,   215,   101,   143,   144,
     393,   328,   329,   330,   101,   327,   101,   306,   298,   101,
     143,   144,   165,   173,   230,   362,   440,   101,   352,   356,
     361,   440,   440,   112,   189,   419,   191,   442,   112,   400,
     401,   402,   195,   442,   394,   442,    88,    93,   112,   212,
     428,   203,   442,   201,   217,   424,   216,   392,   428,   428,
     428,   428,   301,    27,    51,   101,   143,   144,   173,   225,
     366,   371,   376,   377,   379,   383,   278,   277,    88,    93,
     112,   163,   101,   224,   225,   366,   371,   377,   379,   383,
     386,   101,   224,   225,   383,   386,   101,   182,   224,   225,
     366,   371,   377,   379,   442,   442,   427,   439,   100,   102,
     192,   427,   103,   403,   404,    14,   406,   407,   100,   102,
     196,   100,   102,   209,   439,   439,   427,   100,   102,   204,
     202,   100,   102,   218,   101,   440,   440,   428,   428,   428,
     439,   439,   427,   428,   428,   428,   427,   427,   428,   428,
     425,   405,   103,   408,   428,   428,   428,   428,   428,   428,
     428,   370,   375,   193,   411,   403,   407,   409,   425,   197,
     193,   205,   219,   427,   427,   101,   104,   410,   411,    97,
     101,   143,   144,   198,   222,   223,   101,    80,   101,   143,
     144,   164,   173,   206,    63,   101,   143,   144,   164,   173,
     220,     5,   441,   404,   440,   440,   440,   412,   425,   442,
      16,    17,    18,   112,   207,   421,   422,   112,   221,   420,
     441,   441,   427,   427,   439,   427,   439,   105,   106,   441,
      85,   413,   418,   441,   441,   107,   103,   441,   441,   108,
      14,   414,   415,   441,   416,   103,   415,   425,   441,   417,
     425,   441,   103,   441,   425
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   109,   110,   110,   111,   112,   113,   114,   113,   116,
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
     176,   176,   177,   177,   177,   177,   177,   177,   178,   179,
     180,   181,   181,   181,   181,   181,   181,   181,   182,   183,
     184,   184,   185,   185,   185,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   187,   188,   189,   189,   190,   191,
     192,   192,   193,   193,   193,   193,   193,   194,   195,   196,
     196,   197,   197,   197,   197,   197,   197,   198,   200,   199,
     201,   201,   202,   203,   204,   204,   205,   205,   205,   205,
     205,   205,   206,   207,   207,   208,   209,   209,   210,   211,
     212,   212,   212,   214,   213,   215,   215,   216,   217,   218,
     218,   219,   219,   219,   219,   219,   219,   220,   221,   221,
     222,   223,   224,   225,   227,   226,   228,   228,   229,   229,
     229,   229,   229,   229,   229,   229,   229,   230,   230,   230,
     230,   230,   230,   230,   230,   232,   231,   233,   233,   234,
     235,   234,   234,   236,   234,   234,   234,   234,   234,   234,
     237,   234,   238,   234,   239,   234,   240,   234,   241,   234,
     242,   243,   244,   245,   244,   244,   246,   244,   244,   247,
     244,   244,   244,   244,   244,   244,   244,   248,   249,   250,
     251,   250,   250,   252,   250,   250,   250,   253,   250,   250,
     250,   250,   250,   250,   250,   250,   255,   256,   254,   257,
     258,   257,   257,   259,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   260,   257,   261,   257,   262,   257,
     263,   257,   264,   257,   266,   265,   267,   267,   268,   269,
     268,   268,   268,   268,   268,   268,   268,   268,   270,   268,
     271,   271,   272,   272,   272,   272,   272,   272,   272,   274,
     273,   275,   275,   276,   277,   276,   276,   276,   276,   276,
     278,   276,   280,   279,   282,   281,   283,   283,   284,   285,
     284,   284,   286,   284,   284,   284,   284,   284,   284,   288,
     287,   289,   289,   290,   291,   290,   290,   290,   290,   290,
     292,   290,   293,   290,   295,   294,   296,   296,   297,   297,
     298,   298,   298,   298,   298,   298,   298,   298,   298,   298,
     298,   300,   301,   299,   302,   302,   304,   305,   303,   306,
     307,   306,   306,   306,   306,   306,   308,   306,   309,   306,
     310,   306,   311,   306,   312,   312,   313,   314,   315,   316,
     317,   317,   318,   318,   318,   318,   318,   319,   318,   320,
     318,   321,   318,   322,   318,   324,   325,   323,   326,   327,
     326,   328,   326,   329,   326,   330,   326,   332,   333,   331,
     334,   335,   336,   336,   337,   338,   337,   337,   337,   337,
     337,   339,   337,   340,   337,   341,   337,   343,   342,   344,
     344,   344,   344,   345,   345,   346,   346,   347,   347,   347,
     348,   350,   349,   351,   351,   352,   352,   352,   352,   352,
     352,   352,   352,   352,   354,   353,   355,   355,   356,   356,
     356,   356,   356,   358,   357,   359,   360,   360,   361,   361,
     361,   361,   361,   361,   361,   361,   363,   362,   364,   364,
     365,   365,   365,   366,   368,   369,   367,   370,   370,   370,
     371,   373,   374,   372,   375,   375,   375,   376,   377,   378,
     378,   379,   380,   380,   380,   381,   382,   382,   382,   384,
     383,   385,   385,   386,   387,   387,   388,   388,   389,   391,
     390,   390,   393,   392,   392,   394,   395,   396,   397,   397,
     399,   398,   401,   400,   402,   400,   400,   403,   404,   405,
     405,   406,   407,   408,   408,   409,   410,   410,   411,   411,
     412,   413,   414,   415,   416,   416,   417,   417,   418,   419,
     420,   420,   421,   421,   422,   422,   423,   423,   424,   424,
     425,   425,   426,   426,   426,   427,   427,   428,   428,   428,
     430,   429,   431,   432,   432,   433,   433,   433,   434,   434,
     435,   435,   436,   436,   437,   437,   438,   438,   439,   439,
     440,   441,   441,   443,   442,   442,   444,   444,   444,   444,
     444,   444,   444,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   446,   447
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     0,     0,     6,     0,
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
       1,     4,     0,     2,     2,     2,     2,     2,     4,     1,
       7,     0,     3,     2,     2,     2,     2,     2,     4,     1,
       1,     4,     1,     1,     1,     0,     2,     2,     2,     2,
       2,     2,     3,     4,     0,     4,     2,     1,     5,     1,
       1,     4,     0,     2,     2,     2,     2,     5,     1,     1,
       4,     0,     2,     2,     2,     2,     2,     4,     0,     4,
       0,     3,     4,     1,     1,     4,     0,     2,     2,     2,
       2,     2,     4,     2,     1,     4,     1,     4,     4,     4,
       2,     2,     1,     0,     3,     0,     2,     5,     1,     1,
       4,     0,     2,     2,     2,     2,     2,     4,     2,     1,
       4,     4,     4,     4,     0,     5,     1,     4,     0,     2,
       2,     2,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     5,     1,     4,     0,
       0,     4,     2,     0,     4,     2,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       7,     1,     0,     0,     4,     2,     0,     4,     2,     0,
       4,     2,     2,     2,     2,     2,     2,     1,     7,     0,
       0,     4,     2,     0,     4,     2,     2,     0,     4,     2,
       2,     2,     2,     2,     2,     2,     0,     0,     9,     0,
       0,     4,     2,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     5,     1,     4,     0,     0,
       4,     2,     2,     2,     2,     2,     2,     2,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     5,     0,     5,     1,     4,     0,     0,
       4,     2,     0,     4,     2,     2,     2,     2,     2,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     5,     1,     4,     2,     1,
       0,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     0,     0,     9,     2,     1,     0,     0,     9,     0,
       0,     4,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     4,     2,     1,     1,     4,     1,     4,
       1,     4,     0,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     0,     8,     0,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     0,     8,
       1,     4,     1,     4,     0,     0,     4,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     8,     0,
       2,     2,     4,     2,     1,     1,     2,     1,     1,     1,
       3,     0,     4,     1,     4,     0,     2,     3,     2,     2,
       2,     2,     2,     2,     0,     4,     1,     4,     0,     2,
       3,     2,     2,     0,     4,     1,     1,     4,     0,     3,
       2,     2,     2,     2,     2,     2,     0,     5,     1,     4,
       0,     2,     2,     4,     0,     0,     6,     2,     2,     1,
       4,     0,     0,     6,     2,     2,     1,     4,     4,     2,
       1,     4,     2,     2,     1,     4,     2,     2,     1,     0,
       5,     1,     4,     3,     2,     2,     3,     1,     3,     0,
       3,     2,     0,     4,     1,     1,     2,     2,     0,     2,
       0,     3,     0,     2,     0,     2,     1,     3,     2,     0,
       2,     3,     2,     0,     2,     2,     0,     2,     0,     6,
       5,     5,     5,     4,     0,     2,     0,     5,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     1,
       1,     1,     2,     2,     1,     2,     4,     0,     2,     2,
       0,     4,     2,     0,     1,     0,     2,     3,     0,     5,
       1,     4,     0,     4,     2,     5,     1,     1,     0,     2,
       2,     0,     1,     0,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
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
          case 111: /* tmp_string  */

      { free((((*yyvaluep).p_str)) ? *((*yyvaluep).p_str) : NULL); }

        break;

    case 130: /* tmp_identifier_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 179: /* typedef_arg_str  */

      { if (read_all) {
                yang_delete_type(module, (struct yang_type *)((*yyvaluep).nodes).node.ptr_tpdf->type.der);
              }
            }

        break;

    case 195: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 243: /* leaf_arg_str  */

      { if (read_all) {
                yang_delete_type(module, (struct yang_type *)((*yyvaluep).nodes).node.ptr_leaf->type.der);
              }
            }

        break;

    case 248: /* leaf_list_arg_str  */

      { if (read_all) {
                yang_delete_type(module, (struct yang_type *)((*yyvaluep).nodes).node.ptr_leaf->type.der);
              }
            }

        break;

    case 344: /* deviation_opt_stmt  */

      { if (read_all) {
                ly_set_free(((*yyvaluep).nodes).deviation->dflt_check);
                free(((*yyvaluep).nodes).deviation);
              }
            }

        break;

    case 359: /* deviate_replace_stmtsep  */

      { if (read_all && ((*yyvaluep).nodes).deviation->deviate->type) {
                yang_delete_type(module, (struct yang_type *)((*yyvaluep).nodes).deviation->deviate->type->der);
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
struct lys_node_uses *refine_parent = NULL;
int config_inherit = 0, actual_type = 0;
int64_t cnt_val;
int read_string = read_all;
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

    { if (read_string) {
                      if (yyget_text(scanner)[0] == '"') {
                        char *tmp;

                        s = malloc(yyget_leng(scanner) - 1 + 7 * yylval.i);
                        if (!s) {
                          LOGMEM;
                          YYABORT;
                        }
                        if (!(tmp = yang_read_string(yyget_text(scanner) + 1, s, yyget_leng(scanner) - 2, 0, yylloc.first_column, (trg) ? trg->version : 0))) {
                          YYABORT;
                        }
                        s = tmp;
                      } else {
                        s = calloc(1, yyget_leng(scanner) - 1);
                        if (!s) {
                          LOGMEM;
                          YYABORT;
                        }
                        memcpy(s, yyget_text(scanner) + 1, yyget_leng(scanner) - 2);
                      }
                      (yyval.p_str) = &s;
                    } else {
                      (yyval.p_str) = NULL;
                    }
                  }

    break;

  case 7:

    { if (read_string && (yyget_leng(scanner) > 2)) {
                int length_s = strlen(s), length_tmp = yyget_leng(scanner);
                char *tmp;

                tmp = realloc(s, length_s + length_tmp - 1);
                if (!tmp) {
                  LOGMEM;
                  YYABORT;
                }
                s = tmp;
                if (yyget_text(scanner)[0] == '"') {
                  if (!(tmp = yang_read_string(yyget_text(scanner) + 1, s, length_tmp - 2, length_s, yylloc.first_column, (trg) ? trg->version : 0))) {
                    YYABORT;
                  }
                  s = tmp;
                } else {
                  memcpy(s + length_s, yyget_text(scanner) + 1, length_tmp - 2);
                  s[length_s + length_tmp - 2] = '\0';
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
                                                              config_inherit = CONFIG_INHERIT_ENABLE;
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
                                                                    config_inherit = CONFIG_INHERIT_ENABLE;
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

    { if (read_all) {
                                                  if (yang_check_version(module, submodule, s, (yyvsp[-1].i))) {
                                                    YYABORT;
                                                  }
                                                  (yyval.i) = 1;
                                                  s = NULL;
                                                }
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
                             config_inherit = CONFIG_INHERIT_ENABLE;
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
                                        config_inherit = CONFIG_INHERIT_ENABLE;
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

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 102:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

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

    { if (read_string) {
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
                             if (store_flags((struct lys_node *)actual, size_arrays->node[size_arrays->next].flags, CONFIG_INHERIT_DISABLE)) {
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
                                        tpdf_parent = actual;
                                        if (!(actual = yang_read_typedef(trg, actual, s))) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        actual_type = TYPEDEF_KEYWORD;
                                        (yyval.nodes).node.ptr_tpdf = actual;
                                      }
                                    }

    break;

  case 130:

    { if (read_all) {
                      if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                        YYABORT;
                      }
                      if (unres_schema_add_node(trg, unres, &(yyvsp[-1].nodes).node.ptr_tpdf->type, UNRES_TYPE_DER_TPDF, tpdf_parent) == -1) {
                        lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-1].nodes).node.ptr_tpdf->type.der)->name);
                        free((yyvsp[-1].nodes).node.ptr_tpdf->type.der);
                        (yyvsp[-1].nodes).node.ptr_tpdf->type.der = NULL;
                        YYABORT;
                      }
                      actual = tpdf_parent;

                      /* check default value */
                      if (unres_schema_add_str(trg, unres, &(yyvsp[-1].nodes).node.ptr_tpdf->type, UNRES_TYPE_DFLT, (yyvsp[-1].nodes).node.ptr_tpdf->dflt) == -1) {
                        YYABORT;
                      }
                    }
                  }

    break;

  case 131:

    { (yyval.nodes).node.ptr_tpdf = actual;
                        (yyval.nodes).node.flag = 0;
                      }

    break;

  case 132:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).node.ptr_tpdf;
                                         actual_type = TYPEDEF_KEYWORD;
                                         (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                         (yyval.nodes) = (yyvsp[-2].nodes);
                                       }
                                     }

    break;

  case 133:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 134:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 135:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 136:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 137:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 139:

    { if (read_all) {
                                         if(!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }
                                     }

    break;

  case 145:

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
                                     if (strcmp(((struct yang_type *)actual)->name, "union")) {
                                       /* type can be a substatement only in "union" type, not in derived types */
                                       LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "type", "derived type");
                                       YYABORT;
                                     }
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

  case 146:

    { if (read_all && yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 147:

    { if (read_all) {
                                     /* leafref_specification */
                                     if (yang_read_leafref_path(trg, actual, s)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }
                                 }

    break;

  case 148:

    { if (read_all) {
                                     /* identityref_specification */
                                     if (yang_read_identyref(trg, actual, s , unres)) {
                                       YYABORT;
                                     }
                                     s = NULL;
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
                 errno = 0;

                 val = strtoul(s, &endptr, 10);
                 if (*endptr || s[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
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

    { if (read_all) {
                                                                          if (yang_read_pattern(trg, actual, (yyvsp[-2].str), (yyvsp[-1].ch))) {
                                                                            YYABORT;
                                                                          }
                                                                          actual = yang_type;
                                                                          actual_type = TYPE_KEYWORD;
                                                                        }
                                                                      }

    break;

  case 168:

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

  case 169:

    { (yyval.ch) = 0x06; }

    break;

  case 170:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 171:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 172:

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

  case 173:

    { if (read_all && yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 174:

    { if (read_all && yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 175:

    { if (read_all && yang_read_description(trg, actual, s, "pattern")) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 176:

    { if (read_all && yang_read_reference(trg, actual, s, "pattern")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 177:

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

  case 178:

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

  case 182:

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

  case 183:

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

  case 186:

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

  case 187:

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

  case 188:

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

  case 189:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 190:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 191:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 192:

    { if (read_all) {
                        ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                        /* keep the highest enum value for automatic increment */
                        if ((yyvsp[-1].i) >= cnt_val) {
                          cnt_val = (yyvsp[-1].i);
                          cnt_val++;
                        }
                      }
                    }

    break;

  case 193:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 194:

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

  case 195:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 199:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 200:

    { (yyval.i) = 1; }

    break;

  case 201:

    { (yyval.i) = -1; }

    break;

  case 202:

    { if (read_all) {
                  if (!strcmp(s,"true")) {
                    (yyval.i) = 1;
                  } else if (!strcmp(s,"false")) {
                    (yyval.i) = -1;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "require-instance");
                    free(s);
                    YYABORT;
                  }
                  free(s);
                  s = NULL;
                }
              }

    break;

  case 203:

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

  case 207:

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

  case 208:

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

  case 211:

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

  case 212:

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

  case 213:

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

  case 214:

    { if (read_all) {
                                  if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 215:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 216:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 217:

    { if (read_all) {
                           ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                           /* keep the highest position value for automatic increment */
                           if ((yyvsp[-1].uint) >= cnt_val) {
                             cnt_val = (yyvsp[-1].uint);
                             cnt_val++;
                           }
                         }
                       }

    break;

  case 218:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 219:

    { if (read_all) {
                  /* convert it to uint32_t */
                  unsigned long val;
                  char *endptr = NULL;
                  errno = 0;

                  val = strtoul(s, &endptr, 10);
                  if (s[0] == '-' || *endptr || errno || val > UINT32_MAX) {
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

  case 224:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
                                                         }
                                                       }

    break;

  case 226:

    { if (read_all) {
                      if (store_flags((struct lys_node *)actual, 0, config_inherit)) {
                        YYABORT;
                      }
                    }
                  }

    break;

  case 228:

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
                               if (store_flags((struct lys_node *)(yyval.nodes).grouping, size_arrays->node[size_arrays->next].flags, config_inherit)) {
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

  case 229:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 230:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 231:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 232:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 233:

    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }

    break;

  case 234:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 235:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 236:

    { if (read_all) {
                                                     actual = (yyvsp[-2].nodes).grouping;
                                                     actual_type = GROUPING_KEYWORD;
                                                     if (trg->version < 2) {
                                                       LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "notification");
                                                       YYABORT;
                                                     }
                                                   }
                                                 }

    break;

  case 245:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 247:

    { if (read_all) {
                       if (store_flags((struct lys_node *)actual, 0, config_inherit)) {
                         YYABORT;
                       }
                     }
                   }

    break;

  case 248:

    { if (read_all) {
              /* check XPath dependencies */
              if (((yyvsp[-1].nodes).container->when || (yyvsp[-1].nodes).container->must_size) && (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).container, UNRES_XPATH, NULL) == -1)) {
                YYABORT;
              }
            }
          }

    break;

  case 249:

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

  case 250:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 252:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 253:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 255:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 256:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 257:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 258:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 259:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 260:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 262:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 264:

    { if (read_all) {
                                              actual = (yyvsp[-1].nodes).container;
                                              actual_type = CONTAINER_KEYWORD;
                                              data_node = actual;
                                              if (trg->version < 2) {
                                                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "notification");
                                                YYABORT;
                                              }
                                            }
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
                    if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "type", "leaf");
                      YYABORT;
                    }
                    if (unres_schema_add_node(trg, unres, &(yyvsp[-1].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *)(yyvsp[-1].nodes).node.ptr_leaf) == -1) {
                      lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-1].nodes).node.ptr_leaf->type.der)->name);
                      free((yyvsp[-1].nodes).node.ptr_leaf->type.der);
                      (yyvsp[-1].nodes).node.ptr_leaf->type.der = NULL;
                      YYABORT;
                    }
                    if ((yyvsp[-1].nodes).node.ptr_leaf->dflt && ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE)) {
                      /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                      LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                      LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                      YYABORT;
                    }
                    if (unres_schema_add_str(trg, unres, &(yyvsp[-1].nodes).node.ptr_leaf->type, UNRES_TYPE_DFLT, (yyvsp[-1].nodes).node.ptr_leaf->dflt) == -1) {
                      YYABORT;
                    }
                    /* check XPath dependencies */
                    if (((yyvsp[-1].nodes).node.ptr_leaf->when || (yyvsp[-1].nodes).node.ptr_leaf->must_size) &&
                        (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).node.ptr_leaf, UNRES_XPATH, NULL) == -1)) {
                      YYABORT;
                    }
                  }
                }

    break;

  case 271:

    { if (read_all) {
                                     if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYABORT;}
                                     data_node = actual;
                                     s=NULL;
                                     (yyval.nodes).node.ptr_leaf = actual;
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

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                                 }
                               }

    break;

  case 277:

    { (yyval.nodes) = (yyvsp[-3].nodes);}

    break;

  case 278:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 279:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 281:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;}
                                    s = NULL;
                                  }

    break;

  case 282:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i), 0)) {
                                                 YYABORT;
                                               }
                                             }
                                           }

    break;

  case 283:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 284:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 285:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 286:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 287:

    { if (read_all) {
                                          if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {
                                            YYABORT;
                                          }
                                          data_node = actual;
                                          s=NULL;
                                          (yyval.nodes).node.ptr_leaflist = actual;
                                        }
                                      }

    break;

  case 288:

    { if (read_all) {
                          int i;

                          if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_R) {
                            /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                             * ignore oredering MASK - 0x7F
                             */
                            (yyvsp[-1].nodes).node.ptr_leaflist->flags &= 0x7F;
                          }
                          if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYABORT;
                          }
                          if (unres_schema_add_node(trg, unres, &(yyvsp[-1].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER, (struct lys_node *)(yyvsp[-1].nodes).node.ptr_leaflist) == -1) {
                            lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-1].nodes).node.ptr_leaflist->type.der)->name);
                            free((yyvsp[-1].nodes).node.ptr_leaflist->type.der);
                            (yyvsp[-1].nodes).node.ptr_leaflist->type.der = NULL;
                            YYABORT;
                          }
                          if ((yyvsp[-1].nodes).node.ptr_leaflist->dflt_size && (yyvsp[-1].nodes).node.ptr_leaflist->min) {
                            LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "min-elements", "leaf-list");
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                                   "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
                            YYABORT;
                          }

                          /* check default value (if not defined, there still could be some restrictions
                           * that need to be checked against a default value from a derived type) */
                          for (i = 0; i < (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size; i++) {
                            if (unres_schema_add_str(module, unres, &(yyvsp[-1].nodes).node.ptr_leaflist->type, UNRES_TYPE_DFLT, (yyvsp[-1].nodes).node.ptr_leaflist->dflt[i]) == -1) {
                              YYABORT;
                            }
                          }
                          /* check XPath dependencies */
                          if (((yyvsp[-1].nodes).node.ptr_leaflist->when || (yyvsp[-1].nodes).node.ptr_leaflist->must_size) &&
                              (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).node.ptr_leaflist, UNRES_XPATH, NULL) == -1)) {
                            YYABORT;
                          }
                        }
                      }

    break;

  case 289:

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

  case 290:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 292:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 293:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                      (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                                    }
                                  }

    break;

  case 294:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 295:

    { if (read_all) {
                                         int i;

                                         /* check for duplicity in case of configuration data,
                                          * in case of status data duplicities are allowed */
                                         if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_W) {
                                           for (i = 0; i < (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size; i++) {
                                             if (ly_strequal((yyvsp[-1].nodes).node.ptr_leaflist->dflt[i], s, 0)) {
                                               LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "default");
                                               LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", s);
                                               free(s);
                                               YYABORT;
                                             }
                                           }
                                         }
                                         (yyvsp[-1].nodes).node.ptr_leaflist->dflt[(yyvsp[-1].nodes).node.ptr_leaflist->dflt_size++] = lydict_insert_zc(module->ctx, s);
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].dflt++;
                                       }
                                     }

    break;

  case 296:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 297:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 299:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 300:

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

  case 301:

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

  case 302:

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

  case 303:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 304:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 305:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 306:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 307:

    { if (read_all) {
                                 struct lys_node *node;

                                 if ((yyvsp[0].nodes).node.ptr_list->flags & LYS_CONFIG_R) {
                                   /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                    * ignore oredering MASK - 0x7F
                                    */
                                   (yyvsp[0].nodes).node.ptr_list->flags &= 0x7F;
                                 }
                                 /* check - if list is configuration, key statement is mandatory
                                  * (but only if we are not in a grouping or augment, then the check is deferred) */
                                 for (node = (struct lys_node *)(yyvsp[0].nodes).node.ptr_list; node && !(node->nodetype & (LYS_GROUPING | LYS_AUGMENT)); node = node->parent);
                                 if (!node && ((yyvsp[0].nodes).node.ptr_list->flags & LYS_CONFIG_W) && !(yyvsp[0].nodes).node.ptr_list->keys) {
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
                                 /* check XPath dependencies */
                                 if (((yyvsp[0].nodes).node.ptr_list->when || (yyvsp[0].nodes).node.ptr_list->must_size) &&
                                     (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_list, UNRES_XPATH, NULL) == -1)) {
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 309:

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

  case 310:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 312:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 313:

    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }

    break;

  case 315:

    { if (read_all) {
                                if ((yyvsp[-1].nodes).node.ptr_list->keys) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "key", "list");
                                  free(s);
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

  case 316:

    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }

    break;

  case 317:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 318:

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

  case 319:

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

  case 320:

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

  case 321:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 322:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 323:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 324:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }

    break;

  case 326:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 328:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 330:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).node.ptr_list;
                                         actual_type = LIST_KEYWORD;
                                         data_node = actual;
                                         if (trg->version < 2) {
                                           LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "notification");
                                           YYABORT;
                                         }
                                       }
                                     }

    break;

  case 332:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 333:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 334:

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

  case 336:

    { if (read_all) {
                    if (store_flags((struct lys_node *)actual, 0, config_inherit)) {
                      YYABORT;
                    }
                  }
                }

    break;

  case 337:

    { if (read_all) {
             /* check XPath dependencies */
             if ((yyvsp[-1].nodes).node.ptr_choice->when &&
                 (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).node.ptr_choice, UNRES_XPATH, NULL) == -1)) {
               YYABORT;
             }
           }
         }

    break;

  case 338:

    { if (read_all) {
                            (yyval.nodes).node.ptr_choice = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = CHOICE_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).node.ptr_choice->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_choice->iffeature);
                              if (!(yyval.nodes).node.ptr_choice->iffeature) {
                                LOGMEM;
                                YYABORT;
                              }
                            }
                            if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_choice, size_arrays->node[size_arrays->next].flags, config_inherit)) {
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

  case 339:

    { actual = (yyvsp[-1].nodes).node.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 340:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 341:

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

  case 342:

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

  case 343:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 344:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 345:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 346:

    { if (read_all) {
                                          if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 347:

    { if (read_all) {
                                        if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }
                                    }

    break;

  case 348:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 349:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 358:

    { if (read_all && trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 359:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 361:

    { if (read_all) {
                  if (store_flags((struct lys_node *)actual, 0, config_inherit)) {
                    YYABORT;
                  }
                }
              }

    break;

  case 362:

    { if (read_all) {
              /* check XPath dependencies */
              if ((yyvsp[-1].nodes).cs->when &&
                  (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).cs, UNRES_XPATH, NULL) == -1)) {
                YYABORT;
              }
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
                          if (store_flags((struct lys_node *)(yyval.nodes).cs, size_arrays->node[size_arrays->next].flags, config_inherit)) {
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

  case 374:

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

  case 376:

    { if (read_all) {
                    if (store_flags((struct lys_node *)actual, 0, config_inherit)) {
                      YYABORT;
                    }
                  }
                }

    break;

  case 377:

    { if (read_all) {
             /* check XPath dependencies */
             if (((yyvsp[-1].nodes).node.ptr_anydata->when || (yyvsp[-1].nodes).node.ptr_anydata->must_size) &&
                 (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).node.ptr_anydata, UNRES_XPATH, NULL) == -1)) {
               YYABORT;
             }
           }
         }

    break;

  case 378:

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

  case 379:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                   actual_type = (yyvsp[-1].nodes).node.flag;
                                 }
                               }

    break;

  case 381:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, unres, (yyvsp[-1].nodes).node.flag)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 382:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                   actual_type = (yyvsp[-1].nodes).node.flag;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 384:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config",
                                                               (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 385:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory",
                                                                  (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 386:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status",
                                                               (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 387:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 388:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 389:

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

  case 390:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYABORT;
                        }
                        /* check XPath dependencies */
                        if (((struct lys_node_uses *)actual)->when &&
                            (unres_schema_add_node(trg, unres, actual, UNRES_XPATH, NULL) == -1)) {
                          YYABORT;
                        }
                      }
                    }

    break;

  case 393:

    { if (read_all) {
                          (yyval.nodes).uses = actual;
                          actual_type = USES_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).uses->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).uses->iffeature);
                            if (!(yyval.nodes).uses->iffeature) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].refine) {
                            (yyval.nodes).uses->refine = calloc(size_arrays->node[size_arrays->next].refine, sizeof *(yyval.nodes).uses->refine);
                            if (!(yyval.nodes).uses->refine) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].augment) {
                            (yyval.nodes).uses->augment = calloc(size_arrays->node[size_arrays->next].augment, sizeof *(yyval.nodes).uses->augment);
                            if (!(yyval.nodes).uses->augment) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                          if (store_flags((struct lys_node *)(yyval.nodes).uses, size_arrays->node[size_arrays->next].flags, CONFIG_INHERIT_DISABLE)) {
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

  case 394:

    { actual = (yyvsp[-1].nodes).uses; actual_type = USES_KEYWORD; }

    break;

  case 396:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 397:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 398:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 399:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 400:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 402:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).uses;
                                         actual_type = USES_KEYWORD;
                                         data_node = actual;
                                         if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                           data_node = NULL;
                                         }
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].augment++;
                                       }
                                     }

    break;

  case 404:

    { if (read_all) {
                                                   refine_parent = actual;
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 410:

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
                                  if (size_arrays->node[size_arrays->next].if_features) {
                                    (yyval.nodes).refine->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).refine->iffeature);
                                    if (!(yyval.nodes).refine->iffeature) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                    if (trg->version < 2) {
                                      LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                                      YYABORT;
                                    }
                                    /* leaf, leaf-list, list, container or anyxml */
                                    /* check possibility of statements combination */
                                    if ((yyval.nodes).refine->target_type) {
                                        (yyval.nodes).refine->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA);
                                    } else {
                                        (yyval.nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA;
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

  case 411:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 412:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, refine_parent, s, unres, REFINE_KEYWORD)) {
                                                 YYABORT;
                                               }
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 413:

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

  case 414:

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

  case 415:

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

  case 416:

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

  case 417:

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

  case 418:

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

  case 419:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 420:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 421:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYABORT;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 422:

    { if (read_all) {
                                            if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                              YYABORT;
                                            }
                                            config_inherit = (yyvsp[0].nodes).node.flag & CONFIG_MASK;
                                            /* check XPath dependencies */
                                            if ((yyvsp[0].nodes).node.ptr_augment->when &&
                                                (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_augment, UNRES_XPATH, NULL) == -1)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 426:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYABORT;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 427:

    { if (read_all) {
                                       if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                         LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "augment");
                                         YYABORT;
                                       }
                                       if (unres_schema_add_node(trg, unres, actual, UNRES_AUGMENT, NULL) == -1) {
                                         YYABORT;
                                       }
                                       config_inherit = (yyvsp[0].nodes).node.flag & CONFIG_MASK;
                                       /* check XPath dependencies */
                                       if ((yyvsp[0].nodes).node.ptr_augment->when &&
                                           (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_augment, UNRES_XPATH, NULL) == -1)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 429:

    { if (read_all) {
                             (yyval.nodes).node.ptr_augment = actual;
                             (yyval.nodes).node.flag = config_inherit;
                             config_inherit = CONFIG_INHERIT_ENABLE;
                             actual_type = AUGMENT_KEYWORD;
                             if (size_arrays->node[size_arrays->next].if_features) {
                               (yyval.nodes).node.ptr_augment->iffeature = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_augment->iffeature);
                               if (!(yyval.nodes).node.ptr_augment->iffeature) {
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

  case 430:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 432:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 433:

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

  case 434:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 435:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 436:

    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }

    break;

  case 437:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 438:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                      data_node = actual;
                                    }
                                  }

    break;

  case 439:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 440:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_augment;
                                            actual_type = AUGMENT_KEYWORD;
                                            (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                            data_node = actual;
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "notification");
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 441:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 442:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 443:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 446:

    { if (read_all) {
                                       if (!(actual = yang_read_action(trg, actual, s))) {
                                         YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                     }
                                     (yyval.i) = config_inherit;
                                     config_inherit = CONFIG_IGNORE;
                                   }

    break;

  case 447:

    { config_inherit = (yyvsp[-1].i); }

    break;

  case 448:

    { if (read_all) {
                                    if (!(actual = yang_read_node(trg, NULL, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                      YYABORT;
                                    }
                                    data_node = actual;
                                    s = NULL;
                                  }
                                  (yyval.i) = config_inherit;
                                  config_inherit = CONFIG_IGNORE;
                                }

    break;

  case 449:

    { config_inherit = (yyvsp[-1].i); }

    break;

  case 452:

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
                         if (store_flags((struct lys_node *)(yyval.nodes).node.ptr_rpc, size_arrays->node[size_arrays->next].flags, CONFIG_INHERIT_DISABLE)) {
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

  case 453:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 454:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 455:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 456:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 457:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 459:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 461:

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

  case 462:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 463:

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

  case 464:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 465:

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

  case 466:

    { if (read_all) {
                                          if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                            YYABORT;
                                          }
                                          /* check XPath dependencies */
                                          if ((yyvsp[0].nodes).node.ptr_inout->must_size &&
                                              (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_inout, UNRES_XPATH, NULL) == -1)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 468:

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

  case 469:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).node.ptr_inout;
                                         actual_type = INPUT_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                       }
                                     }

    break;

  case 471:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 473:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 475:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 476:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 477:

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

  case 478:

    { if (read_all) {
                                           if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                             LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                             YYABORT;
                                           }
                                           /* check XPath dependencies */
                                           if ((yyvsp[0].nodes).node.ptr_inout->must_size &&
                                               (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_inout, UNRES_XPATH, NULL) == -1)) {
                                             YYABORT;
                                           }
                                         }
                                       }

    break;

  case 480:

    { if (read_all) {
                                             if (!(actual = yang_read_node(trg, actual, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                              YYABORT;
                                             }
                                             data_node = actual;
                                           }
                                           (yyval.i) = config_inherit;
                                           config_inherit = CONFIG_INHERIT_DISABLE;
                                         }

    break;

  case 481:

    { config_inherit = (yyvsp[-1].i); }

    break;

  case 483:

    { if (read_all) {
              /* check XPath dependencies */
              if ((yyvsp[-1].nodes).notif->must_size &&
                  (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).notif, UNRES_XPATH, NULL) == -1)) {
                YYABORT;
              }
            }
          }

    break;

  case 484:

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
                                  if (store_flags((struct lys_node *)(yyval.nodes).notif, size_arrays->node[size_arrays->next].flags, CONFIG_INHERIT_DISABLE)) {
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

  case 485:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).notif;
                                      actual_type = NOTIFICATION_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 487:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 488:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 489:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 490:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 491:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 493:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 495:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 497:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 498:

    { if (read_all) {
                         struct lys_module *mod;
                         struct lys_node *parent;

                         if (actual_type == DEVIATION_KEYWORD) {
                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                           ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                           free((yyvsp[-1].nodes).deviation);
                           YYABORT;
                         }
                         if (yang_check_deviation(trg, (yyvsp[-1].nodes).deviation->dflt_check, unres)) {
                           ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                           free((yyvsp[-1].nodes).deviation);
                           YYABORT;
                         }
                         /* mark all the affected modules as deviated and implemented */
                         for(parent = (yyvsp[-1].nodes).deviation->target; parent; parent = lys_parent(parent)) {
                             mod = lys_node_module(parent);
                             if (module != mod) {
                                 mod->deviated = 1;
                                 lys_set_implemented(mod);
                             }
                         }
                         ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                         free((yyvsp[-1].nodes).deviation);
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

  case 514:

    { if (read_all) {
             /* check XPath dependencies */
             if (((yyvsp[-1].nodes).deviation->trg_must_size && *(yyvsp[-1].nodes).deviation->trg_must_size) &&
                 unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).deviation->target, UNRES_XPATH, NULL)) {
               YYABORT;
             }
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
              /* check XPath dependencies */
              if (((yyvsp[-1].nodes).deviation->trg_must_size && *(yyvsp[-1].nodes).deviation->trg_must_size) &&
                  unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).deviation->target, UNRES_XPATH, NULL)) {
                YYABORT;
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

  case 535:

    { if (read_all) {
                                     (yyval.nodes).deviation = actual;
                                   }
                                 }

    break;

  case 537:

    { if (read_all && (yyvsp[-1].nodes).deviation->deviate->type) {
             if (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-1].nodes).deviation->target) == -1) {
               lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-1].nodes).deviation->deviate->type->der)->name);
               free((yyvsp[-1].nodes).deviation->deviate->type->der);
               (yyvsp[-1].nodes).deviation->deviate->type->der = NULL;
               YYABORT;
             }
           }
         }

    break;

  case 538:

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

  case 539:

    { if (read_all) {
                                                    ly_set_add((yyvsp[-2].nodes).deviation->dflt_check, (yyvsp[-2].nodes).deviation->target, 0);
                                                  }
                                                }

    break;

  case 540:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 541:

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

  case 542:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 543:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 544:

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

  case 545:

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

  case 546:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 551:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 552:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 553:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 554:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 555:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 556:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 557:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 558:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 559:

    { if (read_string) {
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

  case 560:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 561:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 562:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 563:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 564:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 565:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 566:

    { if (read_string) {
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

  case 568:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 569:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 570:

    { if (read_all) {
                  if (strlen(s) == 1 && s[0] == '0') {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr = NULL;
                    errno = 0;

                    val = strtoul(s, &endptr, 10);
                    if (*endptr || s[0] == '-' || errno || val > UINT32_MAX) {
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

  case 571:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 572:

    { (yyval.uint) = 0; }

    break;

  case 573:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 574:

    { if (read_all) {
                  if (!strcmp(s, "unbounded")) {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr = NULL;
                    errno = 0;

                    val = strtoul(s, &endptr, 10);
                    if (*endptr || s[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
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

  case 575:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 576:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 577:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 578:

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

  case 579:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 589:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 592:

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

  case 595:

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

  case 596:

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

  case 600:

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

  case 602:

    { tmp_s = yyget_text(scanner); }

    break;

  case 603:

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

  case 604:

    { tmp_s = yyget_text(scanner); }

    break;

  case 605:

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

  case 629:

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

  case 630:

    { (yyval.uint) = 0; }

    break;

  case 631:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 632:

    { (yyval.i) = 0; }

    break;

  case 633:

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

  case 639:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 644:

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

  case 650:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 673:

    { if (read_all){
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

  case 765:

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
