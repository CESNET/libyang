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
    NODE_PRINT = 355
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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3132

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  317
/* YYNRULES -- Number of rules.  */
#define YYNRULES  743
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1201

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   355

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
     109,   110,     2,   101,     2,     2,     2,   105,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   104,
       2,   108,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   106,     2,   107,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   102,     2,   103,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   259,   259,   260,   262,   285,   288,   290,   289,   313,
     324,   334,   344,   345,   351,   356,   359,   370,   380,   393,
     394,   400,   402,   404,   406,   407,   408,   410,   415,   422,
     423,   424,   435,   446,   453,   460,   462,   463,   467,   468,
     479,   490,   497,   507,   521,   526,   532,   533,   538,   543,
     548,   554,   556,   558,   560,   562,   575,   579,   580,   582,
     583,   588,   589,   594,   601,   601,   608,   614,   653,   672,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     687,   703,   705,   706,   710,   711,   718,   725,   734,   744,
     744,   746,   747,   751,   752,   754,   755,   756,   765,   767,
     768,   769,   770,   785,   796,   798,   799,   814,   815,   824,
     830,   835,   841,   843,   856,   858,   859,   884,   885,   899,
     912,   918,   923,   929,   931,   980,   990,   993,   998,   999,
    1005,  1009,  1014,  1021,  1023,  1029,  1030,  1035,  1070,  1071,
    1074,  1075,  1079,  1085,  1098,  1099,  1100,  1101,  1102,  1104,
    1121,  1127,  1128,  1144,  1148,  1156,  1157,  1162,  1174,  1179,
    1184,  1189,  1195,  1202,  1215,  1216,  1220,  1221,  1231,  1236,
    1241,  1246,  1252,  1263,  1275,  1276,  1279,  1287,  1296,  1297,
    1312,  1313,  1325,  1332,  1336,  1341,  1347,  1357,  1358,  1373,
    1378,  1379,  1384,  1386,  1388,  1389,  1390,  1403,  1415,  1416,
    1418,  1426,  1436,  1437,  1452,  1453,  1465,  1472,  1477,  1482,
    1488,  1498,  1499,  1515,  1517,  1519,  1521,  1523,  1530,  1533,
    1534,  1539,  1542,  1548,  1553,  1558,  1561,  1564,  1567,  1570,
    1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,  1588,  1595,
    1598,  1599,  1622,  1625,  1625,  1629,  1634,  1634,  1638,  1643,
    1649,  1655,  1660,  1665,  1665,  1670,  1670,  1675,  1675,  1684,
    1684,  1688,  1688,  1694,  1731,  1739,  1743,  1743,  1747,  1752,
    1752,  1757,  1762,  1762,  1766,  1771,  1777,  1783,  1789,  1794,
    1800,  1807,  1859,  1863,  1863,  1867,  1873,  1873,  1878,  1889,
    1894,  1894,  1898,  1904,  1917,  1930,  1940,  1946,  1951,  1957,
    1964,  2008,  2012,  2012,  2016,  2022,  2022,  2026,  2035,  2041,
    2047,  2060,  2073,  2083,  2089,  2094,  2099,  2099,  2103,  2103,
    2108,  2108,  2113,  2113,  2122,  2122,  2129,  2136,  2139,  2140,
    2160,  2164,  2164,  2168,  2174,  2184,  2191,  2198,  2205,  2211,
    2217,  2217,  2223,  2224,  2227,  2228,  2229,  2230,  2231,  2232,
    2233,  2240,  2247,  2250,  2251,  2265,  2268,  2268,  2272,  2277,
    2283,  2288,  2293,  2293,  2300,  2308,  2311,  2319,  2322,  2323,
    2346,  2349,  2349,  2353,  2359,  2359,  2363,  2370,  2377,  2384,
    2389,  2395,  2402,  2405,  2406,  2438,  2441,  2441,  2445,  2450,
    2456,  2461,  2466,  2466,  2470,  2470,  2476,  2477,  2479,  2489,
    2491,  2492,  2526,  2529,  2543,  2567,  2589,  2640,  2657,  2674,
    2695,  2716,  2721,  2727,  2728,  2731,  2743,  2748,  2749,  2751,
    2759,  2764,  2767,  2767,  2771,  2776,  2782,  2787,  2792,  2792,
    2797,  2797,  2802,  2802,  2811,  2811,  2817,  2831,  2835,  2844,
    2848,  2849,  2873,  2877,  2883,  2889,  2894,  2899,  2899,  2903,
    2903,  2908,  2908,  2918,  2918,  2929,  2929,  2965,  2968,  2968,
    2972,  2972,  2976,  2976,  2981,  2981,  2987,  2987,  3023,  3031,
    3033,  3034,  3068,  3071,  3071,  3075,  3080,  3086,  3091,  3096,
    3096,  3100,  3100,  3105,  3105,  3111,  3120,  3138,  3141,  3147,
    3153,  3158,  3159,  3161,  3166,  3168,  3170,  3171,  3172,  3174,
    3174,  3180,  3181,  3213,  3216,  3222,  3226,  3232,  3238,  3245,
    3252,  3260,  3269,  3269,  3275,  3276,  3308,  3311,  3317,  3321,
    3327,  3334,  3334,  3340,  3341,  3355,  3358,  3361,  3367,  3373,
    3380,  3387,  3395,  3404,  3411,  3413,  3414,  3418,  3419,  3424,
    3430,  3432,  3433,  3434,  3447,  3449,  3450,  3451,  3464,  3466,
    3468,  3469,  3489,  3491,  3492,  3493,  3513,  3515,  3516,  3517,
    3529,  3592,  3594,  3595,  3600,  3602,  3603,  3605,  3606,  3608,
    3610,  3610,  3617,  3620,  3628,  3647,  3649,  3650,  3653,  3653,
    3670,  3670,  3677,  3677,  3684,  3687,  3689,  3691,  3692,  3694,
    3696,  3698,  3699,  3701,  3703,  3704,  3706,  3707,  3709,  3711,
    3714,  3718,  3720,  3721,  3723,  3724,  3726,  3728,  3739,  3740,
    3743,  3744,  3755,  3756,  3758,  3759,  3761,  3762,  3768,  3769,
    3772,  3773,  3774,  3798,  3799,  3802,  3803,  3804,  3807,  3807,
    3813,  3815,  3816,  3818,  3819,  3820,  3822,  3823,  3825,  3826,
    3828,  3829,  3831,  3832,  3834,  3835,  3838,  3839,  3842,  3844,
    3845,  3848,  3848,  3855,  3857,  3858,  3859,  3860,  3861,  3862,
    3863,  3865,  3866,  3867,  3868,  3869,  3870,  3871,  3872,  3873,
    3874,  3875,  3876,  3877,  3878,  3879,  3880,  3881,  3882,  3883,
    3884,  3885,  3886,  3887,  3888,  3889,  3890,  3891,  3892,  3893,
    3894,  3895,  3896,  3897,  3898,  3899,  3900,  3901,  3902,  3903,
    3904,  3905,  3906,  3907,  3908,  3909,  3910,  3911,  3912,  3913,
    3914,  3915,  3916,  3917,  3918,  3919,  3920,  3921,  3922,  3923,
    3924,  3925,  3926,  3927,  3928,  3929,  3930,  3931,  3932,  3933,
    3934,  3935,  3936,  3937,  3938,  3939,  3940,  3941,  3942,  3943,
    3944,  3945,  3948,  3955
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
  "NODE", "NODE_PRINT", "'+'", "'{'", "'}'", "';'", "'/'", "'['", "']'",
  "'='", "'('", "')'", "$accept", "start", "tmp_string", "string_1",
  "string_2", "$@1", "module_arg_str", "module_stmt",
  "module_header_stmts", "module_header_stmt", "submodule_arg_str",
  "submodule_stmt", "submodule_header_stmts", "submodule_header_stmt",
  "yang_version_stmt", "namespace_stmt", "linkage_stmts", "import_stmt",
  "import_arg_str", "import_opt_stmt", "include_arg_str", "include_stmt",
  "include_end", "include_opt_stmt", "revision_date_stmt",
  "belongs_to_arg_str", "belongs_to_stmt", "prefix_stmt", "meta_stmts",
  "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_arg_stmt", "revision_stmt",
  "revision_end", "revision_opt_stmt", "date_arg_str", "$@2",
  "body_stmts_end", "body_stmts", "body_stmt", "extension_arg_str",
  "extension_stmt", "extension_end", "extension_opt_stmt", "argument_stmt",
  "$@3", "argument_end", "yin_element_stmt", "yin_element_arg_str",
  "status_stmt", "status_arg_str", "feature_arg_str", "feature_stmt",
  "feature_end", "feature_opt_stmt", "if_feature_stmt", "identity_arg_str",
  "identity_stmt", "identity_end", "identity_opt_stmt", "base_stmt",
  "typedef_arg_str", "typedef_stmt", "type_opt_stmt", "type_stmt",
  "type_arg_str", "type_end", "type_body_stmts", "some_restrictions",
  "union_spec", "fraction_digits_stmt", "fraction_digits_arg_str",
  "length_stmt", "length_arg_str", "length_end", "message_opt_stmt",
  "pattern_stmt", "pattern_arg_str", "pattern_end", "pattern_opt_stmt",
  "modifier_stmt", "enum_specification", "enum_stmts", "enum_stmt",
  "enum_arg_str", "enum_end", "enum_opt_stmt", "value_stmt",
  "integer_value_arg_str", "range_stmt", "range_end", "path_stmt",
  "require_instance_stmt", "require_instance_arg_str",
  "bits_specification", "bit_stmts", "bit_stmt", "bit_arg_str", "bit_end",
  "bit_opt_stmt", "position_stmt", "position_value_arg_str",
  "error_message_stmt", "error_app_tag_stmt", "units_stmt", "default_stmt",
  "grouping_arg_str", "grouping_stmt", "grouping_end", "grouping_opt_stmt",
  "data_def_stmt", "container_arg_str", "container_stmt", "container_end",
  "container_opt_stmt", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10",
  "leaf_stmt", "leaf_arg_str", "leaf_opt_stmt", "$@11", "$@12", "$@13",
  "leaf_list_arg_str", "leaf_list_stmt", "leaf_list_opt_stmt", "$@14",
  "$@15", "$@16", "list_arg_str", "list_stmt", "list_opt_stmt", "$@17",
  "$@18", "$@19", "$@20", "$@21", "$@22", "$@23", "choice_arg_str",
  "choice_stmt", "choice_end", "choice_opt_stmt", "$@24", "$@25",
  "short_case_case_stmt", "short_case_stmt", "case_arg_str", "case_stmt",
  "case_end", "case_opt_stmt", "$@26", "$@27", "anyxml_arg_str",
  "anyxml_stmt", "anydata_arg_str", "anydata_stmt", "anyxml_end",
  "anyxml_opt_stmt", "$@28", "$@29", "uses_arg_str", "uses_stmt",
  "uses_end", "uses_opt_stmt", "$@30", "$@31", "$@32", "refine_args_str",
  "refine_arg_str", "refine_stmt", "refine_end", "refine_body_opt_stmts",
  "uses_augment_arg_str", "uses_augment_arg", "uses_augment_stmt",
  "augment_arg_str", "augment_arg", "augment_stmt", "augment_opt_stmt",
  "$@33", "$@34", "$@35", "$@36", "$@37", "action_arg_str", "action_stmt",
  "rpc_arg_str", "rpc_stmt", "rpc_end", "rpc_opt_stmt", "$@38", "$@39",
  "$@40", "$@41", "input_stmt", "$@42", "input_output_opt_stmt", "$@43",
  "$@44", "$@45", "$@46", "output_stmt", "$@47", "notification_arg_str",
  "notification_stmt", "notification_end", "notification_opt_stmt", "$@48",
  "$@49", "$@50", "$@51", "deviation_arg", "deviation_stmt",
  "deviation_opt_stmt", "deviation_arg_str", "deviate_body_stmt",
  "deviate_not_supported_stmt", "deviate_stmts", "deviate_add_stmt",
  "$@52", "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt",
  "$@53", "deviate_delete_end", "deviate_delete_opt_stmt",
  "deviate_replace_stmt", "$@54", "deviate_replace_end",
  "deviate_replace_opt_stmt", "when_arg_str", "when_stmt", "when_end",
  "when_opt_stmt", "config_stmt", "config_arg_str", "mandatory_stmt",
  "mandatory_arg_str", "presence_stmt", "min_elements_stmt",
  "min_value_arg_str", "max_elements_stmt", "max_value_arg_str",
  "ordered_by_stmt", "ordered_by_arg_str", "must_agr_str", "must_stmt",
  "must_end", "unique_stmt", "unique_arg_str", "unique_arg", "key_stmt",
  "key_arg_str", "$@55", "range_arg_str", "absolute_schema_nodeid",
  "absolute_schema_nodeids", "absolute_schema_nodeid_opt",
  "descendant_schema_nodeid", "$@56", "path_arg_str", "$@57", "$@58",
  "absolute_path", "absolute_paths", "absolute_path_opt", "relative_path",
  "relative_path_part1", "relative_path_part1_opt", "descendant_path",
  "descendant_path_opt", "path_predicate", "path_equality_expr",
  "path_key_expr", "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@59", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@60", "strings", "identifier", "identifiers",
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
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,    43,   123,   125,    59,    47,    91,    93,    61,    40,
      41
};
# endif

#define YYPACT_NINF -1020

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1020)))

#define YYTABLE_NINF -597

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1020,   112, -1020, -1020,    91, -1020, -1020, -1020,    73,    73,
   -1020, -1020,  2939,  2939,    73, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,   -19,
   -1020, -1020, -1020,    -2, -1020,    73, -1020,    73, -1020,    24,
     135,   135, -1020, -1020, -1020,   193, -1020, -1020, -1020,    41,
     297,    73,   100,    73,    73,    73, -1020, -1020, -1020,   100,
      73, -1020, -1020, -1020,    53,  2274, -1020,    73,    73, -1020,
   -1020,   174,  2369,  2939,  2369,   174,  2939, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020,  2939,  2939,    73,    73,    73,    73, -1020, -1020, -1020,
   -1020, -1020,    62, -1020,   183, -1020, -1020,   183, -1020,   183,
   -1020,    37, -1020,    73,    73,    73,    73,    71, -1020,   227,
   -1020,  2369,  2369,  2369,  2369,    74,  1920,    73, -1020, -1020,
   -1020, -1020, -1020,    73, -1020,    81, -1020,  1629,    95, -1020,
   -1020, -1020, -1020,   183,   183,   183,   183, -1020,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020,   101,    55,   135,    73, -1020,   289,
   -1020, -1020,    73, -1020, -1020,   135,   135,   135, -1020, -1020,
   -1020, -1020,    73,  2939,     3,  2939,  2939,  2939,     3,  2939,
    2939,  2939,  2939,  2939,  2939,  2939,  2939,  2939,  2464,  2939,
     135, -1020, -1020,   265, -1020, -1020,    73,    98,   269,   332,
      83,   325,   302, -1020,  3034, -1020, -1020,   106, -1020, -1020,
     310, -1020,   327, -1020,   331, -1020, -1020,   134, -1020, -1020,
     344, -1020,   363, -1020,   394, -1020,   142, -1020,   150, -1020,
     172, -1020,   404, -1020,   420, -1020,   184, -1020, -1020, -1020,
     428, -1020, -1020, -1020,   302, -1020, -1020, -1020, -1020, -1020,
     135, -1020, -1020, -1020, -1020, -1020,    73, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020,   186,    73, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020,    73, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020,    73,    73, -1020,    73,
     135,   135,   135,    73,    73,    73,   101,   135,   135,   135,
   -1020,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,    32,  2178,   183,   452,   950,
     334,  1805,   136,    60,   320,  1259,   313,  1379,   411,  1483,
    1056,  1002,   316,   164, -1020, -1020, -1020, -1020, -1020,    73,
      73,    73,    73,    73,    73, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020,    73,    73, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020,    73, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020,    73, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020,    73, -1020, -1020, -1020, -1020, -1020,    73, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020,    73, -1020, -1020, -1020, -1020,
   -1020, -1020,    73,    73, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020,    73,    73,    73, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020,    73,    73, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
      73,    73, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
      73,    18,  2369,    43,  2369,   107,  2369, -1020, -1020,  2939,
    2939, -1020, -1020, -1020, -1020, -1020,  2369, -1020, -1020,  2369,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020,  2939,   286,   135,
     135,   135,   135,   135,  2464,  2464,  2369, -1020, -1020, -1020,
      57,   242,    15, -1020, -1020, -1020,  2559,  2559, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,    73,
      73, -1020, -1020, -1020, -1020,   135,  2559,  2559, -1020, -1020,
   -1020, -1020, -1020, -1020,   183,   183, -1020, -1020, -1020,   183,
     434, -1020, -1020, -1020, -1020, -1020,   183,   441, -1020,   135,
     135,   449, -1020,   420, -1020,   135,   135,   135,   135,   135,
     183,   135,   135,   183,   135,   135,   135,   135,   135,   135,
     135, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020,   183,   472, -1020,   183,   135,   135,   135, -1020,
   -1020, -1020,   183, -1020, -1020, -1020,   183, -1020, -1020, -1020,
   -1020, -1020,   183,   135,   135,   135, -1020,   183, -1020,   183,
   -1020,    68, -1020,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   196,   228,   135,   135,   135,   135,
   -1020, -1020,   247, -1020, -1020, -1020,   473, -1020,   135,   135,
     135,    73,    73, -1020, -1020,    73,    73, -1020, -1020, -1020,
   -1020,    73,    73,    73, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020,   479,    73,    73,    68,    73,    73,
   -1020, -1020, -1020, -1020, -1020,    73, -1020,    73, -1020,    73,
      73,    73, -1020, -1020, -1020, -1020, -1020, -1020,  3034, -1020,
   -1020, -1020, -1020,    73, -1020, -1020, -1020,    73,   135,   135,
     135, -1020, -1020, -1020,   486,   487, -1020,   490,   488,    73,
      68,   186,   135,   135,   135,   135,   387,   143,   733,   117,
     135, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
      73,    73,   205,   771, -1020, -1020, -1020, -1020, -1020,  1301,
    1324,  1840,   849,    73,    73, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
      73,   251,   135,   135,   135,  2939,  2369, -1020,    73,    73,
      73,    73,    73,    73, -1020,   291, -1020, -1020, -1020, -1020,
   -1020, -1020,   135,   350, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020,  2369,  2369, -1020, -1020,    76, -1020,   595,    75,
     160,   491, -1020,   502, -1020,    87,  2369,   190,  2369,  2369,
      78, -1020,   135,   346, -1020, -1020, -1020, -1020, -1020,   135,
     183,   183,   135,   135, -1020, -1020, -1020,   183, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020,   183, -1020,   503,
   -1020, -1020,   183,   285,   374,   517, -1020,   526, -1020, -1020,
   -1020, -1020,   183,   135, -1020,   135,   135,   135,   135, -1020,
   -1020,    73,    73, -1020,   135,   135,   135,   135,   135,   135,
   -1020,    73, -1020, -1020, -1020, -1020,  3034, -1020, -1020,   290,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,    73,    73,
   -1020,   135,   655,   674,   135,   135, -1020,   285, -1020,  2654,
     135,   135,   135,    73, -1020, -1020, -1020, -1020, -1020, -1020,
      73, -1020, -1020, -1020, -1020, -1020, -1020,   418,   311, -1020,
   -1020, -1020,   159,   209,   584,   242,   272, -1020,   403, -1020,
     252,    73, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
     183, -1020, -1020, -1020, -1020, -1020,   183, -1020, -1020, -1020,
    3034, -1020,  2369, -1020,    73, -1020,    73,   403,   403,   183,
     315,   322, -1020, -1020,   403,   339,   403, -1020,   403,   336,
     329,   403,   403,   326,   436, -1020,   403, -1020, -1020,   356,
    2749,   403, -1020, -1020, -1020,  2844, -1020,   357,   403,  3034,
   -1020
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     646,     0,     2,     3,     0,     1,   644,   645,     0,     0,
     647,   646,     0,     0,   648,   662,     4,   661,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     685,   686,   687,   688,   689,   690,   691,   692,   693,   694,
     695,   696,   697,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   713,   714,
     715,   716,   717,   718,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   729,   730,   731,   732,   733,   734,
     735,   736,   737,   738,   739,   740,   741,   646,   617,     0,
       9,   742,   646,     0,    16,     6,   625,   616,   625,     5,
      12,    19,   646,   628,    24,    11,   627,   626,    24,    18,
       0,   631,    46,     0,     0,     0,    13,    14,    15,    46,
       0,    20,    21,     7,     0,   633,   632,     0,     0,    25,
      26,    57,     0,     0,     0,    57,     0,   646,   646,   638,
     629,   646,   654,   657,   655,   659,   660,   658,   630,   646,
     656,     0,     0,     0,     0,     0,     0,    47,    48,    49,
      50,    68,    55,   653,     0,   651,   614,     0,   646,     0,
      68,     0,    43,     8,   640,   636,   634,     0,    28,     0,
      34,     0,     0,     0,     0,     0,    67,     0,   625,   625,
      23,   646,    45,   615,    22,     0,   625,     0,   635,   625,
     625,    36,   625,     0,     0,     0,     0,   646,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   625,    70,    71,    72,    73,
      74,    75,   230,   231,   232,   233,   234,   235,   236,   237,
      76,    77,    78,    79,     0,     0,   623,   652,   646,     0,
     619,   639,   631,   618,   646,    29,    38,    35,    52,    53,
      51,    54,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,    64,    66,     0,    56,   625,    17,     0,     0,     0,
       0,     0,     0,   364,     0,   418,   419,     0,   576,   646,
       0,   326,     0,   238,     0,    80,   492,     0,   485,   646,
       0,   103,     0,   217,     0,   113,     0,   264,     0,   280,
       0,   299,     0,   468,     0,   438,     0,   124,   743,   622,
       0,   381,   646,   646,     0,   366,   646,   625,    59,   625,
     624,   625,   646,   646,   641,   646,     0,   625,    33,    30,
      31,    32,    37,    41,    39,    40,   625,   368,   365,   574,
     625,   575,   417,   625,   328,   327,   625,   240,   239,   625,
      82,    81,   625,   491,   625,   105,   104,   625,   219,   218,
     625,   115,   114,   625,   625,   625,   625,   470,   469,   625,
     440,   439,   625,   625,   383,   382,   620,   621,   367,    65,
      61,    58,    44,   640,   642,   637,     0,    27,   370,   421,
     577,   330,   242,    84,   487,   107,   221,   117,   265,   282,
     301,   472,   442,   126,   385,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    60,    62,    63,   646,    42,     0,
       0,     0,     0,     0,     0,   369,   379,   380,   378,   373,
     371,   376,   377,   374,     0,     0,   420,   426,   427,   425,
     424,   428,   434,   430,   432,   422,     0,   329,   338,   339,
     337,   333,   334,   344,   345,   346,   347,   350,   340,   342,
     343,   348,   349,   331,   335,   336,     0,   241,   251,   252,
     250,   245,   259,   253,   261,   255,   257,   243,   249,   248,
     246,     0,    83,    87,    88,    85,    86,     0,   486,   488,
     489,   106,   110,   111,   109,   108,   220,   223,   224,   222,
     625,   625,   625,   625,   625,     0,   116,   121,   122,   120,
     119,   118,     0,     0,   263,   278,   279,   277,   268,   269,
     271,   274,   266,   275,   276,   272,     0,     0,     0,   281,
     297,   298,   296,   285,   286,   289,   288,   283,   292,   293,
     294,   295,   290,     0,     0,   300,   314,   315,   313,   304,
     316,   318,   324,   320,   322,   302,   309,   310,   311,   312,
     305,   308,   307,   471,   477,   478,   476,   475,   479,   481,
     483,   473,   646,   646,   441,   445,   446,   444,   443,   447,
     449,   451,   453,   125,   131,   132,   130,   625,   128,   129,
       0,     0,   384,   390,   391,   389,   388,   392,   394,   386,
     643,     0,     0,     0,     0,     0,     0,   625,   625,     0,
       0,   625,   625,   625,   625,   625,     0,   625,   625,     0,
     625,   625,   625,   625,   625,   625,   625,     0,     0,   226,
     225,   227,   228,   229,     0,     0,     0,   625,   625,   625,
       0,     0,     0,   625,   625,   625,     0,     0,   625,   625,
     625,   625,   625,   625,   625,   625,   625,   625,   625,   455,
     466,   625,   625,   625,   625,   127,     0,     0,   625,   625,
     625,   646,   646,   543,     0,     0,   646,   646,   547,     0,
       0,   560,   646,   646,   646,   102,     0,     0,   533,   372,
     375,     0,   351,     0,   436,   429,   435,   431,   433,   423,
       0,   341,   332,     0,   260,   254,   262,   256,   258,   244,
     247,    89,   646,   646,   646,   646,   490,   493,   646,   496,
     498,   497,     0,     0,   134,     0,   270,   267,   273,   607,
     646,   555,     0,   646,   608,   551,     0,   609,   646,   646,
     646,   559,     0,   287,   284,   291,   572,     0,   570,     0,
     564,     0,   578,   317,   319,   325,   321,   323,   303,   306,
     480,   482,   484,   474,     0,     0,   448,   450,   452,   454,
     414,   415,     0,   646,   397,   398,     0,   646,   393,   395,
     387,   542,   541,   540,   112,   546,   545,   544,   625,   562,
     561,    99,   101,   100,    98,   625,   535,   534,   625,   353,
     352,   437,   216,   548,     0,   499,   512,     0,   521,   494,
     123,   625,   135,   133,   215,   553,   552,   554,   549,   550,
     558,   557,   556,   569,   646,   566,   565,   568,     0,   576,
     625,   625,   625,   413,   625,   400,   399,   396,   157,   537,
     355,   625,    91,   625,     0,     0,   495,     0,   140,   571,
       0,   579,   457,   457,   421,   402,     0,     0,     0,    93,
      90,   625,   501,   500,   625,   514,   513,   625,   523,   522,
       0,     0,     0,   137,   138,   625,   139,   198,   567,     0,
       0,     0,     0,     0,     0,   563,   160,   161,   158,   159,
     536,   538,   539,   354,   360,   361,   359,   358,   362,   356,
       0,     0,   503,   516,   525,     0,     0,   136,     0,     0,
       0,     0,     0,     0,   143,     0,   146,   144,   145,   625,
     142,   141,   174,   197,   456,   460,   462,   464,   458,   467,
     416,   401,   411,   412,   404,   406,   407,   408,   405,   409,
     410,   625,     0,     0,   625,   625,     0,    92,     0,     0,
       0,     0,   201,     0,   177,     0,     0,   580,     0,     0,
       0,   625,   147,   173,   199,   625,   625,   625,   625,   403,
       0,     0,   363,   357,   646,   646,    97,     0,   502,   504,
     507,   508,   509,   510,   511,   625,   506,   515,   517,   520,
     625,   519,   524,   625,   527,   528,   529,   530,   531,   532,
     625,   202,   625,   625,   178,   176,   152,     0,   646,     0,
     154,   584,     0,     0,     0,     0,   163,     0,   573,   646,
     646,   196,     0,   148,   625,   461,   463,   465,   459,   214,
     213,    96,    95,    94,   505,   518,   526,   204,   200,   180,
     150,   151,   625,   155,   625,   192,     0,   587,   581,     0,
     583,   591,   625,   164,   625,   625,   190,   189,   195,   194,
     193,   175,     0,     0,   157,   153,   596,   586,   590,     0,
     166,   162,   157,     0,   203,   208,   209,   207,   205,   206,
       0,   179,   184,   185,   183,   181,   182,     0,   585,   588,
     592,   589,   594,     0,     0,     0,     0,   156,   649,   593,
       0,     0,   165,   170,   171,   167,   168,   169,   191,   212,
       0,   646,   612,   613,   610,   188,     0,   646,   611,   650,
       0,   595,     0,   210,   211,   186,   187,   649,   649,     0,
       0,     0,   172,   597,   649,     0,   649,   598,   649,     0,
       0,   649,   649,     0,     0,   606,   649,   599,   602,     0,
       0,   649,   603,   604,   601,   649,   600,     0,   649,     0,
     605
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1020, -1020, -1020,   283, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020,   349, -1020,   354, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020,   179, -1020, -1020,  -182,   352, -1020,
   -1020,  -125,   726,   328, -1020, -1020, -1020, -1020,    89, -1020,
     317, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020,  -411, -1020, -1020, -1020, -1020, -1020,  -394, -1020,
   -1020, -1020, -1020,  -404, -1020,  -179, -1020,  -440, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1019, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020,  -512, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,  -450,
   -1020, -1020, -1020, -1020, -1020,  -615,  -607,  -403,  -376, -1020,
    -172, -1020, -1020,  -194, -1020,   104, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020,   130, -1020, -1020, -1020, -1020,
   -1020, -1020,   138, -1020, -1020, -1020, -1020, -1020,   156, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,   157, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020,   170, -1020, -1020, -1020,
   -1020, -1020,   171, -1020,   176,   187, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020,  -367, -1020, -1020,
   -1020, -1020, -1020, -1020,  -138, -1020, -1020,  -193, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020,  -346, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020,  -148, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
   -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020, -1020,
    -359, -1020, -1020,  -434, -1020,  -422, -1020,  -370,  -421, -1020,
    -406, -1020,   175, -1020, -1020,  -380, -1020,  -800, -1020,  -270,
   -1020, -1020, -1020, -1020,  -362,   345,  -240,  -685, -1020, -1020,
   -1020, -1020,  -476,  -507, -1020, -1020,  -475, -1020, -1020, -1020,
    -497, -1020, -1020, -1020,  -553, -1020, -1020, -1020,  -661,  -496,
   -1020, -1020, -1020,   938,  -175,  -235,   466,  1102, -1020, -1020,
     376, -1020, -1020, -1020, -1020,   231, -1020,    -4,   -10,   435,
     415,    90, -1020,   506,   206,  -140, -1020
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   109,   147,    99,     2,   114,   115,
     103,     3,   118,   119,   126,   127,   122,   139,   187,   300,
     189,   140,   212,   301,   358,   181,   132,   128,   141,   167,
     168,   926,   927,   171,   293,   172,   349,   435,   294,   346,
     195,   196,   235,   314,   236,   381,   442,   525,   844,   883,
     941,  1017,   479,   726,   320,   237,   386,   444,   480,   324,
     238,   392,   446,   551,   336,   965,   452,   559,   763,   853,
     912,   913,   955,   956,  1047,   957,  1049,  1084,   896,   958,
    1055,  1094,  1133,  1145,   914,  1003,   915,   993,  1045,  1103,
    1126,  1156,   959,  1097,   960,   961,  1062,   916,   963,   917,
     991,  1042,  1102,  1119,  1150,   928,   929,   560,   492,   322,
     966,   389,   445,   481,   312,   242,   378,   441,   665,   666,
     661,   663,   664,   660,   662,   243,   326,   447,   678,   677,
     679,   328,   244,   448,   684,   683,   685,   330,   245,   449,
     693,   694,   688,   689,   691,   692,   690,   310,   246,   375,
     440,   658,   657,   498,   499,   731,   482,   840,   898,   985,
     984,   302,   247,   344,   248,   368,   438,   647,   648,   340,
     249,   405,   453,   710,   708,   709,   815,   816,   637,   876,
     922,   811,   812,   638,   306,   307,   250,   439,   655,   651,
     653,   654,   652,   733,   483,   334,   251,   401,   451,   701,
     702,   703,   704,   621,   804,   919,  1008,  1005,  1006,  1007,
     622,   805,   332,   484,   398,   450,   698,   695,   696,   697,
     317,   253,   443,   318,   756,   757,   758,   759,   884,   903,
     988,   760,   885,   906,   989,   761,   887,   909,   990,   727,
     485,   837,   897,   471,   714,   472,   719,   519,   579,   776,
     580,   772,   581,   782,   720,   968,   830,   601,   790,   866,
     602,   787,   864,  1057,   308,   309,   371,   791,   869,  1052,
    1053,  1054,  1087,  1088,  1107,  1090,  1091,  1109,  1131,  1139,
    1128,  1167,  1177,  1187,  1188,  1190,  1195,  1178,   777,   778,
    1157,  1158,   177,   100,   792,   341,   867,   110,   116,   121,
     134,   135,   158,   208,   150,   207,   354,   117,     4,   136,
    1160,   174,   201,   175,   101,   102,   343
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    14,   241,   178,    11,    11,   504,   518,   574,   420,
      10,    16,   627,   563,   578,   596,   169,   239,   505,   773,
     169,   813,   817,    16,   240,   564,    16,   468,   597,   490,
     510,   526,   262,   534,   539,   549,   557,   572,   588,   606,
     617,   626,   635,   598,   469,   575,   491,   511,   252,   628,
     535,    16,   550,   558,   573,   589,   607,   618,   473,   636,
       6,   520,     7,   164,   130,    16,   113,   565,   582,   600,
     611,   561,   576,     6,   769,     7,   629,   297,     6,   470,
       7,   503,   517,   106,    16,  1127,    16,   105,   562,   577,
     595,   164,   107,  1134,   639,    16,     6,   527,     7,   166,
     108,    10,   120,    10,   769,   486,   711,   779,   304,    16,
     780,   712,     5,   291,   164,    16,    10,    11,   359,    11,
      11,    11,     6,   125,     7,   112,    11,   166,   113,   369,
     462,   716,   197,    11,    11,   454,   717,   183,   184,   206,
       6,   185,     7,   137,   138,     8,   113,   124,   342,   186,
     166,   770,   584,   553,   356,   148,   521,   149,   295,    11,
      11,    11,    11,   528,  1014,     9,  1059,   164,   203,  1015,
     198,  1060,   199,   209,   164,   360,   364,   217,  1027,    10,
      10,    10,    10,   890,   258,   630,   357,   459,  1026,  1031,
     486,   257,   722,    11,   723,   164,   264,   724,    16,    10,
     940,   351,   163,   166,  -582,   164,   460,   272,   370,   463,
     166,   461,   566,   567,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,   166,   631,   165,   179,   552,   382,   463,   553,   522,
     164,   166,   923,   924,   393,   464,   930,   514,   296,   123,
      16,   542,   394,    10,   299,   592,   610,   124,    11,   769,
     774,   262,   512,  1032,  -596,  -596,   540,   632,    10,   513,
     590,   608,   619,   541,   395,   125,   166,   591,   609,   620,
      16,   213,   214,   215,   216,   198,   402,   199,  1152,  1153,
    1154,   304,    10,   516,     6,    10,     7,   544,   870,   372,
     113,   594,     6,   515,     7,   133,  1141,   543,   947,   383,
     455,   593,  1142,   466,   477,   488,   508,   523,   529,   532,
     537,   547,   555,   570,   586,   604,   615,   624,   633,   210,
     871,   211,   406,   407,  1048,   545,   409,     6,   218,     7,
     355,   160,   413,   414,   164,   415,   486,   164,   160,   872,
     160,   164,    11,   124,   987,   460,   164,  1086,  1138,   474,
     220,   459,   460,   221,   486,   164,   552,   347,    10,   348,
     752,   352,   753,   353,   910,   754,   460,   755,   911,    10,
     166,   227,   228,   166,   229,   461,   463,   166,  1089,   463,
    1086,   552,   166,   463,   553,  1108,   356,   160,   160,   160,
     160,   166,    10,    10,   366,    10,   367,   463,  1159,    10,
      10,    10,   373,   263,   374,   464,   546,  1138,   164,   623,
     923,   924,  1173,   531,  1176,   173,   176,   173,   362,   376,
    1174,   377,   234,   379,  1182,   380,  1185,   487,   459,   762,
     764,   486,   164,    12,    13,  1181,   384,   640,   385,   164,
    1186,   923,   924,   460,   166,    11,    11,    11,    11,    11,
      11,  1191,  1198,   566,   567,   387,   462,   388,   131,   568,
      11,    11,   129,   180,   173,   173,   173,   173,   166,   459,
     363,   145,    11,   164,   463,   166,   552,   936,   976,   553,
     925,  1064,   464,     6,   460,     7,   390,   205,   391,   113,
     977,   979,    11,   461,   937,   437,   396,   462,   397,   954,
     263,   788,   910,  1004,   569,  1001,   980,    11,  1146,   166,
     911,  1137,   399,    11,   400,   463,  1147,   921,   974,   420,
     403,   408,   404,   464,   342,   342,   828,   292,   829,   939,
     841,    11,   981,   835,   493,   836,   975,   920,    11,    11,
    1033,   838,   978,   839,  1021,   465,  1036,   305,   142,   143,
     144,   316,    11,    11,    11,   146,  1022,  1023,  1037,  1038,
     494,   339,   161,   162,   851,   874,   852,   875,   495,    11,
      11,   881,  1024,   882,  1039,  1019,  1028,  1034,   901,   904,
     902,   905,   907,  1040,   908,  1041,   496,   497,   191,   192,
     193,   194,   699,   700,  1043,  1082,  1044,  1083,  1025,  1030,
     500,   501,  1020,  1029,  1035,   164,   502,   923,   924,  1092,
     918,  1093,   459,   319,   599,   486,    11,    11,  1095,   891,
    1096,  1129,   254,  1161,  1130,  1140,    10,  1192,   298,  1151,
     200,   159,   263,   202,   436,   204,   461,   566,   567,     0,
     462,   166,     0,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
       0,     0,   584,   553,     0,     0,     0,     0,     0,   268,
     269,   270,   271,     0,     0,     0,   164,  1148,     0,     0,
       0,  1117,  1124,     0,     0,    10,    10,   460,  1018,   292,
       0,   821,   822,     0,   938,   164,   825,   826,  1118,  1125,
       0,     0,   831,   832,   833,     0,   460,     0,  1113,     0,
       0,     0,   166,     0,     0,   967,   967,     0,   463,     0,
       0,     0,   715,     0,   721,     0,   728,   218,     0,     0,
       0,   166,   845,   846,   847,   848,   740,   463,   849,   743,
       0,     0,     0,     0,  1120,     0,     0,     0,  1114,   220,
     855,     0,   221,   857,   164,     0,   765,     0,   859,   860,
     861,     0,   931,   934,     0,   460,     0,  1121,     0,     0,
     227,   228,     0,   229,     0,     0,     0,    11,     0,     0,
       0,   416,     0,   545,     0,     0,   477,   972,     0,     0,
     166,     0,     0,   873,     0,     0,   463,   877,     0,     0,
     948,     0,   233,     0,   464,     0,     0,    10,    10,     0,
     949,    10,    10,     0,     0,     0,     0,    10,    10,    10,
       0,   234,   950,   951,     0,     0,   933,   952,     0,     0,
     953,    10,    10,    10,    10,    10,  -149,     0,   160,     0,
     160,    10,   160,    10,   889,    10,    10,    10,     0,     0,
       0,     0,   160,     0,     0,   160,     0,   170,     0,    10,
       0,   170,     0,    10,     0,     0,   459,     0,     0,   486,
     164,     0,   160,     0,     0,    10,    11,     0,     0,     0,
       0,   460,   263,   263,   641,   642,   643,   644,   645,   646,
     461,   566,   567,   458,   462,     0,    11,    11,     0,   649,
     650,  1106,   263,   263,   506,     0,   166,     0,     0,    11,
      11,   656,     0,     0,   713,   173,   718,   173,   725,   173,
       0,     0,     0,     0,  1132,     0,    11,     0,     0,   173,
       0,   659,   173,     0,    11,    11,    11,    11,    11,    11,
       0,   104,   971,     0,   218,     0,   667,   339,   339,   173,
       0,     0,   668,   771,   775,   781,     0,     0,     0,   786,
     789,     0,     0,     0,     0,   474,   220,  1115,  1122,   221,
     674,   164,     0,     0,     0,  1168,     0,   675,   676,   810,
     814,     0,   460,     0,     0,     0,     0,   227,   228,     0,
     229,   680,   681,   682,  1071,  1072,     0,   230,  1143,     0,
       0,     0,     0,     0,     0,  1193,     0,   166,   686,   687,
    1196,     0,     0,   463,  1200,     0,   361,   365,     0,   233,
       0,   464,     0,   164,     0,     0,   994,     0,  1081,     0,
       0,     0,   225,     0,   460,     0,   475,   612,   234,  1098,
    1099,     0,     0,   476,     0,     0,     0,     0,     0,     0,
     218,     0,   613,     0,     0,   706,   707,    10,    10,   166,
       0,     0,  1010,  1011,   263,   463,     0,    10,   232,     0,
       0,     0,   220,     0,   182,   221,  1050,   164,  1056,  1058,
       0,     0,     0,     0,    10,    10,   225,     0,   460,   188,
     190,     0,     0,   227,   228,   614,   229,     0,     0,    11,
       0,   462,     0,     0,     0,     0,    11,     0,     0,     0,
       0,     0,     0,   166,     0,     0,     0,     0,     0,   463,
       0,     0,   232,     0,     0,   233,     0,    11,     0,     0,
       0,  1164,     0,     0,     0,     0,     0,  1166,     0,     0,
       0,     0,   160,     0,   234,     0,     0,     0,     0,   603,
      10,   456,    10,     0,   467,   478,   489,   509,   524,   530,
     533,   538,   548,   556,   571,   587,   605,   616,   625,   634,
     823,   824,     0,     0,     0,   827,     0,     0,   160,   160,
       0,     0,   834,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   160,     0,   160,   160,   842,     0,     0,   843,
     111,   303,     0,   311,   313,   315,     0,   321,   323,   325,
     327,   329,   331,   333,   335,   337,   868,   345,   850,   173,
       0,   854,     0,     0,     0,     0,     0,     0,   856,     0,
       0,     0,   858,     0,     0,     0,     0,     0,   862,     0,
       0,     0,  1169,   863,     0,   865,     0,     0,     0,     0,
       0,     0,     0,   218,     0,   173,   173,     0,     0,  1016,
       0,     0,     0,     0,     0,     0,     0,     0,  1046,   173,
    1051,   173,   173,  1061,     0,   220,     0,     0,   221,     0,
     164,     0,   263,     0,     0,     0,     0,     0,     0,   225,
     255,   256,     0,     0,     0,   218,   227,   228,   259,   229,
       0,   265,   266,   886,   267,   263,   230,     0,     0,     0,
       0,     0,     0,     0,     0,   868,   166,   220,   218,     0,
     221,     0,   463,     0,     0,   232,     0,   290,   233,     0,
       0,   225,     0,     0,     0,   945,   946,     0,   227,   228,
     220,   229,     0,   221,     0,   475,   462,   234,   982,   983,
       0,     0,   536,     0,   225,     0,   263,     0,   160,     0,
       0,   227,   228,     0,   229,   986,     0,   232,     0,   462,
     233,     0,     0,   995,   996,   997,   998,   999,  1000,     0,
       0,     0,     0,     0,     0,     0,   263,   350,     0,   234,
     232,   263,     0,   233,   964,   263,   459,     0,     0,   486,
     164,     0,     0,     0,     0,     0,     0,     0,  1149,  1155,
       0,   460,   234,     0,     0,     0,     0,   969,     0,     0,
     461,     0,     0,     0,   462,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   173,   166,     0,     0,   410,
       0,   411,   463,   412,   552,     0,     0,   553,     0,   417,
     464,     0,     0,     0,     0,     0,     0,     0,   418,     0,
       0,     0,   419,     0,     0,   421,  1069,  1070,   422,     0,
       0,   423,   554,  1073,   424,     0,   425,   218,     0,   426,
       0,     0,   427,     0,     0,   428,   429,   430,   431,     0,
       0,   432,     0,     0,   433,   434,     0,     0,     0,   220,
     459,     0,   221,  1080,   164,     0,     0,     0,  1085,     0,
       0,     0,     0,   225,     0,   460,     0,     0,  1100,   583,
     227,   228,     0,   229,     0,   566,   567,     0,   462,     0,
     230,   568,     0,     0,     0,     0,     0,     0,  1135,     0,
     166,     0,     0,     0,     0,  1136,   463,     0,     0,   232,
     584,     0,   233,     0,   464,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1162,     0,     0,   475,
       0,   234,  1170,  1171,     0,     0,   585,   732,   734,  1175,
       0,  1179,     0,  1180,     0,     0,  1183,  1184,     0,     0,
       0,  1189,     0,     0,     0,   751,  1194,     0,     0,     0,
    1197,     0,     0,  1199,     0,     0,  1163,     0,     0,     0,
       0,     0,  1165,   932,   935,     0,     0,     0,     0,     0,
       0,     0,     0,    15,     0,  1172,     0,     0,     0,    17,
     260,     0,   669,   670,   671,   672,   673,   478,   973,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,     0,   705,
       0,     0,   261,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   729,
     730,     0,     0,   735,   736,   737,   738,   739,     0,   741,
     742,     0,   744,   745,   746,   747,   748,   749,   750,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   766,
     767,   768,     0,     0,     0,   783,   784,   785,     0,     0,
     793,   794,   795,   796,   797,   798,   799,   800,   801,   802,
     803,     0,     0,   806,   807,   808,   809,     0,     0,   218,
     818,   819,   820,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1116,  1123,
       0,   220,   459,     0,   221,     0,   164,     0,     0,     0,
       0,     0,     0,     0,   218,   225,     0,   460,     0,     0,
       0,     0,   227,   228,     0,   229,     0,     0,     0,  1144,
     462,     0,   230,     0,     0,   474,   220,     0,     0,   221,
     506,   164,   166,     0,     0,     0,     0,     0,   463,     0,
       0,   232,   460,   992,   233,     0,   464,   227,   228,     0,
     229,     0,     0,     0,     0,     0,     0,   230,     0,     0,
       0,   475,     0,   234,     0,     0,     0,   166,   507,     0,
       0,     0,     0,   463,     0,     0,     0,     0,     0,   233,
       0,   464,     0,     0,   218,     0,     0,     0,     0,     0,
     878,     0,     0,     0,     0,     0,   475,   879,   234,     0,
     880,   219,     0,   970,     0,     0,   220,     0,     0,   221,
       0,     0,     0,   888,     0,   222,   223,     0,   224,     0,
     225,   226,     0,     0,     0,     0,     0,   227,   228,     0,
     229,     0,   892,   893,   894,     0,   895,   230,     0,     0,
       0,     0,     0,   899,     0,   900,     0,     0,     0,     0,
       0,     0,   231,     0,     0,     0,   232,     0,     0,   233,
       0,     0,     0,   942,     0,     0,   943,     0,     0,   944,
       0,     0,     0,     0,     0,     0,     0,   962,   234,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1002,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1009,     0,     0,  1012,  1013,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1063,     0,     0,     0,  1065,  1066,  1067,
    1068,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1074,     0,     0,
       0,     0,  1075,     0,     0,  1076,     0,     0,     0,     0,
       0,     0,  1077,     0,  1078,  1079,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1101,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,     0,  1104,     0,  1105,     0,    17,   260,
       0,     0,     0,     0,  1110,     0,  1111,  1112,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,     0,    15,     0,
       0,   457,   151,   152,    17,   153,   154,     0,     0,     0,
     155,   156,   157,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,    16,   152,    17,
     153,   154,     0,     0,     0,   155,   156,   157,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,     0,
       0,     0,    16,     0,    17,   338,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,    16,     0,    17,
     260,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,     0,
       0,     0,     0,     0,    17,   260,     0,     0,  1089,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,     0,     0,    17,
     260,     0,     0,  1186,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,  1159,
       0,     0,     0,     0,    17,   260,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,    16,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,     0,
       0,     0,     0,     0,    17,   260,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96
};

static const yytype_int16 yycheck[] =
{
       4,    11,   196,   143,     8,     9,   440,   441,   448,   371,
      14,     8,   452,   447,   448,   449,   141,   196,   440,   680,
     145,   706,   707,     8,   196,   447,     8,   438,   449,   440,
     441,   442,   207,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   449,   438,   448,   440,   441,   196,   452,
     444,     8,   446,   447,   448,   449,   450,   451,   438,   453,
       5,   441,     7,    31,    23,     8,    11,   447,   448,   449,
     450,   447,   448,     5,    17,     7,   452,   259,     5,   438,
       7,   440,   441,   102,     8,  1104,     8,    97,   447,   448,
     449,    31,   102,  1112,   453,     8,     5,    37,     7,    67,
     102,   105,   112,   107,    17,    30,    88,    92,   105,     8,
      95,    93,     0,    12,    31,     8,   120,   121,   300,   123,
     124,   125,     5,    82,     7,   101,   130,    67,    11,   304,
      55,    88,    70,   137,   138,   103,    93,   147,   148,   102,
       5,   151,     7,    43,    44,    54,    11,    64,   288,   159,
      67,    94,    77,    78,    71,   102,    20,   104,   103,   163,
     164,   165,   166,   103,    88,    74,    88,    31,   178,    93,
     102,    93,   104,   102,    31,   300,   301,   103,   103,   183,
     184,   185,   186,   868,   103,    21,   103,    27,   988,   989,
      30,   201,    85,   197,    87,    31,   101,    90,     8,   203,
      83,   103,    28,    67,    14,    31,    42,   217,   102,    73,
      67,    51,    52,    53,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,    67,    68,    59,   144,    75,   102,    73,    78,   103,
      31,    67,    33,    34,   102,    81,   103,   441,   258,    56,
       8,   445,   102,   257,   264,   449,   450,    64,   262,    17,
      18,   436,   441,   103,   105,   106,   445,   103,   272,   441,
     449,   450,   451,   445,   102,    82,    67,   449,   450,   451,
       8,   191,   192,   193,   194,   102,   102,   104,    16,    17,
      18,   105,   296,   441,     5,   299,     7,   445,   102,   309,
      11,   449,     5,   441,     7,     8,    97,   445,   103,   319,
     435,   449,   103,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   102,
     102,   104,   342,   343,   995,    22,   346,     5,     4,     7,
       8,   135,   352,   353,    31,   355,    30,    31,   142,   102,
     144,    31,   356,    64,   103,    42,    31,   105,   106,    25,
      26,    27,    42,    29,    30,    31,    75,   102,   372,   104,
      84,   102,    86,   104,    24,    89,    42,    91,    32,   383,
      67,    47,    48,    67,    50,    51,    73,    67,    14,    73,
     105,    75,    67,    73,    78,   105,    71,   191,   192,   193,
     194,    67,   406,   407,   102,   409,   104,    73,     5,   413,
     414,   415,   102,   207,   104,    81,   103,   106,    31,   103,
      33,    34,   107,   103,    85,   142,   143,   144,   103,   102,
     108,   104,    98,   102,   105,   104,   110,   103,    27,   674,
     675,    30,    31,     8,     9,   109,   102,   457,   104,    31,
      14,    33,    34,    42,    67,   459,   460,   461,   462,   463,
     464,   105,   105,    52,    53,   102,    55,   104,   119,    58,
     474,   475,   118,   145,   191,   192,   193,   194,    67,    27,
     301,   129,   486,    31,    73,    67,    75,   898,   922,    78,
     103,  1003,    81,     5,    42,     7,   102,   180,   104,    11,
     922,   922,   506,    51,   898,   416,   102,    55,   104,   913,
     304,   686,    24,   963,   103,   955,   922,   521,  1133,    67,
      32,   103,   102,   527,   104,    73,  1133,   894,   922,   891,
     102,   344,   104,    81,   674,   675,   102,   254,   104,   898,
     733,   545,   922,   102,   440,   104,   922,   893,   552,   553,
     990,   102,   922,   104,   988,   103,   990,   274,   123,   124,
     125,   278,   566,   567,   568,   130,   988,   988,   990,   990,
     440,   288,   137,   138,   102,   102,   104,   104,   440,   583,
     584,   102,   988,   104,   990,   988,   989,   990,   102,   102,
     104,   104,   102,   102,   104,   104,   440,   440,   163,   164,
     165,   166,   612,   613,   102,   102,   104,   104,   988,   989,
     440,   440,   988,   989,   990,    31,   440,    33,    34,   102,
     890,   104,    27,   278,   449,    30,   630,   631,   102,   869,
     104,  1107,   197,  1140,  1109,  1132,   640,  1190,   262,  1135,
     174,   135,   436,   177,   413,   179,    51,    52,    53,    -1,
      55,    67,    -1,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
      -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,   213,
     214,   215,   216,    -1,    -1,    -1,    31,   103,    -1,    -1,
      -1,  1102,  1103,    -1,    -1,   699,   700,    42,   103,   416,
      -1,   711,   712,    -1,   898,    31,   716,   717,  1102,  1103,
      -1,    -1,   722,   723,   724,    -1,    42,    -1,    63,    -1,
      -1,    -1,    67,    -1,    -1,   919,   920,    -1,    73,    -1,
      -1,    -1,   642,    -1,   644,    -1,   646,     4,    -1,    -1,
      -1,    67,   752,   753,   754,   755,   656,    73,   758,   659,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,   103,    26,
     770,    -1,    29,   773,    31,    -1,   676,    -1,   778,   779,
     780,    -1,   897,   898,    -1,    42,    -1,   103,    -1,    -1,
      47,    48,    -1,    50,    -1,    -1,    -1,   791,    -1,    -1,
      -1,   356,    -1,    22,    -1,    -1,   921,   922,    -1,    -1,
      67,    -1,    -1,   813,    -1,    -1,    73,   817,    -1,    -1,
      39,    -1,    79,    -1,    81,    -1,    -1,   821,   822,    -1,
      49,   825,   826,    -1,    -1,    -1,    -1,   831,   832,   833,
      -1,    98,    61,    62,    -1,    -1,   103,    66,    -1,    -1,
      69,   845,   846,   847,   848,   849,    75,    -1,   642,    -1,
     644,   855,   646,   857,   864,   859,   860,   861,    -1,    -1,
      -1,    -1,   656,    -1,    -1,   659,    -1,   141,    -1,   873,
      -1,   145,    -1,   877,    -1,    -1,    27,    -1,    -1,    30,
      31,    -1,   676,    -1,    -1,   889,   890,    -1,    -1,    -1,
      -1,    42,   686,   687,   459,   460,   461,   462,   463,   464,
      51,    52,    53,   437,    55,    -1,   910,   911,    -1,   474,
     475,  1086,   706,   707,    65,    -1,    67,    -1,    -1,   923,
     924,   486,    -1,    -1,   641,   642,   643,   644,   645,   646,
      -1,    -1,    -1,    -1,  1109,    -1,   940,    -1,    -1,   656,
      -1,   506,   659,    -1,   948,   949,   950,   951,   952,   953,
      -1,    13,   103,    -1,     4,    -1,   521,   674,   675,   676,
      -1,    -1,   527,   680,   681,   682,    -1,    -1,    -1,   686,
     687,    -1,    -1,    -1,    -1,    25,    26,  1102,  1103,    29,
     545,    31,    -1,    -1,    -1,  1160,    -1,   552,   553,   706,
     707,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,    -1,
      50,   566,   567,   568,  1014,  1015,    -1,    57,  1133,    -1,
      -1,    -1,    -1,    -1,    -1,  1190,    -1,    67,   583,   584,
    1195,    -1,    -1,    73,  1199,    -1,   300,   301,    -1,    79,
      -1,    81,    -1,    31,    -1,    -1,   946,    -1,  1048,    -1,
      -1,    -1,    40,    -1,    42,    -1,    96,    45,    98,  1059,
    1060,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    60,    -1,    -1,   630,   631,  1071,  1072,    67,
      -1,    -1,   982,   983,   868,    73,    -1,  1081,    76,    -1,
      -1,    -1,    26,    -1,   146,    29,   996,    31,   998,   999,
      -1,    -1,    -1,    -1,  1098,  1099,    40,    -1,    42,   161,
     162,    -1,    -1,    47,    48,   103,    50,    -1,    -1,  1113,
      -1,    55,    -1,    -1,    -1,    -1,  1120,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    76,    -1,    -1,    79,    -1,  1141,    -1,    -1,
      -1,  1151,    -1,    -1,    -1,    -1,    -1,  1157,    -1,    -1,
      -1,    -1,   946,    -1,    98,    -1,    -1,    -1,    -1,   103,
    1164,   435,  1166,    -1,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     714,   715,    -1,    -1,    -1,   719,    -1,    -1,   982,   983,
      -1,    -1,   726,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   996,    -1,   998,   999,   740,    -1,    -1,   743,
     108,   273,    -1,   275,   276,   277,    -1,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   791,   289,   762,   946,
      -1,   765,    -1,    -1,    -1,    -1,    -1,    -1,   772,    -1,
      -1,    -1,   776,    -1,    -1,    -1,    -1,    -1,   782,    -1,
      -1,    -1,  1162,   787,    -1,   789,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,    -1,   982,   983,    -1,    -1,   986,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   995,   996,
     997,   998,   999,  1000,    -1,    26,    -1,    -1,    29,    -1,
      31,    -1,  1086,    -1,    -1,    -1,    -1,    -1,    -1,    40,
     198,   199,    -1,    -1,    -1,     4,    47,    48,   206,    50,
      -1,   209,   210,   847,   212,  1109,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   890,    67,    26,     4,    -1,
      29,    -1,    73,    -1,    -1,    76,    -1,   235,    79,    -1,
      -1,    40,    -1,    -1,    -1,   910,   911,    -1,    47,    48,
      26,    50,    -1,    29,    -1,    96,    55,    98,   923,   924,
      -1,    -1,   103,    -1,    40,    -1,  1160,    -1,  1162,    -1,
      -1,    47,    48,    -1,    50,   940,    -1,    76,    -1,    55,
      79,    -1,    -1,   948,   949,   950,   951,   952,   953,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1190,   295,    -1,    98,
      76,  1195,    -1,    79,   103,  1199,    27,    -1,    -1,    30,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1135,  1136,
      -1,    42,    98,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      51,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1162,    67,    -1,    -1,   347,
      -1,   349,    73,   351,    75,    -1,    -1,    78,    -1,   357,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   366,    -1,
      -1,    -1,   370,    -1,    -1,   373,  1010,  1011,   376,    -1,
      -1,   379,   103,  1017,   382,    -1,   384,     4,    -1,   387,
      -1,    -1,   390,    -1,    -1,   393,   394,   395,   396,    -1,
      -1,   399,    -1,    -1,   402,   403,    -1,    -1,    -1,    26,
      27,    -1,    29,  1047,    31,    -1,    -1,    -1,  1052,    -1,
      -1,    -1,    -1,    40,    -1,    42,    -1,    -1,  1062,    46,
      47,    48,    -1,    50,    -1,    52,    53,    -1,    55,    -1,
      57,    58,    -1,    -1,    -1,    -1,    -1,    -1,  1113,    -1,
      67,    -1,    -1,    -1,    -1,  1120,    73,    -1,    -1,    76,
      77,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1141,    -1,    -1,    96,
      -1,    98,  1167,  1168,    -1,    -1,   103,   649,   650,  1174,
      -1,  1176,    -1,  1178,    -1,    -1,  1181,  1182,    -1,    -1,
      -1,  1186,    -1,    -1,    -1,   667,  1191,    -1,    -1,    -1,
    1195,    -1,    -1,  1198,    -1,    -1,  1150,    -1,    -1,    -1,
      -1,    -1,  1156,   897,   898,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,    -1,  1169,    -1,    -1,    -1,    10,
      11,    -1,   540,   541,   542,   543,   544,   921,   922,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    -1,   627,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   647,
     648,    -1,    -1,   651,   652,   653,   654,   655,    -1,   657,
     658,    -1,   660,   661,   662,   663,   664,   665,   666,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   677,
     678,   679,    -1,    -1,    -1,   683,   684,   685,    -1,    -1,
     688,   689,   690,   691,   692,   693,   694,   695,   696,   697,
     698,    -1,    -1,   701,   702,   703,   704,    -1,    -1,     4,
     708,   709,   710,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1102,  1103,
      -1,    26,    27,    -1,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     4,    40,    -1,    42,    -1,    -1,
      -1,    -1,    47,    48,    -1,    50,    -1,    -1,    -1,  1133,
      55,    -1,    57,    -1,    -1,    25,    26,    -1,    -1,    29,
      65,    31,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    76,    42,   945,    79,    -1,    81,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    96,    -1,    98,    -1,    -1,    -1,    67,   103,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    81,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
     828,    -1,    -1,    -1,    -1,    -1,    96,   835,    98,    -1,
     838,    21,    -1,   103,    -1,    -1,    26,    -1,    -1,    29,
      -1,    -1,    -1,   851,    -1,    35,    36,    -1,    38,    -1,
      40,    41,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,
      50,    -1,   870,   871,   872,    -1,   874,    57,    -1,    -1,
      -1,    -1,    -1,   881,    -1,   883,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    76,    -1,    -1,    79,
      -1,    -1,    -1,   901,    -1,    -1,   904,    -1,    -1,   907,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   915,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   959,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   981,    -1,    -1,   984,   985,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1001,    -1,    -1,    -1,  1005,  1006,  1007,
    1008,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1025,    -1,    -1,
      -1,    -1,  1030,    -1,    -1,  1033,    -1,    -1,    -1,    -1,
      -1,    -1,  1040,    -1,  1042,  1043,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1064,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,    -1,  1082,    -1,  1084,    -1,    10,    11,
      -1,    -1,    -1,    -1,  1092,    -1,  1094,  1095,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    -1,     4,    -1,
      -1,   103,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      16,    17,    18,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,     4,    -1,    -1,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    16,    17,    18,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,     4,    -1,
      -1,    -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,     4,    -1,    -1,    -1,     8,    -1,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,     4,    -1,
      -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,     4,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,     4,     5,
      -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,     4,    -1,    -1,    -1,     8,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,     4,    -1,
      -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   112,   118,   122,   419,     0,     5,     7,    54,    74,
     418,   418,   420,   420,   419,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   113,   114,   117,
     404,   425,   426,   121,   404,   419,   102,   419,   102,   115,
     408,   408,   101,    11,   119,   120,   409,   418,   123,   124,
     419,   410,   127,    56,    64,    82,   125,   126,   138,   127,
      23,   125,   137,     8,   411,   412,   420,    43,    44,   128,
     132,   139,   420,   420,   420,   139,   420,   116,   102,   104,
     415,     8,     9,    11,    12,    16,    17,    18,   413,   424,
     425,   420,   420,    28,    31,    59,    67,   140,   141,   142,
     143,   144,   146,   114,   422,   424,   114,   403,   426,   422,
     144,   136,   404,   419,   419,   419,   419,   129,   404,   131,
     404,   420,   420,   420,   420,   151,   152,    70,   102,   104,
     407,   423,   407,   419,   407,   151,   102,   416,   414,   102,
     102,   104,   133,   422,   422,   422,   422,   103,     4,    21,
      26,    29,    35,    36,    38,    40,    41,    47,    48,    50,
      57,    72,    76,    79,    98,   153,   155,   166,   171,   176,
     221,   224,   226,   236,   243,   249,   259,   273,   275,   281,
     297,   307,   324,   332,   420,   408,   408,   419,   103,   408,
      11,   103,   405,   425,   101,   408,   408,   408,   407,   407,
     407,   407,   419,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   420,   420,   420,
     408,    12,   114,   145,   149,   103,   419,   138,   411,   419,
     130,   134,   272,   404,   105,   114,   295,   296,   375,   376,
     258,   404,   225,   404,   154,   404,   114,   331,   334,   376,
     165,   404,   220,   404,   170,   404,   237,   404,   242,   404,
     248,   404,   323,   404,   306,   404,   175,   404,    11,   114,
     280,   406,   426,   427,   274,   404,   150,   102,   104,   147,
     408,   103,   102,   104,   417,     8,    71,   103,   135,   138,
     142,   143,   103,   135,   142,   143,   102,   104,   276,   405,
     102,   377,   419,   102,   104,   260,   102,   104,   227,   102,
     104,   156,   102,   419,   102,   104,   167,   102,   104,   222,
     102,   104,   172,   102,   102,   102,   102,   104,   325,   102,
     104,   308,   102,   102,   104,   282,   419,   419,   276,   419,
     408,   408,   408,   419,   419,   419,   420,   408,   408,   408,
     375,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   148,   416,   149,   277,   298,
     261,   228,   157,   333,   168,   223,   173,   238,   244,   250,
     326,   309,   177,   283,   103,   142,   143,   103,   407,    27,
      42,    51,    55,    73,    81,   103,   142,   143,   163,   169,
     351,   354,   356,   366,    25,    96,   103,   142,   143,   163,
     169,   224,   267,   305,   324,   351,    30,   103,   142,   143,
     163,   169,   219,   226,   236,   243,   249,   259,   264,   265,
     267,   273,   275,   351,   354,   356,    65,   103,   142,   143,
     163,   169,   176,   221,   224,   305,   324,   351,   354,   358,
     366,    20,   103,   142,   143,   158,   163,    37,   103,   142,
     143,   103,   142,   143,   163,   169,   103,   142,   143,   163,
     176,   221,   224,   305,   324,    22,   103,   142,   143,   163,
     169,   174,    75,    78,   103,   142,   143,   163,   169,   178,
     218,   219,   351,   354,   356,   366,    52,    53,    58,   103,
     142,   143,   163,   169,   178,   218,   219,   351,   354,   359,
     361,   363,   366,    46,    77,   103,   142,   143,   163,   169,
     176,   221,   224,   305,   324,   351,   354,   359,   361,   363,
     366,   368,   371,   103,   142,   143,   163,   169,   176,   221,
     224,   366,    45,    60,   103,   142,   143,   163,   169,   176,
     221,   314,   321,   103,   142,   143,   163,   178,   218,   219,
      21,    68,   103,   142,   143,   163,   169,   289,   294,   351,
     419,   420,   420,   420,   420,   420,   420,   278,   279,   420,
     420,   300,   303,   301,   302,   299,   420,   263,   262,   420,
     234,   231,   235,   232,   233,   229,   230,   420,   420,   408,
     408,   408,   408,   408,   420,   420,   420,   240,   239,   241,
     420,   420,   420,   246,   245,   247,   420,   420,   253,   254,
     257,   255,   256,   251,   252,   328,   329,   330,   327,   419,
     419,   310,   311,   312,   313,   408,   420,   420,   285,   286,
     284,    88,    93,   114,   355,   422,    88,    93,   114,   357,
     365,   422,    85,    87,    90,   114,   164,   350,   422,   408,
     408,   266,   404,   304,   404,   408,   408,   408,   408,   408,
     422,   408,   408,   422,   408,   408,   408,   408,   408,   408,
     408,   404,    84,    86,    89,    91,   335,   336,   337,   338,
     342,   346,   406,   179,   406,   422,   408,   408,   408,    17,
      94,   114,   362,   399,    18,   114,   360,   399,   400,    92,
      95,   114,   364,   408,   408,   408,   114,   372,   405,   114,
     369,   378,   405,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   315,   322,   408,   408,   408,   408,
     114,   292,   293,   378,   114,   287,   288,   378,   408,   408,
     408,   419,   419,   407,   407,   419,   419,   407,   102,   104,
     367,   419,   419,   419,   407,   102,   104,   352,   102,   104,
     268,   308,   407,   407,   159,   419,   419,   419,   419,   419,
     407,   102,   104,   180,   407,   419,   407,   419,   407,   419,
     419,   419,   407,   407,   373,   407,   370,   407,   420,   379,
     102,   102,   102,   419,   102,   104,   290,   419,   408,   408,
     408,   102,   104,   160,   339,   343,   407,   347,   408,   419,
     378,   377,   408,   408,   408,   408,   189,   353,   269,   408,
     408,   102,   104,   340,   102,   104,   344,   102,   104,   348,
      24,    32,   181,   182,   195,   197,   208,   210,   370,   316,
     316,   298,   291,    33,    34,   103,   142,   143,   216,   217,
     103,   142,   143,   103,   142,   143,   163,   169,   224,   351,
      83,   161,   408,   408,   408,   420,   420,   103,    39,    49,
      61,    62,    66,    69,   174,   183,   184,   186,   190,   203,
     205,   206,   408,   209,   103,   176,   221,   224,   366,   103,
     103,   103,   142,   143,   169,   219,   354,   356,   358,   359,
     361,   366,   420,   420,   271,   270,   420,   103,   341,   345,
     349,   211,   404,   198,   422,   420,   420,   420,   420,   420,
     420,   178,   408,   196,   210,   318,   319,   320,   317,   408,
     422,   422,   408,   408,    88,    93,   114,   162,   103,   218,
     219,   354,   356,   359,   361,   366,   368,   103,   218,   219,
     366,   368,   103,   178,   218,   219,   354,   356,   359,   361,
     102,   104,   212,   102,   104,   199,   114,   185,   399,   187,
     422,   114,   380,   381,   382,   191,   422,   374,   422,    88,
      93,   114,   207,   408,   197,   408,   408,   408,   408,   407,
     407,   419,   419,   407,   408,   408,   408,   408,   408,   408,
     407,   419,   102,   104,   188,   407,   105,   383,   384,    14,
     386,   387,   102,   104,   192,   102,   104,   204,   419,   419,
     407,   408,   213,   200,   408,   408,   405,   385,   105,   388,
     408,   408,   408,    63,   103,   142,   143,   163,   169,   214,
      80,   103,   142,   143,   163,   169,   201,   189,   391,   383,
     387,   389,   405,   193,   189,   420,   420,   103,   106,   390,
     391,    97,   103,   142,   143,   194,   216,   217,   103,   114,
     215,   400,    16,    17,    18,   114,   202,   401,   402,     5,
     421,   384,   420,   407,   419,   407,   419,   392,   405,   422,
     421,   421,   407,   107,   108,   421,    85,   393,   398,   421,
     421,   109,   105,   421,   421,   110,    14,   394,   395,   421,
     396,   105,   395,   405,   421,   397,   405,   421,   105,   421,
     405
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   111,   112,   112,   113,   114,   115,   116,   115,   117,
     118,   119,   120,   120,   120,   120,   121,   122,   123,   124,
     124,   124,   125,   126,   127,   127,   127,   128,   129,   130,
     130,   130,   130,   130,   131,   132,   133,   133,   134,   134,
     134,   134,   135,   136,   137,   138,   139,   139,   139,   139,
     139,   140,   141,   142,   143,   144,   145,   146,   146,   147,
     147,   148,   148,   148,   150,   149,   149,   151,   152,   152,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     154,   155,   156,   156,   157,   157,   157,   157,   157,   159,
     158,   160,   160,   161,   161,   162,   162,   162,   163,   164,
     164,   164,   164,   165,   166,   167,   167,   168,   168,   168,
     168,   168,   169,   170,   171,   172,   172,   173,   173,   173,
     173,   173,   173,   174,   175,   176,   177,   177,   177,   177,
     177,   177,   177,   178,   179,   180,   180,   181,   181,   181,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   183,
     184,   185,   185,   186,   187,   188,   188,   189,   189,   189,
     189,   189,   190,   191,   192,   192,   193,   193,   193,   193,
     193,   193,   194,   195,   196,   196,   197,   198,   199,   199,
     200,   200,   200,   200,   200,   200,   201,   202,   202,   203,
     204,   204,   205,   206,   207,   207,   207,   208,   209,   209,
     210,   211,   212,   212,   213,   213,   213,   213,   213,   213,
     214,   215,   215,   216,   217,   218,   219,   220,   221,   222,
     222,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     224,   224,   224,   224,   224,   224,   224,   224,   225,   226,
     227,   227,   228,   229,   228,   228,   230,   228,   228,   228,
     228,   228,   228,   231,   228,   232,   228,   233,   228,   234,
     228,   235,   228,   236,   237,   238,   239,   238,   238,   240,
     238,   238,   241,   238,   238,   238,   238,   238,   238,   238,
     242,   243,   244,   245,   244,   244,   246,   244,   244,   244,
     247,   244,   244,   244,   244,   244,   244,   244,   244,   248,
     249,   250,   251,   250,   250,   252,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   253,   250,   254,   250,
     255,   250,   256,   250,   257,   250,   258,   259,   260,   260,
     261,   262,   261,   261,   261,   261,   261,   261,   261,   261,
     263,   261,   264,   264,   265,   265,   265,   265,   265,   265,
     265,   266,   267,   268,   268,   269,   270,   269,   269,   269,
     269,   269,   271,   269,   272,   273,   274,   275,   276,   276,
     277,   278,   277,   277,   279,   277,   277,   277,   277,   277,
     277,   280,   281,   282,   282,   283,   284,   283,   283,   283,
     283,   283,   285,   283,   286,   283,   287,   287,   288,   289,
     290,   290,   291,   291,   291,   291,   291,   291,   291,   291,
     291,   291,   291,   292,   292,   293,   294,   295,   295,   296,
     297,   298,   299,   298,   298,   298,   298,   298,   300,   298,
     301,   298,   302,   298,   303,   298,   304,   305,   306,   307,
     308,   308,   309,   309,   309,   309,   309,   310,   309,   311,
     309,   312,   309,   313,   309,   315,   314,   316,   317,   316,
     318,   316,   319,   316,   320,   316,   322,   321,   323,   324,
     325,   325,   326,   327,   326,   326,   326,   326,   326,   328,
     326,   329,   326,   330,   326,   331,   332,   333,   333,   333,
     333,   334,   334,   335,   335,   336,   337,   337,   337,   339,
     338,   340,   340,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   343,   342,   344,   344,   345,   345,   345,   345,
     345,   347,   346,   348,   348,   349,   349,   349,   349,   349,
     349,   349,   349,   350,   351,   352,   352,   353,   353,   353,
     354,   355,   355,   355,   356,   357,   357,   357,   358,   359,
     360,   360,   361,   362,   362,   362,   363,   364,   364,   364,
     365,   366,   367,   367,   368,   369,   369,   370,   370,   371,
     373,   372,   372,   374,   375,   376,   377,   377,   379,   378,
     381,   380,   382,   380,   380,   383,   384,   385,   385,   386,
     387,   388,   388,   389,   390,   390,   391,   391,   392,   393,
     394,   395,   396,   396,   397,   397,   398,   399,   400,   400,
     401,   401,   402,   402,   403,   403,   404,   404,   405,   405,
     406,   406,   406,   407,   407,   408,   408,   408,   410,   409,
     411,   412,   412,   413,   413,   413,   414,   414,   415,   415,
     416,   416,   417,   417,   418,   418,   419,   419,   420,   421,
     421,   423,   422,   422,   424,   424,   424,   424,   424,   424,
     424,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   425,   425,   425,   425,   425,   425,   425,   425,
     425,   425,   426,   427
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     0,     0,     6,     1,
      13,     1,     0,     2,     2,     2,     1,    13,     1,     0,
       2,     2,     4,     4,     0,     2,     2,     8,     1,     0,
       2,     2,     2,     2,     1,     5,     1,     4,     0,     2,
       2,     2,     4,     1,     8,     4,     0,     2,     2,     2,
       2,     4,     4,     4,     4,     1,     1,     0,     6,     1,
       4,     0,     2,     2,     0,     3,     1,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     4,     0,     2,     2,     2,     2,     0,
       6,     1,     4,     0,     4,     2,     2,     1,     4,     2,
       2,     2,     1,     1,     4,     1,     4,     0,     2,     2,
       2,     2,     4,     1,     4,     1,     4,     0,     2,     2,
       2,     2,     2,     4,     1,     7,     0,     3,     2,     2,
       2,     2,     2,     4,     1,     1,     4,     1,     1,     1,
       0,     2,     2,     2,     2,     2,     2,     3,     4,     0,
       4,     2,     1,     5,     1,     1,     4,     0,     2,     2,
       2,     2,     5,     1,     1,     4,     0,     2,     2,     2,
       2,     2,     4,     3,     0,     3,     4,     1,     1,     4,
       0,     2,     2,     2,     2,     2,     4,     2,     1,     4,
       1,     4,     4,     4,     2,     2,     1,     2,     0,     2,
       5,     1,     1,     4,     0,     2,     2,     2,     2,     2,
       4,     2,     1,     4,     4,     4,     4,     1,     4,     1,
       4,     0,     2,     2,     2,     3,     3,     3,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     4,     0,     0,     4,     2,     0,     4,     2,     2,
       2,     2,     2,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     7,     1,     0,     0,     4,     2,     0,
       4,     2,     0,     4,     2,     2,     2,     2,     2,     2,
       1,     7,     0,     0,     4,     2,     0,     4,     2,     2,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     1,
       7,     0,     0,     4,     2,     0,     4,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     1,     4,     1,     4,
       0,     0,     4,     2,     2,     2,     2,     2,     2,     2,
       0,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     1,     4,     1,     4,     1,     4,
       0,     0,     4,     2,     0,     4,     2,     2,     2,     2,
       2,     1,     4,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     4,     2,     1,     1,     4,
       1,     4,     0,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     7,     2,     1,     1,
       7,     0,     0,     4,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     4,     1,     4,     1,     4,
       1,     4,     0,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     7,     0,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     7,     1,     4,
       1,     4,     0,     0,     4,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     4,     1,     7,     0,     2,     2,
       4,     2,     1,     1,     2,     3,     1,     1,     1,     0,
       4,     1,     4,     0,     2,     3,     2,     2,     2,     2,
       2,     2,     0,     4,     1,     4,     0,     2,     3,     2,
       2,     0,     4,     1,     4,     0,     3,     2,     2,     2,
       2,     2,     2,     1,     4,     1,     4,     0,     2,     2,
       4,     2,     2,     1,     4,     2,     2,     1,     4,     4,
       2,     1,     4,     2,     2,     1,     4,     2,     2,     1,
       1,     4,     1,     4,     3,     2,     2,     3,     1,     4,
       0,     3,     1,     1,     2,     2,     0,     2,     0,     3,
       0,     2,     0,     2,     1,     3,     2,     0,     2,     3,
       2,     0,     2,     2,     0,     2,     0,     6,     5,     5,
       5,     4,     0,     2,     0,     5,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     1,     1,
       2,     2,     1,     2,     4,     0,     2,     2,     0,     4,
       2,     0,     1,     0,     2,     3,     0,     5,     1,     4,
       0,     4,     2,     5,     1,     1,     0,     2,     2,     0,
       1,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
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
          case 113: /* tmp_string  */

      { free((((*yyvaluep).p_str)) ? *((*yyvaluep).p_str) : NULL); }

        break;

    case 191: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

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
char *s = NULL, *tmp_s = NULL;
struct lys_module *trg = NULL;
struct lys_node *tpdf_parent = NULL, *data_node = NULL;
void *actual = NULL;
int config_inherit = 0, actual_type = 0;
int64_t cnt_val = 0;
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
                  param->value = &s;
                  param->data_node = (void **)&data_node;
                  param->actual_node = &actual;
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
        case 4:

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

  case 7:

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

  case 9:

    { if (param->submodule) {
                                       free(s);
                                       LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "module");
                                       YYABORT;
                                     }
                                     trg = param->module;
                                     yang_read_common(trg,s,MODULE_KEYWORD);
                                     s = NULL;
                                     config_inherit = CONFIG_INHERIT_ENABLE;
                                   }

    break;

  case 11:

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

  case 12:

    { (yyval.i) = 0; }

    break;

  case 13:

    { if (yang_check_version(param->module, param->submodule, s, (yyvsp[-1].i))) {
                                              YYABORT;
                                            }
                                            (yyval.i) = 1;
                                            s = NULL;
                                          }

    break;

  case 14:

    { if (yang_read_common(param->module, s, NAMESPACE_KEYWORD)) {
                                           YYABORT;
                                         }
                                         s=NULL;
                                       }

    break;

  case 16:

    { if (!param->submodule) {
                                          free(s);
                                          LOGVAL(LYE_SUBMODULE, LY_VLOG_NONE, NULL);
                                          YYABORT;
                                        }
                                        trg = (struct lys_module *)param->submodule;
                                        yang_read_common(trg,s,MODULE_KEYWORD);
                                        s = NULL;
                                        config_inherit = CONFIG_INHERIT_ENABLE;
                                      }

    break;

  case 18:

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

  case 19:

    { (yyval.i) = 0; }

    break;

  case 20:

    { if (yang_check_version(param->module, param->submodule, s, (yyvsp[-1].i))) {
                                                 YYABORT;
                                               }
                                               (yyval.i) = 1;
                                               s = NULL;
                                             }

    break;

  case 28:

    { YANG_ADDELEM(trg->imp, trg->imp_size);
                                     /* HACK for unres */
                                     ((struct lys_import *)actual)->module = (struct lys_module *)s;
                                     s = NULL;
                                     actual_type = IMPORT_KEYWORD;
                                   }

    break;

  case 29:

    { (yyval.i) = 0; }

    break;

  case 31:

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

  case 32:

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

  case 33:

    { if ((yyvsp[-1].i)) {
                                            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "import");
                                            YYABORT;
                                          }
                                          (yyval.i) = 1;
                                        }

    break;

  case 34:

    { YANG_ADDELEM(trg->inc, trg->inc_size);
                                     /* HACK for unres */
                                     ((struct lys_include *)actual)->submodule = (struct lys_submodule *)s;
                                     s = NULL;
                                     actual_type = INCLUDE_KEYWORD;
                                   }

    break;

  case 38:

    { (yyval.i) = 0; }

    break;

  case 39:

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

  case 40:

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

  case 41:

    { if ((yyvsp[-1].i)) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "include");
                                             YYABORT;
                                           }
                                           (yyval.i) = 1;
                                         }

    break;

  case 42:

    { if (actual_type==IMPORT_KEYWORD) {
                                memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                              } else {                              // INCLUDE KEYWORD
                                memcpy(((struct lys_include *)actual)->rev, s, LY_REV_SIZE-1);
                              }
                              free(s);
                              s = NULL;
                            }

    break;

  case 43:

    { if (param->submodule->prefix) {
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
                                         s = NULL;
                                       }

    break;

  case 45:

    { if (yang_read_prefix(trg, actual, s)) {
                                                           YYABORT;
                                                         }
                                                         s = NULL;
                                                       }

    break;

  case 47:

    { if (yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 48:

    { if (yang_read_common(trg, s, CONTACT_KEYWORD)) {
                                 YYABORT;
                               }
                               s = NULL;
                             }

    break;

  case 49:

    { if (yang_read_description(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 50:

    { if (yang_read_reference(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 55:

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

  case 56:

    { YANG_ADDELEM(trg->rev, trg->rev_size);
                                  yang_read_revision(trg, s, actual);
                                }

    break;

  case 62:

    { if (yang_read_description(trg, actual, s, "revision",REVISION_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 63:

    { if (yang_read_reference(trg, actual, s, "revision", REVISION_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 64:

    { s = strdup(yyget_text(scanner));
                              if (!s) {
                                LOGMEM;
                                YYABORT;
                              }
                            }

    break;

  case 66:

    { if (lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 67:

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
                           }

    break;

  case 68:

    { /* check the module with respect to the context now */
                         if (!param->submodule) {
                           switch (lyp_ctx_check_module(trg)) {
                           case -1:
                             YYABORT;
                           case 0:
                             break;
                           case 1:
                             /* it's already there */
                             param->exist_module = 1;
                             YYABORT;
                           }
                         }
                         param->remove_import = 0;
                         if (yang_check_imports(trg, param->unres)) {
                           YYABORT;
                         }
                         actual = NULL;
                       }

    break;

  case 69:

    { actual = NULL; }

    break;

  case 80:

    { /* we have the following supported (hardcoded) extensions: */
                                        /* ietf-netconf's get-filter-element-attributes */
                                        if (!strcmp(param->module->ns, LY_NSNC) && !strcmp(s, "get-filter-element-attributes")) {
                                          LOGDBG("NETCONF filter extension found");
                                          /* NACM's default-deny-write and default-deny-all */
                                        } else if (!strcmp(param->module->ns, LY_NSNACM) &&
                                                   (!strcmp(s, "default-deny-write") || !strcmp(s, "default-deny-all"))) {
                                          LOGDBG("NACM extension found");
                                          /* other extensions are not supported, so inform about such an extension */
                                        } else {
                                          LOGWRN("Not supported \"%s\" extension statement found, ignoring.", s);
                                        }
                                        free(s);
                                        s = NULL;
                                      }

    break;

  case 84:

    { (yyval.uint) = 0; }

    break;

  case 85:

    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }

    break;

  case 86:

    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }

    break;

  case 87:

    { free(s);
                                           s= NULL;
                                           if ((yyvsp[-1].uint) & EXTENSION_DSC) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "description", "extension");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].uint) |= EXTENSION_DSC;
                                           (yyval.uint) = (yyvsp[-1].uint);
                                         }

    break;

  case 88:

    { free(s);
                                         s = NULL;
                                         if ((yyvsp[-1].uint) & EXTENSION_REF) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "reference", "extension");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].uint) |= EXTENSION_REF;
                                         (yyval.uint) = (yyvsp[-1].uint);
                                       }

    break;

  case 89:

    { free(s); s = NULL; }

    break;

  case 97:

    { if (strcmp(s, "true") && strcmp(s, "false")) {
                 LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                 free(s);
                 YYABORT;
               }
               free(s);
               s = NULL;
             }

    break;

  case 98:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 99:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 100:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 101:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 102:

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

  case 103:

    { /* check uniqueness of feature's names */
                                      if (lyp_check_identifier(s, LY_IDENT_FEATURE, trg, NULL)) {
                                        free(s);
                                        YYABORT;
                                      }
                                      YANG_ADDELEM(trg->features, trg->features_size);
                                      ((struct lys_feature *)actual)->name = lydict_insert_zc(trg->ctx, s);
                                      ((struct lys_feature *)actual)->module = trg;
                                      s = NULL;
                                    }

    break;

  case 106:

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

  case 108:

    { void *feature;

                                        feature = actual;
                                        YANG_ADDELEM(((struct lys_feature *)actual)->iffeature,
                                                     ((struct lys_feature *)actual)->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                                        s = NULL;
                                        actual = feature;
                                      }

    break;

  case 109:

    { if (((struct lys_feature *)actual)->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "feature");
                                      YYABORT;
                                    }
                                    ((struct lys_feature *)actual)->flags |= (yyvsp[0].i);
                                  }

    break;

  case 110:

    { if (yang_read_description(trg, actual, s, "feature", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 111:

    { if (yang_read_reference(trg, actual, s, "feature", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 113:

    { const char *tmp;

                                       tmp = lydict_insert_zc(trg->ctx, s);
                                       s = NULL;
                                       if (dup_identities_check(tmp, trg)) {
                                         lydict_remove(trg->ctx, tmp);
                                         YYABORT;
                                       }
                                       YANG_ADDELEM(trg->ident, trg->ident_size);
                                       ((struct lys_ident *)actual)->name = tmp;
                                       ((struct lys_ident *)actual)->module = trg;
                                     }

    break;

  case 116:

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

  case 118:

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

  case 119:

    { void *identity;

                                         if (trg->version < 2) {
                                           LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "identity");
                                           YYABORT;
                                         }
                                         identity = actual;
                                         YANG_ADDELEM(((struct lys_ident *)actual)->iffeature,
                                                      ((struct lys_ident *)actual)->iffeature_size);
                                         ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                                         s = NULL;
                                         actual = identity;
                                       }

    break;

  case 120:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 121:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 122:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 124:

    { tpdf_parent = actual;
                                      if (lyp_check_identifier(s, LY_IDENT_TYPE, trg, tpdf_parent)) {
                                        free(s);
                                        YYABORT;
                                      }
                                      if (!tpdf_parent) {
                                        YANG_ADDELEM(trg->tpdf, trg->tpdf_size);
                                      } else {
                                        switch (tpdf_parent->nodetype) {
                                        case LYS_GROUPING:
                                          YANG_ADDELEM(((struct lys_node_grp *)tpdf_parent)->tpdf,
                                                       ((struct lys_node_grp *)tpdf_parent)->tpdf_size);
                                          break;
                                        case LYS_CONTAINER:
                                          YANG_ADDELEM(((struct lys_node_container *)tpdf_parent)->tpdf,
                                                       ((struct lys_node_container *)tpdf_parent)->tpdf_size);
                                          break;
                                        case LYS_LIST:
                                          YANG_ADDELEM(((struct lys_node_list *)tpdf_parent)->tpdf,
                                                       ((struct lys_node_list *)tpdf_parent)->tpdf_size);
                                          break;
                                        case LYS_RPC:
                                        case LYS_ACTION:
                                          YANG_ADDELEM(((struct lys_node_rpc_action *)tpdf_parent)->tpdf,
                                                       ((struct lys_node_rpc_action *)tpdf_parent)->tpdf_size);
                                          break;
                                        case LYS_INPUT:
                                        case LYS_OUTPUT:
                                          YANG_ADDELEM(((struct lys_node_inout *)tpdf_parent)->tpdf,
                                                       ((struct lys_node_inout *)tpdf_parent)->tpdf_size);
                                          break;
                                        case LYS_NOTIF:
                                          YANG_ADDELEM(((struct lys_node_notif *)tpdf_parent)->tpdf,
                                                       ((struct lys_node_notif *)tpdf_parent)->tpdf_size);
                                          break;
                                        default:
                                          /* another type of nodetype is error*/
                                          LOGINT;
                                          free(s);
                                          YYABORT;
                                        }
                                      }
                                      ((struct lys_tpdf *)actual)->name = lydict_insert_zc(param->module->ctx, s);
                                      ((struct lys_tpdf *)actual)->module = trg;
                                      s = NULL;
                                      actual_type = TYPEDEF_KEYWORD;
                                      (yyval.nodes).node.ptr_tpdf = actual;
                                    }

    break;

  case 125:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual = tpdf_parent;
                  }

    break;

  case 126:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 127:

    { actual = (yyvsp[-2].nodes).node.ptr_tpdf;
                                       actual_type = TYPEDEF_KEYWORD;
                                       (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 128:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 129:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 130:

    { if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 131:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 132:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 134:

    { if (!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 137:

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

  case 141:

    { if (yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 142:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 143:

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

  case 148:

    { actual = (yyvsp[-2].v); }

    break;

  case 149:

    { struct yang_type *stype = (struct yang_type *)actual;

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
                           (yyval.v) = actual;
                           YANG_ADDELEM(stype->type->info.uni.types, stype->type->info.uni.count)
                           actual_type = UNION_KEYWORD;
                         }

    break;

  case 150:

    { if (yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 151:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 152:

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

  case 153:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 154:

    { (yyval.v) = actual;
                         if (!(actual = yang_read_length(trg, actual, s))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 157:

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

  case 158:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 159:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 160:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 161:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 162:

    { if (yang_read_pattern(trg, actual, (yyvsp[-2].str), (yyvsp[-1].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual = yang_type;
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 163:

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

  case 164:

    { (yyval.ch) = 0x06; }

    break;

  case 165:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 166:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 167:

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

  case 168:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 169:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 170:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 171:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 172:

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

  case 173:

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

  case 176:

    { if (yang_check_enum(yang_type, actual, &cnt_val, actual_type)) {
               YYABORT;
             }
             actual = yang_type;
             actual_type = TYPE_KEYWORD;
           }

    break;

  case 177:

    { yang_type = actual;
                       YANG_ADDELEM(((struct yang_type *)actual)->type->info.enums.enm, ((struct yang_type *)actual)->type->info.enums.count);
                       if (yang_read_enum(trg, yang_type, actual, s)) {
                         YYABORT;
                       }
                       s = NULL;
                       actual_type = 0;
                     }

    break;

  case 179:

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

  case 181:

    { if (trg->version < 2) {
                                       LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                                       free(s);
                                       YYABORT;
                                     }
                                     (yyval.v) = actual;
                                     YANG_ADDELEM(((struct lys_type_enum *)actual)->iffeature,
                                                  ((struct lys_type_enum *)actual)->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyval.v);
                                     s = NULL;
                                   }

    break;

  case 182:

    { /* actual_type - it is used to check value of enum statement*/
                                if (actual_type) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                actual_type = 1;
                              }

    break;

  case 183:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 184:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 185:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 186:

    { ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                      /* keep the highest enum value for automatic increment */
                      if ((yyvsp[-1].i) >= cnt_val) {
                        cnt_val = (yyvsp[-1].i);
                        cnt_val++;
                      }
                    }

    break;

  case 187:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 188:

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

  case 189:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 193:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 194:

    { (yyval.i) = 1; }

    break;

  case 195:

    { (yyval.i) = -1; }

    break;

  case 196:

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

  case 197:

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

  case 200:

    { if (yang_check_bit(yang_type, actual, &cnt_val, actual_type)) {
                      YYABORT;
                    }
                    actual = yang_type;
                    actual_type = TYPE_KEYWORD;
                  }

    break;

  case 201:

    { yang_type = actual;
                                  YANG_ADDELEM(((struct yang_type *)actual)->type->info.bits.bit,
                                               ((struct yang_type *)actual)->type->info.bits.count);
                                  if (yang_read_bit(trg, yang_type, actual, s)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  actual_type = 0;
                                }

    break;

  case 203:

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

  case 205:

    { if (trg->version < 2) {
                                      LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "bit");
                                      free(s);
                                      YYABORT;
                                    }
                                    (yyval.v) = actual;
                                    YANG_ADDELEM(((struct lys_type_bit *)actual)->iffeature,
                                                 ((struct lys_type_bit *)actual)->iffeature_size);
                                    ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                    actual = (yyval.v);
                                    s = NULL;
                                  }

    break;

  case 206:

    { /* actual_type - it is used to check position of bit statement*/
                                  if (actual_type) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  actual_type = 1;
                                }

    break;

  case 207:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags,
                                                     LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                  YYABORT;
                                }
                              }

    break;

  case 208:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 209:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 210:

    { ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                         /* keep the highest position value for automatic increment */
                         if ((yyvsp[-1].uint) >= cnt_val) {
                           cnt_val = (yyvsp[-1].uint);
                           cnt_val++;
                         }
                       }

    break;

  case 211:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 212:

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

  case 217:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_GROUPING, sizeof(struct lys_node_grp)))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       data_node = actual;
                                     }

    break;

  case 218:

    { LOGDBG("YANG: finished parsing grouping statement \"%s\"", data_node->name); }

    break;

  case 221:

    { (yyval.nodes).grouping = actual;
                                actual_type = GROUPING_KEYWORD;
                              }

    break;

  case 222:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 223:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 224:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 225:

    { actual = (yyvsp[-2].nodes).grouping;
                                               actual_type = GROUPING_KEYWORD;
                                             }

    break;

  case 226:

    { actual = (yyvsp[-2].nodes).grouping;
                                              actual_type = GROUPING_KEYWORD;
                                            }

    break;

  case 227:

    { actual = (yyvsp[-2].nodes).grouping;
                                               actual_type = GROUPING_KEYWORD;
                                             }

    break;

  case 228:

    { actual = (yyvsp[-2].nodes).grouping;
                                             actual_type = GROUPING_KEYWORD;
                                           }

    break;

  case 229:

    { actual = (yyvsp[-2].nodes).grouping;
                                                   actual_type = GROUPING_KEYWORD;
                                                   if (trg->version < 2) {
                                                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 238:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CONTAINER, sizeof(struct lys_node_container)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                      }

    break;

  case 239:

    { LOGDBG("YANG: finished parsing container statement \"%s\"", data_node->name); }

    break;

  case 241:

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

  case 242:

    { (yyval.nodes).container = actual;
                                 actual_type = CONTAINER_KEYWORD;
                               }

    break;

  case 243:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 245:

    { YANG_ADDELEM((yyvsp[-1].nodes).container->iffeature, (yyvsp[-1].nodes).container->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).container;
                                          s = NULL;
                                        }

    break;

  case 246:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 248:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 249:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 250:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 251:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 252:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 253:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 255:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 257:

    { actual = (yyvsp[-1].nodes).container;
                                            actual_type = CONTAINER_KEYWORD;
                                            data_node = actual;
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "notification");
                                              YYABORT;
                                            }
                                          }

    break;

  case 259:

    { actual = (yyvsp[-1].nodes).container;
                                       actual_type = CONTAINER_KEYWORD;
                                     }

    break;

  case 261:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 263:

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
                }

    break;

  case 264:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAF, sizeof(struct lys_node_leaf)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   (yyval.nodes).node.ptr_leaf = actual;
                                 }

    break;

  case 265:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LEAF_KEYWORD;
                          }

    break;

  case 266:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 268:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaf->iffeature, (yyvsp[-1].nodes).node.ptr_leaf->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                     s = NULL;
                                   }

    break;

  case 269:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                               (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                             }

    break;

  case 270:

    { (yyval.nodes) = (yyvsp[-3].nodes);}

    break;

  case 271:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 272:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 274:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 275:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 276:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 277:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 278:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 279:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 280:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAFLIST, sizeof(struct lys_node_leaflist)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                      }

    break;

  case 281:

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
                      }

    break;

  case 282:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                                 actual_type = LEAF_LIST_KEYWORD;
                               }

    break;

  case 283:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 285:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaflist->iffeature,
                                                       (yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                          s = NULL;
                                        }

    break;

  case 286:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                                  }

    break;

  case 287:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 288:

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

  case 289:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 290:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 292:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 293:

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

  case 294:

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

  case 295:

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

  case 296:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 297:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 298:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 299:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LIST, sizeof(struct lys_node_list)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 300:

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
                }

    break;

  case 301:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LIST_KEYWORD;
                          }

    break;

  case 302:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 304:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->iffeature,
                                                  (yyvsp[-1].nodes).node.ptr_list->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_list;
                                     s = NULL;
                                   }

    break;

  case 305:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 307:

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

  case 308:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 309:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 310:

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

  case 311:

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

  case 312:

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

  case 313:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 314:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 315:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 316:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                  actual_type = LIST_KEYWORD;
                                }

    break;

  case 318:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 320:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 322:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                       actual_type = LIST_KEYWORD;
                                       data_node = actual;
                                       if (trg->version < 2) {
                                         LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "notification");
                                         YYABORT;
                                       }
                                     }

    break;

  case 324:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 325:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 326:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CHOICE, sizeof(struct lys_node_choice)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                   }

    break;

  case 327:

    { LOGDBG("YANG: finished parsing choice statement \"%s\"", data_node->name); }

    break;

  case 329:

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

  case 330:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                              actual_type = CHOICE_KEYWORD;
                            }

    break;

  case 331:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                 actual_type = CHOICE_KEYWORD;
                               }

    break;

  case 332:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 333:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_choice->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_choice->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_choice;
                                       s = NULL;
                                     }

    break;

  case 334:

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

  case 335:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 336:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 337:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 338:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 339:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 340:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                          }

    break;

  case 341:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 350:

    { if (trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 351:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CASE, sizeof(struct lys_node_case)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 352:

    { LOGDBG("YANG: finished parsing case statement \"%s\"", data_node->name); }

    break;

  case 354:

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

  case 355:

    { (yyval.nodes).cs = actual;
                            actual_type = CASE_KEYWORD;
                          }

    break;

  case 356:

    { actual = (yyvsp[-1].nodes).cs;
                               actual_type = CASE_KEYWORD;
                             }

    break;

  case 358:

    { YANG_ADDELEM((yyvsp[-1].nodes).cs->iffeature, (yyvsp[-1].nodes).cs->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).cs;
                                     s = NULL;
                                   }

    break;

  case 359:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 360:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 361:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
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

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYXML, sizeof(struct lys_node_anydata)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ANYXML_KEYWORD;
                                   }

    break;

  case 365:

    { LOGDBG("YANG: finished parsing anyxml statement \"%s\"", data_node->name); }

    break;

  case 366:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYDATA, sizeof(struct lys_node_anydata)))) {
                                        YYABORT;
                                      }
                                      data_node = actual;
                                      s = NULL;
                                      actual_type = ANYDATA_KEYWORD;
                                    }

    break;

  case 367:

    { LOGDBG("YANG: finished parsing anydata statement \"%s\"", data_node->name); }

    break;

  case 369:

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

  case 370:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
                            }

    break;

  case 371:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 373:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_anydata->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_anydata->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                       s = NULL;
                                     }

    break;

  case 374:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 376:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "config",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 377:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_MAND_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "mandatory",
                                               ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                    }

    break;

  case 378:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "status",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 379:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 380:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 381:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_USES, sizeof(struct lys_node_uses)))) {
                                         YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                     }

    break;

  case 382:

    { LOGDBG("YANG: finished parsing uses statement \"%s\"", data_node->name); }

    break;

  case 384:

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

  case 385:

    { (yyval.nodes).uses = actual;
                            actual_type = USES_KEYWORD;
                          }

    break;

  case 386:

    { actual = (yyvsp[-1].nodes).uses;
                               actual_type = USES_KEYWORD;
                             }

    break;

  case 388:

    { YANG_ADDELEM((yyvsp[-1].nodes).uses->iffeature, (yyvsp[-1].nodes).uses->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).uses;
                                     s = NULL;
                                   }

    break;

  case 389:

    { if ((yyvsp[-1].nodes).uses->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).uses, "status", "uses");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).uses->flags |= (yyvsp[0].i);
                               }

    break;

  case 390:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 391:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 392:

    { actual = (yyvsp[-1].nodes).uses;
                                 actual_type = USES_KEYWORD;
                               }

    break;

  case 394:

    { actual = (yyvsp[-1].nodes).uses;
                                       actual_type = USES_KEYWORD;
                                       data_node = actual;
                                     }

    break;

  case 398:

    { YANG_ADDELEM(((struct lys_node_uses *)actual)->refine,
                                               ((struct lys_node_uses *)actual)->refine_size);
                                  ((struct lys_refine *)actual)->target_name = transform_schema2json(trg, s);
                                  free(s);
                                  s = NULL;
                                  if (!((struct lys_refine *)actual)->target_name) {
                                    YYABORT;
                                  }
                                }

    break;

  case 401:

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

  case 402:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 403:

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

  case 404:

    { if (trg->version < 2) {
                                               free(s);
                                               LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                                               YYABORT;
                                             }
                                             YANG_ADDELEM((yyvsp[-1].nodes).refine->iffeature, (yyvsp[-1].nodes).refine->iffeature_size);
                                             ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                             actual = (yyvsp[-1].nodes).refine;
                                             s = NULL;
                                             /* leaf, leaf-list, list, container or anyxml */
                                             /* check possibility of statements combination */
                                             if ((yyvsp[-1].nodes).refine->target_type) {
                                               if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA)) {
                                                 (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA);
                                               } else {
                                                 free(s);
                                                 LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "if-feature", "refine");
                                                 LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                 YYABORT;
                                               }
                                             } else {
                                               (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA;
                                             }
                                           }

    break;

  case 405:

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

  case 406:

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

  case 407:

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

  case 408:

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

  case 409:

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

  case 410:

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

  case 411:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 412:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 415:

    { void *parent;

                                         parent = actual;
                                         YANG_ADDELEM(((struct lys_node_uses *)actual)->augment,
                                                      ((struct lys_node_uses *)actual)->augment_size);
                                         if (yang_read_augment(trg, parent, actual, s)) {
                                           YYABORT;
                                         }
                                         data_node = actual;
                                         s = NULL;
                                       }

    break;

  case 419:

    { YANG_ADDELEM(trg->augment, trg->augment_size);
                               if (yang_read_augment(trg, NULL, actual, s)) {
                                 YYABORT;
                               }
                               data_node = actual;
                               s = NULL;
                             }

    break;

  case 421:

    { (yyval.nodes).augment = actual;
                               actual_type = AUGMENT_KEYWORD;
                             }

    break;

  case 422:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                }

    break;

  case 424:

    { YANG_ADDELEM((yyvsp[-1].nodes).augment->iffeature, (yyvsp[-1].nodes).augment->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                        actual = (yyvsp[-1].nodes).augment;
                                        s = NULL;
                                      }

    break;

  case 425:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 426:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 427:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 428:

    { actual = (yyvsp[-1].nodes).augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 430:

    { actual = (yyvsp[-1].nodes).augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    data_node = actual;
                                  }

    break;

  case 432:

    { actual = (yyvsp[-1].nodes).augment;
                                          actual_type = AUGMENT_KEYWORD;
                                          data_node = actual;
                                          if (trg->version < 2) {
                                            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "notification");
                                            YYABORT;
                                          }
                                        }

    break;

  case 434:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 436:

    { if (param->module->version != 2) {
                                       LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "action");
                                       free(s);
                                       YYABORT;
                                     }
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ACTION, sizeof(struct lys_node_rpc_action)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     (yyval.i) = config_inherit;
                                     config_inherit = CONFIG_IGNORE;
                                   }

    break;

  case 437:

    { config_inherit = (yyvsp[-1].i);
                                                         LOGDBG("YANG: finished parsing action statement \"%s\"", data_node->name);
                                                       }

    break;

  case 438:

    { if (!(actual = yang_read_node(trg, NULL, param->node, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                  (yyval.i) = config_inherit;
                                  config_inherit = CONFIG_IGNORE;
                                }

    break;

  case 439:

    { config_inherit = (yyvsp[-1].i);
                                                LOGDBG("YANG: finished parsing rpc statement \"%s\"", data_node->name);
                                              }

    break;

  case 441:

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

  case 442:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                           actual_type = RPC_KEYWORD;
                         }

    break;

  case 443:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_rpc->iffeature,
                                                 (yyvsp[-1].nodes).node.ptr_rpc->iffeature_size);
                                    ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                    actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                    s = NULL;
                                  }

    break;

  case 444:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 445:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 446:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 447:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                 actual_type = RPC_KEYWORD;
                               }

    break;

  case 449:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 451:

    { if ((yyvsp[-1].nodes).node.flag & LYS_RPC_INPUT) {
                                 LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "input", "rpc");
                                 YYABORT;
                               }
                               (yyvsp[-1].nodes).node.flag |= LYS_RPC_INPUT;
                               actual = (yyvsp[-1].nodes).node.ptr_rpc;
                               actual_type = RPC_KEYWORD;
                               data_node = actual;
                             }

    break;

  case 452:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 453:

    { if ((yyvsp[-1].nodes).node.flag & LYS_RPC_OUTPUT) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "output", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.flag |= LYS_RPC_OUTPUT;
                                actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                actual_type = RPC_KEYWORD;
                                data_node = actual;
                              }

    break;

  case 454:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 455:

    { s = strdup("input");
                                   if (!s) {
                                     LOGMEM;
                                     YYABORT;
                                   }
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_INPUT, sizeof(struct lys_node_inout)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 456:

    { void *tmp;

                  if ((yyvsp[-1].nodes).inout->must_size) {
                    tmp = realloc((yyvsp[-1].nodes).inout->must, (yyvsp[-1].nodes).inout->must_size * sizeof *(yyvsp[-1].nodes).inout->must);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).inout->must = tmp;
                  }

                  if ((yyvsp[-1].nodes).inout->tpdf_size) {
                    tmp = realloc((yyvsp[-1].nodes).inout->tpdf, (yyvsp[-1].nodes).inout->tpdf_size * sizeof *(yyvsp[-1].nodes).inout->tpdf);
                    if (!tmp) {
                      LOGMEM;
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).inout->tpdf = tmp;
                  }

                  LOGDBG("YANG: finished parsing input statement \"%s\"", data_node->name);
                }

    break;

  case 457:

    { (yyval.nodes).inout = actual;
                                    actual_type = INPUT_KEYWORD;
                                  }

    break;

  case 458:

    { actual = (yyvsp[-1].nodes).inout;
                                       actual_type = INPUT_KEYWORD;
                                     }

    break;

  case 460:

    { actual = (yyvsp[-1].nodes).inout;
                                          actual_type = INPUT_KEYWORD;
                                        }

    break;

  case 462:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 464:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 466:

    { s = strdup("output");
                                     if (!s) {
                                       LOGMEM;
                                       YYABORT;
                                     }
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_OUTPUT, sizeof(struct lys_node_inout)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                   }

    break;

  case 467:

    { void *tmp;

                   if ((yyvsp[-1].nodes).inout->must_size) {
                     tmp = realloc((yyvsp[-1].nodes).inout->must, (yyvsp[-1].nodes).inout->must_size * sizeof *(yyvsp[-1].nodes).inout->must);
                     if (!tmp) {
                       LOGMEM;
                       YYABORT;
                     }
                     (yyvsp[-1].nodes).inout->must = tmp;
                   }

                   if ((yyvsp[-1].nodes).inout->tpdf_size) {
                     tmp = realloc((yyvsp[-1].nodes).inout->tpdf, (yyvsp[-1].nodes).inout->tpdf_size * sizeof *(yyvsp[-1].nodes).inout->tpdf);
                     if (!tmp) {
                       LOGMEM;
                       YYABORT;
                     }
                     (yyvsp[-1].nodes).inout->tpdf = tmp;
                   }

                   LOGDBG("YANG: finished parsing output statement \"%s\"", data_node->name);
                 }

    break;

  case 468:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                           (yyval.i) = config_inherit;
                                           config_inherit = CONFIG_INHERIT_DISABLE;
                                         }

    break;

  case 469:

    { config_inherit = (yyvsp[-1].i); }

    break;

  case 471:

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

           LOGDBG("YANG: finished parsing notification statement \"%s\"", data_node->name);
          }

    break;

  case 472:

    { (yyval.nodes).notif = actual;
                                    actual_type = NOTIFICATION_KEYWORD;
                                  }

    break;

  case 473:

    { actual = (yyvsp[-1].nodes).notif;
                                       actual_type = NOTIFICATION_KEYWORD;
                                     }

    break;

  case 475:

    { YANG_ADDELEM((yyvsp[-1].nodes).notif->iffeature, (yyvsp[-1].nodes).notif->iffeature_size);
                                             ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                             actual = (yyvsp[-1].nodes).notif;
                                             s = NULL;
                                           }

    break;

  case 476:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 477:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 478:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 479:

    { actual = (yyvsp[-1].nodes).notif;
                                          actual_type = NOTIFICATION_KEYWORD;
                                        }

    break;

  case 481:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 483:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 485:

    { YANG_ADDELEM(trg->deviation, trg->deviation_size);
                                   ((struct lys_deviation *)actual)->target_name = transform_schema2json(trg, s);
                                   free(s);
                                   if (!((struct lys_deviation *)actual)->target_name) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 486:

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
                    }

    break;

  case 487:

    { (yyval.dev) = actual;
                                 actual_type = DEVIATION_KEYWORD;
                               }

    break;

  case 488:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 489:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 490:

    { actual = (yyvsp[-3].dev);
                                                                actual_type = DEVIATION_KEYWORD;
                                                                (yyval.dev) = (yyvsp[-3].dev);
                                                              }

    break;

  case 493:

    { if (yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 499:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_ADD))) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 502:

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

  case 503:

    { (yyval.deviate) = actual;
                                   actual_type = ADD_KEYWORD;
                                 }

    break;

  case 504:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 505:

    { actual = (yyvsp[-2].deviate);
                                              actual_type = ADD_KEYWORD;
                                              (yyval.deviate) = (yyvsp[-2].deviate);
                                            }

    break;

  case 506:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 507:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 508:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 509:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 510:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 511:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 512:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_DEL))) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 515:

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

  case 516:

    { (yyval.deviate) = actual;
                                      actual_type = DELETE_KEYWORD;
                                    }

    break;

  case 517:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 518:

    { actual = (yyvsp[-2].deviate);
                                                 actual_type = DELETE_KEYWORD;
                                                 (yyval.deviate) = (yyvsp[-2].deviate);
                                               }

    break;

  case 519:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 520:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 521:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_RPL))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 524:

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

  case 525:

    { (yyval.deviate) = actual;
                                       actual_type = REPLACE_KEYWORD;
                                     }

    break;

  case 526:

    { actual = (yyvsp[-2].deviate);
                                                  actual_type = REPLACE_KEYWORD;
                                                }

    break;

  case 527:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 528:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 529:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 530:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 531:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 532:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 533:

    { if (!(actual=yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type=WHEN_KEYWORD;
                      }

    break;

  case 538:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 539:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 540:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 541:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 542:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 543:

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

  case 544:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 545:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 546:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 547:

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

  case 549:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 550:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 551:

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

  case 552:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 553:

    { (yyval.uint) = 0; }

    break;

  case 554:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 555:

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

  case 556:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 557:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 558:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 559:

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

  case 560:

    { switch (actual_type) {
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

  case 570:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }

    break;

  case 573:

    { (yyval.v) = actual;
                        if (!(actual = yang_read_range(trg, actual, s))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 574:

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

  case 578:

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

  case 580:

    { tmp_s = yyget_text(scanner); }

    break;

  case 581:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM;
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 582:

    { tmp_s = yyget_text(scanner); }

    break;

  case 583:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM;
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 607:

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

  case 608:

    { (yyval.uint) = 0; }

    break;

  case 609:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 610:

    { (yyval.i) = 0; }

    break;

  case 611:

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

  case 617:

    { if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 622:

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

  case 628:

    { if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 651:

    { s = strdup(yyget_text(scanner));
                  if (!s) {
                    LOGMEM;
                    YYABORT;
                  }
                }

    break;

  case 742:

    { s = strdup(yyget_text(scanner));
                          if (!s) {
                            LOGMEM;
                            YYABORT;
                          }
                        }

    break;

  case 743:

    { s = strdup(yyget_text(scanner));
                                    if (!s) {
                                      LOGMEM;
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
