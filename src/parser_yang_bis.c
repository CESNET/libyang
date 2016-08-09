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
    ACTION_KEYWORD = 351
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
#define YYLAST   3167

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  340
/* YYNRULES -- Number of rules.  */
#define YYNRULES  750
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1210

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

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
     105,   106,     2,    97,     2,     2,     2,   101,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   100,
       2,   104,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   102,     2,   103,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    98,     2,    99,     2,     2,     2,     2,
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
      95,    96
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   249,   249,   250,   271,   271,   290,   292,   291,   323,
     336,   323,   345,   346,   354,   355,   358,   371,   358,   382,
     383,   389,   389,   398,   400,   402,   425,   426,   429,   429,
     449,   450,   456,   467,   478,   487,   489,   489,   512,   513,
     517,   518,   529,   540,   549,   561,   572,   561,   581,   583,
     584,   585,   586,   591,   597,   599,   601,   603,   605,   613,
     616,   616,   625,   626,   626,   631,   632,   637,   644,   644,
     653,   659,   700,   703,   704,   705,   706,   707,   708,   709,
     715,   716,   717,   719,   719,   738,   739,   743,   744,   751,
     758,   769,   781,   781,   783,   784,   788,   790,   791,   792,
     803,   805,   806,   805,   809,   810,   811,   812,   829,   829,
     838,   839,   844,   863,   876,   882,   887,   893,   895,   895,
     904,   905,   910,   950,   959,   968,   974,   979,   985,   989,
     987,  1004,  1014,  1014,  1021,  1024,  1024,  1039,  1040,  1046,
    1052,  1057,  1063,  1064,  1070,  1075,  1076,  1077,  1084,  1092,
    1097,  1101,  1131,  1132,  1136,  1137,  1138,  1146,  1159,  1165,
    1166,  1184,  1188,  1198,  1199,  1204,  1219,  1224,  1229,  1234,
    1240,  1244,  1254,  1255,  1260,  1260,  1280,  1281,  1284,  1296,
    1306,  1307,  1312,  1336,  1345,  1354,  1360,  1365,  1371,  1383,
    1384,  1402,  1407,  1408,  1413,  1415,  1417,  1421,  1425,  1440,
    1440,  1460,  1461,  1463,  1474,  1484,  1485,  1490,  1514,  1523,
    1532,  1538,  1543,  1549,  1561,  1562,  1577,  1579,  1581,  1583,
    1585,  1585,  1592,  1593,  1598,  1618,  1624,  1629,  1634,  1635,
    1642,  1643,  1646,  1647,  1648,  1649,  1650,  1651,  1652,  1655,
    1655,  1663,  1664,  1669,  1703,  1703,  1705,  1712,  1712,  1720,
    1721,  1727,  1733,  1738,  1743,  1743,  1748,  1748,  1753,  1753,
    1761,  1761,  1768,  1775,  1768,  1793,  1821,  1821,  1823,  1830,
    1835,  1830,  1845,  1846,  1846,  1854,  1857,  1863,  1869,  1875,
    1880,  1886,  1893,  1886,  1912,  1940,  1940,  1942,  1949,  1954,
    1949,  1964,  1965,  1965,  1973,  1979,  1993,  2007,  2019,  2025,
    2030,  2036,  2043,  2036,  2068,  2111,  2111,  2113,  2120,  2120,
    2128,  2141,  2149,  2155,  2169,  2183,  2195,  2201,  2206,  2211,
    2211,  2219,  2219,  2224,  2224,  2229,  2229,  2237,  2237,  2248,
    2250,  2249,  2267,  2288,  2288,  2290,  2303,  2315,  2323,  2331,
    2339,  2348,  2357,  2357,  2367,  2368,  2371,  2372,  2373,  2374,
    2375,  2378,  2378,  2386,  2387,  2391,  2411,  2411,  2413,  2420,
    2426,  2431,  2436,  2436,  2443,  2443,  2454,  2455,  2459,  2486,
    2486,  2488,  2495,  2495,  2503,  2509,  2515,  2521,  2526,  2532,
    2532,  2548,  2549,  2553,  2588,  2588,  2590,  2597,  2603,  2608,
    2613,  2613,  2621,  2621,  2636,  2636,  2645,  2646,  2651,  2652,
    2655,  2675,  2682,  2706,  2730,  2749,  2768,  2791,  2814,  2819,
    2825,  2834,  2825,  2841,  2842,  2845,  2854,  2845,  2866,  2887,
    2887,  2889,  2896,  2905,  2910,  2915,  2915,  2923,  2923,  2931,
    2931,  2941,  2942,  2945,  2945,  2956,  2956,  2967,  2968,  2973,
    3001,  3008,  3014,  3019,  3024,  3024,  3032,  3032,  3037,  3037,
    3049,  3049,  3062,  3076,  3062,  3083,  3114,  3114,  3122,  3122,
    3130,  3130,  3135,  3135,  3145,  3159,  3145,  3166,  3166,  3176,
    3180,  3185,  3223,  3223,  3231,  3238,  3244,  3249,  3254,  3254,
    3262,  3262,  3267,  3267,  3274,  3283,  3274,  3296,  3315,  3322,
    3329,  3339,  3340,  3342,  3347,  3349,  3350,  3351,  3354,  3356,
    3356,  3362,  3363,  3367,  3389,  3397,  3405,  3421,  3429,  3436,
    3443,  3454,  3466,  3466,  3472,  3473,  3477,  3499,  3507,  3518,
    3528,  3537,  3537,  3543,  3544,  3548,  3553,  3559,  3567,  3575,
    3582,  3589,  3600,  3612,  3612,  3615,  3616,  3620,  3621,  3626,
    3632,  3634,  3635,  3634,  3638,  3639,  3640,  3655,  3657,  3658,
    3657,  3661,  3662,  3663,  3678,  3680,  3682,  3683,  3704,  3706,
    3707,  3708,  3729,  3731,  3732,  3733,  3745,  3745,  3753,  3754,
    3759,  3761,  3762,  3764,  3765,  3767,  3769,  3769,  3778,  3781,
    3781,  3792,  3795,  3805,  3826,  3828,  3829,  3832,  3832,  3851,
    3851,  3860,  3860,  3869,  3872,  3874,  3876,  3877,  3879,  3881,
    3883,  3884,  3886,  3888,  3889,  3891,  3892,  3894,  3896,  3899,
    3903,  3905,  3906,  3908,  3909,  3911,  3913,  3924,  3925,  3928,
    3929,  3941,  3942,  3944,  3945,  3947,  3948,  3954,  3955,  3958,
    3959,  3960,  3986,  3987,  3990,  3991,  3992,  3995,  3995,  4003,
    4005,  4006,  4008,  4009,  4010,  4012,  4013,  4015,  4016,  4018,
    4019,  4021,  4022,  4024,  4025,  4028,  4029,  4032,  4034,  4035,
    4038,  4038,  4047,  4049,  4050,  4051,  4052,  4053,  4054,  4055,
    4057,  4058,  4059,  4060,  4061,  4062,  4063,  4064,  4065,  4066,
    4067,  4068,  4069,  4070,  4071,  4072,  4073,  4074,  4075,  4076,
    4077,  4078,  4079,  4080,  4081,  4082,  4083,  4084,  4085,  4086,
    4087,  4088,  4089,  4090,  4091,  4092,  4093,  4094,  4095,  4096,
    4097,  4098,  4099,  4100,  4101,  4102,  4103,  4104,  4105,  4106,
    4107,  4108,  4109,  4110,  4111,  4112,  4113,  4114,  4115,  4116,
    4117,  4118,  4119,  4120,  4121,  4122,  4123,  4124,  4125,  4126,
    4127,  4128,  4129,  4130,  4131,  4132,  4133,  4134,  4135,  4138,
    4147
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
  "USER_KEYWORD", "ACTION_KEYWORD", "'+'", "'{'", "'}'", "';'", "'/'",
  "'['", "']'", "'='", "'('", "')'", "$accept", "start", "string_1", "$@1",
  "string_2", "$@2", "module_stmt", "$@3", "$@4", "module_header_stmts",
  "submodule_stmt", "$@5", "$@6", "submodule_header_stmts", "$@7",
  "yang_version_stmt", "namespace_stmt", "linkage_stmts", "import_stmt",
  "$@8", "import_opt_stmt", "tmp_identifier_arg_str", "include_stmt",
  "$@9", "include_end", "include_opt_stmt", "revision_date_stmt",
  "belongs_to_stmt", "$@10", "$@11", "prefix_stmt", "meta_stmts",
  "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_stmt", "$@12",
  "revision_end", "$@13", "revision_opt_stmt", "date_arg_str", "$@14",
  "body_stmts", "body_stmt", "extension_stmt", "$@15", "extension_end",
  "extension_opt_stmt", "argument_stmt", "$@16", "argument_end",
  "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@17", "$@18", "status_arg_str", "feature_stmt",
  "$@19", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@20", "identity_end", "identity_opt_stmt",
  "base_stmt", "typedef_stmt", "$@21", "typedef_arg_str", "type_stmt",
  "$@22", "type_opt_stmt", "$@23", "type_end", "type_stmtsep",
  "type_body_stmts", "decimal_string_restrictions", "union_spec",
  "fraction_digits_stmt", "fraction_digits_arg_str", "length_stmt",
  "length_arg_str", "length_end", "message_opt_stmt", "pattern_stmt",
  "pattern_arg_str", "pattern_end", "enum_specification", "$@24",
  "enum_stmts", "enum_stmt", "enum_arg_str", "enum_end", "enum_opt_stmt",
  "value_stmt", "integer_value_arg_str", "range_stmt", "range_end",
  "path_stmt", "require_instance_stmt", "require_instance_arg_str",
  "bits_specification", "$@25", "bit_stmts", "bit_stmt", "bit_arg_str",
  "bit_end", "bit_opt_stmt", "position_stmt", "position_value_arg_str",
  "error_message_stmt", "error_app_tag_stmt", "units_stmt", "default_stmt",
  "grouping_stmt", "$@26", "grouping_end", "grouping_opt_stmt",
  "data_def_stmt", "container_stmt", "$@27", "container_end",
  "container_opt_stmt", "$@28", "$@29", "$@30", "$@31", "$@32", "$@33",
  "leaf_stmt", "$@34", "$@35", "leaf_opt_stmt", "$@36", "$@37", "$@38",
  "$@39", "leaf_list_stmt", "$@40", "$@41", "leaf_list_opt_stmt", "$@42",
  "$@43", "$@44", "$@45", "list_stmt", "$@46", "$@47", "list_opt_stmt",
  "$@48", "$@49", "$@50", "$@51", "$@52", "$@53", "choice_stmt", "$@54",
  "choice_end", "$@55", "choice_opt_stmt", "$@56", "$@57",
  "short_case_case_stmt", "short_case_stmt", "case_stmt", "$@58",
  "case_end", "case_opt_stmt", "$@59", "$@60", "anyxml_stmt", "$@61",
  "anyxml_end", "anyxml_opt_stmt", "$@62", "$@63", "uses_stmt", "$@64",
  "uses_end", "uses_opt_stmt", "$@65", "$@66", "$@67", "refine_stmt",
  "$@68", "refine_end", "refine_arg_str", "refine_body_opt_stmts",
  "uses_augment_stmt", "$@69", "$@70", "uses_augment_arg_str",
  "augment_stmt", "$@71", "$@72", "augment_opt_stmt", "$@73", "$@74",
  "$@75", "$@76", "augment_arg_str", "action_stmt", "$@77", "rpc_stmt",
  "$@78", "rpc_end", "rpc_opt_stmt", "$@79", "$@80", "$@81", "$@82",
  "input_stmt", "$@83", "$@84", "input_output_opt_stmt", "$@85", "$@86",
  "$@87", "$@88", "output_stmt", "$@89", "$@90", "notification_stmt",
  "$@91", "notification_end", "notification_opt_stmt", "$@92", "$@93",
  "$@94", "$@95", "deviation_stmt", "$@96", "$@97", "deviation_opt_stmt",
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
     345,   346,   347,   348,   349,   350,   351,    43,   123,   125,
      59,    47,    91,    93,    61,    40,    41
};
# endif

#define YYPACT_NINF -1034

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1034)))

#define YYTABLE_NINF -606

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1034,    29, -1034, -1034,   205, -1034, -1034, -1034,   103,   103,
   -1034, -1034,  2978,  2978,   103, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   -55,   103,    24,   103, -1034, -1034,    49,   306,   306,
   -1034, -1034,   187, -1034, -1034,    25,   190,   103,   103,   103,
     103, -1034, -1034, -1034, -1034, -1034,   137, -1034, -1034,   111,
    1959, -1034,  2327,  2978,  2327,   159,   159,   103, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034,   165, -1034, -1034,   165, -1034,   165,
     103,   103, -1034, -1034,   275,   275,  2978,   103,  2420,   103,
   -1034, -1034, -1034, -1034, -1034,   103, -1034,  2978,  2978,   103,
     103,   103,   103, -1034, -1034, -1034, -1034,   100,   100, -1034,
   -1034,   103,    84, -1034,    89,    19,   306,   103, -1034, -1034,
   -1034,  2327,  2327,  2327,  2327,   103, -1034,  1161,  1206,    94,
     253, -1034, -1034, -1034,   115,   273,   165,   165,   165,   165,
      50,   306,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034,   229,   306, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034,  2978,     3,  2978,  2978,  2978,
       3,  2978,  2978,  2978,  2978,  2978,  2978,  2978,  2978,  2978,
    2513,   103,   306,   103,    71,  2420,   103, -1034,   306,   306,
     306, -1034,   283, -1034,  3071, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034,   140, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   151,   103,   578,   279,   103, -1034, -1034, -1034,   297,
   -1034,   143,   152,   103,   333,   344,   351,   197,   103,   384,
     414,   429,   204,   207,   216,   450,   451, -1034,   471,   103,
     103,   161, -1034,   103, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034,   306, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034,   306, -1034, -1034,
   -1034, -1034,   103,    50,   306, -1034,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     226,   306,   306,   165,   105,   294,  1264,   586,   293,   101,
     217,   265,   579,   213,  1475,  1362,  1419,  1340,  1062,   103,
     103,   103, -1034, -1034, -1034,   189,   243, -1034, -1034,   295,
   -1034, -1034, -1034, -1034,   103,   103,   103,   103,   103,   103,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,   103,
     103, -1034, -1034, -1034, -1034, -1034, -1034,   224, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,   231,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034,   103, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   103, -1034, -1034, -1034, -1034, -1034,   103, -1034, -1034,
     248, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034,   103, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034,   258,   243, -1034,
   -1034, -1034, -1034,   103,   103,   103, -1034, -1034, -1034, -1034,
   -1034,   266,   243, -1034, -1034, -1034, -1034, -1034, -1034,   103,
     103, -1034, -1034, -1034, -1034, -1034, -1034, -1034,   271, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,  2327,   109,
    2327, -1034,   103, -1034,   103,   103, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034,  2327, -1034,  2327, -1034,  2327,
   -1034, -1034,  2978,  2978, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034,  2327, -1034, -1034, -1034, -1034, -1034, -1034,  2978,
     609, -1034,   306,   306,   306,   306,  2513, -1034, -1034, -1034,
   -1034,    53,   144,    39, -1034, -1034, -1034, -1034,  2606,  2606,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   103,   103, -1034, -1034, -1034, -1034,   165, -1034, -1034,
   -1034, -1034,   165,   165,  2513,   306,  2606,  2606, -1034, -1034,
   -1034,    55,   165,    56, -1034,   109, -1034,   306,   306, -1034,
   -1034,   306,   306,   306,   306,   306,   306,   165,   306,   306,
     306,   306,   306,   306, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034,   165, -1034,   306,   306, -1034,
   -1034, -1034,   165, -1034, -1034, -1034,   165, -1034, -1034, -1034,
   -1034, -1034,   165, -1034,   306,   306,   165, -1034, -1034,   165,
   -1034,    11, -1034,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   254,   284,   306,   306,   306,   306, -1034,
     103,   103,   103, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034,   306,   306,   306, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034,   474, -1034,   491,   497,   451, -1034,
     501,   103,   103,    11,   103,   103, -1034,   306,   103, -1034,
     103, -1034,   103,   103,   103, -1034,   306, -1034,    11, -1034,
   -1034, -1034,  3071, -1034, -1034, -1034,   525,   298,   103,   563,
     103,   103,   103,   165,   103,   103,   165, -1034, -1034, -1034,
     165, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   576,   582, -1034,   601, -1034, -1034,  3071,    11,   152,
     306,   306, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   306, -1034,   306,   306,   162,   306, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,   668,   668,
     366,   306,   306,   306,    38,   150,  1033,   103,   278,   306,
     306,   306,    11, -1034, -1034, -1034,   292, -1034,   331,   103,
     103, -1034,   337,   630, -1034,   409, -1034, -1034, -1034,   424,
    1264,   373,   103,   103, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,    57,
   -1034,   402,   184,   659, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   171,    66, -1034,   103,   103,   103,   103,   243, -1034,
   -1034, -1034, -1034,   103, -1034,   103, -1034,   359,   103,   103,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
    2327,  2327, -1034, -1034, -1034, -1034, -1034,   165, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
     306,   306,   306,   306, -1034,   165,   358,   453, -1034, -1034,
   -1034,   165,    37,  2327,  2327,  2327, -1034,   306,  2327,   306,
    2978,   424, -1034,    55,    56,   306,   165,   165,   306,   306,
     103,   103, -1034,   306,   306,   306, -1034,  3071, -1034, -1034,
     372, -1034, -1034,   103,   103, -1034, -1034,   165, -1034,   635,
   -1034,   640, -1034,   641, -1034,   306,   645, -1034,   409,   654,
   -1034, -1034,   165,   165, -1034, -1034, -1034,   358, -1034,  2699,
   -1034,   103, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
     375, -1034, -1034, -1034,   145,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   469, -1034,   285,   435,   485,   637,
     717,   539, -1034,  3071, -1034, -1034, -1034, -1034,   103, -1034,
   -1034, -1034, -1034, -1034, -1034,   103, -1034, -1034, -1034, -1034,
   -1034, -1034,   469,   469,    69,   144,   380,   374, -1034, -1034,
   -1034, -1034,   165, -1034, -1034, -1034,   165, -1034, -1034,   469,
   -1034,   103, -1034,   103,   415,   469, -1034,   469,   404,   412,
     469,   469,   430,   506, -1034,   469, -1034, -1034,   420,  2792,
     469, -1034, -1034, -1034,  2885, -1034,   440,   469,  3071, -1034
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     655,     0,     2,     3,     0,     1,   653,   654,     0,     0,
     656,   655,     0,     0,   657,   671,     4,   670,   672,   673,
     674,   675,   676,   677,   678,   679,   680,   681,   682,   683,
     684,   685,   686,   687,   688,   689,   690,   691,   692,   693,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   731,   732,   733,
     734,   735,   736,   737,   738,   739,   740,   741,   742,   743,
     744,   745,   746,   747,   748,   626,     9,   749,   655,    16,
     655,     0,   625,     0,     6,   634,   634,     5,    12,    19,
     655,   637,    10,   636,   635,    17,     0,   640,     0,     0,
       0,    25,    13,    14,    15,    25,     0,    20,     7,     0,
     642,   641,     0,     0,     0,    49,    49,     0,    22,   655,
     655,   647,   638,   655,   663,   666,   664,   668,   669,   667,
     639,   643,   665,   662,     0,   660,   623,     0,   655,     0,
       0,     0,    26,    27,    58,    58,     0,     8,   649,   645,
     634,   634,    24,   655,    48,   624,    23,     0,     0,     0,
       0,     0,     0,    50,    51,    52,    53,    71,    71,    45,
     628,   640,     0,   627,   644,     0,   632,   661,    28,    35,
      36,     0,     0,     0,     0,     0,   634,     0,     0,     0,
       0,   648,   655,   634,     0,     0,     0,     0,     0,     0,
       0,    59,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   655,   634,
      73,    74,    75,    76,    77,    78,   232,   233,   234,   235,
     236,   237,   238,    79,    80,    81,    82,   655,   634,   655,
     655,   650,     0,   633,   634,   634,    38,   634,    55,    56,
      54,    57,    68,    70,    60,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    72,    18,     0,   649,   651,   655,    30,    40,
      37,   655,     0,   364,     0,   432,   415,   585,   655,   327,
     239,    83,   492,   484,   655,   108,   220,   118,   262,   281,
     301,   467,   435,     0,   131,   750,   631,   379,   655,   655,
      46,     0,   646,     0,     0,    69,   634,    62,    61,     0,
     583,     0,   584,   431,     0,     0,     0,     0,   491,     0,
       0,     0,     0,     0,     0,     0,     0,   634,     0,   629,
     630,     0,   655,     0,   634,    34,    31,    32,    33,    39,
      43,    41,    42,    63,   634,   366,   365,   634,   586,   634,
     329,   328,   634,   241,   240,   634,    85,    84,   634,   634,
     110,   109,   634,   222,   221,   634,   120,   119,   634,   634,
     634,   634,   469,   468,   634,   437,   436,   134,   634,   381,
     380,   634,   652,     0,    29,    65,   368,   418,   332,   243,
      87,   487,   112,   224,   122,   265,   284,   304,   471,   439,
     129,   383,    47,     0,     0,     0,   416,   330,     0,     0,
     485,     0,     0,     0,   263,   282,   302,     0,     0,     0,
       0,     0,   140,   141,   139,     0,     0,   137,   138,     0,
      44,    64,    66,    67,     0,     0,     0,     0,     0,     0,
     367,   377,   378,   376,   371,   369,   374,   375,   372,     0,
       0,   423,   424,   422,   421,   425,   429,     0,   427,   419,
     340,   341,   339,   335,   336,   346,   347,   348,   349,     0,
     342,   344,   345,   350,   333,   337,   338,     0,   242,   252,
     253,   251,   246,   258,   254,   260,   256,   244,   250,   249,
     247,     0,    86,    90,    91,    88,    89,     0,   488,   489,
       0,   111,   115,   116,   114,   113,   223,   226,   227,   225,
     634,   634,   634,   634,     0,   121,   126,   127,   125,   124,
     123,   279,   280,   278,   268,   272,   275,     0,     0,   266,
     276,   277,   273,     0,     0,     0,   299,   300,   298,   287,
     291,     0,     0,   285,   294,   295,   296,   297,   292,     0,
       0,   317,   318,   316,   307,   319,   321,   325,     0,   323,
     305,   312,   313,   314,   315,   308,   311,   310,   470,   476,
     477,   475,   474,   478,   480,   482,   472,   655,   655,   438,
     442,   443,   441,   440,   444,   446,   448,   450,     0,     0,
       0,   130,     0,   634,     0,     0,   382,   388,   389,   387,
     386,   390,   392,   384,   541,     0,   548,     0,   101,     0,
     634,   634,     0,     0,   634,   634,   417,   634,   634,   331,
     634,   634,     0,   634,   634,   634,   634,   634,   634,     0,
       0,   486,   229,   228,   230,   231,     0,   264,   270,   634,
     634,     0,     0,     0,   283,   289,   634,   634,     0,     0,
     634,   634,   634,   303,   634,   634,   634,   634,   634,   634,
     634,   452,   464,   634,   634,   634,   634,     0,   655,   655,
     655,   107,     0,     0,     0,   136,     0,     0,   634,   634,
     634,     0,     0,     0,   566,     0,   533,   370,   373,   351,
     433,   426,   430,   428,   420,   343,   334,     0,   259,   255,
     261,   257,   245,   248,    92,   655,   655,   655,   655,   490,
     655,   493,   495,   497,   496,     0,   634,   267,   274,   616,
     655,   561,     0,   655,   617,   557,     0,   618,   655,   655,
     655,   565,     0,   634,   286,   293,     0,   575,   576,     0,
     570,     0,   587,   320,   322,   326,   324,   306,   309,   479,
     481,   483,   473,     0,     0,   445,   447,   449,   451,   219,
     104,   106,   105,   100,   218,   132,   414,   410,   655,   399,
     394,   655,   391,   393,   385,   655,   655,   546,   542,   117,
     655,   655,   553,   549,     0,   102,     0,     0,     0,   554,
       0,   499,   512,     0,   521,   494,   128,   271,   559,   558,
     560,   555,   556,   564,   563,   562,   290,   578,     0,   572,
     571,   574,     0,   585,   634,   634,     0,     0,   413,     0,
     398,   545,   544,     0,   552,   551,     0,   634,   568,   567,
       0,   634,   535,   534,   634,   353,   352,   434,   634,    94,
     634,     0,     0,   498,     0,   577,   581,     0,     0,   588,
     455,   455,   634,   142,   133,   634,   634,   396,   395,   543,
     550,   165,   103,   537,   355,     0,    93,   634,   501,   500,
     634,   514,   513,   634,   523,   522,   579,   573,   453,   465,
     151,   144,   418,   400,     0,     0,     0,     0,     0,   503,
     516,   525,     0,   458,   460,   462,     0,   456,     0,     0,
       0,   148,     0,   145,   146,     0,   147,   149,   150,     0,
     411,     0,     0,     0,   569,   168,   169,   166,   167,   536,
     538,   539,   354,   360,   361,   359,   358,   362,   356,     0,
      95,     0,     0,     0,   580,   634,   634,   634,   454,   634,
     466,   589,     0,   143,     0,     0,     0,     0,     0,   154,
     152,   153,   634,     0,   634,     0,   201,     0,     0,     0,
     397,   408,   409,   403,   404,   405,   402,   406,   407,   634,
       0,     0,   634,   634,   655,   655,    99,     0,   502,   504,
     507,   508,   509,   510,   511,   634,   506,   515,   517,   520,
     634,   519,   524,   634,   527,   528,   529,   530,   531,   532,
     459,   461,   463,   457,   593,     0,     0,     0,   655,   655,
     198,     0,     0,     0,     0,     0,   634,   155,     0,   176,
       0,   200,   412,     0,     0,   401,     0,     0,   363,   357,
      98,    97,    96,   505,   518,   526,   194,     0,   596,   590,
       0,   592,   600,   197,   196,   195,   160,     0,   655,     0,
     162,     0,   171,     0,   582,   156,     0,   179,   175,     0,
     204,   202,     0,     0,   217,   216,   605,   595,   599,     0,
     158,   159,   634,   163,   634,   634,   172,   634,   634,   192,
     191,   634,   180,   178,   634,   634,   205,   634,   540,   547,
     594,   597,   601,   598,   603,   165,   161,   165,   170,   165,
     182,   177,   207,   203,   658,   602,     0,     0,     0,     0,
       0,     0,   659,     0,   604,   164,   173,   193,     0,   181,
     186,   187,   185,   183,   184,     0,   206,   211,   212,   210,
     208,   209,   658,   658,     0,     0,     0,     0,   621,   622,
     619,   190,     0,   655,   620,   215,     0,   655,   606,   658,
     188,   189,   213,   214,     0,   658,   607,   658,     0,     0,
     658,   658,     0,     0,   615,   658,   608,   611,     0,     0,
     658,   612,   613,   610,   658,   609,     0,   658,     0,   614
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1034, -1034,   661, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034,   447, -1034,   419, -1034, -1034,
   -1034,   395, -1034, -1034, -1034, -1034,   262, -1034, -1034, -1034,
    -238,   446, -1034, -1034,   -25,    51,   423, -1034, -1034, -1034,
   -1034, -1034,   179, -1034,   416, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034,  -427,  -318, -1034, -1034,  -115,
   -1034, -1034, -1034, -1034,  -410, -1034, -1034, -1034, -1034,  -307,
    -345, -1034, -1034,  -526, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1033, -1034, -1034,
   -1034, -1034, -1034, -1034,  -481, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,  -437, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034,  -405,  -430,  -258, -1034,
   -1034, -1034,  -363,   193, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034,   198, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034,   203, -1034, -1034, -1034, -1034, -1034, -1034, -1034,   210,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,   236, -1034,
   -1034, -1034, -1034, -1034,   244, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,  -268,
   -1034, -1034, -1034, -1034, -1034,  -148, -1034, -1034, -1034,  -164,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,  -197, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034, -1034,
   -1034, -1034, -1034, -1034,  -378, -1034, -1034, -1034,  -634,   141,
   -1034, -1034,  -368,  -208,  -338, -1034, -1034,  -364,  -250,  -433,
   -1034,  -426, -1034,   256, -1034,  -394, -1034, -1034,  -555, -1034,
    -175, -1034, -1034, -1034,  -213, -1034, -1034,  -336,   437,  -111,
    -684, -1034, -1034, -1034, -1034,  -355,  -390, -1034, -1034,  -343,
   -1034, -1034, -1034,  -367, -1034, -1034, -1034,  -439, -1034, -1034,
   -1034,  -663,  -404, -1034, -1034, -1034,   -11,  -153,  -624,   503,
    1215, -1034, -1034,   571, -1034, -1034, -1034, -1034,   468, -1034,
      -4,   142,   546,   378,     4, -1034,   636,  -113,  -124, -1034
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    95,   100,   107,   139,     2,   101,   121,   112,
       3,   103,   125,   115,   126,   122,   123,   135,   162,   214,
     333,   198,   163,   215,   267,   334,   365,   138,   209,   361,
     124,   164,   183,   184,   945,   946,   187,   206,   302,   338,
     415,   434,   274,   301,   207,   239,   240,   346,   387,   439,
     525,   820,   870,   918,  1007,   483,   473,   715,   860,   702,
     241,   349,   391,   441,   484,   242,   351,   397,   443,   550,
     243,   455,   323,   623,   846,   430,   456,   884,   910,   932,
     933,   978,   979,  1077,   980,  1079,  1104,   914,   981,  1081,
    1107,   934,   935,  1088,   984,  1086,  1113,  1140,  1154,  1172,
     982,  1110,   936,   937,  1041,   938,   939,  1051,   986,  1089,
    1117,  1141,  1161,  1176,   947,   948,   457,   458,   244,   350,
     394,   442,   245,   246,   345,   384,   438,   657,   658,   654,
     656,   653,   655,   247,   352,   557,   444,   669,   558,   746,
     670,   248,   353,   571,   445,   676,   572,   763,   677,   249,
     354,   588,   446,   685,   686,   680,   681,   684,   682,   250,
     344,   381,   499,   437,   651,   650,   500,   501,   486,   817,
     866,   916,  1003,  1002,   251,   339,   376,   435,   640,   641,
     252,   358,   410,   459,   710,   708,   709,   631,   849,   888,
     800,   941,   632,   847,   987,   797,   253,   341,   487,   436,
     648,   644,   647,   645,   306,   488,   818,   254,   356,   406,
     448,   693,   694,   695,   696,   616,   783,   926,   908,   969,
     965,   966,   967,   617,   784,   928,   255,   355,   403,   447,
     690,   687,   688,   689,   256,   347,   530,   440,   313,   739,
     740,   741,   742,   871,   899,   961,   743,   872,   902,   962,
     744,   874,   905,   963,   489,   816,   863,   915,   994,   476,
     711,   853,   808,   995,   477,   713,   856,   813,   519,   575,
     756,   576,   752,   577,   762,   927,   814,   859,   596,   770,
     840,   597,   767,   838,   875,   922,  1083,   307,   308,   342,
     771,   843,  1035,  1036,  1037,  1068,  1069,  1097,  1071,  1072,
    1099,  1123,  1135,  1120,  1162,  1186,  1196,  1197,  1199,  1204,
    1187,   757,   758,  1173,  1174,   157,   199,   772,   327,   841,
     108,   113,   117,   129,   130,   150,   194,   142,   192,   261,
     114,     4,   131,  1143,   154,   173,   155,    97,    98,   329
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    96,    99,   454,    11,    11,   378,   494,   753,   158,
      10,    16,   526,   592,   556,   191,     6,   152,     7,   152,
     593,   152,   798,   801,     6,   474,     7,   493,   512,     5,
     111,   535,   668,   549,   554,   569,   584,   602,   613,   555,
     570,   478,   745,   105,   520,    16,   675,    16,   -21,   630,
     562,   578,   595,   606,   749,   193,   330,   475,    16,   504,
     517,    16,   272,    16,    16,    16,   559,   573,   590,   180,
     749,   942,   943,   485,    16,   515,     6,    16,     7,   542,
     795,   633,   111,   587,   605,  1168,  1169,  1170,   152,   152,
     152,   152,  1137,   513,  1138,   366,  1139,   540,    10,   506,
      10,   585,   603,   614,   304,   182,   561,   120,     6,   170,
       7,   171,    10,    11,    11,    11,    11,    16,   213,   492,
     511,   521,   106,   534,   539,   548,   553,   568,   583,   601,
     612,   759,   180,    11,   760,   119,   180,   944,   159,   185,
     185,   629,   191,   805,   810,  1004,   110,   750,   806,   811,
    1005,   340,    16,    14,  1038,   189,    11,    11,   878,  1039,
     137,   749,   754,    10,    10,    10,   328,     6,   182,     7,
     205,    10,   182,   111,   450,    11,    11,    11,    11,    16,
     514,   180,   193,   211,   541,  -591,   212,    11,   586,   604,
     615,   193,   258,    10,   698,     6,   699,     7,   128,   700,
     522,    11,   160,   161,   461,   216,   217,   218,   219,   140,
       6,   141,     7,   264,   449,   186,   186,   182,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,     6,   544,     7,   297,   357,   467,
     102,   377,   104,   118,   180,   917,  -605,  -605,   180,   949,
     362,   119,   116,   304,   527,   465,   449,   180,    10,     8,
     411,   580,   451,   170,   303,   171,   309,   310,   311,   120,
     315,   316,   317,   318,   319,   320,   321,   322,   324,     9,
     182,   167,   168,  1017,   182,   169,   468,    10,   621,    10,
     516,    10,    10,   182,   543,   388,   180,   222,   589,   450,
     175,  -135,   398,   179,   451,   399,   180,   465,   367,   371,
     180,     6,   545,     7,   400,   197,   624,   111,   622,   224,
     464,   464,   225,   646,   180,   180,   180,  1011,    10,  1026,
     649,    10,   182,   229,   181,   465,   465,   465,   468,    10,
     231,   232,   182,   233,    10,   466,   182,   661,   467,   467,
     363,   259,   844,   260,   262,    10,    10,   667,   507,    11,
     182,   182,   182,   625,   531,   674,   468,   468,   468,   236,
     683,   265,   237,   266,   469,   469,   469,   960,   369,  1078,
     291,   336,   845,   337,   368,   372,  1067,  1134,   544,   480,
    -199,   968,   508,   470,   626,   374,   885,   375,  -174,   293,
     988,   295,   296,   449,   180,   452,  1016,  1021,    10,   462,
     471,   481,   490,   509,   523,   528,   532,   537,   546,   551,
     566,   581,   599,   610,   989,   563,   564,   929,   467,   988,
     970,   379,   449,   380,   627,   930,   973,  1023,   507,   332,
     182,   983,   382,   335,   383,    11,    11,    11,   985,   385,
     343,   386,  1046,   989,   563,   564,   348,   467,  1052,  1067,
      11,    11,    11,    11,    11,    11,   180,  1070,   942,   943,
     359,   360,   990,  1098,  1142,    11,    11,  1134,  1179,   580,
     451,   453,   389,  1178,   390,   463,   472,   482,   491,   510,
     524,   529,   533,   538,   547,   552,   567,   582,   600,   611,
    1185,  1008,   182,    11,   412,   152,   956,   152,   997,  1190,
     628,   993,   392,  1191,   393,   998,   180,    11,   942,   943,
    1195,  1200,   152,    11,   152,   768,   152,   395,  1013,   396,
    1028,  1010,  1019,  1025,  1145,  1014,  1194,  1029,   958,   152,
      11,  1207,   328,   378,   136,   925,   925,   999,   401,   404,
     402,   405,   182,   957,    12,    13,  1009,  1018,  1024,    11,
      11,    11,   127,   923,   923,   193,   193,  1015,  1020,   408,
     180,   409,   857,   200,   858,    11,    11,   485,   505,   518,
     328,   465,   165,   222,  1146,   560,   574,   591,   188,   861,
     222,   862,   433,   193,   193,   864,   370,   865,   955,   868,
     815,   869,  1155,   931,   208,   224,   182,  1114,   225,   180,
     180,   479,   450,   464,  1091,   225,   449,   180,    11,   229,
      11,    11,   697,   882,   703,   883,   231,   232,   465,   233,
     495,   719,   720,   231,   232,   496,   233,   466,  1156,   712,
     497,   714,   119,   716,   940,   182,   182,   498,   734,   363,
     924,   924,   468,   182,   867,   236,   727,   172,   237,   468,
     174,   886,   176,   887,   132,   133,   134,   469,   180,   974,
     942,   943,   222,   502,   897,   480,   898,   364,   536,   975,
     900,   503,   901,   166,   909,  1092,   988,    10,    10,   449,
    1093,   996,   976,   735,   224,   736,   977,   225,   737,   903,
     738,   904,   594,   907,   182,  -157,   177,   178,   229,   964,
     989,   563,   564,  1152,  1159,   231,   232,   314,   233,   268,
     269,   270,   271,   467,   906,   201,   202,   203,   204,   193,
    1153,  1160,   879,  1102,   622,  1103,  1147,   451,  1105,  1108,
    1106,  1109,  1121,  1111,   236,  1112,  1144,   237,   180,   691,
     692,   220,  1115,  1012,  1116,  1027,  1122,  1136,  1022,   465,
    1201,  1177,   210,   331,   193,     0,   151,    11,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   182,     0,    10,    10,    10,     0,
     450,     0,     0,   153,   156,   153,     0,  1148,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1149,    10,    10,    10,
      10,    10,     0,     0,    10,     0,    10,     0,    10,    10,
      10,     0,     0,     0,    11,     0,     0,     0,     0,     0,
     790,   791,   792,     0,    10,     0,    10,    10,    10,     0,
      10,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   153,   153,   153,   153,     0,     0,     0,     0,
       0,     0,     0,     0,    11,     0,     0,   821,   822,   823,
     824,   273,   825,     0,     0,     0,     0,   152,   152,     0,
     950,   953,   828,     0,     0,   830,     0,     0,     0,     0,
     832,   833,   834,     0,     0,     0,     0,     0,     0,   413,
       0,     0,     0,    11,  1096,   481,   991,     0,    11,     0,
       0,     0,     0,     0,     0,    11,    11,     0,     0,     0,
     152,   152,   152,     0,     0,   152,   460,   305,    11,    11,
     848,   312,     0,   850,     0,     0,  1124,   851,   852,     0,
       0,   326,   854,   855,   193,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   951,   954,     0,     0,
      11,    11,    11,    11,     0,     0,     0,     0,     0,    11,
       0,    11,     0,     0,    11,    11,   193,     0,     0,     0,
    1163,   482,   992,     0,     0,   618,   619,   620,     0,     0,
       0,     0,     0,     0,  1056,  1057,     0,     0,     0,     0,
     634,   635,   636,   637,   638,   639,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   642,   643,     0,     0,     0,
     193,     0,     0,     0,     0,     0,     0,   222,     0,  1090,
       0,     0,     0,     0,     0,     0,  1202,  1080,  1082,  1084,
       0,  1205,  1087,   652,     0,  1209,    10,    10,     0,   224,
       0,     0,   225,     0,   180,     0,     0,   659,     0,    10,
      10,     0,     0,   660,   273,   465,     0,     0,     0,     0,
     231,   232,     0,   233,     0,     0,   193,     0,     0,     0,
     666,   193,     0,   180,     0,   193,     0,    10,     0,     0,
     182,     0,   229,     0,   465,     0,   468,   607,     0,   671,
     672,   673,   237,     0,   469,  1150,  1157,     0,     0,     0,
       0,     0,   608,     0,     0,   678,   679,     0,     0,   182,
       0,     0,   952,     0,     0,   468,     0,     0,   236,     0,
       0,     0,     0,     0,    11,     0,  1060,  1061,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   609,     0,     0,     0,   222,     0,     0,   704,     0,
     706,   707,     0,     0,     0,     0,     0,    10,     0,    10,
    1073,  1074,   223,     0,     0,     0,     0,   224,     0,     0,
     225,  1151,  1158,     0,     0,     0,   226,   227,     0,   228,
     789,   229,   230,     0,     0,   793,   794,     0,   231,   232,
     222,   233,     0,     0,     0,   809,     0,     0,   234,     0,
    1101,     0,     0,     0,     0,     0,     0,   223,     0,     0,
     819,     0,   224,   235,     0,   225,     0,   236,     0,     0,
     237,   226,   227,     0,   228,     0,   229,   230,   826,     0,
       0,     0,     0,   231,   232,   829,   233,     0,     0,   831,
     238,     0,     0,   234,     0,   835,     0,     0,   222,   837,
       0,     0,   839,     0,     0,     0,     0,     0,   235,   153,
     701,   153,   236,     0,     0,   237,     0,     0,     0,   479,
     224,     0,     0,   225,     0,   180,   153,     0,   153,     0,
     153,     0,     0,     0,     0,   257,   465,     0,     0,     0,
       0,   231,   232,   153,   233,  1181,     0,   842,     0,  1183,
       0,   109,     0,     0,     0,     0,   873,   326,     0,     0,
       0,   182,   751,   755,   761,     0,     0,   450,     0,   766,
     769,   876,     0,   237,   222,   469,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   889,     0,     0,   890,
     480,     0,     0,   892,     0,   326,   224,   796,   799,   225,
       0,   180,   807,     0,   812,     0,   701,     0,     0,     0,
     229,     0,   465,     0,   877,   195,   196,   231,   232,   464,
     233,     0,     0,   180,     0,   467,     0,     0,     0,     0,
       0,     0,     0,     0,   465,     0,     0,   182,     0,     0,
       0,     0,     0,   468,   563,   564,   236,   467,     0,   237,
     565,   221,     0,   222,   842,   876,     0,     0,   263,   182,
       0,     0,     0,     0,     0,   468,     0,  -288,     0,   598,
     451,     0,     0,   469,     0,   224,   464,     0,   225,     0,
     180,     0,     0,     0,   292,     0,     0,     0,     0,   229,
       0,   465,     0,   959,     0,   579,   231,   232,   877,   233,
       0,   563,   564,   294,   467,   971,   972,   565,     0,   298,
     299,     0,   300,     0,     0,     0,   182,     0,  1000,  1001,
       0,     0,   468,     0,     0,   236,   580,     0,   237,     0,
     469,     0,   464,     0,     0,   449,   180,     0,     0,     0,
    1062,     0,     0,     0,     0,   480,     0,   465,     0,     0,
    1042,  1043,  1044,  1045,     0,     0,   466,     0,     0,  1048,
     467,  1050,     0,     0,  1053,  1054,     0,     0,  1066,     0,
    1166,  1167,   182,     0,  1075,     0,     0,     0,   468,     0,
    -269,   373,     0,   451,     0,     0,   469,  1184,     0,  1094,
    1095,     0,     0,  1188,     0,  1189,     0,     0,  1192,  1193,
       0,     0,   407,  1198,     0,     0,     0,     0,  1203,   414,
    1100,     0,  1206,     0,     0,  1208,     0,     0,     0,   416,
       0,     0,   417,     0,   418,  1118,  1119,   419,     0,     0,
     420,     0,     0,   421,   422,     0,     0,   423,     0,     0,
     424,     0,     0,   425,   426,   427,   428,     0,     0,   429,
    1006,     0,     0,   431,     0,     0,   432,     0,     0,     0,
       0,     0,  1034,  1040,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   153,   153,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1180,     0,     0,     0,  1182,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1164,     0,     0,     0,     0,     0,
       0,  1165,     0,  1076,   153,   153,   153,     0,     0,   153,
       0,     0,     0,     0,   807,   812,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   662,   663,   664,   665,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1171,  1175,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   705,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   717,   718,     0,     0,   721,
     722,     0,   723,   724,     0,   725,   726,     0,   728,   729,
     730,   731,   732,   733,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   747,   748,     0,     0,     0,     0,
       0,   764,   765,     0,     0,   773,   774,   775,     0,   776,
     777,   778,   779,   780,   781,   782,     0,     0,   785,   786,
     787,   788,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   802,   803,   804,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   827,     0,    15,     0,     0,     0,   143,   144,    17,
     145,   146,     0,     0,     0,   147,   148,   149,   836,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,     0,   880,
     881,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   891,     0,     0,     0,   893,     0,     0,   894,
       0,     0,     0,   895,     0,   896,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   911,     0,     0,
     912,   913,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   919,     0,     0,   920,     0,     0,   921,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1030,  1031,  1032,     0,  1033,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1047,     0,  1049,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1055,     0,     0,  1058,  1059,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1063,     0,     0,     0,     0,  1064,     0,     0,  1065,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1085,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1125,     0,  1126,
    1127,     0,  1128,  1129,     0,     0,  1130,     0,     0,  1131,
    1132,    15,  1133,     0,     0,    16,   144,    17,   145,   146,
       0,     0,     0,   147,   148,   149,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    15,     6,     0,     7,     0,     0,
      17,   190,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    15,     0,     0,
       0,    16,     0,    17,   325,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      15,     0,     0,     0,    16,     0,    17,   190,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    15,     0,     0,     0,     0,     0,    17,
     190,     0,     0,  1070,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    15,     0,     0,     0,
       0,     0,    17,   190,     0,     0,  1195,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    15,
    1142,     0,     0,     0,     0,    17,   190,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    15,     0,     0,     0,    16,     0,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    15,     0,     0,     0,     0,
       0,    17,   190,     0,     0,     0,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94
};

static const yytype_int16 yycheck[] =
{
       4,    12,    13,   430,     8,     9,   342,   437,   671,   133,
      14,     8,   439,   446,   444,   168,     5,   130,     7,   132,
     446,   134,   706,   707,     5,   435,     7,   437,   438,     0,
      11,   441,   558,   443,   444,   445,   446,   447,   448,   444,
     445,   435,   666,    98,   438,     8,   572,     8,    23,   459,
     444,   445,   446,   447,    17,   168,   294,   435,     8,   437,
     438,     8,    12,     8,     8,     8,   444,   445,   446,    31,
      17,    33,    34,   436,     8,   438,     5,     8,     7,   442,
     704,   459,    11,   446,   447,    16,    17,    18,   201,   202,
     203,   204,  1125,   438,  1127,   333,  1129,   442,   102,   437,
     104,   446,   447,   448,   101,    67,   444,    82,     5,    98,
       7,   100,   116,   117,   118,   119,   120,     8,    99,   437,
     438,    20,    98,   441,   442,   443,   444,   445,   446,   447,
     448,    92,    31,   137,    95,    64,    31,    99,   134,   164,
     165,   459,   295,    88,    88,    88,    97,    94,    93,    93,
      93,   304,     8,    11,    88,   166,   160,   161,   842,    93,
      23,    17,    18,   167,   168,   169,   290,     5,    67,     7,
      70,   175,    67,    11,    73,   179,   180,   181,   182,     8,
     438,    31,   295,    99,   442,    14,    97,   191,   446,   447,
     448,   304,    98,   197,    85,     5,    87,     7,     8,    90,
      99,   205,    43,    44,    99,   201,   202,   203,   204,    98,
       5,   100,     7,    98,    30,   164,   165,    67,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,     5,    22,     7,     8,    98,    55,
      98,    98,   100,    56,    31,    83,   101,   102,    31,    99,
      99,    64,   110,   101,    37,    42,    30,    31,   262,    54,
      99,    77,    78,    98,   275,   100,   277,   278,   279,    82,
     281,   282,   283,   284,   285,   286,   287,   288,   289,    74,
      67,   139,   140,    99,    67,   143,    73,   291,    99,   293,
     438,   295,   296,    67,   442,    98,    31,     4,   446,    73,
     158,    75,    98,    28,    78,    98,    31,    42,   333,   334,
      31,     5,    99,     7,    98,   173,    21,    11,    75,    26,
      27,    27,    29,    99,    31,    31,    31,   961,   332,   963,
      99,   335,    67,    40,    59,    42,    42,    42,    73,   343,
      47,    48,    67,    50,   348,    51,    67,    99,    55,    55,
      71,    98,    98,   100,   212,   359,   360,    99,    65,   363,
      67,    67,    67,    68,    99,    99,    73,    73,    73,    76,
      99,    98,    79,   100,    81,    81,    81,    99,    99,  1042,
     238,    98,    98,   100,   333,   334,   101,   102,    22,    96,
      24,    99,    99,    99,    99,    98,    98,   100,    32,   257,
      27,   259,   260,    30,    31,   430,   961,   962,   412,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,    51,    52,    53,    61,    55,    27,
      99,    98,    30,   100,   459,    69,    99,   963,    65,   297,
      67,    32,    98,   301,   100,   449,   450,   451,    24,    98,
     308,   100,   978,    51,    52,    53,   314,    55,    99,   101,
     464,   465,   466,   467,   468,   469,    31,    14,    33,    34,
     328,   329,    99,   101,     5,   479,   480,   102,   104,    77,
      78,   430,    98,   103,   100,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
      85,    99,    67,   507,   362,   618,   916,   620,   941,   105,
     459,   941,    98,   101,   100,   941,    31,   521,    33,    34,
      14,   101,   635,   527,   637,   678,   639,    98,   961,   100,
     963,   961,   962,   963,    99,   961,   106,   963,   916,   652,
     544,   101,   666,   879,   125,   908,   909,   941,    98,    98,
     100,   100,    67,   916,     8,     9,   961,   962,   963,   563,
     564,   565,   115,   908,   909,   678,   679,   961,   962,    98,
      31,   100,    98,   178,   100,   579,   580,   940,   437,   438,
     704,    42,   136,     4,    99,   444,   445,   446,   165,    98,
       4,   100,   413,   706,   707,    98,   334,   100,   916,    98,
     715,   100,    63,   910,   188,    26,    67,  1088,    29,    31,
      31,    25,    73,    27,  1051,    29,    30,    31,   622,    40,
     624,   625,   618,    98,   620,   100,    47,    48,    42,    50,
     437,   642,   643,    47,    48,   437,    50,    51,    99,   635,
     437,   637,    64,   639,   912,    67,    67,   437,   659,    71,
     908,   909,    73,    67,   818,    76,   652,   154,    79,    73,
     157,    98,   159,   100,   118,   119,   120,    81,    31,    39,
      33,    34,     4,   437,    98,    96,   100,    99,    99,    49,
      98,   437,   100,   137,   881,  1053,    27,   691,   692,    30,
    1054,   941,    62,    84,    26,    86,    66,    29,    89,    98,
      91,   100,   446,   878,    67,    75,   160,   161,    40,   922,
      51,    52,    53,  1140,  1141,    47,    48,   280,    50,   216,
     217,   218,   219,    55,   877,   179,   180,   181,   182,   842,
    1140,  1141,   843,    98,    75,   100,    99,    78,    98,    98,
     100,   100,  1097,    98,    76,   100,  1136,    79,    31,   607,
     608,   205,    98,   961,   100,   963,  1099,  1124,    99,    42,
    1199,  1165,   191,   295,   877,    -1,   130,   771,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,    67,    -1,   790,   791,   792,    -1,
      73,    -1,    -1,   132,   133,   134,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,   821,   822,   823,
     824,   825,    -1,    -1,   828,    -1,   830,    -1,   832,   833,
     834,    -1,    -1,    -1,   838,    -1,    -1,    -1,    -1,    -1,
     698,   699,   700,    -1,   848,    -1,   850,   851,   852,    -1,
     854,   855,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   201,   202,   203,   204,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   878,    -1,    -1,   735,   736,   737,
     738,   220,   740,    -1,    -1,    -1,    -1,  1000,  1001,    -1,
     915,   916,   750,    -1,    -1,   753,    -1,    -1,    -1,    -1,
     758,   759,   760,    -1,    -1,    -1,    -1,    -1,    -1,   363,
      -1,    -1,    -1,   917,  1067,   940,   941,    -1,   922,    -1,
      -1,    -1,    -1,    -1,    -1,   929,   930,    -1,    -1,    -1,
    1043,  1044,  1045,    -1,    -1,  1048,   433,   276,   942,   943,
     798,   280,    -1,   801,    -1,    -1,  1099,   805,   806,    -1,
      -1,   290,   810,   811,  1067,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   915,   916,    -1,    -1,
     974,   975,   976,   977,    -1,    -1,    -1,    -1,    -1,   983,
      -1,   985,    -1,    -1,   988,   989,  1099,    -1,    -1,    -1,
    1143,   940,   941,    -1,    -1,   449,   450,   451,    -1,    -1,
      -1,    -1,    -1,    -1,  1000,  1001,    -1,    -1,    -1,    -1,
     464,   465,   466,   467,   468,   469,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   479,   480,    -1,    -1,    -1,
    1143,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,  1050,
      -1,    -1,    -1,    -1,    -1,    -1,  1199,  1043,  1044,  1045,
      -1,  1204,  1048,   507,    -1,  1208,  1060,  1061,    -1,    26,
      -1,    -1,    29,    -1,    31,    -1,    -1,   521,    -1,  1073,
    1074,    -1,    -1,   527,   413,    42,    -1,    -1,    -1,    -1,
      47,    48,    -1,    50,    -1,    -1,  1199,    -1,    -1,    -1,
     544,  1204,    -1,    31,    -1,  1208,    -1,  1101,    -1,    -1,
      67,    -1,    40,    -1,    42,    -1,    73,    45,    -1,   563,
     564,   565,    79,    -1,    81,  1140,  1141,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,   579,   580,    -1,    -1,    67,
      -1,    -1,    99,    -1,    -1,    73,    -1,    -1,    76,    -1,
      -1,    -1,    -1,    -1,  1148,    -1,  1004,  1005,    -1,    -1,
      -1,  1155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,     4,    -1,    -1,   622,    -1,
     624,   625,    -1,    -1,    -1,    -1,    -1,  1181,    -1,  1183,
    1038,  1039,    21,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      29,  1140,  1141,    -1,    -1,    -1,    35,    36,    -1,    38,
     697,    40,    41,    -1,    -1,   702,   703,    -1,    47,    48,
       4,    50,    -1,    -1,    -1,   712,    -1,    -1,    57,    -1,
    1078,    -1,    -1,    -1,    -1,    -1,    -1,    21,    -1,    -1,
     727,    -1,    26,    72,    -1,    29,    -1,    76,    -1,    -1,
      79,    35,    36,    -1,    38,    -1,    40,    41,   745,    -1,
      -1,    -1,    -1,    47,    48,   752,    50,    -1,    -1,   756,
      99,    -1,    -1,    57,    -1,   762,    -1,    -1,     4,   766,
      -1,    -1,   769,    -1,    -1,    -1,    -1,    -1,    72,   618,
     619,   620,    76,    -1,    -1,    79,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    -1,    31,   635,    -1,   637,    -1,
     639,    -1,    -1,    -1,    -1,    99,    42,    -1,    -1,    -1,
      -1,    47,    48,   652,    50,  1173,    -1,   771,    -1,  1177,
      -1,   106,    -1,    -1,    -1,    -1,   823,   666,    -1,    -1,
      -1,    67,   671,   672,   673,    -1,    -1,    73,    -1,   678,
     679,   838,    -1,    79,     4,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   853,    -1,    -1,   856,
      96,    -1,    -1,   860,    -1,   704,    26,   706,   707,    29,
      -1,    31,   711,    -1,   713,    -1,   715,    -1,    -1,    -1,
      40,    -1,    42,    -1,   838,   170,   171,    47,    48,    27,
      50,    -1,    -1,    31,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    52,    53,    76,    55,    -1,    79,
      58,   206,    -1,     4,   878,   922,    -1,    -1,   213,    67,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    75,    -1,    99,
      78,    -1,    -1,    81,    -1,    26,    27,    -1,    29,    -1,
      31,    -1,    -1,    -1,   239,    -1,    -1,    -1,    -1,    40,
      -1,    42,    -1,   917,    -1,    46,    47,    48,   922,    50,
      -1,    52,    53,   258,    55,   929,   930,    58,    -1,   264,
     265,    -1,   267,    -1,    -1,    -1,    67,    -1,   942,   943,
      -1,    -1,    73,    -1,    -1,    76,    77,    -1,    79,    -1,
      81,    -1,    27,    -1,    -1,    30,    31,    -1,    -1,    -1,
    1007,    -1,    -1,    -1,    -1,    96,    -1,    42,    -1,    -1,
     974,   975,   976,   977,    -1,    -1,    51,    -1,    -1,   983,
      55,   985,    -1,    -1,   988,   989,    -1,    -1,  1035,    -1,
    1162,  1163,    67,    -1,  1041,    -1,    -1,    -1,    73,    -1,
      75,   336,    -1,    78,    -1,    -1,    81,  1179,    -1,  1056,
    1057,    -1,    -1,  1185,    -1,  1187,    -1,    -1,  1190,  1191,
      -1,    -1,   357,  1195,    -1,    -1,    -1,    -1,  1200,   364,
    1077,    -1,  1204,    -1,    -1,  1207,    -1,    -1,    -1,   374,
      -1,    -1,   377,    -1,   379,  1092,  1093,   382,    -1,    -1,
     385,    -1,    -1,   388,   389,    -1,    -1,   392,    -1,    -1,
     395,    -1,    -1,   398,   399,   400,   401,    -1,    -1,   404,
     959,    -1,    -1,   408,    -1,    -1,   411,    -1,    -1,    -1,
      -1,    -1,   971,   972,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1000,  1001,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1172,    -1,    -1,    -1,  1176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1148,    -1,    -1,    -1,    -1,    -1,
      -1,  1155,    -1,  1042,  1043,  1044,  1045,    -1,    -1,  1048,
      -1,    -1,    -1,    -1,  1053,  1054,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   540,   541,   542,   543,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1164,  1165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   623,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   640,   641,    -1,    -1,   644,
     645,    -1,   647,   648,    -1,   650,   651,    -1,   653,   654,
     655,   656,   657,   658,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   669,   670,    -1,    -1,    -1,    -1,
      -1,   676,   677,    -1,    -1,   680,   681,   682,    -1,   684,
     685,   686,   687,   688,   689,   690,    -1,    -1,   693,   694,
     695,   696,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   708,   709,   710,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   746,    -1,     4,    -1,    -1,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    16,    17,    18,   763,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    -1,    -1,    -1,   844,
     845,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   857,    -1,    -1,    -1,   861,    -1,    -1,   864,
      -1,    -1,    -1,   868,    -1,   870,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   882,    -1,    -1,
     885,   886,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   897,    -1,    -1,   900,    -1,    -1,   903,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     965,   966,   967,    -1,   969,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   982,    -1,   984,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   999,    -1,    -1,  1002,  1003,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1015,    -1,    -1,    -1,    -1,  1020,    -1,    -1,  1023,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1046,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1102,    -1,  1104,
    1105,    -1,  1107,  1108,    -1,    -1,  1111,    -1,    -1,  1114,
    1115,     4,  1117,    -1,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,     4,     5,    -1,     7,    -1,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,     4,    -1,    -1,
      -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
       4,    -1,    -1,    -1,     8,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,     4,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,     4,    -1,    -1,    -1,
      -1,    -1,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,     4,
       5,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,     4,    -1,    -1,    -1,     8,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,     4,    -1,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   108,   113,   117,   438,     0,     5,     7,    54,    74,
     437,   437,   439,   439,   438,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,   109,   423,   444,   445,   423,
     110,   114,   438,   118,   438,    98,    98,   111,   427,   427,
      97,    11,   116,   428,   437,   120,   438,   429,    56,    64,
      82,   115,   122,   123,   137,   119,   121,   122,     8,   430,
     431,   439,   439,   439,   439,   124,   124,    23,   134,   112,
      98,   100,   434,     8,     9,    11,    12,    16,    17,    18,
     432,   443,   444,   109,   441,   443,   109,   422,   445,   441,
      43,    44,   125,   129,   138,   138,   439,   438,   438,   438,
      98,   100,   426,   442,   426,   438,   426,   439,   439,    28,
      31,    59,    67,   139,   140,   141,   142,   143,   143,   423,
      11,   424,   435,   444,   433,   427,   427,   438,   128,   423,
     128,   439,   439,   439,   439,    70,   144,   151,   151,   135,
     430,    99,    97,    99,   126,   130,   441,   441,   441,   441,
     439,   427,     4,    21,    26,    29,    35,    36,    38,    40,
      41,    47,    48,    50,    57,    72,    76,    79,    99,   152,
     153,   167,   172,   177,   225,   229,   230,   240,   248,   256,
     266,   281,   287,   303,   314,   333,   341,    99,    98,    98,
     100,   436,   438,   427,    98,    98,   100,   131,   426,   426,
     426,   426,    12,   109,   149,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   438,   427,   438,   427,   438,   438,     8,   427,   427,
     427,   150,   145,   423,   101,   109,   311,   394,   395,   423,
     423,   423,   109,   345,   395,   423,   423,   423,   423,   423,
     423,   423,   423,   179,   423,    11,   109,   425,   445,   446,
     137,   435,   438,   127,   132,   438,    98,   100,   146,   282,
     424,   304,   396,   438,   267,   231,   154,   342,   438,   168,
     226,   173,   241,   249,   257,   334,   315,    98,   288,   438,
     438,   136,    99,    71,    99,   133,   137,   141,   142,    99,
     133,   141,   142,   427,    98,   100,   283,    98,   394,    98,
     100,   268,    98,   100,   232,    98,   100,   155,    98,    98,
     100,   169,    98,   100,   227,    98,   100,   174,    98,    98,
      98,    98,   100,   335,    98,   100,   316,   427,    98,   100,
     289,    99,   438,   439,   427,   147,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     182,   427,   427,   149,   148,   284,   306,   270,   233,   156,
     344,   170,   228,   175,   243,   251,   259,   336,   317,    30,
      73,    78,   141,   142,   162,   178,   183,   223,   224,   290,
     426,    99,   141,   142,    27,    42,    51,    55,    73,    81,
      99,   141,   142,   163,   171,   361,   366,   371,   382,    25,
      96,   141,   142,   162,   171,   229,   275,   305,   312,   361,
     141,   142,   163,   171,   224,   230,   240,   248,   256,   269,
     273,   274,   275,   281,   361,   366,   371,    65,    99,   141,
     142,   163,   171,   177,   225,   229,   312,   361,   366,   375,
     382,    20,    99,   141,   142,   157,   162,    37,   141,   142,
     343,    99,   141,   142,   163,   171,    99,   141,   142,   163,
     177,   225,   229,   312,    22,    99,   141,   142,   163,   171,
     176,   141,   142,   163,   171,   223,   224,   242,   245,   361,
     366,   371,   382,    52,    53,    58,   141,   142,   163,   171,
     223,   250,   253,   361,   366,   376,   378,   380,   382,    46,
      77,   141,   142,   163,   171,   177,   225,   229,   258,   312,
     361,   366,   376,   378,   380,   382,   385,   388,    99,   141,
     142,   163,   171,   177,   225,   229,   382,    45,    60,    99,
     141,   142,   163,   171,   177,   225,   322,   330,   439,   439,
     439,    99,    75,   180,    21,    68,    99,   141,   142,   163,
     171,   294,   299,   361,   439,   439,   439,   439,   439,   439,
     285,   286,   439,   439,   308,   310,    99,   309,   307,    99,
     272,   271,   439,   238,   236,   239,   237,   234,   235,   439,
     439,    99,   427,   427,   427,   427,   439,    99,   180,   244,
     247,   439,   439,   439,    99,   180,   252,   255,   439,   439,
     262,   263,   265,    99,   264,   260,   261,   338,   339,   340,
     337,   438,   438,   318,   319,   320,   321,   441,    85,    87,
      90,   109,   166,   441,   439,   427,   439,   439,   292,   293,
     291,   367,   441,   372,   441,   164,   441,   427,   427,   423,
     423,   427,   427,   427,   427,   427,   427,   441,   427,   427,
     427,   427,   427,   427,   423,    84,    86,    89,    91,   346,
     347,   348,   349,   353,   357,   425,   246,   427,   427,    17,
      94,   109,   379,   418,    18,   109,   377,   418,   419,    92,
      95,   109,   381,   254,   427,   427,   109,   389,   424,   109,
     386,   397,   424,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   323,   331,   427,   427,   427,   427,   426,
     438,   438,   438,   426,   426,   425,   109,   302,   397,   109,
     297,   397,   427,   427,   427,    88,    93,   109,   369,   426,
      88,    93,   109,   374,   383,   166,   362,   276,   313,   426,
     158,   438,   438,   438,   438,   438,   426,   427,   438,   426,
     438,   426,   438,   438,   438,   426,   427,   426,   390,   426,
     387,   426,   439,   398,    98,    98,   181,   300,   438,   295,
     438,   438,   438,   368,   438,   438,   373,    98,   100,   384,
     165,    98,   100,   363,    98,   100,   277,   316,    98,   100,
     159,   350,   354,   426,   358,   391,   426,   439,   397,   396,
     427,   427,    98,   100,   184,    98,    98,   100,   296,   426,
     426,   427,   426,   427,   427,   427,   427,    98,   100,   351,
      98,   100,   355,    98,   100,   359,   424,   387,   325,   325,
     185,   427,   427,   427,   194,   364,   278,    83,   160,   427,
     427,   427,   392,   177,   225,   229,   324,   382,   332,    61,
      69,   176,   186,   187,   198,   199,   209,   210,   212,   213,
     306,   298,    33,    34,    99,   141,   142,   221,   222,    99,
     141,   142,    99,   141,   142,   163,   171,   229,   361,   439,
      99,   352,   356,   360,   391,   327,   328,   329,    99,   326,
      99,   439,   439,    99,    39,    49,    62,    66,   188,   189,
     191,   195,   207,    32,   201,    24,   215,   301,    27,    51,
      99,   141,   142,   224,   365,   370,   375,   376,   378,   382,
     439,   439,   280,   279,    88,    93,   109,   161,    99,   223,
     224,   365,   370,   376,   378,   382,   385,    99,   223,   224,
     382,   385,    99,   180,   223,   224,   365,   370,   376,   378,
     427,   427,   427,   427,   109,   399,   400,   401,    88,    93,
     109,   211,   439,   439,   439,   439,   180,   427,   439,   427,
     439,   214,    99,   439,   439,   427,   441,   441,   427,   427,
     438,   438,   426,   427,   427,   427,   426,   101,   402,   403,
      14,   405,   406,   438,   438,   426,   109,   190,   418,   192,
     441,   196,   441,   393,   441,   427,   202,   441,   200,   216,
     423,   215,   369,   374,   426,   426,   424,   404,   101,   407,
     426,   438,    98,   100,   193,    98,   100,   197,    98,   100,
     208,    98,   100,   203,   201,    98,   100,   217,   426,   426,
     410,   402,   406,   408,   424,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   102,   409,   410,   194,   194,   194,
     204,   218,     5,   440,   403,    99,    99,    99,    80,    99,
     141,   142,   162,   171,   205,    63,    99,   141,   142,   162,
     171,   219,   411,   424,   439,   439,   440,   440,    16,    17,
      18,   109,   206,   420,   421,   109,   220,   419,   103,   104,
     426,   438,   426,   438,   440,    85,   412,   417,   440,   440,
     105,   101,   440,   440,   106,    14,   413,   414,   440,   415,
     101,   414,   424,   440,   416,   424,   440,   101,   440,   424
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   107,   108,   108,   110,   109,   111,   112,   111,   114,
     115,   113,   116,   116,   116,   116,   118,   119,   117,   120,
     120,   121,   120,   122,   123,   124,   124,   124,   126,   125,
     127,   127,   127,   127,   127,   128,   130,   129,   131,   131,
     132,   132,   132,   132,   133,   135,   136,   134,   137,   138,
     138,   138,   138,   138,   139,   140,   141,   142,   143,   143,
     145,   144,   146,   147,   146,   148,   148,   148,   150,   149,
     149,   151,   151,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   154,   153,   155,   155,   156,   156,   156,
     156,   156,   158,   157,   159,   159,   160,   161,   161,   161,
     162,   164,   165,   163,   166,   166,   166,   166,   168,   167,
     169,   169,   170,   170,   170,   170,   170,   171,   173,   172,
     174,   174,   175,   175,   175,   175,   175,   175,   176,   178,
     177,   179,   181,   180,   182,   183,   182,   182,   182,   182,
     182,   182,   184,   184,   185,   186,   186,   186,   186,   186,
     186,   187,   187,   187,   187,   187,   187,   188,   189,   190,
     190,   191,   192,   193,   193,   194,   194,   194,   194,   194,
     195,   196,   197,   197,   199,   198,   200,   200,   201,   202,
     203,   203,   204,   204,   204,   204,   204,   204,   205,   206,
     206,   207,   208,   208,   209,   210,   211,   211,   211,   213,
     212,   214,   214,   215,   216,   217,   217,   218,   218,   218,
     218,   218,   218,   219,   220,   220,   221,   222,   223,   224,
     226,   225,   227,   227,   228,   228,   228,   228,   228,   228,
     228,   228,   229,   229,   229,   229,   229,   229,   229,   231,
     230,   232,   232,   233,   234,   233,   233,   235,   233,   233,
     233,   233,   233,   233,   236,   233,   237,   233,   238,   233,
     239,   233,   241,   242,   240,   243,   244,   243,   243,   245,
     246,   243,   243,   247,   243,   243,   243,   243,   243,   243,
     243,   249,   250,   248,   251,   252,   251,   251,   253,   254,
     251,   251,   255,   251,   251,   251,   251,   251,   251,   251,
     251,   257,   258,   256,   259,   260,   259,   259,   261,   259,
     259,   259,   259,   259,   259,   259,   259,   259,   259,   262,
     259,   263,   259,   264,   259,   265,   259,   267,   266,   268,
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
     306,   311,   311,   313,   312,   315,   314,   316,   316,   317,
     317,   317,   317,   317,   318,   317,   319,   317,   320,   317,
     321,   317,   323,   324,   322,   325,   326,   325,   327,   325,
     328,   325,   329,   325,   331,   332,   330,   334,   333,   335,
     335,   336,   337,   336,   336,   336,   336,   336,   338,   336,
     339,   336,   340,   336,   342,   343,   341,   344,   344,   344,
     344,   345,   345,   346,   346,   347,   347,   347,   348,   350,
     349,   351,   351,   352,   352,   352,   352,   352,   352,   352,
     352,   352,   354,   353,   355,   355,   356,   356,   356,   356,
     356,   358,   357,   359,   359,   360,   360,   360,   360,   360,
     360,   360,   360,   362,   361,   363,   363,   364,   364,   364,
     365,   367,   368,   366,   369,   369,   369,   370,   372,   373,
     371,   374,   374,   374,   375,   376,   377,   377,   378,   379,
     379,   379,   380,   381,   381,   381,   383,   382,   384,   384,
     385,   386,   386,   387,   387,   388,   390,   389,   389,   392,
     391,   391,   393,   394,   395,   396,   396,   398,   397,   400,
     399,   401,   399,   399,   402,   403,   404,   404,   405,   406,
     407,   407,   408,   409,   409,   410,   410,   411,   412,   413,
     414,   415,   415,   416,   416,   417,   418,   419,   419,   420,
     420,   421,   421,   422,   422,   423,   423,   424,   424,   425,
     425,   425,   426,   426,   427,   427,   427,   429,   428,   430,
     431,   431,   432,   432,   432,   433,   433,   434,   434,   435,
     435,   436,   436,   437,   437,   438,   438,   439,   440,   440,
     442,   441,   441,   443,   443,   443,   443,   443,   443,   443,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   445,
     446
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
       5,     1,     1,     4,     0,     4,     0,     3,     4,     1,
       1,     4,     0,     2,     2,     2,     2,     2,     4,     2,
       1,     4,     1,     4,     4,     4,     2,     2,     1,     0,
       3,     0,     2,     5,     1,     1,     4,     0,     2,     2,
       2,     2,     2,     4,     2,     1,     4,     4,     4,     4,
       0,     5,     1,     4,     0,     2,     2,     2,     3,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     0,
       5,     1,     4,     0,     0,     4,     2,     0,     4,     2,
       2,     2,     2,     2,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     0,     9,     0,     0,     4,     2,     0,
       0,     5,     2,     0,     4,     2,     2,     2,     2,     2,
       2,     0,     0,     9,     0,     0,     4,     2,     0,     0,
       5,     2,     0,     4,     2,     2,     2,     2,     2,     2,
       2,     0,     0,     9,     0,     0,     4,     2,     0,     4,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     5,     1,
       0,     5,     0,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     0,     4,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     5,     1,     4,     0,     0,
       4,     2,     0,     4,     2,     2,     2,     2,     2,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     5,     1,     4,     2,     1,
       0,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     0,     9,     2,     1,     0,     0,     9,     0,     0,
       4,     2,     2,     2,     2,     0,     4,     0,     4,     0,
       4,     2,     1,     0,     5,     0,     5,     1,     4,     0,
       2,     2,     2,     2,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     0,     8,     0,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     0,     8,     0,     5,     1,
       4,     0,     0,     4,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     0,     9,     0,     2,     2,
       4,     2,     1,     1,     2,     1,     1,     1,     3,     0,
       4,     1,     4,     0,     2,     3,     2,     2,     2,     2,
       2,     2,     0,     4,     1,     4,     0,     2,     3,     2,
       2,     0,     4,     1,     4,     0,     3,     2,     2,     2,
       2,     2,     2,     0,     5,     1,     4,     0,     2,     2,
       4,     0,     0,     6,     2,     2,     1,     4,     0,     0,
       6,     2,     2,     1,     4,     4,     2,     1,     4,     2,
       2,     1,     4,     2,     2,     1,     0,     5,     1,     4,
       3,     2,     2,     3,     1,     3,     0,     3,     2,     0,
       4,     1,     1,     2,     2,     0,     2,     0,     3,     0,
       2,     0,     2,     1,     3,     2,     0,     2,     3,     2,
       0,     2,     2,     0,     2,     0,     6,     5,     5,     5,
       4,     0,     2,     0,     5,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     1,     1,     2,
       2,     1,     2,     4,     0,     2,     2,     0,     4,     2,
       0,     1,     0,     1,     3,     0,     5,     1,     4,     0,
       3,     2,     5,     1,     1,     0,     2,     2,     0,     1,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     1,
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
          case 128: /* tmp_identifier_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 185: /* type_stmtsep  */

      { if (read_all) {
                yang_delete_type(module, ((*yyvaluep).type));
              }
            }

        break;

    case 270: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

        break;

    case 344: /* deviation_opt_stmt  */

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
struct lys_include inc;
struct lys_module *trg = NULL;
struct lys_node *tpdf_parent = NULL, *data_node = NULL;
void *actual = NULL;
int config_inherit = 0, actual_type = 0;
int64_t cnt_val;
void *yang_type;


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
                           case PATTERN_KEYWORD:
                             (yyval.str) = "pattern";
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

    { actual = (yyvsp[-2].v);
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 171:

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

  case 174:

    { if (read_all) {
                        if (size_arrays->node[size_arrays->next].enm) {
                          ((struct yang_type *)actual)->type->info.enums.enm = calloc(size_arrays->node[size_arrays->next++].enm, sizeof(struct lys_type_enum));
                          if (!((struct yang_type *)actual)->type->info.enums.enm) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                        ((struct yang_type *)actual)->base = LY_TYPE_ENUM;
                        yang_type = actual;
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

  case 178:

    { if (read_all) {
               if (yang_check_enum((yyvsp[-1].v), actual, &cnt_val, actual_type)) {
                 YYABORT;
               }
               actual = (yyvsp[-1].v);
               actual_type = TYPE_KEYWORD;
             } else {
               size_arrays->node[cnt_val].enm++; /* count of enum*/
             }
           }

    break;

  case 179:

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

  case 182:

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

  case 183:

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

  case 184:

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

  case 185:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 186:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 187:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 188:

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

  case 189:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 190:

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

  case 191:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 196:

    { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }

    break;

  case 197:

    { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }

    break;

  case 198:

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

  case 199:

    { if (read_all) {
                        if (size_arrays->node[size_arrays->next].bit) {
                          ((struct yang_type *)actual)->type->info.bits.bit = calloc(size_arrays->node[size_arrays->next++].bit, sizeof(struct lys_type_bit));
                          if (!((struct yang_type *)actual)->type->info.bits.bit) {
                            LOGMEM;
                            YYABORT;
                          }
                        }
                        ((struct yang_type *)actual)->base = LY_TYPE_BITS;
                        yang_type = actual;
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

  case 203:

    { if (read_all) {
                      if (yang_check_bit((yyvsp[-2].v), actual, &cnt_val, actual_type)) {
                        YYABORT;
                      }
                      actual = (yyvsp[-2].v);
                    } else {
                      size_arrays->node[cnt_val].bit++; /* count of bit*/
                    }
                  }

    break;

  case 204:

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

  case 207:

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

  case 208:

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

  case 209:

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

  case 210:

    { if (read_all) {
                                  if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 211:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 212:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 213:

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

  case 214:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 215:

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

  case 220:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
                                                         }
                                                       }

    break;

  case 224:

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

  case 225:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 226:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 227:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 228:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 229:

    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }

    break;

  case 230:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 231:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 239:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 243:

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

  case 244:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 246:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 247:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 249:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 250:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 251:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 252:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 253:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 254:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 256:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 258:

    { if (read_all) {
                                                 actual = (yyvsp[-1].nodes).container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                               }
                                             }

    break;

  case 260:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 262:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 263:

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

  case 265:

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

  case 266:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }

    break;

  case 268:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 269:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYABORT;
                       }
                     }

    break;

  case 270:

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

  case 271:

    { (yyval.nodes) = (yyvsp[-4].nodes);}

    break;

  case 272:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 273:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 275:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYABORT;}
                                    s = NULL;
                                  }

    break;

  case 276:

    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i), 0)) {
                                                 YYABORT;
                                               }
                                             }
                                           }

    break;

  case 277:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 278:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 279:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 280:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 281:

    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYABORT;}
                                                               data_node = actual;
                                                               s=NULL;
                                                             }
                                                           }

    break;

  case 282:

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
                                           }
                                         }
                                       }

    break;

  case 284:

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

  case 285:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }

    break;

  case 287:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 288:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYABORT;
                          }
                        }

    break;

  case 289:

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

  case 290:

    { (yyval.nodes) = (yyvsp[-4].nodes); }

    break;

  case 291:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 292:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 294:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 295:

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

  case 296:

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

  case 297:

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

  case 298:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 299:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 300:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 301:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 302:

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

  case 304:

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

  case 305:

    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }

    break;

  case 307:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 308:

    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }

    break;

  case 310:

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

  case 311:

    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }

    break;

  case 312:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 313:

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

  case 314:

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

  case 315:

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

  case 316:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 317:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 318:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 319:

    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }

    break;

  case 321:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 323:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 325:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 326:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 327:

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

  case 330:

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

  case 332:

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

  case 333:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 334:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 335:

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

  case 336:

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

  case 337:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 338:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 339:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 340:

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

  case 341:

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

  case 342:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 343:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 351:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 355:

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

  case 356:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 358:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 359:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 360:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 361:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 362:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 364:

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

  case 368:

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

  case 369:

    { actual = (yyvsp[-1].nodes).anyxml; actual_type = ANYXML_KEYWORD; }

    break;

  case 371:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).anyxml, s, unres, ANYXML_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 372:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 374:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 375:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "anyxml", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 376:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 377:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 378:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 379:

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

  case 380:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYABORT;
                        }
                      }
                    }

    break;

  case 383:

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

  case 384:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 386:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 387:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 388:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 389:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 390:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 392:

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

  case 394:

    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 400:

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

  case 401:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 402:

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

  case 403:

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

  case 404:

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

  case 405:

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

  case 406:

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

  case 407:

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

  case 408:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 409:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 410:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYABORT;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 411:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYABORT;
                                          }
                                        }

    break;

  case 415:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYABORT;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 416:

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

  case 418:

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

  case 419:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 421:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 422:

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

  case 423:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 424:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 425:

    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }

    break;

  case 426:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 427:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                      data_node = actual;
                                    }
                                  }

    break;

  case 428:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 429:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 430:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 433:

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

  case 434:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 435:

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

  case 436:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 439:

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

  case 440:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 441:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 442:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 443:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 444:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 446:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 448:

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

  case 449:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 450:

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

  case 451:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 452:

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

  case 453:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYABORT;
                                        }
                                      }

    break;

  case 455:

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

  case 456:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).node.ptr_inout;
                                         actual_type = INPUT_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                       }
                                     }

    break;

  case 458:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 460:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 462:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 463:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 464:

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

  case 465:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYABORT;
                                         }
                                       }

    break;

  case 467:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 468:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 469:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 471:

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

  case 472:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).notif;
                                      actual_type = NOTIFICATION_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 474:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 475:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 476:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 477:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 478:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 480:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 482:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 484:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 485:

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

  case 487:

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

  case 488:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 489:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 490:

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

  case 493:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 499:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 503:

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

  case 504:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 505:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 506:

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

  case 507:

    { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 508:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 509:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 510:

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

  case 511:

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

  case 512:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 516:

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

  case 517:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 518:

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

  case 519:

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

  case 520:

    { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 521:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 525:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }

    break;

  case 526:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target) == -1) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 527:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 528:

    { if (read_all) {
                                               if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                 YYABORT;
                                               }
                                               s = NULL;
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }
                                           }

    break;

  case 529:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 530:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 531:

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

  case 532:

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

  case 533:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 538:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 539:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 540:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 541:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 542:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 543:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 544:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 545:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 546:

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

  case 547:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 548:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 549:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 550:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 551:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 552:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 553:

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

  case 555:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 556:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 557:

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

  case 558:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 559:

    { (yyval.uint) = 0; }

    break;

  case 560:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 561:

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

  case 562:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 563:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 564:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 565:

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

  case 566:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 576:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 579:

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

  case 582:

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

  case 583:

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

  case 587:

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

  case 589:

    { tmp_s = yyget_text(scanner); }

    break;

  case 590:

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

  case 591:

    { tmp_s = yyget_text(scanner); }

    break;

  case 592:

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

  case 616:

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

  case 617:

    { (yyval.uint) = 0; }

    break;

  case 618:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 619:

    { (yyval.i) = 0; }

    break;

  case 620:

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

  case 626:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 631:

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

  case 637:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 660:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 749:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 750:

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
