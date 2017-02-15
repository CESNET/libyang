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
#define YYLAST   3733

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  318
/* YYNRULES -- Number of rules.  */
#define YYNRULES  769
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1252

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
    4277,  4282,  4287,  4292,  4297,  4302,  4307,  4312,  4317,  4330
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

#define YYPACT_NINF -971

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-971)))

#define YYTABLE_NINF -751

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     209,    60,  -971,  -971,   325,  1744,  -971,  -971,  -971,   107,
     107,  -971,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,    -7,    -5,  -971,  3540,  3540,  3065,
    3540,  2875,  2875,  2875,  2875,  2875,  3160,  2875,  2875,    49,
    3540,  2875,  2875,    32,  2875,   498,   107,  -971,   107,  -971,
     107,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,   -12,  -971,  -971,  -971,    -6,
    -971,  -971,  -971,    41,  -971,  -971,  -971,    29,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,    41,  -971,  -971,  -971,
      41,  -971,    41,  -971,    41,  -971,    41,  -971,  -971,  -971,
      41,  -971,  -971,  -971,    41,  -971,    41,  -971,  -971,    41,
    -971,    22,    87,  -971,    41,  -971,  -971,    41,  -971,    41,
    -971,  -971,  -971,    41,  -971,    41,  -971,  -971,  -971,  -971,
    3065,   498,  3540,   498,   107,  -971,   107,  -971,  -971,  -971,
    -971,  -971,  -971,   107,   107,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  3635,  -971,  -971,
      37,  -971,  -971,  -971,   107,  -971,  -971,  -971,  -971,  -971,
      54,  2875,   107,   110,  -971,    46,  -971,    63,   498,   498,
     498,   119,   324,   107,   107,  -971,    22,  -971,  3255,   107,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,   344,  -971,    47,  -971,  -971,    94,   111,  -971,  -971,
    -971,   213,   107,   641,   498,   534,   308,   107,  -971,  -971,
    -971,   308,  -971,  -971,   498,  -971,   216,  -971,   321,  2506,
     107,   107,   120,   868,  -971,  -971,  -971,  -971,   235,  -971,
     107,   107,  -971,  -971,   240,  2875,   240,   498,  -971,  3635,
    -971,  -971,  -971,  -971,   107,  -971,  3540,  2875,  -971,   107,
     107,   107,   107,   107,  -971,  -971,    -7,  -971,  -971,  -971,
    -971,  -971,  -971,   498,   204,   107,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  3540,  3540,   498,   498,  -971,  -971,
    -971,  -971,  -971,   161,    41,  -971,  -971,   216,   216,   107,
     239,  2970,   254,  -971,   333,  -971,   182,  2875,  2875,  -971,
    2875,   115,   498,  -971,   498,   498,   498,   224,  -971,   177,
     498,   107,   133,  -971,   391,  -971,   141,  1723,   107,  -971,
    -971,   145,   165,   168,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,    41,  -971,
    -971,   408,  -971,   469,  -971,   483,  -971,  -971,  -971,  -971,
      41,  -971,  -971,  -971,  -971,  -971,  -971,    41,  -971,  -971,
    -971,  -971,  -971,  -971,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,    32,
     498,  -971,  -971,   216,   107,   107,   107,   498,   498,   498,
    -971,   107,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,   107,   107,  -971,   498,   107,   107,   107,  -971,   498,
     498,   107,  3540,    25,  3540,  3540,  3540,    25,  3540,  3540,
    3540,  3540,  3540,  3540,  3540,  3540,  3065,  3540,   498,   488,
    -971,   107,   206,  2683,   198,   338,   514,   498,   498,   498,
     342,    23,   504,  -971,  3635,  -971,  -971,   212,  -971,  -971,
     516,  -971,   519,  -971,   530,  -971,  -971,   219,  -971,  -971,
     540,  -971,   561,  -971,   567,  -971,   225,  -971,   234,  -971,
     255,  -971,   572,  -971,   577,  -971,   580,  -971,   504,  -971,
    -971,  -971,  -971,   216,  -971,   216,  -971,   107,  -971,   107,
     107,  -971,  -971,  -971,  -971,  -971,  -971,   107,  -971,  -971,
    -971,  -971,  -971,  -971,   424,   481,   537,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,   203,   107,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,   107,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,   498,   202,   237,   583,
     575,  2875,   306,   498,   288,   498,  -971,  -971,  -971,  -971,
    -971,   107,  -971,  -971,  -971,  -971,  -971,  -971,  -971,   498,
     498,  -971,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,    88,   216,   216,  -971,  -971,
    -971,  -971,   586,  -971,  -971,  -971,    41,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,    41,  -971,  -971,  -971,  2875,  1447,
     611,    91,   121,   207,    78,   558,  1135,   553,  1285,  1228,
    1350,   945,  1450,   329,  -971,  -971,  -971,   271,   381,   107,
     107,   107,  -971,  -971,  -971,  -971,   107,  -971,   107,    41,
    -971,   107,   107,   107,   107,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,   107,   107,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,   107,
    -971,  -971,  -971,  -971,  -971,   107,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,   107,   107,   107,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,   107,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,   295,  -971,   301,  -971,   107,
     107,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
     216,  -971,  -971,  2778,   149,  -971,   158,   166,  2875,  2875,
     498,   498,   498,  3540,  3540,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   498,  3540,   498,   348,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,   498,    68,   306,    75,   498,
     498,   498,   498,  3160,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   107,   107,   498,
     498,   498,  -971,   498,  -971,   498,  3160,  3160,   498,   498,
     498,   498,   310,  3350,  -971,  -971,  -971,  -971,  -971,    41,
    -971,  -971,  -971,  -971,    41,  -971,   605,  -971,   614,  -971,
     619,  -971,   577,  -971,   640,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,    41,  -971,  -971,  -971,    41,  -971,  -971,  -971,
    -971,  -971,    41,  -971,  -971,    41,  -971,  -971,  -971,   498,
     498,  -971,  -971,   309,  -971,  -971,  -971,   643,  -971,   216,
    -971,  -971,   107,   107,   107,  -971,   107,   107,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,   498,    48,   106,   129,   130,   107,  -971,   107,
    -971,   107,   107,   107,  -971,  -971,   107,  -971,  1026,  1206,
    -971,   107,  -971,  -971,  -971,   107,  -971,  3445,   498,   498,
     498,    97,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  3635,   203,  -971,  -971,  -971,  -971,
    -971,  -971,   498,   498,  -971,   313,   565,   230,   915,   107,
     326,   232,   498,   498,   498,  -971,   498,   498,   498,   498,
     854,  1405,   216,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,   167,  -971,  -971,  1116,   217,   311,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  3635,   498,   498,   498,  -971,  -971,  -971,    41,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,   498,   498,  -971,   107,   107,  -971,   498,
     498,   498
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     751,     0,     2,     3,     0,     4,     1,   649,   650,     0,
       0,   652,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   763,   767,
     621,   753,   764,   758,   754,   755,   757,   765,   760,   761,
     756,   759,   766,   762,     0,     0,   651,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   573,
       0,     0,     0,     0,     0,   752,     0,   621,     0,   621,
     653,   667,     5,   666,   668,   669,   670,   671,   672,   673,
     674,   675,   676,   677,   678,   679,   680,   681,   682,   683,
     684,   685,   686,   687,   688,   689,   690,   691,   692,   693,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   731,   732,   733,
     734,   735,   736,   737,   738,   739,   740,   741,   742,   743,
     744,   745,   746,   651,   610,     0,    10,   747,   651,     0,
      17,   748,   615,     0,   138,   651,   651,     0,    47,   659,
     662,   660,   664,   665,   663,   658,     0,    58,   656,   661,
       0,   243,     0,    60,     0,   239,     0,   237,   612,   565,
       0,   561,   563,   611,     0,    25,     0,    56,   577,     0,
     213,     0,     0,   607,     0,    49,   651,     0,   535,     0,
      62,    74,    76,     0,    45,     0,   241,   623,   622,   651,
       0,   768,     0,   769,     7,   621,   609,   621,   620,   618,
     139,   621,   621,   613,   614,   621,    59,   651,   244,    61,
     240,   238,   562,   651,    26,    57,   214,     0,   580,   574,
       0,   576,   584,    50,   608,   536,    63,   651,    46,   242,
       0,   627,   626,     0,   150,     0,   140,     6,    13,    20,
     616,     0,     0,   657,   564,   589,   579,   583,     0,    75,
     651,   636,   624,   625,   628,   621,   151,   149,   621,   651,
      27,    12,    27,    19,   619,   621,     0,   578,   581,   585,
     582,   587,   638,   156,   142,     0,    51,     0,    14,    15,
      16,    51,    21,   621,   617,    48,   654,   586,     0,     0,
       0,     0,     0,   153,   154,   621,   155,   221,     0,     8,
       0,     0,   621,   621,    66,     0,    66,    22,   655,     0,
     588,   637,   640,   651,   629,   644,     0,     0,   152,     0,
       0,     0,     0,     0,   159,   621,     0,   162,   621,   621,
     621,   158,   157,   194,   220,     0,   141,   147,   148,   146,
     621,   144,   145,   651,     0,     0,    28,    29,    52,    53,
      54,    55,    78,    64,     0,    23,    78,   654,   654,   639,
       0,   631,     0,   224,     0,   197,     0,     0,     0,   180,
       0,     0,   164,   165,   160,   161,   163,   193,   222,     0,
     143,     9,     0,    31,     0,    37,     0,    77,     0,   621,
      24,     0,     0,     0,   651,   642,   641,   651,   630,   651,
     621,   225,   621,   621,   198,   196,   600,   170,     0,   167,
     651,     0,   172,     0,   182,     0,   566,   651,   651,   219,
       0,   215,   621,   651,   651,   651,   114,     0,   109,   621,
     621,    39,    38,   651,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     621,    80,    81,    82,    83,    84,    85,   258,   259,   260,
     261,   262,   263,   264,   265,    86,    87,    88,    89,     0,
      67,   651,   590,   654,   645,   634,   632,   227,   223,   200,
     168,   169,   621,   173,   171,   621,   183,   181,   621,   211,
     210,   218,   217,   216,   195,   111,   113,   112,   110,    32,
      41,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    79,     0,
      65,    18,     0,     0,   633,     0,     0,   175,   185,   175,
       0,     0,     0,   368,     0,   417,   418,     0,   569,   651,
       0,   334,     0,   266,     0,    90,   474,     0,   467,   651,
       0,   115,     0,   245,     0,   128,     0,   285,     0,   298,
       0,   314,     0,   454,     0,   432,     0,   383,     0,   370,
     621,    69,    68,   654,   591,   654,   643,   629,   651,     0,
       0,   226,   231,   232,   230,   621,   229,     0,   199,   204,
     205,   203,   621,   202,     0,     0,     0,    30,    36,    33,
      34,    35,    40,    44,    42,    43,   621,   372,   369,   567,
     621,   568,   416,   621,   336,   335,   621,   268,   267,   621,
      92,    91,   621,   473,   621,   117,   116,   621,   247,   246,
     621,   130,   129,   621,   621,   621,   621,   456,   455,   621,
     434,   433,   621,   385,   384,   371,    71,     0,     0,     0,
       0,     0,     0,   228,     0,   201,   174,   178,   179,   176,
     177,     0,   184,   189,   190,   186,   187,   188,   212,   374,
     420,   570,   338,   270,    94,   469,   119,   249,   132,   286,
     300,   316,   458,   436,   387,     0,   654,   654,   651,   651,
     646,   651,     0,   124,   601,   236,     0,   233,   602,   651,
     605,   606,   603,   209,     0,   206,   651,   604,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    70,    72,    73,     0,     0,   645,
     647,   635,   621,   126,   125,   234,   235,   207,   208,     0,
     191,     0,     0,     0,     0,   373,   381,   382,   380,   621,
     621,   378,   379,   621,     0,     0,   419,   424,   425,   423,
     621,   621,   621,   621,   621,   621,   337,   345,   346,   344,
     621,   341,   350,   351,   352,   353,   356,   621,   348,   349,
     354,   355,   621,   342,   343,   269,   277,   278,   276,   621,
     621,   621,   621,   621,   621,   621,   275,   274,   621,     0,
      93,    97,    98,   621,    96,     0,   468,   470,   471,   118,
     122,   123,   121,   621,   248,   251,   252,   250,   621,   621,
     621,   621,   621,   131,   136,   137,   135,   621,   133,   284,
     296,   297,   295,   621,   621,   290,   292,   621,   293,   294,
     621,     0,     0,     0,   299,   312,   313,   311,   621,   621,
     305,   304,   621,   307,   308,   309,   310,   621,     0,   315,
     327,   328,   326,   621,   621,   621,   621,   621,   621,   621,
     322,   323,   324,   325,   621,   321,   320,   457,   462,   463,
     461,   621,   621,   621,   621,   621,   651,   651,   435,   439,
     440,   438,   621,   621,   621,     0,   621,     0,   621,     0,
       0,   386,   391,   392,   390,   621,   621,   621,   621,   599,
     654,   592,   595,     0,     0,   192,     0,     0,     0,     0,
     376,   375,   377,     0,     0,   422,   426,   429,   427,   428,
     421,   340,   347,   339,   272,   282,   279,   283,   280,   281,
     271,   273,     0,    95,     0,   120,   254,   253,   255,   256,
     257,   134,   288,   289,   287,   291,     0,     0,     0,   302,
     303,   301,   306,     0,   318,   329,   330,   333,   331,   332,
     317,   319,   460,   464,   465,   466,   459,   445,   452,   437,
     441,   442,   621,   443,   621,   444,     0,     0,   389,   393,
     394,   388,     0,     0,   651,   127,   651,   651,   529,     0,
     525,   651,   651,   534,     0,   530,     0,   551,     0,   518,
       0,   357,     0,   430,     0,    99,   484,   497,   477,   506,
     621,   651,   475,   476,   651,   481,   651,   483,   651,   482,
     651,   545,     0,   541,   651,   540,     0,   537,   651,   651,
     651,   550,     0,   546,   558,     0,   555,   559,   571,   447,
     447,   413,   414,     0,   651,   396,   397,     0,   651,   654,
     596,   597,   648,   528,   527,   526,   533,   532,   531,   621,
     553,   552,   621,   520,   519,   621,   359,   358,   431,   621,
     101,   100,   472,     0,     0,     0,     0,   543,   542,   544,
     538,   539,   549,   548,   547,   556,   557,   569,     0,     0,
     621,   412,   621,   399,   398,   395,   594,   654,   175,   522,
     361,   104,   621,   479,   478,   621,   486,   485,   621,   499,
     498,   621,   508,   507,     0,   572,   446,   621,   621,   621,
     621,   453,   420,   401,   593,     0,     0,     0,     0,     0,
       0,     0,   488,   501,   510,   560,   449,   450,   451,   448,
       0,     0,   654,   554,   521,   523,   524,   360,   365,   366,
     364,   621,   621,   621,     0,   102,   480,     0,     0,     0,
     415,   400,   410,   411,   621,   405,   406,   407,   404,   408,
     409,   621,     0,   363,   367,   362,   651,   651,   108,     0,
     103,   487,   489,   492,   493,   494,   495,   496,   621,   491,
     500,   502,   505,   621,   504,   509,   621,   512,   513,   514,
     515,   516,   517,   403,   402,   598,   107,   106,   105,   490,
     503,   511
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -971,  -971,  -971,  1508,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,   157,  -971,   208,   164,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -117,  -971,   211,
    -971,  -235,   148,  -971,   490,  -971,   511,  -971,    56,  -971,
      95,   190,  -971,  -971,  -971,  -971,  -971,    30,  -971,   160,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -263,  -971,  -971,  -971,  -971,  -971,  -971,
    -463,  -971,  -971,  -971,  -971,  -971,  -971,  -296,  -971,   -14,
    -971,  -324,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -530,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,   142,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,   242,  -971,  -971,  -971,  -971,  -971,
     205,  -971,  -971,  -971,  -971,  -971,  -971,  -971,    -2,  -971,
       2,  -971,  -327,  -971,  -300,  -971,  -374,  -971,  -971,  -376,
    -971,  -168,  -971,  -971,  -162,  -971,  -971,  -971,  -151,  -971,
    -971,  -144,  -971,  -971,  -131,  -971,  -971,  -971,  -971,  -971,
    -127,  -971,  -971,  -971,  -113,  -971,  -112,    36,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -523,  -971,  -498,  -971,  -971,  -398,
    -971,  -971,  -971,  -433,  -971,  -971,  -971,  -359,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,  -971,
    -971,  -971,  -643,  -971,  -971,  -971,  -662,  -971,  -971,  -689,
    -971,  -971,  -704,  -971,  -661,  -971,  -971,  -647,  -971,  -971,
     -96,  -971,  -971,  -700,  -971,  -971,  -596,  -971,  -971,  -971,
     -87,  -971,  -971,  -971,  -615,   134,  -460,  -970,  -971,  -971,
    -971,  -971,   398,   361,  -971,  -971,   434,  -971,  -971,  -971,
     401,  -971,  -971,  -971,  -310,  -971,  -971,  -971,  -394,  -266,
    -971,  -971,  -971,    19,   -21,  -190,    -3,  -971,  -971,  -971,
     819,   406,  -971,  -971,  -971,   122,  -971,  -971,  -971,  -971,
    -971,  -971,  -971,   -29,  -971,    -4,   287,  1521,  -363,    20,
    -971,   353,   642,   -15,   -13,  -971,  -971,  -971
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   153,   154,   267,   373,   155,     2,   290,   291,
     159,     3,   292,   293,   384,   308,   194,    28,   306,   332,
     412,   560,   414,   333,   462,   561,   213,    29,   167,    30,
     204,    31,   334,   196,   378,   176,   379,   182,   687,   209,
     688,   382,   549,   383,   419,   602,   715,   214,   257,   416,
     417,   480,   574,   481,   651,   743,  1044,   833,  1111,  1219,
    1170,  1220,   457,   789,   458,   580,   482,   656,   745,   722,
     790,   764,   584,   483,   662,   747,   163,    36,   265,  1157,
     328,    67,   263,   287,   322,   323,   355,   356,   438,   357,
     439,   358,   441,   514,   624,   398,   359,   443,   517,   625,
     769,   695,   324,   407,   325,   394,   435,   556,   734,   623,
     735,   360,   520,   199,    37,   450,   362,   451,   326,   364,
     327,   392,   432,   555,   726,   616,   727,   186,   689,   184,
     690,   215,    40,   180,    41,   582,  1158,   659,   746,   791,
     572,   487,   648,   742,   488,   586,   748,   588,   489,   749,
     590,   490,   750,   570,   491,   645,   741,   807,   808,  1040,
     792,  1107,  1168,   562,   492,   598,   493,   638,   739,   596,
     494,   674,   753,  1086,  1087,   936,  1134,  1181,  1082,  1083,
     937,   566,   567,   495,   740,  1042,   793,   594,   496,   671,
     752,   925,   926,  1128,   927,   928,   592,   794,   668,   751,
     577,   498,   744,   578,  1050,  1051,  1052,  1144,  1053,  1054,
    1055,  1147,  1197,  1056,  1057,  1150,  1198,  1058,  1059,  1153,
    1199,  1038,   795,  1104,  1167,  1029,   781,  1030,  1034,   782,
    1035,   207,    42,  1066,   884,  1067,  1062,   885,  1063,  1072,
     886,  1073,  1036,  1160,  1101,  1075,   905,  1076,  1126,   190,
      43,   191,   243,   445,   568,   569,   641,  1077,  1127,   200,
     201,   202,   248,   249,   276,   251,   252,   278,   300,   317,
     297,   387,   604,   941,   942,  1023,  1137,   605,   728,   729,
     736,   737,   205,   156,  1078,   164,   230,   231,   295,   232,
      65,   217,   260,   261,   283,   390,   428,   554,   282,   319,
     343,   426,   344,   553,   720,   218,     4,   391,   339,   177,
     237,   178,   157,   158,   219,    44,    45,     5
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      11,   371,   440,    38,   370,    46,    46,    39,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,   422,   423,   701,   354,   372,   626,
     264,    69,   403,    72,   165,   192,   166,   296,   827,   783,
      72,   486,   828,   485,   211,   206,  1084,  1088,   870,   887,
     904,   915,   814,     7,    16,     8,   310,    72,   497,   869,
       6,    34,    46,  -575,    46,   369,    11,   160,    66,   168,
      13,    68,   181,   183,   185,   187,    72,   195,   197,   813,
     826,   208,   210,    72,   216,   436,   868,   883,   900,   901,
      25,   225,   615,   622,    26,   464,   780,   227,   812,   825,
      35,   250,     7,   902,     8,   867,   882,   899,   161,    16,
     938,     7,     7,     8,     8,   835,   784,   466,   771,    16,
     467,    15,    16,    72,     7,   464,     8,   632,   247,   307,
     161,   564,   235,   609,     7,     7,     8,     8,   473,   474,
     552,   475,   772,   277,   228,    25,   229,   466,   771,   288,
     467,  1142,    16,  1143,     7,    25,     8,   280,    25,   281,
     161,   471,  1060,   609,   365,   289,    72,  1069,   473,   474,
    1070,   475,   774,   236,    72,    72,   773,   238,   476,   239,
    1169,   240,   836,   241,  1175,    72,    24,   242,    25,   479,
      72,   244,   754,   245,   365,   796,   246,    68,   315,   436,
     478,   253,   774,   447,   255,   165,   256,   166,   448,  1145,
     258,  1146,   259,   285,  -651,   286,  -651,   785,   316,   479,
      11,   338,    11,   294,   348,   815,   275,   829,   320,    11,
      11,   418,  1148,  1151,  1149,  1152,   459,     7,    16,     8,
     677,   266,   678,   161,   823,   463,  1026,    15,   851,   501,
      11,  1027,   897,  1025,  1031,  1216,   321,   301,    11,  1032,
    1217,    16,   453,  -651,   454,    15,    16,   455,    14,    11,
      11,    16,   773,   502,    25,    11,   779,   503,   800,   819,
     365,   284,   843,  -651,   857,   863,   878,   893,   911,   922,
     935,   603,   614,   621,   888,    27,    72,    25,    11,    21,
     608,    11,    25,    46,   730,   731,   732,    25,   365,   564,
      66,   830,   716,    27,    72,   640,    46,    46,   388,  -589,
    -589,  1230,   652,   436,   724,   629,    46,    46,   663,     7,
       7,     8,     8,    70,  1184,   161,  1196,   664,   771,   366,
      46,    15,   424,   717,   425,    46,    46,    46,    46,    46,
     929,   330,   331,   757,   758,   385,   597,   430,   665,   431,
      16,    46,   772,   871,   872,   393,   822,   395,   821,    16,
     850,   609,   849,    16,   896,   914,   895,   913,   924,     9,
     609,   420,   939,   824,   367,    11,    66,   852,    23,    27,
     380,   898,   380,   413,   415,   940,    25,   930,  1012,    10,
      20,   610,   365,   484,  1014,    25,    23,    11,    23,    25,
     774,   365,  1130,    26,    46,  1235,  1089,   442,   444,  1182,
     446,   865,   880,   368,   864,   879,   307,   247,   316,   381,
    1195,   381,  1046,   931,  1047,   510,   433,  1048,   434,  1049,
     224,   801,   611,   628,   633,   226,   627,   523,   866,   881,
     312,   858,   233,   234,   528,    16,   311,    17,    18,   336,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,   778,  1208,   799,   818,
     834,  1211,   842,   847,   856,   862,   877,   892,   910,   921,
     934,    25,  1207,   254,   460,    32,   461,  1228,  1233,   309,
      11,    11,    11,     7,   313,     8,   262,    11,  1225,   161,
    1240,   512,    16,   513,    17,    18,    33,    11,    11,  1206,
    1209,    11,    11,    11,   273,  1193,   386,    11,   686,   550,
     274,   165,   607,   166,  1210,  1224,  1226,  1239,  1241,     7,
     701,     8,   329,   639,   279,    16,   421,    11,    25,   452,
    1227,   563,  1242,   571,   573,   575,   609,   581,   583,   585,
     587,   589,   591,   593,   595,   361,   599,   302,    16,   408,
      17,    18,   515,   802,   516,    12,   305,  1022,   691,   803,
       7,    25,     8,   721,    16,   692,   518,   365,   519,    16,
     804,   600,  1064,   601,   617,   609,    16,   805,    17,    18,
     609,  1229,  1234,    46,    25,    46,    46,   636,  1166,   637,
     806,   612,   619,    46,   809,   464,   630,   634,   618,   643,
      25,   644,   646,   696,   647,    25,   365,   697,   810,   811,
     389,   365,    25,   649,   675,   650,   784,   466,    11,  1180,
     467,   698,    16,   654,  1108,   655,     7,  1129,     8,    11,
     613,   620,   161,   609,   903,   631,   635,   853,   473,   474,
     411,   475,   839,   906,   657,   320,   658,  1155,   476,  1183,
     660,   579,   661,   321,   298,   666,    11,   667,    25,   340,
     669,   693,   670,   672,   365,   673,   718,    46,   719,   762,
     478,   763,   774,   179,   179,   179,   179,   179,   193,   179,
     179,   723,   318,   179,   179,  1191,   179,   785,  1099,   479,
    1100,   504,   299,  1090,   505,   786,   506,  1102,  1204,  1103,
     694,  1068,  1105,   765,  1106,   342,  1136,   511,   820,   679,
     943,   767,   848,     0,   521,   522,   894,   912,   923,     0,
     525,   526,   527,  1109,   429,  1110,  1132,     0,  1133,     0,
     531,     0,  1159,  1159,     0,    11,    11,    11,   770,     0,
       0,     0,    11,     0,    11,     0,   945,    46,    46,    46,
      46,   755,     0,     0,  1165,     0,     0,     0,     0,     0,
      46,    46,     0,     0,     0,     0,     0,     0,   551,     0,
       0,     0,  1192,     0,     0,   776,   787,   797,   816,   831,
     837,   840,   845,   854,   860,   875,   890,   908,   919,   932,
     756,     0,     0,     0,     0,     0,     0,     0,     0,  1212,
       0,     0,     0,     0,     0,    46,     0,     0,     0,     0,
       0,    46,     0,     0,   777,   788,   798,   817,   832,   838,
     841,   846,   855,   861,   876,   891,   909,   920,   933,     0,
       0,     0,     0,     0,     0,     0,   642,     0,   464,     0,
       0,     0,     0,     0,     0,     0,   653,    46,    46,    46,
    1222,  1231,  1237,     0,     0,  1236,     0,     0,     0,   784,
     466,  1205,     0,   467,    46,    16,   221,     0,   223,   193,
      12,     0,     0,     0,     0,   680,   609,  1223,  1232,  1238,
       0,   473,   474,   179,   475,  1190,     0,   349,     0,     0,
       0,   476,     0,     0,     0,     0,     0,   350,     0,   464,
     193,    25,   607,     0,     0,    46,    46,   365,     0,    22,
     351,     0,     0,   478,   352,   774,     0,   353,     0,     0,
       0,   466,     0,  -166,   467,     0,    16,     0,     0,   464,
     785,     0,   479,     0,     0,     0,     0,   609,  1200,     0,
       0,   345,   473,   474,     0,   475,     0,     0,  1037,  1039,
       0,   466,  1041,  1043,   467,     0,    16,   179,     0,     0,
       0,   193,    25,     0,     0,   471,     0,   609,   365,   179,
       0,  1045,   473,   474,   478,   475,   774,     0,     0,     0,
     773,     0,  1091,    11,    11,   759,   760,     0,   761,     0,
       0,     0,    25,   479,     0,     0,   766,     0,   365,  1187,
       0,    68,     0,   768,   478,     0,  1095,     0,     0,     0,
     464,  1098,     0,   179,     0,     0,     0,     0,     0,   179,
     179,     0,   179,   479,   268,     0,   269,     0,     0,   907,
     270,   271,   466,     0,   272,   467,     0,     0,     0,  1118,
       0,     0,     0,  1120,     0,     0,   471,     0,     0,  1124,
       0,     0,  1125,   473,   474,     0,   475,     0,     0,     0,
       0,   773,     0,     0,     0,     0,     0,     0,    11,    11,
      11,     0,    11,    11,     0,     0,     0,     0,     0,     0,
       0,     0,    68,     0,   303,   478,     0,   304,     0,    11,
      11,    11,    11,    11,   314,    11,  1164,    11,    11,    11,
       0,     0,    46,     0,   479,     0,     0,    11,     0,     0,
    1156,    11,   337,     0,     0,     0,     0,     0,     0,   464,
       0,     0,     0,   771,   363,     0,    15,     0,     0,     0,
       0,   376,   377,     0,     0,     0,     0,     0,     0,     0,
       0,   466,     0,     0,   467,    46,    16,   772,   871,   872,
       0,   773,     0,     0,   402,   471,     0,   404,   405,   406,
       0,     0,   473,   474,     0,   475,     0,     0,     0,   410,
       0,  1245,   476,   888,    27,   193,     0,     0,     0,     0,
       0,     0,    25,  1007,  1008,     0,   193,     0,   365,     0,
     464,    68,     0,     0,   478,     0,  1248,     0,     0,     0,
    1221,     0,     0,  1185,  1188,     0,     0,     0,     0,     0,
       0,   785,   466,   479,     0,   467,   787,  1202,   500,   844,
       0,     0,    11,    11,     0,     0,   471,     0,     0,   507,
       0,   508,   509,   473,   474,   771,   475,     0,    15,    16,
       0,   773,  1186,  1189,     0,     0,     0,     0,     0,     0,
     609,   524,     0,     0,     0,   788,  1203,     0,   529,   530,
     871,   872,    68,   773,     0,   478,   873,     0,     0,     0,
       0,     0,     0,     0,     0,    25,     0,     0,     0,   548,
       0,   365,     0,    66,   479,     0,    27,     0,     0,   774,
    1161,  1092,   771,  1093,  1094,    15,    16,     0,  1096,  1097,
       0,     0,     0,   179,     0,     0,     0,   609,     0,     0,
       0,   557,   874,     0,   558,     0,   772,   559,  1113,     0,
     773,  1114,     0,  1115,     0,  1116,     0,  1117,     0,     0,
       0,  1119,    25,     0,   464,  1121,  1122,  1123,   365,     0,
      66,     0,     0,    27,     0,     0,   774,     0,     0,     0,
       0,  1131,     0,     0,     0,  1135,   466,   771,     0,   467,
     179,    16,     0,     0,     0,     0,     0,     0,     0,   859,
     471,     0,   609,     0,     0,     0,    19,   473,   474,     0,
     475,     0,   871,   872,     0,   773,     0,   476,   873,     0,
       0,     0,     0,     0,     0,     0,     0,    25,     0,   676,
       0,     0,     0,   365,     0,     0,    68,   888,     0,   478,
       0,   774,   771,     0,   683,    15,    16,     0,     0,     0,
       0,   685,     0,     0,     0,     0,   785,   609,   479,     0,
       0,     0,     0,     0,   889,   699,   772,   871,   872,   700,
     773,     0,   702,     0,     0,   703,     0,     0,   704,     0,
      24,   705,    25,   706,   771,     0,   707,     0,    16,   708,
       0,    16,   709,   710,   711,   712,     0,     0,   713,   609,
     471,   714,   609,     0,     0,   916,     0,     0,   772,     0,
       0,     0,   773,  1246,  1247,     0,     0,     0,     0,  1201,
     917,     0,     0,     0,    25,     0,     0,    25,     0,     0,
     365,     0,     0,   365,     0,     0,    68,     0,   774,     0,
      47,    48,     0,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,   775,     0,     0,   918,     0,     0,   162,     0,   175,
     175,   175,   175,   175,   189,   175,   175,   198,   203,   175,
     175,   212,   175,     0,     0,     0,     0,     0,     0,     0,
       0,   944,     0,     0,     0,   193,     0,   220,     0,   222,
     179,   179,     0,     0,     0,     0,     0,     0,   950,   951,
       0,     0,   952,     0,     0,     0,     0,     0,     0,   955,
     956,   957,   958,   959,   960,     0,     0,     0,     0,   961,
       0,     0,     0,     0,     0,     0,   962,     0,     0,     0,
       0,   963,     0,     0,     0,   193,     0,     0,   964,   965,
     966,   967,   968,   969,   970,     0,     0,   971,     0,     0,
       0,     0,   973,     0,     0,     0,     0,     0,   193,   193,
       0,     0,   975,     0,     0,   193,     0,   976,   977,   978,
     979,   980,     0,     0,     0,     0,   981,     0,     0,     0,
       0,     0,   982,   983,     0,     0,   984,     0,     0,   985,
       0,     0,     0,     0,     0,     0,     0,   989,   990,     0,
       0,   991,     0,     0,     0,     0,   992,     0,     0,     0,
       0,     0,   994,   995,   996,   997,   998,   999,  1000,     0,
       0,     0,     0,  1001,     0,     0,     0,   464,   162,     0,
    1002,  1003,  1004,  1005,  1006,     0,     0,     0,     0,     0,
       0,  1009,  1010,  1011,   465,  1013,     0,  1015,     0,   466,
       0,     0,   467,     0,  1018,  1019,  1020,  1021,   468,   469,
       0,   470,     0,   471,   472,     0,    12,    13,     0,   175,
     473,   474,    14,   475,    15,    16,     0,    17,    18,   193,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,   477,   193,     0,     0,    68,
      20,     0,   478,    21,     0,    22,     0,     0,    23,    24,
       0,    25,     0,     0,     0,    26,     0,     0,     0,  -749,
    -750,   479,    27,     0,     0,     0,     0,     0,   335,     0,
       0,  1079,     0,  1080,     0,     0,     0,     0,     0,     0,
       0,   346,   347,   175,     0,     0,     0,     0,     0,     0,
       0,   374,   375,     0,   193,   175,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1112,
     396,   397,   399,   400,   401,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   409,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   437,   175,   175,     0,   175,   449,
       0,     0,     0,     0,     0,     0,     0,   456,  1138,     0,
       0,  1139,     0,     0,  1140,     0,     0,     0,  1141,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1162,
       0,  1163,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1171,     0,     0,  1172,     0,     0,  1173,     0,     0,
    1174,     0,     0,     0,     0,     0,  1176,  1177,  1178,  1179,
       0,     0,     0,     0,     0,   532,   533,   534,   535,   536,
     537,   538,   539,   540,   541,   542,   543,   544,   545,   546,
     547,     0,     0,     0,     0,     0,     0,   212,     0,     0,
    1213,  1214,  1215,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1243,     0,     0,     0,     0,     0,     0,
    1244,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   565,     0,     0,     0,   576,     0,  1249,     0,     0,
       0,     0,  1250,     0,   162,  1251,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     681,   682,     0,     0,     0,     0,     0,     0,   684,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   175,
     725,     0,   733,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   738,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   175,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   946,   947,   948,   949,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   953,   954,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     972,     0,     0,     0,     0,     0,   974,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   986,   987,   988,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   993,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1016,  1017,     0,     0,  1028,  1033,   175,   175,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1061,  1065,  1071,     0,     0,     0,
       0,  1074,     0,     0,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,     0,    73,   161,     0,     0,
       0,     0,     0,     0,  1081,  1085,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,     0,     0,     0,     0,     0,
     341,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    71,     0,     0,
    1194,     0,     0,    73,   188,     0,     0,     0,     0,     0,
       0,     0,  1218,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,    71,     0,     0,     0,     0,   606,    73,   188,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,     0,     0,    71,
       0,     0,  1024,    72,   169,    73,   170,   171,     0,     0,
       0,   172,   173,   174,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,    71,     0,     0,     0,   427,   169,
      73,   170,   171,     0,     0,     0,   172,   173,   174,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    71,
       0,     0,     0,    72,     0,    73,   161,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,    71,     0,     0,     0,    72,     0,
      73,   188,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    71,
       0,     0,     0,     0,     0,    73,   188,     0,     0,   250,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,    71,     0,     0,     0,     0,     0,
      73,   188,     0,     0,   940,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    71,
     338,     0,     0,     0,     0,    73,   188,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,    71,     0,     0,     0,    72,     0,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    71,
       0,     0,     0,     0,     0,    73,   188,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152
};

static const yytype_int16 yycheck[] =
{
       4,   328,   396,     5,   328,     9,    10,     5,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   387,   388,   641,   323,   328,   559,
     220,    45,   356,     8,    49,    56,    49,   272,   742,   739,
       8,   417,   742,   417,    12,    60,  1016,  1017,   748,   749,
     750,   751,   741,     5,    31,     7,   291,     8,   417,   748,
       0,     5,    66,    14,    68,   328,    70,    48,    75,    50,
      23,    76,    52,    53,    54,    55,     8,    57,    58,   741,
     742,    61,    62,     8,    64,    17,   748,   749,   750,   750,
      67,   103,   555,   556,    71,     4,   739,   103,   741,   742,
       5,    14,     5,   750,     7,   748,   749,   750,    11,    31,
     753,     5,     5,     7,     7,    37,    25,    26,    27,    31,
      29,    30,    31,     8,     5,     4,     7,   104,   106,    82,
      11,   106,   103,    42,     5,     5,     7,     7,    47,    48,
     503,    50,    51,   106,   103,    67,   105,    26,    27,   103,
      29,   103,    31,   105,     5,    67,     7,   103,    67,   105,
      11,    40,    94,    42,    73,   102,     8,    92,    47,    48,
      95,    50,    81,   176,     8,     8,    55,   180,    57,   182,
      83,   184,   104,   186,  1154,     8,    65,   190,    67,    98,
       8,   194,   104,   196,    73,   104,   199,    76,   104,    17,
      79,   204,    81,    88,   207,   220,   209,   220,    93,   103,
     213,   105,   215,   103,     5,   105,     7,    96,   107,    98,
     224,     5,   226,   104,   104,   104,   247,    20,    24,   233,
     234,    70,   103,   103,   105,   105,   103,     5,    31,     7,
     603,   222,   605,    11,   742,   104,    88,    30,   746,   104,
     254,    93,   750,   104,    88,    88,    32,   278,   262,    93,
      93,    31,    85,    54,    87,    30,    31,    90,    28,   273,
     274,    31,    55,   108,    67,   279,   739,   109,   741,   742,
      73,   261,   745,    74,   747,   748,   749,   750,   751,   752,
     753,    85,   555,   556,    77,    78,     8,    67,   302,    59,
     102,   305,    67,   307,    16,    17,    18,    67,    73,   106,
      75,   104,   110,    78,     8,   103,   320,   321,   339,   106,
     107,   104,   103,    17,    18,   560,   330,   331,   103,     5,
       5,     7,     7,    46,   104,    11,   104,   103,    27,   104,
     344,    30,   103,   106,   105,   349,   350,   351,   352,   353,
      21,    43,    44,   716,   717,   335,   546,   103,   103,   105,
      31,   365,    51,    52,    53,   346,   742,   347,   742,    31,
     746,    42,   746,    31,   750,   751,   750,   751,   752,    54,
      42,   384,   111,   742,   328,   389,    75,   746,    64,    78,
     334,   750,   336,   374,   375,    14,    67,    68,   103,    74,
      56,    63,    73,   417,   103,    67,    64,   411,    64,    67,
      81,    73,   103,    71,   418,   104,   106,   397,   398,   106,
     400,   748,   749,   328,   748,   749,    82,   106,   107,   334,
     104,   336,    84,   104,    86,   438,   103,    89,   105,    91,
     153,   741,   104,   560,   561,   158,   104,   450,   748,   749,
     293,   747,   165,   166,   457,    31,   292,    33,    34,   311,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   739,  1181,   741,   742,
     743,  1181,   745,   746,   747,   748,   749,   750,   751,   752,
     753,    67,  1181,   206,   103,     5,   105,  1197,  1198,   291,
     504,   505,   506,     5,   293,     7,   219,   511,  1197,    11,
    1199,   103,    31,   105,    33,    34,     5,   521,   522,  1181,
    1181,   525,   526,   527,   237,  1168,   336,   531,   104,   499,
     243,   546,   553,   546,  1181,  1197,  1197,  1199,  1199,     5,
    1155,     7,     8,   564,   257,    31,   386,   551,    67,   407,
    1197,   532,  1199,   534,   535,   536,    42,   538,   539,   540,
     541,   542,   543,   544,   545,   323,   547,   280,    31,   364,
      33,    34,   103,   741,   105,    22,   289,   940,    97,   741,
       5,    67,     7,     8,    31,   104,   103,    73,   105,    31,
     741,   103,   986,   105,    80,    42,    31,   741,    33,    34,
      42,  1197,  1198,   607,    67,   609,   610,   103,  1138,   105,
     741,   555,   556,   617,   741,     4,   560,   561,   104,   103,
      67,   105,   103,   625,   105,    67,    73,   625,   741,   741,
     343,    73,    67,   103,   598,   105,    25,    26,   642,  1162,
      29,   104,    31,   103,  1042,   105,     5,  1080,     7,   653,
     555,   556,    11,    42,   750,   560,   561,   104,    47,    48,
     373,    50,   104,   750,   103,    24,   105,  1127,    57,   104,
     103,   537,   105,    32,   276,   103,   680,   105,    67,   318,
     103,   625,   105,   103,    73,   105,   103,   691,   105,   103,
      79,   105,    81,    51,    52,    53,    54,    55,    56,    57,
      58,   681,   301,    61,    62,  1168,    64,    96,   103,    98,
     105,   424,   278,  1023,   427,   104,   429,   103,  1181,   105,
     625,   987,   103,   726,   105,   319,  1089,   440,   742,   607,
     759,   734,   746,    -1,   447,   448,   750,   751,   752,    -1,
     453,   454,   455,   103,   391,   105,   103,    -1,   105,    -1,
     463,    -1,  1128,  1129,    -1,   759,   760,   761,   738,    -1,
      -1,    -1,   766,    -1,   768,    -1,   769,   771,   772,   773,
     774,   715,    -1,    -1,  1137,    -1,    -1,    -1,    -1,    -1,
     784,   785,    -1,    -1,    -1,    -1,    -1,    -1,   501,    -1,
      -1,    -1,  1168,    -1,    -1,   739,   740,   741,   742,   743,
     744,   745,   746,   747,   748,   749,   750,   751,   752,   753,
     715,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1182,
      -1,    -1,    -1,    -1,    -1,   829,    -1,    -1,    -1,    -1,
      -1,   835,    -1,    -1,   739,   740,   741,   742,   743,   744,
     745,   746,   747,   748,   749,   750,   751,   752,   753,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   569,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   579,   871,   872,   873,
    1197,  1198,  1199,    -1,    -1,  1199,    -1,    -1,    -1,    25,
      26,  1181,    -1,    29,   888,    31,    67,    -1,    69,   247,
      22,    -1,    -1,    -1,    -1,   608,    42,  1197,  1198,  1199,
      -1,    47,    48,   261,    50,  1168,    -1,    39,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    49,    -1,     4,
     278,    67,   943,    -1,    -1,   929,   930,    73,    -1,    61,
      62,    -1,    -1,    79,    66,    81,    -1,    69,    -1,    -1,
      -1,    26,    -1,    75,    29,    -1,    31,    -1,    -1,     4,
      96,    -1,    98,    -1,    -1,    -1,    -1,    42,   104,    -1,
      -1,   319,    47,    48,    -1,    50,    -1,    -1,   948,   949,
      -1,    26,   953,   954,    29,    -1,    31,   335,    -1,    -1,
      -1,   339,    67,    -1,    -1,    40,    -1,    42,    73,   347,
      -1,   972,    47,    48,    79,    50,    81,    -1,    -1,    -1,
      55,    -1,  1023,  1007,  1008,   718,   719,    -1,   721,    -1,
      -1,    -1,    67,    98,    -1,    -1,   729,    -1,    73,   104,
      -1,    76,    -1,   736,    79,    -1,  1029,    -1,    -1,    -1,
       4,  1034,    -1,   391,    -1,    -1,    -1,    -1,    -1,   397,
     398,    -1,   400,    98,   225,    -1,   227,    -1,    -1,   104,
     231,   232,    26,    -1,   235,    29,    -1,    -1,    -1,  1062,
      -1,    -1,    -1,  1066,    -1,    -1,    40,    -1,    -1,  1072,
      -1,    -1,  1075,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,  1092,  1093,
    1094,    -1,  1096,  1097,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    -1,   285,    79,    -1,   288,    -1,  1113,
    1114,  1115,  1116,  1117,   295,  1119,  1137,  1121,  1122,  1123,
      -1,    -1,  1126,    -1,    98,    -1,    -1,  1131,    -1,    -1,
     104,  1135,   313,    -1,    -1,    -1,    -1,    -1,    -1,     4,
      -1,    -1,    -1,    27,   325,    -1,    30,    -1,    -1,    -1,
      -1,   332,   333,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    29,  1169,    31,    51,    52,    53,
      -1,    55,    -1,    -1,   355,    40,    -1,   358,   359,   360,
      -1,    -1,    47,    48,    -1,    50,    -1,    -1,    -1,   370,
      -1,  1212,    57,    77,    78,   553,    -1,    -1,    -1,    -1,
      -1,    -1,    67,   916,   917,    -1,   564,    -1,    73,    -1,
       4,    76,    -1,    -1,    79,    -1,  1219,    -1,    -1,    -1,
     104,    -1,    -1,  1167,  1168,    -1,    -1,    -1,    -1,    -1,
      -1,    96,    26,    98,    -1,    29,  1180,  1181,   419,   104,
      -1,    -1,  1246,  1247,    -1,    -1,    40,    -1,    -1,   430,
      -1,   432,   433,    47,    48,    27,    50,    -1,    30,    31,
      -1,    55,  1167,  1168,    -1,    -1,    -1,    -1,    -1,    -1,
      42,   452,    -1,    -1,    -1,  1180,  1181,    -1,   459,   460,
      52,    53,    76,    55,    -1,    79,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,   480,
      -1,    73,    -1,    75,    98,    -1,    78,    -1,    -1,    81,
     104,  1024,    27,  1026,  1027,    30,    31,    -1,  1031,  1032,
      -1,    -1,    -1,   681,    -1,    -1,    -1,    42,    -1,    -1,
      -1,   512,   104,    -1,   515,    -1,    51,   518,  1051,    -1,
      55,  1054,    -1,  1056,    -1,  1058,    -1,  1060,    -1,    -1,
      -1,  1064,    67,    -1,     4,  1068,  1069,  1070,    73,    -1,
      75,    -1,    -1,    78,    -1,    -1,    81,    -1,    -1,    -1,
      -1,  1084,    -1,    -1,    -1,  1088,    26,    27,    -1,    29,
     738,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      40,    -1,    42,    -1,    -1,    -1,    46,    47,    48,    -1,
      50,    -1,    52,    53,    -1,    55,    -1,    57,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,   600,
      -1,    -1,    -1,    73,    -1,    -1,    76,    77,    -1,    79,
      -1,    81,    27,    -1,   615,    30,    31,    -1,    -1,    -1,
      -1,   622,    -1,    -1,    -1,    -1,    96,    42,    98,    -1,
      -1,    -1,    -1,    -1,   104,   636,    51,    52,    53,   640,
      55,    -1,   643,    -1,    -1,   646,    -1,    -1,   649,    -1,
      65,   652,    67,   654,    27,    -1,   657,    -1,    31,   660,
      -1,    31,   663,   664,   665,   666,    -1,    -1,   669,    42,
      40,   672,    42,    -1,    -1,    45,    -1,    -1,    51,    -1,
      -1,    -1,    55,  1216,  1217,    -1,    -1,    -1,    -1,   104,
      60,    -1,    -1,    -1,    67,    -1,    -1,    67,    -1,    -1,
      73,    -1,    -1,    73,    -1,    -1,    76,    -1,    81,    -1,
       9,    10,    -1,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,   104,    -1,    -1,   104,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   762,    -1,    -1,    -1,   943,    -1,    66,    -1,    68,
     948,   949,    -1,    -1,    -1,    -1,    -1,    -1,   779,   780,
      -1,    -1,   783,    -1,    -1,    -1,    -1,    -1,    -1,   790,
     791,   792,   793,   794,   795,    -1,    -1,    -1,    -1,   800,
      -1,    -1,    -1,    -1,    -1,    -1,   807,    -1,    -1,    -1,
      -1,   812,    -1,    -1,    -1,   993,    -1,    -1,   819,   820,
     821,   822,   823,   824,   825,    -1,    -1,   828,    -1,    -1,
      -1,    -1,   833,    -1,    -1,    -1,    -1,    -1,  1016,  1017,
      -1,    -1,   843,    -1,    -1,  1023,    -1,   848,   849,   850,
     851,   852,    -1,    -1,    -1,    -1,   857,    -1,    -1,    -1,
      -1,    -1,   863,   864,    -1,    -1,   867,    -1,    -1,   870,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   878,   879,    -1,
      -1,   882,    -1,    -1,    -1,    -1,   887,    -1,    -1,    -1,
      -1,    -1,   893,   894,   895,   896,   897,   898,   899,    -1,
      -1,    -1,    -1,   904,    -1,    -1,    -1,     4,   220,    -1,
     911,   912,   913,   914,   915,    -1,    -1,    -1,    -1,    -1,
      -1,   922,   923,   924,    21,   926,    -1,   928,    -1,    26,
      -1,    -1,    29,    -1,   935,   936,   937,   938,    35,    36,
      -1,    38,    -1,    40,    41,    -1,    22,    23,    -1,   261,
      47,    48,    28,    50,    30,    31,    -1,    33,    34,  1137,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    -1,    -1,    72,  1154,    -1,    -1,    76,
      56,    -1,    79,    59,    -1,    61,    -1,    -1,    64,    65,
      -1,    67,    -1,    -1,    -1,    71,    -1,    -1,    -1,    75,
      76,    98,    78,    -1,    -1,    -1,    -1,    -1,   307,    -1,
      -1,  1012,    -1,  1014,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   320,   321,   335,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   330,   331,    -1,  1212,   347,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1050,
     349,   350,   351,   352,   353,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   365,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   396,   397,   398,    -1,   400,   401,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   409,  1099,    -1,
      -1,  1102,    -1,    -1,  1105,    -1,    -1,    -1,  1109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   418,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1130,
      -1,  1132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1142,    -1,    -1,  1145,    -1,    -1,  1148,    -1,    -1,
    1151,    -1,    -1,    -1,    -1,    -1,  1157,  1158,  1159,  1160,
      -1,    -1,    -1,    -1,    -1,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,    -1,    -1,    -1,    -1,    -1,    -1,   499,    -1,    -1,
    1191,  1192,  1193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1204,    -1,    -1,    -1,    -1,    -1,    -1,
    1211,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   533,    -1,    -1,    -1,   537,    -1,  1228,    -1,    -1,
      -1,    -1,  1233,    -1,   546,  1236,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     609,   610,    -1,    -1,    -1,    -1,    -1,    -1,   617,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   681,
     682,    -1,   684,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   691,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   738,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   771,   772,   773,   774,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   784,   785,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     829,    -1,    -1,    -1,    -1,    -1,   835,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   871,   872,   873,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   888,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     929,   930,    -1,    -1,   946,   947,   948,   949,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   986,   987,   988,    -1,    -1,    -1,
      -1,   993,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,  1016,  1017,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1126,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
    1169,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1194,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,    -1,   104,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    -1,    -1,     4,
      -1,    -1,   104,     8,     9,    10,    11,    12,    -1,    -1,
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
       0,   113,   119,   123,   418,   429,     0,     5,     7,    54,
      74,   417,    22,    23,    28,    30,    31,    33,    34,    46,
      56,    59,    61,    64,    65,    67,    71,    78,   129,   139,
     141,   143,   146,   148,   150,   152,   189,   226,   240,   242,
     244,   246,   344,   362,   427,   428,   417,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   402,    75,   193,    76,   191,
     418,     4,     8,    10,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,   114,   115,   118,   395,   424,   425,   122,
     395,    11,   115,   188,   397,   425,   426,   140,   395,     9,
      11,    12,    16,    17,    18,   115,   147,   421,   423,   424,
     245,   421,   149,   421,   241,   421,   239,   421,    11,   115,
     361,   363,   396,   424,   128,   421,   145,   421,   115,   225,
     371,   372,   373,   115,   142,   394,   425,   343,   421,   151,
     421,    12,   115,   138,   159,   243,   421,   403,   417,   426,
     419,   402,   419,   402,   418,   103,   418,   103,   103,   105,
     398,   399,   401,   418,   418,   103,   398,   422,   398,   398,
     398,   398,   398,   364,   398,   398,   398,   106,   374,   375,
      14,   377,   378,   398,   418,   398,   398,   160,   398,   398,
     404,   405,   418,   194,   397,   190,   395,   116,   402,   402,
     402,   402,   402,   418,   418,   396,   376,   106,   379,   418,
     103,   105,   410,   406,   421,   103,   105,   195,   103,   102,
     120,   121,   124,   125,   104,   400,   143,   382,   374,   378,
     380,   396,   418,   402,   402,   418,   130,    82,   127,   129,
     143,   130,   127,   141,   402,   104,   107,   381,   382,   411,
      24,    32,   196,   197,   214,   216,   230,   232,   192,     8,
      43,    44,   131,   135,   144,   419,   144,   402,     5,   420,
     375,   104,   403,   412,   414,   424,   419,   419,   104,    39,
      49,    62,    66,    69,   189,   198,   199,   201,   203,   208,
     223,   226,   228,   402,   231,    73,   104,   150,   152,   175,
     193,   244,   246,   117,   419,   419,   402,   402,   146,   148,
     150,   152,   153,   155,   126,   421,   153,   383,   396,   418,
     407,   419,   233,   395,   217,   421,   419,   419,   207,   419,
     419,   419,   402,   193,   402,   402,   402,   215,   232,   419,
     402,   418,   132,   395,   134,   395,   161,   162,    70,   156,
     398,   161,   420,   420,   103,   105,   413,     8,   408,   423,
     103,   105,   234,   103,   105,   218,    17,   115,   200,   202,
     390,   204,   421,   209,   421,   365,   421,    88,    93,   115,
     227,   229,   216,    85,    87,    90,   115,   174,   176,   103,
     103,   105,   136,   104,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    79,    98,
     163,   165,   178,   185,   191,   248,   251,   253,   256,   260,
     263,   266,   276,   278,   282,   295,   300,   309,   313,   419,
     402,   104,   108,   109,   418,   418,   418,   402,   402,   402,
     398,   418,   103,   105,   205,   103,   105,   210,   103,   105,
     224,   418,   418,   398,   402,   418,   418,   418,   398,   402,
     402,   418,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   402,   154,
     159,   418,   420,   415,   409,   235,   219,   402,   402,   402,
     133,   137,   275,   395,   106,   115,   293,   294,   366,   367,
     265,   395,   252,   395,   164,   395,   115,   312,   315,   367,
     177,   395,   247,   395,   184,   395,   257,   395,   259,   395,
     262,   395,   308,   395,   299,   395,   281,   397,   277,   395,
     103,   105,   157,    85,   384,   389,   104,   396,   102,    42,
      63,   104,   150,   152,   175,   182,   237,    80,   104,   150,
     152,   175,   182,   221,   206,   211,   206,   104,   139,   143,
     150,   152,   104,   139,   150,   152,   103,   105,   279,   396,
     103,   368,   418,   103,   105,   267,   103,   105,   254,   103,
     105,   166,   103,   418,   103,   105,   179,   103,   105,   249,
     103,   105,   186,   103,   103,   103,   103,   105,   310,   103,
     105,   301,   103,   105,   283,   279,   402,   420,   420,   407,
     418,   419,   419,   402,   419,   402,   104,   150,   152,   240,
     242,    97,   104,   150,   152,   213,   240,   242,   104,   402,
     402,   366,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   158,   110,   106,   103,   105,
     416,     8,   181,   421,    18,   115,   236,   238,   390,   391,
      16,    17,    18,   115,   220,   222,   392,   393,   419,   280,
     296,   268,   255,   167,   314,   180,   250,   187,   258,   261,
     264,   311,   302,   284,   104,   150,   152,   420,   420,   418,
     418,   418,   103,   105,   183,   398,   418,   398,   418,   212,
     421,    27,    51,    55,    81,   104,   150,   152,   175,   182,
     334,   338,   341,   355,    25,    96,   104,   150,   152,   175,
     182,   251,   272,   298,   309,   334,   104,   150,   152,   175,
     182,   246,   253,   256,   260,   263,   266,   269,   270,   272,
     276,   278,   334,   338,   341,   104,   150,   152,   175,   182,
     191,   248,   251,   298,   309,   334,   338,   344,   355,    20,
     104,   150,   152,   169,   175,    37,   104,   150,   152,   104,
     150,   152,   175,   182,   104,   150,   152,   175,   191,   248,
     251,   298,   309,   104,   150,   152,   175,   182,   189,   104,
     150,   152,   175,   182,   193,   244,   246,   334,   338,   341,
     355,    52,    53,    58,   104,   150,   152,   175,   182,   193,
     244,   246,   334,   338,   346,   349,   352,   355,    77,   104,
     150,   152,   175,   182,   191,   248,   251,   298,   309,   334,
     338,   346,   349,   352,   355,   358,   362,   104,   150,   152,
     175,   182,   191,   248,   251,   355,    45,    60,   104,   150,
     152,   175,   182,   191,   248,   303,   304,   306,   307,    21,
      68,   104,   150,   152,   175,   182,   287,   292,   334,   111,
      14,   385,   386,   415,   402,   398,   419,   419,   419,   419,
     402,   402,   402,   419,   419,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   419,   402,   419,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   419,   419,   419,   402,
     402,   402,   402,   419,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   418,   418,   402,
     402,   402,   103,   402,   103,   402,   419,   419,   402,   402,
     402,   402,   420,   387,   104,   104,    88,    93,   115,   337,
     339,    88,    93,   115,   340,   342,   354,   421,   333,   421,
     271,   395,   297,   395,   168,   395,    84,    86,    89,    91,
     316,   317,   318,   320,   321,   322,   325,   326,   329,   330,
      94,   115,   348,   350,   390,   115,   345,   347,   391,    92,
      95,   115,   351,   353,   115,   357,   359,   369,   396,   402,
     402,   115,   290,   291,   369,   115,   285,   286,   369,   106,
     386,   396,   418,   418,   418,   398,   418,   418,   398,   103,
     105,   356,   103,   105,   335,   103,   105,   273,   301,   103,
     105,   170,   402,   418,   418,   418,   418,   418,   398,   418,
     398,   418,   418,   418,   398,   398,   360,   370,   305,   305,
     103,   418,   103,   105,   288,   418,   420,   388,   402,   402,
     402,   402,   103,   105,   319,   103,   105,   323,   103,   105,
     327,   103,   105,   331,   419,   368,   104,   191,   248,   251,
     355,   104,   402,   402,   396,   420,   206,   336,   274,    83,
     172,   402,   402,   402,   402,   369,   402,   402,   402,   402,
     296,   289,   106,   104,   104,   150,   152,   104,   150,   152,
     175,   182,   251,   334,   419,   104,   104,   324,   328,   332,
     104,   104,   150,   152,   182,   246,   338,   341,   344,   346,
     349,   355,   420,   402,   402,   402,    88,    93,   115,   171,
     173,   104,   244,   246,   338,   341,   346,   349,   355,   358,
     104,   244,   246,   355,   358,   104,   193,   244,   246,   338,
     341,   346,   349,   402,   402,   396,   418,   418,   398,   402,
     402,   402
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
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429
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
       2,     2,     2,     2,     2,     2,     2,     2,     4,     4
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
