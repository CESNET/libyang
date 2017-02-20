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

#define YANG_ADDELEM(current_ptr, size)                                                  \
    if (!(size % LY_YANG_ARRAY_SIZE)) {                                                  \
        void *tmp;                                                                       \
                                                                                         \
        tmp = realloc(current_ptr, (sizeof *current_ptr) * (size + LY_YANG_ARRAY_SIZE)); \
        if (!tmp) {                                                                      \
            LOGMEM;                                                                      \
            free(s);                                                                     \
            YYABORT;                                                                     \
        }                                                                                \
        memset(tmp + (sizeof *current_ptr) * size, 0, (sizeof *current_ptr) * LY_YANG_ARRAY_SIZE); \
        current_ptr = tmp;                                                               \
    }                                                                                    \
    actual = &current_ptr[size++];                                                       \

void yyerror(YYLTYPE *yylloc, void *scanner, struct yang_parameter *param, ...);
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
    ANYDATA_KEYWORD = 353,
    NODE = 354,
    NODE_PRINT = 355,
    EXTENSION_INSTANCE = 356,
    SUBMODULE_EXT_KEYWORD = 357
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
  struct yang_type *type;
  struct lys_deviation *dev;
  struct lys_deviate *deviate;
  union {
    uint32_t index;
    struct lys_node_container *container;
    struct lys_node_anydata *anydata;
    struct type_node node;
    struct lys_node_case *cs;
    struct lys_node_grp *grouping;
    struct lys_refine *refine;
    struct lys_node_notif *notif;
    struct lys_node_uses *uses;
    struct lys_node_inout *inout;
    struct lys_node_augment *augment;
  } nodes;
  enum yytokentype token;
  struct {
    void *actual;
    enum yytokentype token;
  } backup_token;
  struct {
    struct lys_revision **revision;
    int index;
  } revisions;


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



int yyparse (void *scanner, struct yang_parameter *param);

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
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3524

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  113
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  330
/* YYNRULES -- Number of rules.  */
#define YYNRULES  829
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1322

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   357

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
     111,   112,     2,   103,     2,     2,     2,   107,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   106,
       2,   110,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   108,     2,   109,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   104,     2,   105,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101,   102
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   332,   332,   333,   334,   336,   359,   362,   364,   363,
     387,   398,   408,   418,   419,   425,   430,   436,   447,   457,
     470,   471,   477,   479,   483,   485,   489,   491,   492,   493,
     495,   503,   511,   512,   517,   528,   539,   550,   558,   563,
     564,   568,   569,   580,   591,   602,   606,   608,   631,   648,
     652,   654,   655,   660,   665,   670,   676,   680,   682,   686,
     688,   692,   694,   698,   700,   713,   724,   725,   736,   740,
     741,   745,   746,   751,   758,   758,   765,   771,   819,   838,
     841,   842,   843,   844,   845,   846,   847,   848,   849,   850,
     853,   868,   875,   876,   880,   881,   882,   888,   893,   899,
     917,   919,   920,   924,   929,   930,   952,   953,   954,   967,
     972,   974,   975,   976,   977,   992,  1006,  1011,  1012,  1027,
    1028,  1029,  1035,  1040,  1046,  1103,  1108,  1109,  1111,  1127,
    1132,  1133,  1158,  1159,  1173,  1174,  1180,  1185,  1191,  1195,
    1197,  1250,  1261,  1264,  1267,  1272,  1277,  1283,  1288,  1294,
    1299,  1308,  1309,  1313,  1348,  1349,  1351,  1352,  1356,  1362,
    1375,  1376,  1377,  1381,  1382,  1384,  1388,  1406,  1411,  1413,
    1414,  1430,  1435,  1444,  1445,  1449,  1465,  1470,  1475,  1480,
    1486,  1490,  1497,  1512,  1513,  1517,  1518,  1528,  1533,  1538,
    1543,  1549,  1553,  1564,  1576,  1577,  1580,  1588,  1599,  1600,
    1615,  1616,  1617,  1629,  1633,  1638,  1644,  1649,  1651,  1652,
    1667,  1672,  1673,  1678,  1682,  1684,  1689,  1691,  1692,  1693,
    1706,  1718,  1719,  1721,  1729,  1741,  1742,  1757,  1758,  1759,
    1771,  1776,  1781,  1787,  1792,  1794,  1795,  1811,  1815,  1817,
    1821,  1823,  1827,  1829,  1833,  1835,  1845,  1852,  1853,  1857,
    1858,  1864,  1869,  1874,  1875,  1876,  1877,  1878,  1884,  1885,
    1886,  1887,  1888,  1889,  1890,  1891,  1894,  1904,  1911,  1912,
    1935,  1936,  1937,  1938,  1939,  1944,  1950,  1956,  1961,  1966,
    1967,  1968,  1973,  1974,  1976,  2016,  2026,  2029,  2030,  2031,
    2034,  2039,  2040,  2045,  2051,  2057,  2063,  2068,  2074,  2084,
    2139,  2142,  2143,  2144,  2147,  2158,  2163,  2164,  2170,  2183,
    2196,  2206,  2212,  2217,  2223,  2233,  2280,  2283,  2284,  2285,
    2286,  2295,  2301,  2307,  2320,  2333,  2343,  2349,  2354,  2359,
    2360,  2361,  2362,  2367,  2369,  2379,  2386,  2387,  2407,  2410,
    2411,  2412,  2422,  2429,  2436,  2443,  2449,  2455,  2457,  2458,
    2460,  2461,  2462,  2463,  2464,  2465,  2466,  2472,  2482,  2489,
    2490,  2504,  2505,  2506,  2507,  2513,  2518,  2523,  2526,  2536,
    2543,  2553,  2560,  2561,  2584,  2587,  2588,  2589,  2590,  2597,
    2604,  2611,  2616,  2622,  2632,  2639,  2640,  2672,  2673,  2674,
    2675,  2681,  2686,  2691,  2692,  2694,  2695,  2697,  2710,  2715,
    2716,  2748,  2751,  2765,  2781,  2803,  2854,  2871,  2888,  2909,
    2930,  2935,  2941,  2942,  2945,  2960,  2969,  2970,  2972,  2983,
    2992,  2993,  2994,  2995,  3001,  3006,  3011,  3012,  3013,  3018,
    3020,  3035,  3042,  3052,  3059,  3060,  3084,  3087,  3088,  3094,
    3099,  3104,  3105,  3106,  3113,  3121,  3136,  3166,  3167,  3168,
    3169,  3170,  3172,  3187,  3217,  3226,  3233,  3234,  3266,  3267,
    3268,  3269,  3275,  3280,  3285,  3286,  3287,  3289,  3301,  3321,
    3322,  3328,  3334,  3336,  3337,  3339,  3340,  3343,  3351,  3356,
    3357,  3359,  3360,  3361,  3363,  3371,  3376,  3377,  3409,  3410,
    3416,  3417,  3423,  3429,  3436,  3443,  3451,  3460,  3468,  3473,
    3474,  3506,  3507,  3513,  3514,  3520,  3527,  3535,  3540,  3541,
    3555,  3556,  3557,  3563,  3569,  3576,  3583,  3591,  3600,  3609,
    3614,  3615,  3619,  3620,  3625,  3631,  3636,  3638,  3639,  3640,
    3653,  3658,  3660,  3661,  3662,  3675,  3679,  3681,  3686,  3688,
    3689,  3709,  3714,  3716,  3717,  3718,  3738,  3743,  3745,  3746,
    3747,  3759,  3827,  3832,  3833,  3837,  3841,  3843,  3844,  3846,
    3847,  3849,  3853,  3855,  3855,  3862,  3865,  3874,  3893,  3895,
    3896,  3899,  3899,  3916,  3916,  3923,  3923,  3930,  3933,  3935,
    3937,  3938,  3940,  3942,  3944,  3945,  3947,  3949,  3950,  3952,
    3953,  3955,  3957,  3960,  3964,  3966,  3967,  3969,  3970,  3972,
    3974,  3985,  3986,  3989,  3990,  4001,  4002,  4004,  4005,  4007,
    4008,  4014,  4015,  4018,  4019,  4020,  4044,  4045,  4048,  4054,
    4058,  4063,  4064,  4065,  4068,  4073,  4083,  4085,  4086,  4088,
    4089,  4091,  4092,  4093,  4095,  4096,  4098,  4099,  4101,  4102,
    4106,  4107,  4134,  4172,  4173,  4175,  4177,  4179,  4180,  4182,
    4183,  4185,  4186,  4189,  4190,  4193,  4195,  4196,  4199,  4199,
    4206,  4208,  4209,  4210,  4211,  4212,  4213,  4214,  4216,  4217,
    4218,  4220,  4221,  4222,  4223,  4224,  4225,  4226,  4227,  4228,
    4229,  4232,  4233,  4234,  4235,  4236,  4237,  4238,  4239,  4240,
    4241,  4242,  4243,  4244,  4245,  4246,  4247,  4248,  4249,  4250,
    4251,  4252,  4253,  4254,  4255,  4256,  4257,  4258,  4259,  4260,
    4261,  4262,  4263,  4264,  4265,  4266,  4267,  4268,  4269,  4270,
    4271,  4272,  4273,  4274,  4275,  4276,  4277,  4278,  4279,  4280,
    4281,  4282,  4283,  4284,  4285,  4286,  4287,  4288,  4289,  4290,
    4291,  4292,  4293,  4294,  4295,  4296,  4297,  4298,  4299,  4300,
    4301,  4303,  4310,  4317,  4332,  4344,  4356,  4379,  4386,  4404,
    4443,  4445,  4446,  4447,  4448,  4449,  4450,  4451,  4452,  4453,
    4454,  4455,  4456,  4457,  4459,  4460,  4461,  4462,  4463,  4464,
    4465,  4466,  4467,  4468,  4469,  4470,  4471,  4472,  4474,  4475,
    4476,  4477,  4479,  4486,  4487,  4492,  4497,  4502,  4507,  4512,
    4517,  4522,  4527,  4532,  4537,  4542,  4547,  4552,  4557,  4562,
    4575,  4594,  4599,  4604,  4609,  4614,  4619,  4623,  4633,  4644,
    4655,  4666,  4677,  4693,  4708,  4709,  4715,  4722,  4737,  4738
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
  "NODE", "NODE_PRINT", "EXTENSION_INSTANCE", "SUBMODULE_EXT_KEYWORD",
  "'+'", "'{'", "'}'", "';'", "'/'", "'['", "']'", "'='", "'('", "')'",
  "$accept", "start", "tmp_string", "string_1", "string_2", "$@1",
  "module_arg_str", "module_stmt", "module_header_stmts",
  "module_header_stmt", "submodule_arg_str", "submodule_stmt",
  "submodule_header_stmts", "submodule_header_stmt", "yang_version_arg",
  "yang_version_stmt", "namespace_arg_str", "namespace_stmt",
  "linkage_stmts", "import_stmt", "import_arg_str", "import_opt_stmt",
  "include_arg_str", "include_stmt", "include_end", "include_opt_stmt",
  "revision_date_arg", "revision_date_stmt", "belongs_to_arg_str",
  "belongs_to_stmt", "prefix_arg", "prefix_stmt", "meta_stmts",
  "organization_arg", "organization_stmt", "contact_arg", "contact_stmt",
  "description_arg", "description_stmt", "reference_arg", "reference_stmt",
  "revision_stmts", "revision_arg_stmt", "revision_stmts_opt",
  "revision_stmt", "revision_end", "revision_opt_stmt", "date_arg_str",
  "$@2", "body_stmts_end", "body_stmts", "body_stmt", "extension_arg_str",
  "extension_stmt", "extension_end", "extension_opt_stmt", "argument_str",
  "argument_stmt", "argument_end", "yin_element_arg", "yin_element_stmt",
  "yin_element_arg_str", "status_arg", "status_stmt", "status_arg_str",
  "feature_arg_str", "feature_stmt", "feature_end", "feature_opt_stmt",
  "if_feature_arg", "if_feature_stmt", "if_feature_end",
  "identity_arg_str", "identity_stmt", "identity_end", "identity_opt_stmt",
  "base_arg", "base_stmt", "typedef_arg_str", "typedef_stmt",
  "type_opt_stmt", "type_stmt", "type_arg_str", "type_end",
  "type_body_stmts", "some_restrictions", "union_stmt", "union_spec",
  "fraction_digits_arg", "fraction_digits_stmt", "fraction_digits_arg_str",
  "length_stmt", "length_arg_str", "length_end", "message_opt_stmt",
  "pattern_sep", "pattern_stmt", "pattern_arg_str", "pattern_end",
  "pattern_opt_stmt", "modifier_arg", "modifier_stmt",
  "enum_specification", "enum_stmts", "enum_stmt", "enum_arg_str",
  "enum_end", "enum_opt_stmt", "value_arg", "value_stmt",
  "integer_value_arg_str", "range_stmt", "range_end", "path_arg",
  "path_stmt", "require_instance_arg", "require_instance_stmt",
  "require_instance_arg_str", "bits_specification", "bit_stmts",
  "bit_stmt", "bit_arg_str", "bit_end", "bit_opt_stmt",
  "position_value_arg", "position_stmt", "position_value_arg_str",
  "error_message_arg", "error_message_stmt", "error_app_tag_arg",
  "error_app_tag_stmt", "units_arg", "units_stmt", "default_arg",
  "default_stmt", "grouping_arg_str", "grouping_stmt", "grouping_end",
  "grouping_opt_stmt", "data_def_stmt", "container_arg_str",
  "container_stmt", "container_end", "container_opt_stmt", "leaf_stmt",
  "leaf_arg_str", "leaf_opt_stmt", "leaf_list_arg_str", "leaf_list_stmt",
  "leaf_list_opt_stmt", "list_arg_str", "list_stmt", "list_opt_stmt",
  "choice_arg_str", "choice_stmt", "choice_end", "choice_opt_stmt",
  "short_case_case_stmt", "short_case_stmt", "case_arg_str", "case_stmt",
  "case_end", "case_opt_stmt", "anyxml_arg_str", "anyxml_stmt",
  "anydata_arg_str", "anydata_stmt", "anyxml_end", "anyxml_opt_stmt",
  "uses_arg_str", "uses_stmt", "uses_end", "uses_opt_stmt",
  "refine_args_str", "refine_arg_str", "refine_stmt", "refine_end",
  "refine_body_opt_stmts", "uses_augment_arg_str", "uses_augment_arg",
  "uses_augment_stmt", "augment_arg_str", "augment_arg", "augment_stmt",
  "augment_opt_stmt", "action_arg_str", "action_stmt", "rpc_arg_str",
  "rpc_stmt", "rpc_end", "rpc_opt_stmt", "input_arg", "input_stmt",
  "input_output_opt_stmt", "output_arg", "output_stmt",
  "notification_arg_str", "notification_stmt", "notification_end",
  "notification_opt_stmt", "deviation_arg", "deviation_stmt",
  "deviation_opt_stmt", "deviation_arg_str", "deviate_body_stmt",
  "deviate_not_supported", "deviate_not_supported_stmt",
  "deviate_not_supported_end", "deviate_stmts", "deviate_add",
  "deviate_add_stmt", "deviate_add_end", "deviate_add_opt_stmt",
  "deviate_delete", "deviate_delete_stmt", "deviate_delete_end",
  "deviate_delete_opt_stmt", "deviate_replace", "deviate_replace_stmt",
  "deviate_replace_end", "deviate_replace_opt_stmt", "when_arg_str",
  "when_stmt", "when_end", "when_opt_stmt", "config_arg", "config_stmt",
  "config_arg_str", "mandatory_arg", "mandatory_stmt", "mandatory_arg_str",
  "presence_arg", "presence_stmt", "min_value_arg", "min_elements_stmt",
  "min_value_arg_str", "max_value_arg", "max_elements_stmt",
  "max_value_arg_str", "ordered_by_arg", "ordered_by_stmt",
  "ordered_by_arg_str", "must_agr_str", "must_stmt", "must_end",
  "unique_arg", "unique_stmt", "unique_arg_str", "unique_arg_opt",
  "key_arg", "key_stmt", "key_arg_str", "$@3", "range_arg_str",
  "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@4",
  "path_arg_str", "$@5", "$@6", "absolute_path", "absolute_paths",
  "absolute_path_opt", "relative_path", "relative_path_part1",
  "relative_path_part1_opt", "descendant_path", "descendant_path_opt",
  "path_predicate", "path_equality_expr", "path_key_expr",
  "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "semicolom", "curly_bracket_close",
  "curly_bracket_open", "stmtsep", "unknown_statement", "string_opt",
  "string_opt_part1", "string_opt_part2", "unknown_string",
  "unknown_string_part1", "unknown_string_part2", "unknown_statement_end",
  "unknown_statement2_opt", "unknown_statement2", "unknown_statement2_end",
  "unknown_statement2_yang_stmt", "unknown_statement2_module_stmt",
  "unknown_statement3_opt", "unknown_statement3_opt_end", "sep_stmt",
  "optsep", "sep", "whitespace_opt", "string", "$@7", "strings",
  "identifier", "identifier1", "yang_stmt", "identifiers",
  "identifiers_ref", "type_ext_alloc", "typedef_ext_alloc",
  "iffeature_ext_alloc", "restriction_ext_alloc", "when_ext_alloc",
  "revision_ext_alloc", "datadef_ext_check", "not_supported_ext_check",
  "not_supported_ext", "datadef_ext_stmt", "restriction_ext_stmt",
  "ext_substatements", YY_NULLPTR
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
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,    43,   123,   125,    59,    47,    91,    93,
      61,    40,    41
};
# endif

#define YYPACT_NINF -818

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-818)))

#define YYTABLE_NINF -759

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     311,    63,  -818,  -818,   602,  1854,  -818,  -818,  -818,   213,
     213,  -818,   213,  -818,   213,   213,  -818,   213,   213,   213,
     213,  -818,   213,   213,  -818,  -818,  -818,  -818,   213,  -818,
    -818,  -818,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
    -818,  -818,   213,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,    10,     1,    65,   616,    35,    57,  2021,
     213,  -818,  -818,  3331,  3331,  3331,  2856,  3331,   138,  2666,
    2666,  2666,  2666,  2666,    47,  2951,   187,    98,   198,  2666,
     101,  2666,   247,   198,  3331,  2666,  2666,   224,   258,   162,
    2951,  2666,   260,  2666,   486,   486,   213,  -818,   213,  -818,
     213,  -818,   213,   213,   213,   213,  -818,  -818,  -818,  -818,
    -818,   213,  -818,   213,  -818,   213,   213,   213,   213,   213,
    -818,   213,   213,   213,   213,  -818,   213,   213,   213,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
      30,  -818,    54,  -818,  -818,  -818,   140,  2761,   213,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,    73,  -818,  -818,  -818,  -818,  -818,    96,
    -818,   305,  -818,  -818,  -818,   314,  -818,  -818,  -818,   178,
    -818,  -818,  -818,  -818,   314,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,   314,  -818,  -818,  -818,   314,  -818,   314,
    -818,   314,  -818,   314,  -818,  -818,  -818,   314,  -818,  -818,
    -818,  -818,   314,  -818,  -818,  -818,  -818,  -818,  -818,   314,
    -818,  -818,  -818,   314,  -818,  -818,  -818,  -818,   314,  -818,
    -818,  -818,   314,  -818,  -818,  -818,  -818,   314,  -818,   314,
    -818,  -818,   314,  -818,    44,   154,  -818,   314,  -818,  -818,
    -818,   314,  -818,  -818,   314,  -818,   314,  -818,  -818,  -818,
    -818,   314,  -818,  -818,  -818,   314,  -818,  -818,  -818,  -818,
    -818,   314,  -818,  -818,   314,  -818,  -818,  -818,   314,  -818,
    -818,  -818,  -818,  -818,   314,  -818,  -818,  -818,   314,  -818,
    -818,  -818,  -818,  2856,   486,  3331,   486,  2666,   486,  2666,
    2666,  2666,  -818,  2666,   486,  2666,   486,   258,   486,  3331,
    3331,  3331,  3331,  3331,   213,  3331,  3331,  3331,  3331,   213,
    2856,  3331,  3331,  -818,  -818,   486,  -818,  -818,  -818,  -818,
    -818,  -818,   213,  -818,   213,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,   213,   213,  -818,   213,   213,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,   213,  -818,  -818,
     213,   213,  -818,   213,  -818,   213,  -818,   213,  -818,   213,
     213,  -818,  -818,  -818,  3426,  -818,  -818,   104,  -818,  -818,
    -818,   213,  -818,   213,  -818,  -818,   213,   213,  -818,  -818,
    -818,   213,   213,   213,  -818,  -818,   213,  -818,  -818,  -818,
     213,  -818,   346,  2666,   213,   363,  -818,   210,  -818,   415,
    -818,   449,  -818,   477,  -818,   490,  -818,   506,  -818,   518,
    -818,   549,  -818,   564,  -818,   573,  -818,   627,  -818,   640,
    -818,   648,  -818,   236,  -818,   242,  -818,   256,  -818,   654,
    -818,   659,  -818,   660,  -818,   564,  -818,   486,   486,   213,
     213,   213,   213,   142,   486,   486,   262,   486,   160,   563,
     213,   213,  -818,    44,  -818,  3046,   213,  3426,   164,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  1402,  1637,  2154,
     263,  -818,  -818,   177,  -818,   141,   213,   271,  -818,  -818,
     283,   323,  -818,  -818,  -818,   202,  -818,  3426,  -818,   213,
     582,   486,   174,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,   213,  -818,   317,   290,   213,
    -818,  -818,  -818,   290,  -818,  -818,   227,  -818,   486,  -818,
     400,  -818,   264,  -818,  2515,   213,   213,   329,  1172,  -818,
    -818,  -818,  -818,   675,  -818,   296,   475,   393,   510,   411,
     453,  1084,  2007,   366,  1918,  1955,   966,  2226,   842,   327,
     331,   967,   486,   486,   486,   486,   213,   140,   382,  -818,
     213,   213,  -818,  -818,   541,  2666,   541,   486,  -818,  -818,
    -818,   314,  -818,  -818,  3426,  -818,  -818,  -818,  -818,  -818,
     213,   213,  -818,  3331,  2666,  -818,  -818,  -818,    10,  -818,
    -818,  -818,  -818,  -818,  -818,   486,   422,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,   213,   213,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  3331,  3331,   486,   486,
    -818,  -818,  -818,  -818,  -818,    57,   314,  -818,  -818,   213,
     213,  -818,   400,   400,   213,   663,   663,   664,  -818,   667,
    -818,   486,  -818,   486,   486,   486,   419,  -818,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   486,   486,   486,   486,   486,   486,   486,
     486,   486,  2951,  2951,   486,   486,   486,   486,   486,   486,
     486,   486,   486,   213,   213,   351,  -818,   668,  -818,   353,
    1418,  -818,  -818,   357,   359,   371,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,   381,
    -818,  -818,  -818,   672,  -818,  -818,  -818,  -818,  -818,  -818,
     213,   213,   213,   213,   213,   213,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,   486,  -818,  -818,
     400,   213,   486,   486,   486,   486,  -818,   213,  -818,  -818,
    -818,   213,   486,   486,   213,    59,  3331,    59,  3331,  3331,
    3331,   486,   213,   401,  2329,   781,   886,   486,   486,   297,
      89,  -818,  -818,   410,  -818,  -818,   694,  -818,  -818,   427,
    -818,  -818,   700,  -818,   705,  -818,   660,  -818,   400,  -818,
     400,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  1555,   508,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,   164,   213,  -818,  -818,
    -818,  -818,   213,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
     404,   430,   486,   486,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,   486,   486,   486,   486,   486,
     400,   400,   486,   486,   486,   486,   486,   486,   486,   486,
    1764,   167,   567,   498,   407,   437,   520,  -818,  -818,  -818,
    -818,  -818,  -818,   213,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,   400,
    -818,  -818,   486,   537,   486,   486,   457,  3141,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,   400,  -818,  -818,   486,    51,   118,   133,   156,
    -818,  3236,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,   469,   214,   486,   486,   486,
     400,  -818,   788,   302,  1152,  3426,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,   486,
     486,   486
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     792,     0,     2,     3,     0,   759,     1,   651,   652,     0,
       0,   654,     0,   765,     0,     0,   763,     0,     0,     0,
       0,   764,     0,     0,   768,   766,   767,   769,     0,   770,
     771,   772,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     761,   762,     0,   773,   804,   808,   621,   794,   805,   799,
     795,   796,   621,   811,   798,   817,   816,   821,   806,   815,
     820,   801,   802,   797,   800,   812,   813,   807,   818,   819,
     814,   822,   803,     0,     0,     0,     0,     0,     0,     0,
     629,   760,   653,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   573,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   793,   824,     0,   621,     0,   621,
       0,   621,     0,     0,     0,     0,   791,   789,   790,   788,
     621,     0,   621,     0,   621,     0,     0,     0,     0,     0,
     653,     0,     0,     0,     0,   653,     0,     0,     0,   780,
     779,   782,   783,   784,   778,   777,   776,   775,   787,   774,
       0,   781,     0,   786,   785,   621,     0,   631,   655,   681,
       5,   671,   682,   683,   684,   685,   686,   687,   688,   689,
     690,   691,   692,   693,   694,   695,   696,   697,   698,   699,
     700,   701,   702,   703,   704,   705,   706,   707,   708,   709,
     710,   711,   712,   713,   714,   715,   668,   716,   717,   718,
     719,   720,   721,   722,   723,   724,   725,   726,   727,   728,
     729,   730,   731,   732,   733,   734,   735,   736,   737,   738,
     739,   740,   741,   742,   743,   744,   745,   672,   746,   673,
     674,   675,   676,   747,   677,   678,   679,   680,   748,   749,
     750,   653,   610,     0,    10,   751,   669,   670,   653,     0,
      17,     0,    99,   752,   615,     0,   138,   653,   653,     0,
      47,   653,   653,   529,     0,   525,   661,   664,   662,   666,
     667,   665,   660,     0,    58,   658,   663,     0,   243,     0,
      60,     0,   239,     0,   237,   600,   170,     0,   167,   653,
     612,   565,     0,   561,   563,   611,   653,   653,   534,     0,
     530,   653,   545,     0,   541,   653,   601,   540,     0,   537,
     602,   653,     0,    25,   653,   653,   550,     0,   546,     0,
      56,   577,     0,   213,     0,     0,   236,     0,   233,   653,
     607,     0,    49,   653,     0,   535,     0,    62,   653,   653,
     219,     0,   215,    74,    76,     0,    45,   653,   653,   653,
     114,     0,   109,   558,     0,   555,   559,   571,     0,   241,
     605,   606,   603,   209,     0,   206,   653,   604,     0,   191,
     623,   622,   653,     0,   809,     0,   810,     0,   823,     0,
       0,     0,   180,     0,   825,     0,   826,     0,   827,     0,
       0,     0,     0,     0,   445,     0,     0,     0,     0,   452,
       0,     0,     0,   621,   621,   828,   653,   653,   829,   653,
     630,   653,     7,   621,   609,   621,   621,   101,   100,   620,
     618,   139,   621,   621,   613,   614,   621,   528,   527,   526,
      59,   653,   244,    61,   240,   238,   168,   169,   562,   653,
     533,   532,   531,   543,   542,   544,   538,   539,    26,   549,
     548,   547,    57,   214,     0,   580,   574,     0,   576,   584,
     234,   235,    50,   608,   536,    63,   218,   217,   216,   653,
      46,   111,   113,   112,   110,   556,   557,   569,   242,   207,
     208,   192,     0,   627,   626,     0,   150,     0,   140,     0,
     124,     0,   172,     0,   551,     0,   182,     0,   566,     0,
     518,     0,    65,     0,   368,     0,   357,     0,   334,     0,
     266,     0,   245,     0,   285,     0,   298,     0,   314,     0,
     454,     0,   383,     0,   430,     0,   370,   447,   447,   647,
     649,   634,   632,     6,    13,    20,   104,   616,     0,     0,
     659,   564,   589,   579,   583,     0,    75,     0,   572,   653,
     636,   624,   625,   628,   621,   151,   149,   621,   621,   126,
     125,   621,   173,   171,   621,   553,   552,   621,   183,   181,
     621,   211,   210,   621,   520,   519,   621,    69,    68,   621,
     372,   369,   621,   359,   358,   621,   336,   335,   621,   268,
     267,   621,   247,   246,   621,   621,   621,   621,   456,   455,
     621,   385,   384,   621,   434,   431,   371,     0,     0,     0,
     633,   653,    27,    12,    27,    19,     0,     0,   619,   621,
       0,   578,   581,   585,   582,   587,   560,     0,   570,   638,
     156,   142,     0,   175,   175,   185,   175,   522,    71,   374,
     361,   338,   270,   249,   286,   300,   316,   458,   387,   436,
     446,   621,   621,   621,   258,   259,   260,   261,   262,   263,
     264,   265,   621,   453,   653,   629,   653,     0,    51,     0,
      14,    15,    16,    51,    21,   621,     0,   102,   617,    48,
     656,   586,     0,   567,     0,     0,     0,     0,   153,   154,
     621,   155,   221,     0,   127,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   449,   450,   451,   448,   650,     0,     0,     8,
       0,     0,   621,   621,    66,     0,    66,    22,   653,   653,
     108,     0,   103,   657,     0,   588,   646,   637,   640,   653,
     629,   629,   645,     0,     0,   152,   159,   621,     0,   162,
     621,   621,   621,   158,   157,   194,   220,   141,   147,   148,
     146,   621,   144,   145,   174,   178,   179,   176,   177,   554,
     184,   189,   190,   186,   187,   188,   212,   521,   523,   524,
      70,    72,    73,   373,   381,   382,   380,   621,   621,   378,
     379,   621,   360,   365,   366,   364,   621,   621,   621,   337,
     345,   346,   344,   621,   341,   350,   351,   352,   353,   356,
     621,   348,   349,   354,   355,   621,   342,   343,   269,   277,
     278,   276,   621,   621,   621,   621,   621,   621,   621,   275,
     274,   621,   248,   251,   252,   250,   621,   621,   621,   621,
     621,   284,   296,   297,   295,   621,   621,   290,   292,   621,
     293,   294,   621,   299,   312,   313,   311,   621,   621,   305,
     304,   621,   307,   308,   309,   310,   621,   315,   327,   328,
     326,   621,   621,   621,   621,   621,   621,   621,   322,   323,
     324,   325,   621,   321,   320,   457,   462,   463,   461,   621,
     621,   621,   621,   621,     0,     0,   386,   391,   392,   390,
     621,   621,   621,   621,   435,   439,   440,   438,   621,   621,
     621,   621,   621,   648,   653,   653,     0,     0,    28,    29,
      52,    53,    54,    55,    78,    64,     0,    23,    78,   107,
     106,   105,   656,   656,   639,     0,     0,     0,   224,     0,
     197,   164,   165,   160,   161,   163,   193,   222,   143,   376,
     375,   377,   363,   367,   362,   340,   347,   339,   272,   282,
     279,   283,   280,   281,   271,   273,   254,   253,   255,   256,
     257,   288,   289,   287,   291,   302,   303,   301,   306,   318,
     329,   330,   333,   331,   332,   317,   319,   460,   464,   465,
     466,   459,     0,     0,   389,   393,   394,   388,   437,   441,
     442,   443,   444,   635,     9,     0,    31,     0,    37,     0,
      77,   621,    24,     0,     0,     0,   653,   643,   641,   642,
     621,   225,   621,   621,   198,   196,   621,   413,   414,     0,
     653,   396,   397,     0,   653,   621,   621,    39,    38,   653,
       0,     0,     0,     0,     0,     0,   621,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    67,   653,   590,
     656,   647,   227,   223,   200,   195,   621,   412,   621,   399,
     398,   395,    32,    41,    11,     0,     0,     0,     0,     0,
       0,    79,    18,     0,     0,     0,     0,   420,   401,     0,
       0,   417,   418,     0,   569,   653,     0,    90,   474,     0,
     467,   653,     0,   115,     0,   128,     0,   432,   656,   591,
     656,   644,   226,   231,   232,   230,   621,   229,   199,   204,
     205,   203,   621,   202,     0,     0,    30,    36,    33,    34,
      35,    40,    44,    42,    43,   621,   568,   416,   621,    92,
      91,   621,   473,   621,   117,   116,   621,   130,   129,   433,
       0,     0,   228,   201,   415,   424,   425,   423,   621,   621,
     621,   621,   621,   621,   400,   410,   411,   621,   405,   406,
     407,   404,   408,   409,   621,   420,    94,   469,   119,   132,
     656,   656,   422,   426,   429,   427,   428,   421,   403,   402,
       0,     0,     0,     0,     0,     0,     0,   419,    93,    97,
      98,   621,    96,     0,   468,   470,   471,   118,   122,   123,
     121,   621,   131,   136,   137,   135,   621,   133,   599,   656,
     592,   595,    95,     0,   120,   134,     0,     0,   484,   497,
     477,   506,   621,   653,   475,   476,   653,   481,   653,   483,
     653,   482,   656,   596,   597,   472,     0,     0,     0,     0,
     594,   656,   621,   479,   478,   621,   486,   485,   621,   499,
     498,   621,   508,   507,   593,     0,     0,   488,   501,   510,
     656,   480,     0,     0,     0,     0,   487,   489,   492,   493,
     494,   495,   496,   621,   491,   500,   502,   505,   621,   504,
     509,   621,   512,   513,   514,   515,   516,   517,   598,   490,
     503,   511
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -818,  -818,  -818,   185,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,   -40,  -818,   -22,   -33,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -755,  -818,   -10,
    -818,  -512,   -58,  -818,   649,  -818,   652,  -818,    64,  -818,
     259,   -73,  -818,  -818,  -287,  -818,  -818,   267,  -818,  -251,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -515,  -818,  -818,
    -818,  -818,  -818,   107,  -818,  -818,  -818,  -818,  -818,  -818,
     -13,  -818,  -818,  -818,  -818,  -818,  -818,  -704,  -818,   -62,
    -818,  -551,  -818,  -818,  -818,  -818,  -818,  -818,  -818,    -5,
    -818,    -1,  -818,  -818,  -233,  -818,     4,  -818,  -818,  -818,
    -818,     3,  -818,  -818,  -262,  -818,  -818,  -818,  -818,  -381,
    -818,    27,  -818,  -818,    33,  -818,    39,  -818,  -818,  -818,
     -21,  -818,  -818,  -818,  -818,  -346,  -818,  -818,     2,  -818,
      12,  -818,  -594,  -818,  -554,  -818,   -88,  -818,  -818,  -611,
    -818,   -64,  -818,  -818,   -63,  -818,  -818,  -818,   -43,  -818,
    -818,   -39,  -818,  -818,   -32,  -818,  -818,  -818,  -818,  -818,
     -30,  -818,  -818,  -818,   -29,  -818,   -28,   230,  -818,  -818,
     690,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -413,  -818,   -38,  -818,  -818,  -313,
    -818,  -818,    83,   268,  -818,    86,  -818,   -86,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,
    -818,  -818,     0,  -818,  -818,  -818,  -470,  -818,  -818,  -543,
    -818,  -818,  -697,  -818,  -675,  -818,  -818,  -646,  -818,  -818,
    -252,  -818,  -818,   -77,  -818,  -818,  -726,  -818,  -818,  -818,
      91,  -818,  -818,  -818,  -660,  -276,  -289,  -518,  -818,  -818,
    -818,  -818,   279,   124,  -818,  -818,   280,  -818,  -818,  -818,
     182,  -818,  -818,  -818,  -400,  -818,  -818,  -818,   219,   736,
    -818,  -818,  -818,   -74,   -93,  -310,  1211,  -818,  -818,  -818,
     489,   146,  -818,  -818,  -818,  -411,  -818,  -818,  -818,  -818,
    -818,  -105,  -818,  -818,  -229,   116,    -4,  1413,    56,  -817,
     127,  -818,   678,   402,  -818,   152,   -34,   -42,  -818,  -818,
    -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818,  -818
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   261,   262,   553,   935,   263,     2,   632,   633,
     269,     3,   634,   635,   946,   690,   332,    54,   688,   742,
    1025,  1109,  1027,   743,  1058,  1110,   365,    55,   279,    56,
     351,    57,   744,   339,   940,   293,   941,   299,   785,   356,
     786,   944,   521,   945,   144,   598,   720,   366,   489,  1029,
    1030,  1066,  1116,  1067,  1160,  1211,   271,    62,   438,   751,
     637,   752,   371,  1177,   372,  1122,  1068,  1165,  1213,   509,
    1178,   580,  1124,  1069,  1168,  1214,   275,    64,   507,   671,
     713,   127,   505,   576,   707,   708,   767,   768,   307,    65,
     308,   136,   511,   583,   715,   401,   137,   515,   589,   717,
     388,    66,   709,   966,   710,   959,  1045,  1106,   384,    67,
     385,   138,   592,   342,    68,   361,    69,   362,   711,   776,
     712,   957,  1042,  1105,   347,    70,   348,   303,   787,   301,
     788,   378,    73,   297,    74,   531,   672,   613,   725,   673,
     529,   674,   610,   724,   675,   533,   726,   535,   676,   727,
     537,   677,   728,   527,   678,   607,   723,   830,   831,   525,
    1180,   604,   722,   523,   679,   545,   680,   601,   721,   541,
     681,   622,   730,  1052,  1053,   921,  1090,  1145,  1048,  1049,
     922,  1112,  1113,  1073,  1144,   543,  1181,  1126,  1074,   625,
     731,   170,   171,   627,   172,   173,   539,  1182,   619,   729,
    1119,  1076,  1212,  1120,  1252,  1253,  1254,  1274,  1255,  1256,
    1257,  1277,  1292,  1258,  1259,  1280,  1293,  1260,  1261,  1283,
    1294,   519,  1183,   595,   719,   284,    75,   285,   319,    76,
     320,   354,    77,   328,    78,   329,   323,    79,   324,   337,
      80,   338,   513,   682,   586,   374,    81,   375,   496,   312,
      82,   313,   459,   517,   648,  1115,   568,   376,   497,   343,
     344,   345,   475,   476,   563,   478,   479,   565,   644,   701,
     641,   952,  1129,  1240,  1241,  1247,  1271,  1130,   330,   331,
     386,   387,   352,   264,   377,   276,   441,   442,   639,   443,
     124,   390,   502,   503,   572,   176,   430,   630,   571,   704,
     759,  1038,   760,   761,   629,   428,   391,     4,   177,   754,
     294,   451,   295,   265,   266,   267,   268,   392,    83,    84,
      85,    86,    87,    88,    89,    90,    91,   175,   140,     5
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      11,   159,   903,   174,   766,    92,    92,    71,    92,   139,
      92,    92,   314,    92,    92,    92,    92,    72,    92,    92,
     270,   272,   129,   280,    92,   160,   161,   850,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,   162,   640,    92,   646,
     163,   169,   883,   899,   278,   180,     7,   164,     8,   165,
     166,   167,   277,     6,   305,    93,    94,   180,    95,    60,
      96,    97,   131,    98,    99,   100,   101,   128,   102,   103,
     353,   884,   900,   506,   104,   126,    92,   142,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   180,   130,   123,   180,
     542,   817,    63,   845,   858,   305,   141,   894,   912,   782,
      20,   692,    92,     7,    92,     8,    92,   143,    92,    92,
      92,    92,   867,   879,   423,  1034,  1035,    92,     7,    92,
       8,    92,    92,    92,    92,    92,   180,    92,    92,    92,
      92,   474,    92,    92,    92,  1272,    43,  1273,   424,   783,
      45,     7,   781,     8,    15,     7,   647,     8,   477,   824,
     180,   273,   868,   880,    11,   866,   878,   433,   810,     7,
     837,     8,   393,   871,   395,   273,   397,    12,   399,   400,
     402,   403,   321,   334,  1151,   180,   335,   405,    20,   407,
     435,   409,   410,   411,   412,   413,   180,   415,   416,   417,
     418,   564,   420,   421,   422,   305,   326,   962,     7,     7,
       8,     8,  1275,   689,  1276,   273,   281,   298,   300,   302,
     304,   282,   180,    36,    43,   180,   333,  1278,   340,  1279,
      46,    41,   355,   357,   426,   631,   427,   367,   379,   368,
     389,   809,   369,   836,   849,   180,   870,   882,   898,   689,
    1281,  -575,  1282,  1103,    61,   638,   180,     7,   180,     8,
     363,   647,  1218,   273,   737,   316,   380,   381,   382,   714,
     317,   274,   446,   283,   292,   292,   292,   292,   292,   306,
     311,   318,   322,   327,   292,   336,   292,   341,   346,   350,
     292,   292,   360,   364,   370,   373,   292,   383,   292,  -589,
    -589,  1170,   358,  1171,   577,   748,  -653,   359,  -653,  1291,
     749,   508,     7,   309,     8,   739,   325,    20,    20,    22,
      23,   145,    19,   740,   741,   524,   526,   528,   530,   532,
     614,   534,   536,   538,   540,   636,   615,   544,   546,   955,
     956,   278,   914,   147,  1147,  1152,   148,   133,    20,   277,
     616,    41,    20,    43,    43,  -653,   686,   149,    45,   130,
     145,   474,   700,   130,   151,   152,   697,   153,   278,    47,
      48,   562,   133,  1215,  1216,  -653,   277,     7,   699,     8,
     934,   146,   147,    17,    43,   148,    19,    20,    43,   915,
      46,   784,  1146,   128,    46,   753,   156,  1305,   130,   436,
      11,   437,   141,   151,   152,    11,   153,    33,   439,  1072,
     440,   716,  1246,   718,    20,   158,    22,    23,    11,    14,
      11,   700,   905,    43,   765,  1114,   916,  1114,    20,    46,
      11,    11,    20,    11,    11,  1270,   705,   141,  1191,   130,
     569,   706,   570,    11,  1285,  1055,    11,    11,  1059,    11,
      43,    11,  1078,    11,   158,    11,    11,   574,  1079,   575,
    1192,   819,   645,  1295,    43,   885,   901,    11,    43,    11,
      46,  1080,    11,    11,    20,  1086,  1128,    11,    11,    11,
      52,     7,    92,     8,  1050,  1054,    11,   273,   790,  1193,
      11,   296,   296,   296,   296,   296,    20,   315,    22,    23,
    1237,   296,  1232,   296,  1155,  1200,   797,   296,   296,   578,
      43,   579,   315,   296,   510,   296,   512,   514,   516,    20,
     518,  1161,   520,  1179,  1239,    17,   685,  1201,    19,    20,
     130,    20,    43,    22,    23,    11,    11,    11,    11,  1238,
     130,   125,   567,   581,   703,   582,    11,    11,   800,    33,
      34,    35,    11,   133,  1262,    43,  1304,  1309,     7,    18,
       8,    46,    20,    42,   273,    43,  1290,    43,   274,   296,
     789,   584,   292,   585,   292,   292,   292,     7,   292,     8,
     292,  1188,   364,   273,   587,   694,   588,  1148,    20,  1179,
      38,   693,  1190,  1227,  1223,   274,   705,     7,    43,     8,
     590,   691,   591,  1184,   706,   796,   394,  1301,   396,  1316,
     398,  1248,   593,  1249,   594,   695,  1250,    41,  1251,   404,
     573,   406,    92,   408,    43,   746,   844,   857,   847,   860,
     893,   911,   896,   930,   811,    11,  1302,   851,  1317,   872,
     886,   902,   913,   596,    58,   597,     9,    59,  1031,   825,
     826,   953,   843,   856,   425,   132,   892,   910,   599,   929,
     600,   133,  1224,   948,   522,  1189,    10,   602,   134,   603,
     827,    92,   135,    11,   828,    92,   846,   859,   292,   958,
     895,   829,   696,   832,   833,   834,  1221,  1033,  1297,  1306,
    1312,    92,    92,   769,  1046,    19,    20,   770,   807,   816,
     823,   842,   771,   865,   877,   891,   909,   920,   928,   794,
     793,   808,   818,   835,   848,  1143,   869,   881,   897,   795,
     923,   605,    11,   606,    11,   772,    92,    92,  1298,  1307,
    1313,   773,    43,  1311,   608,   745,   609,   774,    46,  1300,
     126,  1315,   611,    48,   612,   967,    92,    92,   617,  1137,
     618,   763,   764,   620,   623,   621,   624,  1036,  1040,  1037,
    1041,  1043,  1056,  1044,  1057,   626,  1088,   778,  1089,   168,
     777,   791,  1210,   798,   801,   804,   813,   820,   839,   853,
     862,   874,   888,   906,   917,   925,   936,   937,  1158,   296,
    1159,   296,   296,   296,  1163,   296,  1164,   296,   942,  1166,
     942,  1167,    20,  1169,   931,    17,   628,   932,    19,   904,
     780,  1121,  1299,   130,  1314,  1156,   755,   702,   806,   815,
     822,   841,   855,   864,   876,   890,   908,   919,   927,    33,
      34,    35,   642,   133,    40,   643,   145,  1263,    43,   349,
     758,  1039,  1104,   933,    46,   431,   762,     0,     0,     0,
       0,     0,  1026,  1028,     0,    47,    48,     0,   147,    17,
       0,   148,   947,    20,     0,     0,   315,     0,     0,     0,
       0,   750,   149,     0,   130,     0,  1132,     0,    32,   151,
     152,   960,   153,  1296,    34,    35,     0,   133,     0,   154,
      37,     0,     0,     0,     0,   296,     0,     0,     0,    43,
      92,    92,   547,   548,     0,    46,     0,    20,   128,    47,
       0,   156,   554,   141,   555,   556,     0,     0,   130,     0,
     292,   557,   558,     0,     0,   559,     0,     0,   157,     0,
     158,     0,  1071,     0,  1075,    11,    11,   887,     0,   292,
      11,     0,     0,    43,     0,     0,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,    49,   315,  1070,   315,
    1012,  1013,   779,     0,     0,     0,   792,     0,   799,   802,
     805,   814,   821,   840,   854,   863,   875,   889,   907,   918,
     926,  1138,     0,    17,     0,     0,    19,    20,    20,     0,
       0,     0,     0,   943,     0,   943,     0,   149,   130,   130,
       0,   685,   150,     0,     0,     0,     0,    33,     0,    11,
      11,   133,  1117,     0,  1123,  1125,  1127,   155,     0,     0,
       0,   315,     0,    43,    43,     0,     0,     0,     0,    46,
      46,   126,     0,   128,    48,     0,     0,   141,     0,   315,
       0,     0,     0,     0,     0,     0,    92,    92,    92,    92,
      92,    92,     0,   650,     0,     0,   651,   652,  1194,     0,
     653,   861,   924,   654,     0,     0,   655,    11,     0,   656,
       0,     0,   657,    11,     0,   658,     0,    11,   659,     0,
      11,   660,  1136,  1142,   661,     0,     0,   662,    11,     0,
     663,     0,     0,   664,   665,   666,   667,     0,     0,   668,
       0,    17,   669,     0,     0,    20,  1095,  1096,  1097,  1098,
    1099,  1100,     0,     0,     0,     0,   130,     0,   698,     0,
       0,     0,  1187,     0,     0,    33,     0,     0,     0,   133,
       0,     0,     0,     0,     0,     0,     0,   296,     0,     0,
       0,    43,     0,    11,  1264,     0,   315,    46,    11,     0,
     732,   733,   734,     0,     0,   141,   296,     0,     0,  1133,
    1139,   735,     0,  1149,  1153,     0,     0,     0,  1284,    17,
       0,     0,    19,     0,   747,     0,     0,     0,     0,   803,
       0,     0,     0,     0,    14,     0,     0,  1047,  1051,   775,
    1231,  1236,  1318,    33,    34,    35,     0,     0,  1175,  1185,
       0,    28,  1135,  1141,     0,  1303,  1308,     0,     0,    92,
       0,   132,     0,     0,     0,     0,     0,   126,     0,     0,
      48,   938,   939,    39,   134,     0,     0,     0,   135,     0,
       0,    44,     0,     0,     0,     0,     0,  -166,     0,     0,
       0,     0,     0,     0,     0,     0,   961,  1310,     0,   963,
     964,   965,    11,    11,    11,    11,     0,     0,     0,     0,
     968,     0,     0,     0,  1175,  1219,  1225,  1228,  1233,  1243,
    1111,     0,  1118,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   969,   970,     0,     0,
     971,     0,     0,     0,     0,   972,   973,   974,     0,     0,
       0,     0,   975,     0,     0,     0,     0,     0,  1222,   976,
    1230,  1235,     0,     0,   977,     0,     0,     0,     0,     0,
       0,   978,   979,   980,   981,   982,   983,   984,     0,     0,
     985,     0,     0,     0,     0,   986,   987,   988,   989,   990,
       0,     0,     0,     0,   991,   992,     0,     0,   993,     0,
       0,   994,     0,     0,  1134,  1140,   995,   996,  1150,  1154,
     997,     0,     0,     0,     0,   998,     0,     0,     0,     0,
     999,  1000,  1001,  1002,  1003,  1004,  1005,     0,     0,     0,
       0,  1006,     0,     0,     0,     0,     0,     0,  1007,  1008,
    1009,  1010,  1011,  1176,  1186,     0,   145,     0,     0,  1014,
    1015,  1016,  1017,     0,   315,   315,     0,  1018,  1019,  1020,
    1021,  1022,   145,     0,     0,     0,     0,     0,   147,     0,
       0,   148,     0,     0,     0,     0,     0,     0,     0,  1060,
       0,     0,   149,     0,   147,     0,     0,   148,     0,   151,
     152,     0,   153,  1061,  1062,     0,  1063,   133,   149,  1064,
       0,     0,     0,     0,     0,   151,   152,     0,   153,  1176,
    1220,  1226,  1229,  1234,     0,   154,     0,     0,   128,     0,
       0,   156,     0,     0,     0,     0,     0,     0,     0,     0,
    1065,     0,     0,     0,   128,   449,     0,   156,     0,     0,
     158,     0,     0,     0,   450,   178,   315,   670,   452,     0,
     453,     0,   454,     0,   455,     0,   158,     0,   456,     0,
    1077,     0,     0,   458,     0,     0,     0,     0,     0,  1082,
     462,  1083,  1084,     0,   464,  1085,     0,     0,     0,   466,
       0,     0,     0,   468,  1092,  1093,     0,     0,   471,     0,
     472,     0,     0,   473,     0,  1101,     0,     0,   480,   145,
       0,     0,   482,   414,     0,   484,     0,   485,   419,     0,
       0,     0,   488,     0,     0,  1107,   490,  1108,     0,     0,
     146,   147,   494,     0,   148,   495,    20,     0,     0,   498,
       0,     0,     0,     0,     0,   499,     0,   130,     0,   501,
       0,     0,   151,   152,     0,   153,     0,     0,     0,     0,
       0,     0,   154,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,  1172,     0,     0,    46,     0,
       0,  1173,     0,     0,   156,     0,   141,     0,     0,     0,
       0,   145,     0,     0,  1195,     0,     0,  1196,     0,   315,
    1197,   157,  1198,   158,     0,  1199,     0,     0,     0,     0,
    1174,     0,     0,   147,     0,     0,   148,  1202,  1203,  1204,
    1205,  1206,  1207,   315,   432,     0,  1208,   149,     0,     0,
       0,   434,     0,  1209,   151,   152,     0,   153,     0,     0,
     444,   445,   133,     0,   447,   448,     0,   315,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1242,     0,     0,   128,     0,     0,   156,     0,     0,     0,
    1244,     0,   457,     0,     0,  1245,     0,     0,     0,   460,
     461,     0,     0,     0,   463,   158,     0,     0,   465,     0,
       0,  1265,   683,     0,   467,     0,     0,   469,   470,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1286,   481,     0,  1287,     0,   483,  1288,   145,     0,
    1289,   486,   487,     0,     0,     0,     0,     0,     0,     0,
     491,   492,   493,     0,     0,     0,     0,     0,     0,   146,
     147,     0,  1319,   148,     0,    20,     0,  1320,     0,   500,
    1321,     0,     0,     0,     0,   504,   130,     0,     0,     0,
       0,   151,   152,     0,   153,     0,     0,     0,     0,     0,
       0,   154,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,    46,     0,   549,
     550,     0,   551,   156,   552,   141,     0,     0,     0,     0,
       0,     0,     0,     0,    -4,     0,     0,     0,     0,     0,
     157,     0,   158,     0,   560,     0,     0,     0,     0,  1217,
       0,     0,   561,     0,    12,    13,    14,    15,    16,     0,
       0,    17,    18,     0,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,  -755,    30,    31,     0,
      32,     0,   566,  -756,     0,    33,    34,    35,     0,  -756,
      36,     0,    37,    38,     0,    39,  -756,    40,    41,    42,
    -756,    43,   145,    44,  -758,    45,     0,    46,     0,  -753,
    -754,    47,    48,     0,    49,  -757,    50,    51,     0,     0,
       0,     0,     0,     0,   147,    17,     0,   148,     0,    20,
       0,    52,     0,     0,     0,     0,    53,     0,   149,   145,
     130,     0,   951,     0,     0,   151,   152,     0,   153,     0,
       0,     0,     0,   133,     0,   154,     0,     0,     0,     0,
       0,   147,   649,    42,   148,    43,    20,     0,     0,     0,
       0,    46,     0,     0,   128,   149,     0,   156,     0,   141,
       0,     0,   151,   152,     0,   153,     0,     0,     0,     0,
       0,   145,   154,     0,   157,     0,   158,     0,     0,     0,
       0,     0,    43,   838,     0,   145,     0,     0,    46,     0,
       0,   128,     0,   147,   156,     0,   148,     0,    20,     0,
       0,     0,     0,     0,   687,     0,   146,   147,     0,   130,
     148,   157,     0,   158,   151,   152,     0,   153,     0,     0,
     852,   149,     0,     0,     0,     0,   150,     0,   151,   152,
       0,   153,     0,     0,    43,     0,     0,     0,   154,     0,
      46,   155,     0,     0,     0,     0,   156,     0,   141,     0,
       0,     0,     0,     0,     0,     0,     0,   736,     0,   738,
     156,     0,     0,     0,     0,   158,     0,     0,     0,     0,
       0,     0,   812,     0,     0,     0,     0,   157,     0,   158,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1032,   179,     0,
       0,   949,   950,     0,   181,   310,     0,     0,     0,     0,
       0,     0,   954,     0,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,    17,     0,     0,    19,    20,     0,   684,
       0,     0,     0,     0,     0,     0,     0,     0,   130,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,    35,
       0,   133,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,    46,
       0,   126,     0,     0,    48,     0,     0,   141,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   873,     0,   179,     0,     0,     0,     0,     0,   181,
     310,     0,     0,     0,     0,     0,     0,  1023,  1024,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,     0,     0,
       0,     0,     0,     0,  1131,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1081,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1087,     0,     0,     0,  1091,     0,     0,
       0,     0,  1094,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   179,
       0,     0,     0,     0,     0,     0,   273,     0,  1157,     0,
       0,     0,     0,     0,  1162,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   756,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
       0,   248,     0,     0,     0,     0,   253,     0,     0,     0,
       0,   258,   259,   260,     0,     0,     0,     0,     0,     0,
     757,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1266,     0,     0,  1267,
     179,  1268,     0,  1269,   180,   286,   181,   287,   288,     0,
       0,     0,   289,   290,   291,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   179,     0,     0,     0,   429,
     286,   181,   287,   288,     0,     0,     0,   289,   290,   291,
       0,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     179,     0,     0,     0,   180,     0,   181,   273,     0,     0,
       0,     0,     0,     0,     0,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   179,     0,     0,     0,   180,
       0,   181,   310,     0,     0,     0,     0,     0,     0,     0,
       0,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     179,     0,     0,     0,     0,     0,   181,   310,     0,     0,
     477,     0,     0,     0,     0,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   179,     0,     0,     0,     0,
       0,   181,   310,     0,     0,  1239,     0,     0,     0,     0,
       0,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     179,   753,     0,     0,     0,     0,   181,   310,     0,     0,
       0,     0,     0,     0,     0,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   179,     0,     0,     0,   180,
       0,   181,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     179,     0,     0,     0,     0,     0,   181,   310,     0,     0,
       0,     0,     0,     0,     0,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260
};

static const yytype_int16 yycheck[] =
{
       4,    89,   728,    89,   708,     9,    10,     5,    12,    86,
      14,    15,   105,    17,    18,    19,    20,     5,    22,    23,
      94,    95,    84,    97,    28,    89,    89,   724,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    89,   559,    52,   567,
      89,    89,   727,   728,    96,     8,     5,    89,     7,    89,
      89,    89,    96,     0,    17,     9,    10,     8,    12,     5,
      14,    15,    85,    17,    18,    19,    20,    76,    22,    23,
     114,   727,   728,   393,    28,    75,    90,    87,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     8,    42,    52,     8,
     420,   722,     5,   724,   725,    17,    81,   728,   729,   713,
      31,   633,   126,     5,   128,     7,   130,    70,   132,   133,
     134,   135,   726,   727,   104,   952,   953,   141,     5,   143,
       7,   145,   146,   147,   148,   149,     8,   151,   152,   153,
     154,   107,   156,   157,   158,   104,    67,   106,   104,   713,
      71,     5,   713,     7,    23,     5,   107,     7,    14,   723,
       8,    11,   726,   727,   178,   726,   727,   104,   721,     5,
     723,     7,   126,   726,   128,    11,   130,    20,   132,   133,
     134,   135,    94,    92,   105,     8,    95,   141,    31,   143,
     104,   145,   146,   147,   148,   149,     8,   151,   152,   153,
     154,   107,   156,   157,   158,    17,    18,   768,     5,     5,
       7,     7,   104,    82,   106,    11,    88,   100,   101,   102,
     103,    93,     8,    56,    67,     8,   109,   104,   111,   106,
      73,    64,   115,   116,   104,   103,   106,    85,   121,    87,
     123,   721,    90,   723,   724,     8,   726,   727,   728,    82,
     104,    14,   106,  1080,     5,   105,     8,     5,     8,     7,
      12,   107,   105,    11,   685,    88,    16,    17,    18,   105,
      93,    96,   104,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   107,
     108,  1128,    88,  1130,   104,    88,     5,    93,     7,   105,
      93,   395,     5,   104,     7,     8,   107,    31,    31,    33,
      34,     4,    30,    43,    44,   409,   410,   411,   412,   413,
     104,   415,   416,   417,   418,    83,   104,   421,   422,   760,
     761,   393,    21,    26,  1109,  1110,    29,    55,    31,   393,
     104,    64,    31,    67,    67,    54,   103,    40,    71,    42,
       4,   107,   108,    42,    47,    48,   105,    50,   420,    77,
      78,   474,    55,  1200,  1201,    74,   420,     5,   105,     7,
       8,    25,    26,    27,    67,    29,    30,    31,    67,    68,
      73,   105,   105,    76,    73,     5,    79,   105,    42,   104,
     414,   106,    81,    47,    48,   419,    50,    51,   104,  1030,
     106,   654,  1239,   656,    31,    98,    33,    34,   432,    22,
     434,   108,   105,    67,   105,  1095,   105,  1097,    31,    73,
     444,   445,    31,   447,   448,  1262,    24,    81,  1145,    42,
     104,    32,   106,   457,  1271,   104,   460,   461,   105,   463,
      67,   465,   105,   467,    98,   469,   470,   104,   109,   106,
    1145,   105,   565,  1290,    67,   727,   728,   481,    67,   483,
      73,   110,   486,   487,    31,   104,    85,   491,   492,   493,
      97,     5,   496,     7,  1012,  1013,   500,    11,   105,  1145,
     504,    99,   100,   101,   102,   103,    31,   105,    33,    34,
    1214,   109,   105,   111,   104,   111,   105,   115,   116,   104,
      67,   106,   120,   121,   397,   123,   399,   400,   401,    31,
     403,   104,   405,  1144,    14,    27,   629,   107,    30,    31,
      42,    31,    67,    33,    34,   549,   550,   551,   552,   112,
      42,    62,   496,   104,   647,   106,   560,   561,   105,    51,
      52,    53,   566,    55,   107,    67,  1292,  1293,     5,    28,
       7,    73,    31,    65,    11,    67,   107,    67,   393,   177,
     105,   104,   397,   106,   399,   400,   401,     5,   403,     7,
     405,  1145,   407,    11,   104,   635,   106,  1109,    31,  1210,
      59,   634,  1145,   105,    37,   420,    24,     5,    67,     7,
     104,   633,   106,   105,    32,   105,   127,  1292,   129,  1294,
     131,    84,   104,    86,   106,   635,    89,    64,    91,   140,
     503,   142,   636,   144,    67,   693,   724,   725,   724,   725,
     728,   729,   728,   731,   721,   649,  1292,   724,  1294,   726,
     727,   728,   729,   104,     5,   106,    54,     5,   945,   723,
     723,   754,   724,   725,   175,    49,   728,   729,   104,   731,
     106,    55,   105,   746,   407,  1145,    74,   104,    62,   106,
     723,   685,    66,   687,   723,   689,   724,   725,   503,   763,
     728,   723,   636,   723,   723,   723,  1211,   948,  1292,  1293,
    1294,   705,   706,   708,   966,    30,    31,   708,   721,   722,
     723,   724,   708,   726,   727,   728,   729,   730,   731,   717,
     717,   721,   722,   723,   724,  1106,   726,   727,   728,   717,
     730,   104,   736,   106,   738,   708,   740,   741,  1292,  1293,
    1294,   708,    67,  1294,   104,   689,   106,   708,    73,  1292,
      75,  1294,   104,    78,   106,   776,   760,   761,   104,  1105,
     106,   705,   706,   104,   104,   106,   106,   104,   104,   106,
     106,   104,   104,   106,   106,   545,   104,   713,   106,    89,
     105,   717,  1195,   719,   720,   721,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   740,   741,   104,   397,
     106,   399,   400,   401,   104,   403,   106,   405,   744,   104,
     746,   106,    31,  1126,   731,    27,   548,   731,    30,   728,
     713,  1097,  1292,    42,  1294,  1114,   702,   645,   721,   722,
     723,   724,   725,   726,   727,   728,   729,   730,   731,    51,
      52,    53,   563,    55,    63,   565,     4,  1247,    67,   113,
     704,   956,  1081,   737,    73,   177,   704,    -1,    -1,    -1,
      -1,    -1,   936,   937,    -1,    77,    78,    -1,    26,    27,
      -1,    29,   745,    31,    -1,    -1,   474,    -1,    -1,    -1,
      -1,   696,    40,    -1,    42,    -1,   105,    -1,    46,    47,
      48,   764,    50,   105,    52,    53,    -1,    55,    -1,    57,
      58,    -1,    -1,    -1,    -1,   503,    -1,    -1,    -1,    67,
     914,   915,   423,   424,    -1,    73,    -1,    31,    76,    77,
      -1,    79,   433,    81,   435,   436,    -1,    -1,    42,    -1,
     745,   442,   443,    -1,    -1,   446,    -1,    -1,    96,    -1,
      98,    -1,  1030,    -1,  1030,   949,   950,   105,    -1,   764,
     954,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,   565,  1030,   567,
     914,   915,   713,    -1,    -1,    -1,   717,    -1,   719,   720,
     721,   722,   723,   724,   725,   726,   727,   728,   729,   730,
     731,   105,    -1,    27,    -1,    -1,    30,    31,    31,    -1,
      -1,    -1,    -1,   744,    -1,   746,    -1,    40,    42,    42,
      -1,  1104,    45,    -1,    -1,    -1,    -1,    51,    -1,  1023,
    1024,    55,  1096,    -1,  1098,  1099,  1100,    60,    -1,    -1,
      -1,   629,    -1,    67,    67,    -1,    -1,    -1,    -1,    73,
      73,    75,    -1,    76,    78,    -1,    -1,    81,    -1,   647,
      -1,    -1,    -1,    -1,    -1,    -1,  1060,  1061,  1062,  1063,
    1064,  1065,    -1,   574,    -1,    -1,   577,   578,  1145,    -1,
     581,   105,   105,   584,    -1,    -1,   587,  1081,    -1,   590,
      -1,    -1,   593,  1087,    -1,   596,    -1,  1091,   599,    -1,
    1094,   602,  1105,  1106,   605,    -1,    -1,   608,  1102,    -1,
     611,    -1,    -1,   614,   615,   616,   617,    -1,    -1,   620,
      -1,    27,   623,    -1,    -1,    31,  1060,  1061,  1062,  1063,
    1064,  1065,    -1,    -1,    -1,    -1,    42,    -1,   639,    -1,
      -1,    -1,  1145,    -1,    -1,    51,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   745,    -1,    -1,
      -1,    67,    -1,  1157,  1247,    -1,   754,    73,  1162,    -1,
     671,   672,   673,    -1,    -1,    81,   764,    -1,    -1,  1105,
    1106,   682,    -1,  1109,  1110,    -1,    -1,    -1,  1271,    27,
      -1,    -1,    30,    -1,   695,    -1,    -1,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    22,    -1,    -1,  1012,  1013,   710,
    1213,  1214,  1295,    51,    52,    53,    -1,    -1,  1144,  1145,
      -1,    39,  1105,  1106,    -1,  1292,  1293,    -1,    -1,  1223,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      78,   742,   743,    61,    62,    -1,    -1,    -1,    66,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   767,   105,    -1,   770,
     771,   772,  1266,  1267,  1268,  1269,    -1,    -1,    -1,    -1,
     781,    -1,    -1,    -1,  1210,  1211,  1212,  1213,  1214,  1223,
    1095,    -1,  1097,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   807,   808,    -1,    -1,
     811,    -1,    -1,    -1,    -1,   816,   817,   818,    -1,    -1,
      -1,    -1,   823,    -1,    -1,    -1,    -1,    -1,  1211,   830,
    1213,  1214,    -1,    -1,   835,    -1,    -1,    -1,    -1,    -1,
      -1,   842,   843,   844,   845,   846,   847,   848,    -1,    -1,
     851,    -1,    -1,    -1,    -1,   856,   857,   858,   859,   860,
      -1,    -1,    -1,    -1,   865,   866,    -1,    -1,   869,    -1,
      -1,   872,    -1,    -1,  1105,  1106,   877,   878,  1109,  1110,
     881,    -1,    -1,    -1,    -1,   886,    -1,    -1,    -1,    -1,
     891,   892,   893,   894,   895,   896,   897,    -1,    -1,    -1,
      -1,   902,    -1,    -1,    -1,    -1,    -1,    -1,   909,   910,
     911,   912,   913,  1144,  1145,    -1,     4,    -1,    -1,   920,
     921,   922,   923,    -1,  1012,  1013,    -1,   928,   929,   930,
     931,   932,     4,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      -1,    -1,    40,    -1,    26,    -1,    -1,    29,    -1,    47,
      48,    -1,    50,    35,    36,    -1,    38,    55,    40,    41,
      -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    50,  1210,
    1211,  1212,  1213,  1214,    -1,    57,    -1,    -1,    76,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    -1,    -1,    76,   284,    -1,    79,    -1,    -1,
      98,    -1,    -1,    -1,   293,    92,  1104,   105,   297,    -1,
     299,    -1,   301,    -1,   303,    -1,    98,    -1,   307,    -1,
    1031,    -1,    -1,   312,    -1,    -1,    -1,    -1,    -1,  1040,
     319,  1042,  1043,    -1,   323,  1046,    -1,    -1,    -1,   328,
      -1,    -1,    -1,   332,  1055,  1056,    -1,    -1,   337,    -1,
     339,    -1,    -1,   342,    -1,  1066,    -1,    -1,   347,     4,
      -1,    -1,   351,   150,    -1,   354,    -1,   356,   155,    -1,
      -1,    -1,   361,    -1,    -1,  1086,   365,  1088,    -1,    -1,
      25,    26,   371,    -1,    29,   374,    31,    -1,    -1,   378,
      -1,    -1,    -1,    -1,    -1,   384,    -1,    42,    -1,   388,
      -1,    -1,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,  1136,    -1,    -1,    73,    -1,
      -1,  1142,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,
      -1,     4,    -1,    -1,  1155,    -1,    -1,  1158,    -1,  1247,
    1161,    96,  1163,    98,    -1,  1166,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    26,    -1,    -1,    29,  1178,  1179,  1180,
    1181,  1182,  1183,  1271,   261,    -1,  1187,    40,    -1,    -1,
      -1,   268,    -1,  1194,    47,    48,    -1,    50,    -1,    -1,
     277,   278,    55,    -1,   281,   282,    -1,  1295,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1221,    -1,    -1,    76,    -1,    -1,    79,    -1,    -1,    -1,
    1231,    -1,   309,    -1,    -1,  1236,    -1,    -1,    -1,   316,
     317,    -1,    -1,    -1,   321,    98,    -1,    -1,   325,    -1,
      -1,  1252,   105,    -1,   331,    -1,    -1,   334,   335,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1272,   349,    -1,  1275,    -1,   353,  1278,     4,    -1,
    1281,   358,   359,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     367,   368,   369,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    -1,  1303,    29,    -1,    31,    -1,  1308,    -1,   386,
    1311,    -1,    -1,    -1,    -1,   392,    42,    -1,    -1,    -1,
      -1,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,   426,
     427,    -1,   429,    79,   431,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     0,    -1,    -1,    -1,    -1,    -1,
      96,    -1,    98,    -1,   451,    -1,    -1,    -1,    -1,   105,
      -1,    -1,   459,    -1,    20,    21,    22,    23,    24,    -1,
      -1,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    41,    42,    43,    44,    -1,
      46,    -1,   489,    49,    -1,    51,    52,    53,    -1,    55,
      56,    -1,    58,    59,    -1,    61,    62,    63,    64,    65,
      66,    67,     4,    69,    70,    71,    -1,    73,    -1,    75,
      76,    77,    78,    -1,    80,    81,    82,    83,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    27,    -1,    29,    -1,    31,
      -1,    97,    -1,    -1,    -1,    -1,   102,    -1,    40,     4,
      42,    -1,   751,    -1,    -1,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    26,   569,    65,    29,    67,    31,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    76,    40,    -1,    79,    -1,    81,
      -1,    -1,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,     4,    57,    -1,    96,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    67,   105,    -1,     4,    -1,    -1,    73,    -1,
      -1,    76,    -1,    26,    79,    -1,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,   631,    -1,    25,    26,    -1,    42,
      29,    96,    -1,    98,    47,    48,    -1,    50,    -1,    -1,
     105,    40,    -1,    -1,    -1,    -1,    45,    -1,    47,    48,
      -1,    50,    -1,    -1,    67,    -1,    -1,    -1,    57,    -1,
      73,    60,    -1,    -1,    -1,    -1,    79,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   684,    -1,   686,
      79,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    96,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   946,     4,    -1,
      -1,   748,   749,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,   759,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    27,    -1,    -1,    30,    31,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,
      -1,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    78,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,   934,   935,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1036,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1050,    -1,    -1,    -1,  1054,    -1,    -1,
      -1,    -1,  1059,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1078,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    11,    -1,  1115,    -1,
      -1,    -1,    -1,    -1,  1121,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    96,    97,    98,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1253,    -1,    -1,  1256,
       4,  1258,    -1,  1260,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    16,    17,    18,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,     4,    -1,    -1,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    16,    17,    18,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       4,    -1,    -1,    -1,     8,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,     4,    -1,    -1,    -1,     8,
      -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,     4,    -1,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       4,     5,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,     4,    -1,    -1,    -1,     8,
      -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   114,   120,   124,   420,   442,     0,     5,     7,    54,
      74,   419,    20,    21,    22,    23,    24,    27,    28,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    41,
      43,    44,    46,    51,    52,    53,    56,    58,    59,    61,
      63,    64,    65,    67,    69,    71,    73,    77,    78,    80,
      82,    83,    97,   102,   130,   140,   142,   144,   147,   149,
     151,   153,   170,   176,   190,   202,   214,   222,   227,   229,
     238,   241,   243,   245,   247,   339,   342,   345,   347,   350,
     353,   359,   363,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   419,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   403,   403,    75,   194,    76,   192,
      42,   183,    49,    55,    62,    66,   204,   209,   224,   356,
     441,    81,   335,    70,   157,     4,    25,    26,    29,    40,
      45,    47,    48,    50,    57,    60,    79,    96,    98,   249,
     254,   257,   261,   264,   267,   273,   277,   279,   283,   299,
     304,   305,   307,   308,   310,   440,   408,   421,   420,     4,
       8,    10,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,   115,   116,   119,   396,   426,   427,   428,   429,   123,
     396,   169,   396,    11,   116,   189,   398,   429,   430,   141,
     396,    88,    93,   116,   338,   340,     9,    11,    12,    16,
      17,    18,   116,   148,   423,   425,   426,   246,   423,   150,
     423,   242,   423,   240,   423,    17,   116,   201,   203,   391,
      11,   116,   362,   364,   397,   426,    88,    93,   116,   341,
     343,    94,   116,   349,   351,   391,    18,   116,   346,   348,
     391,   392,   129,   423,    92,    95,   116,   352,   354,   146,
     423,   116,   226,   372,   373,   374,   116,   237,   239,   392,
     116,   143,   395,   429,   344,   423,   152,   423,    88,    93,
     116,   228,   230,    12,   116,   139,   160,    85,    87,    90,
     116,   175,   177,   116,   358,   360,   370,   397,   244,   423,
      16,    17,    18,   116,   221,   223,   393,   394,   213,   423,
     404,   419,   430,   421,   403,   421,   403,   421,   403,   421,
     421,   208,   421,   421,   403,   421,   403,   421,   403,   421,
     421,   421,   421,   421,   420,   421,   421,   421,   421,   420,
     421,   421,   421,   104,   104,   403,   104,   106,   418,     8,
     409,   425,   420,   104,   420,   104,   104,   106,   171,   104,
     106,   399,   400,   402,   420,   420,   104,   420,   420,   399,
     399,   424,   399,   399,   399,   399,   399,   420,   399,   365,
     420,   420,   399,   420,   399,   420,   399,   420,   399,   420,
     420,   399,   399,   399,   107,   375,   376,    14,   378,   379,
     399,   420,   399,   420,   399,   399,   420,   420,   399,   161,
     399,   420,   420,   420,   399,   399,   361,   371,   399,   399,
     420,   399,   405,   406,   420,   195,   398,   191,   396,   182,
     423,   205,   423,   355,   423,   210,   423,   366,   423,   334,
     423,   155,   160,   276,   396,   272,   396,   266,   396,   253,
     396,   248,   396,   258,   396,   260,   396,   263,   396,   309,
     396,   282,   398,   298,   396,   278,   396,   403,   403,   420,
     420,   420,   420,   117,   403,   403,   403,   403,   403,   403,
     420,   420,   397,   377,   107,   380,   420,   421,   369,   104,
     106,   411,   407,   423,   104,   106,   196,   104,   104,   106,
     184,   104,   106,   206,   104,   106,   357,   104,   106,   211,
     104,   106,   225,   104,   106,   336,   104,   106,   158,   104,
     106,   280,   104,   106,   274,   104,   106,   268,   104,   106,
     255,   104,   106,   250,   104,   104,   104,   104,   106,   311,
     104,   106,   284,   104,   106,   302,   280,   306,   306,   417,
     410,   103,   121,   122,   125,   126,    83,   173,   105,   401,
     144,   383,   375,   379,   381,   397,   370,   107,   367,   420,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     105,   192,   249,   252,   254,   257,   261,   264,   267,   277,
     279,   283,   356,   105,   105,   397,   103,   420,   131,    82,
     128,   130,   144,   131,   128,   142,   421,   105,   403,   105,
     108,   382,   383,   397,   412,    24,    32,   197,   198,   215,
     217,   231,   233,   193,   105,   207,   207,   212,   207,   337,
     159,   281,   275,   269,   256,   251,   259,   262,   265,   312,
     285,   303,   403,   403,   403,   403,   420,   408,   420,     8,
      43,    44,   132,   136,   145,   421,   145,   403,    88,    93,
     116,   172,   174,     5,   422,   376,    54,   105,   404,   413,
     415,   416,   428,   421,   421,   105,   190,   199,   200,   202,
     204,   209,   224,   227,   229,   403,   232,   105,   151,   153,
     176,   194,   245,   247,   105,   151,   153,   241,   243,   105,
     105,   151,   153,   214,   241,   243,   105,   105,   151,   153,
     105,   151,   153,   105,   151,   153,   176,   183,   335,   339,
     342,   356,   105,   151,   153,   176,   183,   252,   335,   105,
     151,   153,   176,   183,   247,   254,   257,   261,   264,   267,
     270,   271,   273,   277,   279,   335,   339,   342,   105,   151,
     153,   176,   183,   192,   249,   252,   299,   310,   335,   339,
     345,   356,   105,   151,   153,   176,   192,   249,   252,   299,
     310,   105,   151,   153,   176,   183,   194,   245,   247,   335,
     339,   342,   356,   105,   151,   153,   176,   183,   194,   245,
     247,   335,   339,   347,   350,   353,   356,   105,   151,   153,
     176,   183,   192,   249,   252,   299,   310,   335,   339,   347,
     350,   353,   356,   359,   363,   105,   151,   153,   176,   183,
     192,   249,   252,   356,    21,    68,   105,   151,   153,   176,
     183,   288,   293,   335,   105,   151,   153,   176,   183,   192,
     249,   305,   308,   418,     8,   118,   421,   421,   403,   403,
     147,   149,   151,   153,   154,   156,   127,   423,   154,   420,
     420,   399,   384,   397,   420,   408,   408,   234,   396,   218,
     423,   403,   194,   403,   403,   403,   216,   233,   403,   403,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   421,   421,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   420,   420,   133,   396,   135,   396,   162,
     163,   157,   399,   162,   422,   422,   104,   106,   414,   414,
     104,   106,   235,   104,   106,   219,   217,   116,   291,   292,
     370,   116,   286,   287,   370,   104,   104,   106,   137,   105,
      21,    35,    36,    38,    41,    72,   164,   166,   179,   186,
     192,   249,   252,   296,   301,   310,   314,   403,   105,   109,
     110,   420,   403,   403,   403,   403,   104,   420,   104,   106,
     289,   420,   403,   403,   420,   421,   421,   421,   421,   421,
     421,   403,   420,   422,   417,   236,   220,   403,   403,   134,
     138,   116,   294,   295,   367,   368,   165,   396,   116,   313,
     316,   368,   178,   396,   185,   396,   300,   396,    85,   385,
     390,   105,   105,   151,   153,   176,   183,   238,   105,   151,
     153,   176,   183,   222,   297,   290,   105,   140,   144,   151,
     153,   105,   140,   151,   153,   104,   369,   420,   104,   106,
     167,   104,   420,   104,   106,   180,   104,   106,   187,   302,
     422,   422,   403,   403,   105,   151,   153,   176,   183,   252,
     273,   299,   310,   335,   105,   151,   153,   183,   247,   339,
     342,   345,   347,   350,   356,   403,   403,   403,   403,   403,
     111,   107,   403,   403,   403,   403,   403,   403,   403,   403,
     297,   168,   315,   181,   188,   422,   422,   105,   105,   151,
     153,   170,   176,    37,   105,   151,   153,   105,   151,   153,
     176,   183,   105,   151,   153,   176,   183,   190,   112,    14,
     386,   387,   403,   421,   403,   403,   422,   388,    84,    86,
      89,    91,   317,   318,   319,   321,   322,   323,   326,   327,
     330,   331,   107,   387,   397,   403,   420,   420,   420,   420,
     422,   389,   104,   106,   320,   104,   106,   324,   104,   106,
     328,   104,   106,   332,   397,   422,   403,   403,   403,   403,
     107,   105,   325,   329,   333,   422,   105,   245,   247,   339,
     342,   347,   350,   356,   359,   105,   245,   247,   356,   359,
     105,   194,   245,   247,   339,   342,   347,   350,   397,   403,
     403,   403
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   113,   114,   114,   114,   115,   116,   117,   118,   117,
     119,   120,   121,   122,   122,   122,   122,   123,   124,   125,
     126,   126,   126,   127,   128,   129,   130,   131,   131,   131,
     132,   133,   134,   134,   134,   134,   134,   135,   136,   137,
     137,   138,   138,   138,   138,   139,   140,   141,   142,   143,
     144,   145,   145,   145,   145,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   156,   157,   158,
     158,   159,   159,   159,   161,   160,   160,   162,   163,   163,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     165,   166,   167,   167,   168,   168,   168,   168,   168,   169,
     170,   171,   171,   172,   173,   173,   174,   174,   174,   175,
     176,   177,   177,   177,   177,   178,   179,   180,   180,   181,
     181,   181,   181,   181,   182,   183,   184,   184,   185,   186,
     187,   187,   188,   188,   188,   188,   188,   188,   189,   190,
     191,   192,   193,   193,   193,   193,   193,   193,   193,   194,
     195,   196,   196,   197,   197,   197,   198,   198,   198,   198,
     198,   198,   198,   198,   198,   199,   200,   201,   202,   203,
     203,   204,   205,   206,   206,   207,   207,   207,   207,   207,
     208,   209,   210,   211,   211,   212,   212,   212,   212,   212,
     212,   213,   214,   215,   216,   216,   217,   218,   219,   219,
     220,   220,   220,   220,   220,   220,   221,   222,   223,   223,
     224,   225,   225,   226,   227,   228,   229,   230,   230,   230,
     231,   232,   232,   233,   234,   235,   235,   236,   236,   236,
     236,   236,   236,   237,   238,   239,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   250,   251,
     251,   251,   251,   251,   251,   251,   251,   251,   252,   252,
     252,   252,   252,   252,   252,   252,   253,   254,   255,   255,
     256,   256,   256,   256,   256,   256,   256,   256,   256,   256,
     256,   256,   256,   256,   257,   258,   259,   259,   259,   259,
     259,   259,   259,   259,   259,   259,   259,   259,   260,   261,
     262,   262,   262,   262,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   262,   263,   264,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   266,   267,   268,   268,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   270,   270,
     271,   271,   271,   271,   271,   271,   271,   272,   273,   274,
     274,   275,   275,   275,   275,   275,   275,   275,   276,   277,
     278,   279,   280,   280,   281,   281,   281,   281,   281,   281,
     281,   281,   281,   282,   283,   284,   284,   285,   285,   285,
     285,   285,   285,   285,   285,   286,   286,   287,   288,   289,
     289,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   291,   291,   292,   293,   294,   294,   295,   296,
     297,   297,   297,   297,   297,   297,   297,   297,   297,   297,
     298,   299,   300,   301,   302,   302,   303,   303,   303,   303,
     303,   303,   303,   303,   303,   304,   305,   306,   306,   306,
     306,   306,   307,   308,   309,   310,   311,   311,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   313,   314,   315,
     315,   315,   315,   316,   316,   317,   317,   318,   319,   320,
     320,   321,   321,   321,   322,   323,   324,   324,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   326,   327,   328,
     328,   329,   329,   329,   329,   329,   330,   331,   332,   332,
     333,   333,   333,   333,   333,   333,   333,   333,   334,   335,
     336,   336,   337,   337,   337,   338,   339,   340,   340,   340,
     341,   342,   343,   343,   343,   344,   345,   346,   347,   348,
     348,   349,   350,   351,   351,   351,   352,   353,   354,   354,
     354,   355,   356,   357,   357,   358,   359,   360,   360,   361,
     361,   362,   363,   365,   364,   364,   366,   367,   368,   369,
     369,   371,   370,   373,   372,   374,   372,   372,   375,   376,
     377,   377,   378,   379,   380,   380,   381,   382,   382,   383,
     383,   384,   385,   386,   387,   388,   388,   389,   389,   390,
     391,   392,   392,   393,   393,   394,   394,   395,   395,   396,
     396,   397,   397,   398,   398,   398,   399,   399,   400,   401,
     402,   403,   403,   403,   404,   405,   406,   407,   407,   408,
     408,   409,   409,   409,   410,   410,   411,   411,   412,   412,
     413,   413,   413,   414,   414,   415,   416,   417,   417,   418,
     418,   419,   419,   420,   420,   421,   422,   422,   424,   423,
     423,   425,   425,   425,   425,   425,   425,   425,   426,   426,
     426,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   440,   440,   440,   440,   440,   441,   441,
     441,   441,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     0,     0,     6,
       1,    13,     1,     0,     2,     2,     2,     1,    13,     1,
       0,     2,     3,     1,     4,     1,     4,     0,     3,     3,
       7,     1,     0,     2,     2,     2,     2,     1,     4,     1,
       4,     0,     2,     2,     2,     1,     4,     1,     7,     1,
       4,     0,     2,     2,     2,     2,     1,     4,     1,     4,
       1,     4,     1,     4,     1,     1,     0,     3,     4,     1,
       4,     0,     2,     2,     0,     3,     1,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     4,     0,     3,     2,     2,     2,     1,
       4,     1,     4,     1,     0,     4,     2,     2,     1,     1,
       4,     2,     2,     2,     1,     1,     4,     1,     4,     0,
       3,     2,     2,     2,     1,     4,     1,     3,     1,     4,
       1,     4,     0,     2,     3,     2,     2,     2,     1,     4,
       1,     7,     0,     3,     2,     2,     2,     2,     2,     4,
       1,     1,     4,     1,     1,     1,     0,     2,     2,     2,
       3,     3,     2,     3,     3,     2,     0,     1,     4,     2,
       1,     4,     1,     1,     4,     0,     2,     2,     2,     2,
       1,     4,     1,     1,     4,     0,     2,     2,     2,     2,
       2,     1,     4,     3,     0,     3,     4,     1,     1,     4,
       0,     3,     2,     2,     2,     2,     1,     4,     2,     1,
       4,     1,     4,     1,     4,     1,     4,     2,     2,     1,
       2,     0,     2,     5,     1,     1,     4,     0,     3,     2,
       2,     2,     2,     1,     4,     2,     1,     1,     4,     1,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     0,
       2,     2,     2,     3,     3,     3,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     4,
       0,     3,     3,     3,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     7,     1,     0,     3,     3,     3,
       2,     3,     2,     2,     2,     2,     2,     2,     1,     7,
       0,     3,     3,     3,     2,     2,     3,     2,     2,     2,
       2,     2,     2,     2,     1,     7,     0,     3,     3,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     1,     4,     1,     4,     0,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       4,     0,     3,     3,     2,     2,     2,     3,     1,     4,
       1,     4,     1,     4,     0,     3,     3,     3,     2,     2,
       2,     2,     2,     1,     4,     1,     4,     0,     3,     3,
       2,     2,     2,     3,     3,     2,     1,     1,     4,     1,
       4,     0,     3,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     7,     2,     1,     1,     7,
       0,     3,     3,     2,     2,     2,     3,     3,     3,     3,
       1,     4,     1,     4,     1,     4,     0,     3,     2,     2,
       2,     3,     3,     3,     3,     2,     5,     0,     3,     3,
       3,     3,     2,     5,     1,     4,     1,     4,     0,     3,
       3,     2,     2,     2,     3,     3,     3,     1,     7,     0,
       2,     2,     5,     2,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     1,     3,     1,     4,     0,     2,
       3,     2,     2,     2,     2,     2,     2,     1,     3,     1,
       4,     0,     2,     3,     2,     2,     1,     3,     1,     4,
       0,     3,     2,     2,     2,     2,     2,     2,     1,     4,
       1,     4,     0,     2,     2,     1,     4,     2,     2,     1,
       1,     4,     2,     2,     1,     1,     4,     1,     4,     2,
       1,     1,     4,     2,     2,     1,     1,     4,     2,     2,
       1,     1,     4,     1,     4,     1,     4,     2,     1,     0,
       3,     1,     4,     0,     3,     1,     1,     2,     2,     0,
       2,     0,     3,     0,     2,     0,     2,     1,     3,     2,
       0,     2,     3,     2,     0,     2,     2,     0,     2,     0,
       6,     5,     5,     5,     4,     0,     2,     0,     5,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       1,     1,     1,     2,     2,     1,     2,     4,     1,     1,
       1,     0,     2,     2,     3,     2,     1,     0,     1,     0,
       2,     0,     2,     3,     0,     5,     1,     4,     0,     3,
       1,     3,     3,     1,     4,     1,     1,     0,     4,     2,
       5,     1,     1,     0,     2,     2,     0,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     0,     0,     0,     0,     0,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       4,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     4,     3,     4,     4,     4,     4,     4
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
      yyerror (&yylloc, scanner, param, YY_("syntax error: cannot back up")); \
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
                  Type, Value, Location, scanner, param); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct yang_parameter *param)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (param);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct yang_parameter *param)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, param);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void *scanner, struct yang_parameter *param)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , scanner, param);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, param); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void *scanner, struct yang_parameter *param)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (param);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 115: /* tmp_string  */

      { free((((*yyvaluep).p_str)) ? *((*yyvaluep).p_str) : NULL); }

        break;

    case 210: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 400: /* semicolom  */

      { free(((*yyvaluep).str)); }

        break;

    case 402: /* curly_bracket_open  */

      { free(((*yyvaluep).str)); }

        break;

    case 406: /* string_opt_part1  */

      { free(((*yyvaluep).str)); }

        break;

    case 431: /* type_ext_alloc  */

      { yang_type_free(param->module->ctx, ((*yyvaluep).v)); }

        break;

    case 432: /* typedef_ext_alloc  */

      { yang_type_free(param->module->ctx, &((struct lys_tpdf *)((*yyvaluep).v))->type); }

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
yyparse (void *scanner, struct yang_parameter *param)
{
/* The lookahead symbol.  */
int yychar;
char *s = NULL, *tmp_s = NULL, *ext_name;
struct lys_module *trg = NULL;
struct lys_node *tpdf_parent = NULL, *data_node = NULL;
struct lys_ext_instance_complex *ext_instance;
int is_ext_instance;
void *actual = NULL;
enum yytokentype backup_type, actual_type;
int64_t cnt_val = 0;
int is_value = 0;
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

{ yylloc.last_column = 0;
                  if (param->flags & EXT_INSTANCE_SUBSTMT) {
                    is_ext_instance = 1;
                    ext_instance = (struct lys_ext_instance_complex *)param->actual_node;
                    ext_name = (char *)param->data_node;
                  } else {
                    is_ext_instance = 0;
                  }
                  yylloc.last_line = is_ext_instance;     /* HACK for flex - return SUBMODULE_KEYWORD or SUBMODULE_EXT_KEYWORD */
                  param->value = &s;
                  param->data_node = (void **)&data_node;
                  param->actual_node = &actual;
                  backup_type = NODE;
                  trg = (param->submodule) ? (struct lys_module *)param->submodule : param->module;
                }


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
        case 5:

    { if (yyget_text(scanner)[0] == '"') {
                      char *tmp;

                      s = malloc(yyget_leng(scanner) - 1 + 7 * yylval.i);
                      if (!s) {
                        LOGMEM;
                        YYABORT;
                      }
                      if (!(tmp = yang_read_string(yyget_text(scanner) + 1, s, yyget_leng(scanner) - 2, 0, yylloc.first_column))) {
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
                  }

    break;

  case 8:

    { if (yyget_leng(scanner) > 2) {
                int length_s = strlen(s), length_tmp = yyget_leng(scanner);
                char *tmp;

                tmp = realloc(s, length_s + length_tmp - 1);
                if (!tmp) {
                  LOGMEM;
                  YYABORT;
                }
                s = tmp;
                if (yyget_text(scanner)[0] == '"') {
                  if (!(tmp = yang_read_string(yyget_text(scanner) + 1, s, length_tmp - 2, length_s, yylloc.first_column))) {
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

  case 10:

    { if (param->submodule) {
                                       free(s);
                                       LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "module");
                                       YYABORT;
                                     }
                                     trg = param->module;
                                     yang_read_common(trg,s,MODULE_KEYWORD);
                                     s = NULL;
                                     actual_type = MODULE_KEYWORD;
                                   }

    break;

  case 12:

    { if (!param->module->ns) {
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "namespace", "module");
                                            YYABORT;
                                          }
                                          if (!param->module->prefix) {
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "module");
                                            YYABORT;
                                          }
                                        }

    break;

  case 13:

    { (yyval.i) = 0; }

    break;

  case 14:

    { if (yang_check_version(param->module, param->submodule, s, (yyvsp[-1].i))) {
                                              YYABORT;
                                            }
                                            (yyval.i) = 1;
                                            s = NULL;
                                          }

    break;

  case 15:

    { if (yang_read_common(param->module, s, NAMESPACE_KEYWORD)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 16:

    { if (yang_read_prefix(trg, NULL, s)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 17:

    { if (!param->submodule) {
                                          free(s);
                                          LOGVAL(LYE_SUBMODULE, LY_VLOG_NONE, NULL);
                                          YYABORT;
                                        }
                                        trg = (struct lys_module *)param->submodule;
                                        yang_read_common(trg,s,MODULE_KEYWORD);
                                        s = NULL;
                                        actual_type = SUBMODULE_KEYWORD;
                                      }

    break;

  case 19:

    { if (!param->submodule->prefix) {
                                                  LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                                  YYABORT;
                                                }
                                                if (!(yyvsp[0].i)) {
                                                  /* check version compatibility with the main module */
                                                  if (param->module->version > 1) {
                                                      LOGVAL(LYE_INVER, LY_VLOG_NONE, NULL);
                                                      YYABORT;
                                                  }
                                                }
                                              }

    break;

  case 20:

    { (yyval.i) = 0; }

    break;

  case 21:

    { if (yang_check_version(param->module, param->submodule, s, (yyvsp[-1].i))) {
                                                 YYABORT;
                                               }
                                               (yyval.i) = 1;
                                               s = NULL;
                                             }

    break;

  case 23:

    { backup_type = actual_type;
                           actual_type = YANG_VERSION_KEYWORD;
                         }

    break;

  case 25:

    { backup_type = actual_type;
                            actual_type = NAMESPACE_KEYWORD;
                          }

    break;

  case 30:

    { actual_type = (yyvsp[-4].token);
                   backup_type = NODE;
                   actual = NULL;
                 }

    break;

  case 31:

    { YANG_ADDELEM(trg->imp, trg->imp_size);
                                     /* HACK for unres */
                                     ((struct lys_import *)actual)->module = (struct lys_module *)s;
                                     s = NULL;
                                     (yyval.token) = actual_type;
                                     actual_type = IMPORT_KEYWORD;
                                   }

    break;

  case 32:

    { (yyval.i) = 0; }

    break;

  case 33:

    { if (yang_read_prefix(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 34:

    { if (trg->version != 2) {
                                          LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "description");
                                          free(s);
                                          YYABORT;
                                        }
                                        if (yang_read_description(trg, actual, s, "import", IMPORT_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.i) = (yyvsp[-1].i);
                                      }

    break;

  case 35:

    { if (trg->version != 2) {
                                        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "reference");
                                        free(s);
                                        YYABORT;
                                      }
                                      if (yang_read_reference(trg, actual, s, "import", IMPORT_KEYWORD)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.i) = (yyvsp[-1].i);
                                    }

    break;

  case 36:

    { if ((yyvsp[-1].i)) {
                                            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "import");
                                            free(s);
                                            YYABORT;
                                          }
                                          memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                                          free(s);
                                          s = NULL;
                                          (yyval.i) = 1;
                                        }

    break;

  case 37:

    { YANG_ADDELEM(trg->inc, trg->inc_size);
                                     /* HACK for unres */
                                     ((struct lys_include *)actual)->submodule = (struct lys_submodule *)s;
                                     s = NULL;
                                     (yyval.token) = actual_type;
                                     actual_type = INCLUDE_KEYWORD;
                                   }

    break;

  case 38:

    { actual_type = (yyvsp[-1].token);
                                                                backup_type = NODE;
                                                                actual = NULL;
                                                              }

    break;

  case 41:

    { (yyval.i) = 0; }

    break;

  case 42:

    { if (trg->version != 2) {
                                           LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "description");
                                           free(s);
                                           YYABORT;
                                         }
                                         if (yang_read_description(trg, actual, s, "include", INCLUDE_KEYWORD)) {
                                            YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.i) = (yyvsp[-1].i);
                                       }

    break;

  case 43:

    { if (trg->version != 2) {
                                         LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "reference");
                                         free(s);
                                         YYABORT;
                                       }
                                       if (yang_read_reference(trg, actual, s, "include", INCLUDE_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.i) = (yyvsp[-1].i);
                                     }

    break;

  case 44:

    { if ((yyvsp[-1].i)) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "include");
                                             free(s);
                                             YYABORT;
                                           }
                                           memcpy(((struct lys_include *)actual)->rev, s, LY_REV_SIZE-1);
                                           free(s);
                                           s = NULL;
                                           (yyval.i) = 1;
                                         }

    break;

  case 45:

    { backup_type = actual_type;
                                  actual_type = REVISION_DATE_KEYWORD;
                                }

    break;

  case 47:

    { (yyval.token) = actual_type;
                                         if (is_ext_instance) {
                                           if (yang_read_extcomplex_str(trg, ext_instance, "belongs-to", ext_name, s,
                                                                        0, LY_STMT_BELONGSTO)) {
                                             YYABORT;
                                           }
                                         } else {
                                           if (param->submodule->prefix) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                             free(s);
                                             YYABORT;
                                           }
                                           if (!ly_strequal(s, param->submodule->belongsto->name, 0)) {
                                             LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "belongs-to");
                                             free(s);
                                             YYABORT;
                                           }
                                           free(s);
                                         }
                                         s = NULL;
                                         actual_type = BELONGS_TO_KEYWORD;
                                       }

    break;

  case 48:

    { if (is_ext_instance) {
                         if (yang_read_extcomplex_str(trg, ext_instance, "prefix", "belongs-to", s,
                                                      LY_STMT_BELONGSTO, LY_STMT_PREFIX)) {
                           YYABORT;
                         }
                       } else {
                         if (yang_read_prefix(trg, NULL, s)) {
                           YYABORT;
                         }
                       }
                       s = NULL;
                       actual_type = (yyvsp[-4].token);
                     }

    break;

  case 49:

    { backup_type = actual_type;
                             actual_type = PREFIX_KEYWORD;
                           }

    break;

  case 52:

    { if (yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 53:

    { if (yang_read_common(trg, s, CONTACT_KEYWORD)) {
                                 YYABORT;
                               }
                               s = NULL;
                             }

    break;

  case 54:

    { if (yang_read_description(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 55:

    { if (yang_read_reference(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 56:

    { backup_type = actual_type;
                           actual_type = ORGANIZATION_KEYWORD;
                         }

    break;

  case 58:

    { backup_type = actual_type;
                      actual_type = CONTACT_KEYWORD;
                    }

    break;

  case 60:

    { backup_type = actual_type;
                          actual_type = DESCRIPTION_KEYWORD;
                        }

    break;

  case 62:

    { backup_type = actual_type;
                        actual_type = REFERENCE_KEYWORD;
                      }

    break;

  case 64:

    { if (trg->rev_size) {
                                      struct lys_revision *tmp;

                                      tmp = realloc(trg->rev, trg->rev_size * sizeof *trg->rev);
                                      if (!tmp) {
                                        LOGMEM;
                                        YYABORT;
                                      }
                                      trg->rev = tmp;
                                    }
                                  }

    break;

  case 65:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  if (!is_ext_instance) {
                                    YANG_ADDELEM(trg->rev, trg->rev_size);
                                  }
                                  memcpy(((struct lys_revision *)actual)->date, s, LY_REV_SIZE);
                                  free(s);
                                  s = NULL;
                                  actual_type = REVISION_KEYWORD;
                                }

    break;

  case 67:

    { int i;

                                                /* check uniqueness of the revision date - not required by RFC */
                                                for (i = 0; i < (trg->rev_size - 1); i++) {
                                                  if (!strcmp(trg->rev[i].date, trg->rev[trg->rev_size - 1].date)) {
                                                    LOGWRN("Module's revisions are not unique (%s).", trg->rev[trg->rev_size - 1].date);
                                                    break;
                                                  }
                                                }
                                              }

    break;

  case 68:

    { actual_type = (yyvsp[-1].backup_token).token;
                                                                     actual = (yyvsp[-1].backup_token).actual;
                                                                   }

    break;

  case 72:

    { if (yang_read_description(trg, actual, s, "revision",REVISION_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 73:

    { if (yang_read_reference(trg, actual, s, "revision", REVISION_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 74:

    { s = strdup(yyget_text(scanner));
                              if (!s) {
                                LOGMEM;
                                YYABORT;
                              }
                            }

    break;

  case 76:

    { if (lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 77:

    { void *tmp;

                             if (trg->tpdf_size) {
                               tmp = realloc(trg->tpdf, trg->tpdf_size * sizeof *trg->tpdf);
                               if (!tmp) {
                                 LOGMEM;
                                 YYABORT;
                               }
                               trg->tpdf = tmp;
                             }

                             if (trg->features_size) {
                               tmp = realloc(trg->features, trg->features_size * sizeof *trg->features);
                               if (!tmp) {
                                 LOGMEM;
                                 YYABORT;
                               }
                               trg->features = tmp;
                             }

                             if (trg->ident_size) {
                               tmp = realloc(trg->ident, trg->ident_size * sizeof *trg->ident);
                               if (!tmp) {
                                 LOGMEM;
                                 YYABORT;
                               }
                               trg->ident = tmp;
                             }

                             if (trg->augment_size) {
                               tmp = realloc(trg->augment, trg->augment_size * sizeof *trg->augment);
                               if (!tmp) {
                                 LOGMEM;
                                 YYABORT;
                               }
                               trg->augment = tmp;
                             }

                             if (trg->extensions_size) {
                               tmp = realloc(trg->extensions, trg->extensions_size * sizeof *trg->extensions);
                               if (!tmp) {
                                 LOGMEM;
                                 YYABORT;
                               }
                               trg->extensions = tmp;
                             }
                           }

    break;

  case 78:

    { /* check the module with respect to the context now */
                         if (!param->submodule) {
                           switch (lyp_ctx_check_module(trg)) {
                           case -1:
                             YYABORT;
                           case 0:
                             break;
                           case 1:
                             /* it's already there */
                             param->flags |= YANG_EXIST_MODULE;
                             YYABORT;
                           }
                         }
                         param->flags &= (~YANG_REMOVE_IMPORT);
                         if (yang_check_imports(trg, param->unres)) {
                           YYABORT;
                         }
                         actual = NULL;
                       }

    break;

  case 79:

    { actual = NULL; }

    break;

  case 90:

    { (yyval.backup_token).token = actual_type;
                                        (yyval.backup_token).actual = actual;
                                        YANG_ADDELEM(trg->extensions, trg->extensions_size);
                                        trg->extensions_size--;
                                        ((struct lys_ext *)actual)->name = lydict_insert_zc(param->module->ctx, s);
                                        ((struct lys_ext *)actual)->module = trg;
                                        if (lyp_check_identifier(((struct lys_ext *)actual)->name, LY_IDENT_EXTENSION, trg, NULL)) {
                                          trg->extensions_size++;
                                          YYABORT;
                                        }
                                        trg->extensions_size++;
                                        s = NULL;
                                        actual_type = EXTENSION_KEYWORD;
                                      }

    break;

  case 91:

    { struct lys_ext *ext = actual;
                  ext->plugin = ext_get_plugin(ext->name, ext->module->name, ext->module->rev ? ext->module->rev[0].date : NULL);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 96:

    { if (((struct lys_ext *)actual)->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      ((struct lys_ext *)actual)->flags |= (yyvsp[0].i);
                                    }

    break;

  case 97:

    { if (yang_read_description(trg, actual, s, "extension", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 98:

    { if (yang_read_reference(trg, actual, s, "extension", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 99:

    { (yyval.token) = actual_type;
                                   if (is_ext_instance) {
                                     if (yang_read_extcomplex_str(trg, ext_instance, "argument", ext_name, s,
                                                                  0, LY_STMT_ARGUMENT)) {
                                       YYABORT;
                                     }
                                   } else {
                                     if (((struct lys_ext *)actual)->argument) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                        free(s);
                                        YYABORT;
                                     }
                                     ((struct lys_ext *)actual)->argument = lydict_insert_zc(param->module->ctx, s);
                                   }
                                   s = NULL;
                                   actual_type = ARGUMENT_KEYWORD;
                                 }

    break;

  case 100:

    { actual_type = (yyvsp[-1].token); }

    break;

  case 103:

    { (yyval.uint) = (yyvsp[0].uint);
                                       backup_type = actual_type;
                                       actual_type = YIN_ELEMENT_KEYWORD;
                                     }

    break;

  case 105:

    { if (is_ext_instance) {
         int c;
         const char ***p;
         uint8_t *val;
         struct lyext_substmt *info;

         c = 0;
         p = lys_ext_complex_get_substmt(LY_STMT_ARGUMENT, ext_instance, &info);
         if (info->cardinality >= LY_STMT_CARD_SOME) {
           /* get the index in the array to add new item */
           for (c = 0; p[0][c + 1]; c++);
           val = (uint8_t *)p[1];
         } else {
           val = (uint8_t *)(p + 1);
         }
         val[c] = ((yyvsp[-1].uint) == LYS_YINELEM) ? 1 : 2;
       } else {
         ((struct lys_ext *)actual)->flags |= (yyvsp[-1].uint);
       }
     }

    break;

  case 106:

    { (yyval.uint) = LYS_YINELEM; }

    break;

  case 107:

    { (yyval.uint) = 0; }

    break;

  case 108:

    { if (!strcmp(s, "true")) {
                 (yyval.uint) = LYS_YINELEM;
               } else if (!strcmp(s, "false")) {
                 (yyval.uint) = 0;
               } else {
                 LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                 free(s);
                 YYABORT;
               }
               free(s);
               s = NULL;
             }

    break;

  case 109:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = STATUS_KEYWORD;
                           }

    break;

  case 110:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 111:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 112:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 113:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 114:

    { if (!strcmp(s, "current")) {
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

    break;

  case 115:

    { /* check uniqueness of feature's names */
                                      if (lyp_check_identifier(s, LY_IDENT_FEATURE, trg, NULL)) {
                                        free(s);
                                        YYABORT;
                                      }
                                      (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
                                      YANG_ADDELEM(trg->features, trg->features_size);
                                      ((struct lys_feature *)actual)->name = lydict_insert_zc(trg->ctx, s);
                                      ((struct lys_feature *)actual)->module = trg;
                                      s = NULL;
                                      actual_type = FEATURE_KEYWORD;
                                    }

    break;

  case 116:

    { actual = (yyvsp[-1].backup_token).actual;
                actual_type = (yyvsp[-1].backup_token).token;
              }

    break;

  case 118:

    { struct lys_iffeature *tmp;

          if (((struct lys_feature *)actual)->iffeature_size) {
            tmp = realloc(((struct lys_feature *)actual)->iffeature,
                          ((struct lys_feature *)actual)->iffeature_size * sizeof *tmp);
            if (!tmp) {
              LOGMEM;
              YYABORT;
            }
            ((struct lys_feature *)actual)->iffeature = tmp;
          }
        }

    break;

  case 121:

    { if (((struct lys_feature *)actual)->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "feature");
                                      YYABORT;
                                    }
                                    ((struct lys_feature *)actual)->flags |= (yyvsp[0].i);
                                  }

    break;

  case 122:

    { if (yang_read_description(trg, actual, s, "feature", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 123:

    { if (yang_read_reference(trg, actual, s, "feature", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 124:

    { (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         switch (actual_type) {
                         case FEATURE_KEYWORD:
                           YANG_ADDELEM(((struct lys_feature *)actual)->iffeature,
                                        ((struct lys_feature *)actual)->iffeature_size);
                           break;
                         case IDENTITY_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "identity");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_ident *)actual)->iffeature,
                                        ((struct lys_ident *)actual)->iffeature_size);
                           break;
                         case ENUM_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_type_enum *)actual)->iffeature,
                                        ((struct lys_type_enum *)actual)->iffeature_size);
                           break;
                         case BIT_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "bit");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_type_bit *)actual)->iffeature,
                                        ((struct lys_type_bit *)actual)->iffeature_size);
                           break;
                         case REFINE_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_refine *)actual)->iffeature,
                                        ((struct lys_refine *)actual)->iffeature_size);
                           break;
                         case EXTENSION_INSTANCE:
                           /* nothing change */
                           break;
                         default:
                           /* lys_node_* */
                           YANG_ADDELEM(((struct lys_node *)actual)->iffeature,
                                        ((struct lys_node *)actual)->iffeature_size);
                           break;
                         }
                         ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                         s = NULL;
                         actual_type = IF_FEATURE_KEYWORD;
                       }

    break;

  case 125:

    { actual = (yyvsp[-1].backup_token).actual;
                   actual_type = (yyvsp[-1].backup_token).token;
                 }

    break;

  case 128:

    { const char *tmp;

                                       tmp = lydict_insert_zc(trg->ctx, s);
                                       s = NULL;
                                       if (dup_identities_check(tmp, trg)) {
                                         lydict_remove(trg->ctx, tmp);
                                         YYABORT;
                                       }
                                       (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       YANG_ADDELEM(trg->ident, trg->ident_size);
                                       ((struct lys_ident *)actual)->name = tmp;
                                       ((struct lys_ident *)actual)->module = trg;
                                       actual_type = IDENTITY_KEYWORD;
                                     }

    break;

  case 129:

    { actual = (yyvsp[-1].backup_token).actual;
                 actual_type = (yyvsp[-1].backup_token).token;
               }

    break;

  case 131:

    { void *tmp;

           if (((struct lys_ident *)actual)->base_size) {
             tmp = realloc(((struct lys_ident *)actual)->base,
                           ((struct lys_ident *)actual)->base_size * sizeof *((struct lys_ident *)actual)->base);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             ((struct lys_ident *)actual)->base = tmp;
           }

           if (((struct lys_ident *)actual)->iffeature_size) {
             tmp = realloc(((struct lys_ident *)actual)->iffeature,
                           ((struct lys_ident *)actual)->iffeature_size * sizeof *((struct lys_ident *)actual)->iffeature);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             ((struct lys_ident *)actual)->iffeature = tmp;
           }
         }

    break;

  case 133:

    { void *identity;

                                   if ((trg->version < 2) && ((struct lys_ident *)actual)->base_size) {
                                     free(s);
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                                     YYABORT;
                                   }
                                   identity = actual;
                                   YANG_ADDELEM(((struct lys_ident *)actual)->base,
                                                ((struct lys_ident *)actual)->base_size);
                                   *((struct lys_ident **)actual) = (struct lys_ident *)s;
                                   s = NULL;
                                   actual = identity;
                                 }

    break;

  case 135:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 136:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 137:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 138:

    { backup_type = actual_type;
                                   actual_type = BASE_KEYWORD;
                                 }

    break;

  case 140:

    { tpdf_parent = (actual_type == EXTENSION_INSTANCE) ? ext_instance : actual;
                                      (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
                                      if (lyp_check_identifier(s, LY_IDENT_TYPE, trg, tpdf_parent)) {
                                        free(s);
                                        YYABORT;
                                      }
                                      switch (actual_type) {
                                      case MODULE_KEYWORD:
                                      case SUBMODULE_KEYWORD:
                                        YANG_ADDELEM(trg->tpdf, trg->tpdf_size);
                                        break;
                                      case GROUPING_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_grp *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_grp *)tpdf_parent)->tpdf_size);
                                        break;
                                      case CONTAINER_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_container *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_container *)tpdf_parent)->tpdf_size);
                                        break;
                                      case LIST_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_list *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_list *)tpdf_parent)->tpdf_size);
                                        break;
                                      case RPC_KEYWORD:
                                      case ACTION_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_rpc_action *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_rpc_action *)tpdf_parent)->tpdf_size);
                                        break;
                                      case INPUT_KEYWORD:
                                      case OUTPUT_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_inout *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_inout *)tpdf_parent)->tpdf_size);
                                        break;
                                      case NOTIFICATION_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_notif *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_notif *)tpdf_parent)->tpdf_size);
                                        break;
                                      case EXTENSION_INSTANCE:
                                        /* typedef is already allocated */
                                        break;
                                      default:
                                        /* another type of nodetype is error*/
                                        LOGINT;
                                        free(s);
                                        YYABORT;
                                      }
                                      ((struct lys_tpdf *)actual)->name = lydict_insert_zc(param->module->ctx, s);
                                      ((struct lys_tpdf *)actual)->module = trg;
                                      s = NULL;
                                      actual_type = TYPEDEF_KEYWORD;
                                    }

    break;

  case 141:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 142:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 143:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 144:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 145:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 146:

    { if ((yyvsp[-1].nodes).node.ptr_tpdf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "typedef");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_tpdf->flags |= (yyvsp[0].i);
                               }

    break;

  case 147:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 148:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 149:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 150:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       actual_type = TYPE_KEYWORD;
                                     }

    break;

  case 153:

    { if (((struct yang_type *)actual)->base == LY_TYPE_STRING &&
                                         ((struct yang_type *)actual)->type->info.str.pat_count) {
                                       struct lys_restr *tmp;

                                       tmp = realloc(((struct yang_type *)actual)->type->info.str.patterns,
                                                     ((struct yang_type *)actual)->type->info.str.pat_count * sizeof *tmp);
                                       if (!tmp) {
                                         LOGMEM;
                                         YYABORT;
                                       }
                                       ((struct yang_type *)actual)->type->info.str.patterns = tmp;
                                     }
                                     if (((struct yang_type *)actual)->base == LY_TYPE_UNION) {
                                       struct lys_type *tmp;

                                       tmp = realloc(((struct yang_type *)actual)->type->info.uni.types,
                                                     ((struct yang_type *)actual)->type->info.uni.count * sizeof *tmp);
                                       if (!tmp) {
                                         LOGMEM;
                                         YYABORT;
                                       }
                                       ((struct yang_type *)actual)->type->info.uni.types = tmp;
                                     }
                                     if (((struct yang_type *)actual)->base == LY_TYPE_IDENT) {
                                       struct lys_ident **tmp;

                                       tmp = realloc(((struct yang_type *)actual)->type->info.ident.ref,
                                                     ((struct yang_type *)actual)->type->info.ident.count* sizeof *tmp);
                                       if (!tmp) {
                                         LOGMEM;
                                         YYABORT;
                                       }
                                       ((struct yang_type *)actual)->type->info.ident.ref = tmp;
                                     }
                                   }

    break;

  case 157:

    { if (yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 158:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 159:

    { /* identityref_specification */
                                   if (((struct yang_type *)actual)->base && ((struct yang_type *)actual)->base != LY_TYPE_IDENT) {
                                     LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "base");
                                     return EXIT_FAILURE;
                                   }
                                   ((struct yang_type *)actual)->base = LY_TYPE_IDENT;
                                   yang_type = actual;
                                   YANG_ADDELEM(((struct yang_type *)actual)->type->info.ident.ref,
                                                ((struct yang_type *)actual)->type->info.ident.count);
                                   *((struct lys_ident **)actual) = (struct lys_ident *)s;
                                   actual = yang_type;
                                   s = NULL;
                                 }

    break;

  case 162:

    { if (yang_read_fraction(actual, (yyvsp[0].uint))) {
                                                YYABORT;
                                              }
                                            }

    break;

  case 165:

    { actual_type = (yyvsp[-1].backup_token).token;
                                   actual = (yyvsp[-1].backup_token).actual;
                                 }

    break;

  case 166:

    { struct yang_type *stype = (struct yang_type *)actual;

                         (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         if (stype->base != 0 && stype->base != LY_TYPE_UNION) {
                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected type statement.");
                           YYABORT;
                         }
                         stype->base = LY_TYPE_UNION;
                         if (strcmp(stype->name, "union")) {
                           /* type can be a substatement only in "union" type, not in derived types */
                           LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "type", "derived type");
                           YYABORT;
                         }
                         YANG_ADDELEM(stype->type->info.uni.types, stype->type->info.uni.count)
                         actual_type = UNION_KEYWORD;
                       }

    break;

  case 167:

    { (yyval.uint) = (yyvsp[0].uint);
                                               backup_type = actual_type;
                                               actual_type = FRACTION_DIGITS_KEYWORD;
                                             }

    break;

  case 168:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 169:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 170:

    { char *endptr = NULL;
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

    break;

  case 171:

    { actual = (yyvsp[-1].backup_token).actual;
               actual_type = (yyvsp[-1].backup_token).token;
             }

    break;

  case 172:

    { (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         if (!(actual = yang_read_length(trg, actual, s, is_ext_instance))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 175:

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
                               default:
                                 LOGINT;
                                 YYABORT;
                                 break;
                               }
                             }

    break;

  case 176:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 177:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 178:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 179:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 180:

    { (yyval.backup_token).token = actual_type;
                   (yyval.backup_token).actual = actual;
                 }

    break;

  case 181:

    { if (yang_read_pattern(trg, actual, (yyvsp[-1].str), (yyvsp[0].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual_type = (yyvsp[-2].backup_token).token;
                                                                        actual = (yyvsp[-2].backup_token).actual;
                                                                      }

    break;

  case 182:

    { if (actual_type != EXTENSION_INSTANCE) {
                            if (((struct yang_type *)actual)->base != 0 && ((struct yang_type *)actual)->base != LY_TYPE_STRING) {
                              free(s);
                              LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected pattern statement.");
                              YYABORT;
                            }
                            ((struct yang_type *)actual)->base = LY_TYPE_STRING;
                            YANG_ADDELEM(((struct yang_type *)actual)->type->info.str.patterns,
                                         ((struct yang_type *)actual)->type->info.str.pat_count);
                          }
                          (yyval.str) = s;
                          s = NULL;
                          actual_type = PATTERN_KEYWORD;
                        }

    break;

  case 183:

    { (yyval.ch) = 0x06; }

    break;

  case 184:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 185:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 186:

    { if (trg->version < 2) {
                                        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "modifier");
                                        YYABORT;
                                      }
                                      if ((yyvsp[-1].ch) != 0x06) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "modifier", "pattern");
                                        YYABORT;
                                      }
                                      (yyval.ch) = (yyvsp[0].ch);
                                    }

    break;

  case 187:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 188:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 189:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 190:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 191:

    { backup_type = actual_type;
                       actual_type = MODIFIER_KEYWORD;
                     }

    break;

  case 192:

    { if (!strcmp(s, "invert-match")) {
                                                             (yyval.ch) = 0x15;
                                                             free(s);
                                                             s = NULL;
                                                           } else {
                                                             LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                                                             free(s);
                                                             YYABORT;
                                                           }
                                                         }

    break;

  case 193:

    { struct lys_type_enum * tmp;

                                                   cnt_val = 0;
                                                   tmp = realloc(((struct yang_type *)actual)->type->info.enums.enm,
                                                                 ((struct yang_type *)actual)->type->info.enums.count * sizeof *tmp);
                                                   if (!tmp) {
                                                     LOGMEM;
                                                     YYABORT;
                                                   }
                                                   ((struct yang_type *)actual)->type->info.enums.enm = tmp;
                                                 }

    break;

  case 196:

    { if (yang_check_enum(yang_type, actual, &cnt_val, is_value)) {
               YYABORT;
             }
             actual = (yyvsp[-1].backup_token).actual;
             actual_type = (yyvsp[-1].backup_token).token;
           }

    break;

  case 197:

    { (yyval.backup_token).token = actual_type;
                       (yyval.backup_token).actual = yang_type = actual;
                       YANG_ADDELEM(((struct yang_type *)actual)->type->info.enums.enm, ((struct yang_type *)actual)->type->info.enums.count);
                       if (yang_read_enum(trg, yang_type, actual, s)) {
                         YYABORT;
                       }
                       s = NULL;
                       is_value = 0;
                       actual_type = ENUM_KEYWORD;
                     }

    break;

  case 199:

    { if (((struct lys_type_enum *)actual)->iffeature_size) {
             struct lys_iffeature *tmp;

             tmp = realloc(((struct lys_type_enum *)actual)->iffeature,
                           ((struct lys_type_enum *)actual)->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             ((struct lys_type_enum *)actual)->iffeature = tmp;
           }
         }

    break;

  case 202:

    { if (is_value) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                ((struct lys_type_enum *)actual)->value = (yyvsp[0].i);

                                /* keep the highest enum value for automatic increment */
                                if ((yyvsp[0].i) >= cnt_val) {
                                  cnt_val = (yyvsp[0].i) + 1;
                                }
                                is_value = 1;
                              }

    break;

  case 203:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 204:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 205:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 206:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = VALUE_KEYWORD;
                                 }

    break;

  case 207:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 208:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 209:

    { /* convert it to int32_t */
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

    break;

  case 210:

    { actual_type = (yyvsp[-1].backup_token).token;
                                                        actual = (yyvsp[-1].backup_token).actual;
                                                      }

    break;

  case 213:

    { backup_type = actual_type;
                         actual_type = PATH_KEYWORD;
                       }

    break;

  case 215:

    { (yyval.i) = (yyvsp[0].i);
                                                 backup_type = actual_type;
                                                 actual_type = REQUIRE_INSTANCE_KEYWORD;
                                               }

    break;

  case 216:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 217:

    { (yyval.i) = 1; }

    break;

  case 218:

    { (yyval.i) = -1; }

    break;

  case 219:

    { if (!strcmp(s,"true")) {
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

    break;

  case 220:

    { struct lys_type_bit * tmp;

                                         cnt_val = 0;
                                         tmp = realloc(((struct yang_type *)actual)->type->info.bits.bit,
                                                       ((struct yang_type *)actual)->type->info.bits.count * sizeof *tmp);
                                         if (!tmp) {
                                           LOGMEM;
                                           YYABORT;
                                         }
                                         ((struct yang_type *)actual)->type->info.bits.bit = tmp;
                                       }

    break;

  case 223:

    { if (yang_check_bit(yang_type, actual, &cnt_val, is_value)) {
                      YYABORT;
                    }
                    actual = (yyvsp[-2].backup_token).actual;
                    actual_type = (yyvsp[-2].backup_token).token;
                  }

    break;

  case 224:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = yang_type = actual;
                                  YANG_ADDELEM(((struct yang_type *)actual)->type->info.bits.bit,
                                               ((struct yang_type *)actual)->type->info.bits.count);
                                  if (yang_read_bit(trg, yang_type, actual, s)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  is_value = 0;
                                  actual_type = BIT_KEYWORD;
                                }

    break;

  case 226:

    { if (((struct lys_type_bit *)actual)->iffeature_size) {
             struct lys_iffeature *tmp;

             tmp = realloc(((struct lys_type_bit *)actual)->iffeature,
                           ((struct lys_type_bit *)actual)->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             ((struct lys_type_bit *)actual)->iffeature = tmp;
           }
         }

    break;

  case 229:

    { if (is_value) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  ((struct lys_type_bit *)actual)->pos = (yyvsp[0].uint);

                                  /* keep the highest position value for automatic increment */
                                  if ((yyvsp[0].uint) >= cnt_val) {
                                    cnt_val = (yyvsp[0].uint) + 1;
                                  }
                                  is_value = 1;
                                }

    break;

  case 230:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags,
                                                     LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                  YYABORT;
                                }
                              }

    break;

  case 231:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 232:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 233:

    { (yyval.uint) = (yyvsp[0].uint);
                                             backup_type = actual_type;
                                             actual_type = POSITION_KEYWORD;
                                           }

    break;

  case 234:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 235:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 236:

    { /* convert it to uint32_t */
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

    break;

  case 237:

    { backup_type = actual_type;
                            actual_type = ERROR_MESSAGE_KEYWORD;
                          }

    break;

  case 239:

    { backup_type = actual_type;
                            actual_type = ERROR_APP_TAG_KEYWORD;
                          }

    break;

  case 241:

    { backup_type = actual_type;
                    actual_type = UNITS_KEYWORD;
                  }

    break;

  case 243:

    { backup_type = actual_type;
                      actual_type = DEFAULT_KEYWORD;
                    }

    break;

  case 245:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_GROUPING, sizeof(struct lys_node_grp)))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       data_node = actual;
                                       actual_type = GROUPING_KEYWORD;
                                     }

    break;

  case 246:

    { LOGDBG("YANG: finished parsing grouping statement \"%s\"", data_node->name);
                 actual_type = (yyvsp[-1].backup_token).token;
                 actual = (yyvsp[-1].backup_token).actual;
                 data_node = (yyvsp[-1].backup_token).actual;
               }

    break;

  case 249:

    { (yyval.nodes).grouping = actual; }

    break;

  case 250:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 251:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 252:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 257:

    { if (trg->version < 2) {
                                                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 266:

    { (yyval.backup_token).token = actual_type;
                                        (yyval.backup_token).actual = actual;
                                        if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CONTAINER, sizeof(struct lys_node_container)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                        actual_type = CONTAINER_KEYWORD;
                                      }

    break;

  case 267:

    { LOGDBG("YANG: finished parsing container statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                  data_node = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 269:

    { void *tmp;

            if ((yyvsp[-1].nodes).container->iffeature_size) {
              tmp = realloc((yyvsp[-1].nodes).container->iffeature, (yyvsp[-1].nodes).container->iffeature_size * sizeof *(yyvsp[-1].nodes).container->iffeature);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].nodes).container->iffeature = tmp;
            }

            if ((yyvsp[-1].nodes).container->must_size) {
              tmp = realloc((yyvsp[-1].nodes).container->must, (yyvsp[-1].nodes).container->must_size * sizeof *(yyvsp[-1].nodes).container->must);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].nodes).container->must = tmp;
            }
          }

    break;

  case 270:

    { (yyval.nodes).container = actual; }

    break;

  case 274:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 275:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 276:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 277:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 278:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 281:

    { if (trg->version < 2) {
                                                      LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).container, "notification");
                                                      YYABORT;
                                                    }
                                                  }

    break;

  case 284:

    { void *tmp;

                  if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "type", "leaf");
                    YYABORT;
                  }
                  if ((yyvsp[-1].nodes).node.ptr_leaf->dflt && ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE)) {
                    /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                    LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                    YYABORT;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_leaf->iffeature_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_leaf->iffeature, (yyvsp[-1].nodes).node.ptr_leaf->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaf->iffeature);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_leaf->iffeature = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_leaf->must_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_leaf->must, (yyvsp[-1].nodes).node.ptr_leaf->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaf->must);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_leaf->must = tmp;
                  }

                  LOGDBG("YANG: finished parsing leaf statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 285:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAF, sizeof(struct lys_node_leaf)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   actual_type = LEAF_KEYWORD;
                                 }

    break;

  case 286:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 289:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 290:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 292:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 293:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 294:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 295:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 296:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 297:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 298:

    { (yyval.backup_token).token = actual_type;
                                        (yyval.backup_token).actual = actual;
                                        if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAFLIST, sizeof(struct lys_node_leaflist)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                        actual_type = LEAF_LIST_KEYWORD;
                                      }

    break;

  case 299:

    { void *tmp;

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
                        if ((yyvsp[-1].nodes).node.ptr_leaflist->dflt_size && (yyvsp[-1].nodes).node.ptr_leaflist->min) {
                          LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                          LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist,
                                 "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
                          YYABORT;
                        }

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size) {
                          tmp = realloc((yyvsp[-1].nodes).node.ptr_leaflist->iffeature, (yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaflist->iffeature);
                          if (!tmp) {
                            LOGMEM;
                            YYABORT;
                          }
                          (yyvsp[-1].nodes).node.ptr_leaflist->iffeature = tmp;
                        }

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->must_size) {
                          tmp = realloc((yyvsp[-1].nodes).node.ptr_leaflist->must, (yyvsp[-1].nodes).node.ptr_leaflist->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaflist->must);
                          if (!tmp) {
                            LOGMEM;
                            YYABORT;
                          }
                          (yyvsp[-1].nodes).node.ptr_leaflist->must = tmp;
                        }

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->dflt_size) {
                          tmp = realloc((yyvsp[-1].nodes).node.ptr_leaflist->dflt, (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaflist->dflt);
                          if (!tmp) {
                            LOGMEM;
                            YYABORT;
                          }
                          (yyvsp[-1].nodes).node.ptr_leaflist->dflt = tmp;
                        }

                        LOGDBG("YANG: finished parsing leaf-list statement \"%s\"", data_node->name);
                        actual_type = (yyvsp[-4].backup_token).token;
                        actual = (yyvsp[-4].backup_token).actual;
                        data_node = (yyvsp[-4].backup_token).actual;
                      }

    break;

  case 300:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                               }

    break;

  case 303:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                            (yyval.nodes) = (yyvsp[-2].nodes);
                                          }

    break;

  case 304:

    { if (trg->version < 2) {
                                         free(s);
                                         LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "default");
                                         YYABORT;
                                       }
                                       YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaflist->dflt,
                                                    (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size);
                                       (*(const char **)actual) = lydict_insert_zc(param->module->ctx, s);
                                       s = NULL;
                                       actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                     }

    break;

  case 305:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 307:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 308:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].nodes).node.ptr_leaflist->min = (yyvsp[0].uint);
                                            (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                            if ((yyvsp[-1].nodes).node.ptr_leaflist->max && ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max)) {
                                              LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                              LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                                              YYABORT;
                                            }
                                          }

    break;

  case 309:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "max-elements", "leaf-list");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].nodes).node.ptr_leaflist->max = (yyvsp[0].uint);
                                            (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                            if ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max) {
                                              LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "max-elements");
                                              LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "\"max-elements\" is smaller than \"min-elements\".");
                                              YYABORT;
                                            }
                                          }

    break;

  case 310:

    { if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "ordered by", "leaf-list");
                                            YYABORT;
                                          }
                                          if ((yyvsp[0].i) & LYS_USERORDERED) {
                                            (yyvsp[-1].nodes).node.ptr_leaflist->flags |= LYS_USERORDERED;
                                          }
                                          (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }

    break;

  case 311:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 312:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 313:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 314:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LIST, sizeof(struct lys_node_list)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   actual_type = LIST_KEYWORD;
                                 }

    break;

  case 315:

    { void *tmp;

                  if ((yyvsp[-1].nodes).node.ptr_list->iffeature_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->iffeature, (yyvsp[-1].nodes).node.ptr_list->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->iffeature);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->iffeature = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_list->must_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->must, (yyvsp[-1].nodes).node.ptr_list->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->must);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->must = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_list->tpdf_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->tpdf, (yyvsp[-1].nodes).node.ptr_list->tpdf_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->tpdf);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->tpdf = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_list->unique_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->unique);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->unique = tmp;
                  }

                  LOGDBG("YANG: finished parsing list statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 316:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 320:

    { if ((yyvsp[-1].nodes).node.ptr_list->keys) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "key", "list");
                                  free(s);
                                  YYABORT;
                              }
                              (yyvsp[-1].nodes).node.ptr_list->keys = (struct lys_node_leaf **)s;
                              (yyval.nodes) = (yyvsp[-1].nodes);
                              s = NULL;
                            }

    break;

  case 321:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 322:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 323:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "min-elements", "list");
                                         YYABORT;
                                       }
                                       (yyvsp[-1].nodes).node.ptr_list->min = (yyvsp[0].uint);
                                       (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                       (yyval.nodes) = (yyvsp[-1].nodes);
                                       if ((yyvsp[-1].nodes).node.ptr_list->max && ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max)) {
                                         LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                         LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "\"min-elements\" is bigger than \"max-elements\".");
                                         YYABORT;
                                       }
                                     }

    break;

  case 324:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "max-elements", "list");
                                         YYABORT;
                                       }
                                       (yyvsp[-1].nodes).node.ptr_list->max = (yyvsp[0].uint);
                                       (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                       (yyval.nodes) = (yyvsp[-1].nodes);
                                       if ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max) {
                                         LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                         LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "\"max-elements\" is smaller than \"min-elements\".");
                                         YYABORT;
                                       }
                                     }

    break;

  case 325:

    { if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "ordered by", "list");
                                       YYABORT;
                                     }
                                     if ((yyvsp[0].i) & LYS_USERORDERED) {
                                       (yyvsp[-1].nodes).node.ptr_list->flags |= LYS_USERORDERED;
                                     }
                                     (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                     (yyval.nodes) = (yyvsp[-1].nodes);
                                   }

    break;

  case 326:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 327:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 328:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 332:

    { if (trg->version < 2) {
                                                 LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_list, "notification");
                                                 YYABORT;
                                               }
                                             }

    break;

  case 334:

    { (yyval.backup_token).token = actual_type;
                                     (yyval.backup_token).actual = actual;
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CHOICE, sizeof(struct lys_node_choice)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = CHOICE_KEYWORD;
                                   }

    break;

  case 335:

    { LOGDBG("YANG: finished parsing choice statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 337:

    { struct lys_iffeature *tmp;

           if (((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_TRUE) && (yyvsp[-1].nodes).node.ptr_choice->dflt) {
              LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "default", "choice");
              LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "The \"default\" statement is forbidden on choices with \"mandatory\".");
              YYABORT;
            }

           if ((yyvsp[-1].nodes).node.ptr_choice->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).node.ptr_choice->iffeature, (yyvsp[-1].nodes).node.ptr_choice->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).node.ptr_choice->iffeature = tmp;
           }
         }

    break;

  case 338:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                            }

    break;

  case 341:

    { if ((yyvsp[-1].nodes).node.flag & LYS_CHOICE_DEFAULT) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "default", "choice");
                                      free(s);
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->dflt = (struct lys_node *) s;
                                    s = NULL;
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                    (yyval.nodes).node.flag |= LYS_CHOICE_DEFAULT;
                                  }

    break;

  case 342:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 343:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 344:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 345:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 346:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 356:

    { if (trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 357:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CASE, sizeof(struct lys_node_case)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   actual_type = CASE_KEYWORD;
                                 }

    break;

  case 358:

    { LOGDBG("YANG: finished parsing case statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 360:

    { struct lys_iffeature *tmp;

           if ((yyvsp[-1].nodes).cs->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).cs->iffeature, (yyvsp[-1].nodes).cs->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).cs->iffeature = tmp;
           }
          }

    break;

  case 361:

    { (yyval.nodes).cs = actual; }

    break;

  case 364:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 365:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 366:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 368:

    { (yyval.backup_token).token = actual_type;
                                     (yyval.backup_token).actual = actual;
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYXML, sizeof(struct lys_node_anydata)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ANYXML_KEYWORD;
                                   }

    break;

  case 369:

    { LOGDBG("YANG: finished parsing anyxml statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 370:

    { (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
                                      if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYDATA, sizeof(struct lys_node_anydata)))) {
                                        YYABORT;
                                      }
                                      data_node = actual;
                                      s = NULL;
                                      actual_type = ANYDATA_KEYWORD;
                                    }

    break;

  case 371:

    { LOGDBG("YANG: finished parsing anydata statement \"%s\"", data_node->name);
                actual_type = (yyvsp[-1].backup_token).token;
                actual = (yyvsp[-1].backup_token).actual;
                data_node = (yyvsp[-1].backup_token).actual;
              }

    break;

  case 373:

    { void *tmp;

           if ((yyvsp[-1].nodes).node.ptr_anydata->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).node.ptr_anydata->iffeature, (yyvsp[-1].nodes).node.ptr_anydata->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_anydata->iffeature);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).node.ptr_anydata->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).node.ptr_anydata->must_size) {
             tmp = realloc((yyvsp[-1].nodes).node.ptr_anydata->must, (yyvsp[-1].nodes).node.ptr_anydata->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_anydata->must);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).node.ptr_anydata->must = tmp;
           }
         }

    break;

  case 374:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
                            }

    break;

  case 378:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "config",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 379:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_MAND_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "mandatory",
                                               ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                    }

    break;

  case 380:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "status",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 381:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 382:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 383:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_USES, sizeof(struct lys_node_uses)))) {
                                         YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                       actual_type = USES_KEYWORD;
                                     }

    break;

  case 384:

    { LOGDBG("YANG: finished parsing uses statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 386:

    { void *tmp;

           if ((yyvsp[-1].nodes).uses->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).uses->iffeature, (yyvsp[-1].nodes).uses->iffeature_size * sizeof *(yyvsp[-1].nodes).uses->iffeature);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).uses->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).uses->refine_size) {
             tmp = realloc((yyvsp[-1].nodes).uses->refine, (yyvsp[-1].nodes).uses->refine_size * sizeof *(yyvsp[-1].nodes).uses->refine);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).uses->refine = tmp;
           }

           if ((yyvsp[-1].nodes).uses->augment_size) {
             tmp = realloc((yyvsp[-1].nodes).uses->augment, (yyvsp[-1].nodes).uses->augment_size * sizeof *(yyvsp[-1].nodes).uses->augment);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).uses->augment = tmp;
           }
         }

    break;

  case 387:

    { (yyval.nodes).uses = actual; }

    break;

  case 390:

    { if ((yyvsp[-1].nodes).uses->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).uses, "status", "uses");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).uses->flags |= (yyvsp[0].i);
                               }

    break;

  case 391:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 392:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 397:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  YANG_ADDELEM(((struct lys_node_uses *)actual)->refine,
                                               ((struct lys_node_uses *)actual)->refine_size);
                                  ((struct lys_refine *)actual)->target_name = transform_schema2json(trg, s);
                                  free(s);
                                  s = NULL;
                                  if (!((struct lys_refine *)actual)->target_name) {
                                    YYABORT;
                                  }
                                  actual_type = REFINE_KEYWORD;
                                }

    break;

  case 398:

    { actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 400:

    { void *tmp;

           if ((yyvsp[-1].nodes).refine->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).refine->iffeature, (yyvsp[-1].nodes).refine->iffeature_size * sizeof *(yyvsp[-1].nodes).refine->iffeature);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).refine->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).refine->must_size) {
             tmp = realloc((yyvsp[-1].nodes).refine->must, (yyvsp[-1].nodes).refine->must_size * sizeof *(yyvsp[-1].nodes).refine->must);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).refine->must = tmp;
           }

           if ((yyvsp[-1].nodes).refine->dflt_size) {
             tmp = realloc((yyvsp[-1].nodes).refine->dflt, (yyvsp[-1].nodes).refine->dflt_size * sizeof *(yyvsp[-1].nodes).refine->dflt);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).refine->dflt = tmp;
           }
         }

    break;

  case 401:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 402:

    { actual = (yyvsp[-2].nodes).refine;
                                               actual_type = REFINE_KEYWORD;
                                               if ((yyvsp[-2].nodes).refine->target_type) {
                                                 if ((yyvsp[-2].nodes).refine->target_type & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML)) {
                                                   (yyvsp[-2].nodes).refine->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML);
                                                 } else {
                                                   LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "must", "refine");
                                                   LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                   YYABORT;
                                                 }
                                               } else {
                                                 (yyvsp[-2].nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
                                               }
                                             }

    break;

  case 403:

    { /* leaf, leaf-list, list, container or anyxml */
               /* check possibility of statements combination */
               if ((yyvsp[-2].nodes).refine->target_type) {
                 if ((yyvsp[-2].nodes).refine->target_type & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA)) {
                   (yyvsp[-2].nodes).refine->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA);
                 } else {
                   free(s);
                   LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "if-feature", "refine");
                   LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                   YYABORT;
                 }
               } else {
                 (yyvsp[-2].nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA;
               }
             }

    break;

  case 404:

    { if ((yyvsp[-1].nodes).refine->target_type) {
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

    break;

  case 405:

    { int i;

                                          if ((yyvsp[-1].nodes).refine->dflt_size) {
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "default", "refine");
                                              YYABORT;
                                            }
                                            if ((yyvsp[-1].nodes).refine->target_type & LYS_LEAFLIST) {
                                              (yyvsp[-1].nodes).refine->target_type = LYS_LEAFLIST;
                                            } else {
                                              free(s);
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                              LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                              YYABORT;
                                            }
                                          } else {
                                            if ((yyvsp[-1].nodes).refine->target_type) {
                                              if (trg->version < 2 && ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE))) {
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                              } if (trg->version > 1 && ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE))) {
                                                /* YANG 1.1 */
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE);
                                              } else {
                                                free(s);
                                                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                YYABORT;
                                              }
                                            } else {
                                              if (trg->version < 2) {
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE;
                                              } else {
                                                /* YANG 1.1 */
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE;
                                              }
                                            }
                                          }
                                          /* check for duplicity */
                                          for (i = 0; i < (yyvsp[-1].nodes).refine->dflt_size; ++i) {
                                              if (ly_strequal((yyvsp[-1].nodes).refine->dflt[i], s, 0)) {
                                                  LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "default");
                                                  LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", s);
                                                  YYABORT;
                                              }
                                          }
                                          YANG_ADDELEM((yyvsp[-1].nodes).refine->dflt, (yyvsp[-1].nodes).refine->dflt_size);
                                          *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                          actual = (yyvsp[-1].nodes).refine;
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }

    break;

  case 406:

    { if ((yyvsp[-1].nodes).refine->target_type) {
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

    break;

  case 407:

    { if ((yyvsp[-1].nodes).refine->target_type) {
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

    break;

  case 408:

    { if ((yyvsp[-1].nodes).refine->target_type) {
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

    break;

  case 409:

    { if ((yyvsp[-1].nodes).refine->target_type) {
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

    break;

  case 410:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 411:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 414:

    { void *parent;

                                         (yyval.backup_token).token = actual_type;
                                         (yyval.backup_token).actual = actual;
                                         parent = actual;
                                         YANG_ADDELEM(((struct lys_node_uses *)actual)->augment,
                                                      ((struct lys_node_uses *)actual)->augment_size);
                                         if (yang_read_augment(trg, parent, actual, s)) {
                                           YYABORT;
                                         }
                                         data_node = actual;
                                         s = NULL;
                                         actual_type = AUGMENT_KEYWORD;
                                       }

    break;

  case 415:

    { LOGDBG("YANG: finished parsing augment statement \"%s\"", data_node->name);
                         actual_type = (yyvsp[-4].backup_token).token;
                         actual = (yyvsp[-4].backup_token).actual;
                         data_node = (yyvsp[-4].backup_token).actual;
                       }

    break;

  case 418:

    { (yyval.backup_token).token = actual_type;
                               (yyval.backup_token).actual = actual;
                               YANG_ADDELEM(trg->augment, trg->augment_size);
                               if (yang_read_augment(trg, NULL, actual, s)) {
                                 YYABORT;
                               }
                               data_node = actual;
                               s = NULL;
                               actual_type = AUGMENT_KEYWORD;
                             }

    break;

  case 419:

    { LOGDBG("YANG: finished parsing augment statement \"%s\"", data_node->name);
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                    data_node = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 420:

    { (yyval.nodes).augment = actual; }

    break;

  case 423:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 424:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 425:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 428:

    { if (trg->version < 2) {
                                                    LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).augment, "notification");
                                                    YYABORT;
                                                  }
                                                }

    break;

  case 430:

    { if (param->module->version != 2) {
                                       LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "action");
                                       free(s);
                                       YYABORT;
                                     }
                                     (yyval.backup_token).token = actual_type;
                                     (yyval.backup_token).actual = actual;
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ACTION, sizeof(struct lys_node_rpc_action)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ACTION_KEYWORD;
                                   }

    break;

  case 431:

    { LOGDBG("YANG: finished parsing action statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 432:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  if (!(actual = yang_read_node(trg, NULL, param->node, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                  actual_type = RPC_KEYWORD;
                                }

    break;

  case 433:

    { LOGDBG("YANG: finished parsing rpc statement \"%s\"", data_node->name);
            actual_type = (yyvsp[-1].backup_token).token;
            actual = (yyvsp[-1].backup_token).actual;
            data_node = (yyvsp[-1].backup_token).actual;
          }

    break;

  case 435:

    { void *tmp;

            if ((yyvsp[-1].nodes).node.ptr_rpc->iffeature_size) {
              tmp = realloc((yyvsp[-1].nodes).node.ptr_rpc->iffeature, (yyvsp[-1].nodes).node.ptr_rpc->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_rpc->iffeature);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].nodes).node.ptr_rpc->iffeature = tmp;
            }

            if ((yyvsp[-1].nodes).node.ptr_rpc->tpdf_size) {
              tmp = realloc((yyvsp[-1].nodes).node.ptr_rpc->tpdf, (yyvsp[-1].nodes).node.ptr_rpc->tpdf_size * sizeof *(yyvsp[-1].nodes).node.ptr_rpc->tpdf);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].nodes).node.ptr_rpc->tpdf = tmp;
            }
          }

    break;

  case 436:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                         }

    break;

  case 438:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 439:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 440:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 443:

    { if ((yyvsp[-2].nodes).node.flag & LYS_RPC_INPUT) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_rpc, "input", "rpc");
                                         YYABORT;
                                       }
                                       (yyvsp[-2].nodes).node.flag |= LYS_RPC_INPUT;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 444:

    { if ((yyvsp[-2].nodes).node.flag & LYS_RPC_OUTPUT) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_rpc, "output", "rpc");
                                          YYABORT;
                                        }
                                        (yyvsp[-2].nodes).node.flag |= LYS_RPC_OUTPUT;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      }

    break;

  case 445:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  s = strdup("input");
                                  if (!s) {
                                    LOGMEM;
                                    YYABORT;
                                  }
                                  if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_INPUT, sizeof(struct lys_node_inout)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                  actual_type = INPUT_KEYWORD;
                                }

    break;

  case 446:

    { void *tmp;
                  struct lys_node_inout *input = actual;

                  if (input->must_size) {
                    tmp = realloc(input->must, input->must_size * sizeof *input->must);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    input->must = tmp;
                  }

                  if (input->tpdf_size) {
                    tmp = realloc(input->tpdf, input->tpdf_size * sizeof *input->tpdf);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    input->tpdf = tmp;
                  }

                  LOGDBG("YANG: finished parsing input statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 452:

    { (yyval.backup_token).token = actual_type;
                                    (yyval.backup_token).actual = actual;
                                    s = strdup("output");
                                    if (!s) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                    if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_OUTPUT, sizeof(struct lys_node_inout)))) {
                                      YYABORT;
                                    }
                                    data_node = actual;
                                    s = NULL;
                                    actual_type = OUTPUT_KEYWORD;
                                  }

    break;

  case 453:

    { void *tmp;
                   struct lys_node_inout *output = actual;

                   if (output->must_size) {
                     tmp = realloc(output->must, output->must_size * sizeof *output->must);
                     if (!tmp) {
                       LOGMEM;
                       YYABORT;
                     }
                     output->must = tmp;
                   }

                   if (output->tpdf_size) {
                     tmp = realloc(output->tpdf, output->tpdf_size * sizeof *output->tpdf);
                     if (!tmp) {
                       LOGMEM;
                       YYABORT;
                     }
                     output->tpdf = tmp;
                   }

                   LOGDBG("YANG: finished parsing output statement \"%s\"", data_node->name);
                   actual_type = (yyvsp[-4].backup_token).token;
                   actual = (yyvsp[-4].backup_token).actual;
                   data_node = (yyvsp[-4].backup_token).actual;
                 }

    break;

  case 454:

    { (yyval.backup_token).token = actual_type;
                                           (yyval.backup_token).actual = actual;
                                           if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                           actual_type = NOTIFICATION_KEYWORD;
                                         }

    break;

  case 455:

    { LOGDBG("YANG: finished parsing notification statement \"%s\"", data_node->name);
                     actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                     data_node = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 457:

    { void *tmp;

            if ((yyvsp[-1].nodes).notif->must_size) {
              tmp = realloc((yyvsp[-1].nodes).notif->must, (yyvsp[-1].nodes).notif->must_size * sizeof *(yyvsp[-1].nodes).notif->must);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].nodes).notif->must = tmp;
            }

           if ((yyvsp[-1].nodes).notif->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).notif->iffeature, (yyvsp[-1].nodes).notif->iffeature_size * sizeof *(yyvsp[-1].nodes).notif->iffeature);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).notif->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).notif->tpdf_size) {
             tmp = realloc((yyvsp[-1].nodes).notif->tpdf, (yyvsp[-1].nodes).notif->tpdf_size * sizeof *(yyvsp[-1].nodes).notif->tpdf);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].nodes).notif->tpdf = tmp;
           }
          }

    break;

  case 458:

    { (yyval.nodes).notif = actual; }

    break;

  case 461:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 462:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 463:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 467:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   YANG_ADDELEM(trg->deviation, trg->deviation_size);
                                   ((struct lys_deviation *)actual)->target_name = transform_schema2json(trg, s);
                                   free(s);
                                   if (!((struct lys_deviation *)actual)->target_name) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                   actual_type = DEVIATION_KEYWORD;
                                 }

    break;

  case 468:

    { void *tmp;

                      if ((yyvsp[-1].dev)->deviate_size) {
                        tmp = realloc((yyvsp[-1].dev)->deviate, (yyvsp[-1].dev)->deviate_size * sizeof *(yyvsp[-1].dev)->deviate);
                        if (!tmp) {
                          LOGINT;
                          YYABORT;
                        }
                        (yyvsp[-1].dev)->deviate = tmp;
                      } else {
                        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                        YYABORT;
                      }
                      actual_type = (yyvsp[-4].backup_token).token;
                      actual = (yyvsp[-4].backup_token).actual;
                    }

    break;

  case 469:

    { (yyval.dev) = actual; }

    break;

  case 470:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 471:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 477:

    { (yyval.backup_token).token = actual_type;
                                               (yyval.backup_token).actual = actual;
                                               if (!(actual = yang_read_deviate_unsupported(actual))) {
                                                 YYABORT;
                                               }
                                               actual_type = NOT_SUPPORTED_KEYWORD;
                                             }

    break;

  case 478:

    { actual_type = (yyvsp[-2].backup_token).token;
                              actual = (yyvsp[-2].backup_token).actual;
                            }

    break;

  case 484:

    { (yyval.backup_token).token = actual_type;
                           (yyval.backup_token).actual = actual;
                           if (!(actual = yang_read_deviate(actual, LY_DEVIATE_ADD))) {
                             YYABORT;
                           }
                           actual_type = ADD_KEYWORD;
                         }

    break;

  case 485:

    { actual_type = (yyvsp[-2].backup_token).token;
                    actual = (yyvsp[-2].backup_token).actual;
                  }

    break;

  case 487:

    { void *tmp;

           if ((yyvsp[-1].deviate)->must_size) {
             tmp = realloc((yyvsp[-1].deviate)->must, (yyvsp[-1].deviate)->must_size * sizeof *(yyvsp[-1].deviate)->must);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].deviate)->must = tmp;
           }

           if ((yyvsp[-1].deviate)->unique_size) {
             tmp = realloc((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size * sizeof *(yyvsp[-1].deviate)->unique);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].deviate)->unique = tmp;
           }

           if ((yyvsp[-1].deviate)->dflt_size) {
             tmp = realloc((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size * sizeof *(yyvsp[-1].deviate)->dflt);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].deviate)->dflt = tmp;
           }
         }

    break;

  case 488:

    { (yyval.deviate) = actual; }

    break;

  case 489:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 491:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 492:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 493:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 494:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 495:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 496:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 497:

    { (yyval.backup_token).token = actual_type;
                                 (yyval.backup_token).actual = actual;
                                 if (!(actual = yang_read_deviate(actual, LY_DEVIATE_DEL))) {
                                   YYABORT;
                                 }
                                 actual_type = DELETE_KEYWORD;
                               }

    break;

  case 498:

    { actual_type = (yyvsp[-2].backup_token).token;
                       actual = (yyvsp[-2].backup_token).actual;
                     }

    break;

  case 500:

    { void *tmp;

            if ((yyvsp[-1].deviate)->must_size) {
              tmp = realloc((yyvsp[-1].deviate)->must, (yyvsp[-1].deviate)->must_size * sizeof *(yyvsp[-1].deviate)->must);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].deviate)->must = tmp;
            }

            if ((yyvsp[-1].deviate)->unique_size) {
              tmp = realloc((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size * sizeof *(yyvsp[-1].deviate)->unique);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].deviate)->unique = tmp;
            }

            if ((yyvsp[-1].deviate)->dflt_size) {
              tmp = realloc((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size * sizeof *(yyvsp[-1].deviate)->dflt);
              if (!tmp) {
                LOGMEM;
                YYABORT;
              }
              (yyvsp[-1].deviate)->dflt = tmp;
            }
          }

    break;

  case 501:

    { (yyval.deviate) = actual; }

    break;

  case 502:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 504:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 505:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 506:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_deviate(actual, LY_DEVIATE_RPL))) {
                                     YYABORT;
                                   }
                                   actual_type = REPLACE_KEYWORD;
                                 }

    break;

  case 507:

    { actual_type = (yyvsp[-2].backup_token).token;
                        actual = (yyvsp[-2].backup_token).actual;
                      }

    break;

  case 509:

    { void *tmp;

           if ((yyvsp[-1].deviate)->dflt_size) {
             tmp = realloc((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size * sizeof *(yyvsp[-1].deviate)->dflt);
             if (!tmp) {
               LOGMEM;
               YYABORT;
             }
             (yyvsp[-1].deviate)->dflt = tmp;
           }
         }

    break;

  case 510:

    { (yyval.deviate) = actual; }

    break;

  case 512:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 513:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 514:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 515:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 516:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 517:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 518:

    { (yyval.backup_token).token = actual_type;
                        (yyval.backup_token).actual = actual;
                        if (!(actual = yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type = WHEN_KEYWORD;
                      }

    break;

  case 519:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 523:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 524:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 525:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = CONFIG_KEYWORD;
                           }

    break;

  case 526:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 527:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 528:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 529:

    { if (!strcmp(s, "true")) {
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

    break;

  case 530:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = MANDATORY_KEYWORD;
                                 }

    break;

  case 531:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 532:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 533:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 534:

    { if (!strcmp(s, "true")) {
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

    break;

  case 535:

    { backup_type = actual_type;
                       actual_type = PRESENCE_KEYWORD;
                     }

    break;

  case 537:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MIN_ELEMENTS_KEYWORD;
                                 }

    break;

  case 538:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 539:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 540:

    { if (strlen(s) == 1 && s[0] == '0') {
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

    break;

  case 541:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MAX_ELEMENTS_KEYWORD;
                                 }

    break;

  case 542:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 543:

    { (yyval.uint) = 0; }

    break;

  case 544:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 545:

    { if (!strcmp(s, "unbounded")) {
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

    break;

  case 546:

    { (yyval.i) = (yyvsp[0].i);
                                     backup_type = actual_type;
                                     actual_type = ORDERED_BY_KEYWORD;
                                   }

    break;

  case 547:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 548:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 549:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 550:

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

  case 551:

    { (yyval.backup_token).token = actual_type;
                       (yyval.backup_token).actual = actual;
                       switch (actual_type) {
                       case CONTAINER_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_container *)actual)->must,
                                     ((struct lys_node_container *)actual)->must_size);
                         break;
                       case ANYDATA_KEYWORD:
                       case ANYXML_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_anydata *)actual)->must,
                                     ((struct lys_node_anydata *)actual)->must_size);
                         break;
                       case LEAF_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_leaf *)actual)->must,
                                     ((struct lys_node_leaf *)actual)->must_size);
                         break;
                       case LEAF_LIST_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_leaflist *)actual)->must,
                                     ((struct lys_node_leaflist *)actual)->must_size);
                         break;
                       case LIST_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_list *)actual)->must,
                                     ((struct lys_node_list *)actual)->must_size);
                         break;
                       case REFINE_KEYWORD:
                         YANG_ADDELEM(((struct lys_refine *)actual)->must,
                                     ((struct lys_refine *)actual)->must_size);
                         break;
                       case ADD_KEYWORD:
                       case DELETE_KEYWORD:
                         YANG_ADDELEM(((struct lys_deviate *)actual)->must,
                                      ((struct lys_deviate *)actual)->must_size);
                         break;
                       case NOTIFICATION_KEYWORD:
                         if (trg->version < 2) {
                           free(s);
                           LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "must");
                           YYABORT;
                         }
                         YANG_ADDELEM(((struct lys_node_notif *)actual)->must,
                                     ((struct lys_node_notif *)actual)->must_size);
                         break;
                       case INPUT_KEYWORD:
                         if (trg->version < 2) {
                           free(s);
                           LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "must");
                           YYABORT;
                         }
                         YANG_ADDELEM(((struct lys_node_inout *)actual)->must,
                                     ((struct lys_node_inout *)actual)->must_size);
                         break;
                       case EXTENSION_INSTANCE:
                         /* must is already allocated */
                         break;
                       default:
                         free(s);
                         LOGINT;
                         YYABORT;
                       }
                       ((struct lys_restr *)actual)->expr = transform_schema2json(trg, s);
                       free(s);
                       if (!((struct lys_restr *)actual)->expr) {
                         YYABORT;
                       }
                       s = NULL;
                       actual_type = MUST_KEYWORD;
                     }

    break;

  case 552:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 555:

    { backup_type = actual_type;
                             actual_type = UNIQUE_KEYWORD;
                           }

    break;

  case 561:

    { backup_type = actual_type;
                       actual_type = KEY_KEYWORD;
                     }

    break;

  case 563:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }

    break;

  case 566:

    { (yyval.backup_token).token = actual_type;
                        (yyval.backup_token).actual = actual;
                        if (!(actual = yang_read_range(trg, actual, s, is_ext_instance))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 567:

    { if (s) {
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

    break;

  case 571:

    { if (s) {
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

    break;

  case 573:

    { tmp_s = yyget_text(scanner); }

    break;

  case 574:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM;
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 575:

    { tmp_s = yyget_text(scanner); }

    break;

  case 576:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM;
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 600:

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

  case 601:

    { (yyval.uint) = 0; }

    break;

  case 602:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 603:

    { (yyval.i) = 0; }

    break;

  case 604:

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

  case 610:

    { if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 615:

    { char *tmp;

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

    break;

  case 616:

    { s = (yyvsp[-1].str); }

    break;

  case 617:

    { s = (yyvsp[-3].str); }

    break;

  case 618:

    { actual_type = backup_type;
                 backup_type = NODE;
                 (yyval.str) = s;
                 s = NULL;
               }

    break;

  case 619:

    { actual_type = backup_type;
                           backup_type = NODE;
                         }

    break;

  case 620:

    { (yyval.str) = s;
                          s = NULL;
                        }

    break;

  case 624:

    { actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 625:

    { (yyval.backup_token).token = actual_type;
                                                (yyval.backup_token).actual = actual;
                                                if (!(actual = yang_read_ext(trg, actual, (yyvsp[-1].str), s, actual_type,
                                                                             backup_type, is_ext_instance))) {
                                                  YYABORT;
                                                }
                                                s = NULL;
                                                actual_type = EXTENSION_INSTANCE;
                                              }

    break;

  case 626:

    { (yyval.str) = s; s = NULL; }

    break;

  case 641:

    {  struct yang_ext_substmt *substmt = ((struct lys_ext_instance *)actual)->parent;
        int32_t length = 0, old_length = 0;
        char *tmp_value;

        if (!substmt) {
          substmt = calloc(1, sizeof *substmt);
          if (!substmt) {
            LOGMEM;
            YYABORT;
          }
          ((struct lys_ext_instance *)actual)->parent = substmt;
        }
        length = strlen((yyvsp[-2].str));
        old_length = (substmt->ext_substmt) ? strlen(substmt->ext_substmt) + 2 : 2;
        tmp_value = realloc(substmt->ext_substmt, old_length + length + 1);
        if (!tmp_value) {
          LOGMEM;
          YYABORT;
        }
        substmt->ext_substmt = tmp_value;
        tmp_value += old_length - 2;
        memcpy(tmp_value, (yyvsp[-2].str), length);
        tmp_value[length] = ' ';
        tmp_value[length + 1] = '\0';
        tmp_value[length + 2] = '\0';
      }

    break;

  case 642:

    {  struct yang_ext_substmt *substmt = ((struct lys_ext_instance *)actual)->parent;
        int32_t length;
        char *tmp_value, **array;
        int i = 0;

        if (!substmt) {
          substmt = calloc(1, sizeof *substmt);
          if (!substmt) {
            LOGMEM;
            YYABORT;
          }
          ((struct lys_ext_instance *)actual)->parent = substmt;
        }
        length = strlen((yyvsp[-2].str));
        if (!substmt->ext_modules) {
          array = malloc(2 * sizeof *substmt->ext_modules);
        } else {
          for (i = 0; substmt->ext_modules[i]; ++i);
          array = realloc(substmt->ext_modules, (i + 2) * sizeof *substmt->ext_modules);
        }
        if (!array) {
          LOGMEM;
          YYABORT;
        }
        substmt->ext_modules = array;
        array[i + 1] = NULL;
        tmp_value = malloc(length + 2);
        if (!tmp_value) {
          LOGMEM;
          YYABORT;
        }
        array[i] = tmp_value;
        memcpy(tmp_value, (yyvsp[-2].str), length);
        tmp_value[length] = '\0';
        tmp_value[length + 1] = '\0';
      }

    break;

  case 645:

    { (yyval.str) = yyget_text(scanner); }

    break;

  case 646:

    { (yyval.str) = yyget_text(scanner); }

    break;

  case 658:

    { s = strdup(yyget_text(scanner));
                  if (!s) {
                    LOGMEM;
                    YYABORT;
                  }
                }

    break;

  case 751:

    { s = strdup(yyget_text(scanner));
                          if (!s) {
                            LOGMEM;
                            YYABORT;
                          }
                        }

    break;

  case 752:

    { s = strdup(yyget_text(scanner));
                                    if (!s) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }

    break;

  case 753:

    { struct lys_type **type;

                             type = (struct lys_type **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                            "type", LY_STMT_TYPE);
                             if (!type) {
                               YYABORT;
                             }
                             /* allocate type structure */
                             (*type) = calloc(1, sizeof **type);

                             /* HACK for unres */
                             (*type)->parent = (struct lys_tpdf *)ext_instance;
                             (yyval.v) = actual = *type;
                            }

    break;

  case 754:

    { struct lys_tpdf **tpdf;

                                tpdf = (struct lys_tpdf **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                               "typedef", LY_STMT_TYPEDEF);
                                if (!tpdf) {
                                  YYABORT;
                                }
                                /* allocate typedef structure */
                                (*tpdf) = calloc(1, sizeof **tpdf);
                                (yyval.v) = actual = *tpdf;
                              }

    break;

  case 755:

    { struct lys_iffeature **iffeature;

                                 iffeature = (struct lys_iffeature **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                          "if-feature", LY_STMT_IFFEATURE);
                                 if (!iffeature) {
                                   YYABORT;
                                 }
                                 /* allocate typedef structure */
                                 (*iffeature) = calloc(1, sizeof **iffeature);
                                 (yyval.v) = actual = *iffeature;
                               }

    break;

  case 756:

    { struct lys_restr **restr;
                                    LY_STMT stmt;

                                    s = yyget_text(scanner);
                                    if (!strcmp(s, "must")) {
                                      stmt = LY_STMT_MUST;
                                    } else if (!strcmp(s, "pattern")) {
                                      stmt = LY_STMT_PATTERN;
                                    } else if (!strcmp(s, "range")) {
                                      stmt = LY_STMT_RANGE;
                                    } else {
                                      stmt = LY_STMT_LENGTH;
                                    }
                                    restr = (struct lys_restr **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name, s, stmt);
                                    if (!restr) {
                                      YYABORT;
                                    }
                                    /* allocate structure for must */
                                    (*restr) = calloc(1, sizeof(struct lys_restr));
                                    (yyval.v) = actual = *restr;
                                    s = NULL;
                                  }

    break;

  case 757:

    { actual = yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name, "when", LY_STMT_WHEN);
                             if (!actual) {
                               YYABORT;
                             }
                             (yyval.v) = actual;
                           }

    break;

  case 758:

    { struct lys_revision **rev;
                                 int i;

                                 rev = (struct lys_revision **)yang_getplace_for_extcomplex_struct(ext_instance, &i, ext_name,
                                                                                                   "revision", LY_STMT_REVISION);
                                 if (!rev) {
                                   YYABORT;
                                 }
                                 rev[i] = calloc(1, sizeof **rev);
                                 if (!*rev) {
                                   LOGMEM;
                                   YYABORT;
                                 }
                                 actual = rev[i];
                                 (yyval.revisions).revision = rev;
                                 (yyval.revisions).index = i;
                               }

    break;

  case 759:

    { LY_STMT stmt;

                                s = yyget_text(scanner);
                                if (!strcmp(s, "action")) {
                                  stmt = LY_STMT_ACTION;
                                } else if (!strcmp(s, "anydata")) {
                                  stmt = LY_STMT_ANYDATA;
                                } else if (!strcmp(s, "anyxml")) {
                                  stmt = LY_STMT_ANYXML;
                                } else if (!strcmp(s, "case")) {
                                  stmt = LY_STMT_CASE;
                                } else if (!strcmp(s, "choice")) {
                                  stmt = LY_STMT_CHOICE;
                                } else if (!strcmp(s, "container")) {
                                  stmt = LY_STMT_CONTAINER;
                                } else if (!strcmp(s, "grouping")) {
                                  stmt = LY_STMT_GROUPING;
                                } else if (!strcmp(s, "input")) {
                                  stmt = LY_STMT_INPUT;
                                } else if (!strcmp(s, "leaf")) {
                                  stmt = LY_STMT_LEAF;
                                } else if (!strcmp(s, "leaf-list")) {
                                  stmt = LY_STMT_LEAFLIST;
                                } else if (!strcmp(s, "list")) {
                                  stmt = LY_STMT_LIST;
                                } else if (!strcmp(s, "notification")) {
                                  stmt = LY_STMT_NOTIFICATION;
                                } else if (!strcmp(s, "output")) {
                                  stmt = LY_STMT_OUTPUT;
                                } else {
                                  stmt = LY_STMT_USES;
                                }
                                if (yang_extcomplex_node(ext_instance, ext_name, s, *param->node, stmt)) {
                                  YYABORT;
                                }
                                actual = NULL;
                                s = NULL;
                              }

    break;

  case 760:

    { LOGERR(LY_SUCCESS, "Extension's substatement \"%s\" not supported.", yyget_text(scanner)); }

    break;

  case 792:

    { actual_type = EXTENSION_INSTANCE;
                                actual = ext_instance;
                                if (!is_ext_instance) {
                                  LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
                                  YYABORT;
                                }
                              }

    break;

  case 794:

    { if (yang_read_extcomplex_str(trg, ext_instance, "prefix", ext_name, s,
                                                                  0, LY_STMT_PREFIX)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 795:

    { if (yang_read_extcomplex_str(trg, ext_instance, "description", ext_name, s,
                                                                       0, LY_STMT_DESCRIPTION)) {
                                            YYABORT;
                                          }
                                        }

    break;

  case 796:

    { if (yang_read_extcomplex_str(trg, ext_instance, "reference", ext_name, s,
                                                                     0, LY_STMT_REFERENCE)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 797:

    { if (yang_read_extcomplex_str(trg, ext_instance, "units", ext_name, s,
                                                                     0, LY_STMT_UNITS)) {
                                      YYABORT;
                                    }
                                  }

    break;

  case 798:

    { if (yang_read_extcomplex_str(trg, ext_instance, "base", ext_name, s,
                                                                0, LY_STMT_BASE)) {
                                     YYABORT;
                                   }
                                 }

    break;

  case 799:

    { if (yang_read_extcomplex_str(trg, ext_instance, "contact", ext_name, s,
                                                                     0, LY_STMT_CONTACT)) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 800:

    { if (yang_read_extcomplex_str(trg, ext_instance, "default", ext_name, s,
                                                                     0, LY_STMT_DEFAULT)) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 801:

    { if (yang_read_extcomplex_str(trg, ext_instance, "error-message", ext_name, s,
                                                                         0, LY_STMT_ERRMSG)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 802:

    { if (yang_read_extcomplex_str(trg, ext_instance, "error-app-tag", ext_name, s,
                                                                         0, LY_STMT_ERRTAG)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 803:

    { if (yang_read_extcomplex_str(trg, ext_instance, "key", ext_name, s,
                                                               0, LY_STMT_KEY)) {
                                    YYABORT;
                                  }
                                }

    break;

  case 804:

    { if (yang_read_extcomplex_str(trg, ext_instance, "namespace", ext_name, s,
                                                                     0, LY_STMT_NAMESPACE)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 805:

    { if (yang_read_extcomplex_str(trg, ext_instance, "organization", ext_name, s,
                                                                        0, LY_STMT_ORGANIZATION)) {
                                             YYABORT;
                                           }
                                         }

    break;

  case 806:

    { if (yang_read_extcomplex_str(trg, ext_instance, "path", ext_name, s,
                                                                0, LY_STMT_PATH)) {
                                     YYABORT;
                                   }
                                 }

    break;

  case 807:

    { if (yang_read_extcomplex_str(trg, ext_instance, "presence", ext_name, s,
                                                                    0, LY_STMT_PRESENCE)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 808:

    { if (yang_read_extcomplex_str(trg, ext_instance, "revision-date", ext_name, s,
                                                                         0, LY_STMT_REVISIONDATE)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 809:

    { struct lys_type *type = (yyvsp[-2].v);

       if (yang_fill_type(trg, type, (struct yang_type *)type->der, ext_instance, param->unres)) {
         yang_type_free(trg->ctx, type);
         YYABORT;
       }
       if (unres_schema_add_node(trg, param->unres, type, UNRES_TYPE_DER_EXT, NULL) == -1) {
         yang_type_free(trg->ctx, type);
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 810:

    { struct lys_tpdf *tpdf = (yyvsp[-2].v);

       if (yang_fill_type(trg, &tpdf->type, (struct yang_type *)tpdf->type.der, tpdf, param->unres)) {
         yang_type_free(trg->ctx, &tpdf->type);
       }
       if (yang_check_ext_instance(trg, &tpdf->ext, tpdf->ext_size, tpdf, param->unres)) {
         YYABORT;
       }
       if (unres_schema_add_node(trg, param->unres, &tpdf->type, UNRES_TYPE_DER_TPDF, (struct lys_node *)ext_instance) == -1) {
         yang_type_free(trg->ctx, &tpdf->type);
         YYABORT;
       }
       /* check default value*/
       if (unres_schema_add_node(trg, param->unres, &tpdf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&tpdf->dflt)) == -1)  {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 811:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "status", LY_STMT_STATUS,
                                                                    (yyvsp[0].i), LYS_STATUS_MASK)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 812:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "config", LY_STMT_CONFIG,
                                                                    (yyvsp[0].i), LYS_CONFIG_MASK)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 813:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "mandatory", LY_STMT_MANDATORY,
                                                                       (yyvsp[0].i), LYS_MAND_MASK)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 814:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "ordered-by", LY_STMT_ORDEREDBY,
                                                                        (yyvsp[0].i), LYS_ORDERED_MASK)) {
                                           YYABORT;
                                         }
                                       }

    break;

  case 815:

    { if (yang_fill_extcomplex_uint8(ext_instance, ext_name, "require-instance",
                                                                              LY_STMT_REQINSTANCE, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 816:

    { if (yang_fill_extcomplex_uint8(ext_instance, ext_name, "modifier", LY_STMT_MODIFIER, 0)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 817:

    { /* range check */
       if ((yyvsp[0].uint) < 1 || (yyvsp[0].uint) > 18) {
         LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "fraction-digits");
         YYABORT;
       }
       if (yang_fill_extcomplex_uint8(ext_instance, ext_name, "fraction-digits", LY_STMT_DIGITS, (yyvsp[0].uint))) {
         YYABORT;
       }
     }

    break;

  case 818:

    { uint32_t **val;

                                           val = (uint32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                  "min-elements", LY_STMT_MIN);
                                           if (!val) {
                                             YYABORT;
                                           }
                                           /* store the value */
                                           *val = malloc(sizeof(uint32_t));
                                           **val = (yyvsp[0].uint);
                                         }

    break;

  case 819:

    { uint32_t **val;

                                           val = (uint32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                  "max-elements", LY_STMT_MAX);
                                           if (!val) {
                                             YYABORT;
                                           }
                                           /* store the value */
                                           *val = malloc(sizeof(uint32_t));
                                           **val = (yyvsp[0].uint);
                                         }

    break;

  case 820:

    { uint32_t **val;

                                       val = (uint32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                              "position", LY_STMT_POSITION);
                                       if (!val) {
                                         YYABORT;
                                       }
                                       /* store the value */
                                       *val = malloc(sizeof(uint32_t));
                                       **val = (yyvsp[0].uint);
                                     }

    break;

  case 821:

    { int32_t **val;

                                    val = (int32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                          "value", LY_STMT_VALUE);
                                    if (!val) {
                                      YYABORT;
                                    }
                                    /* store the value */
                                    *val = malloc(sizeof(int32_t));
                                    **val = (yyvsp[0].i);
                                  }

    break;

  case 822:

    { struct lys_unique **unique;
                                     int rc;

                                     unique = (struct lys_unique **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                        "unique", LY_STMT_UNIQUE);
                                     if (!unique) {
                                       YYABORT;
                                     }
                                     *unique = calloc(1, sizeof(struct lys_unique));
                                     rc = yang_fill_unique(trg, (struct lys_node_list *)ext_instance, *unique, s, param->unres);
                                     free(s);
                                     s = NULL;
                                     if (rc) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 823:

    { struct lys_iffeature *iffeature;

       iffeature = (yyvsp[-2].v);
       s = (char *)iffeature->features;
       iffeature->features = NULL;
       if (yang_fill_iffeature(trg, iffeature, ext_instance, s, param->unres, 0)) {
         YYABORT;
       }
       if (yang_check_ext_instance(trg, &iffeature->ext, iffeature->ext_size, iffeature, param->unres)) {
         YYABORT;
       }
       s = NULL;
       actual = ext_instance;
     }

    break;

  case 825:

    { if (yang_check_ext_instance(trg, &((struct lys_restr *)(yyvsp[-2].v))->ext, ((struct lys_restr *)(yyvsp[-2].v))->ext_size, (yyvsp[-2].v), param->unres)) {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 826:

    { if (yang_check_ext_instance(trg, &(*(struct lys_when **)(yyvsp[-2].v))->ext, (*(struct lys_when **)(yyvsp[-2].v))->ext_size,
                                   *(struct lys_when **)(yyvsp[-2].v), param->unres)) {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 827:

    { int i;

       for (i = 0; i < (yyvsp[-2].revisions).index; ++i) {
         if (!strcmp((yyvsp[-2].revisions).revision[i]->date, (yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index]->date)) {
           LOGWRN("Module's revisions are not unique (%s).", (yyvsp[-2].revisions).revision[i]->date);
           break;
         }
       }
       if (yang_check_ext_instance(trg, &(yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index]->ext, (yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index]->ext_size,
                                   &(yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index], param->unres)) {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 828:

    { actual = ext_instance; }

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
      yyerror (&yylloc, scanner, param, YY_("syntax error"));
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
        yyerror (&yylloc, scanner, param, yymsgp);
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
                      yytoken, &yylval, &yylloc, scanner, param);
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
                  yystos[yystate], yyvsp, yylsp, scanner, param);
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
  yyerror (&yylloc, scanner, param, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, scanner, param);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, scanner, param);
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



void yyerror(YYLTYPE *yylloc, void *scanner, struct yang_parameter *param, ...){

  free(*param->value);
  if (yylloc->first_line != -1) {
    if (*param->data_node && (*param->data_node) == (*param->actual_node)) {
      LOGVAL(LYE_INSTMT, LY_VLOG_LYS, *param->data_node, yyget_text(scanner));
    } else {
      LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
    }
  }
}
