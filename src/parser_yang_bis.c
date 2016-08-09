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
#define YYLAST   3077

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  338
/* YYNRULES -- Number of rules.  */
#define YYNRULES  746
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1204

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
    3049,  3049,  3062,  3076,  3062,  3083,  3103,  3103,  3111,  3111,
    3116,  3116,  3126,  3140,  3126,  3147,  3147,  3157,  3161,  3166,
    3193,  3200,  3206,  3211,  3216,  3216,  3224,  3224,  3229,  3229,
    3236,  3245,  3236,  3258,  3277,  3284,  3291,  3301,  3302,  3304,
    3309,  3311,  3312,  3313,  3316,  3318,  3318,  3324,  3325,  3329,
    3351,  3359,  3367,  3383,  3391,  3398,  3405,  3416,  3428,  3428,
    3434,  3435,  3439,  3461,  3469,  3480,  3490,  3499,  3499,  3505,
    3506,  3510,  3515,  3521,  3529,  3537,  3544,  3551,  3562,  3574,
    3574,  3577,  3578,  3582,  3583,  3588,  3594,  3596,  3597,  3596,
    3600,  3601,  3602,  3617,  3619,  3620,  3619,  3623,  3624,  3625,
    3640,  3642,  3644,  3645,  3666,  3668,  3669,  3670,  3691,  3693,
    3694,  3695,  3707,  3707,  3715,  3716,  3721,  3723,  3724,  3726,
    3727,  3729,  3731,  3731,  3740,  3743,  3743,  3754,  3757,  3767,
    3788,  3790,  3791,  3794,  3794,  3813,  3813,  3822,  3822,  3831,
    3834,  3836,  3838,  3839,  3841,  3843,  3845,  3846,  3848,  3850,
    3851,  3853,  3854,  3856,  3858,  3861,  3865,  3867,  3868,  3870,
    3871,  3873,  3875,  3886,  3887,  3890,  3891,  3903,  3904,  3906,
    3907,  3909,  3910,  3916,  3917,  3920,  3921,  3922,  3948,  3949,
    3952,  3953,  3954,  3957,  3957,  3965,  3967,  3968,  3970,  3971,
    3972,  3974,  3975,  3977,  3978,  3980,  3981,  3983,  3984,  3986,
    3987,  3990,  3991,  3994,  3996,  3997,  4000,  4000,  4009,  4011,
    4012,  4013,  4014,  4015,  4016,  4017,  4019,  4020,  4021,  4022,
    4023,  4024,  4025,  4026,  4027,  4028,  4029,  4030,  4031,  4032,
    4033,  4034,  4035,  4036,  4037,  4038,  4039,  4040,  4041,  4042,
    4043,  4044,  4045,  4046,  4047,  4048,  4049,  4050,  4051,  4052,
    4053,  4054,  4055,  4056,  4057,  4058,  4059,  4060,  4061,  4062,
    4063,  4064,  4065,  4066,  4067,  4068,  4069,  4070,  4071,  4072,
    4073,  4074,  4075,  4076,  4077,  4078,  4079,  4080,  4081,  4082,
    4083,  4084,  4085,  4086,  4087,  4088,  4089,  4090,  4091,  4092,
    4093,  4094,  4095,  4096,  4097,  4100,  4109
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
  "$@87", "output_stmt", "$@88", "$@89", "notification_stmt", "$@90",
  "notification_end", "notification_opt_stmt", "$@91", "$@92", "$@93",
  "deviation_stmt", "$@94", "$@95", "deviation_opt_stmt",
  "deviation_arg_str", "deviate_body_stmt", "deviate_stmts",
  "deviate_not_supported_stmt", "deviate_add_stmt", "$@96",
  "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt", "$@97",
  "deviate_delete_end", "deviate_delete_opt_stmt", "deviate_replace_stmt",
  "$@98", "deviate_replace_end", "deviate_replace_opt_stmt", "when_stmt",
  "$@99", "when_end", "when_opt_stmt", "config_stmt", "config_read_stmt",
  "$@100", "$@101", "config_arg_str", "mandatory_stmt",
  "mandatory_read_stmt", "$@102", "$@103", "mandatory_arg_str",
  "presence_stmt", "min_elements_stmt", "min_value_arg_str",
  "max_elements_stmt", "max_value_arg_str", "ordered_by_stmt",
  "ordered_by_arg_str", "must_stmt", "$@104", "must_end", "unique_stmt",
  "unique_arg_str", "unique_arg", "key_stmt", "key_arg_str", "$@105",
  "key_opt", "$@106", "range_arg_str", "absolute_schema_nodeid",
  "absolute_schema_nodeids", "absolute_schema_nodeid_opt",
  "descendant_schema_nodeid", "$@107", "path_arg_str", "$@108", "$@109",
  "absolute_path", "absolute_paths", "absolute_path_opt", "relative_path",
  "relative_path_part1", "relative_path_part1_opt", "descendant_path",
  "descendant_path_opt", "path_predicate", "path_equality_expr",
  "path_key_expr", "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@110", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@111", "strings", "identifier", "identifiers",
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

#define YYPACT_NINF -699

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-699)))

#define YYTABLE_NINF -602

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -699,     7,  -699,  -699,   236,  -699,  -699,  -699,    66,    66,
    -699,  -699,  2888,  2888,    66,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,   -70,    66,   -59,    66,  -699,  -699,   -34,   243,   243,
    -699,  -699,   128,  -699,  -699,    15,   358,    66,    66,    66,
      66,  -699,  -699,  -699,  -699,  -699,    58,  -699,  -699,    85,
    1873,  -699,  2237,  2888,  2237,   105,   105,    66,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,   165,  -699,  -699,   165,  -699,   165,
      66,    66,  -699,  -699,   320,   320,  2888,    66,  2330,    66,
    -699,  -699,  -699,  -699,  -699,    66,  -699,  2888,  2888,    66,
      66,    66,    66,  -699,  -699,  -699,  -699,    23,    23,  -699,
    -699,    66,     2,  -699,    14,    59,   243,    66,  -699,  -699,
    -699,  2237,  2237,  2237,  2237,    66,  -699,  1064,  1563,    19,
     195,  -699,  -699,  -699,    33,   206,   165,   165,   165,   165,
     174,   243,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,   370,   243,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  2888,     5,  2888,  2888,  2888,
       5,  2888,  2888,  2888,  2888,  2888,  2888,  2888,  2888,  2888,
    2423,    66,   243,    66,    72,  2330,    66,  -699,   243,   243,
     243,  -699,   300,  -699,  2981,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,    69,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,    92,    66,   230,   231,    66,  -699,  -699,  -699,   332,
    -699,    98,   133,    66,   354,   359,   379,   104,    66,   450,
     455,   472,   141,   151,   157,   473,   482,  -699,   496,    66,
      66,   100,  -699,    66,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,   243,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,   243,  -699,  -699,
    -699,  -699,    66,   174,   243,  -699,   243,   243,   243,   243,
     243,   243,   243,   243,   243,   243,   243,   243,   243,   243,
     178,   243,   243,   165,   -12,   400,  1186,  1281,   295,   139,
      41,   274,   614,   138,  1374,  1319,  1659,  1077,  1092,    66,
      66,    66,  -699,  -699,  -699,   170,   204,  -699,  -699,   265,
    -699,  -699,  -699,  -699,    66,    66,    66,    66,    66,    66,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,    66,
      66,  -699,  -699,  -699,  -699,  -699,  -699,   189,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   215,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,    66,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,    66,  -699,  -699,  -699,  -699,  -699,    66,  -699,  -699,
     219,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,    66,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,   224,   204,  -699,
    -699,  -699,  -699,    66,    66,    66,  -699,  -699,  -699,  -699,
    -699,   228,   204,  -699,  -699,  -699,  -699,  -699,  -699,    66,
      66,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   235,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  2237,   113,  2237,
    -699,    66,  -699,    66,    66,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  2237,  -699,  2237,  -699,  2237,  -699,
    -699,  2888,  2888,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  2237,  -699,  -699,  -699,  -699,  -699,  -699,  2888,   306,
    -699,   243,   243,   243,   243,  2423,  -699,  -699,  -699,  -699,
      13,   199,    40,  -699,  -699,  -699,  -699,  2516,  2516,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,    66,
      66,  -699,  -699,  -699,  -699,   165,  -699,  -699,  -699,  -699,
     165,   165,  2423,   243,  2516,  2516,  -699,  -699,  -699,    80,
     165,    86,  -699,   113,  -699,   243,   243,  -699,  -699,   243,
     243,   243,   243,   243,   243,   165,   243,   243,   243,   243,
     243,   243,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,   165,  -699,   243,   243,  -699,  -699,  -699,
     165,  -699,  -699,  -699,   165,  -699,  -699,  -699,  -699,  -699,
     165,  -699,   243,   243,   165,  -699,  -699,   165,  -699,    52,
    -699,   243,   243,   243,   243,   243,   243,   243,   243,   243,
     205,   242,   243,   243,   243,   243,  -699,    66,    66,    66,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   243,
     243,   243,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,   500,  -699,   505,   509,   482,  -699,   523,    66,    66,
      52,    66,    66,  -699,   243,    66,  -699,    66,  -699,    66,
      66,    66,  -699,   243,  -699,    52,  -699,  -699,  -699,  2981,
    -699,  -699,  -699,   524,   251,    66,   528,    66,    66,    66,
     165,    66,    66,   165,  -699,  -699,  -699,   165,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   538,   541,
    -699,   544,  -699,  -699,  2981,    52,   133,   243,   243,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   243,  -699,
     243,   243,    35,   243,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,   587,   587,   364,   243,   243,
     243,   213,    55,  1151,    66,   253,   243,   243,   243,    52,
    -699,  -699,  -699,   259,   270,    66,    66,  -699,   271,   517,
    -699,   349,  -699,  -699,  -699,   365,  1186,   703,    66,    66,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,   101,  -699,   709,   158,   954,
    -699,  -699,  -699,  -699,  -699,  -699,    88,   102,  -699,    66,
      66,    66,    66,   204,  -699,  -699,  -699,  -699,    66,  -699,
      66,  -699,   283,    66,    66,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  2237,  2237,  -699,  -699,  -699,
    -699,  -699,   165,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,   243,   243,   243,  -699,   165,
     302,   412,  -699,  -699,  -699,   165,    12,  2237,  2237,  2237,
    -699,   243,  2237,   243,  2888,   365,  -699,    80,    86,   243,
     165,   165,   243,   243,    66,    66,  -699,   243,   243,   243,
    -699,  2981,  -699,  -699,   340,  -699,  -699,    66,    66,  -699,
    -699,   165,  -699,   548,  -699,   551,  -699,   557,  -699,   243,
     558,  -699,   349,   567,  -699,  -699,   165,   165,  -699,  -699,
    -699,   302,  -699,  2609,  -699,    66,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,   342,  -699,  -699,  -699,    60,   243,
     243,   243,   243,   243,   243,   243,   243,   243,   443,  -699,
     182,   286,   373,   435,   526,   294,  -699,  2981,  -699,  -699,
    -699,  -699,    66,  -699,  -699,  -699,  -699,  -699,  -699,    66,
    -699,  -699,  -699,  -699,  -699,  -699,   443,   443,   129,   199,
     346,   374,  -699,  -699,  -699,  -699,   165,  -699,  -699,  -699,
     165,  -699,  -699,   443,  -699,    66,  -699,    66,   368,   443,
    -699,   443,   353,   381,   443,   443,   377,   460,  -699,   443,
    -699,  -699,   411,  2702,   443,  -699,  -699,  -699,  2795,  -699,
     419,   443,  2981,  -699
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     651,     0,     2,     3,     0,     1,   649,   650,     0,     0,
     652,   651,     0,     0,   653,   667,     4,   666,   668,   669,
     670,   671,   672,   673,   674,   675,   676,   677,   678,   679,
     680,   681,   682,   683,   684,   685,   686,   687,   688,   689,
     690,   691,   692,   693,   694,   695,   696,   697,   698,   699,
     700,   701,   702,   703,   704,   705,   706,   707,   708,   709,
     710,   711,   712,   713,   714,   715,   716,   717,   718,   719,
     720,   721,   722,   723,   724,   725,   726,   727,   728,   729,
     730,   731,   732,   733,   734,   735,   736,   737,   738,   739,
     740,   741,   742,   743,   744,   622,     9,   745,   651,    16,
     651,     0,   621,     0,     6,   630,   630,     5,    12,    19,
     651,   633,    10,   632,   631,    17,     0,   636,     0,     0,
       0,    25,    13,    14,    15,    25,     0,    20,     7,     0,
     638,   637,     0,     0,     0,    49,    49,     0,    22,   651,
     651,   643,   634,   651,   659,   662,   660,   664,   665,   663,
     635,   639,   661,   658,     0,   656,   619,     0,   651,     0,
       0,     0,    26,    27,    58,    58,     0,     8,   645,   641,
     630,   630,    24,   651,    48,   620,    23,     0,     0,     0,
       0,     0,     0,    50,    51,    52,    53,    71,    71,    45,
     624,   636,     0,   623,   640,     0,   628,   657,    28,    35,
      36,     0,     0,     0,     0,     0,   630,     0,     0,     0,
       0,   644,   651,   630,     0,     0,     0,     0,     0,     0,
       0,    59,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   651,   630,
      73,    74,    75,    76,    77,    78,   232,   233,   234,   235,
     236,   237,   238,    79,    80,    81,    82,   651,   630,   651,
     651,   646,     0,   629,   630,   630,    38,   630,    55,    56,
      54,    57,    68,    70,    60,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    72,    18,     0,   645,   647,   651,    30,    40,
      37,   651,     0,   364,     0,   432,   415,   581,   651,   327,
     239,    83,   488,   480,   651,   108,   220,   118,   262,   281,
     301,   465,   435,     0,   131,   746,   627,   379,   651,   651,
      46,     0,   642,     0,     0,    69,   630,    62,    61,     0,
     579,     0,   580,   431,     0,     0,     0,     0,   487,     0,
       0,     0,     0,     0,     0,     0,     0,   630,     0,   625,
     626,     0,   651,     0,   630,    34,    31,    32,    33,    39,
      43,    41,    42,    63,   630,   366,   365,   630,   582,   630,
     329,   328,   630,   241,   240,   630,    85,    84,   630,   630,
     110,   109,   630,   222,   221,   630,   120,   119,   630,   630,
     630,   630,   467,   466,   630,   437,   436,   134,   630,   381,
     380,   630,   648,     0,    29,    65,   368,   418,   332,   243,
      87,   483,   112,   224,   122,   265,   284,   304,   469,   439,
     129,   383,    47,     0,     0,     0,   416,   330,     0,     0,
     481,     0,     0,     0,   263,   282,   302,     0,     0,     0,
       0,     0,   140,   141,   139,     0,     0,   137,   138,     0,
      44,    64,    66,    67,     0,     0,     0,     0,     0,     0,
     367,   377,   378,   376,   371,   369,   374,   375,   372,     0,
       0,   423,   424,   422,   421,   425,   429,     0,   427,   419,
     340,   341,   339,   335,   336,   346,   347,   348,   349,     0,
     342,   344,   345,   350,   333,   337,   338,     0,   242,   252,
     253,   251,   246,   258,   254,   260,   256,   244,   250,   249,
     247,     0,    86,    90,    91,    88,    89,     0,   484,   485,
       0,   111,   115,   116,   114,   113,   223,   226,   227,   225,
     630,   630,   630,   630,     0,   121,   126,   127,   125,   124,
     123,   279,   280,   278,   268,   272,   275,     0,     0,   266,
     276,   277,   273,     0,     0,     0,   299,   300,   298,   287,
     291,     0,     0,   285,   294,   295,   296,   297,   292,     0,
       0,   317,   318,   316,   307,   319,   321,   325,     0,   323,
     305,   312,   313,   314,   315,   308,   311,   310,   468,   472,
     473,   471,   470,   474,   476,   478,   651,   651,   438,   442,
     443,   441,   440,   444,   446,   448,   450,     0,     0,     0,
     130,     0,   630,     0,     0,   382,   388,   389,   387,   386,
     390,   392,   384,   537,     0,   544,     0,   101,     0,   630,
     630,     0,     0,   630,   630,   417,   630,   630,   331,   630,
     630,     0,   630,   630,   630,   630,   630,   630,     0,     0,
     482,   229,   228,   230,   231,     0,   264,   270,   630,   630,
       0,     0,     0,   283,   289,   630,   630,     0,     0,   630,
     630,   630,   303,   630,   630,   630,   630,   630,   630,   452,
     462,   630,   630,   630,   630,     0,   651,   651,   651,   107,
       0,     0,     0,   136,     0,     0,   630,   630,   630,     0,
       0,     0,   562,     0,   529,   370,   373,   351,   433,   426,
     430,   428,   420,   343,   334,     0,   259,   255,   261,   257,
     245,   248,    92,   651,   651,   651,   651,   486,   651,   489,
     491,   493,   492,     0,   630,   267,   274,   612,   651,   557,
       0,   651,   613,   553,     0,   614,   651,   651,   651,   561,
       0,   630,   286,   293,     0,   571,   572,     0,   566,     0,
     583,   320,   322,   326,   324,   306,   309,   475,   477,   479,
       0,     0,   445,   447,   449,   451,   219,   104,   106,   105,
     100,   218,   132,   414,   410,   651,   399,   394,   651,   391,
     393,   385,   651,   651,   542,   538,   117,   651,   651,   549,
     545,     0,   102,     0,     0,     0,   550,     0,   495,   508,
       0,   517,   490,   128,   271,   555,   554,   556,   551,   552,
     560,   559,   558,   290,   574,     0,   568,   567,   570,     0,
     581,   630,   630,     0,     0,   413,     0,   398,   541,   540,
       0,   548,   547,     0,   630,   564,   563,     0,   630,   531,
     530,   630,   353,   352,   434,   630,    94,   630,     0,     0,
     494,     0,   573,   577,     0,     0,   584,   455,   455,   630,
     142,   133,   630,   630,   396,   395,   539,   546,   165,   103,
     533,   355,     0,    93,   630,   497,   496,   630,   510,   509,
     630,   519,   518,   575,   569,   453,   463,   151,   144,   418,
     400,     0,     0,     0,     0,     0,   499,   512,   521,     0,
     456,   458,   460,     0,     0,     0,     0,   148,     0,   145,
     146,     0,   147,   149,   150,     0,   411,     0,     0,     0,
     565,   168,   169,   166,   167,   532,   534,   535,   354,   360,
     361,   359,   358,   362,   356,     0,    95,     0,     0,     0,
     576,   630,   630,   630,   454,   464,   585,     0,   143,     0,
       0,     0,     0,     0,   154,   152,   153,   630,     0,   630,
       0,   201,     0,     0,     0,   397,   408,   409,   403,   404,
     405,   402,   406,   407,   630,     0,     0,   630,   630,   651,
     651,    99,     0,   498,   500,   503,   504,   505,   506,   507,
     630,   502,   511,   513,   516,   630,   515,   520,   630,   523,
     524,   525,   526,   527,   528,   457,   459,   461,   589,     0,
       0,     0,   651,   651,   198,     0,     0,     0,     0,     0,
     630,   155,     0,   176,     0,   200,   412,     0,     0,   401,
       0,     0,   363,   357,    98,    97,    96,   501,   514,   522,
     194,     0,   592,   586,     0,   588,   596,   197,   196,   195,
     160,     0,   651,     0,   162,     0,   171,     0,   578,   156,
       0,   179,   175,     0,   204,   202,     0,     0,   217,   216,
     601,   591,   595,     0,   158,   159,   630,   163,   630,   630,
     172,   630,   630,   192,   191,   630,   180,   178,   630,   630,
     205,   630,   536,   543,   590,   593,   597,   594,   599,   165,
     161,   165,   170,   165,   182,   177,   207,   203,   654,   598,
       0,     0,     0,     0,     0,     0,   655,     0,   600,   164,
     173,   193,     0,   181,   186,   187,   185,   183,   184,     0,
     206,   211,   212,   210,   208,   209,   654,   654,     0,     0,
       0,     0,   617,   618,   615,   190,     0,   651,   616,   215,
       0,   651,   602,   654,   188,   189,   213,   214,     0,   654,
     603,   654,     0,     0,   654,   654,     0,     0,   611,   654,
     604,   607,     0,     0,   654,   608,   609,   606,   654,   605,
       0,   654,     0,   610
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -699,  -699,   570,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,   398,  -699,   396,  -699,  -699,
    -699,   352,  -699,  -699,  -699,  -699,   247,  -699,  -699,  -699,
    -258,   402,  -699,  -699,   -25,    50,   393,  -699,  -699,  -699,
    -699,  -699,   136,  -699,   376,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -427,  -318,  -699,  -699,  -148,
    -699,  -699,  -699,  -699,  -394,  -699,  -699,  -699,  -699,  -330,
    -343,  -699,  -699,  -535,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -613,  -699,  -699,
    -699,  -699,  -699,  -699,  -498,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -456,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -430,  -426,  -304,  -699,
    -699,  -699,  -362,   153,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,   160,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,   167,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   171,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,   177,  -699,
    -699,  -699,  -699,  -699,   196,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -294,
    -699,  -699,  -699,  -699,  -699,  -411,  -699,  -699,  -699,  -183,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -228,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,  -699,
    -699,  -699,  -377,  -699,  -699,  -699,  -646,    -9,  -699,  -699,
    -388,  -289,  -240,  -699,  -699,  -379,  -277,  -422,  -699,  -390,
    -699,   229,  -699,  -412,  -699,  -699,  -698,  -699,  -199,  -699,
    -699,  -699,  -242,  -699,  -699,  -334,   403,  -162,  -688,  -699,
    -699,  -699,  -699,  -409,  -450,  -699,  -699,  -404,  -699,  -699,
    -699,  -434,  -699,  -699,  -699,  -502,  -699,  -699,  -699,  -664,
    -467,  -699,  -699,  -699,   -11,  -123,  -640,   593,  1131,  -699,
    -699,   503,  -699,  -699,  -699,  -699,   401,  -699,    -4,   142,
     492,  -265,  -112,  -699,   565,   384,  -124,  -699
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    95,   100,   107,   139,     2,   101,   121,   112,
       3,   103,   125,   115,   126,   122,   123,   135,   162,   214,
     333,   198,   163,   215,   267,   334,   365,   138,   209,   361,
     124,   164,   183,   184,   941,   942,   187,   206,   302,   338,
     415,   434,   274,   301,   207,   239,   240,   346,   387,   439,
     525,   817,   867,   915,  1002,   483,   473,   713,   857,   700,
     241,   349,   391,   441,   484,   242,   351,   397,   443,   550,
     243,   455,   323,   622,   843,   430,   456,   881,   907,   928,
     929,   973,   974,  1071,   975,  1073,  1098,   911,   976,  1075,
    1101,   930,   931,  1082,   979,  1080,  1107,  1134,  1148,  1166,
     977,  1104,   932,   933,  1035,   934,   935,  1045,   981,  1083,
    1111,  1135,  1155,  1170,   943,   944,   457,   458,   244,   350,
     394,   442,   245,   246,   345,   384,   438,   656,   657,   653,
     655,   652,   654,   247,   352,   557,   444,   668,   558,   744,
     669,   248,   353,   571,   445,   675,   572,   761,   676,   249,
     354,   588,   446,   684,   685,   679,   680,   683,   681,   250,
     344,   381,   499,   437,   650,   649,   500,   501,   486,   814,
     863,   913,   998,   997,   251,   339,   376,   435,   639,   640,
     252,   358,   410,   459,   708,   706,   707,   630,   846,   885,
     797,   937,   631,   844,   982,   794,   253,   341,   487,   436,
     647,   643,   646,   644,   306,   488,   815,   254,   356,   406,
     448,   691,   692,   693,   694,   615,   780,   923,   905,   961,
     962,   963,   616,   781,   924,   255,   355,   403,   447,   686,
     687,   688,   256,   347,   530,   440,   313,   737,   738,   739,
     740,   868,   896,   957,   741,   869,   899,   958,   742,   871,
     902,   959,   489,   813,   860,   912,   989,   476,   709,   850,
     805,   990,   477,   711,   853,   810,   519,   575,   754,   576,
     750,   577,   760,   478,   811,   856,   596,   768,   837,   597,
     765,   835,   872,   919,  1077,   307,   308,   342,   769,   840,
    1029,  1030,  1031,  1062,  1063,  1091,  1065,  1066,  1093,  1117,
    1129,  1114,  1156,  1180,  1190,  1191,  1193,  1198,  1181,   755,
     756,  1167,  1168,   157,   199,   770,   327,   838,   108,   113,
     117,   129,   130,   150,   194,   142,   192,   261,   114,     4,
     131,  1137,   154,   173,   155,    97,    98,   329
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    96,    99,   454,    11,    11,   751,     5,   378,   158,
      10,   494,   526,    16,   555,   570,   795,   798,   556,   180,
      16,    16,   159,   667,   592,   743,   520,   516,   105,   747,
     747,   543,   562,   578,   595,   589,   330,   674,   -21,   106,
       6,   474,     7,   493,   512,   191,   111,   535,    16,   549,
     554,   569,   584,   602,   612,   182,   593,     6,   475,     7,
     504,   517,   792,   110,     6,   629,     7,   559,   573,   590,
     111,     6,   180,     7,   485,   366,   515,     6,   527,     7,
     542,   137,   632,   111,   587,   605,   180,   461,    16,   216,
     217,   218,   219,   205,    16,   513,    16,   120,    10,   540,
      10,   211,  -587,   585,   603,   613,   304,   748,   182,    16,
      16,   212,    10,    11,    11,    11,    11,   258,   914,   492,
     511,    16,   182,   534,   539,   548,   553,   568,   583,   601,
     611,   264,   757,    11,   514,   758,   119,    16,   541,   185,
     185,   628,   586,   604,   614,  1162,  1163,  1164,   160,   161,
     170,   875,   171,    14,   945,   189,    11,    11,   213,   521,
     544,  -601,  -601,    10,    10,    10,   328,   357,   802,   180,
     180,    10,   191,   803,   807,    11,    11,    11,    11,   808,
     465,   340,    16,   140,   118,   141,   272,    11,   449,   999,
    1032,   362,   119,    10,  1000,  1033,   377,   506,   696,   411,
     697,    11,   388,   698,   561,   182,   182,    16,   449,   180,
     120,   468,   450,   467,   186,   186,   747,   752,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,   304,   580,   451,   545,   522,   398,
     102,     6,   104,     7,   180,   182,   938,   939,     6,   399,
       7,   450,   116,  -135,   111,   400,   451,  1012,    10,  1011,
    1016,   180,   180,   170,   303,   171,   309,   310,   311,   620,
     315,   316,   317,   318,   319,   320,   321,   322,   324,   621,
     182,   167,   168,  1061,  1128,   169,   623,    10,   645,    10,
       8,    10,    10,   259,   119,   260,   180,   182,   182,   222,
     175,   363,   363,   841,   265,   180,   266,   465,   367,   371,
       9,  1006,   940,  1021,   648,   197,   465,   180,   660,   938,
     939,   224,   464,   666,   225,   180,   180,   673,    10,   364,
     369,    10,   182,   624,   682,   229,   465,   465,   468,    10,
     842,   182,   231,   232,    10,   233,   469,   468,   179,   882,
     467,   180,   956,   182,   262,    10,    10,  1149,   964,    11,
     507,   182,   182,     6,   625,     7,   128,   450,   468,   965,
     968,   236,  1072,   531,   237,     6,   469,     7,   297,   181,
     291,   978,  1046,   368,   372,  1139,   544,   182,  -199,   980,
     733,   480,   734,  1150,   508,   735,  -174,   736,   336,   293,
     337,   295,   296,  1061,   180,   452,   938,   939,    10,   462,
     471,   481,   490,   509,   523,   528,   532,   537,   546,   551,
     566,   581,   599,   609,  1018,   925,  1064,   464,   505,   518,
     374,   180,   375,   926,   626,   560,   574,   591,  1040,   332,
     182,  1092,   465,   335,  1128,    11,    11,    11,  1136,  1172,
     343,   466,   379,  1179,   380,   467,   348,   382,  1184,   383,
      11,    11,    11,    11,    11,    11,   180,   182,   938,   939,
     359,   360,  1140,   468,  1189,    11,    11,   385,  1173,   386,
     453,   469,  1185,  1188,   463,   472,   482,   491,   510,   524,
     529,   533,   538,   547,   552,   567,   582,   600,   610,   470,
      12,    13,   182,    11,   412,   695,  1131,   701,  1132,   627,
    1133,   988,  1194,   127,   152,   992,   152,    11,   152,   952,
    1201,   136,   710,    11,   712,   994,   714,  1004,  1013,  1019,
     200,  1005,  1014,  1020,  1141,  1008,   954,  1023,   165,   725,
      11,   328,   378,   922,   922,  1010,  1015,   993,   389,   433,
     390,   953,   193,   392,   766,   393,   969,   180,   188,    11,
      11,    11,   920,   920,   208,   812,   970,  1009,   465,  1024,
     395,   401,   396,   402,   485,    11,    11,   927,   328,   971,
     404,   370,   405,   972,  1108,   152,   152,   152,   152,  1085,
     495,   222,  -157,   182,   408,   951,   409,   496,   854,   450,
     855,   921,   921,   858,   497,   859,  1142,   861,   498,   862,
     132,   133,   134,   224,   502,   936,   225,    11,   222,    11,
      11,   865,   879,   866,   880,  1143,   883,   229,   884,   166,
     717,   718,   864,   503,   231,   232,   894,   233,   895,   897,
     224,   898,   900,   225,   901,   180,  1096,   732,  1097,  1099,
     906,  1100,   177,   178,   229,  1102,  1105,  1103,  1106,  1086,
     991,   231,   232,   236,   233,  1109,   237,  1110,  1007,  1087,
    1022,   201,   202,   203,   204,   594,   904,   960,   876,   193,
    1138,   182,  1115,   314,  1130,    10,    10,   468,   193,  1116,
     236,  1195,  1171,   237,   210,   151,   331,   220,     0,     0,
       0,     0,   153,   156,   153,     0,     0,  1146,  1153,     0,
     480,     0,     0,   536,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     983,     0,     0,   449,   180,     0,   983,     0,     0,   449,
    1147,  1154,     0,     0,     0,     0,     0,   172,   689,   690,
     174,   903,   176,     0,   984,   563,   564,     0,   467,     0,
     984,   563,   564,     0,   467,    11,     0,     0,   507,     0,
     182,   153,   153,   153,   153,     0,     0,     0,     0,     0,
       0,     0,     0,    10,    10,    10,   580,   451,     0,     0,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   985,     0,     0,     0,     0,     0,  1003,   268,
     269,   270,   271,     0,    10,    10,    10,    10,    10,     0,
       0,    10,     0,    10,     0,    10,    10,    10,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,   787,   788,
     789,    10,     0,    10,    10,    10,   305,    10,    10,     0,
     312,     0,     0,     0,     0,   413,     0,     0,     0,     0,
     326,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,     0,     0,   818,   819,   820,   821,     0,
     822,     0,     0,  1050,  1051,     0,     0,   946,   949,     0,
     825,  1160,  1161,   827,     0,     0,     0,     0,   829,   830,
     831,     0,     0,     0,     0,     0,     0,     0,  1178,     0,
      11,   481,   986,     0,  1182,    11,  1183,     0,     0,  1186,
    1187,    11,    11,     0,  1192,  1074,  1076,  1078,     0,  1197,
    1081,     0,     0,  1200,    11,    11,  1202,   845,  1090,     0,
     847,   617,   618,   619,   848,   849,     0,     0,     0,   851,
     852,     0,     0,     0,     0,     0,   633,   634,   635,   636,
     637,   638,   947,   950,     0,    11,    11,    11,    11,     0,
    1118,   641,   642,     0,    11,     0,    11,     0,     0,    11,
      11,   983,     0,   273,   449,     0,   482,   987,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   651,
       0,   152,     0,   152,     0,   984,   563,   564,     0,     0,
       0,     0,     0,   658,  1157,     0,     0,     0,   152,   659,
     152,     0,   152,     0,     0,     0,   460,     0,     0,   621,
       0,     0,   451,  1084,     0,   152,   665,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      10,    10,     0,  1017,     0,   670,   671,   672,     0,     0,
       0,   193,   193,    10,    10,     0,     0,     0,   222,     0,
    1196,   677,   678,     0,     0,  1199,     0,     0,     0,  1203,
       0,   222,     0,     0,     0,   223,     0,     0,   193,   193,
     224,    10,     0,   225,     0,     0,     0,     0,     0,   226,
     227,     0,   228,   224,   229,   230,   225,     0,   180,  1144,
    1151,   231,   232,   702,   233,   704,   705,   229,     0,   465,
       0,   234,     0,   180,   231,   232,     0,   233,     0,     0,
       0,     0,   229,     0,   465,     0,   235,   606,    11,     0,
     236,  1054,  1055,   237,   182,    11,     0,     0,     0,     0,
     468,     0,   607,   236,     0,   222,   237,     0,     0,   182,
       0,     0,     0,   238,     0,   468,     0,     0,   236,     0,
       0,    10,     0,    10,  1067,  1068,   598,   224,     0,     0,
     225,     0,   180,     0,  1145,  1152,     0,   153,   699,   153,
     222,   608,     0,   465,     0,     0,     0,     0,   231,   232,
       0,   233,     0,     0,   153,     0,   153,     0,   153,     0,
       0,   479,   224,     0,  1095,   225,     0,   180,   182,     0,
       0,   153,     0,   193,   468,     0,     0,     0,   465,     0,
     237,     0,   469,   231,   232,   326,   233,   109,     0,     0,
     749,   753,   759,     0,     0,     0,     0,   764,   767,     0,
     948,     0,     0,   182,     0,     0,     0,     0,   193,   450,
       0,   839,     0,     0,     0,   237,     0,   469,     0,     0,
       0,     0,   326,     0,   793,   796,     0,     0,     0,   804,
       0,   809,   480,   699,     0,   222,     0,     0,   786,     0,
       0,     0,     0,   790,   791,     0,     0,     0,     0,     0,
       0,   195,   196,   806,     0,     0,   479,     0,   464,  1175,
     225,   449,   180,  1177,     0,     0,     0,     0,   816,     0,
       0,     0,     0,   465,     0,     0,     0,   874,   231,   232,
       0,   233,   466,     0,     0,     0,   823,   221,     0,     0,
       0,     0,     0,   826,   263,     0,   464,   828,   182,     0,
     180,     0,     0,   832,   468,     0,     0,   834,     0,     0,
     836,   465,   469,     0,     0,     0,     0,   839,     0,     0,
     292,   563,   564,     0,   467,     0,     0,   565,     0,   152,
     152,     0,     0,     0,     0,     0,   182,     0,     0,   294,
       0,     0,   468,     0,  -288,   298,   299,   451,   300,     0,
     469,   464,     0,     0,   449,   180,   955,     0,     0,     0,
       0,   874,     0,   870,     0,     0,   465,   966,   967,     0,
       0,   152,   152,   152,     0,   466,   152,     0,   873,   467,
     995,   996,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   182,     0,   886,     0,   193,   887,   468,     0,  -269,
     889,     0,   451,     0,     0,   469,     0,     0,     0,     0,
       0,  1036,  1037,  1038,  1039,     0,     0,   373,     0,     0,
    1042,     0,  1044,     0,     0,  1047,  1048,   193,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   407,     0,
       0,     0,     0,     0,     0,   414,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   416,     0,     0,   417,     0,
     418,     0,   873,   419,     0,     0,   420,     0,     0,   421,
     422,   193,     0,   423,     0,  1001,   424,     0,     0,   425,
     426,   427,   428,     0,     0,   429,  1028,  1034,     0,   431,
       0,     0,   432,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   153,   153,   222,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   193,     0,     0,
       0,     0,   193,     0,   223,     0,   193,     0,     0,   224,
       0,     0,   225,     0,     0,  1056,     0,     0,   226,   227,
       0,   228,     0,   229,   230,     0,  1070,   153,   153,   153,
     231,   232,   153,   233,     0,     0,     0,   804,   809,     0,
     234,     0,  1060,     0,     0,     0,     0,     0,  1069,     0,
       0,     0,     0,     0,  1158,   235,     0,     0,     0,   236,
       0,  1159,   237,  1088,  1089,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,   222,  1094,     0,     0,     0,     0,     0,
       0,   661,   662,   663,   664,     0,     0,     0,     0,  1112,
    1113,     0,     0,     0,     0,   224,   464,     0,   225,     0,
     180,     0,     0,     0,     0,     0,     0,     0,     0,   229,
       0,   465,     0,     0,     0,   579,   231,   232,     0,   233,
       0,   563,   564,     0,   467,     0,     0,   565,     0,     0,
       0,     0,     0,     0,     0,     0,   182,     0,  1165,  1169,
       0,     0,   468,     0,     0,   236,   580,     0,   237,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   703,     0,   480,     0,     0,     0,  1174,
       0,     0,     0,  1176,     0,     0,     0,     0,     0,     0,
     715,   716,     0,     0,   719,   720,     0,   721,   722,     0,
     723,   724,     0,   726,   727,   728,   729,   730,   731,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   745,
     746,     0,     0,     0,     0,     0,   762,   763,     0,     0,
     771,   772,   773,     0,   774,   775,   776,   777,   778,   779,
       0,     0,   782,   783,   784,   785,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   799,   800,   801,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   824,     0,    15,     0,     0,
       0,   143,   144,    17,   145,   146,     0,     0,     0,   147,
     148,   149,   833,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,     0,   877,   878,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   888,     0,     0,     0,   890,
       0,     0,   891,     0,     0,     0,   892,     0,   893,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     908,     0,     0,   909,   910,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   916,     0,     0,   917,     0,
       0,   918,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,  1026,  1027,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1041,     0,
    1043,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1049,     0,     0,  1052,  1053,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1057,     0,     0,     0,     0,  1058,     0,     0,  1059,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1079,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1119,     0,  1120,
    1121,     0,  1122,  1123,     0,     0,  1124,     0,     0,  1125,
    1126,    15,  1127,     0,     0,    16,   144,    17,   145,   146,
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
     190,     0,     0,  1064,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    15,     0,     0,     0,
       0,     0,    17,   190,     0,     0,  1189,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    15,
    1136,     0,     0,     0,     0,    17,   190,     0,     0,     0,
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
       4,    12,    13,   430,     8,     9,   670,     0,   342,   133,
      14,   437,   439,     8,   444,   445,   704,   705,   444,    31,
       8,     8,   134,   558,   446,   665,   438,   438,    98,    17,
      17,   442,   444,   445,   446,   446,   294,   572,    23,    98,
       5,   435,     7,   437,   438,   168,    11,   441,     8,   443,
     444,   445,   446,   447,   448,    67,   446,     5,   435,     7,
     437,   438,   702,    97,     5,   459,     7,   444,   445,   446,
      11,     5,    31,     7,   436,   333,   438,     5,    37,     7,
     442,    23,   459,    11,   446,   447,    31,    99,     8,   201,
     202,   203,   204,    70,     8,   438,     8,    82,   102,   442,
     104,    99,    14,   446,   447,   448,   101,    94,    67,     8,
       8,    97,   116,   117,   118,   119,   120,    98,    83,   437,
     438,     8,    67,   441,   442,   443,   444,   445,   446,   447,
     448,    98,    92,   137,   438,    95,    64,     8,   442,   164,
     165,   459,   446,   447,   448,    16,    17,    18,    43,    44,
      98,   839,   100,    11,    99,   166,   160,   161,    99,    20,
      22,   101,   102,   167,   168,   169,   290,    98,    88,    31,
      31,   175,   295,    93,    88,   179,   180,   181,   182,    93,
      42,   304,     8,    98,    56,   100,    12,   191,    30,    88,
      88,    99,    64,   197,    93,    93,    98,   437,    85,    99,
      87,   205,    98,    90,   444,    67,    67,     8,    30,    31,
      82,    73,    73,    55,   164,   165,    17,    18,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   101,    77,    78,    99,    99,    98,
      98,     5,   100,     7,    31,    67,    33,    34,     5,    98,
       7,    73,   110,    75,    11,    98,    78,    99,   262,   957,
     958,    31,    31,    98,   275,   100,   277,   278,   279,    99,
     281,   282,   283,   284,   285,   286,   287,   288,   289,    75,
      67,   139,   140,   101,   102,   143,    21,   291,    99,   293,
      54,   295,   296,    98,    64,   100,    31,    67,    67,     4,
     158,    71,    71,    98,    98,    31,   100,    42,   333,   334,
      74,   957,    99,   959,    99,   173,    42,    31,    99,    33,
      34,    26,    27,    99,    29,    31,    31,    99,   332,    99,
      99,   335,    67,    68,    99,    40,    42,    42,    73,   343,
      98,    67,    47,    48,   348,    50,    81,    73,    28,    98,
      55,    31,    99,    67,   212,   359,   360,    63,    99,   363,
      65,    67,    67,     5,    99,     7,     8,    73,    73,    99,
      99,    76,  1036,    99,    79,     5,    81,     7,     8,    59,
     238,    32,    99,   333,   334,    99,    22,    67,    24,    24,
      84,    96,    86,    99,    99,    89,    32,    91,    98,   257,
     100,   259,   260,   101,    31,   430,    33,    34,   412,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   959,    61,    14,    27,   437,   438,
      98,    31,   100,    69,   459,   444,   445,   446,   973,   297,
      67,   101,    42,   301,   102,   449,   450,   451,     5,   103,
     308,    51,    98,    85,   100,    55,   314,    98,   105,   100,
     464,   465,   466,   467,   468,   469,    31,    67,    33,    34,
     328,   329,    99,    73,    14,   479,   480,    98,   104,   100,
     430,    81,   101,   106,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,    99,
       8,     9,    67,   507,   362,   617,  1119,   619,  1121,   459,
    1123,   937,   101,   115,   130,   937,   132,   521,   134,   913,
     101,   125,   634,   527,   636,   937,   638,   957,   958,   959,
     178,   957,   958,   959,    99,   957,   913,   959,   136,   651,
     544,   665,   876,   905,   906,   957,   958,   937,    98,   413,
     100,   913,   168,    98,   677,   100,    39,    31,   165,   563,
     564,   565,   905,   906,   188,   713,    49,   957,    42,   959,
      98,    98,   100,   100,   936,   579,   580,   907,   702,    62,
      98,   334,   100,    66,  1082,   201,   202,   203,   204,  1045,
     437,     4,    75,    67,    98,   913,   100,   437,    98,    73,
     100,   905,   906,    98,   437,   100,    80,    98,   437,   100,
     118,   119,   120,    26,   437,   909,    29,   621,     4,   623,
     624,    98,    98,   100,   100,    99,    98,    40,   100,   137,
     641,   642,   815,   437,    47,    48,    98,    50,   100,    98,
      26,   100,    98,    29,   100,    31,    98,   658,   100,    98,
     878,   100,   160,   161,    40,    98,    98,   100,   100,  1047,
     937,    47,    48,    76,    50,    98,    79,   100,   957,  1048,
     959,   179,   180,   181,   182,   446,   875,   919,   840,   295,
    1130,    67,  1091,   280,  1118,   689,   690,    73,   304,  1093,
      76,  1193,  1159,    79,   191,   130,   295,   205,    -1,    -1,
      -1,    -1,   132,   133,   134,    -1,    -1,  1134,  1135,    -1,
      96,    -1,    -1,    99,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
      27,    -1,    -1,    30,    31,    -1,    27,    -1,    -1,    30,
    1134,  1135,    -1,    -1,    -1,    -1,    -1,   154,   606,   607,
     157,   874,   159,    -1,    51,    52,    53,    -1,    55,    -1,
      51,    52,    53,    -1,    55,   769,    -1,    -1,    65,    -1,
      67,   201,   202,   203,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   787,   788,   789,    77,    78,    -1,    -1,
     220,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    99,   216,
     217,   218,   219,    -1,   818,   819,   820,   821,   822,    -1,
      -1,   825,    -1,   827,    -1,   829,   830,   831,    -1,    -1,
      -1,   835,    -1,    -1,    -1,    -1,    -1,    -1,   696,   697,
     698,   845,    -1,   847,   848,   849,   276,   851,   852,    -1,
     280,    -1,    -1,    -1,    -1,   363,    -1,    -1,    -1,    -1,
     290,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   875,    -1,    -1,    -1,   733,   734,   735,   736,    -1,
     738,    -1,    -1,   995,   996,    -1,    -1,   912,   913,    -1,
     748,  1156,  1157,   751,    -1,    -1,    -1,    -1,   756,   757,
     758,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1173,    -1,
     914,   936,   937,    -1,  1179,   919,  1181,    -1,    -1,  1184,
    1185,   925,   926,    -1,  1189,  1037,  1038,  1039,    -1,  1194,
    1042,    -1,    -1,  1198,   938,   939,  1201,   795,  1061,    -1,
     798,   449,   450,   451,   802,   803,    -1,    -1,    -1,   807,
     808,    -1,    -1,    -1,    -1,    -1,   464,   465,   466,   467,
     468,   469,   912,   913,    -1,   969,   970,   971,   972,    -1,
    1093,   479,   480,    -1,   978,    -1,   980,    -1,    -1,   983,
     984,    27,    -1,   413,    30,    -1,   936,   937,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   507,
      -1,   617,    -1,   619,    -1,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,   521,  1137,    -1,    -1,    -1,   634,   527,
     636,    -1,   638,    -1,    -1,    -1,   433,    -1,    -1,    75,
      -1,    -1,    78,  1044,    -1,   651,   544,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1054,  1055,    -1,    99,    -1,   563,   564,   565,    -1,    -1,
      -1,   677,   678,  1067,  1068,    -1,    -1,    -1,     4,    -1,
    1193,   579,   580,    -1,    -1,  1198,    -1,    -1,    -1,  1202,
      -1,     4,    -1,    -1,    -1,    21,    -1,    -1,   704,   705,
      26,  1095,    -1,    29,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    38,    26,    40,    41,    29,    -1,    31,  1134,
    1135,    47,    48,   621,    50,   623,   624,    40,    -1,    42,
      -1,    57,    -1,    31,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    40,    -1,    42,    -1,    72,    45,  1142,    -1,
      76,   999,  1000,    79,    67,  1149,    -1,    -1,    -1,    -1,
      73,    -1,    60,    76,    -1,     4,    79,    -1,    -1,    67,
      -1,    -1,    -1,    99,    -1,    73,    -1,    -1,    76,    -1,
      -1,  1175,    -1,  1177,  1032,  1033,    99,    26,    -1,    -1,
      29,    -1,    31,    -1,  1134,  1135,    -1,   617,   618,   619,
       4,    99,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,
      -1,    50,    -1,    -1,   634,    -1,   636,    -1,   638,    -1,
      -1,    25,    26,    -1,  1072,    29,    -1,    31,    67,    -1,
      -1,   651,    -1,   839,    73,    -1,    -1,    -1,    42,    -1,
      79,    -1,    81,    47,    48,   665,    50,   106,    -1,    -1,
     670,   671,   672,    -1,    -1,    -1,    -1,   677,   678,    -1,
      99,    -1,    -1,    67,    -1,    -1,    -1,    -1,   874,    73,
      -1,   769,    -1,    -1,    -1,    79,    -1,    81,    -1,    -1,
      -1,    -1,   702,    -1,   704,   705,    -1,    -1,    -1,   709,
      -1,   711,    96,   713,    -1,     4,    -1,    -1,   695,    -1,
      -1,    -1,    -1,   700,   701,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,   710,    -1,    -1,    25,    -1,    27,  1167,
      29,    30,    31,  1171,    -1,    -1,    -1,    -1,   725,    -1,
      -1,    -1,    -1,    42,    -1,    -1,    -1,   835,    47,    48,
      -1,    50,    51,    -1,    -1,    -1,   743,   206,    -1,    -1,
      -1,    -1,    -1,   750,   213,    -1,    27,   754,    67,    -1,
      31,    -1,    -1,   760,    73,    -1,    -1,   764,    -1,    -1,
     767,    42,    81,    -1,    -1,    -1,    -1,   875,    -1,    -1,
     239,    52,    53,    -1,    55,    -1,    -1,    58,    -1,   995,
     996,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,   258,
      -1,    -1,    73,    -1,    75,   264,   265,    78,   267,    -1,
      81,    27,    -1,    -1,    30,    31,   914,    -1,    -1,    -1,
      -1,   919,    -1,   820,    -1,    -1,    42,   925,   926,    -1,
      -1,  1037,  1038,  1039,    -1,    51,  1042,    -1,   835,    55,
     938,   939,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,   850,    -1,  1061,   853,    73,    -1,    75,
     857,    -1,    78,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,   969,   970,   971,   972,    -1,    -1,   336,    -1,    -1,
     978,    -1,   980,    -1,    -1,   983,   984,  1093,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   357,    -1,
      -1,    -1,    -1,    -1,    -1,   364,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   374,    -1,    -1,   377,    -1,
     379,    -1,   919,   382,    -1,    -1,   385,    -1,    -1,   388,
     389,  1137,    -1,   392,    -1,   955,   395,    -1,    -1,   398,
     399,   400,   401,    -1,    -1,   404,   966,   967,    -1,   408,
      -1,    -1,   411,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   995,   996,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1193,    -1,    -1,
      -1,    -1,  1198,    -1,    21,    -1,  1202,    -1,    -1,    26,
      -1,    -1,    29,    -1,    -1,  1002,    -1,    -1,    35,    36,
      -1,    38,    -1,    40,    41,    -1,  1036,  1037,  1038,  1039,
      47,    48,  1042,    50,    -1,    -1,    -1,  1047,  1048,    -1,
      57,    -1,  1029,    -1,    -1,    -1,    -1,    -1,  1035,    -1,
      -1,    -1,    -1,    -1,  1142,    72,    -1,    -1,    -1,    76,
      -1,  1149,    79,  1050,  1051,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,     4,  1071,    -1,    -1,    -1,    -1,    -1,
      -1,   540,   541,   542,   543,    -1,    -1,    -1,    -1,  1086,
    1087,    -1,    -1,    -1,    -1,    26,    27,    -1,    29,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    42,    -1,    -1,    -1,    46,    47,    48,    -1,    50,
      -1,    52,    53,    -1,    55,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,  1158,  1159,
      -1,    -1,    73,    -1,    -1,    76,    77,    -1,    79,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   622,    -1,    96,    -1,    -1,    -1,  1166,
      -1,    -1,    -1,  1170,    -1,    -1,    -1,    -1,    -1,    -1,
     639,   640,    -1,    -1,   643,   644,    -1,   646,   647,    -1,
     649,   650,    -1,   652,   653,   654,   655,   656,   657,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   668,
     669,    -1,    -1,    -1,    -1,    -1,   675,   676,    -1,    -1,
     679,   680,   681,    -1,   683,   684,   685,   686,   687,   688,
      -1,    -1,   691,   692,   693,   694,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   706,   707,   708,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   744,    -1,     4,    -1,    -1,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,   761,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      -1,    -1,   841,   842,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   854,    -1,    -1,    -1,   858,
      -1,    -1,   861,    -1,    -1,    -1,   865,    -1,   867,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     879,    -1,    -1,   882,   883,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   894,    -1,    -1,   897,    -1,
      -1,   900,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   961,   962,   963,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   977,    -1,
     979,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   994,    -1,    -1,   997,   998,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1010,    -1,    -1,    -1,    -1,  1015,    -1,    -1,  1018,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1040,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1096,    -1,  1098,
    1099,    -1,  1101,  1102,    -1,    -1,  1105,    -1,    -1,  1108,
    1109,     4,  1111,    -1,    -1,     8,     9,    10,    11,    12,
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
       0,   108,   113,   117,   436,     0,     5,     7,    54,    74,
     435,   435,   437,   437,   436,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,   109,   421,   442,   443,   421,
     110,   114,   436,   118,   436,    98,    98,   111,   425,   425,
      97,    11,   116,   426,   435,   120,   436,   427,    56,    64,
      82,   115,   122,   123,   137,   119,   121,   122,     8,   428,
     429,   437,   437,   437,   437,   124,   124,    23,   134,   112,
      98,   100,   432,     8,     9,    11,    12,    16,    17,    18,
     430,   441,   442,   109,   439,   441,   109,   420,   443,   439,
      43,    44,   125,   129,   138,   138,   437,   436,   436,   436,
      98,   100,   424,   440,   424,   436,   424,   437,   437,    28,
      31,    59,    67,   139,   140,   141,   142,   143,   143,   421,
      11,   422,   433,   442,   431,   425,   425,   436,   128,   421,
     128,   437,   437,   437,   437,    70,   144,   151,   151,   135,
     428,    99,    97,    99,   126,   130,   439,   439,   439,   439,
     437,   425,     4,    21,    26,    29,    35,    36,    38,    40,
      41,    47,    48,    50,    57,    72,    76,    79,    99,   152,
     153,   167,   172,   177,   225,   229,   230,   240,   248,   256,
     266,   281,   287,   303,   314,   332,   339,    99,    98,    98,
     100,   434,   436,   425,    98,    98,   100,   131,   424,   424,
     424,   424,    12,   109,   149,   437,   437,   437,   437,   437,
     437,   437,   437,   437,   437,   437,   437,   437,   437,   437,
     437,   436,   425,   436,   425,   436,   436,     8,   425,   425,
     425,   150,   145,   421,   101,   109,   311,   392,   393,   421,
     421,   421,   109,   343,   393,   421,   421,   421,   421,   421,
     421,   421,   421,   179,   421,    11,   109,   423,   443,   444,
     137,   433,   436,   127,   132,   436,    98,   100,   146,   282,
     422,   304,   394,   436,   267,   231,   154,   340,   436,   168,
     226,   173,   241,   249,   257,   333,   315,    98,   288,   436,
     436,   136,    99,    71,    99,   133,   137,   141,   142,    99,
     133,   141,   142,   425,    98,   100,   283,    98,   392,    98,
     100,   268,    98,   100,   232,    98,   100,   155,    98,    98,
     100,   169,    98,   100,   227,    98,   100,   174,    98,    98,
      98,    98,   100,   334,    98,   100,   316,   425,    98,   100,
     289,    99,   436,   437,   425,   147,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     182,   425,   425,   149,   148,   284,   306,   270,   233,   156,
     342,   170,   228,   175,   243,   251,   259,   335,   317,    30,
      73,    78,   141,   142,   162,   178,   183,   223,   224,   290,
     424,    99,   141,   142,    27,    42,    51,    55,    73,    81,
      99,   141,   142,   163,   171,   359,   364,   369,   380,    25,
      96,   141,   142,   162,   171,   229,   275,   305,   312,   359,
     141,   142,   163,   171,   224,   230,   240,   248,   256,   269,
     273,   274,   275,   281,   359,   364,   369,    65,    99,   141,
     142,   163,   171,   177,   225,   229,   312,   359,   364,   373,
     380,    20,    99,   141,   142,   157,   162,    37,   141,   142,
     341,    99,   141,   142,   163,   171,    99,   141,   142,   163,
     177,   225,   229,   312,    22,    99,   141,   142,   163,   171,
     176,   141,   142,   163,   171,   223,   224,   242,   245,   359,
     364,   369,   380,    52,    53,    58,   141,   142,   163,   171,
     223,   250,   253,   359,   364,   374,   376,   378,   380,    46,
      77,   141,   142,   163,   171,   177,   225,   229,   258,   312,
     359,   364,   374,   376,   378,   380,   383,   386,    99,   141,
     142,   163,   171,   177,   225,   229,    45,    60,    99,   141,
     142,   163,   171,   177,   225,   322,   329,   437,   437,   437,
      99,    75,   180,    21,    68,    99,   141,   142,   163,   171,
     294,   299,   359,   437,   437,   437,   437,   437,   437,   285,
     286,   437,   437,   308,   310,    99,   309,   307,    99,   272,
     271,   437,   238,   236,   239,   237,   234,   235,   437,   437,
      99,   425,   425,   425,   425,   437,    99,   180,   244,   247,
     437,   437,   437,    99,   180,   252,   255,   437,   437,   262,
     263,   265,    99,   264,   260,   261,   336,   337,   338,   436,
     436,   318,   319,   320,   321,   439,    85,    87,    90,   109,
     166,   439,   437,   425,   437,   437,   292,   293,   291,   365,
     439,   370,   439,   164,   439,   425,   425,   421,   421,   425,
     425,   425,   425,   425,   425,   439,   425,   425,   425,   425,
     425,   425,   421,    84,    86,    89,    91,   344,   345,   346,
     347,   351,   355,   423,   246,   425,   425,    17,    94,   109,
     377,   416,    18,   109,   375,   416,   417,    92,    95,   109,
     379,   254,   425,   425,   109,   387,   422,   109,   384,   395,
     422,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     323,   330,   425,   425,   425,   425,   424,   436,   436,   436,
     424,   424,   423,   109,   302,   395,   109,   297,   395,   425,
     425,   425,    88,    93,   109,   367,   424,    88,    93,   109,
     372,   381,   166,   360,   276,   313,   424,   158,   436,   436,
     436,   436,   436,   424,   425,   436,   424,   436,   424,   436,
     436,   436,   424,   425,   424,   388,   424,   385,   424,   437,
     396,    98,    98,   181,   300,   436,   295,   436,   436,   436,
     366,   436,   436,   371,    98,   100,   382,   165,    98,   100,
     361,    98,   100,   277,   316,    98,   100,   159,   348,   352,
     424,   356,   389,   424,   437,   395,   394,   425,   425,    98,
     100,   184,    98,    98,   100,   296,   424,   424,   425,   424,
     425,   425,   425,   425,    98,   100,   349,    98,   100,   353,
      98,   100,   357,   422,   385,   325,   325,   185,   425,   425,
     425,   194,   362,   278,    83,   160,   425,   425,   425,   390,
     177,   225,   229,   324,   331,    61,    69,   176,   186,   187,
     198,   199,   209,   210,   212,   213,   306,   298,    33,    34,
      99,   141,   142,   221,   222,    99,   141,   142,    99,   141,
     142,   163,   171,   229,   359,   437,    99,   350,   354,   358,
     389,   326,   327,   328,    99,    99,   437,   437,    99,    39,
      49,    62,    66,   188,   189,   191,   195,   207,    32,   201,
      24,   215,   301,    27,    51,    99,   141,   142,   224,   363,
     368,   373,   374,   376,   380,   437,   437,   280,   279,    88,
      93,   109,   161,    99,   223,   224,   363,   368,   374,   376,
     380,   383,    99,   223,   224,   380,   383,    99,   180,   223,
     224,   363,   368,   374,   376,   425,   425,   425,   109,   397,
     398,   399,    88,    93,   109,   211,   437,   437,   437,   437,
     180,   425,   437,   425,   437,   214,    99,   437,   437,   425,
     439,   439,   425,   425,   436,   436,   424,   425,   425,   425,
     424,   101,   400,   401,    14,   403,   404,   436,   436,   424,
     109,   190,   416,   192,   439,   196,   439,   391,   439,   425,
     202,   439,   200,   216,   421,   215,   367,   372,   424,   424,
     422,   402,   101,   405,   424,   436,    98,   100,   193,    98,
     100,   197,    98,   100,   208,    98,   100,   203,   201,    98,
     100,   217,   424,   424,   408,   400,   404,   406,   422,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   102,   407,
     408,   194,   194,   194,   204,   218,     5,   438,   401,    99,
      99,    99,    80,    99,   141,   142,   162,   171,   205,    63,
      99,   141,   142,   162,   171,   219,   409,   422,   437,   437,
     438,   438,    16,    17,    18,   109,   206,   418,   419,   109,
     220,   417,   103,   104,   424,   436,   424,   436,   438,    85,
     410,   415,   438,   438,   105,   101,   438,   438,   106,    14,
     411,   412,   438,   413,   101,   412,   422,   438,   414,   422,
     438,   101,   438,   422
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
     328,   325,   330,   331,   329,   333,   332,   334,   334,   335,
     335,   335,   335,   335,   336,   335,   337,   335,   338,   335,
     340,   341,   339,   342,   342,   342,   342,   343,   343,   344,
     344,   345,   345,   345,   346,   348,   347,   349,   349,   350,
     350,   350,   350,   350,   350,   350,   350,   350,   352,   351,
     353,   353,   354,   354,   354,   354,   354,   356,   355,   357,
     357,   358,   358,   358,   358,   358,   358,   358,   358,   360,
     359,   361,   361,   362,   362,   362,   363,   365,   366,   364,
     367,   367,   367,   368,   370,   371,   369,   372,   372,   372,
     373,   374,   375,   375,   376,   377,   377,   377,   378,   379,
     379,   379,   381,   380,   382,   382,   383,   384,   384,   385,
     385,   386,   388,   387,   387,   390,   389,   389,   391,   392,
     393,   394,   394,   396,   395,   398,   397,   399,   397,   397,
     400,   401,   402,   402,   403,   404,   405,   405,   406,   407,
     407,   408,   408,   409,   410,   411,   412,   413,   413,   414,
     414,   415,   416,   417,   417,   418,   418,   419,   419,   420,
     420,   421,   421,   422,   422,   423,   423,   423,   424,   424,
     425,   425,   425,   427,   426,   428,   429,   429,   430,   430,
     430,   431,   431,   432,   432,   433,   433,   434,   434,   435,
     435,   436,   436,   437,   438,   438,   440,   439,   439,   441,
     441,   441,   441,   441,   441,   441,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   443,   444
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
       0,     4,     0,     0,     8,     0,     5,     1,     4,     0,
       2,     2,     2,     2,     0,     4,     0,     4,     0,     4,
       0,     0,     9,     0,     2,     2,     4,     2,     1,     1,
       2,     1,     1,     1,     3,     0,     4,     1,     4,     0,
       2,     3,     2,     2,     2,     2,     2,     2,     0,     4,
       1,     4,     0,     2,     3,     2,     2,     0,     4,     1,
       4,     0,     3,     2,     2,     2,     2,     2,     2,     0,
       5,     1,     4,     0,     2,     2,     4,     0,     0,     6,
       2,     2,     1,     4,     0,     0,     6,     2,     2,     1,
       4,     4,     2,     1,     4,     2,     2,     1,     4,     2,
       2,     1,     0,     5,     1,     4,     3,     2,     2,     3,
       1,     3,     0,     3,     2,     0,     4,     1,     1,     2,
       2,     0,     2,     0,     3,     0,     2,     0,     2,     1,
       3,     2,     0,     2,     3,     2,     0,     2,     2,     0,
       2,     0,     6,     5,     5,     5,     4,     0,     2,     0,
       5,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     1,     1,     1,     2,     2,     1,     2,     4,
       0,     2,     2,     0,     4,     2,     0,     1,     0,     1,
       3,     0,     5,     1,     4,     0,     3,     2,     5,     1,
       1,     0,     2,     2,     0,     1,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
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

    case 342: /* deviation_opt_stmt  */

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

  case 456:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 458:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 460:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 461:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 462:

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

  case 463:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYABORT;
                                         }
                                       }

    break;

  case 465:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 466:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 467:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 469:

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

  case 470:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 471:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 472:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 473:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 474:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 476:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 478:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 480:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 481:

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

  case 483:

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

  case 484:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 485:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 486:

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

  case 489:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 495:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 499:

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

  case 500:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 501:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 502:

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

  case 503:

    { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 504:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 505:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 506:

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

  case 507:

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

  case 508:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 512:

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

  case 513:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 514:

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

  case 515:

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

  case 516:

    { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 517:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 521:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }

    break;

  case 522:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target) == -1) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 523:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 524:

    { if (read_all) {
                                               if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                 YYABORT;
                                               }
                                               s = NULL;
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }
                                           }

    break;

  case 525:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 526:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 527:

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

  case 528:

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

  case 529:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 534:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 535:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 536:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 537:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 538:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 539:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 540:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 541:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 542:

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

  case 543:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 544:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 545:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 546:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 547:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 548:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 549:

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

  case 551:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 552:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 553:

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

  case 554:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 555:

    { (yyval.uint) = 0; }

    break;

  case 556:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 557:

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

  case 558:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 559:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 560:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 561:

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

  case 562:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 572:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 575:

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

  case 578:

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

  case 579:

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

  case 583:

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

  case 585:

    { tmp_s = yyget_text(scanner); }

    break;

  case 586:

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

  case 587:

    { tmp_s = yyget_text(scanner); }

    break;

  case 588:

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

  case 612:

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

  case 613:

    { (yyval.uint) = 0; }

    break;

  case 614:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 615:

    { (yyval.i) = 0; }

    break;

  case 616:

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

  case 622:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 627:

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

  case 633:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 656:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 745:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 746:

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
