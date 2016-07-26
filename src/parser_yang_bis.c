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
#define YYLAST   2921

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  339
/* YYNRULES -- Number of rules.  */
#define YYNRULES  738
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1198

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
       0,   245,   245,   246,   267,   267,   286,   288,   287,   319,
     332,   319,   341,   342,   350,   351,   354,   367,   354,   378,
     379,   385,   385,   394,   396,   398,   421,   422,   425,   433,
     425,   450,   452,   452,   474,   475,   479,   480,   482,   494,
     505,   494,   514,   516,   517,   518,   519,   524,   530,   532,
     534,   536,   538,   546,   549,   549,   558,   559,   559,   564,
     565,   570,   577,   577,   586,   592,   633,   636,   637,   638,
     639,   640,   641,   642,   648,   649,   650,   652,   652,   671,
     672,   676,   677,   684,   691,   702,   714,   714,   716,   717,
     721,   723,   724,   725,   736,   738,   739,   738,   742,   743,
     744,   745,   762,   762,   771,   772,   777,   796,   809,   815,
     820,   826,   828,   828,   837,   839,   838,   846,   847,   858,
     864,   870,   877,   881,   879,   896,   906,   906,   913,   916,
     916,   931,   932,   938,   944,   949,   955,   956,   962,   967,
     968,   969,   976,   984,   989,   993,  1023,  1024,  1028,  1029,
    1030,  1038,  1051,  1057,  1058,  1076,  1080,  1090,  1091,  1096,
    1111,  1116,  1121,  1126,  1132,  1136,  1146,  1147,  1152,  1152,
    1170,  1171,  1174,  1186,  1196,  1197,  1202,  1203,  1212,  1218,
    1223,  1229,  1241,  1242,  1260,  1265,  1266,  1271,  1273,  1275,
    1279,  1283,  1298,  1298,  1316,  1317,  1319,  1330,  1340,  1341,
    1346,  1347,  1356,  1362,  1367,  1373,  1385,  1386,  1401,  1403,
    1405,  1407,  1409,  1409,  1416,  1417,  1422,  1442,  1448,  1453,
    1458,  1459,  1466,  1467,  1470,  1471,  1472,  1473,  1474,  1475,
    1476,  1479,  1479,  1487,  1488,  1493,  1527,  1527,  1529,  1536,
    1536,  1544,  1545,  1551,  1557,  1562,  1567,  1567,  1572,  1572,
    1577,  1577,  1585,  1585,  1592,  1599,  1592,  1617,  1645,  1645,
    1647,  1654,  1659,  1654,  1669,  1670,  1670,  1678,  1681,  1687,
    1693,  1699,  1704,  1710,  1717,  1710,  1736,  1764,  1764,  1766,
    1773,  1778,  1773,  1788,  1789,  1789,  1797,  1803,  1817,  1831,
    1843,  1849,  1854,  1860,  1867,  1860,  1892,  1935,  1935,  1937,
    1944,  1944,  1952,  1965,  1973,  1979,  1993,  2007,  2019,  2025,
    2030,  2035,  2035,  2043,  2043,  2048,  2048,  2053,  2053,  2061,
    2061,  2072,  2074,  2073,  2091,  2112,  2112,  2114,  2127,  2139,
    2147,  2155,  2163,  2172,  2181,  2181,  2191,  2192,  2195,  2196,
    2197,  2198,  2199,  2202,  2202,  2210,  2211,  2215,  2235,  2235,
    2237,  2244,  2250,  2255,  2260,  2260,  2267,  2267,  2278,  2279,
    2283,  2310,  2310,  2312,  2319,  2319,  2327,  2333,  2339,  2345,
    2350,  2356,  2356,  2372,  2373,  2377,  2412,  2412,  2414,  2421,
    2427,  2432,  2437,  2437,  2445,  2445,  2460,  2460,  2469,  2470,
    2475,  2476,  2479,  2499,  2506,  2530,  2554,  2573,  2592,  2615,
    2638,  2643,  2649,  2658,  2649,  2665,  2666,  2669,  2678,  2669,
    2690,  2711,  2711,  2713,  2720,  2729,  2734,  2739,  2739,  2747,
    2747,  2755,  2755,  2765,  2766,  2769,  2769,  2780,  2780,  2791,
    2792,  2797,  2825,  2832,  2838,  2843,  2848,  2848,  2856,  2856,
    2861,  2861,  2873,  2873,  2886,  2900,  2886,  2907,  2927,  2927,
    2935,  2935,  2940,  2940,  2950,  2964,  2950,  2971,  2971,  2981,
    2985,  2990,  3017,  3024,  3030,  3035,  3040,  3040,  3048,  3048,
    3053,  3053,  3060,  3069,  3060,  3082,  3101,  3108,  3115,  3125,
    3126,  3128,  3133,  3135,  3136,  3137,  3140,  3142,  3142,  3148,
    3149,  3153,  3175,  3183,  3191,  3207,  3215,  3222,  3229,  3240,
    3252,  3252,  3258,  3259,  3263,  3285,  3293,  3304,  3314,  3323,
    3323,  3329,  3330,  3334,  3339,  3345,  3353,  3361,  3368,  3375,
    3386,  3398,  3398,  3401,  3402,  3406,  3407,  3412,  3418,  3420,
    3421,  3420,  3424,  3425,  3426,  3441,  3443,  3444,  3443,  3447,
    3448,  3449,  3464,  3466,  3468,  3469,  3490,  3492,  3493,  3494,
    3515,  3517,  3518,  3519,  3531,  3531,  3539,  3540,  3545,  3547,
    3548,  3550,  3551,  3553,  3555,  3555,  3564,  3567,  3567,  3578,
    3581,  3591,  3612,  3614,  3615,  3618,  3618,  3637,  3637,  3646,
    3646,  3655,  3658,  3660,  3662,  3663,  3665,  3667,  3669,  3670,
    3672,  3674,  3675,  3677,  3678,  3680,  3682,  3685,  3689,  3691,
    3692,  3694,  3695,  3697,  3699,  3710,  3711,  3714,  3715,  3727,
    3728,  3730,  3731,  3733,  3734,  3740,  3741,  3744,  3745,  3746,
    3772,  3773,  3776,  3777,  3778,  3781,  3781,  3789,  3791,  3792,
    3794,  3795,  3796,  3798,  3799,  3801,  3802,  3804,  3805,  3807,
    3808,  3810,  3811,  3814,  3815,  3818,  3820,  3821,  3824,  3824,
    3833,  3835,  3836,  3837,  3838,  3839,  3840,  3841,  3843,  3844,
    3845,  3846,  3847,  3848,  3849,  3850,  3851,  3852,  3853,  3854,
    3855,  3856,  3857,  3858,  3859,  3860,  3861,  3862,  3863,  3864,
    3865,  3866,  3867,  3868,  3869,  3870,  3871,  3872,  3873,  3874,
    3875,  3876,  3877,  3878,  3879,  3880,  3881,  3882,  3883,  3884,
    3885,  3886,  3887,  3888,  3889,  3890,  3891,  3892,  3893,  3894,
    3895,  3896,  3897,  3898,  3899,  3900,  3901,  3902,  3903,  3904,
    3905,  3906,  3907,  3908,  3909,  3910,  3911,  3912,  3913,  3914,
    3915,  3916,  3917,  3918,  3919,  3920,  3921,  3924,  3933
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
  "$@8", "$@9", "tmp_identifier_arg_str", "include_stmt", "$@10",
  "include_end", "revision_date_opt", "revision_date_stmt",
  "belongs_to_stmt", "$@11", "$@12", "prefix_stmt", "meta_stmts",
  "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_stmt", "$@13",
  "revision_end", "$@14", "revision_opt_stmt", "date_arg_str", "$@15",
  "body_stmts", "body_stmt", "extension_stmt", "$@16", "extension_end",
  "extension_opt_stmt", "argument_stmt", "$@17", "argument_end",
  "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@18", "$@19", "status_arg_str", "feature_stmt",
  "$@20", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@21", "identity_end", "$@22", "identity_opt_stmt",
  "base_stmt", "typedef_stmt", "$@23", "typedef_arg_str", "type_stmt",
  "$@24", "type_opt_stmt", "$@25", "type_end", "type_stmtsep",
  "type_body_stmts", "decimal_string_restrictions", "union_spec",
  "fraction_digits_stmt", "fraction_digits_arg_str", "length_stmt",
  "length_arg_str", "length_end", "message_opt_stmt", "pattern_stmt",
  "pattern_arg_str", "pattern_end", "enum_specification", "$@26",
  "enum_stmts", "enum_stmt", "enum_arg_str", "enum_end", "enum_opt_stmt",
  "value_stmt", "integer_value_arg_str", "range_stmt", "range_end",
  "path_stmt", "require_instance_stmt", "require_instance_arg_str",
  "bits_specification", "$@27", "bit_stmts", "bit_stmt", "bit_arg_str",
  "bit_end", "bit_opt_stmt", "position_stmt", "position_value_arg_str",
  "error_message_stmt", "error_app_tag_stmt", "units_stmt", "default_stmt",
  "grouping_stmt", "$@28", "grouping_end", "grouping_opt_stmt",
  "data_def_stmt", "container_stmt", "$@29", "container_end",
  "container_opt_stmt", "$@30", "$@31", "$@32", "$@33", "$@34", "$@35",
  "leaf_stmt", "$@36", "$@37", "leaf_opt_stmt", "$@38", "$@39", "$@40",
  "$@41", "leaf_list_stmt", "$@42", "$@43", "leaf_list_opt_stmt", "$@44",
  "$@45", "$@46", "$@47", "list_stmt", "$@48", "$@49", "list_opt_stmt",
  "$@50", "$@51", "$@52", "$@53", "$@54", "$@55", "choice_stmt", "$@56",
  "choice_end", "$@57", "choice_opt_stmt", "$@58", "$@59",
  "short_case_case_stmt", "short_case_stmt", "case_stmt", "$@60",
  "case_end", "case_opt_stmt", "$@61", "$@62", "anyxml_stmt", "$@63",
  "anyxml_end", "anyxml_opt_stmt", "$@64", "$@65", "uses_stmt", "$@66",
  "uses_end", "uses_opt_stmt", "$@67", "$@68", "$@69", "refine_stmt",
  "$@70", "refine_end", "refine_arg_str", "refine_body_opt_stmts",
  "uses_augment_stmt", "$@71", "$@72", "uses_augment_arg_str",
  "augment_stmt", "$@73", "$@74", "augment_opt_stmt", "$@75", "$@76",
  "$@77", "$@78", "augment_arg_str", "action_stmt", "$@79", "rpc_stmt",
  "$@80", "rpc_end", "rpc_opt_stmt", "$@81", "$@82", "$@83", "$@84",
  "input_stmt", "$@85", "$@86", "input_output_opt_stmt", "$@87", "$@88",
  "$@89", "output_stmt", "$@90", "$@91", "notification_stmt", "$@92",
  "notification_end", "notification_opt_stmt", "$@93", "$@94", "$@95",
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
     345,   346,   347,   348,   349,   350,   351,    43,   123,   125,
      59,    47,    91,    93,    61,    40,    41
};
# endif

#define YYPACT_NINF -693

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-693)))

#define YYTABLE_NINF -594

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -693,    55,  -693,  -693,    72,  -693,  -693,  -693,    86,    86,
    -693,  -693,  1592,  1592,    86,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,   -36,    86,   -18,    86,  -693,  -693,   -14,   178,   178,
    -693,  -693,   -26,  -693,  -693,    -1,   245,    86,    86,    86,
      86,  -693,  -693,  -693,  -693,  -693,   104,  -693,  -693,    19,
    1810,  -693,  2174,  1592,  2174,   -16,   -16,    86,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,   112,  -693,  -693,   112,  -693,   112,
      86,    86,  -693,  -693,    29,    29,  1592,    86,  2267,    86,
    -693,  -693,  -693,  -693,  -693,    86,  -693,  1592,  1592,    86,
      86,    86,    86,  -693,  -693,  -693,  -693,    62,    62,  -693,
    -693,    86,    36,  -693,    41,    26,   178,    86,  -693,  -693,
    -693,  2174,  2174,  2174,  2174,    86,  -693,  1191,  1272,    63,
     201,  -693,  -693,  -693,    75,   262,   112,   112,   112,   112,
     182,   178,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,   414,   178,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  1592,     8,  1592,  1592,  1592,
       8,  1592,  1592,  1592,  1592,  1592,  1592,  1592,  1592,  1592,
    2360,    86,   178,    86,    79,  2267,    86,  -693,    79,    71,
     178,  -693,   274,  -693,  2825,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,    82,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,    92,    86,  -693,    86,   137,  -693,    86,  -693,  -693,
    -693,   350,  -693,   113,   142,    86,   355,   396,   397,   153,
      86,   402,   405,   460,   171,   183,   195,   461,   465,  -693,
     466,    86,    86,   208,  -693,   229,   182,  -693,   178,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,   178,  -693,  -693,  -693,  -693,    86,   214,   112,
    -693,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   434,   178,   178,  -693,  -693,
     172,  1222,  1911,   667,  1126,   235,   167,   237,   236,   292,
    1510,  1213,  1478,   726,   371,    86,    86,    86,  -693,  -693,
    -693,   217,   246,  -693,  -693,   347,   178,  -693,  -693,  -693,
      86,    86,    86,    86,    86,    86,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,    86,    86,  -693,  -693,  -693,
    -693,  -693,  -693,   230,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,   240,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,    86,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,    86,  -693,  -693,
    -693,  -693,  -693,    86,  -693,  -693,   248,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
      86,  -693,  -693,  -693,   278,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,   300,   246,  -693,  -693,  -693,  -693,    86,    86,
      86,  -693,  -693,  -693,  -693,  -693,   307,   246,  -693,  -693,
    -693,  -693,  -693,  -693,    86,    86,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,   319,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  2174,   115,  2174,  -693,    86,  -693,    86,    86,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  2360,
    -693,  2174,  -693,  2174,  -693,  -693,  1592,  1592,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  2174,  -693,  -693,  -693,
    -693,  -693,  -693,  1592,   321,  -693,   178,   178,   178,   178,
    2360,  -693,  -693,  -693,  -693,  -693,    50,    27,    39,  -693,
    -693,  -693,  -693,  2453,  2453,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,    86,    86,  -693,  -693,  -693,
    -693,   112,  -693,  -693,  -693,  -693,   112,   112,  2360,   178,
    2453,  2453,  -693,  -693,  -693,    60,   112,    81,  -693,   115,
    -693,   178,   178,  -693,  -693,   178,   178,   178,   178,   178,
     178,   112,   178,   178,   178,   178,   178,   178,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,   112,
    -693,   178,   178,  -693,  -693,  -693,   112,  -693,  -693,  -693,
     112,  -693,  -693,  -693,  -693,  -693,   112,  -693,   178,   178,
     112,  -693,  -693,   112,  -693,    54,  -693,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   326,   332,   178,   178,
     178,   178,  -693,    86,    86,    86,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,   178,   178,   178,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,   523,  -693,   529,
     534,   465,  -693,   537,    86,    86,    54,    86,    86,  -693,
     178,    86,  -693,    86,  -693,    86,    86,    86,  -693,   178,
    -693,    54,  -693,  -693,  -693,  2825,  -693,  -693,  -693,   538,
     338,    86,   544,    86,    86,    86,   112,    86,    86,   112,
    -693,  -693,  -693,   112,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,   565,   575,  -693,   576,  -693,  -693,
    2825,    54,   142,   178,   178,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,   178,  -693,   178,   178,    64,   178,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,   593,   593,   258,   178,   178,   178,   239,   257,  1422,
      86,   341,   178,   178,   178,    54,  -693,  -693,  -693,   346,
     363,    86,    86,  -693,   364,   133,  -693,   417,  -693,  -693,
    -693,   430,  1911,   318,    86,    86,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,    91,  -693,   374,   219,   756,  -693,  -693,  -693,  -693,
    -693,  -693,    94,    93,  -693,    86,    86,    86,    86,   246,
    -693,  -693,  -693,  -693,    86,  -693,    86,  -693,   367,    86,
      86,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  2174,  2174,  -693,  -693,  -693,  -693,  -693,   112,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,   178,   178,   178,  -693,   112,   366,   464,  -693,  -693,
    -693,   112,    77,  2174,  2174,  2174,  -693,   178,  2174,   178,
    1592,   430,  -693,    60,    81,   178,   112,   112,   178,   178,
      86,    86,  -693,   178,   178,   178,  -693,  2825,  -693,  -693,
     409,  -693,  -693,    86,    86,  -693,  -693,   112,  -693,   579,
    -693,   585,  -693,   586,  -693,   178,   595,  -693,   417,   601,
    -693,  -693,   112,   112,  -693,  -693,  -693,   366,  -693,  2546,
    -693,    86,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
     404,  -693,  -693,  -693,     9,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   516,  -693,   176,   309,   333,   546,
     264,   484,  -693,  2825,  -693,  -693,  -693,  -693,    86,  -693,
    -693,  -693,  -693,  -693,    86,  -693,  -693,  -693,  -693,  -693,
     516,   516,   199,    27,   422,   442,  -693,  -693,  -693,  -693,
     112,  -693,  -693,  -693,   112,  -693,  -693,   516,  -693,    86,
    -693,    86,   468,   516,  -693,   516,   444,   467,   516,   516,
     463,   559,  -693,   516,  -693,  -693,   473,  2639,   516,  -693,
    -693,  -693,  2732,  -693,   477,   516,  2825,  -693
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     643,     0,     2,     3,     0,     1,   641,   642,     0,     0,
     644,   643,     0,     0,   645,   659,     4,   658,   660,   661,
     662,   663,   664,   665,   666,   667,   668,   669,   670,   671,
     672,   673,   674,   675,   676,   677,   678,   679,   680,   681,
     682,   683,   684,   685,   686,   687,   688,   689,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   699,   700,   701,
     702,   703,   704,   705,   706,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   717,   718,   719,   720,   721,
     722,   723,   724,   725,   726,   727,   728,   729,   730,   731,
     732,   733,   734,   735,   736,   614,     9,   737,   643,    16,
     643,     0,   613,     0,     6,   622,   622,     5,    12,    19,
     643,   625,    10,   624,   623,    17,     0,   628,     0,     0,
       0,    25,    13,    14,    15,    25,     0,    20,     7,     0,
     630,   629,     0,     0,     0,    43,    43,     0,    22,   643,
     643,   635,   626,   643,   651,   654,   652,   656,   657,   655,
     627,   631,   653,   650,     0,   648,   611,     0,   643,     0,
       0,     0,    26,    27,    52,    52,     0,     8,   637,   633,
     622,   622,    24,   643,    42,   612,    23,     0,     0,     0,
       0,     0,     0,    44,    45,    46,    47,    65,    65,    39,
     616,   628,     0,   615,   632,     0,   620,   649,    28,    31,
      32,     0,     0,     0,     0,     0,   622,     0,     0,     0,
       0,   636,   643,   622,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   643,   622,
      67,    68,    69,    70,    71,    72,   224,   225,   226,   227,
     228,   229,   230,    73,    74,    75,    76,   643,   622,   643,
     643,   638,     0,   621,   622,   622,    34,   622,    49,    50,
      48,    51,    62,    64,    54,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    66,    18,     0,   637,   639,   643,     0,    36,
      33,   643,     0,   356,     0,   424,   407,   573,   643,   319,
     231,    77,   480,   472,   643,   102,   212,   112,   254,   273,
     293,   457,   427,     0,   125,   738,   619,   371,   643,   643,
      40,     0,   634,    29,     0,     0,    37,    63,   622,    56,
      55,     0,   571,     0,   572,   423,     0,     0,     0,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,   622,
       0,   617,   618,     0,   643,    36,     0,    35,    57,   622,
     358,   357,   622,   574,   622,   321,   320,   622,   233,   232,
     622,    79,    78,   622,   622,   104,   103,   622,   214,   213,
     622,   114,   113,   622,   622,   622,   622,   459,   458,   622,
     429,   428,   128,   622,   373,   372,   622,   640,     0,     0,
      59,   360,   410,   324,   235,    81,   475,   106,   216,   117,
     257,   276,   296,   461,   431,   123,   375,    41,   622,    38,
       0,     0,   408,   322,     0,     0,   473,     0,     0,   115,
     255,   274,   294,     0,     0,     0,     0,     0,   134,   135,
     133,     0,     0,   131,   132,     0,    30,    58,    60,    61,
       0,     0,     0,     0,     0,     0,   359,   369,   370,   368,
     363,   361,   366,   367,   364,     0,     0,   415,   416,   414,
     413,   417,   421,     0,   419,   411,   332,   333,   331,   327,
     328,   338,   339,   340,   341,     0,   334,   336,   337,   342,
     325,   329,   330,     0,   234,   244,   245,   243,   238,   250,
     246,   252,   248,   236,   242,   241,   239,     0,    80,    84,
      85,    82,    83,     0,   476,   477,     0,   105,   109,   110,
     108,   107,   215,   218,   219,   217,   622,   622,   622,   622,
       0,   120,   121,   119,     0,   118,   271,   272,   270,   260,
     264,   267,     0,     0,   258,   268,   269,   265,     0,     0,
       0,   291,   292,   290,   279,   283,     0,     0,   277,   286,
     287,   288,   289,   284,     0,     0,   309,   310,   308,   299,
     311,   313,   317,     0,   315,   297,   304,   305,   306,   307,
     300,   303,   302,   460,   464,   465,   463,   462,   466,   468,
     470,   643,   643,   430,   434,   435,   433,   432,   436,   438,
     440,   442,     0,     0,     0,   124,     0,   622,     0,     0,
     374,   380,   381,   379,   378,   382,   384,   376,   529,     0,
     536,     0,    95,     0,   622,   622,     0,     0,   622,   622,
     409,   622,   622,   323,   622,   622,     0,   622,   622,   622,
     622,   622,   622,     0,     0,   474,   221,   220,   222,   223,
       0,   116,   256,   262,   622,   622,     0,     0,     0,   275,
     281,   622,   622,     0,     0,   622,   622,   622,   295,   622,
     622,   622,   622,   622,   622,   444,   454,   622,   622,   622,
     622,     0,   643,   643,   643,   101,     0,     0,     0,   130,
       0,     0,   622,   622,   622,     0,     0,     0,   554,     0,
     521,   362,   365,   343,   425,   418,   422,   420,   412,   335,
     326,     0,   251,   247,   253,   249,   237,   240,    86,   643,
     643,   643,   643,   478,   643,   481,   483,   485,   484,     0,
     622,   259,   266,   604,   643,   549,     0,   643,   605,   545,
       0,   606,   643,   643,   643,   553,     0,   622,   278,   285,
       0,   563,   564,     0,   558,     0,   575,   312,   314,   318,
     316,   298,   301,   467,   469,   471,     0,     0,   437,   439,
     441,   443,   211,    98,   100,    99,    94,   210,   126,   406,
     402,   643,   391,   386,   643,   383,   385,   377,   643,   643,
     534,   530,   111,   643,   643,   541,   537,     0,    96,     0,
       0,     0,   542,     0,   487,   500,     0,   509,   482,   122,
     263,   547,   546,   548,   543,   544,   552,   551,   550,   282,
     566,     0,   560,   559,   562,     0,   573,   622,   622,     0,
       0,   405,     0,   390,   533,   532,     0,   540,   539,     0,
     622,   556,   555,     0,   622,   523,   522,   622,   345,   344,
     426,   622,    88,   622,     0,     0,   486,     0,   565,   569,
       0,     0,   576,   447,   447,   622,   136,   127,   622,   622,
     388,   387,   531,   538,   159,    97,   525,   347,     0,    87,
     622,   489,   488,   622,   502,   501,   622,   511,   510,   567,
     561,   445,   455,   145,   138,   410,   392,     0,     0,     0,
       0,     0,   491,   504,   513,     0,   448,   450,   452,     0,
       0,     0,     0,   142,     0,   139,   140,     0,   141,   143,
     144,     0,   403,     0,     0,     0,   557,   162,   163,   160,
     161,   524,   526,   527,   346,   352,   353,   351,   350,   354,
     348,     0,    89,     0,     0,     0,   568,   622,   622,   622,
     446,   456,   577,     0,   137,     0,     0,     0,     0,     0,
     148,   146,   147,   622,     0,   622,     0,   194,     0,     0,
       0,   389,   400,   401,   395,   396,   397,   394,   398,   399,
     622,     0,     0,   622,   622,   643,   643,    93,     0,   490,
     492,   495,   496,   497,   498,   499,   622,   494,   503,   505,
     508,   622,   507,   512,   622,   515,   516,   517,   518,   519,
     520,   449,   451,   453,   581,     0,     0,     0,   643,   643,
     191,     0,     0,     0,     0,     0,   622,   149,     0,   170,
       0,   193,   404,     0,     0,   393,     0,     0,   355,   349,
      92,    91,    90,   493,   506,   514,   187,     0,   584,   578,
       0,   580,   588,   190,   189,   188,   154,     0,   643,     0,
     156,     0,   165,     0,   570,   150,     0,   173,   169,     0,
     197,   195,     0,     0,   209,   208,   593,   583,   587,     0,
     152,   153,   622,   157,   622,   622,   166,   622,   622,   185,
     184,   622,   174,   172,   622,   622,   198,   622,   528,   535,
     582,   585,   589,   586,   591,   159,   155,   159,   164,   159,
     176,   171,   200,   196,   646,   590,     0,     0,     0,     0,
       0,     0,   647,     0,   592,   158,   167,   186,     0,   175,
     179,   180,   178,   177,     0,   199,   203,   204,   202,   201,
     646,   646,     0,     0,     0,     0,   609,   610,   607,   183,
       0,   643,   608,   207,     0,   643,   594,   646,   181,   182,
     205,   206,     0,   646,   595,   646,     0,     0,   646,   646,
       0,     0,   603,   646,   596,   599,     0,     0,   646,   600,
     601,   598,   646,   597,     0,   646,     0,   602
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -693,  -693,   390,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,   474,  -693,   456,  -693,  -693,
    -693,   412,  -693,  -693,  -693,   231,  -693,  -693,  -693,  -693,
     -34,   475,  -693,  -693,    49,   671,   435,  -693,  -693,  -693,
    -693,  -693,   250,  -693,   413,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -424,  -196,  -693,  -693,   -92,
    -693,  -693,  -693,  -693,  -391,  -693,  -693,  -693,  -693,  -693,
    -283,  -293,  -693,  -693,  -546,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -680,  -693,
    -693,  -693,  -693,  -693,  -693,  -454,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -410,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -427,  -416,  -276,
    -693,  -693,  -693,  -314,   206,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,   213,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,   234,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
     254,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,   255,
    -693,  -693,  -693,  -693,  -693,   267,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -277,  -693,  -693,  -693,  -693,  -693,  -246,  -693,  -693,  -693,
    -147,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -206,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,  -693,
    -693,  -693,  -693,  -368,  -693,  -693,  -693,  -692,    35,  -693,
    -693,  -365,  -371,  -392,  -693,  -693,  -342,  -230,  -413,  -693,
    -403,  -693,   266,  -693,  -422,  -693,  -693,  -603,  -693,  -167,
    -693,  -693,  -693,  -205,  -693,  -693,  -338,   431,  -124,  -675,
    -693,  -693,  -693,  -693,  -367,  -407,  -693,  -693,  -373,  -693,
    -693,  -693,  -393,  -693,  -693,  -693,  -465,  -693,  -693,  -693,
    -657,  -430,  -693,  -693,  -693,   109,  -165,  -606,   163,  1072,
    -693,  -693,   533,  -693,  -693,  -693,  -693,   437,  -693,    -4,
      -3,   425,   590,   151,  -693,   603,   -98,  -131,  -693
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    95,   100,   107,   139,     2,   101,   121,   112,
       3,   103,   125,   115,   126,   122,   123,   135,   162,   214,
     365,   198,   163,   215,   267,   335,   336,   138,   209,   363,
     124,   164,   183,   184,   937,   938,   187,   206,   302,   340,
     410,   430,   274,   301,   207,   239,   240,   348,   382,   435,
     521,   813,   863,   911,   998,   479,   469,   709,   853,   696,
     241,   351,   386,   437,   480,   242,   353,   392,   544,   439,
     545,   243,   451,   323,   617,   839,   425,   452,   877,   903,
     924,   925,   969,   970,  1067,   971,  1069,  1094,   907,   972,
    1071,  1097,   926,   927,  1078,   975,  1076,  1103,  1130,  1143,
    1160,   973,  1100,   928,   929,  1031,   930,   931,  1041,   977,
    1079,  1107,  1131,  1149,  1164,   939,   940,   453,   454,   244,
     352,   389,   438,   245,   246,   347,   379,   434,   651,   652,
     648,   650,   647,   649,   247,   354,   552,   440,   664,   553,
     740,   665,   248,   355,   566,   441,   671,   567,   757,   672,
     249,   356,   583,   442,   680,   681,   675,   676,   679,   677,
     250,   346,   376,   495,   433,   645,   644,   496,   497,   482,
     810,   859,   909,   994,   993,   251,   341,   371,   431,   634,
     635,   252,   360,   405,   455,   704,   702,   703,   625,   842,
     881,   793,   933,   626,   840,   978,   790,   253,   343,   483,
     432,   642,   638,   641,   639,   306,   484,   811,   254,   358,
     401,   444,   687,   688,   689,   690,   610,   776,   919,   901,
     957,   958,   959,   611,   777,   920,   255,   357,   398,   443,
     682,   683,   684,   256,   349,   526,   436,   313,   733,   734,
     735,   736,   864,   892,   953,   737,   865,   895,   954,   738,
     867,   898,   955,   485,   809,   856,   908,   985,   472,   705,
     846,   801,   986,   473,   707,   849,   806,   515,   570,   750,
     571,   746,   572,   756,   474,   807,   852,   591,   764,   833,
     592,   761,   831,   868,   915,  1073,   307,   308,   344,   765,
     836,  1025,  1026,  1027,  1058,  1059,  1087,  1061,  1062,  1089,
    1113,  1125,  1110,  1150,  1174,  1184,  1185,  1187,  1192,  1175,
     751,   752,  1161,  1162,   157,   199,   766,   327,   834,   108,
     113,   117,   129,   130,   150,   194,   142,   192,   261,   114,
       4,   131,  1133,   154,   173,   155,    97,    98,   329
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,   450,   158,   191,    11,    11,   373,   663,    14,   747,
      10,   522,   516,   550,   565,   543,    16,   490,   557,   573,
     590,   670,   -21,   706,   551,   791,   794,   160,   161,   587,
     118,     6,   152,     7,   152,    16,   152,   111,   119,   588,
     470,   502,   489,   508,   743,   748,   531,    16,   556,   549,
     564,   579,   597,   607,   739,     5,   120,   179,    16,     6,
     180,     7,   105,   471,   624,   500,   513,   743,    16,     6,
     193,     7,   554,   568,   585,   111,     6,     6,     7,     7,
     106,   120,   111,   110,     6,    16,     7,   627,   181,    16,
     111,     6,   788,     7,   743,   102,   182,   104,    10,    16,
      10,    16,    16,   152,   152,   152,   152,   116,  -579,   304,
    -593,  -593,    10,    11,    11,    11,    11,   140,   481,   141,
     511,    96,    99,    16,   538,   213,     8,   137,   582,   600,
     191,   753,   205,    11,   754,   211,   167,   168,   212,   342,
     169,   509,   334,   119,   744,   536,     9,   910,   798,   580,
     598,   608,   170,   799,   171,   175,    11,    11,   510,   328,
     871,   258,   537,    10,    10,    10,   581,   599,   609,   803,
     197,    10,   965,   264,   804,    11,    11,    11,    11,   995,
     359,  1028,   966,     6,   996,     7,  1029,    11,   512,   111,
      16,   364,   539,    10,   272,   967,   584,   193,   180,   968,
     692,    11,   693,   180,   523,   694,   193,    16,  -151,   262,
     170,   372,   171,   185,   185,  1156,  1157,  1158,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,   182,   291,   367,   488,   507,   182,
     222,   530,   535,   304,   548,   563,   578,   596,   606,   445,
       6,   383,     7,   128,   293,   517,   295,   296,    10,   623,
     330,  1002,   224,  1017,   333,   225,   180,   180,   180,   393,
     180,   457,   934,   935,   463,   189,   229,  1057,  1124,   461,
     540,   394,  -192,   231,   232,   159,   233,    10,   180,    10,
    -168,    10,    10,   395,   332,   180,   575,   447,   337,   259,
     334,   260,   182,   182,   182,   345,   182,   406,   446,   464,
     464,   350,   236,   428,   540,   237,   615,   172,  1008,   921,
     174,   616,   176,   180,   182,   361,   362,   922,    10,   640,
      11,   182,   476,    10,   518,   532,   527,   446,   936,   643,
     180,    10,   934,   935,  1138,   979,    10,   655,   445,   180,
    1007,  1012,   216,   217,   218,   219,   941,    10,    10,   182,
     265,   407,   266,  1139,   180,   446,   934,   935,   618,   980,
     558,   559,   338,   463,   339,  1068,   182,   661,   180,   268,
     269,   270,   271,   503,   303,   182,   309,   310,   311,   461,
     315,   316,   317,   318,   319,   320,   321,   322,   324,   662,
     182,   979,   180,    10,   445,   729,   669,   730,  1135,  1014,
     731,   229,   732,   461,   182,   619,   601,   981,   678,     6,
     464,     7,   297,  1036,   837,   980,   558,   559,   465,   463,
     838,   602,  1136,    12,    13,  1127,   878,  1128,   182,  1129,
     952,    11,    11,    11,   464,   960,   620,   236,   369,   974,
     370,   575,   447,   374,   976,   375,    11,    11,    11,    11,
      11,    11,   961,   964,   445,   180,  1042,  1057,   501,   514,
     603,    11,    11,   999,   448,   555,   569,   586,  1060,   458,
     467,   477,   486,   505,   519,   524,   528,   533,   541,   546,
     561,   576,   594,   604,   377,   380,   378,   381,   328,    11,
     384,   182,   385,   387,   621,   388,  1124,   446,   762,  -129,
    1088,   990,   447,    11,   152,   180,   152,   984,   948,    11,
     988,  1132,   153,   156,   153,  1166,  1000,  1009,  1015,   328,
     989,  1006,  1011,   152,   373,   152,    11,  1001,  1010,  1016,
    1004,   950,  1019,   132,   133,   134,  1167,  1144,   152,  1178,
    1005,   182,  1020,  1173,    11,    11,    11,   446,   390,   396,
     391,   397,   166,   399,   403,   400,   404,   328,  1179,  1182,
      11,    11,   429,  1183,  1188,   193,   193,   180,  1195,   934,
     935,   136,  1003,  1145,  1018,   177,   178,   918,   918,   127,
     200,   153,   153,   153,   153,   949,   408,   222,   685,   686,
     188,   208,   193,   193,   201,   202,   203,   204,   916,   916,
     273,   165,    11,   182,    11,    11,   409,   808,   481,   224,
     923,   850,   225,   851,  1104,   917,   917,   854,   932,   855,
     220,  1081,   857,   229,   858,   861,   875,   862,   876,   491,
     231,   232,   879,   233,   880,  1137,   492,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   890,   860,   891,   305,   493,   902,   236,
     312,   222,   237,   893,   896,   894,   897,  1092,  1082,  1093,
     326,    10,    10,  1095,  1098,  1096,  1099,   494,   498,   783,
     784,   785,   475,  1101,   460,  1102,   225,   445,   180,  1105,
     499,  1106,  1083,   987,   900,   899,  1142,  1148,   589,   461,
     956,   314,   872,   947,   231,   232,  1112,   233,   462,  1134,
    1111,  1126,  1189,  1165,   210,     0,   814,   815,   816,   817,
     222,   818,   331,   151,   182,     0,     0,   193,     0,     0,
     464,   821,     0,     0,   823,   713,   714,     0,   465,   825,
     826,   827,   224,     0,     0,   225,   273,   180,     0,   366,
       0,    11,   728,   691,     0,   697,   229,     0,   461,     0,
       0,     0,   193,   231,   232,     0,   233,     0,     0,    10,
      10,    10,   708,   979,   710,     0,   445,     0,   841,     0,
       0,   843,     0,   182,     0,   844,   845,   721,     0,   464,
     847,   848,   236,     0,     0,   237,     0,   980,   558,   559,
      10,    10,    10,    10,    10,     0,     0,    10,     0,    10,
       0,    10,    10,    10,     0,   593,     0,    11,     0,     0,
       0,   616,     0,     0,   447,   186,   186,    10,     0,    10,
      10,    10,     0,    10,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   782,  1013,     0,     0,     0,   786,
     787,     0,     0,     0,     0,     0,     0,    11,     0,   802,
     612,   613,   614,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   812,   628,   629,   630,   631,   632,
     633,     0,  1086,   152,   152,     0,     0,     0,     0,     0,
     636,   637,   819,     0,     0,     0,    11,     0,     0,   822,
       0,    11,     0,   824,     0,     0,     0,    11,    11,   828,
       0,     0,     0,   830,  1114,     0,   832,     0,   646,     0,
      11,    11,     0,     0,     0,   152,   152,   152,     0,     0,
     152,     0,   653,     0,     0,     0,     0,     0,   654,     0,
       0,     0,     0,     0,     0,     0,     0,   942,   945,   193,
       0,    11,    11,    11,    11,   660,     0,     0,  1151,     0,
      11,     0,    11,     0,     0,    11,    11,     0,     0,   866,
       0,   477,   982,   666,   667,   668,     0,     0,     0,     0,
       0,   193,  1050,  1051,   869,     0,     0,     0,     0,   673,
     674,     0,   153,   695,   153,     0,     0,     0,     0,   882,
       0,     0,   883,     0,     0,     0,   885,     0,     0,   326,
       0,   153,  1190,   153,     0,  1063,  1064,  1193,     0,     0,
       0,  1197,     0,     0,     0,   193,   153,     0,     0,     0,
       0,   698,     0,   700,   701,     0,    10,    10,     0,     0,
     326,     0,     0,     0,     0,     0,   745,   749,   755,    10,
      10,     0,     0,   760,   763,  1091,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   869,     0,
       0,     0,     0,     0,     0,     0,     0,    10,   326,   193,
     789,   792,     0,     0,   193,   800,   449,   805,   193,   695,
       0,   459,   468,   478,   487,   506,   520,   525,   529,   534,
     542,   547,   562,   577,   595,   605,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   622,     0,     0,     0,
     222,     0,     0,     0,    11,     0,     0,     0,     0,     0,
      11,     0,  1046,  1047,     0,     0,     0,     0,     0,  1080,
       0,     0,   224,   460,     0,   225,     0,   180,  1169,     0,
       0,  1052,  1171,     0,     0,    10,   229,    10,   461,     0,
       0,     0,     0,   231,   232,     0,   233,     0,   109,  1140,
    1146,   463,     0,     0,  1070,  1072,  1074,     0,  1056,  1077,
     835,   503,     0,   182,  1065,   222,     0,     0,     0,   464,
       0,     0,   236,     0,     0,   237,     0,   465,     0,  1084,
    1085,     0,   223,     0,     0,     0,     0,   224,     0,     0,
     225,     0,   476,     0,     0,   504,   226,   227,     0,   228,
    1090,   229,   230,     0,     0,     0,     0,     0,   231,   232,
     460,   233,   195,   196,   180,  1108,  1109,     0,   234,   460,
       0,     0,     0,   180,     0,   461,   870,     0,     0,     0,
       0,     0,     0,   235,   461,   558,   559,   236,   463,     0,
     237,   560,     0,   462,     0,     0,   222,   463,   221,     0,
     182,     0,     0,     0,     0,   263,   464,     0,  -280,   182,
     238,   447,     0,   223,   465,   464,   835,     0,   224,     0,
       0,   225,     0,   465,     0,     0,     0,   226,   227,     0,
     228,   292,   229,   230,     0,     0,     0,     0,     0,   231,
     232,   466,   233,  1168,     0,     0,     0,  1170,     0,   234,
     294,     0,     0,     0,     0,   951,   298,   299,     0,   300,
     870,   997,     0,     0,   235,     0,   962,   963,   236,     0,
       0,   237,  1024,  1030,     0,     0,     0,     0,     0,   991,
     992,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   257,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   153,   153,     0,     0,     0,     0,     0,     0,     0,
    1032,  1033,  1034,  1035,     0,     0,     0,     0,     0,  1038,
       0,  1040,     0,     0,  1043,  1044,     0,     0,     0,     0,
     368,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1066,   153,   153,   153,   222,     0,   153,     0,
       0,   402,     0,   800,   805,     0,     0,     0,     0,     0,
       0,   411,     0,     0,   412,     0,   413,     0,   224,   414,
       0,   225,   415,   180,     0,   416,   417,     0,     0,   418,
       0,     0,   419,     0,   461,   420,   421,   422,   423,   231,
     232,   424,   233,     0,     0,   426,     0,     0,   427,     0,
       0,     0,   222,     0,     0,     0,     0,     0,     0,   182,
       0,     0,     0,     0,     0,   464,     0,     0,     0,     0,
     456,   237,     0,   465,   224,   460,     0,   225,     0,   180,
       0,     0,     0,     0,     0,     0,     0,     0,   229,     0,
     461,   944,     0,     0,   574,   231,   232,     0,   233,     0,
     558,   559,     0,   463,     0,     0,   560,   460,     0,     0,
     445,   180,  1159,  1163,     0,   182,     0,     0,     0,     0,
       0,   464,   461,     0,   236,   575,     0,   237,     0,   465,
       0,   462,     0,  1152,     0,   463,     0,     0,     0,  1153,
       0,     0,     0,     0,   476,     0,     0,   182,     0,   943,
     946,     0,     0,   464,     0,  -261,     0,     0,   447,     0,
       0,   465,     0,     0,     0,     0,    15,     0,     0,     0,
      16,     0,    17,   478,   983,     0,     0,     0,   656,   657,
     658,   659,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,   699,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   711,   712,     0,     0,
     715,   716,     0,   717,   718,     0,   719,   720,     0,   722,
     723,   724,   725,   726,   727,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   741,   742,     0,     0,
    1154,  1155,     0,   758,   759,     0,     0,   767,   768,   769,
       0,   770,   771,   772,   773,   774,   775,  1172,     0,   778,
     779,   780,   781,  1176,     0,  1177,     0,     0,  1180,  1181,
       0,     0,     0,  1186,   795,   796,   797,     0,  1191,     0,
       0,     0,  1194,     0,     0,  1196,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1141,  1147,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   820,     0,    15,     0,     0,     0,   143,   144,
      17,   145,   146,     0,     0,     0,   147,   148,   149,   829,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,   873,
     874,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,   884,     0,     0,     0,   886,     0,     0,   887,
       0,     0,     0,   888,     0,   889,   475,   224,     0,     0,
     225,     0,   180,     0,     0,     0,     0,   904,     0,     0,
     905,   906,     0,   461,     0,     0,     0,     0,   231,   232,
       0,   233,   912,     0,     0,   913,     0,     0,   914,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,     0,
       0,     0,     0,     0,   446,     0,     0,     0,     0,     0,
     237,     0,   465,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   476,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1021,
    1022,  1023,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1037,     0,  1039,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1045,     0,     0,  1048,  1049,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1053,     0,
       0,     0,     0,  1054,     0,     0,  1055,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1075,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1115,     0,  1116,  1117,     0,  1118,
    1119,     0,     0,  1120,     0,     0,  1121,  1122,    15,  1123,
       0,     0,    16,   144,    17,   145,   146,     0,     0,     0,
     147,   148,   149,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    15,     6,     0,     7,     0,     0,    17,   190,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    15,     0,     0,     0,    16,     0,
      17,   325,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    15,     0,     0,
       0,    16,     0,    17,   190,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      15,     0,     0,     0,     0,     0,    17,   190,     0,     0,
    1060,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    15,     0,     0,     0,     0,     0,    17,
     190,     0,     0,  1183,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    15,  1132,     0,     0,
       0,     0,    17,   190,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    15,
       0,     0,     0,     0,     0,    17,   190,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94
};

static const yytype_int16 yycheck[] =
{
       4,   425,   133,   168,     8,     9,   344,   553,    11,   666,
      14,   435,   434,   440,   441,   439,     8,   433,   440,   441,
     442,   567,    23,   629,   440,   700,   701,    43,    44,   442,
      56,     5,   130,     7,   132,     8,   134,    11,    64,   442,
     431,   433,   433,   434,    17,    18,   437,     8,   440,   440,
     441,   442,   443,   444,   660,     0,    82,    28,     8,     5,
      31,     7,    98,   431,   455,   433,   434,    17,     8,     5,
     168,     7,   440,   441,   442,    11,     5,     5,     7,     7,
      98,    82,    11,    97,     5,     8,     7,   455,    59,     8,
      11,     5,   698,     7,    17,    98,    67,   100,   102,     8,
     104,     8,     8,   201,   202,   203,   204,   110,    14,   101,
     101,   102,   116,   117,   118,   119,   120,    98,   432,   100,
     434,    12,    13,     8,   438,    99,    54,    23,   442,   443,
     295,    92,    70,   137,    95,    99,   139,   140,    97,   304,
     143,   434,    71,    64,    94,   438,    74,    83,    88,   442,
     443,   444,    98,    93,   100,   158,   160,   161,   434,   290,
     835,    98,   438,   167,   168,   169,   442,   443,   444,    88,
     173,   175,    39,    98,    93,   179,   180,   181,   182,    88,
      98,    88,    49,     5,    93,     7,    93,   191,   434,    11,
       8,    99,   438,   197,    12,    62,   442,   295,    31,    66,
      85,   205,    87,    31,    37,    90,   304,     8,    75,   212,
      98,    98,   100,   164,   165,    16,    17,    18,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,    67,   238,    99,   433,   434,    67,
       4,   437,   438,   101,   440,   441,   442,   443,   444,    30,
       5,    98,     7,     8,   257,    20,   259,   260,   262,   455,
     294,   953,    26,   955,   298,    29,    31,    31,    31,    98,
      31,    99,    33,    34,    55,   166,    40,   101,   102,    42,
      22,    98,    24,    47,    48,   134,    50,   291,    31,   293,
      32,   295,   296,    98,   297,    31,    77,    78,   301,    98,
      71,   100,    67,    67,    67,   308,    67,    99,    73,    73,
      73,   314,    76,    99,    22,    79,    99,   154,    99,    61,
     157,    75,   159,    31,    67,   328,   329,    69,   332,    99,
     334,    67,    96,   337,    99,    99,    99,    73,    99,    99,
      31,   345,    33,    34,    80,    27,   350,    99,    30,    31,
     953,   954,   201,   202,   203,   204,    99,   361,   362,    67,
      98,   364,   100,    99,    31,    73,    33,    34,    21,    51,
      52,    53,    98,    55,   100,  1032,    67,    99,    31,   216,
     217,   218,   219,    65,   275,    67,   277,   278,   279,    42,
     281,   282,   283,   284,   285,   286,   287,   288,   289,    99,
      67,    27,    31,   407,    30,    84,    99,    86,    99,   955,
      89,    40,    91,    42,    67,    68,    45,    99,    99,     5,
      73,     7,     8,   969,    98,    51,    52,    53,    81,    55,
      98,    60,    99,     8,     9,  1115,    98,  1117,    67,  1119,
      99,   445,   446,   447,    73,    99,    99,    76,    98,    32,
     100,    77,    78,    98,    24,   100,   460,   461,   462,   463,
     464,   465,    99,    99,    30,    31,    99,   101,   433,   434,
      99,   475,   476,    99,   425,   440,   441,   442,    14,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,    98,    98,   100,   100,   629,   503,
      98,    67,   100,    98,   455,   100,   102,    73,   673,    75,
     101,   933,    78,   517,   612,    31,   614,   933,   909,   523,
     933,     5,   132,   133,   134,   103,   953,   954,   955,   660,
     933,   953,   954,   631,   872,   633,   540,   953,   954,   955,
     953,   909,   955,   118,   119,   120,   104,    63,   646,   105,
     953,    67,   955,    85,   558,   559,   560,    73,    98,    98,
     100,   100,   137,    98,    98,   100,   100,   698,   101,   106,
     574,   575,   409,    14,   101,   673,   674,    31,   101,    33,
      34,   125,   953,    99,   955,   160,   161,   901,   902,   115,
     178,   201,   202,   203,   204,   909,   365,     4,   601,   602,
     165,   188,   700,   701,   179,   180,   181,   182,   901,   902,
     220,   136,   616,    67,   618,   619,   366,   709,   932,    26,
     903,    98,    29,   100,  1078,   901,   902,    98,   905,   100,
     205,  1041,    98,    40,   100,    98,    98,   100,   100,   433,
      47,    48,    98,    50,   100,    99,   433,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,    98,   811,   100,   276,   433,   874,    76,
     280,     4,    79,    98,    98,   100,   100,    98,  1043,   100,
     290,   685,   686,    98,    98,   100,   100,   433,   433,   692,
     693,   694,    25,    98,    27,   100,    29,    30,    31,    98,
     433,   100,  1044,   933,   871,   870,  1130,  1131,   442,    42,
     915,   280,   836,   909,    47,    48,  1089,    50,    51,  1126,
    1087,  1114,  1187,  1153,   191,    -1,   729,   730,   731,   732,
       4,   734,   295,   130,    67,    -1,    -1,   835,    -1,    -1,
      73,   744,    -1,    -1,   747,   636,   637,    -1,    81,   752,
     753,   754,    26,    -1,    -1,    29,   366,    31,    -1,   334,
      -1,   765,   653,   612,    -1,   614,    40,    -1,    42,    -1,
      -1,    -1,   870,    47,    48,    -1,    50,    -1,    -1,   783,
     784,   785,   631,    27,   633,    -1,    30,    -1,   791,    -1,
      -1,   794,    -1,    67,    -1,   798,   799,   646,    -1,    73,
     803,   804,    76,    -1,    -1,    79,    -1,    51,    52,    53,
     814,   815,   816,   817,   818,    -1,    -1,   821,    -1,   823,
      -1,   825,   826,   827,    -1,    99,    -1,   831,    -1,    -1,
      -1,    75,    -1,    -1,    78,   164,   165,   841,    -1,   843,
     844,   845,    -1,   847,   848,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   691,    99,    -1,    -1,    -1,   696,
     697,    -1,    -1,    -1,    -1,    -1,    -1,   871,    -1,   706,
     445,   446,   447,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   721,   460,   461,   462,   463,   464,
     465,    -1,  1057,   991,   992,    -1,    -1,    -1,    -1,    -1,
     475,   476,   739,    -1,    -1,    -1,   910,    -1,    -1,   746,
      -1,   915,    -1,   750,    -1,    -1,    -1,   921,   922,   756,
      -1,    -1,    -1,   760,  1089,    -1,   763,    -1,   503,    -1,
     934,   935,    -1,    -1,    -1,  1033,  1034,  1035,    -1,    -1,
    1038,    -1,   517,    -1,    -1,    -1,    -1,    -1,   523,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   908,   909,  1057,
      -1,   965,   966,   967,   968,   540,    -1,    -1,  1133,    -1,
     974,    -1,   976,    -1,    -1,   979,   980,    -1,    -1,   816,
      -1,   932,   933,   558,   559,   560,    -1,    -1,    -1,    -1,
      -1,  1089,   995,   996,   831,    -1,    -1,    -1,    -1,   574,
     575,    -1,   612,   613,   614,    -1,    -1,    -1,    -1,   846,
      -1,    -1,   849,    -1,    -1,    -1,   853,    -1,    -1,   629,
      -1,   631,  1187,   633,    -1,  1028,  1029,  1192,    -1,    -1,
      -1,  1196,    -1,    -1,    -1,  1133,   646,    -1,    -1,    -1,
      -1,   616,    -1,   618,   619,    -1,  1050,  1051,    -1,    -1,
     660,    -1,    -1,    -1,    -1,    -1,   666,   667,   668,  1063,
    1064,    -1,    -1,   673,   674,  1068,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   915,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1091,   698,  1187,
     700,   701,    -1,    -1,  1192,   705,   425,   707,  1196,   709,
      -1,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   455,    -1,    -1,    -1,
       4,    -1,    -1,    -1,  1138,    -1,    -1,    -1,    -1,    -1,
    1144,    -1,   991,   992,    -1,    -1,    -1,    -1,    -1,  1040,
      -1,    -1,    26,    27,    -1,    29,    -1,    31,  1161,    -1,
      -1,   998,  1165,    -1,    -1,  1169,    40,  1171,    42,    -1,
      -1,    -1,    -1,    47,    48,    -1,    50,    -1,   106,  1130,
    1131,    55,    -1,    -1,  1033,  1034,  1035,    -1,  1025,  1038,
     765,    65,    -1,    67,  1031,     4,    -1,    -1,    -1,    73,
      -1,    -1,    76,    -1,    -1,    79,    -1,    81,    -1,  1046,
    1047,    -1,    21,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      29,    -1,    96,    -1,    -1,    99,    35,    36,    -1,    38,
    1067,    40,    41,    -1,    -1,    -1,    -1,    -1,    47,    48,
      27,    50,   170,   171,    31,  1082,  1083,    -1,    57,    27,
      -1,    -1,    -1,    31,    -1,    42,   831,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    42,    52,    53,    76,    55,    -1,
      79,    58,    -1,    51,    -1,    -1,     4,    55,   206,    -1,
      67,    -1,    -1,    -1,    -1,   213,    73,    -1,    75,    67,
      99,    78,    -1,    21,    81,    73,   871,    -1,    26,    -1,
      -1,    29,    -1,    81,    -1,    -1,    -1,    35,    36,    -1,
      38,   239,    40,    41,    -1,    -1,    -1,    -1,    -1,    47,
      48,    99,    50,  1160,    -1,    -1,    -1,  1164,    -1,    57,
     258,    -1,    -1,    -1,    -1,   910,   264,   265,    -1,   267,
     915,   951,    -1,    -1,    72,    -1,   921,   922,    76,    -1,
      -1,    79,   962,   963,    -1,    -1,    -1,    -1,    -1,   934,
     935,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   991,   992,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     965,   966,   967,   968,    -1,    -1,    -1,    -1,    -1,   974,
      -1,   976,    -1,    -1,   979,   980,    -1,    -1,    -1,    -1,
     338,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1032,  1033,  1034,  1035,     4,    -1,  1038,    -1,
      -1,   359,    -1,  1043,  1044,    -1,    -1,    -1,    -1,    -1,
      -1,   369,    -1,    -1,   372,    -1,   374,    -1,    26,   377,
      -1,    29,   380,    31,    -1,   383,   384,    -1,    -1,   387,
      -1,    -1,   390,    -1,    42,   393,   394,   395,   396,    47,
      48,   399,    50,    -1,    -1,   403,    -1,    -1,   406,    -1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
     428,    79,    -1,    81,    26,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      42,    99,    -1,    -1,    46,    47,    48,    -1,    50,    -1,
      52,    53,    -1,    55,    -1,    -1,    58,    27,    -1,    -1,
      30,    31,  1152,  1153,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    42,    -1,    76,    77,    -1,    79,    -1,    81,
      -1,    51,    -1,  1138,    -1,    55,    -1,    -1,    -1,  1144,
      -1,    -1,    -1,    -1,    96,    -1,    -1,    67,    -1,   908,
     909,    -1,    -1,    73,    -1,    75,    -1,    -1,    78,    -1,
      -1,    81,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
       8,    -1,    10,   932,   933,    -1,    -1,    -1,   536,   537,
     538,   539,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,   617,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   634,   635,    -1,    -1,
     638,   639,    -1,   641,   642,    -1,   644,   645,    -1,   647,
     648,   649,   650,   651,   652,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   664,   665,    -1,    -1,
    1150,  1151,    -1,   671,   672,    -1,    -1,   675,   676,   677,
      -1,   679,   680,   681,   682,   683,   684,  1167,    -1,   687,
     688,   689,   690,  1173,    -1,  1175,    -1,    -1,  1178,  1179,
      -1,    -1,    -1,  1183,   702,   703,   704,    -1,  1188,    -1,
      -1,    -1,  1192,    -1,    -1,  1195,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1130,  1131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   740,    -1,     4,    -1,    -1,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,   757,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    -1,    -1,   837,
     838,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,   850,    -1,    -1,    -1,   854,    -1,    -1,   857,
      -1,    -1,    -1,   861,    -1,   863,    25,    26,    -1,    -1,
      29,    -1,    31,    -1,    -1,    -1,    -1,   875,    -1,    -1,
     878,   879,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,
      -1,    50,   890,    -1,    -1,   893,    -1,    -1,   896,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   957,
     958,   959,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   973,    -1,   975,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   990,    -1,    -1,   993,   994,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1006,    -1,
      -1,    -1,    -1,  1011,    -1,    -1,  1014,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1036,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1092,    -1,  1094,  1095,    -1,  1097,
    1098,    -1,    -1,  1101,    -1,    -1,  1104,  1105,     4,  1107,
      -1,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      16,    17,    18,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,     4,     5,    -1,     7,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,     4,    -1,    -1,    -1,     8,    -1,
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
       4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
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
      91,    92,    93,    94,    95,    96,     4,     5,    -1,    -1,
      -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,     4,
      -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   108,   113,   117,   437,     0,     5,     7,    54,    74,
     436,   436,   438,   438,   437,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,   109,   422,   443,   444,   422,
     110,   114,   437,   118,   437,    98,    98,   111,   426,   426,
      97,    11,   116,   427,   436,   120,   437,   428,    56,    64,
      82,   115,   122,   123,   137,   119,   121,   122,     8,   429,
     430,   438,   438,   438,   438,   124,   124,    23,   134,   112,
      98,   100,   433,     8,     9,    11,    12,    16,    17,    18,
     431,   442,   443,   109,   440,   442,   109,   421,   444,   440,
      43,    44,   125,   129,   138,   138,   438,   437,   437,   437,
      98,   100,   425,   441,   425,   437,   425,   438,   438,    28,
      31,    59,    67,   139,   140,   141,   142,   143,   143,   422,
      11,   423,   434,   443,   432,   426,   426,   437,   128,   422,
     128,   438,   438,   438,   438,    70,   144,   151,   151,   135,
     429,    99,    97,    99,   126,   130,   440,   440,   440,   440,
     438,   426,     4,    21,    26,    29,    35,    36,    38,    40,
      41,    47,    48,    50,    57,    72,    76,    79,    99,   152,
     153,   167,   172,   178,   226,   230,   231,   241,   249,   257,
     267,   282,   288,   304,   315,   333,   340,    99,    98,    98,
     100,   435,   437,   426,    98,    98,   100,   131,   425,   425,
     425,   425,    12,   109,   149,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   437,   426,   437,   426,   437,   437,     8,   426,   426,
     426,   150,   145,   422,   101,   109,   312,   393,   394,   422,
     422,   422,   109,   344,   394,   422,   422,   422,   422,   422,
     422,   422,   422,   180,   422,    11,   109,   424,   444,   445,
     137,   434,   437,   137,    71,   132,   133,   437,    98,   100,
     146,   283,   423,   305,   395,   437,   268,   232,   154,   341,
     437,   168,   227,   173,   242,   250,   258,   334,   316,    98,
     289,   437,   437,   136,    99,   127,   438,    99,   426,    98,
     100,   284,    98,   393,    98,   100,   269,    98,   100,   233,
      98,   100,   155,    98,    98,   100,   169,    98,   100,   228,
      98,   100,   174,    98,    98,    98,    98,   100,   335,    98,
     100,   317,   426,    98,   100,   290,    99,   437,   132,   149,
     147,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   183,   426,   426,    99,   425,
     148,   285,   307,   271,   234,   156,   343,   170,   229,   176,
     244,   252,   260,   336,   318,    30,    73,    78,   141,   142,
     162,   179,   184,   224,   225,   291,   426,    99,   141,   142,
      27,    42,    51,    55,    73,    81,    99,   141,   142,   163,
     171,   360,   365,   370,   381,    25,    96,   141,   142,   162,
     171,   230,   276,   306,   313,   360,   141,   142,   163,   171,
     225,   231,   241,   249,   257,   270,   274,   275,   276,   282,
     360,   365,   370,    65,    99,   141,   142,   163,   171,   178,
     226,   230,   313,   360,   365,   374,   381,    20,    99,   141,
     142,   157,   162,    37,   141,   142,   342,    99,   141,   142,
     163,   171,    99,   141,   142,   163,   178,   226,   230,   313,
      22,   141,   142,   162,   175,   177,   141,   142,   163,   171,
     224,   225,   243,   246,   360,   365,   370,   381,    52,    53,
      58,   141,   142,   163,   171,   224,   251,   254,   360,   365,
     375,   377,   379,   381,    46,    77,   141,   142,   163,   171,
     178,   226,   230,   259,   313,   360,   365,   375,   377,   379,
     381,   384,   387,    99,   141,   142,   163,   171,   178,   226,
     230,    45,    60,    99,   141,   142,   163,   171,   178,   226,
     323,   330,   438,   438,   438,    99,    75,   181,    21,    68,
      99,   141,   142,   163,   171,   295,   300,   360,   438,   438,
     438,   438,   438,   438,   286,   287,   438,   438,   309,   311,
      99,   310,   308,    99,   273,   272,   438,   239,   237,   240,
     238,   235,   236,   438,   438,    99,   426,   426,   426,   426,
     438,    99,    99,   181,   245,   248,   438,   438,   438,    99,
     181,   253,   256,   438,   438,   263,   264,   266,    99,   265,
     261,   262,   337,   338,   339,   437,   437,   319,   320,   321,
     322,   440,    85,    87,    90,   109,   166,   440,   438,   426,
     438,   438,   293,   294,   292,   366,   424,   371,   440,   164,
     440,   426,   426,   422,   422,   426,   426,   426,   426,   426,
     426,   440,   426,   426,   426,   426,   426,   426,   422,    84,
      86,    89,    91,   345,   346,   347,   348,   352,   356,   424,
     247,   426,   426,    17,    94,   109,   378,   417,    18,   109,
     376,   417,   418,    92,    95,   109,   380,   255,   426,   426,
     109,   388,   423,   109,   385,   396,   423,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   324,   331,   426,   426,
     426,   426,   425,   437,   437,   437,   425,   425,   424,   109,
     303,   396,   109,   298,   396,   426,   426,   426,    88,    93,
     109,   368,   425,    88,    93,   109,   373,   382,   166,   361,
     277,   314,   425,   158,   437,   437,   437,   437,   437,   425,
     426,   437,   425,   437,   425,   437,   437,   437,   425,   426,
     425,   389,   425,   386,   425,   438,   397,    98,    98,   182,
     301,   437,   296,   437,   437,   437,   367,   437,   437,   372,
      98,   100,   383,   165,    98,   100,   362,    98,   100,   278,
     317,    98,   100,   159,   349,   353,   425,   357,   390,   425,
     438,   396,   395,   426,   426,    98,   100,   185,    98,    98,
     100,   297,   425,   425,   426,   425,   426,   426,   426,   426,
      98,   100,   350,    98,   100,   354,    98,   100,   358,   423,
     386,   326,   326,   186,   426,   426,   426,   195,   363,   279,
      83,   160,   426,   426,   426,   391,   178,   226,   230,   325,
     332,    61,    69,   177,   187,   188,   199,   200,   210,   211,
     213,   214,   307,   299,    33,    34,    99,   141,   142,   222,
     223,    99,   141,   142,    99,   141,   142,   163,   171,   230,
     360,   438,    99,   351,   355,   359,   390,   327,   328,   329,
      99,    99,   438,   438,    99,    39,    49,    62,    66,   189,
     190,   192,   196,   208,    32,   202,    24,   216,   302,    27,
      51,    99,   141,   142,   225,   364,   369,   374,   375,   377,
     381,   438,   438,   281,   280,    88,    93,   109,   161,    99,
     224,   225,   364,   369,   375,   377,   381,   384,    99,   224,
     225,   381,   384,    99,   181,   224,   225,   364,   369,   375,
     377,   426,   426,   426,   109,   398,   399,   400,    88,    93,
     109,   212,   438,   438,   438,   438,   181,   426,   438,   426,
     438,   215,    99,   438,   438,   426,   440,   440,   426,   426,
     437,   437,   425,   426,   426,   426,   425,   101,   401,   402,
      14,   404,   405,   437,   437,   425,   109,   191,   417,   193,
     440,   197,   440,   392,   440,   426,   203,   440,   201,   217,
     422,   216,   368,   373,   425,   425,   423,   403,   101,   406,
     425,   437,    98,   100,   194,    98,   100,   198,    98,   100,
     209,    98,   100,   204,   202,    98,   100,   218,   425,   425,
     409,   401,   405,   407,   423,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   102,   408,   409,   195,   195,   195,
     205,   219,     5,   439,   402,    99,    99,    99,    80,    99,
     141,   142,   162,   206,    63,    99,   141,   142,   162,   220,
     410,   423,   438,   438,   439,   439,    16,    17,    18,   109,
     207,   419,   420,   109,   221,   418,   103,   104,   425,   437,
     425,   437,   439,    85,   411,   416,   439,   439,   105,   101,
     439,   439,   106,    14,   412,   413,   439,   414,   101,   413,
     423,   439,   415,   423,   439,   101,   439,   423
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   107,   108,   108,   110,   109,   111,   112,   111,   114,
     115,   113,   116,   116,   116,   116,   118,   119,   117,   120,
     120,   121,   120,   122,   123,   124,   124,   124,   126,   127,
     125,   128,   130,   129,   131,   131,   132,   132,   133,   135,
     136,   134,   137,   138,   138,   138,   138,   138,   139,   140,
     141,   142,   143,   143,   145,   144,   146,   147,   146,   148,
     148,   148,   150,   149,   149,   151,   151,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   154,   153,   155,
     155,   156,   156,   156,   156,   156,   158,   157,   159,   159,
     160,   161,   161,   161,   162,   164,   165,   163,   166,   166,
     166,   166,   168,   167,   169,   169,   170,   170,   170,   170,
     170,   171,   173,   172,   174,   175,   174,   176,   176,   176,
     176,   176,   177,   179,   178,   180,   182,   181,   183,   184,
     183,   183,   183,   183,   183,   183,   185,   185,   186,   187,
     187,   187,   187,   187,   187,   188,   188,   188,   188,   188,
     188,   189,   190,   191,   191,   192,   193,   194,   194,   195,
     195,   195,   195,   195,   196,   197,   198,   198,   200,   199,
     201,   201,   202,   203,   204,   204,   205,   205,   205,   205,
     205,   206,   207,   207,   208,   209,   209,   210,   211,   212,
     212,   212,   214,   213,   215,   215,   216,   217,   218,   218,
     219,   219,   219,   219,   219,   220,   221,   221,   222,   223,
     224,   225,   227,   226,   228,   228,   229,   229,   229,   229,
     229,   229,   229,   229,   230,   230,   230,   230,   230,   230,
     230,   232,   231,   233,   233,   234,   235,   234,   234,   236,
     234,   234,   234,   234,   234,   234,   237,   234,   238,   234,
     239,   234,   240,   234,   242,   243,   241,   244,   245,   244,
     244,   246,   247,   244,   244,   248,   244,   244,   244,   244,
     244,   244,   244,   250,   251,   249,   252,   253,   252,   252,
     254,   255,   252,   252,   256,   252,   252,   252,   252,   252,
     252,   252,   252,   258,   259,   257,   260,   261,   260,   260,
     262,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   263,   260,   264,   260,   265,   260,   266,   260,   268,
     267,   269,   270,   269,   271,   272,   271,   271,   271,   271,
     271,   271,   271,   271,   273,   271,   274,   274,   275,   275,
     275,   275,   275,   277,   276,   278,   278,   279,   280,   279,
     279,   279,   279,   279,   281,   279,   283,   282,   284,   284,
     285,   286,   285,   285,   287,   285,   285,   285,   285,   285,
     285,   289,   288,   290,   290,   291,   292,   291,   291,   291,
     291,   291,   293,   291,   294,   291,   296,   295,   297,   297,
     298,   298,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   301,   302,   300,   303,   303,   305,   306,   304,
     307,   308,   307,   307,   307,   307,   307,   309,   307,   310,
     307,   311,   307,   312,   312,   314,   313,   316,   315,   317,
     317,   318,   318,   318,   318,   318,   319,   318,   320,   318,
     321,   318,   322,   318,   324,   325,   323,   326,   327,   326,
     328,   326,   329,   326,   331,   332,   330,   334,   333,   335,
     335,   336,   336,   336,   336,   336,   337,   336,   338,   336,
     339,   336,   341,   342,   340,   343,   343,   343,   343,   344,
     344,   345,   345,   346,   346,   346,   347,   349,   348,   350,
     350,   351,   351,   351,   351,   351,   351,   351,   351,   351,
     353,   352,   354,   354,   355,   355,   355,   355,   355,   357,
     356,   358,   358,   359,   359,   359,   359,   359,   359,   359,
     359,   361,   360,   362,   362,   363,   363,   363,   364,   366,
     367,   365,   368,   368,   368,   369,   371,   372,   370,   373,
     373,   373,   374,   375,   376,   376,   377,   378,   378,   378,
     379,   380,   380,   380,   382,   381,   383,   383,   384,   385,
     385,   386,   386,   387,   389,   388,   388,   391,   390,   390,
     392,   393,   394,   395,   395,   397,   396,   399,   398,   400,
     398,   398,   401,   402,   403,   403,   404,   405,   406,   406,
     407,   408,   408,   409,   409,   410,   411,   412,   413,   414,
     414,   415,   415,   416,   417,   418,   418,   419,   419,   420,
     420,   421,   421,   422,   422,   423,   423,   424,   424,   424,
     425,   425,   426,   426,   426,   428,   427,   429,   430,   430,
     431,   431,   431,   432,   432,   433,   433,   434,   434,   435,
     435,   436,   436,   437,   437,   438,   439,   439,   441,   440,
     440,   442,   442,   442,   442,   442,   442,   442,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   443,   443,   444,   445
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     4,     0,     0,     6,     0,
       0,    15,     0,     2,     2,     2,     0,     0,    15,     0,
       2,     0,     3,     4,     4,     0,     2,     2,     0,     0,
      11,     1,     0,     6,     1,     4,     0,     1,     4,     0,
       0,    10,     4,     0,     2,     2,     2,     2,     4,     4,
       4,     4,     0,     3,     0,     5,     1,     0,     5,     0,
       2,     2,     0,     3,     1,     0,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     5,     1,
       4,     0,     2,     2,     2,     2,     0,     6,     1,     4,
       4,     2,     2,     1,     4,     0,     0,     6,     2,     2,
       2,     1,     0,     5,     1,     4,     0,     2,     2,     2,
       2,     4,     0,     5,     1,     0,     5,     0,     2,     2,
       2,     2,     4,     0,     8,     1,     0,     5,     0,     0,
       4,     2,     2,     2,     2,     2,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     0,     2,     2,     2,     3,
       4,     0,     4,     2,     1,     5,     1,     1,     4,     0,
       2,     2,     2,     2,     5,     1,     1,     4,     0,     4,
       0,     3,     4,     1,     1,     4,     0,     2,     2,     2,
       2,     4,     2,     1,     4,     1,     4,     4,     4,     2,
       2,     1,     0,     3,     0,     2,     5,     1,     1,     4,
       0,     2,     2,     2,     2,     4,     2,     1,     4,     4,
       4,     4,     0,     5,     1,     4,     0,     2,     2,     2,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     0,     4,     2,     0,
       4,     2,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     0,     9,     0,     0,     4,
       2,     0,     0,     5,     2,     0,     4,     2,     2,     2,
       2,     2,     2,     0,     0,     9,     0,     0,     4,     2,
       0,     0,     5,     2,     0,     4,     2,     2,     2,     2,
       2,     2,     2,     0,     0,     9,     0,     0,     4,     2,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       5,     1,     0,     5,     0,     0,     4,     2,     2,     2,
       2,     2,     2,     2,     0,     4,     1,     1,     1,     1,
       1,     1,     1,     0,     5,     1,     4,     0,     0,     4,
       2,     2,     2,     2,     0,     4,     0,     5,     1,     4,
       0,     0,     4,     2,     0,     4,     2,     2,     2,     2,
       2,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     5,     1,     4,
       2,     1,     0,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     0,     0,     9,     2,     1,     0,     0,     9,
       0,     0,     4,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     2,     1,     0,     5,     0,     5,     1,
       4,     0,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     0,     8,     0,     0,     4,
       0,     4,     0,     4,     0,     0,     8,     0,     5,     1,
       4,     0,     2,     2,     2,     2,     0,     4,     0,     4,
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
       1,     1,     1,     1,     1,     1,     1,     1,     1
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

    case 186: /* type_stmtsep  */

      { if (read_all) {
                yang_delete_type(module, ((*yyvaluep).type));
              }
            }

        break;

    case 271: /* choice_opt_stmt  */

      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }

        break;

    case 343: /* deviation_opt_stmt  */

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
                 }
             }

    break;

  case 29:

    { if (read_all) {
                                 if (yang_read_prefix(trg, actual, s, IMPORT_KEYWORD)) {YYABORT;}
                                 s=NULL;
                                 actual_type=IMPORT_KEYWORD;
                               }
                             }

    break;

  case 30:

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

  case 31:

    { (yyval.str) = s; s = NULL; }

    break;

  case 32:

    { if (read_all) {
                                                             memset(rev, 0, LY_REV_SIZE);
                                                             actual_type = INCLUDE_KEYWORD;
                                                           }
                                                           else {
                                                             size_arrays->inc++;
                                                           }
                                                         }

    break;

  case 33:

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

  case 38:

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

  case 39:

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

  case 40:

    { if (read_all) {
                                     if (yang_read_prefix(trg, NULL, s, MODULE_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }
                                 }

    break;

  case 44:

    { if (read_all && yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 45:

    { if (read_all && yang_read_common(trg, s, CONTACT_KEYWORD)) {YYABORT;} s=NULL; }

    break;

  case 46:

    { if (read_all && yang_read_description(trg, NULL, s, NULL)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 47:

    { if (read_all && yang_read_reference(trg, NULL, s, NULL)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 52:

    { if (read_all && size_arrays->rev) {
                           trg->rev = calloc(size_arrays->rev, sizeof *trg->rev);
                           if (!trg->rev) {
                             LOGMEM;
                             YYABORT;
                           }
                         }
                       }

    break;

  case 54:

    { if (read_all) {
                                                     if(!(actual=yang_read_revision(trg,s))) {YYABORT;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }

    break;

  case 57:

    { actual_type = REVISION_KEYWORD; }

    break;

  case 60:

    { if (read_all && yang_read_description(trg, actual, s, "revision")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 61:

    { if (read_all && yang_read_reference(trg, actual, s, "revision")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 62:

    { if (read_all) {
                                s = strdup(yyget_text(scanner));
                                if (!s) {
                                  LOGMEM;
                                  YYABORT;
                                }
                              }
                            }

    break;

  case 64:

    { if (read_all && lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 65:

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

  case 66:

    { actual = NULL; }

    break;

  case 70:

    { if (!read_all) { size_arrays->tpdf++; } }

    break;

  case 73:

    { if (!read_all) {
                     size_arrays->augment++;
                   } else {
                     config_inherit = ENABLE_INHERIT;
                   }
                 }

    break;

  case 76:

    { if (!read_all) { size_arrays->deviation++; } }

    break;

  case 77:

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

  case 81:

    { (yyval.uint) = 0; }

    break;

  case 82:

    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }

    break;

  case 83:

    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }

    break;

  case 84:

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

  case 85:

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

  case 86:

    { free(s); s = NULL; }

    break;

  case 93:

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

  case 94:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 95:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 96:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 97:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 98:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 99:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 100:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 101:

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

  case 102:

    { if (read_all) {
                                                         if (!(actual = yang_read_feature(trg, s))) {YYABORT;}
                                                         s=NULL;
                                                       } else {
                                                         size_arrays->features++;
                                                       }
                                                     }

    break;

  case 106:

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

  case 107:

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

  case 108:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].i)].flags, LYS_STATUS_MASK, "status", "feature", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         }
                                       }

    break;

  case 109:

    { if (read_all && yang_read_description(trg, actual, s, "feature")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 110:

    { if (read_all && yang_read_reference(trg, actual, s, "feature")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 112:

    { if (read_all) {
                                                           if (!(actual = yang_read_identity(trg,s))) {YYABORT;}
                                                           s = NULL;
                                                         } else {
                                                           size_arrays->ident++;
                                                         }
                                                       }

    break;

  case 115:

    { if (read_all && yang_read_base(module, actual, (yyvsp[0].str), unres)) {
                               YYABORT;
                             }
                           }

    break;

  case 117:

    { (yyval.str) = NULL; }

    break;

  case 118:

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

  case 119:

    { if (read_all) {
                                       if (yang_check_flags((uint16_t*)&((struct lys_ident *)actual)->flags, LYS_STATUS_MASK, "status", "identity", (yyvsp[0].i), 1)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 120:

    { if (read_all && yang_read_description(trg, actual, s, "identity")) {
                                            free((yyvsp[-1].str));
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 121:

    { if (read_all && yang_read_reference(trg, actual, s, "identity")) {
                                          free((yyvsp[-1].str));
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 123:

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

  case 125:

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

  case 126:

    { if (read_all && !(actual = yang_read_type(trg, actual, s, actual_type))) {
                                                       YYABORT;
                                                     }
                                                     s = NULL;
                                                   }

    break;

  case 128:

    { (yyval.nodes).node.ptr_tpdf = actual;
                        (yyval.nodes).node.flag = 0;
                      }

    break;

  case 129:

    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, (yyvsp[0].nodes).node.ptr_tpdf, "type", "typedef");
                       YYABORT;
                     }
                   }

    break;

  case 130:

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

  case 131:

    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 132:

    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 133:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 134:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 135:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 138:

    { if (read_all) {
                          (yyval.type) = actual;
                        }
                      }

    break;

  case 141:

    { /*leafref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 142:

    { /*identityref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->flags |= LYS_TYPE_BASE;
                   ((struct yang_type *)actual)->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }

    break;

  case 143:

    { /*instance_identifier_specification */
                             if (read_all) {
                               ((struct yang_type *)actual)->base = LY_TYPE_INST;
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

  case 147:

    { if (!read_all) {
                                                  size_arrays->node[(yyvsp[-1].uint)].pattern++; /* count of pattern*/
                                                }
                                              }

    break;

  case 150:

    { if (read_all) {
                                                                  actual = (yyvsp[-2].v);
                                                                } else {
                                                                  size_arrays->node[(yyvsp[-3].uint)].uni++; /* count of union*/
                                                                }
                                                              }

    break;

  case 151:

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

  case 152:

    { if (read_all && yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 153:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 154:

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

  case 155:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 156:

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

  case 159:

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

  case 160:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 161:

    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 162:

    { if (read_all && yang_read_description(trg, actual, s, (yyvsp[-1].str))) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 163:

    { if (read_all && yang_read_reference(trg, actual, s, (yyvsp[-1].str))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 164:

    { actual = (yyvsp[-2].v);
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 165:

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

  case 168:

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

  case 172:

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

  case 173:

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

  case 177:

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

  case 178:

    { if (read_all) {
                                   if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                     YYABORT;
                                   }
                                 }
                               }

    break;

  case 179:

    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 180:

    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 181:

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

  case 182:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 183:

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

  case 184:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 189:

    { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }

    break;

  case 190:

    { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }

    break;

  case 191:

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

  case 192:

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

  case 196:

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

  case 197:

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

  case 201:

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

  case 202:

    { if (read_all) {
                                  if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                    YYABORT;
                                  }
                                }
                              }

    break;

  case 203:

    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 204:

    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 205:

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

  case 206:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 207:

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

  case 212:

    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYABORT;}
                                                           s=NULL;
                                                         }
                                                       }

    break;

  case 216:

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

  case 217:

    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i), 0)) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 218:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 219:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 220:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 221:

    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }

    break;

  case 222:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 223:

    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }

    break;

  case 231:

    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYABORT;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }

    break;

  case 235:

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

  case 236:

    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }

    break;

  case 238:

    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYABORT;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }

    break;

  case 239:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }

    break;

  case 241:

    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYABORT;} s=NULL; }

    break;

  case 242:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 243:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 244:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 245:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 246:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
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

  case 257:

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
                     if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaf) == -1) {
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

  case 276:

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
                   if (unres_schema_add_node(trg, unres, &(yyvsp[-2].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER, (struct lys_node *)(yyvsp[-2].nodes).node.ptr_leaflist) == -1) {
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
                              } else {
                                /* list has keys */
                                size_arrays->node[(yyvsp[-1].nodes).index].keys = 1;
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
                                 data_node = actual;
                               }

    break;

  case 317:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 318:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 319:

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

  case 322:

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

  case 324:

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

  case 325:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }

    break;

  case 326:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 327:

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

  case 328:

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

  case 329:

    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i), 0)) {
                                             YYABORT;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 330:

    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i), 0)) {
                                        YYABORT;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }

    break;

  case 331:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 332:

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

  case 333:

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

  case 334:

    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }

    break;

  case 335:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 343:

    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYABORT;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }

    break;

  case 347:

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

  case 348:

    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }

    break;

  case 350:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 351:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 352:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 353:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 354:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 356:

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

  case 360:

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

  case 361:

    { actual = (yyvsp[-1].nodes).anyxml; actual_type = ANYXML_KEYWORD; }

    break;

  case 363:

    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).anyxml, s, unres, ANYXML_KEYWORD)) {YYABORT;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }

    break;

  case 364:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }

    break;

  case 366:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 367:

    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "anyxml", (yyvsp[0].i), 0)) {
                                               YYABORT;
                                             }
                                           }
                                         }

    break;

  case 368:

    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "anyxml", (yyvsp[0].i), 0)) {
                                            YYABORT;
                                          }
                                        }
                                      }

    break;

  case 369:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 370:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 371:

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

  case 372:

    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYABORT;
                        }
                      }
                    }

    break;

  case 375:

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

  case 376:

    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }

    break;

  case 378:

    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYABORT;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }

    break;

  case 379:

    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i), 0)) {
                                          YYABORT;
                                        }
                                      }
                                    }

    break;

  case 380:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 381:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 382:

    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }

    break;

  case 384:

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

  case 386:

    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYABORT;
                                                   }
                                                   s = NULL;
                                                 }
                                               }

    break;

  case 392:

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

  case 393:

    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }

    break;

  case 394:

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

  case 395:

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

  case 396:

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

  case 397:

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

  case 398:

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

  case 399:

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

  case 400:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 401:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 402:

    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYABORT;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }

    break;

  case 403:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYABORT;
                                          }
                                        }

    break;

  case 407:

    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYABORT;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }

    break;

  case 408:

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

  case 410:

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

  case 411:

    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }

    break;

  case 413:

    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYABORT;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }

    break;

  case 414:

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

  case 415:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 416:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

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

  case 419:

    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                      data_node = actual;
                                    }
                                  }

    break;

  case 420:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 421:

    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }

    break;

  case 422:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 425:

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

  case 426:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 427:

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

  case 428:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 431:

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

  case 432:

    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYABORT;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }

    break;

  case 433:

    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i), 0)) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 434:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 435:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 436:

    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }

    break;

  case 438:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 440:

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

  case 441:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 442:

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

  case 443:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 444:

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

  case 445:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYABORT;
                                        }
                                      }

    break;

  case 447:

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

  case 448:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 450:

    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 452:

    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }

    break;

  case 453:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 454:

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

  case 455:

    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYABORT;
                                         }
                                       }

    break;

  case 457:

    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYABORT;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }

    break;

  case 458:

    { config_inherit = ENABLE_INHERIT; }

    break;

  case 459:

    { if (read_all) {
                          size_arrays->next++;
                        }
                      }

    break;

  case 461:

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

  case 462:

    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYABORT;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }

    break;

  case 463:

    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i), 0)) {
                                                  YYABORT;
                                                }
                                              }
                                            }

    break;

  case 464:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 465:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 466:

    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }

    break;

  case 468:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 470:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 472:

    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYABORT;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }

    break;

  case 473:

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

  case 475:

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

  case 476:

    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             free((yyvsp[-1].nodes).deviation);
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 477:

    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           free((yyvsp[-1].nodes).deviation);
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 478:

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

  case 481:

    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 487:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 491:

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

  case 492:

    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }

    break;

  case 493:

    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }

    break;

  case 494:

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

  case 495:

    { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }

    break;

  case 496:

    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYABORT;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }

    break;

  case 497:

    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYABORT;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }

    break;

  case 498:

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

  case 499:

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

  case 500:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 504:

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

  case 505:

    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }

    break;

  case 506:

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

  case 507:

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

  case 508:

    { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 509:

    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 513:

    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }

    break;

  case 514:

    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target) == -1) {
                                              YYABORT;
                                            }
                                          }
                                        }

    break;

  case 515:

    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYABORT;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
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

    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYABORT;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }

    break;

  case 518:

    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYABORT;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }

    break;

  case 519:

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

  case 520:

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

  case 521:

    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYABORT;} s=NULL; actual_type=WHEN_KEYWORD;}

    break;

  case 526:

    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 527:

    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYABORT;
                                    }
                                    s = NULL;
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

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 533:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 534:

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

  case 535:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 536:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 537:

    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }

    break;

  case 538:

    { (yyval.i) = (yyvsp[-2].i); }

    break;

  case 539:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 540:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 541:

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

  case 543:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 544:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 545:

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

  case 546:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 547:

    { (yyval.uint) = 0; }

    break;

  case 548:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 549:

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

  case 550:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 551:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 552:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 553:

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

  case 554:

    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYABORT;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }

    break;

  case 564:

    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                               }
                             }

    break;

  case 567:

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

  case 570:

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

  case 571:

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

  case 575:

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

  case 577:

    { tmp_s = yyget_text(scanner); }

    break;

  case 578:

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

  case 579:

    { tmp_s = yyget_text(scanner); }

    break;

  case 580:

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

  case 605:

    { (yyval.uint) = 0; }

    break;

  case 606:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 607:

    { (yyval.i) = 0; }

    break;

  case 608:

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

  case 614:

    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 619:

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

  case 625:

    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYABORT;
                                       }
                                     }
                                   }

    break;

  case 648:

    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYABORT;
                    }
                  }
                }

    break;

  case 737:

    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYABORT;
                            }
                          }
                        }

    break;

  case 738:

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
