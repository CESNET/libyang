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
    EXTENSION_INSTANCE = 356
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
#define YYLAST   3590

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  318
/* YYNRULES -- Number of rules.  */
#define YYNRULES  773
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1256

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   356

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
     110,   111,     2,   102,     2,     2,     2,   106,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   105,
       2,   109,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   107,     2,   108,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   103,     2,   104,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   318,   318,   319,   320,   322,   345,   348,   350,   349,
     373,   384,   394,   404,   405,   411,   416,   422,   433,   443,
     456,   457,   463,   465,   469,   471,   475,   477,   478,   479,
     481,   489,   497,   498,   503,   514,   525,   536,   544,   549,
     550,   554,   555,   566,   577,   588,   592,   594,   617,   634,
     638,   640,   641,   646,   651,   656,   662,   666,   668,   672,
     674,   678,   680,   684,   686,   699,   705,   706,   708,   712,
     713,   717,   718,   723,   730,   730,   737,   743,   791,   810,
     813,   814,   815,   816,   817,   818,   819,   820,   821,   822,
     825,   840,   847,   848,   852,   853,   854,   860,   865,   871,
     882,   884,   885,   889,   894,   895,   897,   898,   899,   912,
     917,   919,   920,   921,   922,   937,   951,   956,   957,   972,
     973,   974,   980,   985,   991,  1045,  1050,  1051,  1053,  1069,
    1074,  1075,  1100,  1101,  1115,  1116,  1122,  1127,  1133,  1137,
    1139,  1192,  1203,  1206,  1209,  1214,  1219,  1225,  1230,  1236,
    1241,  1250,  1251,  1255,  1290,  1291,  1293,  1294,  1298,  1304,
    1317,  1318,  1319,  1320,  1321,  1323,  1327,  1345,  1350,  1356,
    1357,  1373,  1378,  1387,  1388,  1392,  1408,  1413,  1418,  1423,
    1429,  1434,  1441,  1454,  1455,  1459,  1460,  1470,  1475,  1480,
    1485,  1491,  1495,  1506,  1518,  1519,  1522,  1530,  1541,  1542,
    1557,  1558,  1559,  1565,  1569,  1574,  1580,  1585,  1595,  1596,
    1611,  1616,  1617,  1622,  1626,  1628,  1633,  1635,  1636,  1637,
    1650,  1662,  1663,  1665,  1673,  1685,  1686,  1701,  1702,  1703,
    1709,  1714,  1719,  1725,  1730,  1740,  1741,  1757,  1761,  1763,
    1767,  1769,  1773,  1775,  1779,  1781,  1791,  1798,  1799,  1803,
    1804,  1810,  1815,  1820,  1821,  1822,  1823,  1824,  1830,  1831,
    1832,  1833,  1834,  1835,  1836,  1837,  1840,  1850,  1857,  1858,
    1881,  1882,  1883,  1884,  1885,  1890,  1896,  1902,  1907,  1912,
    1913,  1914,  1919,  1920,  1922,  1962,  1972,  1975,  1976,  1977,
    1980,  1985,  1986,  1991,  1997,  2003,  2009,  2014,  2020,  2030,
    2085,  2088,  2089,  2090,  2093,  2104,  2109,  2110,  2116,  2129,
    2142,  2152,  2158,  2163,  2169,  2179,  2226,  2229,  2230,  2231,
    2232,  2241,  2247,  2253,  2266,  2279,  2289,  2295,  2300,  2305,
    2306,  2307,  2308,  2313,  2315,  2325,  2332,  2333,  2353,  2356,
    2357,  2358,  2368,  2375,  2382,  2389,  2395,  2401,  2403,  2404,
    2406,  2407,  2408,  2409,  2410,  2411,  2412,  2418,  2428,  2435,
    2436,  2450,  2451,  2452,  2453,  2459,  2464,  2469,  2472,  2482,
    2489,  2499,  2506,  2507,  2530,  2533,  2534,  2535,  2536,  2543,
    2550,  2557,  2562,  2568,  2578,  2585,  2586,  2618,  2619,  2620,
    2621,  2627,  2632,  2637,  2638,  2640,  2641,  2643,  2656,  2661,
    2662,  2694,  2697,  2711,  2727,  2749,  2800,  2817,  2834,  2855,
    2876,  2881,  2887,  2888,  2891,  2906,  2915,  2916,  2918,  2929,
    2938,  2939,  2940,  2941,  2947,  2952,  2957,  2958,  2959,  2964,
    2966,  2981,  2988,  2998,  3005,  3006,  3030,  3033,  3034,  3040,
    3045,  3050,  3051,  3052,  3059,  3067,  3082,  3112,  3113,  3114,
    3115,  3116,  3118,  3133,  3163,  3172,  3179,  3180,  3212,  3213,
    3214,  3215,  3221,  3226,  3231,  3232,  3233,  3235,  3247,  3267,
    3268,  3274,  3280,  3282,  3283,  3285,  3286,  3289,  3297,  3302,
    3303,  3305,  3306,  3307,  3309,  3317,  3322,  3323,  3355,  3356,
    3362,  3363,  3369,  3375,  3382,  3389,  3397,  3406,  3414,  3419,
    3420,  3452,  3453,  3459,  3460,  3466,  3473,  3481,  3486,  3487,
    3501,  3502,  3503,  3509,  3515,  3522,  3529,  3537,  3546,  3555,
    3560,  3561,  3565,  3566,  3571,  3577,  3582,  3584,  3585,  3586,
    3599,  3604,  3606,  3607,  3608,  3621,  3625,  3627,  3632,  3634,
    3635,  3655,  3660,  3662,  3663,  3664,  3684,  3689,  3691,  3692,
    3693,  3705,  3770,  3775,  3776,  3780,  3784,  3786,  3787,  3789,
    3790,  3792,  3796,  3798,  3798,  3805,  3808,  3817,  3836,  3838,
    3839,  3842,  3842,  3859,  3859,  3866,  3866,  3873,  3876,  3878,
    3880,  3881,  3883,  3885,  3887,  3888,  3890,  3892,  3893,  3895,
    3896,  3898,  3900,  3903,  3907,  3909,  3910,  3912,  3913,  3915,
    3917,  3928,  3929,  3932,  3933,  3944,  3945,  3947,  3948,  3950,
    3951,  3957,  3958,  3961,  3962,  3963,  3987,  3988,  3991,  3997,
    4001,  4006,  4007,  4008,  4011,  4016,  4026,  4028,  4029,  4031,
    4032,  4034,  4035,  4036,  4038,  4039,  4041,  4042,  4044,  4045,
    4049,  4050,  4070,  4071,  4073,  4075,  4076,  4078,  4079,  4081,
    4082,  4085,  4086,  4089,  4091,  4092,  4095,  4095,  4102,  4104,
    4105,  4106,  4107,  4108,  4109,  4110,  4112,  4113,  4114,  4115,
    4116,  4117,  4118,  4119,  4120,  4121,  4122,  4123,  4124,  4125,
    4126,  4127,  4128,  4129,  4130,  4131,  4132,  4133,  4134,  4135,
    4136,  4137,  4138,  4139,  4140,  4141,  4142,  4143,  4144,  4145,
    4146,  4147,  4148,  4149,  4150,  4151,  4152,  4153,  4154,  4155,
    4156,  4157,  4158,  4159,  4160,  4161,  4162,  4163,  4164,  4165,
    4166,  4167,  4168,  4169,  4170,  4171,  4172,  4173,  4174,  4175,
    4176,  4177,  4178,  4179,  4180,  4181,  4182,  4183,  4184,  4185,
    4186,  4187,  4188,  4189,  4190,  4191,  4192,  4195,  4202,  4209,
    4223,  4234,  4241,  4242,  4247,  4252,  4257,  4262,  4267,  4272,
    4277,  4282,  4287,  4292,  4297,  4302,  4307,  4312,  4317,  4330,
    4349,  4354,  4359,  4364
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
  "NODE", "NODE_PRINT", "EXTENSION_INSTANCE", "'+'", "'{'", "'}'", "';'",
  "'/'", "'['", "']'", "'='", "'('", "')'", "$accept", "start",
  "tmp_string", "string_1", "string_2", "$@1", "module_arg_str",
  "module_stmt", "module_header_stmts", "module_header_stmt",
  "submodule_arg_str", "submodule_stmt", "submodule_header_stmts",
  "submodule_header_stmt", "yang_version_arg", "yang_version_stmt",
  "namespace_arg_str", "namespace_stmt", "linkage_stmts", "import_stmt",
  "import_arg_str", "import_opt_stmt", "include_arg_str", "include_stmt",
  "include_end", "include_opt_stmt", "revision_date_arg",
  "revision_date_stmt", "belongs_to_arg_str", "belongs_to_stmt",
  "prefix_arg", "prefix_stmt", "meta_stmts", "organization_arg",
  "organization_stmt", "contact_arg", "contact_stmt", "description_arg",
  "description_stmt", "reference_arg", "reference_stmt", "revision_stmts",
  "revision_arg_stmt", "revision_stmts_opt", "revision_stmt",
  "revision_end", "revision_opt_stmt", "date_arg_str", "$@2",
  "body_stmts_end", "body_stmts", "body_stmt", "extension_arg_str",
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
  "unknown_statement2_identifier", "unknown_statement3_opt",
  "unknown_statement3_opt_end", "sep_stmt", "optsep", "sep",
  "whitespace_opt", "string", "$@7", "strings", "identifier",
  "identifiers", "identifiers_ref", "type_ext_alloc", "typedef_ext_alloc",
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
     355,   356,    43,   123,   125,    59,    47,    91,    93,    61,
      40,    41
};
# endif

#define YYPACT_NINF -983

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-983)))

#define YYTABLE_NINF -751

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      75,    97,  -983,  -983,   334,  2011,  -983,  -983,  -983,   253,
     253,  -983,   253,   253,   253,   253,   253,   253,   253,   253,
     253,   253,   253,   253,   253,   253,   253,   253,   253,   253,
     253,   253,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,    39,    69,  -983,  3397,  3397,  2922,  3397,   105,
    2732,  2732,  2732,  2732,  2732,  3017,   109,  2732,    65,  2732,
     113,  3397,  2732,  2732,    58,   138,  2732,   341,   253,  -983,
     253,  -983,   253,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,    53,  -983,  -983,
    -983,    56,  -983,  -983,  -983,   179,  -983,  -983,  -983,    61,
    -983,  -983,  -983,  -983,   179,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,   179,  -983,  -983,  -983,   179,  -983,   179,
    -983,   179,  -983,   179,  -983,  -983,  -983,   179,  -983,  -983,
    -983,  -983,  -983,  -983,   179,  -983,   179,  -983,  -983,  -983,
    -983,   179,  -983,   179,  -983,  -983,   179,  -983,    62,   161,
    -983,   179,  -983,  -983,   179,  -983,   179,  -983,  -983,  -983,
     179,  -983,  -983,  -983,  -983,  -983,   179,  -983,   179,  -983,
    -983,  -983,  -983,  2922,   341,  3397,   341,   253,  -983,   253,
    -983,  -983,  -983,  -983,  -983,  -983,   253,   253,  -983,   253,
     253,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
     253,   253,  -983,  -983,   253,   253,  -983,  -983,  -983,  3492,
    -983,  -983,    88,  -983,  -983,  -983,   253,  -983,  -983,  -983,
    -983,   253,   253,   253,  -983,  -983,   197,  2732,   253,   256,
    -983,    76,  -983,    99,   341,   341,   341,    91,   237,   253,
     253,  -983,    62,  -983,  3112,   253,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,   272,  -983,    48,
    -983,  -983,   118,   122,  -983,  -983,  -983,   -23,   253,   602,
     341,   304,   174,   253,  -983,  -983,  -983,   174,  -983,  -983,
     341,  -983,   236,  -983,    28,  2306,   253,   253,   142,   731,
    -983,  -983,  -983,  -983,   511,  -983,   253,   253,  -983,  -983,
     219,  2732,   219,   341,  -983,  3492,  -983,  -983,  -983,  -983,
     253,  -983,  3397,  2732,  -983,   253,   253,   253,   253,   253,
    -983,  -983,    39,  -983,  -983,  -983,  -983,  -983,  -983,   341,
     228,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  3397,
    3397,   341,   341,  -983,  -983,  -983,  -983,  -983,   235,   179,
    -983,  -983,   236,   236,   253,   282,  2827,   300,  -983,   326,
    -983,   124,  2732,  2732,  -983,  2732,   116,   341,  -983,   341,
     341,   341,   302,  -983,   341,   253,   248,  -983,   330,  -983,
     260,  1979,   253,  -983,  -983,   277,   281,   298,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,   179,  -983,  -983,   338,  -983,   339,  -983,   371,
    -983,  -983,  -983,  -983,   179,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,   253,   253,   253,   253,   253,   253,   253,   253,
     253,   253,   253,   253,   253,   253,   253,   253,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,    58,   341,  -983,
    -983,   236,   253,   253,   253,   341,   341,   341,  -983,   253,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,   253,
     253,  -983,   341,   341,   341,   253,  3397,    45,  3397,  3397,
    3397,    45,  3397,  3397,  3397,  3397,  3397,  3397,  3397,  3397,
    2922,  3397,   341,   378,  -983,   253,   317,  2401,   309,   484,
     512,   341,   341,   341,   239,    73,   391,  -983,  3492,  -983,
    -983,   335,  -983,  -983,   416,  -983,   426,  -983,   445,  -983,
    -983,   345,  -983,  -983,   457,  -983,   465,  -983,   499,  -983,
     361,  -983,   379,  -983,   383,  -983,   505,  -983,   509,  -983,
     514,  -983,   391,  -983,  -983,  -983,  -983,   236,  -983,   236,
    -983,   253,  -983,   253,   253,  -983,  -983,  -983,  -983,  -983,
    -983,   253,  -983,  -983,  -983,  -983,  -983,  -983,   262,   241,
     491,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,   384,   253,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,   253,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
     341,   375,   414,   547,   350,  2732,   216,   341,   382,   341,
    -983,  -983,  -983,  -983,  -983,   253,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,   341,   341,  -983,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,    80,
     236,   236,  -983,  -983,  -983,  -983,   548,  -983,  -983,  -983,
     179,  -983,  -983,  -983,  -983,  -983,  -983,  -983,   179,  -983,
    -983,  -983,  2732,   744,   988,  1223,  1732,   147,   293,   604,
    1476,   492,  1368,  1132,  1125,  1604,  1155,   252,  -983,  -983,
    -983,   386,   518,   253,   253,   253,  -983,  -983,  -983,  -983,
     253,  -983,   253,   179,  -983,   253,   253,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,   253,   253,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,   253,  -983,  -983,  -983,  -983,  -983,   253,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,   253,   253,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,   253,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,   425,  -983,   434,  -983,
     253,   253,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,   236,  -983,  -983,  2579,   101,  -983,  2732,  2732,   341,
     341,   341,  3397,  3397,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,  3397,   341,   251,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   102,   216,   341,   341,   341,
     341,  3017,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   253,   253,   341,   341,   341,
    -983,   341,  -983,   341,  3017,  3017,   341,   341,   341,   341,
     443,  3207,  -983,  -983,   552,  -983,   556,  -983,   561,  -983,
     509,  -983,   562,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
     179,  -983,  -983,  -983,   179,  -983,  -983,  -983,   179,  -983,
    -983,  -983,   341,   341,  -983,  -983,   450,  -983,  -983,  -983,
     583,  -983,   236,  -983,  -983,   253,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,   341,
      82,    83,    86,    87,   253,  -983,   253,  -983,   253,  -983,
     253,  -983,   209,  1333,  -983,   253,  -983,  -983,  -983,   253,
    -983,  3302,   341,   341,   341,   132,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  3492,   384,
    -983,  -983,  -983,  -983,  -983,  -983,   341,   341,  -983,   461,
     502,   141,  1480,   253,   467,   115,   341,   341,   341,  -983,
     341,   341,   341,   341,  1293,   651,   236,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,   123,  -983,
    -983,   440,   213,   374,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  3492,   341,   341,   341,
    -983,  -983,  -983,   179,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,   341,   341,  -983,
     253,   253,  -983,   341,   341,   341
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     751,     0,     2,     3,     0,     4,     1,   649,   650,     0,
       0,   652,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   763,   767,   621,   753,   764,   758,   754,   755,
     770,   757,   765,   760,   761,   756,   759,   771,   772,   766,
     773,   762,     0,     0,   651,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     573,     0,     0,     0,     0,     0,     0,   752,     0,   621,
       0,   621,   653,   667,     5,   666,   668,   669,   670,   671,
     672,   673,   674,   675,   676,   677,   678,   679,   680,   681,
     682,   683,   684,   685,   686,   687,   688,   689,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   699,   700,   701,
     702,   703,   704,   705,   706,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   717,   718,   719,   720,   721,
     722,   723,   724,   725,   726,   727,   728,   729,   730,   731,
     732,   733,   734,   735,   736,   737,   738,   739,   740,   741,
     742,   743,   744,   745,   746,   651,   610,     0,    10,   747,
     651,     0,    17,   748,   615,     0,   138,   651,   651,     0,
      47,   651,   651,   529,     0,   525,   659,   662,   660,   664,
     665,   663,   658,     0,    58,   656,   661,     0,   243,     0,
      60,     0,   239,     0,   237,   612,   565,     0,   561,   563,
     611,   651,   651,   534,     0,   530,     0,    25,   651,   651,
     550,     0,   546,     0,    56,   577,     0,   213,     0,     0,
     607,     0,    49,   651,     0,   535,     0,    62,    74,    76,
       0,    45,   651,   651,   651,   114,     0,   109,     0,   241,
     623,   622,   651,     0,   768,     0,   769,     7,   621,   609,
     621,   620,   618,   139,   621,   621,   613,   614,   621,   528,
     527,   526,    59,   651,   244,    61,   240,   238,   562,   651,
     533,   532,   531,    26,   549,   548,   547,    57,   214,     0,
     580,   574,     0,   576,   584,    50,   608,   536,    63,   651,
      46,   111,   113,   112,   110,   242,     0,   627,   626,     0,
     150,     0,   140,     6,    13,    20,   616,     0,     0,   657,
     564,   589,   579,   583,     0,    75,   651,   636,   624,   625,
     628,   621,   151,   149,   621,   651,    27,    12,    27,    19,
     619,   621,     0,   578,   581,   585,   582,   587,   638,   156,
     142,     0,    51,     0,    14,    15,    16,    51,    21,   621,
     617,    48,   654,   586,     0,     0,     0,     0,     0,   153,
     154,   621,   155,   221,     0,     8,     0,     0,   621,   621,
      66,     0,    66,    22,   655,     0,   588,   637,   640,   651,
     629,   644,     0,     0,   152,     0,     0,     0,     0,     0,
     159,   621,     0,   162,   621,   621,   621,   158,   157,   194,
     220,   141,   147,   148,   146,   621,   144,   145,   651,     0,
       0,    28,    29,    52,    53,    54,    55,    78,    64,     0,
      23,    78,   654,   654,   639,     0,   631,     0,   224,     0,
     197,     0,     0,     0,   180,     0,     0,   164,   165,   160,
     161,   163,   193,   222,   143,     9,     0,    31,     0,    37,
       0,    77,     0,   621,    24,     0,     0,     0,   651,   642,
     641,   651,   630,   651,   621,   225,   621,   621,   198,   196,
     600,   170,     0,   167,   651,     0,   172,     0,   182,     0,
     566,   651,   651,   219,     0,   215,   621,   621,   621,    39,
      38,   651,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   621,    80,
      81,    82,    83,    84,    85,   258,   259,   260,   261,   262,
     263,   264,   265,    86,    87,    88,    89,     0,    67,   651,
     590,   654,   645,   634,   632,   227,   223,   200,   168,   169,
     621,   173,   171,   621,   183,   181,   621,   211,   210,   218,
     217,   216,   195,    32,    41,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    79,     0,    65,    18,     0,     0,   633,     0,
       0,   175,   185,   175,     0,     0,     0,   368,     0,   417,
     418,     0,   569,   651,     0,   334,     0,   266,     0,    90,
     474,     0,   467,   651,     0,   115,     0,   245,     0,   128,
       0,   285,     0,   298,     0,   314,     0,   454,     0,   432,
       0,   383,     0,   370,   621,    69,    68,   654,   591,   654,
     643,   629,   651,     0,     0,   226,   231,   232,   230,   621,
     229,     0,   199,   204,   205,   203,   621,   202,     0,     0,
       0,    30,    36,    33,    34,    35,    40,    44,    42,    43,
     621,   372,   369,   567,   621,   568,   416,   621,   336,   335,
     621,   268,   267,   621,    92,    91,   621,   473,   621,   117,
     116,   621,   247,   246,   621,   130,   129,   621,   621,   621,
     621,   456,   455,   621,   434,   433,   621,   385,   384,   371,
      71,     0,     0,     0,     0,     0,     0,   228,     0,   201,
     174,   178,   179,   176,   177,     0,   184,   189,   190,   186,
     187,   188,   212,   374,   420,   570,   338,   270,    94,   469,
     119,   249,   132,   286,   300,   316,   458,   436,   387,     0,
     654,   654,   651,   651,   646,   651,     0,   124,   601,   236,
       0,   233,   602,   651,   605,   606,   603,   209,     0,   206,
     651,   604,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    70,    72,
      73,     0,     0,   645,   647,   635,   621,   126,   125,   234,
     235,   207,   208,     0,   191,     0,     0,   373,   381,   382,
     380,   621,   621,   378,   379,   621,     0,     0,   419,   424,
     425,   423,   621,   621,   621,   621,   621,   621,   337,   345,
     346,   344,   621,   341,   350,   351,   352,   353,   356,   621,
     348,   349,   354,   355,   621,   342,   343,   269,   277,   278,
     276,   621,   621,   621,   621,   621,   621,   621,   275,   274,
     621,     0,    93,    97,    98,   621,    96,     0,   468,   470,
     471,   118,   122,   123,   121,   621,   248,   251,   252,   250,
     621,   621,   621,   621,   621,   131,   136,   137,   135,   621,
     133,   284,   296,   297,   295,   621,   621,   290,   292,   621,
     293,   294,   621,     0,     0,   299,   312,   313,   311,   621,
     621,   305,   304,   621,   307,   308,   309,   310,   621,     0,
     315,   327,   328,   326,   621,   621,   621,   621,   621,   621,
     621,   322,   323,   324,   325,   621,   321,   320,   457,   462,
     463,   461,   621,   621,   621,   621,   621,   651,   651,   435,
     439,   440,   438,   621,   621,   621,     0,   621,     0,   621,
       0,     0,   386,   391,   392,   390,   621,   621,   621,   621,
     599,   654,   592,   595,     0,     0,   192,     0,     0,   376,
     375,   377,     0,     0,   422,   426,   429,   427,   428,   421,
     340,   347,   339,   272,   282,   279,   283,   280,   281,   271,
     273,     0,    95,     0,   120,   254,   253,   255,   256,   257,
     134,   288,   289,   287,   291,     0,     0,   302,   303,   301,
     306,     0,   318,   329,   330,   333,   331,   332,   317,   319,
     460,   464,   465,   466,   459,   445,   452,   437,   441,   442,
     621,   443,   621,   444,     0,     0,   389,   393,   394,   388,
       0,     0,   651,   127,     0,   551,     0,   518,     0,   357,
       0,   430,     0,    99,   484,   497,   477,   506,   621,   651,
     475,   476,   651,   481,   651,   483,   651,   482,   651,   545,
       0,   541,   651,   540,     0,   537,   651,   558,     0,   555,
     559,   571,   447,   447,   413,   414,     0,   651,   396,   397,
       0,   651,   654,   596,   597,   648,   621,   553,   552,   621,
     520,   519,   621,   359,   358,   431,   621,   101,   100,   472,
       0,     0,     0,     0,   543,   542,   544,   538,   539,   556,
     557,   569,     0,     0,   621,   412,   621,   399,   398,   395,
     594,   654,   175,   522,   361,   104,   621,   479,   478,   621,
     486,   485,   621,   499,   498,   621,   508,   507,     0,   572,
     446,   621,   621,   621,   621,   453,   420,   401,   593,     0,
       0,     0,     0,     0,     0,     0,   488,   501,   510,   560,
     449,   450,   451,   448,     0,     0,   654,   554,   521,   523,
     524,   360,   365,   366,   364,   621,   621,   621,     0,   102,
     480,     0,     0,     0,   415,   400,   410,   411,   621,   405,
     406,   407,   404,   408,   409,   621,     0,   363,   367,   362,
     651,   651,   108,     0,   103,   487,   489,   492,   493,   494,
     495,   496,   621,   491,   500,   502,   505,   621,   504,   509,
     621,   512,   513,   514,   515,   516,   517,   403,   402,   598,
     107,   106,   105,   490,   503,   511
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -983,  -983,  -983,  1553,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,   233,  -983,   250,   238,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -358,  -983,   254,
    -983,  -262,   240,  -983,   585,  -983,   589,  -983,    54,  -983,
      95,   223,  -983,  -983,  -983,  -983,  -983,    74,  -983,   167,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,   110,  -983,  -983,  -983,  -983,  -983,  -983,
    -363,  -983,  -983,  -983,  -983,  -983,  -983,  -319,  -983,   -24,
    -983,  -295,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -565,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,   166,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,   259,  -983,  -983,  -983,  -983,  -983,
     221,  -983,  -983,  -983,  -983,  -983,  -983,  -983,    -3,  -983,
      -1,  -983,  -337,  -983,  -343,  -983,  -406,  -983,  -983,  -458,
    -983,  -137,  -983,  -983,  -134,  -983,  -983,  -983,  -133,  -983,
    -983,  -130,  -983,  -983,  -121,  -983,  -983,  -983,  -983,  -983,
    -115,  -983,  -983,  -983,  -112,  -983,  -107,    37,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -493,  -983,  -699,  -983,  -983,  -384,
    -983,  -983,  -983,  -414,  -983,  -983,  -983,  -409,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,  -983,
    -983,  -983,  -715,  -983,  -983,  -983,  -621,  -983,  -983,  -735,
    -983,  -983,  -725,  -983,  -738,  -983,  -983,  -730,  -983,  -983,
    -415,  -983,  -983,  -602,  -983,  -983,  -748,  -983,  -983,  -983,
    -104,  -983,  -983,  -983,  -643,   120,  -439,  -982,  -983,  -983,
    -983,  -983,   372,   331,  -983,  -983,   373,  -983,  -983,  -983,
     349,  -983,  -983,  -983,  -353,  -983,  -983,  -983,  -440,  -315,
    -983,  -983,  -983,   154,   -35,  -204,   -98,  -983,  -983,  -983,
    1023,   342,  -983,  -983,  -983,    68,  -983,  -983,  -983,  -983,
    -983,  -983,  -983,   -82,  -983,    -4,   -12,   949,  -368,   -28,
    -983,   276,   560,   -14,   -50,  -983,  -983,  -983
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   165,   166,   313,   418,   167,     2,   336,   337,
     171,     3,   338,   339,   429,   354,   216,    32,   352,   378,
     456,   594,   458,   379,   500,   595,   240,    33,   179,    34,
     231,    35,   380,   223,   423,   193,   424,   199,   721,   236,
     722,   427,   583,   428,   463,   636,   749,   241,   299,   460,
     461,   518,   608,   519,   685,   777,  1062,   865,  1118,  1223,
    1174,  1224,   246,   821,   247,   614,   520,   690,   779,   756,
     822,   798,   618,   521,   696,   781,   175,    41,   311,  1161,
     374,    79,   309,   333,   368,   369,   401,   402,   482,   403,
     483,   404,   485,   552,   658,   443,   405,   487,   555,   659,
     803,   729,   370,   452,   371,   439,   479,   590,   768,   657,
     769,   406,   558,   226,    42,   494,   408,   495,   372,   410,
     373,   437,   476,   589,   760,   650,   761,   203,   723,   201,
     724,   248,    45,   197,    46,   616,  1162,   693,   780,   823,
     606,   525,   682,   776,   526,   620,   782,   622,   527,   783,
     624,   528,   784,   604,   529,   679,   775,   839,   840,  1058,
     824,  1114,  1172,   596,   530,   632,   531,   672,   773,   630,
     532,   708,   787,  1099,  1100,   967,  1138,  1185,  1095,  1096,
     968,   600,   601,   533,   774,  1060,   825,   628,   534,   705,
     786,   956,   957,  1132,   958,   959,   626,   826,   702,   785,
     611,   536,   778,   612,  1068,  1069,  1070,  1148,  1071,  1072,
    1073,  1151,  1201,  1074,  1075,  1154,  1202,  1076,  1077,  1157,
    1203,  1056,   827,  1111,  1171,   184,    47,   185,   214,    48,
     215,   234,    49,  1084,   915,  1085,  1080,   916,  1081,   221,
      50,   222,  1054,  1164,  1108,  1088,   936,  1089,  1130,   207,
      51,   208,   279,   489,   602,   603,   675,  1090,  1131,   227,
     228,   229,   290,   291,   322,   293,   294,   324,   346,   363,
     343,   432,   638,   972,   973,  1051,  1141,   639,   762,   763,
     770,   771,   232,   168,  1091,   176,   263,   264,   341,   265,
      77,   250,   306,   307,   329,   435,   472,   588,   328,   365,
     389,   470,   390,   587,   754,   251,     4,   436,   385,   194,
     273,   195,   169,   170,   252,    52,    53,     5
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      11,   484,    43,   524,    44,    54,    54,   178,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,   660,    81,
     209,   417,   735,   198,   200,   202,   204,   416,   814,   217,
     846,   224,    82,   177,   235,   237,   932,   901,   249,   310,
     400,   859,   535,    84,   933,   523,   342,   233,   812,    38,
     844,   857,  1097,  1101,   466,   467,    84,   899,   913,   930,
     238,    13,   969,    84,    54,   356,    54,   855,    11,   415,
    -651,   883,  -651,  -589,  -589,   928,   271,     7,     7,     8,
       8,     7,     7,     8,     8,   272,     7,     6,     8,   274,
      39,   275,   173,   276,    17,   277,     7,   448,     8,   278,
      84,    17,   173,    84,    78,    40,   282,    84,   283,   480,
       7,    84,     8,   286,    84,   287,   173,  -575,   288,  -651,
     353,    84,    84,   295,   289,   362,   297,     7,   298,     8,
      28,   480,   300,   173,    29,    80,    84,    28,   304,  -651,
     305,   598,   813,   257,   845,   858,   258,   218,   259,   260,
     219,   900,   914,   931,   268,   266,   267,   861,   289,   269,
     270,   815,    17,   586,   860,   292,  1179,   666,    17,   334,
     902,   918,   935,   946,   788,  1146,  1149,  1147,  1150,  1152,
    1155,  1153,  1156,   181,   323,   340,  1078,   211,   182,   280,
     281,   335,   212,   178,   491,  1053,   284,   285,    28,   492,
     172,  1220,   180,   502,    28,  1173,  1221,   376,   377,  1200,
      30,   296,   361,   242,    84,   243,   649,   656,   244,   362,
     301,   302,   303,   480,   758,   504,   662,   667,   505,   177,
     308,   384,     7,    16,     8,  1188,   394,    15,   173,   509,
      17,   862,   366,    11,   321,    11,   511,   512,     7,   513,
       8,   319,    11,    11,   805,    11,    11,   320,   805,   711,
      17,   712,    17,   960,    18,    19,    11,    11,    24,   330,
      11,    11,   261,    17,   262,    80,    28,   325,   516,   347,
     919,    31,    11,    17,   643,    18,    19,    11,    11,    11,
     326,    26,   327,    26,    11,   462,    28,   517,    28,     7,
      29,     8,   375,  1160,   348,    11,    11,  1234,   854,    28,
     961,    11,   882,   351,    17,    30,   927,   945,    22,    28,
     867,   464,   663,   806,   367,  1064,    26,  1065,   725,     7,
    1066,     8,  1067,   661,    11,   726,     7,    11,     8,    54,
     433,   497,   173,   430,   353,     7,   962,     8,   755,   331,
      28,   332,    54,    54,   501,   440,   720,   856,   917,   934,
     853,   884,    54,    54,   881,   929,   631,   434,   926,   944,
     955,   539,   791,   792,   548,   468,    54,   469,     9,   540,
      84,    54,    54,    54,    54,    54,   561,   868,   764,   765,
     766,    14,   637,   474,    16,   475,   455,   541,    10,   312,
     811,   642,   832,   851,   486,   488,   875,   490,   889,   895,
     909,   924,   942,   953,   966,    21,   903,   904,   412,   477,
      11,   478,   833,   498,   425,   499,   425,   522,   674,   898,
     912,   550,   553,   551,   554,   897,   911,  1213,   686,    78,
    1211,    11,    31,  1233,  1238,  1214,   542,  1197,    54,   543,
    1212,   544,   890,  1230,   697,  1245,  1229,    14,  1244,   413,
      16,  1231,   549,  1246,   556,   426,   557,   426,  1239,   559,
     560,   634,   698,   635,   414,   750,   699,   896,   910,   565,
     598,    21,   903,   904,   670,   805,   671,   970,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    12,    17,   735,   919,    31,   677,
     751,   678,    17,    17,    18,    19,   643,   585,  1040,   680,
     178,   681,   971,    17,   643,    18,    19,  1042,    11,    11,
      11,    16,    17,    17,  1225,    11,   438,   644,   683,  1102,
     684,    28,   641,  1134,   643,    11,    11,    30,    28,    28,
     688,    11,   689,   673,  1210,    30,   177,  1186,   691,    28,
     692,  1199,   358,   457,   459,  1082,   357,  1170,    28,    28,
    1228,    11,  1243,  1215,    30,    30,    78,   355,   645,    31,
      36,   676,   651,   359,    37,   732,   885,   382,   465,  1232,
    1237,   687,   694,  1050,   695,   431,  1187,     7,   700,     8,
     701,   584,   703,   173,   704,   411,   652,   706,   496,   707,
     196,   196,   196,   196,   196,   210,   366,   196,   407,   196,
     714,   453,   196,   196,   367,    17,   196,    54,   834,    54,
      54,   835,   836,   646,   653,   837,   643,    54,   664,   668,
     752,   796,   753,   797,   838,  1106,   730,  1107,   731,  1109,
     841,  1110,   799,   842,  1112,  1116,  1113,  1117,   843,   709,
     801,    28,    11,  1184,  1163,  1163,  1115,    30,    14,  1133,
     937,    16,    17,    11,   647,   654,  1136,   757,  1137,   665,
     669,   613,  1159,   643,   344,   386,   364,   345,  1103,   648,
     655,  1086,    21,   903,   904,   976,   805,   388,   871,   713,
      11,   974,   473,   727,  1196,     0,    27,     0,    28,     0,
     597,    54,   605,   607,   609,     0,   615,   617,   619,   621,
     623,   625,   627,   629,  1140,   633,     0,     0,     0,     0,
     793,   794,     0,   795,   804,     0,     0,     0,     0,     0,
       0,   800,   852,    12,   728,  1205,   880,     0,   802,     0,
     925,   943,   954,     0,     0,     0,     0,     0,     0,     0,
     395,    14,     0,  1169,     0,    17,     0,     0,     0,     0,
     396,     0,     0,     0,     0,     0,   643,     0,     0,    11,
      11,    11,    25,   397,     0,    21,    11,   398,    11,   805,
     399,    54,    54,   789,     0,     0,  -166,     0,     0,  1195,
       0,    28,    54,    54,     0,     0,     0,    30,  1216,     0,
       0,     0,  1208,     0,     0,   806,     0,   808,   819,   829,
     848,   863,   869,   872,   877,   886,   892,   906,   921,   939,
     950,   963,  1209,     0,   790,     0,     0,     0,   807,   210,
       0,     0,     0,     0,     0,     0,     0,    54,  1227,  1236,
    1242,     0,     0,    54,  1226,  1235,  1241,   196,   809,   820,
     830,   849,   864,   870,   873,   878,   887,   893,   907,   922,
     940,   951,   964,   810,   210,   831,   850,   866,     0,   874,
     879,   888,   894,   908,   923,   941,   952,   965,     0,    54,
      54,     0,     0,     0,     0,     0,     0,     0,  1240,     0,
       0,     0,     0,     0,     0,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   391,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1035,  1036,     0,     0,   641,
       0,   196,     0,     0,     0,   210,     0,     0,     0,  1055,
    1057,     0,     0,   196,     0,     0,    54,    54,    55,    56,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,     0,  1125,     0,     0,     0,  1127,     0,     0,     0,
    1129,     0,   502,     0,     0,     0,   196,     0,     0,     0,
       0,     0,   196,   196,     0,   196,     0,     0,     0,     0,
       0,     0,     0,   816,   504,     0,  1104,   505,     0,    17,
       0,     0,     0,     0,     0,     0,     0,   253,     0,   255,
     643,    11,    11,     0,     0,   511,   512,     0,   513,     0,
    1105,     0,     0,     0,     0,   514,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    28,     0,  1120,     0,     0,
    1121,    30,  1122,     0,  1123,     0,  1124,   516,     0,   806,
    1126,     0,     0,     0,  1128,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   817,  1135,   517,     0,     0,  1139,
       0,     0,   818,     0,     0,     0,     0,     0,     0,     0,
       0,    11,   254,     0,   256,     0,  1168,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    11,    11,    11,
      11,     0,    11,     0,    11,  1252,    54,     0,     0,   502,
       0,    11,     0,     0,     0,    11,  1059,  1061,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
       0,   504,    14,     0,   505,  1063,    17,     0,   210,    14,
       0,     0,    16,    17,     0,   509,     0,   643,     0,    54,
       0,    20,   511,   512,   643,   513,     0,   903,   904,     0,
     805,  1249,   514,    23,   903,   904,    17,   805,     0,     0,
      23,     0,    28,     0,     0,   509,     0,   643,    30,    28,
     947,    80,   919,     0,   516,    30,   806,    78,  1250,  1251,
      31,     0,     0,   806,     0,   948,     0,     0,     0,     0,
       0,   817,    28,   517,     0,  1189,  1192,   502,    30,   920,
       0,    80,     0,     0,     0,     0,   905,     0,   819,  1206,
       0,     0,     0,     0,     0,     0,    11,    11,   816,   504,
      14,     0,   505,    16,    17,     0,     0,     0,     0,   949,
       0,     0,     0,     0,     0,   643,  1190,  1193,     0,     0,
     511,   512,     0,   513,    21,   196,     0,     0,     0,   820,
    1207,   314,  1194,   315,     0,     0,     0,   316,   317,     0,
      28,   318,     0,     0,     0,     0,    30,   502,     0,     0,
       0,     0,   381,     0,   806,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   392,   393,     0,   816,   504,
       0,   517,   505,     0,    17,   419,   420,   828,     0,     0,
       0,     0,   196,     0,     0,   643,     0,   502,     0,     0,
     511,   512,     0,   513,   441,   442,   444,   445,   446,     0,
     514,     0,     0,     0,   349,     0,     0,   350,     0,   504,
      28,     0,   505,     0,   360,     0,    30,     0,     0,     0,
       0,     0,   516,   509,   806,     0,     0,     0,     0,     0,
     511,   512,   383,   513,     0,     0,     0,     0,   805,   817,
       0,   517,     0,     0,   409,    14,     0,  1204,    16,    17,
       0,   421,   422,     0,     0,     0,     0,     0,     0,    80,
     643,   537,   516,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,   805,   447,     0,     0,   449,   450,   451,
       0,   517,     0,     0,     0,    28,     0,  1165,   454,     0,
       0,    30,     0,    78,     0,     0,    31,     0,     0,   806,
       0,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,     0,     0,     0,
       0,     0,   891,     0,     0,     0,     0,     0,     0,     0,
     502,     0,     0,     0,   502,     0,   538,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   545,     0,   546,
     547,     0,   504,     0,     0,   505,   504,    17,     0,   505,
       0,    17,     0,     0,     0,     0,   509,     0,     0,   562,
     563,   564,   643,   511,   512,     0,   513,   511,   512,     0,
     513,     0,     0,   514,   210,     0,     0,   196,   196,     0,
       0,   582,     0,    28,     0,     0,     0,    28,     0,    30,
       0,     0,    80,    30,     0,   516,     0,     0,     0,   516,
       0,   806,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   817,   591,   517,     0,   592,     0,   517,   593,
     876,   210,     0,     0,  1191,     0,     0,     0,     0,     0,
       0,     0,   715,   716,     0,     0,     0,     0,     0,     0,
     718,     0,     0,     0,   210,   210,     0,     0,   502,     0,
     174,   210,   183,   192,   192,   192,   192,   192,   206,   213,
     192,   220,   192,   225,   230,   192,   192,   239,   245,   192,
     504,     0,     0,   505,     0,    17,     0,     0,     0,     0,
       0,     0,     0,     0,   509,     0,   643,     0,     0,     0,
       0,   511,   512,     0,   513,     0,     0,   710,     0,   805,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    28,   717,     0,   772,     0,     0,    30,     0,   719,
      80,     0,     0,   516,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   733,     0,     0,     0,   734,     0,     0,
     736,   210,   517,   737,     0,     0,   738,     0,   938,   739,
       0,   740,     0,     0,   741,     0,     0,   742,   210,     0,
     743,   744,   745,   746,     0,     0,   747,     0,     0,   748,
       0,     0,     0,     0,     0,     0,   502,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   977,   978,     0,     0,   504,    14,
       0,   505,     0,    17,     0,   982,   983,     0,     0,     0,
       0,     0,   509,     0,   643,     0,   210,     0,     0,   511,
     512,     0,   513,     0,     0,     0,     0,   805,     0,   514,
       0,     0,     0,     0,     0,     0,     0,    27,     0,    28,
       0,     0,     0,     0,     0,    30,   174,     0,    80,     0,
    1001,   516,     0,   806,     0,     0,  1003,     0,     0,   975,
       0,     0,     0,     0,     0,     0,     0,     0,   817,     0,
     517,     0,     0,     0,   979,   980,   847,     0,   981,     0,
       0,     0,     0,     0,     0,   984,   985,   986,   987,   988,
     989,     0,  1015,  1016,     0,   990,     0,     0,     0,     0,
     192,     0,   991,     0,     0,     0,     0,   992,  1021,     0,
       0,     0,     0,     0,   993,   994,   995,   996,   997,   998,
     999,     0,     0,  1000,     0,     0,     0,     0,  1002,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1004,     0,
       0,     0,     0,  1005,  1006,  1007,  1008,  1009,     0,  1044,
    1045,     0,  1010,     0,     0,     0,     0,     0,  1011,  1012,
       0,     0,  1013,     0,     0,  1014,     0,     0,     0,     0,
       0,     0,  1017,  1018,   192,     0,  1019,     0,     0,     0,
       0,  1020,     0,     0,     0,     0,   192,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,     0,     0,     0,     0,  1029,     0,
       0,     0,     0,     0,     0,  1030,  1031,  1032,  1033,  1034,
       0,     0,     0,     0,     0,     0,  1037,  1038,  1039,     0,
    1041,     0,  1043,   502,     0,     0,     0,     0,     0,  1046,
    1047,  1048,  1049,     0,   481,   192,   192,     0,   192,   493,
     503,     0,     0,     0,     0,   504,     0,     0,   505,     0,
       0,     0,     0,     0,   506,   507,     0,   508,     0,   509,
     510,     0,     0,     0,     0,     0,   511,   512,     0,   513,
       0,     0,     0,    12,    13,     0,   514,     0,    14,    15,
       0,    16,    17,     0,    18,    19,     0,     0,     0,     0,
       0,   515,     0,     0,     0,    80,     0,    20,   516,     0,
       0,     0,    21,  1092,     0,  1093,     0,    22,     0,    23,
      24,     0,    25,     0,     0,    26,    27,   517,    28,  1158,
       0,     0,    29,     0,    30,     0,  -749,  -750,     0,    31,
     239,  1119,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     599,     0,  1198,     0,   610,     0,     0,     0,     0,  1142,
       0,     0,  1143,   174,     0,  1144,     0,     0,     0,  1145,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1166,     0,  1167,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1175,
       0,     0,  1176,     0,     0,  1177,     0,     0,  1178,     0,
       0,     0,     0,     0,  1180,  1181,  1182,  1183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1217,  1218,
    1219,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1247,     0,     0,     0,     0,     0,     0,  1248,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1253,     0,     0,     0,     0,
    1254,     0,     0,  1255,     0,     0,     0,     0,   192,   759,
       0,   767,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,     0,    85,   173,     0,     0,
       0,     0,     0,     0,     0,   192,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,    83,     0,     0,     0,     0,
     387,    85,   205,     0,     0,     0,     0,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
       0,     0,     0,     0,     0,   640,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     192,   192,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1079,  1083,
       0,     0,     0,     0,  1087,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,     0,    85,
     205,     0,     0,     0,     0,     0,     0,  1094,  1098,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,     0,     0,
       0,     0,     0,  1052,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
      84,   186,    85,   187,   188,     0,     0,     0,   189,   190,
     191,  1222,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    83,     0,     0,     0,   471,   186,    85,   187,   188,
       0,     0,     0,   189,   190,   191,     0,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    83,     0,     0,     0,
      84,     0,    85,   173,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    83,     0,     0,     0,    84,     0,    85,   205,     0,
       0,     0,     0,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    83,     0,     0,     0,
       0,     0,    85,   205,     0,     0,   292,     0,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    83,     0,     0,     0,     0,     0,    85,   205,     0,
       0,   971,     0,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    83,   384,     0,     0,
       0,     0,    85,   205,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    83,     0,     0,     0,    84,     0,    85,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    83,     0,     0,     0,
       0,     0,    85,   205,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164
};

static const yytype_int16 yycheck[] =
{
       4,   441,     5,   461,     5,     9,    10,    57,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,   593,    53,
      65,   374,   675,    61,    62,    63,    64,   374,   773,    67,
     775,    69,    54,    57,    72,    73,   784,   782,    76,   253,
     369,   776,   461,     8,   784,   461,   318,    71,   773,     5,
     775,   776,  1044,  1045,   432,   433,     8,   782,   783,   784,
      12,    23,   787,     8,    78,   337,    80,   776,    82,   374,
       5,   780,     7,   106,   107,   784,   184,     5,     5,     7,
       7,     5,     5,     7,     7,   193,     5,     0,     7,   197,
       5,   199,    11,   201,    31,   203,     5,   402,     7,   207,
       8,    31,    11,     8,    75,     5,   214,     8,   216,    17,
       5,     8,     7,   221,     8,   223,    11,    14,   226,    54,
      82,     8,     8,   231,   106,   107,   234,     5,   236,     7,
      67,    17,   240,    11,    71,    76,     8,    67,   246,    74,
     248,   106,   773,   165,   775,   776,   103,    92,   170,   103,
      95,   782,   783,   784,   103,   177,   178,    20,   106,   181,
     182,   773,    31,   541,   776,    14,  1158,   104,    31,   103,
     782,   783,   784,   785,   104,   103,   103,   105,   105,   103,
     103,   105,   105,    88,   106,   104,    94,    88,    93,   211,
     212,   102,    93,   253,    88,   104,   218,   219,    67,    93,
      56,    88,    58,     4,    67,    83,    93,    43,    44,   104,
      73,   233,   104,    85,     8,    87,   589,   590,    90,   107,
     242,   243,   244,    17,    18,    26,   594,   595,    29,   253,
     252,     5,     5,    30,     7,   104,   104,    28,    11,    40,
      31,   104,    24,   257,   289,   259,    47,    48,     5,    50,
       7,   273,   266,   267,    55,   269,   270,   279,    55,   637,
      31,   639,    31,    21,    33,    34,   280,   281,    59,   307,
     284,   285,   103,    31,   105,    76,    67,   299,    79,   324,
      77,    78,   296,    31,    42,    33,    34,   301,   302,   303,
     103,    64,   105,    64,   308,    70,    67,    98,    67,     5,
      71,     7,     8,   104,   326,   319,   320,   104,   776,    67,
      68,   325,   780,   335,    31,    73,   784,   785,    56,    67,
      37,   429,   594,    81,    32,    84,    64,    86,    97,     5,
      89,     7,    91,   104,   348,   104,     5,   351,     7,   353,
     385,   103,    11,   381,    82,     5,   104,     7,     8,   103,
      67,   105,   366,   367,   104,   393,   104,   776,   783,   784,
     776,   780,   376,   377,   780,   784,   580,   389,   784,   785,
     786,   104,   750,   751,   482,   103,   390,   105,    54,   108,
       8,   395,   396,   397,   398,   399,   494,   104,    16,    17,
      18,    27,    85,   103,    30,   105,   418,   109,    74,   255,
     773,   102,   775,   776,   442,   443,   779,   445,   781,   782,
     783,   784,   785,   786,   787,    51,    52,    53,   374,   103,
     434,   105,   775,   103,   380,   105,   382,   461,   103,   782,
     783,   103,   103,   105,   105,   782,   783,  1185,   103,    75,
    1185,   455,    78,  1201,  1202,  1185,   468,  1172,   462,   471,
    1185,   473,   781,  1201,   103,  1203,  1201,    27,  1203,   374,
      30,  1201,   484,  1203,   103,   380,   105,   382,   104,   491,
     492,   103,   103,   105,   374,   110,   103,   782,   783,   501,
     106,    51,    52,    53,   103,    55,   105,   111,   502,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,    22,    31,  1159,    77,    78,   103,
     106,   105,    31,    31,    33,    34,    42,   539,   103,   103,
     580,   105,    14,    31,    42,    33,    34,   103,   542,   543,
     544,    30,    31,    31,   104,   549,   392,    63,   103,   106,
     105,    67,   587,   103,    42,   559,   560,    73,    67,    67,
     103,   565,   105,   598,  1185,    73,   580,   106,   103,    67,
     105,   104,   339,   419,   420,  1015,   338,  1142,    67,    67,
    1201,   585,  1203,  1185,    73,    73,    75,   337,   104,    78,
       5,   603,    80,   339,     5,   104,   104,   357,   431,  1201,
    1202,   613,   103,   971,   105,   382,   104,     5,   103,     7,
     105,   537,   103,    11,   105,   104,   104,   103,   452,   105,
      60,    61,    62,    63,    64,    65,    24,    67,   369,    69,
     642,   410,    72,    73,    32,    31,    76,   641,   775,   643,
     644,   775,   775,   589,   590,   775,    42,   651,   594,   595,
     103,   103,   105,   105,   775,   103,   659,   105,   659,   103,
     775,   105,   760,   775,   103,   103,   105,   105,   775,   632,
     768,    67,   676,  1166,  1132,  1133,  1060,    73,    27,  1093,
     784,    30,    31,   687,   589,   590,   103,   715,   105,   594,
     595,   571,  1131,    42,   322,   364,   347,   324,  1051,   589,
     590,  1016,    51,    52,    53,   803,    55,   365,   104,   641,
     714,   793,   436,   659,  1172,    -1,    65,    -1,    67,    -1,
     566,   725,   568,   569,   570,    -1,   572,   573,   574,   575,
     576,   577,   578,   579,  1102,   581,    -1,    -1,    -1,    -1,
     752,   753,    -1,   755,   772,    -1,    -1,    -1,    -1,    -1,
      -1,   763,   776,    22,   659,   104,   780,    -1,   770,    -1,
     784,   785,   786,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    27,    -1,  1141,    -1,    31,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,   793,
     794,   795,    61,    62,    -1,    51,   800,    66,   802,    55,
      69,   805,   806,   749,    -1,    -1,    75,    -1,    -1,  1172,
      -1,    67,   816,   817,    -1,    -1,    -1,    73,  1186,    -1,
      -1,    -1,  1185,    -1,    -1,    81,    -1,   773,   774,   775,
     776,   777,   778,   779,   780,   781,   782,   783,   784,   785,
     786,   787,  1185,    -1,   749,    -1,    -1,    -1,   104,   289,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   861,  1201,  1202,
    1203,    -1,    -1,   867,  1201,  1202,  1203,   307,   773,   774,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   773,   324,   775,   776,   777,    -1,   779,
     780,   781,   782,   783,   784,   785,   786,   787,    -1,   903,
     904,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1203,    -1,
      -1,    -1,    -1,    -1,    -1,   919,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   365,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   947,   948,    -1,    -1,   974,
      -1,   381,    -1,    -1,    -1,   385,    -1,    -1,    -1,   977,
     978,    -1,    -1,   393,    -1,    -1,   960,   961,     9,    10,
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,  1080,    -1,    -1,    -1,  1084,    -1,    -1,    -1,
    1088,    -1,     4,    -1,    -1,    -1,   436,    -1,    -1,    -1,
      -1,    -1,   442,   443,    -1,   445,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    26,    -1,  1051,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,
      42,  1035,  1036,    -1,    -1,    47,    48,    -1,    50,    -1,
    1052,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,  1069,    -1,    -1,
    1072,    73,  1074,    -1,  1076,    -1,  1078,    79,    -1,    81,
    1082,    -1,    -1,    -1,  1086,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,  1097,    98,    -1,    -1,  1101,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1105,    79,    -1,    81,    -1,  1141,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1120,  1121,  1122,  1123,
    1124,    -1,  1126,    -1,  1128,  1223,  1130,    -1,    -1,     4,
      -1,  1135,    -1,    -1,    -1,  1139,   982,   983,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   587,    -1,    -1,
      -1,    26,    27,    -1,    29,  1001,    31,    -1,   598,    27,
      -1,    -1,    30,    31,    -1,    40,    -1,    42,    -1,  1173,
      -1,    46,    47,    48,    42,    50,    -1,    52,    53,    -1,
      55,  1216,    57,    58,    52,    53,    31,    55,    -1,    -1,
      58,    -1,    67,    -1,    -1,    40,    -1,    42,    73,    67,
      45,    76,    77,    -1,    79,    73,    81,    75,  1220,  1221,
      78,    -1,    -1,    81,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    96,    67,    98,    -1,  1171,  1172,     4,    73,   104,
      -1,    76,    -1,    -1,    -1,    -1,   104,    -1,  1184,  1185,
      -1,    -1,    -1,    -1,    -1,    -1,  1250,  1251,    25,    26,
      27,    -1,    29,    30,    31,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    42,  1171,  1172,    -1,    -1,
      47,    48,    -1,    50,    51,   715,    -1,    -1,    -1,  1184,
    1185,   258,  1172,   260,    -1,    -1,    -1,   264,   265,    -1,
      67,   268,    -1,    -1,    -1,    -1,    73,     4,    -1,    -1,
      -1,    -1,   353,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   366,   367,    -1,    25,    26,
      -1,    98,    29,    -1,    31,   376,   377,   104,    -1,    -1,
      -1,    -1,   772,    -1,    -1,    42,    -1,     4,    -1,    -1,
      47,    48,    -1,    50,   395,   396,   397,   398,   399,    -1,
      57,    -1,    -1,    -1,   331,    -1,    -1,   334,    -1,    26,
      67,    -1,    29,    -1,   341,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    79,    40,    81,    -1,    -1,    -1,    -1,    -1,
      47,    48,   359,    50,    -1,    -1,    -1,    -1,    55,    96,
      -1,    98,    -1,    -1,   371,    27,    -1,   104,    30,    31,
      -1,   378,   379,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      42,   462,    79,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    55,   401,    -1,    -1,   404,   405,   406,
      -1,    98,    -1,    -1,    -1,    67,    -1,   104,   415,    -1,
      -1,    73,    -1,    75,    -1,    -1,    78,    -1,    -1,    81,
      -1,   502,   503,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   517,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,     4,    -1,   463,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   474,    -1,   476,
     477,    -1,    26,    -1,    -1,    29,    26,    31,    -1,    29,
      -1,    31,    -1,    -1,    -1,    -1,    40,    -1,    -1,   496,
     497,   498,    42,    47,    48,    -1,    50,    47,    48,    -1,
      50,    -1,    -1,    57,   974,    -1,    -1,   977,   978,    -1,
      -1,   518,    -1,    67,    -1,    -1,    -1,    67,    -1,    73,
      -1,    -1,    76,    73,    -1,    79,    -1,    -1,    -1,    79,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,   550,    98,    -1,   553,    -1,    98,   556,
     104,  1021,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   643,   644,    -1,    -1,    -1,    -1,    -1,    -1,
     651,    -1,    -1,    -1,  1044,  1045,    -1,    -1,     4,    -1,
      57,  1051,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      26,    -1,    -1,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,
      -1,    47,    48,    -1,    50,    -1,    -1,   634,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,   649,    -1,   725,    -1,    -1,    73,    -1,   656,
      76,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   670,    -1,    -1,    -1,   674,    -1,    -1,
     677,  1141,    98,   680,    -1,    -1,   683,    -1,   104,   686,
      -1,   688,    -1,    -1,   691,    -1,    -1,   694,  1158,    -1,
     697,   698,   699,   700,    -1,    -1,   703,    -1,    -1,   706,
      -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   805,   806,    -1,    -1,    26,    27,
      -1,    29,    -1,    31,    -1,   816,   817,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    42,    -1,  1216,    -1,    -1,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,   253,    -1,    76,    -1,
     861,    79,    -1,    81,    -1,    -1,   867,    -1,    -1,   796,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    -1,
      98,    -1,    -1,    -1,   811,   812,   104,    -1,   815,    -1,
      -1,    -1,    -1,    -1,    -1,   822,   823,   824,   825,   826,
     827,    -1,   903,   904,    -1,   832,    -1,    -1,    -1,    -1,
     307,    -1,   839,    -1,    -1,    -1,    -1,   844,   919,    -1,
      -1,    -1,    -1,    -1,   851,   852,   853,   854,   855,   856,
     857,    -1,    -1,   860,    -1,    -1,    -1,    -1,   865,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   875,    -1,
      -1,    -1,    -1,   880,   881,   882,   883,   884,    -1,   960,
     961,    -1,   889,    -1,    -1,    -1,    -1,    -1,   895,   896,
      -1,    -1,   899,    -1,    -1,   902,    -1,    -1,    -1,    -1,
      -1,    -1,   909,   910,   381,    -1,   913,    -1,    -1,    -1,
      -1,   918,    -1,    -1,    -1,    -1,   393,   924,   925,   926,
     927,   928,   929,   930,    -1,    -1,    -1,    -1,   935,    -1,
      -1,    -1,    -1,    -1,    -1,   942,   943,   944,   945,   946,
      -1,    -1,    -1,    -1,    -1,    -1,   953,   954,   955,    -1,
     957,    -1,   959,     4,    -1,    -1,    -1,    -1,    -1,   966,
     967,   968,   969,    -1,   441,   442,   443,    -1,   445,   446,
      21,    -1,    -1,    -1,    -1,    26,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    -1,    40,
      41,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    50,
      -1,    -1,    -1,    22,    23,    -1,    57,    -1,    27,    28,
      -1,    30,    31,    -1,    33,    34,    -1,    -1,    -1,    -1,
      -1,    72,    -1,    -1,    -1,    76,    -1,    46,    79,    -1,
      -1,    -1,    51,  1040,    -1,  1042,    -1,    56,    -1,    58,
      59,    -1,    61,    -1,    -1,    64,    65,    98,    67,  1130,
      -1,    -1,    71,    -1,    73,    -1,    75,    76,    -1,    78,
     537,  1068,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     567,    -1,  1173,    -1,   571,    -1,    -1,    -1,    -1,  1106,
      -1,    -1,  1109,   580,    -1,  1112,    -1,    -1,    -1,  1116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1134,    -1,  1136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1146,
      -1,    -1,  1149,    -1,    -1,  1152,    -1,    -1,  1155,    -1,
      -1,    -1,    -1,    -1,  1161,  1162,  1163,  1164,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1195,  1196,
    1197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1208,    -1,    -1,    -1,    -1,    -1,    -1,  1215,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1232,    -1,    -1,    -1,    -1,
    1237,    -1,    -1,  1240,    -1,    -1,    -1,    -1,   715,   716,
      -1,   718,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   772,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,     4,    -1,    -1,    -1,    -1,
     104,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     977,   978,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1015,  1016,
      -1,    -1,    -1,    -1,  1021,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,  1044,  1045,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    16,    17,
      18,  1198,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,     4,    -1,    -1,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,     4,    -1,    -1,    -1,
       8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,     4,    -1,    -1,    -1,     8,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,     4,    -1,    -1,    -1,
      -1,    -1,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,     4,     5,    -1,    -1,
      -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,     4,    -1,    -1,    -1,     8,    -1,    10,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,     4,    -1,    -1,    -1,
      -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   113,   119,   123,   418,   429,     0,     5,     7,    54,
      74,   417,    22,    23,    27,    28,    30,    31,    33,    34,
      46,    51,    56,    58,    59,    61,    64,    65,    67,    71,
      73,    78,   129,   139,   141,   143,   146,   148,   150,   152,
     175,   189,   226,   240,   242,   244,   246,   338,   341,   344,
     352,   362,   427,   428,   417,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   402,    75,   193,
      76,   191,   418,     4,     8,    10,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,   114,   115,   118,   395,   424,
     425,   122,   395,    11,   115,   188,   397,   425,   426,   140,
     395,    88,    93,   115,   337,   339,     9,    11,    12,    16,
      17,    18,   115,   147,   421,   423,   424,   245,   421,   149,
     421,   241,   421,   239,   421,    11,   115,   361,   363,   396,
     424,    88,    93,   115,   340,   342,   128,   421,    92,    95,
     115,   351,   353,   145,   421,   115,   225,   371,   372,   373,
     115,   142,   394,   425,   343,   421,   151,   421,    12,   115,
     138,   159,    85,    87,    90,   115,   174,   176,   243,   421,
     403,   417,   426,   419,   402,   419,   402,   418,   103,   418,
     103,   103,   105,   398,   399,   401,   418,   418,   103,   418,
     418,   398,   398,   422,   398,   398,   398,   398,   398,   364,
     418,   418,   398,   398,   418,   418,   398,   398,   398,   106,
     374,   375,    14,   377,   378,   398,   418,   398,   398,   160,
     398,   418,   418,   418,   398,   398,   404,   405,   418,   194,
     397,   190,   395,   116,   402,   402,   402,   402,   402,   418,
     418,   396,   376,   106,   379,   418,   103,   105,   410,   406,
     421,   103,   105,   195,   103,   102,   120,   121,   124,   125,
     104,   400,   143,   382,   374,   378,   380,   396,   418,   402,
     402,   418,   130,    82,   127,   129,   143,   130,   127,   141,
     402,   104,   107,   381,   382,   411,    24,    32,   196,   197,
     214,   216,   230,   232,   192,     8,    43,    44,   131,   135,
     144,   419,   144,   402,     5,   420,   375,   104,   403,   412,
     414,   424,   419,   419,   104,    39,    49,    62,    66,    69,
     189,   198,   199,   201,   203,   208,   223,   226,   228,   402,
     231,   104,   150,   152,   175,   193,   244,   246,   117,   419,
     419,   402,   402,   146,   148,   150,   152,   153,   155,   126,
     421,   153,   383,   396,   418,   407,   419,   233,   395,   217,
     421,   419,   419,   207,   419,   419,   419,   402,   193,   402,
     402,   402,   215,   232,   402,   418,   132,   395,   134,   395,
     161,   162,    70,   156,   398,   161,   420,   420,   103,   105,
     413,     8,   408,   423,   103,   105,   234,   103,   105,   218,
      17,   115,   200,   202,   390,   204,   421,   209,   421,   365,
     421,    88,    93,   115,   227,   229,   216,   103,   103,   105,
     136,   104,     4,    21,    26,    29,    35,    36,    38,    40,
      41,    47,    48,    50,    57,    72,    79,    98,   163,   165,
     178,   185,   191,   248,   251,   253,   256,   260,   263,   266,
     276,   278,   282,   295,   300,   309,   313,   419,   402,   104,
     108,   109,   418,   418,   418,   402,   402,   402,   398,   418,
     103,   105,   205,   103,   105,   210,   103,   105,   224,   418,
     418,   398,   402,   402,   402,   418,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   402,   154,   159,   418,   420,   415,   409,   235,
     219,   402,   402,   402,   133,   137,   275,   395,   106,   115,
     293,   294,   366,   367,   265,   395,   252,   395,   164,   395,
     115,   312,   315,   367,   177,   395,   247,   395,   184,   395,
     257,   395,   259,   395,   262,   395,   308,   395,   299,   395,
     281,   397,   277,   395,   103,   105,   157,    85,   384,   389,
     104,   396,   102,    42,    63,   104,   150,   152,   175,   182,
     237,    80,   104,   150,   152,   175,   182,   221,   206,   211,
     206,   104,   139,   143,   150,   152,   104,   139,   150,   152,
     103,   105,   279,   396,   103,   368,   418,   103,   105,   267,
     103,   105,   254,   103,   105,   166,   103,   418,   103,   105,
     179,   103,   105,   249,   103,   105,   186,   103,   103,   103,
     103,   105,   310,   103,   105,   301,   103,   105,   283,   279,
     402,   420,   420,   407,   418,   419,   419,   402,   419,   402,
     104,   150,   152,   240,   242,    97,   104,   150,   152,   213,
     240,   242,   104,   402,   402,   366,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   158,
     110,   106,   103,   105,   416,     8,   181,   421,    18,   115,
     236,   238,   390,   391,    16,    17,    18,   115,   220,   222,
     392,   393,   419,   280,   296,   268,   255,   167,   314,   180,
     250,   187,   258,   261,   264,   311,   302,   284,   104,   150,
     152,   420,   420,   418,   418,   418,   103,   105,   183,   398,
     418,   398,   418,   212,   421,    55,    81,   104,   150,   152,
     175,   182,   334,   338,   341,   355,    25,    96,   104,   150,
     152,   175,   182,   251,   272,   298,   309,   334,   104,   150,
     152,   175,   182,   246,   253,   256,   260,   263,   266,   269,
     270,   272,   276,   278,   334,   338,   341,   104,   150,   152,
     175,   182,   191,   248,   251,   298,   309,   334,   338,   344,
     355,    20,   104,   150,   152,   169,   175,    37,   104,   150,
     152,   104,   150,   152,   175,   182,   104,   150,   152,   175,
     191,   248,   251,   298,   309,   104,   150,   152,   175,   182,
     189,   104,   150,   152,   175,   182,   193,   244,   246,   334,
     338,   341,   355,    52,    53,   104,   150,   152,   175,   182,
     193,   244,   246,   334,   338,   346,   349,   352,   355,    77,
     104,   150,   152,   175,   182,   191,   248,   251,   298,   309,
     334,   338,   346,   349,   352,   355,   358,   362,   104,   150,
     152,   175,   182,   191,   248,   251,   355,    45,    60,   104,
     150,   152,   175,   182,   191,   248,   303,   304,   306,   307,
      21,    68,   104,   150,   152,   175,   182,   287,   292,   334,
     111,    14,   385,   386,   415,   402,   398,   419,   419,   402,
     402,   402,   419,   419,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   419,   402,   419,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   419,   419,   402,   402,   402,
     402,   419,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   418,   418,   402,   402,   402,
     103,   402,   103,   402,   419,   419,   402,   402,   402,   402,
     420,   387,   104,   104,   354,   421,   333,   421,   271,   395,
     297,   395,   168,   395,    84,    86,    89,    91,   316,   317,
     318,   320,   321,   322,   325,   326,   329,   330,    94,   115,
     348,   350,   390,   115,   345,   347,   391,   115,   357,   359,
     369,   396,   402,   402,   115,   290,   291,   369,   115,   285,
     286,   369,   106,   386,   396,   418,   103,   105,   356,   103,
     105,   335,   103,   105,   273,   301,   103,   105,   170,   402,
     418,   418,   418,   418,   418,   398,   418,   398,   418,   398,
     360,   370,   305,   305,   103,   418,   103,   105,   288,   418,
     420,   388,   402,   402,   402,   402,   103,   105,   319,   103,
     105,   323,   103,   105,   327,   103,   105,   331,   419,   368,
     104,   191,   248,   251,   355,   104,   402,   402,   396,   420,
     206,   336,   274,    83,   172,   402,   402,   402,   402,   369,
     402,   402,   402,   402,   296,   289,   106,   104,   104,   150,
     152,   104,   150,   152,   175,   182,   251,   334,   419,   104,
     104,   324,   328,   332,   104,   104,   150,   152,   182,   246,
     338,   341,   344,   346,   349,   355,   420,   402,   402,   402,
      88,    93,   115,   171,   173,   104,   244,   246,   338,   341,
     346,   349,   355,   358,   104,   244,   246,   355,   358,   104,
     193,   244,   246,   338,   341,   346,   349,   402,   402,   396,
     418,   418,   398,   402,   402,   402
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   112,   113,   113,   113,   114,   115,   116,   117,   116,
     118,   119,   120,   121,   121,   121,   121,   122,   123,   124,
     125,   125,   125,   126,   127,   128,   129,   130,   130,   130,
     131,   132,   133,   133,   133,   133,   133,   134,   135,   136,
     136,   137,   137,   137,   137,   138,   139,   140,   141,   142,
     143,   144,   144,   144,   144,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   155,   156,   157,
     157,   158,   158,   158,   160,   159,   159,   161,   162,   162,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     164,   165,   166,   166,   167,   167,   167,   167,   167,   168,
     169,   170,   170,   171,   172,   172,   173,   173,   173,   174,
     175,   176,   176,   176,   176,   177,   178,   179,   179,   180,
     180,   180,   180,   180,   181,   182,   183,   183,   184,   185,
     186,   186,   187,   187,   187,   187,   187,   187,   188,   189,
     190,   191,   192,   192,   192,   192,   192,   192,   192,   193,
     194,   195,   195,   196,   196,   196,   197,   197,   197,   197,
     197,   197,   197,   197,   197,   198,   199,   200,   201,   202,
     202,   203,   204,   205,   205,   206,   206,   206,   206,   206,
     207,   208,   209,   210,   210,   211,   211,   211,   211,   211,
     211,   212,   213,   214,   215,   215,   216,   217,   218,   218,
     219,   219,   219,   219,   219,   219,   220,   221,   222,   222,
     223,   224,   224,   225,   226,   227,   228,   229,   229,   229,
     230,   231,   231,   232,   233,   234,   234,   235,   235,   235,
     235,   235,   235,   236,   237,   238,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   249,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   251,   251,
     251,   251,   251,   251,   251,   251,   252,   253,   254,   254,
     255,   255,   255,   255,   255,   255,   255,   255,   255,   255,
     255,   255,   255,   255,   256,   257,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   259,   260,
     261,   261,   261,   261,   261,   261,   261,   261,   261,   261,
     261,   261,   261,   261,   262,   263,   264,   264,   264,   264,
     264,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     264,   264,   264,   264,   265,   266,   267,   267,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   269,   269,
     270,   270,   270,   270,   270,   270,   270,   271,   272,   273,
     273,   274,   274,   274,   274,   274,   274,   274,   275,   276,
     277,   278,   279,   279,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   281,   282,   283,   283,   284,   284,   284,
     284,   284,   284,   284,   284,   285,   285,   286,   287,   288,
     288,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   290,   290,   291,   292,   293,   293,   294,   295,
     296,   296,   296,   296,   296,   296,   296,   296,   296,   296,
     297,   298,   299,   300,   301,   301,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   303,   304,   305,   305,   305,
     305,   305,   306,   307,   308,   309,   310,   310,   311,   311,
     311,   311,   311,   311,   311,   311,   311,   312,   313,   314,
     314,   314,   314,   315,   315,   316,   316,   317,   318,   319,
     319,   320,   320,   320,   321,   322,   323,   323,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   325,   326,   327,
     327,   328,   328,   328,   328,   328,   329,   330,   331,   331,
     332,   332,   332,   332,   332,   332,   332,   332,   333,   334,
     335,   335,   336,   336,   336,   337,   338,   339,   339,   339,
     340,   341,   342,   342,   342,   343,   344,   345,   346,   347,
     347,   348,   349,   350,   350,   350,   351,   352,   353,   353,
     353,   354,   355,   356,   356,   357,   358,   359,   359,   360,
     360,   361,   362,   364,   363,   363,   365,   366,   367,   368,
     368,   370,   369,   372,   371,   373,   371,   371,   374,   375,
     376,   376,   377,   378,   379,   379,   380,   381,   381,   382,
     382,   383,   384,   385,   386,   387,   387,   388,   388,   389,
     390,   391,   391,   392,   392,   393,   393,   394,   394,   395,
     395,   396,   396,   397,   397,   397,   398,   398,   399,   400,
     401,   402,   402,   402,   403,   404,   405,   406,   406,   407,
     407,   408,   408,   408,   409,   409,   410,   410,   411,   411,
     412,   412,   413,   413,   414,   415,   415,   416,   416,   417,
     417,   418,   418,   419,   420,   420,   422,   421,   421,   423,
     423,   423,   423,   423,   423,   423,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   424,   424,   424,
     424,   424,   424,   424,   424,   424,   424,   425,   426,   427,
     428,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429
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
       1,     3,     1,     4,     1,     0,     4,     2,     5,     1,
       1,     0,     2,     2,     0,     1,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       0,     0,     3,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     4,     4,
       2,     2,     2,     2
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
          case 114: /* tmp_string  */

      { free((((*yyvaluep).p_str)) ? *((*yyvaluep).p_str) : NULL); }

        break;

    case 209: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 399: /* semicolom  */

      { free(((*yyvaluep).str)); }

        break;

    case 401: /* curly_bracket_open  */

      { free(((*yyvaluep).str)); }

        break;

    case 405: /* string_opt_part1  */

      { free(((*yyvaluep).str)); }

        break;

    case 427: /* type_ext_alloc  */

      { yang_type_free(param->module->ctx, ((*yyvaluep).v)); }

        break;

    case 428: /* typedef_ext_alloc  */

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

    { YANG_ADDELEM(trg->rev, trg->rev_size);
                                  actual = yang_read_revision(trg, s, actual);
                                  (yyval.token) = actual_type;
                                  actual_type = REVISION_KEYWORD;
                                }

    break;

  case 68:

    { actual_type = (yyvsp[-1].token);
                                                                     actual = NULL;
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
                                   if (((struct lys_ext *)actual)->argument) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                      free(s);
                                      YYABORT;
                                   }
                                   ((struct lys_ext *)actual)->argument = lydict_insert_zc(param->module->ctx, s);
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

    { ((struct lys_ext *)actual)->flags |= (yyvsp[-1].uint); }

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

    { if (yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

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
                         if (!(actual = yang_read_length(trg, actual, s))) {
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
                   actual_type = PATTERN_KEYWORD;
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

    { if (((struct yang_type *)actual)->base != 0 && ((struct yang_type *)actual)->base != LY_TYPE_STRING) {
                            free(s);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected pattern statement.");
                            YYABORT;
                          }
                          ((struct yang_type *)actual)->base = LY_TYPE_STRING;
                          yang_type = actual;
                          YANG_ADDELEM(((struct yang_type *)actual)->type->info.str.patterns,
                                       ((struct yang_type *)actual)->type->info.str.pat_count);
                          (yyval.str) = s;
                          s = NULL;
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

    { ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                      /* keep the highest enum value for automatic increment */
                      if ((yyvsp[-1].i) >= cnt_val) {
                        cnt_val = (yyvsp[-1].i);
                        cnt_val++;
                      }
                    }

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

    { ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                         /* keep the highest position value for automatic increment */
                         if ((yyvsp[-1].uint) >= cnt_val) {
                           cnt_val = (yyvsp[-1].uint);
                           cnt_val++;
                         }
                       }

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
                        if (!(actual = yang_read_range(trg, actual, s))) {
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

    {  struct lys_ext_instance *ext = actual;
        int32_t length = 0, old_length = 0;
        char *tmp_value;

        length = strlen((yyvsp[-2].str));
        old_length = (ext->parent) ? strlen((char *)ext->parent) + 2 : 2;
        tmp_value = realloc(ext->parent, old_length + length + 1);
        if (!tmp_value) {
          LOGMEM;
          YYABORT;
        }
        ext->parent = tmp_value;
        tmp_value += old_length - 2;
        memcpy(tmp_value, (yyvsp[-2].str), length);
        tmp_value[length] = ' ';
        tmp_value[length + 1] = '\0';
        tmp_value[length + 2] = '\0';
      }

    break;

  case 644:

    { (yyval.str) = yyget_text(scanner); }

    break;

  case 656:

    { s = strdup(yyget_text(scanner));
                  if (!s) {
                    LOGMEM;
                    YYABORT;
                  }
                }

    break;

  case 747:

    { s = strdup(yyget_text(scanner));
                          if (!s) {
                            LOGMEM;
                            YYABORT;
                          }
                        }

    break;

  case 748:

    { s = strdup(yyget_text(scanner));
                                    if (!s) {
                                      LOGMEM;
                                      YYABORT;
                                    }
                                  }

    break;

  case 749:

    { struct lys_type **type;

                             type = (struct lys_type **)yang_getplace_for_extcomplex_struct(ext_name, "type", ext_instance, LY_STMT_TYPE);
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

  case 750:

    { struct lys_tpdf **tpdf;

                                tpdf = (struct lys_tpdf **)yang_getplace_for_extcomplex_struct(ext_name, "typedef", ext_instance, LY_STMT_TYPEDEF);
                                if (!tpdf) {
                                  YYABORT;
                                }
                                /* allocate typedef structure */
                                (*tpdf) = calloc(1, sizeof **tpdf);
                                (yyval.v) = actual = *tpdf;
                              }

    break;

  case 751:

    { actual_type = EXTENSION_INSTANCE;
                                actual = ext_instance;
                                if (!is_ext_instance) {
                                  LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
                                  YYABORT;
                                }
                              }

    break;

  case 753:

    { if (yang_read_extcomplex_str(trg, ext_instance, "prefix", ext_name, s,
                                                                  0, LY_STMT_PREFIX)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 754:

    { if (yang_read_extcomplex_str(trg, ext_instance, "description", ext_name, s,
                                                                       0, LY_STMT_DESCRIPTION)) {
                                            YYABORT;
                                          }
                                        }

    break;

  case 755:

    { if (yang_read_extcomplex_str(trg, ext_instance, "reference", ext_name, s,
                                                                     0, LY_STMT_REFERENCE)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 756:

    { if (yang_read_extcomplex_str(trg, ext_instance, "units", ext_name, s,
                                                                     0, LY_STMT_UNITS)) {
                                      YYABORT;
                                    }
                                  }

    break;

  case 757:

    { if (yang_read_extcomplex_str(trg, ext_instance, "base", ext_name, s,
                                                                0, LY_STMT_BASE)) {
                                     YYABORT;
                                   }
                                 }

    break;

  case 758:

    { if (yang_read_extcomplex_str(trg, ext_instance, "contact", ext_name, s,
                                                                     0, LY_STMT_CONTACT)) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 759:

    { if (yang_read_extcomplex_str(trg, ext_instance, "default", ext_name, s,
                                                                     0, LY_STMT_DEFAULT)) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 760:

    { if (yang_read_extcomplex_str(trg, ext_instance, "error-message", ext_name, s,
                                                                         0, LY_STMT_ERRMSG)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 761:

    { if (yang_read_extcomplex_str(trg, ext_instance, "error-app-tag", ext_name, s,
                                                                         0, LY_STMT_ERRTAG)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 762:

    { if (yang_read_extcomplex_str(trg, ext_instance, "key", ext_name, s,
                                                               0, LY_STMT_KEY)) {
                                    YYABORT;
                                  }
                                }

    break;

  case 763:

    { if (yang_read_extcomplex_str(trg, ext_instance, "namespace", ext_name, s,
                                                                     0, LY_STMT_NAMESPACE)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 764:

    { if (yang_read_extcomplex_str(trg, ext_instance, "organization", ext_name, s,
                                                                        0, LY_STMT_ORGANIZATION)) {
                                             YYABORT;
                                           }
                                         }

    break;

  case 765:

    { if (yang_read_extcomplex_str(trg, ext_instance, "path", ext_name, s,
                                                                0, LY_STMT_PATH)) {
                                     YYABORT;
                                   }
                                 }

    break;

  case 766:

    { if (yang_read_extcomplex_str(trg, ext_instance, "presence", ext_name, s,
                                                                    0, LY_STMT_PRESENCE)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 767:

    { if (yang_read_extcomplex_str(trg, ext_instance, "revision-date", ext_name, s,
                                                                         0, LY_STMT_REVISIONDATE)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 768:

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

  case 769:

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

  case 770:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "status", LY_STMT_STATUS,
                                                                    (yyvsp[0].i), LYS_STATUS_MASK)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 771:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "config", LY_STMT_CONFIG,
                                                                    (yyvsp[0].i), LYS_CONFIG_MASK)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 772:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "mandatory", LY_STMT_MANDATORY,
                                                                       (yyvsp[0].i), LYS_MAND_MASK)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 773:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "ordered-by", LY_STMT_ORDEREDBY,
                                                                        (yyvsp[0].i), LYS_ORDERED_MASK)) {
                                           YYABORT;
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
