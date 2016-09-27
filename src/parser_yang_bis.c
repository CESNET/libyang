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
  char **p_str;
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
#define YYLAST   3358

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  341
/* YYNRULES -- Number of rules.  */
#define YYNRULES  767
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1237

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
       0,   270,   270,   271,   291,   315,   318,   320,   319,   342,
     355,   342,   364,   365,   373,   374,   377,   390,   377,   401,
     402,   410,   410,   419,   421,   423,   446,   447,   450,   450,
     470,   471,   477,   488,   499,   508,   510,   510,   533,   534,
     538,   539,   550,   561,   570,   582,   593,   582,   602,   604,
     605,   606,   607,   612,   618,   620,   622,   624,   626,   634,
     637,   637,   646,   647,   647,   652,   653,   658,   665,   665,
     674,   680,   719,   722,   723,   724,   725,   726,   727,   728,
     734,   735,   736,   738,   738,   757,   758,   762,   763,   770,
     777,   788,   800,   800,   802,   803,   807,   809,   810,   811,
     822,   824,   825,   824,   828,   829,   830,   831,   848,   848,
     857,   858,   863,   884,   897,   903,   908,   914,   916,   916,
     925,   926,   931,   971,   980,   989,   995,  1000,  1006,  1008,
    1019,  1042,  1045,  1052,  1053,  1059,  1065,  1070,  1077,  1079,
    1087,  1088,  1093,  1094,  1095,  1098,  1133,  1137,  1145,  1153,
    1154,  1158,  1159,  1160,  1168,  1181,  1187,  1188,  1206,  1210,
    1220,  1221,  1226,  1238,  1243,  1248,  1253,  1259,  1268,  1281,
    1282,  1286,  1287,  1299,  1304,  1309,  1314,  1320,  1333,  1333,
    1352,  1353,  1356,  1368,  1378,  1379,  1384,  1408,  1417,  1426,
    1432,  1437,  1443,  1455,  1456,  1474,  1479,  1480,  1485,  1487,
    1489,  1490,  1491,  1507,  1507,  1526,  1527,  1529,  1540,  1550,
    1551,  1556,  1580,  1589,  1598,  1604,  1609,  1615,  1627,  1628,
    1643,  1645,  1647,  1649,  1651,  1651,  1658,  1659,  1664,  1686,
    1692,  1697,  1702,  1703,  1710,  1711,  1712,  1723,  1724,  1725,
    1726,  1727,  1728,  1729,  1730,  1733,  1733,  1741,  1742,  1752,
    1788,  1788,  1790,  1797,  1797,  1805,  1806,  1812,  1818,  1823,
    1828,  1828,  1833,  1833,  1838,  1838,  1849,  1849,  1857,  1857,
    1864,  1895,  1903,  1933,  1933,  1935,  1942,  1942,  1949,  1950,
    1950,  1958,  1961,  1967,  1973,  1979,  1984,  1989,  1999,  2047,
    2088,  2088,  2090,  2097,  2097,  2104,  2124,  2125,  2125,  2133,
    2139,  2153,  2167,  2179,  2185,  2190,  2196,  2203,  2196,  2233,
    2278,  2278,  2280,  2287,  2287,  2295,  2309,  2317,  2323,  2337,
    2351,  2363,  2369,  2374,  2379,  2379,  2387,  2387,  2392,  2392,
    2397,  2397,  2408,  2408,  2416,  2416,  2427,  2429,  2428,  2451,
    2474,  2474,  2476,  2489,  2501,  2509,  2517,  2525,  2534,  2543,
    2543,  2553,  2554,  2557,  2558,  2559,  2560,  2561,  2562,  2563,
    2570,  2570,  2578,  2579,  2590,  2612,  2612,  2614,  2621,  2627,
    2632,  2637,  2637,  2644,  2644,  2656,  2656,  2668,  2669,  2680,
    2709,  2709,  2715,  2722,  2722,  2730,  2737,  2744,  2751,  2756,
    2762,  2762,  2783,  2784,  2788,  2825,  2825,  2827,  2834,  2840,
    2845,  2850,  2850,  2858,  2858,  2873,  2873,  2883,  2884,  2889,
    2890,  2893,  2961,  2968,  2977,  3001,  3021,  3040,  3059,  3082,
    3105,  3110,  3116,  3125,  3116,  3139,  3140,  3143,  3152,  3143,
    3169,  3190,  3190,  3192,  3199,  3208,  3213,  3218,  3218,  3226,
    3226,  3234,  3234,  3246,  3246,  3256,  3257,  3260,  3260,  3271,
    3271,  3282,  3283,  3288,  3318,  3325,  3331,  3336,  3341,  3341,
    3349,  3349,  3354,  3354,  3366,  3366,  3379,  3393,  3379,  3407,
    3438,  3438,  3446,  3446,  3454,  3454,  3459,  3459,  3469,  3483,
    3469,  3497,  3497,  3507,  3511,  3523,  3563,  3563,  3571,  3578,
    3584,  3589,  3594,  3594,  3602,  3602,  3607,  3607,  3614,  3623,
    3614,  3637,  3657,  3665,  3673,  3683,  3684,  3686,  3691,  3693,
    3694,  3695,  3698,  3700,  3700,  3706,  3707,  3718,  3745,  3753,
    3761,  3777,  3787,  3794,  3801,  3812,  3824,  3824,  3830,  3831,
    3856,  3883,  3891,  3902,  3912,  3923,  3923,  3929,  3933,  3934,
    3946,  3963,  3967,  3975,  3985,  3992,  3999,  4010,  4022,  4022,
    4025,  4026,  4030,  4031,  4036,  4042,  4044,  4045,  4044,  4048,
    4049,  4050,  4065,  4067,  4068,  4067,  4071,  4072,  4073,  4088,
    4090,  4092,  4093,  4114,  4116,  4117,  4118,  4139,  4141,  4142,
    4143,  4155,  4155,  4163,  4164,  4169,  4171,  4172,  4174,  4175,
    4177,  4179,  4179,  4188,  4191,  4191,  4202,  4205,  4215,  4236,
    4238,  4239,  4242,  4242,  4261,  4261,  4270,  4270,  4279,  4282,
    4284,  4286,  4287,  4289,  4291,  4293,  4294,  4296,  4298,  4299,
    4301,  4302,  4304,  4306,  4309,  4313,  4315,  4316,  4318,  4319,
    4321,  4323,  4334,  4335,  4338,  4339,  4351,  4352,  4354,  4355,
    4357,  4358,  4364,  4365,  4368,  4369,  4370,  4396,  4397,  4400,
    4401,  4402,  4405,  4405,  4413,  4415,  4416,  4418,  4419,  4420,
    4422,  4423,  4425,  4426,  4428,  4429,  4431,  4432,  4434,  4435,
    4438,  4439,  4442,  4444,  4445,  4448,  4448,  4457,  4459,  4460,
    4461,  4462,  4463,  4464,  4465,  4467,  4468,  4469,  4470,  4471,
    4472,  4473,  4474,  4475,  4476,  4477,  4478,  4479,  4480,  4481,
    4482,  4483,  4484,  4485,  4486,  4487,  4488,  4489,  4490,  4491,
    4492,  4493,  4494,  4495,  4496,  4497,  4498,  4499,  4500,  4501,
    4502,  4503,  4504,  4505,  4506,  4507,  4508,  4509,  4510,  4511,
    4512,  4513,  4514,  4515,  4516,  4517,  4518,  4519,  4520,  4521,
    4522,  4523,  4524,  4525,  4526,  4527,  4528,  4529,  4530,  4531,
    4532,  4533,  4534,  4535,  4536,  4537,  4538,  4539,  4540,  4541,
    4542,  4543,  4544,  4545,  4546,  4547,  4550,  4559
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
  "$@46", "choice_end", "$@47", "choice_opt_stmt", "$@48", "$@49",
  "short_case_case_stmt", "short_case_stmt", "case_stmt", "$@50",
  "case_end", "case_opt_stmt", "$@51", "$@52", "anyxml_stmt", "$@53",
  "anydata_stmt", "$@54", "anyxml_end", "anyxml_opt_stmt", "$@55", "$@56",
  "uses_stmt", "$@57", "uses_end", "uses_opt_stmt", "$@58", "$@59", "$@60",
  "refine_stmt", "$@61", "refine_end", "refine_arg_str",
  "refine_body_opt_stmts", "uses_augment_stmt", "$@62", "$@63",
  "uses_augment_arg_str", "augment_stmt", "$@64", "$@65",
  "augment_opt_stmt", "$@66", "$@67", "$@68", "$@69", "$@70",
  "augment_arg_str", "action_stmt", "$@71", "rpc_stmt", "$@72", "rpc_end",
  "rpc_opt_stmt", "$@73", "$@74", "$@75", "$@76", "input_stmt", "$@77",
  "$@78", "input_output_opt_stmt", "$@79", "$@80", "$@81", "$@82",
  "output_stmt", "$@83", "$@84", "notification_stmt", "$@85",
  "notification_end", "notification_opt_stmt", "$@86", "$@87", "$@88",
  "$@89", "deviation_stmt", "$@90", "$@91", "deviation_opt_stmt",
  "deviation_arg_str", "deviate_body_stmt", "deviate_stmts",
  "deviate_not_supported_stmt", "deviate_add_stmt", "$@92",
  "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt", "$@93",
  "deviate_delete_end", "deviate_delete_opt_stmt", "deviate_replace_stmt",
  "$@94", "deviate_replace_stmtsep", "deviate_replace_end",
  "deviate_replace_opt_stmt", "when_stmt", "$@95", "when_end",
  "when_opt_stmt", "config_stmt", "config_read_stmt", "$@96", "$@97",
  "config_arg_str", "mandatory_stmt", "mandatory_read_stmt", "$@98",
  "$@99", "mandatory_arg_str", "presence_stmt", "min_elements_stmt",
  "min_value_arg_str", "max_elements_stmt", "max_value_arg_str",
  "ordered_by_stmt", "ordered_by_arg_str", "must_stmt", "$@100",
  "must_end", "unique_stmt", "unique_arg_str", "unique_arg", "key_stmt",
  "key_arg_str", "$@101", "key_opt", "$@102", "range_arg_str",
  "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@103",
  "path_arg_str", "$@104", "$@105", "absolute_path", "absolute_paths",
  "absolute_path_opt", "relative_path", "relative_path_part1",
  "relative_path_part1_opt", "descendant_path", "descendant_path_opt",
  "path_predicate", "path_equality_expr", "path_key_expr",
  "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@106", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@107", "strings", "identifier", "identifiers",
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

#define YYPACT_NINF -1074

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1074)))

#define YYTABLE_NINF -621

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1074,     3, -1074, -1074,   276, -1074, -1074, -1074,   125,   125,
   -1074, -1074,  3165,  3165,   125, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074,   125,   -80,   125,   -58,    23, -1074, -1074,
   -1074,   375,   375,   244, -1074,   106, -1074, -1074,    -5, -1074,
     125,   125,   125,   125, -1074, -1074, -1074, -1074, -1074,   111,
   -1074, -1074,   171,  2405, -1074,  2500,  3165,  2500,    25,    25,
     125, -1074,   125, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074,   224, -1074, -1074,
     224, -1074,   224,   125,   125, -1074, -1074,   130,   130,  3165,
    2595,   125,   125, -1074, -1074, -1074, -1074, -1074,   125, -1074,
    3165,  3165,   125,   125,   125,   125, -1074, -1074, -1074, -1074,
      81,    81, -1074, -1074,   125,    40, -1074,    57,    49,   375,
     125, -1074, -1074, -1074,  2500,  2500,  2500,  2500,   125, -1074,
     737,  1308,    55,   231, -1074, -1074, -1074,    65,   257,   224,
     224,   224,   224,   117,   375,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074,   283,   375, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
    3165,    -7,  3165,  3165,  3165,    -7,  3165,  3165,  3165,  3165,
    3165,  3165,  3165,  3165,  3165,  2690,  3165,   125,   375,   125,
     265,  2595,   125, -1074,   375,   375,   375, -1074,   268, -1074,
    3260, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074,    85, -1074,   101, -1074, -1074, -1074,
   -1074,   105, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
     109,   125,   278,   395,   125, -1074, -1074, -1074,   323, -1074,
     120,    99,   125,   361,   377,   387,   147,   125,   416,   417,
     425, -1074, -1074,   148,   432,   436, -1074,   452,   125,   125,
     323,   143, -1074,   125, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074,   375, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   375,   375,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074,   375, -1074, -1074,
   -1074, -1074, -1074,   125,   117,   375, -1074,   375,   375,   375,
     375,   375,   375,   375,   375,   375,  1224,   363,   375,   375,
     375,   141,   375,   375,   224,   102,   440,  1456,  1789,  1180,
      90,   408,   145,   567,   142,   125,   125,   125,   125,   125,
     125,   125,   125,   125, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074,   125,   125,   125, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074,  1715,  1331,   896,   125, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074,   254, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   125,   125,
   -1074, -1074, -1074, -1074, -1074, -1074,   158, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
     161, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   125,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074,   125, -1074, -1074, -1074, -1074, -1074,
     125, -1074, -1074,   164, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   125, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074,  2500,  2500, -1074,  2500,
   -1074,  2690,  2500,  2500, -1074, -1074, -1074,    45,   396,    32,
   -1074, -1074, -1074,   125,   125, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074,   167, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074,    58,   375,   125,   125, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074,  3165,  3165, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,  2500, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074,  3165,   317, -1074,   375,
     375,   375,   375,   375,  2690,    27,   224,   224,    47, -1074,
      58,   453, -1074,   224, -1074,   375,   375,   375, -1074, -1074,
   -1074,   224, -1074, -1074, -1074,   224, -1074, -1074, -1074, -1074,
   -1074,   224,   375,   375,   375,  2785,  2785, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   125,
     125, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   224,
    2785,  2785, -1074, -1074, -1074,   375,   375, -1074, -1074,   375,
     375,   375,   375,   375,   375,   375,   224,   375,   375,   375,
     375,   375,   375,   375, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074,   224, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074,   456, -1074, -1074, -1074,
   -1074, -1074,   499,   125, -1074,   125, -1074,   125,   125,   125,
   -1074,   224, -1074, -1074,   224, -1074,    26, -1074,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   174,
     186,   375,   375,   375,   375,   125,   125,   125, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074,   375,   375,   375,   516,   436,
   -1074,   526,   125,   125,    26,   125,   125, -1074,   125,   125,
     224,   125,   125,   224, -1074, -1074, -1074,   224,   535, -1074,
   -1074, -1074, -1074,    26, -1074, -1074, -1074,  3260, -1074, -1074,
   -1074,   187,   125,   527,   125, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074,   531,   537, -1074,   558, -1074, -1074,   375, -1074,
     198,   620, -1074,   270, -1074,   182,   375, -1074, -1074,  3260,
      26,    99,   375,   375, -1074, -1074, -1074, -1074,   375,    71,
     375, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
     233, -1074,   125,   125,   125,   125,   125,   125, -1074,   250,
   -1074, -1074, -1074, -1074, -1074, -1074,   125, -1074,   125, -1074,
     321, -1074, -1074,   352,   352,   375,   375,  1411,   125,   245,
     375,   375, -1074,   375,   125,   125, -1074, -1074, -1074, -1074,
   -1074,    53,  2500,    94,  2500,  2500,    56, -1074,   375,  2500,
     375,  3165,   182, -1074, -1074, -1074,    26, -1074, -1074, -1074,
     253, -1074,   284,  1456,   227, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074,    59, -1074,   570,   183,   309,  2500,  2500, -1074,
     224, -1074,   562, -1074, -1074,   224,   229,   337,   577, -1074,
     585, -1074, -1074, -1074, -1074,   224,   375,   594, -1074,   270,
     599, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
     290,   125,   125, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   224,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074,   224,   224, -1074,   125, -1074, -1074, -1074, -1074,
    3260, -1074, -1074,   263, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074,   125,   125, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074,   375,   375,   375,   375, -1074,    27,    47,   375,
     375,   375,   125,   125, -1074,   375,   375,   375, -1074, -1074,
     375,   375, -1074,   229, -1074,  2880,   375,   375,   375,   375,
     375,   375,   375,   224,   224,   310,   291, -1074, -1074, -1074,
     -23,   222,   401,   316,   144, -1074, -1074, -1074,   393, -1074,
     -10,   125, -1074, -1074, -1074, -1074, -1074, -1074, -1074,   125,
   -1074, -1074, -1074, -1074, -1074, -1074,   125, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074,  3260, -1074,  2500,   468,   396,   393,
     393,   224, -1074, -1074, -1074, -1074,   224, -1074, -1074, -1074,
     224, -1074,   319,   327, -1074, -1074,   125, -1074,   125, -1074,
     393,   344,   393, -1074,   393,   333,   340,   393,   393,   339,
     446, -1074,   393, -1074, -1074,   362,  2975,   393, -1074, -1074,
   -1074,  3070, -1074,   367,   393,  3260, -1074
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     670,     0,     2,     3,     0,     1,   668,   669,     0,     0,
     671,   670,     0,     0,   672,   686,     4,   685,   687,   688,
     689,   690,   691,   692,   693,   694,   695,   696,   697,   698,
     699,   700,   701,   702,   703,   704,   705,   706,   707,   708,
     709,   710,   711,   712,   713,   714,   715,   716,   717,   718,
     719,   720,   721,   722,   723,   724,   725,   726,   727,   728,
     729,   730,   731,   732,   733,   734,   735,   736,   737,   738,
     739,   740,   741,   742,   743,   744,   745,   746,   747,   748,
     749,   750,   751,   752,   753,   754,   755,   756,   757,   758,
     759,   760,   761,   762,   763,   764,   765,   670,   641,     9,
     766,   670,    16,     6,     0,   640,     0,     5,   649,   649,
     670,    12,    19,     0,   652,    10,   651,   650,    17,     7,
     655,     0,     0,     0,    25,    13,    14,    15,    25,     0,
      20,   670,     0,   657,   656,     0,     0,     0,    49,    49,
       0,    22,     8,   670,   662,   653,   670,   678,   681,   679,
     683,   684,   682,   654,   670,   680,   677,     0,   675,   638,
       0,   670,     0,     0,     0,    26,    27,    58,    58,     0,
     664,   660,   658,   649,   649,    24,   670,    48,   639,    23,
       0,     0,     0,     0,     0,     0,    50,    51,    52,    53,
      71,    71,    45,   643,   655,     0,   642,   659,     0,   647,
     676,    28,    35,    36,     0,     0,     0,     0,     0,   649,
       0,     0,     0,     0,   663,   670,   649,     0,     0,     0,
       0,     0,     0,     0,    59,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   670,   649,    73,    74,    75,    76,    77,    78,
     237,   238,   239,   240,   241,   242,   243,   244,    79,    80,
      81,    82,   670,   649,   670,   670,   665,     0,   648,   649,
     649,    38,   649,    55,    56,    54,    57,    68,    70,    60,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    72,    18,
       0,   664,   666,   670,    30,    40,    37,   670,     0,   373,
       0,   446,   427,   600,   670,   334,   245,    83,   506,   498,
     670,   108,   224,   118,     0,   271,     0,   287,   306,   481,
     449,     0,   129,   767,   646,   390,   670,   670,   375,    46,
       0,   661,     0,     0,    69,   649,    62,    61,     0,   598,
       0,   599,   445,     0,     0,     0,     0,   505,     0,     0,
       0,   649,   649,     0,     0,     0,   649,     0,   644,   645,
       0,     0,   670,     0,   649,    34,    31,    32,    33,    39,
      43,    41,    42,    63,   649,   377,   374,   649,   601,   649,
     336,   335,   649,   247,   246,   649,    85,    84,   649,   649,
     110,   109,   649,   226,   225,   649,   120,   119,   272,   289,
     649,   649,   483,   482,   649,   451,   450,   131,   649,   392,
     391,   376,   649,   667,     0,    29,    65,   379,   430,   339,
     249,    87,   501,   112,   228,   122,     0,     0,   309,   485,
     453,     0,   394,    47,     0,     0,     0,   428,   337,     0,
       0,   499,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   285,   286,   284,   275,   276,
     278,   281,   273,   282,   283,   279,     0,     0,     0,   288,
     304,   305,   303,   292,   293,   296,   295,   290,   299,   300,
     301,   302,   297,   307,     0,     0,     0,   130,   136,   137,
     135,   649,   133,   134,     0,    44,    64,    66,    67,   378,
     388,   389,   387,   382,   380,   385,   386,   383,     0,     0,
     435,   436,   434,   433,   437,   443,     0,   439,   441,   431,
     347,   348,   346,   342,   343,   353,   354,   355,   356,   359,
       0,   349,   351,   352,   357,   358,   340,   344,   345,     0,
     248,   258,   259,   257,   252,   266,   260,   268,   262,   264,
     250,   256,   255,   253,     0,    86,    90,    91,    88,    89,
       0,   502,   503,     0,   111,   115,   116,   114,   113,   227,
     230,   231,   229,   649,   649,   649,   649,   649,     0,   121,
     126,   127,   125,   124,   123,   556,     0,     0,   563,     0,
     101,     0,     0,     0,   649,   649,   649,     0,     0,     0,
     649,   649,   649,     0,     0,   322,   323,   321,   312,   324,
     326,   332,     0,   328,   330,   310,   317,   318,   319,   320,
     313,   316,   315,   484,   490,   491,   489,   488,   492,   494,
     496,   486,   670,   670,   452,   456,   457,   455,   454,   458,
     460,   462,   464,     0,   132,     0,     0,   393,   399,   400,
     398,   397,   401,   403,   395,   649,   649,     0,     0,   649,
     649,   429,   649,   649,   649,   338,   649,   649,     0,   649,
     649,   649,   649,   649,   649,   649,     0,     0,   500,   233,
     232,   234,   235,   236,     0,     0,     0,     0,     0,   581,
       0,     0,   139,     0,   548,   277,   274,   280,   631,   670,
     576,     0,   670,   632,   572,     0,   633,   670,   670,   670,
     580,     0,   294,   291,   298,     0,     0,   649,   649,   649,
     308,   649,   649,   649,   649,   649,   649,   649,   649,   466,
     478,   649,   649,   649,   649,   670,   670,   670,   107,     0,
       0,     0,   649,   649,   649,   381,   384,   360,   447,   438,
     444,   440,   442,   432,   350,   341,     0,   267,   261,   269,
     263,   265,   251,   254,    92,   670,   670,   670,   670,   504,
     670,   507,   509,   511,   510,     0,   670,   670,   561,   557,
     223,   117,   670,   670,   568,   564,     0,   102,   649,   140,
     138,   222,     0,   574,   573,   575,   570,   571,   579,   578,
     577,     0,   590,   591,     0,   585,     0,   602,   325,   327,
     333,   329,   331,   311,   314,   493,   495,   497,   487,     0,
       0,   459,   461,   463,   465,   104,   106,   105,   100,   426,
     422,   670,   410,   405,   670,   402,   404,   396,     0,     0,
     569,     0,   513,   526,     0,   535,   508,   128,   560,   559,
       0,   567,   566,     0,   649,   583,   582,     0,   145,   649,
     550,   549,   593,     0,   587,   586,   589,     0,   600,   649,
     649,     0,   425,     0,   409,   649,   362,   361,   448,   649,
      94,   649,     0,     0,   512,     0,   558,   565,   162,   103,
       0,   142,   143,     0,   144,     0,   552,   592,   596,     0,
       0,   603,   469,   469,   649,   649,   407,   406,   364,     0,
      93,   649,   515,   514,   649,   528,   527,   649,   538,   536,
       0,   141,     0,     0,     0,     0,     0,     0,   148,     0,
     151,   149,   150,   649,   147,   146,     0,   649,     0,   205,
       0,   594,   588,   467,   479,   430,   411,     0,     0,     0,
     517,   530,   540,   537,     0,     0,   584,   165,   166,   163,
     164,     0,     0,   604,     0,     0,     0,   649,   152,     0,
     180,     0,   204,   551,   553,   554,     0,   472,   474,   476,
       0,   470,     0,   423,     0,   363,   369,   370,   368,   367,
     371,   365,     0,    95,     0,     0,     0,     0,     0,   157,
       0,   670,     0,   159,   608,     0,     0,     0,     0,   168,
       0,   597,   670,   670,   202,     0,   153,     0,   183,   179,
       0,   208,   206,   595,   649,   649,   649,   468,   649,   480,
       0,     0,     0,   408,   420,   421,   413,   415,   416,   417,
     414,   418,   419,   649,   649,   649,   670,   670,    99,     0,
     516,   518,   521,   522,   523,   524,   525,   649,   520,   529,
     531,   534,   649,   533,   539,   649,   542,   543,   544,   545,
     546,   547,     0,     0,   155,   156,   649,   160,   649,   198,
       0,   611,   605,     0,   607,   615,   649,   169,   649,   649,
     196,   195,   201,   200,   199,   649,   184,   182,   649,   649,
     209,   649,   473,   475,   477,   471,   424,     0,     0,   412,
     372,   366,    98,    97,    96,   519,   532,   541,   221,   220,
     162,   158,   620,   610,   614,     0,   171,   167,   162,   186,
     181,   211,   207,     0,     0,     0,   609,   612,   616,   613,
     618,     0,     0,     0,     0,   555,   562,   161,   673,   617,
       0,     0,   170,   175,   176,   172,   173,   174,   197,     0,
     185,   190,   191,   189,   187,   188,     0,   210,   215,   216,
     214,   212,   213,   674,     0,   619,     0,     0,     0,   673,
     673,     0,   636,   637,   634,   194,     0,   670,   635,   219,
       0,   670,     0,     0,   177,   192,   193,   217,   218,   621,
     673,     0,   673,   622,   673,     0,     0,   673,   673,     0,
       0,   630,   673,   623,   626,     0,     0,   673,   627,   628,
     625,   673,   624,     0,   673,     0,   629
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1074, -1074, -1074,   616, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074,   365, -1074,   366, -1074, -1074,
   -1074,   299, -1074, -1074, -1074, -1074,   154, -1074, -1074, -1074,
    -242,   351, -1074, -1074,   675,   745,   332, -1074, -1074, -1074,
   -1074, -1074,    77, -1074,   314, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074,  -411,  -254, -1074, -1074,  -180,
   -1074, -1074, -1074, -1074,  -420, -1074, -1074, -1074, -1074,  -375,
   -1074,  -404, -1074,  -428, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1073, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074,  -500, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074,  -443, -1074, -1074,
   -1074, -1074, -1074,  -608,  -607,  -416,  -429,  -390, -1074, -1074,
   -1074,  -406,   131, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074,   146, -1074, -1074, -1074, -1074, -1074, -1074,
     156, -1074, -1074, -1074, -1074,   157, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074,   160, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074,   172, -1074, -1074, -1074, -1074,
   -1074,   184, -1074,   188, -1074,   221, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,  -398, -1074,
   -1074, -1074, -1074, -1074, -1074,  -370, -1074, -1074, -1074,  -230,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,  -318, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074,  -401, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074, -1074,
   -1074, -1074, -1074, -1074, -1074,  -351, -1074, -1074, -1074,  -436,
       0, -1074, -1074,  -476,  -302,   259, -1074, -1074,  -483,  -350,
    -482, -1074,  -471, -1074,   152, -1074,  -422, -1074, -1074,  -898,
   -1074,  -261, -1074, -1074, -1074,  -336, -1074, -1074,  -349,   372,
    -217,  -700, -1074, -1074, -1074, -1074,  -467,  -493, -1074, -1074,
    -465, -1074, -1074, -1074,  -460, -1074, -1074, -1074,  -520, -1074,
   -1074, -1074,  -600,  -480, -1074, -1074, -1074,    24,  -164,  -563,
     864,  1299, -1074, -1074,   515, -1074, -1074, -1074, -1074,   410,
   -1074,    -4,   266,   490,  -534,   -93, -1074,   579,   168,  -113,
   -1074
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   107,   131,     2,   104,   124,   115,
       3,   106,   128,   118,   129,   125,   126,   138,   165,   217,
     342,   201,   166,   218,   272,   343,   375,   141,   212,   371,
     127,   167,   186,   187,   967,   968,   190,   209,   308,   347,
     426,   445,   279,   307,   210,   243,   244,   355,   397,   450,
     568,   851,   891,   959,  1059,   522,   467,   700,   867,   749,
     245,   358,   401,   452,   523,   246,   360,   407,   454,   594,
     331,   247,   441,   469,   701,   800,   900,   901,   939,   940,
    1010,   941,  1012,  1088,   930,   942,  1018,  1098,  1151,  1165,
     902,   903,  1029,   947,  1027,  1107,  1153,  1175,  1196,   943,
    1101,   944,   945,  1025,   904,   905,   982,   949,  1030,  1111,
    1154,  1182,  1200,   969,   970,   470,   471,   248,   359,   404,
     453,   249,   250,   354,   394,   449,   684,   685,   680,   682,
     683,   679,   681,   251,   324,   436,   605,   604,   606,   326,
     252,   437,   611,   610,   612,   253,   363,   622,   493,   733,
     734,   727,   728,   731,   732,   729,   254,   353,   391,   540,
     448,   677,   676,   541,   542,   525,   848,   887,   957,  1055,
    1054,   255,   348,   256,   370,   386,   446,   665,   666,   257,
     367,   420,   504,   754,   752,   753,   662,   883,   917,   843,
     994,   663,   881,  1040,   840,   258,   350,   526,   447,   674,
     669,   672,   673,   670,   312,   527,   849,   259,   365,   416,
     495,   741,   742,   743,   744,   651,   829,   990,   953,  1038,
    1034,  1035,  1036,   652,   830,   992,   260,   364,   413,   494,
     738,   735,   736,   737,   261,   356,   573,   451,   319,   779,
     780,   781,   782,   892,   923,  1004,   783,   893,   926,  1005,
     784,   895,   962,   929,  1006,   529,   802,   871,   950,  1048,
     473,   695,   860,   789,  1049,   474,   698,   863,   795,   562,
     489,   715,   490,   711,   491,   721,   991,   796,   866,   631,
     815,   875,   632,   812,   873,   907,   986,  1020,   313,   314,
     351,   816,   878,  1015,  1016,  1017,  1091,  1092,  1133,  1094,
    1095,  1135,  1149,  1159,  1146,  1189,  1213,  1223,  1224,  1226,
    1231,  1214,   716,   717,  1197,  1198,   160,   202,   817,   335,
     876,   111,   116,   120,   132,   133,   153,   197,   145,   195,
     266,   117,     4,   134,  1184,   157,   176,   158,   100,   101,
     337
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    16,   388,     5,    11,    11,   194,   712,   486,   484,
      10,   627,   503,   501,   475,   492,   468,   483,   -21,   534,
     108,   485,   628,   161,   517,   502,   513,   563,   533,   554,
     500,     6,   578,     7,   593,    16,    99,   102,   702,   569,
      16,   524,   109,   557,   162,   555,   528,   585,   559,   583,
     841,   844,   587,    16,     6,    16,     7,  1145,   339,   556,
     114,    16,   708,   584,    16,  1152,    16,    16,   163,   164,
     708,   630,   641,   618,   637,   648,     6,   123,     7,   558,
    -620,  -620,   114,   586,   661,   472,   487,   621,   640,   619,
     638,   649,   624,  1090,  1158,   514,   310,   546,   560,    10,
     376,    10,    16,   620,   639,   650,  1068,  1073,  -606,    10,
     564,   219,   220,   221,   222,   786,    11,    11,    11,    11,
     787,   183,   110,   623,   718,    16,   173,   719,   174,   277,
       6,   785,     7,   183,   140,   792,    11,   194,    10,   709,
     793,   214,   625,   745,  1022,   746,   349,  1056,   747,  1023,
     216,   208,  1057,   664,   958,   263,   215,   185,   182,    11,
      11,   183,   121,   496,   588,   269,    10,    10,    10,   185,
     122,   456,   183,   183,    10,   183,   183,   910,    11,    11,
      11,    11,   336,   482,   457,   361,   457,   457,   123,   184,
      11,   565,   512,   192,   532,   553,    10,   185,   577,   582,
     592,   362,   310,   506,    11,   366,   948,  1176,   185,   185,
     372,   185,   185,   456,   496,   460,   461,   496,   460,   462,
     387,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,   459,   617,
     636,   647,   497,   589,   422,  1177,   574,   398,   410,     6,
     660,     7,   119,   183,  1041,   964,   965,   456,   183,   671,
     614,   462,   675,    10,   183,   688,   964,   965,   730,   457,
       6,   143,     7,   144,   879,   655,   114,    14,  1042,   476,
     477,     6,   459,     7,  1069,   183,   880,   914,     6,   185,
       7,   303,   549,    10,   185,    10,   457,    10,    10,   931,
     185,   155,   946,   155,   309,   155,   315,   316,   317,   183,
     321,   322,   323,   325,   327,   328,   329,   330,   332,  1161,
     338,   185,   656,  1162,   173,   461,   174,   460,  1043,   122,
       8,   264,  1090,   265,   966,   463,  1041,    10,   196,   456,
      10,   183,   122,   964,   965,   185,  1003,   183,    10,   373,
       9,  1093,   183,    10,  1037,   657,   225,   270,   457,   271,
    1042,   476,   477,   103,    10,    10,  1134,   105,   345,    11,
     346,  1011,   155,   155,   155,   155,   113,   185,   227,   374,
       6,   228,     7,   185,   461,  1039,   114,   462,   185,   496,
     455,  1116,   232,   456,   183,  1158,  1169,   142,  1183,   234,
     235,   775,   236,   776,    16,   457,   777,   459,   778,   170,
    1074,  1157,   171,   708,   713,   476,   477,  1170,   459,    10,
     172,   478,   983,   384,  1209,   385,   183,   178,   239,  1212,
     185,   240,   183,  1210,   964,   965,   460,   488,   461,   183,
    1217,   462,   200,  1218,   463,   570,   515,  1221,   547,   561,
     241,    11,    11,    11,    11,    11,    11,    11,    11,    11,
    1222,   389,   185,   390,   479,  1227,   373,   455,   185,   196,
    1234,   183,    11,    11,    11,   185,    16,   392,   196,   393,
     203,   267,   457,   130,  1192,  1193,  1194,   395,   336,   396,
     168,   458,    11,   626,   139,   459,   379,   380,    12,    13,
     191,   444,  1168,   696,   697,   211,   699,   185,   297,   703,
     704,   977,  1051,   460,    11,    11,   399,   402,   400,   403,
     797,   463,  1065,  1052,  1080,   405,   938,   406,   299,  1108,
     301,   302,   411,  1066,   412,  1081,   414,   999,   415,  1032,
       6,   509,     7,  1166,  1167,    11,   114,   989,   989,   987,
     987,  1000,   418,   798,   419,   799,   864,   993,   865,  -203,
      11,   813,   388,   988,   988,  1047,    11,  -178,  1063,   341,
    1078,   225,  1053,   344,  1046,  1062,  1071,  1077,  1075,   535,
     352,   336,  1067,  1072,    11,   766,   357,   524,  1061,  1070,
    1076,   421,   528,   227,   536,   954,   228,  1041,   183,   869,
     456,   870,   368,   369,   537,   538,  1001,   232,   539,    11,
      11,   135,   136,   137,   234,   235,   885,   236,   886,   888,
     543,  1042,   476,   477,   237,   459,   889,   915,   890,   916,
     169,   921,   544,   922,   185,  1144,   545,   924,   423,   925,
     460,  1143,   588,   239,  1050,   629,   240,   614,   462,   952,
    1033,    11,    11,   180,   181,  1202,  1203,   320,   927,   932,
     928,   911,  1086,   519,  1087,   241,  1147,  1185,   579,   933,
    1148,  1060,   204,   205,   206,   207,  1211,  1096,  1215,  1097,
    1216,   934,   935,  1219,  1220,  1099,   936,  1100,  1225,   937,
    1160,   757,   758,  1230,  1105,  -154,  1106,  1233,   223,  1109,
    1235,  1110,  1064,   998,  1079,   516,  1228,   548,  1201,   213,
     774,   340,   154,     0,     0,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,     0,  1174,  1181,    10,    10,     0,     0,     0,
       0,   225,  1173,  1180,     0,   951,     0,     0,     0,     0,
       0,   156,   159,   156,     0,     0,     0,     0,   226,     0,
       0,     0,     0,   227,   155,   155,   228,   155,     0,     0,
     155,   155,   229,   230,     0,   231,     0,   232,   233,     0,
       0,     0,     0,     0,   234,   235,     0,   236,     0,     0,
       0,     0,     0,     0,   237,     0,     0,     0,     0,    10,
       0,    10,     0,    10,    10,    10,     0,     0,     0,   238,
       0,     0,    11,   239,     0,     0,   240,     0,     0,     0,
     156,   156,   156,   156,     0,     0,     0,     0,     0,     0,
       0,    10,    10,    10,     0,   241,     0,     0,   242,   278,
       0,     0,   188,   188,     0,     0,   155,     0,    10,    10,
      10,    10,    10,     0,    10,    10,     0,    10,    10,     0,
       0,     0,     0,   424,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,    10,  1013,
      10,  1019,  1021,     0,     0,     0,  1028,     0,     0,     0,
       0,     0,     0,   196,   196,     0,     0,   311,     0,     0,
       0,   318,     0,     0,     0,     0,    11,     0,   739,   740,
       0,   334,   189,   189,  1082,  1083,     0,     0,   196,   196,
       0,     0,     0,     0,     0,     0,  1132,   183,    11,    11,
      11,    11,    11,    11,     0,     0,   232,     0,   457,     0,
       0,   642,    11,     0,    11,   595,   596,   597,   598,   599,
     600,   601,   602,   603,    11,     0,   643,     0,     0,     0,
      11,    11,     0,   185,     0,     0,   607,   608,   609,   460,
       0,  1150,   239,     0,     0,   803,     0,     0,   805,     0,
       0,     0,    11,   807,   808,   809,   653,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   644,     0,     0,
       0,     0,     0,     0,     0,  1031,     0,     0,   667,   668,
       0,   835,   836,   837,     0,     0,     0,   377,   381,     0,
    1190,   175,     0,     0,   177,     0,   179,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    11,   678,
     278,   852,   853,   854,   855,   196,   856,     0,     0,     0,
       0,     0,   858,   859,   686,     0,     0,     0,   861,   862,
     687,     0,  1229,     0,     0,     0,     0,  1232,     0,     0,
       0,  1236,     0,     0,     0,     0,     0,   196,   694,     0,
       0,    10,     0,   273,   274,   275,   276,   378,   382,     0,
       0,     0,     0,  1191,     0,     0,     0,     0,    10,    10,
       0,     0,     0,   725,   726,     0,     0,   882,     0,     0,
     884,   465,   480,     0,     0,     0,   498,     0,    10,    10,
     507,   510,   520,   530,   551,   566,   571,   575,   580,   590,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     155,     0,   155,   155,     0,   750,   751,   155,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     0,     0,
       0,     0,     0,     0,     0,    11,     0,     0,   615,   634,
     645,     0,    11,     0,     0,   155,   155,     0,     0,   658,
       0,   466,   481,     0,   225,     0,   499,     0,     0,     0,
     508,   511,   521,   531,   552,   567,   572,   576,   581,   591,
       0,     0,    10,     0,    10,     0,   227,   455,     0,   228,
       0,   183,   156,   156,     0,   156,     0,   334,   156,   156,
     232,     0,   457,   710,   714,   720,     0,   234,   235,     0,
     236,     0,     0,     0,     0,   459,     0,   237,   616,   635,
     646,     0,     0,     0,     0,   549,     0,   185,     0,   659,
       0,   455,     0,   460,   456,   183,   239,     0,   196,   240,
       0,   463,     0,     0,     0,     0,   457,     0,     0,   748,
       0,     0,     0,     0,     0,   458,   519,  1085,   241,   459,
       0,   550,     0,     0,     0,     0,     0,     0,  1102,  1103,
       0,   185,     0,     0,   156,     0,     0,   460,     0,   461,
       0,     0,   462,   196,     0,   463,   877,     0,   505,     0,
     334,   788,   225,     0,   794,     0,   748,     0,     0,     0,
       0,     0,  1122,  1123,     0,   464,     0,     0,     0,   226,
       0,     0,     0,     0,   227,   225,     0,   228,     0,     0,
       0,   811,   814,   229,   230,     0,   231,     0,   232,   233,
       0,     0,   196,     0,   155,   234,   235,   227,   236,     0,
     228,     0,   183,   909,     0,   237,   839,   842,     0,     0,
       0,   232,     0,   457,     0,     0,     0,     0,   234,   235,
     238,   236,     0,     0,   239,     0,   459,   240,     0,     0,
       0,     0,     0,     0,   196,     0,     0,     0,   185,   196,
     877,     0,     0,   196,   460,     0,   241,   239,   112,   262,
     240,     0,     0,     0,     0,   225,     0,     0,     0,     0,
       0,     0,   971,   972,   973,   974,   975,   976,     0,   241,
       0,     0,   633,     0,     0,     0,   979,   227,   981,     0,
     228,     0,   183,     0,     0,     0,     0,     0,  1002,     0,
       0,     0,     0,   457,  1007,  1008,     0,     0,   234,   235,
     225,   236,     0,  1206,     0,     0,     0,  1208,     0,     0,
       0,     0,   198,   199,     0,     0,   909,     0,   185,     0,
       0,   518,   227,     0,   460,   228,     0,   183,     0,     0,
     240,     0,   463,     0,     0,     0,     0,     0,   457,     0,
       0,     0,     0,   234,   235,     0,   236,     0,   224,   241,
       0,     0,   995,   237,     0,   268,     0,     0,     0,     0,
       0,     0,     0,   185,     0,     0,     0,     0,     0,   496,
       0,  1117,  1118,     0,     0,   240,     0,   463,     0,     0,
       0,     0,   298,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   519,     0,   241,     0,     0,     0,     0,     0,
     790,   791,   300,     0,     0,     0,     0,   801,   304,   305,
       0,   306,     0,     0,     0,   804,     0,     0,     0,   806,
       0,     0,     0,     0,     0,   810,     0,  1009,   156,  1014,
     156,   156,  1024,     0,     0,   156,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   838,     0,     0,     0,     0,  1058,     0,
       0,     0,     0,   156,   156,   984,     0,     0,     0,     0,
     850,     0,   996,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   383,     0,     0,     0,     0,   857,
       0,  1186,     0,     0,     0,     0,     0,     0,     0,  1187,
     408,   409,     0,     0,     0,   417,  1188,     0,   520,  1044,
       0,     0,     0,   425,     0,   872,     0,     0,   874,     0,
       0,     0,     0,   427,     0,     0,   428,     0,   429,     0,
       0,   430,     0,     0,   431,   985,     0,   432,   433,     0,
       0,   434,   997,     0,   435,     0,     0,     0,     0,   438,
     439,     0,     0,   440,     0,     0,     0,   442,   894,   225,
       0,   443,     0,     0,   896,     0,     0,   897,     0,     0,
       0,   899,     0,   788,   794,     0,     0,   908,   521,  1045,
       0,   227,   455,     0,   228,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,   232,     0,   457,     0,     0,
       0,   613,   234,   235,     0,   236,     0,   476,   477,     0,
     459,     0,   237,   478,     0,     0,     0,     0,     0,     0,
       0,     0,   185,     0,     0,     0,     0,     0,   460,     0,
       0,   239,   614,   225,   240,     0,   463,     0,     0,     0,
     654,     0,   156,  1195,  1199,     0,     0,     0,     0,     0,
       0,   519,     0,   241,   518,   227,   455,     0,   228,   456,
     183,     0,     0,     0,     0,     0,  1163,     0,  1171,  1178,
       0,   457,     0,     0,     0,     0,   234,   235,     0,   236,
     458,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     908,     0,     0,     0,     0,     0,   185,     0,     0,     0,
       0,     0,   460,     0,     0,     0,     0,     0,     0,     0,
     463,     0,     0,     0,  1084,     0,     0,     0,     0,  1089,
       0,     0,   689,   690,   691,   692,   693,   241,     0,  1104,
       0,     0,     0,     0,     0,     0,  1164,     0,  1172,  1179,
       0,     0,     0,   705,   706,   707,     0,     0,     0,   722,
     723,   724,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1124,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1128,  1129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   755,   756,     0,     0,   759,   760,
       0,   761,   762,   763,     0,   764,   765,     0,   767,   768,
     769,   770,   771,   772,   773,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1155,  1156,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   818,   819,   820,     0,
     821,   822,   823,   824,   825,   826,   827,   828,     0,     0,
     831,   832,   833,   834,     0,     0,     0,     0,     0,     0,
       0,   845,   846,   847,     0,  1204,     0,     0,     0,     0,
    1205,     0,     0,     0,  1207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   868,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   898,     0,     0,     0,     0,   906,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   912,   913,
       0,     0,     0,     0,   918,     0,     0,     0,   919,     0,
     920,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   955,   956,     0,     0,     0,     0,     0,
     960,     0,     0,   961,     0,     0,   963,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   978,     0,     0,     0,   980,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1026,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1112,  1113,  1114,     0,  1115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1119,  1120,  1121,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1125,     0,     0,     0,
       0,  1126,     0,     0,  1127,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1130,     0,  1131,     0,     0,
       0,     0,     0,     0,     0,  1136,     0,  1137,  1138,     0,
       0,     0,     0,     0,  1139,     0,     0,  1140,  1141,    15,
    1142,     0,     0,   146,   147,    17,   148,   149,     0,     0,
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
       6,     0,     7,     0,     0,    17,   193,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,     0,
      17,   333,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,    16,     0,    17,   193,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,     0,     0,
      17,   193,     0,     0,  1093,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,     0,     0,    17,   193,     0,     0,  1222,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,  1183,     0,     0,     0,     0,
      17,   193,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,    16,     0,    17,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,     0,     0,
      17,   193,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96
};

static const yytype_int16 yycheck[] =
{
       4,     8,   351,     0,     8,     9,   170,   607,   437,   437,
      14,   493,   441,   441,   436,   437,   436,   437,    23,   448,
     100,   437,   493,   136,   446,   441,   446,   449,   448,   449,
     441,     5,   452,     7,   454,     8,    12,    13,   601,   450,
       8,   447,   100,   449,   137,   449,   447,   453,   449,   453,
     750,   751,   453,     8,     5,     8,     7,  1130,   300,   449,
      11,     8,    17,   453,     8,  1138,     8,     8,    43,    44,
      17,   493,   494,   493,   494,   495,     5,    82,     7,   449,
     103,   104,    11,   453,   504,   436,   437,   493,   494,   493,
     494,   495,   493,   103,   104,   446,   103,   448,   449,   103,
     342,   105,     8,   493,   494,   495,  1004,  1005,    14,   113,
      20,   204,   205,   206,   207,    88,   120,   121,   122,   123,
      93,    31,    99,   493,    92,     8,   100,    95,   102,    12,
       5,   694,     7,    31,    23,    88,   140,   301,   142,    94,
      93,   101,   493,    85,    88,    87,   310,    88,    90,    93,
     101,    70,    93,   504,    83,   100,    99,    67,    28,   163,
     164,    31,    56,    73,    22,   100,   170,   171,   172,    67,
      64,    30,    31,    31,   178,    31,    31,   877,   182,   183,
     184,   185,   295,   437,    42,   100,    42,    42,    82,    59,
     194,   101,   446,   169,   448,   449,   200,    67,   452,   453,
     454,   100,   103,   101,   208,   100,    24,    63,    67,    67,
     101,    67,    67,    30,    73,    73,    75,    73,    73,    78,
     100,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,    55,   493,
     494,   495,   101,   101,   101,   101,   101,   100,   100,     5,
     504,     7,     8,    31,    27,    33,    34,    30,    31,   101,
      77,    78,   101,   267,    31,   101,    33,    34,   101,    42,
       5,   100,     7,   102,   100,    21,    11,    11,    51,    52,
      53,     5,    55,     7,   101,    31,   100,   100,     5,    67,
       7,     8,    65,   297,    67,   299,    42,   301,   302,   101,
      67,   133,    32,   135,   280,   137,   282,   283,   284,    31,
     286,   287,   288,   289,   290,   291,   292,   293,   294,    97,
     296,    67,    68,   101,   100,    75,   102,    73,   101,    64,
      54,   100,   103,   102,   101,    81,    27,   341,   170,    30,
     344,    31,    64,    33,    34,    67,   101,    31,   352,    71,
      74,    14,    31,   357,   101,   101,     4,   100,    42,   102,
      51,    52,    53,    97,   368,   369,   103,   101,   100,   373,
     102,   971,   204,   205,   206,   207,   110,    67,    26,   101,
       5,    29,     7,    67,    75,   101,    11,    78,    67,    73,
      27,   101,    40,    30,    31,   104,    80,   131,     5,    47,
      48,    84,    50,    86,     8,    42,    89,    55,    91,   143,
     101,   101,   146,    17,    18,    52,    53,   101,    55,   423,
     154,    58,   101,   100,   105,   102,    31,   161,    76,    85,
      67,    79,    31,   106,    33,    34,    73,   437,    75,    31,
     107,    78,   176,   103,    81,    37,   446,   108,   448,   449,
      98,   455,   456,   457,   458,   459,   460,   461,   462,   463,
      14,   100,    67,   102,   101,   103,    71,    27,    67,   301,
     103,    31,   476,   477,   478,    67,     8,   100,   310,   102,
     181,   215,    42,   118,    16,    17,    18,   100,   601,   102,
     139,    51,   496,   493,   128,    55,   101,   343,     8,     9,
     168,   424,   101,   596,   597,   191,   599,    67,   242,   602,
     603,   939,   994,    73,   518,   519,   100,   100,   102,   102,
     700,    81,  1004,   994,  1006,   100,   901,   102,   262,  1029,
     264,   265,   100,  1004,   102,  1006,   100,   957,   102,   982,
       5,   101,     7,  1151,  1151,   549,    11,   953,   954,   953,
     954,   957,   100,   100,   102,   102,   100,   955,   102,    24,
     564,   725,   911,   953,   954,   994,   570,    32,  1004,   303,
    1006,     4,   994,   307,   994,  1004,  1005,  1006,  1006,   448,
     314,   694,  1004,  1005,   588,   678,   320,   993,  1004,  1005,
    1006,   370,   993,    26,   448,   913,    29,    27,    31,   100,
      30,   102,   336,   337,   448,   448,   957,    40,   448,   613,
     614,   121,   122,   123,    47,    48,   100,    50,   102,   849,
     448,    51,    52,    53,    57,    55,   100,   100,   102,   102,
     140,   100,   448,   102,    67,  1118,   448,   100,   372,   102,
      73,  1117,    22,    76,   994,   493,    79,    77,    78,   910,
     986,   655,   656,   163,   164,  1189,  1190,   285,   100,    39,
     102,   878,   100,    96,   102,    98,  1133,  1160,   101,    49,
    1135,   101,   182,   183,   184,   185,  1210,   100,  1212,   102,
    1214,    61,    62,  1217,  1218,   100,    66,   102,  1222,    69,
    1150,   667,   668,  1227,   100,    75,   102,  1231,   208,   100,
    1234,   102,  1004,   957,  1006,   446,  1226,   448,  1188,   194,
     686,   301,   133,    -1,    -1,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,  1153,  1154,   739,   740,    -1,    -1,    -1,
      -1,     4,  1153,  1154,    -1,   909,    -1,    -1,    -1,    -1,
      -1,   135,   136,   137,    -1,    -1,    -1,    -1,    21,    -1,
      -1,    -1,    -1,    26,   596,   597,    29,   599,    -1,    -1,
     602,   603,    35,    36,    -1,    38,    -1,    40,    41,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,   803,
      -1,   805,    -1,   807,   808,   809,    -1,    -1,    -1,    72,
      -1,    -1,   816,    76,    -1,    -1,    79,    -1,    -1,    -1,
     204,   205,   206,   207,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   835,   836,   837,    -1,    98,    -1,    -1,   101,   223,
      -1,    -1,   167,   168,    -1,    -1,   678,    -1,   852,   853,
     854,   855,   856,    -1,   858,   859,    -1,   861,   862,    -1,
      -1,    -1,    -1,   373,    -1,    -1,    -1,    -1,    -1,   873,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   882,   972,
     884,   974,   975,    -1,    -1,    -1,   979,    -1,    -1,    -1,
      -1,    -1,    -1,   725,   726,    -1,    -1,   281,    -1,    -1,
      -1,   285,    -1,    -1,    -1,    -1,   910,    -1,   642,   643,
      -1,   295,   167,   168,  1007,  1008,    -1,    -1,   750,   751,
      -1,    -1,    -1,    -1,    -1,    -1,  1090,    31,   932,   933,
     934,   935,   936,   937,    -1,    -1,    40,    -1,    42,    -1,
      -1,    45,   946,    -1,   948,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   958,    -1,    60,    -1,    -1,    -1,
     964,   965,    -1,    67,    -1,    -1,   476,   477,   478,    73,
      -1,  1135,    76,    -1,    -1,   709,    -1,    -1,   712,    -1,
      -1,    -1,   986,   717,   718,   719,   496,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   981,    -1,    -1,   518,   519,
      -1,   745,   746,   747,    -1,    -1,    -1,   342,   343,    -1,
    1184,   157,    -1,    -1,   160,    -1,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1041,  1042,   549,
     424,   775,   776,   777,   778,   877,   780,    -1,    -1,    -1,
      -1,    -1,   786,   787,   564,    -1,    -1,    -1,   792,   793,
     570,    -1,  1226,    -1,    -1,    -1,    -1,  1231,    -1,    -1,
      -1,  1235,    -1,    -1,    -1,    -1,    -1,   909,   588,    -1,
      -1,  1085,    -1,   219,   220,   221,   222,   342,   343,    -1,
      -1,    -1,    -1,  1186,    -1,    -1,    -1,    -1,  1102,  1103,
      -1,    -1,    -1,   613,   614,    -1,    -1,   841,    -1,    -1,
     844,   436,   437,    -1,    -1,    -1,   441,    -1,  1122,  1123,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     972,    -1,   974,   975,    -1,   655,   656,   979,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1169,    -1,    -1,   493,   494,
     495,    -1,  1176,    -1,    -1,  1007,  1008,    -1,    -1,   504,
      -1,   436,   437,    -1,     4,    -1,   441,    -1,    -1,    -1,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
      -1,    -1,  1206,    -1,  1208,    -1,    26,    27,    -1,    29,
      -1,    31,   596,   597,    -1,   599,    -1,   601,   602,   603,
      40,    -1,    42,   607,   608,   609,    -1,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    55,    -1,    57,   493,   494,
     495,    -1,    -1,    -1,    -1,    65,    -1,    67,    -1,   504,
      -1,    27,    -1,    73,    30,    31,    76,    -1,  1090,    79,
      -1,    81,    -1,    -1,    -1,    -1,    42,    -1,    -1,   653,
      -1,    -1,    -1,    -1,    -1,    51,    96,  1011,    98,    55,
      -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,  1022,  1023,
      -1,    67,    -1,    -1,   678,    -1,    -1,    73,    -1,    75,
      -1,    -1,    78,  1135,    -1,    81,   816,    -1,   444,    -1,
     694,   695,     4,    -1,   698,    -1,   700,    -1,    -1,    -1,
      -1,    -1,  1056,  1057,    -1,   101,    -1,    -1,    -1,    21,
      -1,    -1,    -1,    -1,    26,     4,    -1,    29,    -1,    -1,
      -1,   725,   726,    35,    36,    -1,    38,    -1,    40,    41,
      -1,    -1,  1184,    -1,  1186,    47,    48,    26,    50,    -1,
      29,    -1,    31,   873,    -1,    57,   750,   751,    -1,    -1,
      -1,    40,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,
      72,    50,    -1,    -1,    76,    -1,    55,    79,    -1,    -1,
      -1,    -1,    -1,    -1,  1226,    -1,    -1,    -1,    67,  1231,
     910,    -1,    -1,  1235,    73,    -1,    98,    76,   109,   101,
      79,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,   932,   933,   934,   935,   936,   937,    -1,    98,
      -1,    -1,   101,    -1,    -1,    -1,   946,    26,   948,    -1,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,   958,    -1,
      -1,    -1,    -1,    42,   964,   965,    -1,    -1,    47,    48,
       4,    50,    -1,  1197,    -1,    -1,    -1,  1201,    -1,    -1,
      -1,    -1,   173,   174,    -1,    -1,   986,    -1,    67,    -1,
      -1,    25,    26,    -1,    73,    29,    -1,    31,    -1,    -1,
      79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    47,    48,    -1,    50,    -1,   209,    98,
      -1,    -1,   101,    57,    -1,   216,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,  1041,  1042,    -1,    -1,    79,    -1,    81,    -1,    -1,
      -1,    -1,   243,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    -1,    98,    -1,    -1,    -1,    -1,    -1,
     696,   697,   263,    -1,    -1,    -1,    -1,   703,   269,   270,
      -1,   272,    -1,    -1,    -1,   711,    -1,    -1,    -1,   715,
      -1,    -1,    -1,    -1,    -1,   721,    -1,   971,   972,   973,
     974,   975,   976,    -1,    -1,   979,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   749,    -1,    -1,    -1,    -1,  1002,    -1,
      -1,    -1,    -1,  1007,  1008,   950,    -1,    -1,    -1,    -1,
     766,    -1,   957,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,    -1,   785,
      -1,  1161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1169,
     361,   362,    -1,    -1,    -1,   366,  1176,    -1,   993,   994,
      -1,    -1,    -1,   374,    -1,   811,    -1,    -1,   814,    -1,
      -1,    -1,    -1,   384,    -1,    -1,   387,    -1,   389,    -1,
      -1,   392,    -1,    -1,   395,   950,    -1,   398,   399,    -1,
      -1,   402,   957,    -1,   405,    -1,    -1,    -1,    -1,   410,
     411,    -1,    -1,   414,    -1,    -1,    -1,   418,   854,     4,
      -1,   422,    -1,    -1,   860,    -1,    -1,   863,    -1,    -1,
      -1,   867,    -1,  1117,  1118,    -1,    -1,   873,   993,   994,
      -1,    26,    27,    -1,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    42,    -1,    -1,
      -1,    46,    47,    48,    -1,    50,    -1,    52,    53,    -1,
      55,    -1,    57,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    76,    77,     4,    79,    -1,    81,    -1,    -1,    -1,
     501,    -1,  1186,  1187,  1188,    -1,    -1,    -1,    -1,    -1,
      -1,    96,    -1,    98,    25,    26,    27,    -1,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,  1151,    -1,  1153,  1154,
      -1,    42,    -1,    -1,    -1,    -1,    47,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     986,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,  1010,    -1,    -1,    -1,    -1,  1015,
      -1,    -1,   583,   584,   585,   586,   587,    98,    -1,  1025,
      -1,    -1,    -1,    -1,    -1,    -1,  1151,    -1,  1153,  1154,
      -1,    -1,    -1,   604,   605,   606,    -1,    -1,    -1,   610,
     611,   612,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1059,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1082,  1083,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   665,   666,    -1,    -1,   669,   670,
      -1,   672,   673,   674,    -1,   676,   677,    -1,   679,   680,
     681,   682,   683,   684,   685,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1143,  1144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   727,   728,   729,    -1,
     731,   732,   733,   734,   735,   736,   737,   738,    -1,    -1,
     741,   742,   743,   744,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   752,   753,   754,    -1,  1191,    -1,    -1,    -1,    -1,
    1196,    -1,    -1,    -1,  1200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   798,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   864,    -1,    -1,    -1,    -1,   869,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   879,   880,
      -1,    -1,    -1,    -1,   885,    -1,    -1,    -1,   889,    -1,
     891,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   914,   915,    -1,    -1,    -1,    -1,    -1,
     921,    -1,    -1,   924,    -1,    -1,   927,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   943,    -1,    -1,    -1,   947,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   977,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1034,  1035,  1036,    -1,  1038,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1053,  1054,  1055,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1067,    -1,    -1,    -1,
      -1,  1072,    -1,    -1,  1075,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1086,    -1,  1088,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1096,    -1,  1098,  1099,    -1,
      -1,    -1,    -1,    -1,  1105,    -1,    -1,  1108,  1109,     4,
    1111,    -1,    -1,     8,     9,    10,    11,    12,    -1,    -1,
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
       5,    -1,     7,    -1,    -1,    10,    11,    -1,    -1,    -1,
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
      -1,    -1,    -1,     8,    -1,    10,    11,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    14,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,     5,    -1,    -1,    -1,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
      -1,    -1,    -1,     8,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,     4,    -1,    -1,    -1,    -1,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   110,   115,   119,   441,     0,     5,     7,    54,    74,
     440,   440,   442,   442,   441,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   111,   112,   426,
     447,   448,   426,   441,   116,   441,   120,   113,   100,   100,
      99,   430,   430,   441,    11,   118,   431,   440,   122,     8,
     432,    56,    64,    82,   117,   124,   125,   139,   121,   123,
     124,   114,   433,   434,   442,   442,   442,   442,   126,   126,
      23,   136,   441,   100,   102,   437,     8,     9,    11,    12,
      16,    17,    18,   435,   446,   447,   112,   444,   446,   112,
     425,   448,   444,    43,    44,   127,   131,   140,   140,   442,
     441,   441,   441,   100,   102,   429,   445,   429,   441,   429,
     442,   442,    28,    31,    59,    67,   141,   142,   143,   144,
     145,   145,   426,    11,   427,   438,   447,   436,   430,   430,
     441,   130,   426,   130,   442,   442,   442,   442,    70,   146,
     153,   153,   137,   433,   101,    99,   101,   128,   132,   444,
     444,   444,   444,   442,   430,     4,    21,    26,    29,    35,
      36,    38,    40,    41,    47,    48,    50,    57,    72,    76,
      79,    98,   101,   154,   155,   169,   174,   180,   226,   230,
     231,   242,   249,   254,   265,   280,   282,   288,   304,   316,
     335,   343,   101,   100,   100,   102,   439,   441,   430,   100,
     100,   102,   133,   429,   429,   429,   429,    12,   112,   151,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   441,   430,   441,
     430,   441,   441,     8,   430,   430,   430,   152,   147,   426,
     103,   112,   313,   397,   398,   426,   426,   426,   112,   347,
     398,   426,   426,   426,   243,   426,   248,   426,   426,   426,
     426,   179,   426,    11,   112,   428,   448,   449,   426,   139,
     438,   441,   129,   134,   441,   100,   102,   148,   281,   427,
     305,   399,   441,   266,   232,   156,   344,   441,   170,   227,
     175,   100,   100,   255,   336,   317,   100,   289,   441,   441,
     283,   138,   101,    71,   101,   135,   139,   143,   144,   101,
     135,   143,   144,   430,   100,   102,   284,   100,   397,   100,
     102,   267,   100,   102,   233,   100,   102,   157,   100,   100,
     102,   171,   100,   102,   228,   100,   102,   176,   430,   430,
     100,   100,   102,   337,   100,   102,   318,   430,   100,   102,
     290,   284,   101,   441,   442,   430,   149,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   244,   250,   430,   430,
     430,   181,   430,   430,   151,   150,   285,   307,   269,   234,
     158,   346,   172,   229,   177,    27,    30,    42,    51,    55,
      73,    75,    78,    81,   101,   143,   144,   165,   173,   182,
     224,   225,   364,   369,   374,   385,    52,    53,    58,   101,
     143,   144,   165,   173,   182,   224,   225,   364,   369,   379,
     381,   383,   385,   257,   338,   319,    73,   101,   143,   144,
     164,   182,   224,   225,   291,   429,   101,   143,   144,   101,
     143,   144,   165,   173,   364,   369,   374,   385,    25,    96,
     143,   144,   164,   173,   230,   274,   306,   314,   335,   364,
     143,   144,   165,   173,   225,   231,   242,   249,   254,   265,
     268,   272,   273,   274,   280,   282,   364,   369,   374,    65,
     101,   143,   144,   165,   173,   180,   226,   230,   314,   335,
     364,   369,   378,   385,    20,   101,   143,   144,   159,   164,
      37,   143,   144,   345,   101,   143,   144,   165,   173,   101,
     143,   144,   165,   180,   226,   230,   314,   335,    22,   101,
     143,   144,   165,   173,   178,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   246,   245,   247,   442,   442,   442,
     252,   251,   253,    46,    77,   143,   144,   165,   173,   180,
     226,   230,   256,   314,   335,   364,   369,   379,   381,   383,
     385,   388,   391,   101,   143,   144,   165,   173,   180,   226,
     230,   385,    45,    60,   101,   143,   144,   165,   173,   180,
     226,   324,   332,   442,   430,    21,    68,   101,   143,   144,
     165,   173,   295,   300,   364,   286,   287,   442,   442,   309,
     312,   101,   310,   311,   308,   101,   271,   270,   442,   240,
     237,   241,   238,   239,   235,   236,   442,   442,   101,   430,
     430,   430,   430,   430,   442,   370,   444,   444,   375,   444,
     166,   183,   428,   444,   444,   430,   430,   430,    17,    94,
     112,   382,   421,    18,   112,   380,   421,   422,    92,    95,
     112,   384,   430,   430,   430,   442,   442,   260,   261,   264,
     101,   262,   263,   258,   259,   340,   341,   342,   339,   441,
     441,   320,   321,   322,   323,    85,    87,    90,   112,   168,
     442,   442,   293,   294,   292,   430,   430,   426,   426,   430,
     430,   430,   430,   430,   430,   430,   444,   430,   430,   430,
     430,   430,   430,   430,   426,    84,    86,    89,    91,   348,
     349,   350,   351,   355,   359,   428,    88,    93,   112,   372,
     429,   429,    88,    93,   112,   377,   386,   168,   100,   102,
     184,   429,   365,   441,   429,   441,   429,   441,   441,   441,
     429,   112,   392,   427,   112,   389,   400,   427,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   325,
     333,   430,   430,   430,   430,   441,   441,   441,   429,   112,
     303,   400,   112,   298,   400,   430,   430,   430,   275,   315,
     429,   160,   441,   441,   441,   441,   441,   429,   441,   441,
     371,   441,   441,   376,   100,   102,   387,   167,   430,   100,
     102,   366,   429,   393,   429,   390,   429,   442,   401,   100,
     100,   301,   441,   296,   441,   100,   102,   276,   318,   100,
     102,   161,   352,   356,   429,   360,   429,   429,   430,   429,
     185,   186,   199,   200,   213,   214,   430,   394,   429,   442,
     400,   399,   430,   430,   100,   100,   102,   297,   430,   430,
     430,   100,   102,   353,   100,   102,   357,   100,   102,   362,
     193,   101,    39,    49,    61,    62,    66,    69,   178,   187,
     188,   190,   194,   208,   210,   211,    32,   202,    24,   216,
     367,   427,   390,   327,   327,   430,   430,   277,    83,   162,
     430,   430,   361,   430,    33,    34,   101,   143,   144,   222,
     223,   442,   442,   442,   442,   442,   442,   182,   430,   442,
     430,   442,   215,   101,   143,   144,   395,   180,   226,   230,
     326,   385,   334,   307,   299,   101,   143,   144,   165,   173,
     230,   364,   442,   101,   354,   358,   363,   442,   442,   112,
     189,   421,   191,   444,   112,   402,   403,   404,   195,   444,
     396,   444,    88,    93,   112,   212,   430,   203,   444,   201,
     217,   426,   216,   394,   329,   330,   331,   101,   328,   101,
     302,    27,    51,   101,   143,   144,   173,   225,   368,   373,
     378,   379,   381,   385,   279,   278,    88,    93,   112,   163,
     101,   224,   225,   368,   373,   379,   381,   385,   388,   101,
     224,   225,   385,   388,   101,   182,   224,   225,   368,   373,
     379,   381,   444,   444,   429,   441,   100,   102,   192,   429,
     103,   405,   406,    14,   408,   409,   100,   102,   196,   100,
     102,   209,   441,   441,   429,   100,   102,   204,   202,   100,
     102,   218,   430,   430,   430,   430,   101,   442,   442,   430,
     430,   430,   441,   441,   429,   430,   430,   430,   429,   429,
     430,   430,   427,   407,   103,   410,   430,   430,   430,   430,
     430,   430,   430,   372,   377,   193,   413,   405,   409,   411,
     427,   197,   193,   205,   219,   429,   429,   101,   104,   412,
     413,    97,   101,   143,   144,   198,   222,   223,   101,    80,
     101,   143,   144,   164,   173,   206,    63,   101,   143,   144,
     164,   173,   220,     5,   443,   406,   442,   442,   442,   414,
     427,   444,    16,    17,    18,   112,   207,   423,   424,   112,
     221,   422,   443,   443,   429,   429,   441,   429,   441,   105,
     106,   443,    85,   415,   420,   443,   443,   107,   103,   443,
     443,   108,    14,   416,   417,   443,   418,   103,   417,   427,
     443,   419,   427,   443,   103,   443,   427
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
     263,   257,   264,   257,   266,   265,   267,   268,   267,   269,
     270,   269,   269,   269,   269,   269,   269,   269,   269,   271,
     269,   272,   272,   273,   273,   273,   273,   273,   273,   273,
     275,   274,   276,   276,   277,   278,   277,   277,   277,   277,
     277,   279,   277,   281,   280,   283,   282,   284,   284,   285,
     286,   285,   285,   287,   285,   285,   285,   285,   285,   285,
     289,   288,   290,   290,   291,   292,   291,   291,   291,   291,
     291,   293,   291,   294,   291,   296,   295,   297,   297,   298,
     298,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   301,   302,   300,   303,   303,   305,   306,   304,
     307,   308,   307,   307,   307,   307,   307,   309,   307,   310,
     307,   311,   307,   312,   307,   313,   313,   315,   314,   317,
     316,   318,   318,   319,   319,   319,   319,   319,   320,   319,
     321,   319,   322,   319,   323,   319,   325,   326,   324,   327,
     328,   327,   329,   327,   330,   327,   331,   327,   333,   334,
     332,   336,   335,   337,   337,   338,   339,   338,   338,   338,
     338,   338,   340,   338,   341,   338,   342,   338,   344,   345,
     343,   346,   346,   346,   346,   347,   347,   348,   348,   349,
     349,   349,   350,   352,   351,   353,   353,   354,   354,   354,
     354,   354,   354,   354,   354,   354,   356,   355,   357,   357,
     358,   358,   358,   358,   358,   360,   359,   361,   362,   362,
     363,   363,   363,   363,   363,   363,   363,   363,   365,   364,
     366,   366,   367,   367,   367,   368,   370,   371,   369,   372,
     372,   372,   373,   375,   376,   374,   377,   377,   377,   378,
     379,   380,   380,   381,   382,   382,   382,   383,   384,   384,
     384,   386,   385,   387,   387,   388,   389,   389,   390,   390,
     391,   393,   392,   392,   395,   394,   394,   396,   397,   398,
     399,   399,   401,   400,   403,   402,   404,   402,   402,   405,
     406,   407,   407,   408,   409,   410,   410,   411,   412,   412,
     413,   413,   414,   415,   416,   417,   418,   418,   419,   419,
     420,   421,   422,   422,   423,   423,   424,   424,   425,   425,
     426,   426,   427,   427,   428,   428,   428,   429,   429,   430,
     430,   430,   432,   431,   433,   434,   434,   435,   435,   435,
     436,   436,   437,   437,   438,   438,   439,   439,   440,   440,
     441,   441,   442,   443,   443,   445,   444,   444,   446,   446,
     446,   446,   446,   446,   446,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   448,   449
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
       0,     4,     0,     4,     0,     5,     1,     0,     5,     0,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     0,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     1,     4,     0,     0,     4,     2,     2,     2,
       2,     0,     4,     0,     5,     0,     5,     1,     4,     0,
       0,     4,     2,     0,     4,     2,     2,     2,     2,     2,
       0,     5,     1,     4,     0,     0,     4,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     5,     1,     4,     2,
       1,     0,     3,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     0,     0,     9,     2,     1,     0,     0,     9,
       0,     0,     4,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     2,     1,     0,     5,     0,
       5,     1,     4,     0,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     0,     8,     0,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     0,
       8,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     4,     0,     0,
       9,     0,     2,     2,     4,     2,     1,     1,     2,     1,
       1,     1,     3,     0,     4,     1,     4,     0,     2,     3,
       2,     2,     2,     2,     2,     2,     0,     4,     1,     4,
       0,     2,     3,     2,     2,     0,     4,     1,     1,     4,
       0,     3,     2,     2,     2,     2,     2,     2,     0,     5,
       1,     4,     0,     2,     2,     4,     0,     0,     6,     2,
       2,     1,     4,     0,     0,     6,     2,     2,     1,     4,
       4,     2,     1,     4,     2,     2,     1,     4,     2,     2,
       1,     0,     5,     1,     4,     3,     2,     2,     3,     1,
       3,     0,     3,     2,     0,     4,     1,     1,     2,     2,
       0,     2,     0,     3,     0,     2,     0,     2,     1,     3,
       2,     0,     2,     3,     2,     0,     2,     2,     0,     2,
       0,     6,     5,     5,     5,     4,     0,     2,     0,     5,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     1,     1,     1,     2,     2,     1,     2,     4,     0,
       2,     2,     0,     4,     2,     0,     1,     0,     2,     3,
       0,     5,     1,     4,     0,     3,     2,     5,     1,     1,
       0,     2,     2,     0,     1,     0,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
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

    case 269: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

        break;

    case 346: /* deviation_opt_stmt  */

      { if (read_all) {
                ly_set_free(((*yyvaluep).nodes).deviation->dflt_check);
                free(((*yyvaluep).nodes).deviation);
              }
            }

        break;

    case 361: /* deviate_replace_stmtsep  */

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
struct lys_node_uses *refine_parent;
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
                        s = malloc(yyget_leng(scanner) - 1 + 7 * yylval.i);
                        if (!s) {
                          LOGMEM;
                          YYABORT;
                        }
                        if (yang_read_string(yyget_text(scanner) + 1, s, yyget_leng(scanner) - 2, yylloc.first_column, (trg) ? trg->version : 0)) {
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
                      (yyval.p_str) = &s;
                    } else {
                      (yyval.p_str) = NULL;
                    }
                  }

    break;

  case 7:

    { if (read_string){
                int length = yyget_leng(scanner) - 1 + strlen(s);
                char *tmp;

                tmp = realloc(s, length);
                if (!tmp) {
                  LOGMEM;
                  YYABORT;
                }
                s = tmp;
                if (yyget_text(scanner)[0] == '"') {
                  if (yang_read_string(yyget_text(scanner) + 1, s + strlen(s), yyget_leng(scanner) - 2, yylloc.first_column, (trg) ? trg->version : 0)) {
                    YYABORT;
                  }
                } else {
                  memcpy(s + strlen(s), yyget_text(scanner) + 1, yyget_leng(scanner) - 2);
                  s[length - 1] = '\0';
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

  case 224:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
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
                    if ((yyvsp[-1].nodes).node.ptr_leaf->dflt && ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE)) {
                      /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                      LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                      LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                      YYABORT;
                    }
                    if (unres_schema_add_node(trg, unres, &(yyvsp[-1].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *)(yyvsp[-1].nodes).node.ptr_leaf) == -1) {
                      lydict_remove(trg->ctx, ((struct yang_type *)(yyvsp[-1].nodes).node.ptr_leaf->type.der)->name);
                      free((yyvsp[-1].nodes).node.ptr_leaf->type.der);
                      (yyvsp[-1].nodes).node.ptr_leaf->type.der = NULL;
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
                          if ((yyvsp[-1].nodes).node.ptr_leaflist->max && (yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max) {
                            LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                            YYABORT;
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

  case 337:

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
                              /* check XPath dependencies */
                              if ((yyvsp[0].nodes).choice.ptr_choice->when &&
                                  (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).choice.ptr_choice, UNRES_XPATH, NULL) == -1)) {
                                YYABORT;
                              }
                            }
                          }

    break;

  case 339:

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

  case 340:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 341:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 342:

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

  case 343:

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

  case 344:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 345:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 346:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 347:

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

  case 348:

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

  case 349:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 350:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 359:

    { if (read_all && trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 360:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 363:

    { if (read_all) {
              /* check XPath dependencies */
              if ((yyvsp[-1].nodes).cs->when &&
                  (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).cs, UNRES_XPATH, NULL) == -1)) {
                YYABORT;
              }
            }
          }

    break;

  case 364:

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

  case 365:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 367:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 368:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 369:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 370:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 371:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 373:

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

  case 375:

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

  case 378:

    { if (read_all) {
             /* check XPath dependencies */
             if (((yyvsp[-1].nodes).node.ptr_anydata->when || (yyvsp[-1].nodes).node.ptr_anydata->must_size) &&
                 (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).node.ptr_anydata, UNRES_XPATH, NULL) == -1)) {
               YYABORT;
             }
           }
         }

    break;

  case 379:

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

  case 380:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                   actual_type = (yyvsp[-1].nodes).node.flag;
                                 }
                               }

    break;

  case 382:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, unres, (yyvsp[-1].nodes).node.flag)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 383:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                   actual_type = (yyvsp[-1].nodes).node.flag;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 385:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config",
                                                               (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 386:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory",
                                                                  (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 387:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status",
                                                               (actual_type == ANYXML_KEYWORD) ? "anyxml" : "anydata", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 388:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 389:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 390:

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

  case 391:

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

  case 394:

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

  case 395:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 397:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 398:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 399:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 400:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 401:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 403:

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

  case 405:

    { if (read_all) {
                                                   refine_parent = actual;
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 411:

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

  case 412:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 413:

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

    { if (read_all) {
                                            if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                              YYABORT;
                                            }
                                            /* check XPath dependencies */
                                            if ((yyvsp[0].nodes).node.ptr_augment->when &&
                                                (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_augment, UNRES_XPATH, NULL) == -1)) {
                                              YYABORT;
                                            }
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
                                       /* check XPath dependencies */
                                       if ((yyvsp[0].nodes).node.ptr_augment->when &&
                                           (unres_schema_add_node(trg, unres, (yyvsp[0].nodes).node.ptr_augment, UNRES_XPATH, NULL) == -1)) {
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
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "notification");
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 442:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 443:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 444:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 447:

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

  case 448:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 449:

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

  case 450:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 453:

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

  case 454:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 455:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 456:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 457:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 458:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 460:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 462:

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

  case 463:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 464:

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

  case 465:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 466:

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

  case 467:

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

  case 469:

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

  case 470:

    { if (read_all) {
                                         actual = (yyvsp[-1].nodes).node.ptr_inout;
                                         actual_type = INPUT_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                       }
                                     }

    break;

  case 472:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 474:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 476:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 477:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 478:

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

  case 479:

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

  case 481:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, actual, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 482:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 483:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 484:

    { if (read_all) {
              /* check XPath dependencies */
              if ((yyvsp[-1].nodes).notif->must_size &&
                  (unres_schema_add_node(trg, unres, (yyvsp[-1].nodes).notif, UNRES_XPATH, NULL) == -1)) {
                YYABORT;
              }
            }
          }

    break;

  case 485:

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
                                  if (store_flags((struct lys_node *)(yyval.nodes).notif, size_arrays->node[size_arrays->next].flags, config_inherit)) {
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

  case 486:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).notif;
                                      actual_type = NOTIFICATION_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 488:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 489:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 490:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 491:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 492:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 494:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 496:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 498:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 499:

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

  case 501:

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

  case 502:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 503:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           ly_set_free((yyvsp[-1].nodes).deviation->dflt_check);
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 504:

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

  case 507:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 513:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 516:

    { if (read_all) {
             /* check XPath dependencies */
             if (((yyvsp[-1].nodes).deviation->trg_must_size && *(yyvsp[-1].nodes).deviation->trg_must_size) &&
                 unres_schema_add_node((yyvsp[-1].nodes).deviation->target->module, unres, (yyvsp[-1].nodes).deviation->target, UNRES_XPATH, NULL)) {
               YYABORT;
             }
           }
         }

    break;

  case 517:

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

  case 518:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 519:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 520:

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

  case 521:

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

  case 522:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 523:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 524:

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

  case 525:

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

  case 526:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 529:

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
                  unres_schema_add_node((yyvsp[-1].nodes).deviation->target->module, unres, (yyvsp[-1].nodes).deviation->target, UNRES_XPATH, NULL)) {
                YYABORT;
              }
            }
          }

    break;

  case 530:

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

  case 531:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 532:

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

  case 533:

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

  case 534:

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

  case 535:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 537:

    { if (read_all) {
                                     (yyval.nodes).deviation = actual;
                                   }
                                 }

    break;

  case 539:

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

  case 540:

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

  case 541:

    { if (read_all) {
                                                    ly_set_add((yyvsp[-2].nodes).deviation->dflt_check, (yyvsp[-2].nodes).deviation->target, 0);
                                                  }
                                                }

    break;

  case 542:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 543:

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

  case 544:

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 545:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 546:

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

  case 547:

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

  case 548:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 553:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 554:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 555:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 556:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 557:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 558:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 559:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 560:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 561:

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

  case 562:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 563:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 564:

    { read_string = (read_string) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 565:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 566:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 567:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 568:

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

  case 570:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 571:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 572:

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

  case 573:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 574:

    { (yyval.uint) = 0; }

    break;

  case 575:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 576:

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

  case 577:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 578:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 579:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 580:

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

  case 581:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 591:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 594:

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

  case 597:

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

  case 598:

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

  case 602:

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

  case 606:

    { tmp_s = yyget_text(scanner); }

    break;

  case 607:

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

  case 631:

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

  case 632:

    { (yyval.uint) = 0; }

    break;

  case 633:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 634:

    { (yyval.i) = 0; }

    break;

  case 635:

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

  case 641:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 646:

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

  case 652:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 675:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 766:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 767:

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
