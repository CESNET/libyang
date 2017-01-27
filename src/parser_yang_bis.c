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
#define YYLAST   3222

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  318
/* YYNRULES -- Number of rules.  */
#define YYNRULES  744
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1202

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
     324,   334,   344,   345,   351,   356,   359,   370,   380,   385,
     386,   392,   394,   396,   398,   404,   405,   406,   409,   414,
     421,   422,   423,   434,   445,   452,   459,   461,   462,   466,
     467,   478,   489,   496,   506,   520,   525,   531,   532,   537,
     542,   547,   553,   555,   557,   559,   561,   574,   578,   579,
     581,   582,   587,   588,   593,   600,   600,   607,   613,   652,
     653,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   668,   684,   686,   687,   691,   692,   699,   706,   715,
     725,   725,   727,   728,   732,   733,   735,   736,   737,   746,
     748,   749,   750,   751,   766,   777,   779,   780,   795,   796,
     805,   811,   816,   822,   824,   837,   839,   840,   865,   866,
     880,   893,   899,   904,   910,   912,   961,   971,   974,   979,
     980,   986,   990,   995,  1002,  1004,  1010,  1011,  1016,  1051,
    1052,  1055,  1056,  1060,  1066,  1079,  1080,  1081,  1082,  1083,
    1085,  1102,  1108,  1109,  1125,  1129,  1137,  1138,  1143,  1155,
    1160,  1165,  1170,  1176,  1183,  1196,  1197,  1201,  1202,  1212,
    1217,  1222,  1227,  1233,  1244,  1256,  1257,  1260,  1268,  1277,
    1278,  1293,  1294,  1306,  1313,  1317,  1322,  1328,  1338,  1339,
    1354,  1359,  1360,  1365,  1367,  1369,  1370,  1371,  1384,  1396,
    1397,  1399,  1407,  1417,  1418,  1433,  1434,  1446,  1453,  1458,
    1463,  1469,  1479,  1480,  1496,  1498,  1500,  1502,  1504,  1511,
    1514,  1515,  1520,  1523,  1529,  1534,  1539,  1542,  1545,  1548,
    1551,  1559,  1560,  1561,  1562,  1563,  1564,  1565,  1566,  1569,
    1576,  1579,  1580,  1603,  1606,  1606,  1610,  1615,  1615,  1619,
    1624,  1630,  1636,  1641,  1646,  1646,  1651,  1651,  1656,  1656,
    1665,  1665,  1669,  1669,  1675,  1712,  1720,  1724,  1724,  1728,
    1733,  1733,  1738,  1743,  1743,  1747,  1752,  1758,  1764,  1770,
    1775,  1781,  1788,  1840,  1844,  1844,  1848,  1854,  1854,  1859,
    1870,  1875,  1875,  1879,  1885,  1898,  1911,  1921,  1927,  1932,
    1938,  1945,  1989,  1993,  1993,  1997,  2003,  2003,  2007,  2016,
    2022,  2028,  2041,  2054,  2064,  2070,  2075,  2080,  2080,  2084,
    2084,  2089,  2089,  2094,  2094,  2103,  2103,  2110,  2117,  2120,
    2121,  2141,  2145,  2145,  2149,  2155,  2165,  2172,  2179,  2186,
    2192,  2198,  2198,  2204,  2205,  2208,  2209,  2210,  2211,  2212,
    2213,  2214,  2221,  2228,  2231,  2232,  2246,  2249,  2249,  2253,
    2258,  2264,  2269,  2274,  2274,  2281,  2289,  2292,  2300,  2303,
    2304,  2327,  2330,  2330,  2334,  2340,  2340,  2344,  2351,  2358,
    2365,  2370,  2376,  2383,  2386,  2387,  2419,  2422,  2422,  2426,
    2431,  2437,  2442,  2447,  2447,  2451,  2451,  2457,  2458,  2460,
    2470,  2472,  2473,  2507,  2510,  2524,  2548,  2570,  2621,  2638,
    2655,  2676,  2697,  2702,  2708,  2709,  2712,  2724,  2729,  2730,
    2732,  2740,  2745,  2748,  2748,  2752,  2757,  2763,  2768,  2773,
    2773,  2778,  2778,  2783,  2783,  2792,  2792,  2798,  2812,  2816,
    2825,  2829,  2830,  2854,  2858,  2864,  2870,  2875,  2880,  2880,
    2884,  2884,  2889,  2889,  2899,  2899,  2910,  2910,  2946,  2949,
    2949,  2953,  2953,  2957,  2957,  2962,  2962,  2968,  2968,  3004,
    3012,  3014,  3015,  3049,  3052,  3052,  3056,  3061,  3067,  3072,
    3077,  3077,  3081,  3081,  3086,  3086,  3092,  3101,  3119,  3122,
    3128,  3134,  3139,  3140,  3142,  3147,  3149,  3151,  3152,  3153,
    3155,  3155,  3161,  3162,  3194,  3197,  3203,  3207,  3213,  3219,
    3226,  3233,  3241,  3250,  3250,  3256,  3257,  3289,  3292,  3298,
    3302,  3308,  3315,  3315,  3321,  3322,  3336,  3339,  3342,  3348,
    3354,  3361,  3368,  3376,  3385,  3392,  3394,  3395,  3399,  3400,
    3405,  3411,  3413,  3414,  3415,  3428,  3430,  3431,  3432,  3445,
    3447,  3449,  3450,  3470,  3472,  3473,  3474,  3494,  3496,  3497,
    3498,  3510,  3573,  3575,  3576,  3581,  3583,  3584,  3586,  3587,
    3589,  3591,  3591,  3598,  3601,  3609,  3628,  3630,  3631,  3634,
    3634,  3651,  3651,  3658,  3658,  3665,  3668,  3670,  3672,  3673,
    3675,  3677,  3679,  3680,  3682,  3684,  3685,  3687,  3688,  3690,
    3692,  3695,  3699,  3701,  3702,  3704,  3705,  3707,  3709,  3720,
    3721,  3724,  3725,  3736,  3737,  3739,  3740,  3742,  3743,  3749,
    3750,  3753,  3754,  3755,  3779,  3780,  3783,  3784,  3785,  3788,
    3788,  3794,  3796,  3797,  3799,  3800,  3801,  3803,  3804,  3806,
    3807,  3809,  3810,  3812,  3813,  3815,  3816,  3819,  3820,  3823,
    3825,  3826,  3829,  3829,  3836,  3838,  3839,  3840,  3841,  3842,
    3843,  3844,  3846,  3847,  3848,  3849,  3850,  3851,  3852,  3853,
    3854,  3855,  3856,  3857,  3858,  3859,  3860,  3861,  3862,  3863,
    3864,  3865,  3866,  3867,  3868,  3869,  3870,  3871,  3872,  3873,
    3874,  3875,  3876,  3877,  3878,  3879,  3880,  3881,  3882,  3883,
    3884,  3885,  3886,  3887,  3888,  3889,  3890,  3891,  3892,  3893,
    3894,  3895,  3896,  3897,  3898,  3899,  3900,  3901,  3902,  3903,
    3904,  3905,  3906,  3907,  3908,  3909,  3910,  3911,  3912,  3913,
    3914,  3915,  3916,  3917,  3918,  3919,  3920,  3921,  3922,  3923,
    3924,  3925,  3926,  3929,  3936
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
  "yang_version_stmt", "namespace_stmt", "linkage_stmt", "linkage_stmts",
  "import_stmt", "import_arg_str", "import_opt_stmt", "include_arg_str",
  "include_stmt", "include_end", "include_opt_stmt", "revision_date_stmt",
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

#define YYPACT_NINF -1058

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1058)))

#define YYTABLE_NINF -598

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1058,    21, -1058, -1058,    90, -1058, -1058, -1058,   149,   149,
   -1058, -1058,  3029,  3029,   149, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,   -47,
   -1058, -1058, -1058,   -44, -1058,   149, -1058,   149, -1058,   -31,
     283,   283, -1058, -1058, -1058,   174, -1058, -1058, -1058,    24,
     381,   149, -1058,   135,   149,   149,   149, -1058, -1058, -1058,
   -1058,   149, -1058, -1058, -1058,   235,  2364, -1058,   465,   149,
     149, -1058, -1058,  2459,  3029,  2459,   465,  3029, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058,   149,   149,   149,   149, -1058, -1058, -1058, -1058,
   -1058,    11,  3029,  3029, -1058,   254, -1058, -1058,   254, -1058,
     254, -1058,   -29, -1058,   149,   149,   149,   149,  2459,  2459,
    2459,  2459,   -17,  1296,   149,    -9, -1058,   278, -1058, -1058,
   -1058, -1058, -1058, -1058,   149, -1058,     7, -1058,  2173,    83,
     254,   254,   254,   254, -1058,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,   240, -1058, -1058, -1058, -1058,    12,   283,   149, -1058,
     246, -1058, -1058,   149, -1058, -1058, -1058, -1058, -1058, -1058,
     149,  3029,     3,  3029,  3029,  3029,     3,  3029,  3029,  3029,
    3029,  3029,  3029,  3029,  3029,  3029,  2554,  3029,   283, -1058,
   -1058,   305, -1058,   283,   283,   283, -1058,   149,    15,   328,
     397,   343, -1058,  3124, -1058, -1058,   126, -1058, -1058,   359,
   -1058,   385, -1058,   390, -1058, -1058,   129, -1058, -1058,   395,
   -1058,   399, -1058,   405, -1058,   140, -1058,   145, -1058,   156,
   -1058,   406, -1058,   417, -1058,   160, -1058, -1058, -1058,   423,
   -1058, -1058, -1058,   343, -1058, -1058, -1058, -1058, -1058,   370,
     208,   283, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058,    96,   149, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058,   149, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058,   149,   149, -1058,
     149,   283,   283,   149, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058,   283,   149,   149,   149,   283,   283, -1058,
     283,   283,   283,   283,   283,   283,   283,   283,   283,   283,
     283,   283,   283,   283,    82,   240,   283,  2268,  1052,  1508,
    1425,  1194,    99,   106,   566,  1051,   311,   294,  1358,   913,
    1667,  1151,   287,   219, -1058, -1058, -1058,   254, -1058,   149,
     149,   149,   149,   149,   149, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058,   149,   149, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058,   149, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058,   149, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,   149, -1058, -1058, -1058, -1058, -1058,   149, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058,   149, -1058, -1058, -1058, -1058,
   -1058, -1058,   149,   149, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058,   149,   149,   149, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058,   149,   149, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
     149,   149, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,   149,    59,  2459,    74,  2459,   118,  2459, -1058, -1058,
    3029,  3029, -1058, -1058, -1058, -1058, -1058,  2459, -1058, -1058,
    2459, -1058, -1058, -1058, -1058, -1058, -1058, -1058,  3029,   282,
     283,   283,   283,   283,   283,  2554,  2554,  2459, -1058, -1058,
   -1058,    35,   189,    17, -1058, -1058, -1058,  2649,  2649, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
     149,   149, -1058, -1058, -1058, -1058,   283,  2649,  2649, -1058,
   -1058, -1058, -1058, -1058, -1058,   254,   254, -1058, -1058, -1058,
     254,   424, -1058, -1058, -1058, -1058, -1058,   254,   427, -1058,
     283,   283,   431, -1058,   417, -1058,   283,   283,   283,   283,
     283,   254,   283,   283,   254,   283,   283,   283,   283,   283,
     283,   283, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058,   254,   438, -1058,   254,   283,   283,   283,
   -1058, -1058, -1058,   254, -1058, -1058, -1058,   254, -1058, -1058,
   -1058, -1058, -1058,   254,   283,   283,   283, -1058,   254, -1058,
     254, -1058,    89, -1058,   283,   283,   283,   283,   283,   283,
     283,   283,   283,   283,   283,   163,   168,   283,   283,   283,
     283, -1058, -1058,   183, -1058, -1058, -1058,   469, -1058,   283,
     283,   283,   149,   149, -1058, -1058,   149,   149, -1058, -1058,
   -1058, -1058,   149,   149,   149, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058,   474,   149,   149,    89,   149,
     149, -1058, -1058, -1058, -1058, -1058,   149, -1058,   149, -1058,
     149,   149,   149, -1058, -1058, -1058, -1058, -1058, -1058,  3124,
   -1058, -1058, -1058, -1058,   149, -1058, -1058, -1058,   149,   283,
     283,   283, -1058, -1058, -1058,   479,   502, -1058,   507,   376,
     149,    89,    96,   283,   283,   283,   283,   143,   260,   402,
     146,   283, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,   149,   149,   173,   863, -1058, -1058, -1058, -1058, -1058,
     300,  1254,  1584,   808,   149,   149, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,   149,   196,   283,   283,   283,  3029,  2459, -1058,   149,
     149,   149,   149,   149,   149, -1058,   227, -1058, -1058, -1058,
   -1058, -1058, -1058,   283,   279, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058,  2459,  2459, -1058, -1058,    75, -1058,   413,
      68,   773,   508, -1058,   511, -1058,    97,  2459,   120,  2459,
    2459,    77, -1058,   283,   274, -1058, -1058, -1058, -1058, -1058,
     283,   254,   254,   283,   283, -1058, -1058, -1058,   254, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,   254, -1058,
     516, -1058, -1058,   254,   204,   298,   517, -1058,   521, -1058,
   -1058, -1058, -1058,   254,   283, -1058,   283,   283,   283,   283,
   -1058, -1058,   149,   149, -1058,   283,   283,   283,   283,   283,
     283, -1058,   149, -1058, -1058, -1058, -1058,  3124, -1058, -1058,
     214, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,   149,
     149, -1058,   283,   411,   945,   283,   283, -1058,   204, -1058,
    2744,   283,   283,   283,   149, -1058, -1058, -1058, -1058, -1058,
   -1058,   149, -1058, -1058, -1058, -1058, -1058, -1058,   565,   228,
   -1058, -1058, -1058,    93,   210,   597,   189,   178, -1058,   323,
   -1058,   131,   149, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,   254, -1058, -1058, -1058, -1058, -1058,   254, -1058, -1058,
   -1058,  3124, -1058,  2459, -1058,   149, -1058,   149,   323,   323,
     254,   231,   233, -1058, -1058,   323,   266,   323, -1058,   323,
     243,   252,   323,   323,   264,   350, -1058,   323, -1058, -1058,
     272,  2839,   323, -1058, -1058, -1058,  2934, -1058,   280,   323,
    3124, -1058
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     647,     0,     2,     3,     0,     1,   645,   646,     0,     0,
     648,   647,     0,     0,   649,   663,     4,   662,   664,   665,
     666,   667,   668,   669,   670,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   695,
     696,   697,   698,   699,   700,   701,   702,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   719,   720,   721,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   732,   733,   734,   735,
     736,   737,   738,   739,   740,   741,   742,   647,   618,     0,
       9,   743,   647,     0,    16,     6,   626,   617,   626,     5,
      12,    19,   647,   629,    25,    11,   628,   627,    25,    18,
       0,   632,    47,    24,     0,     0,     0,    13,    14,    15,
      47,     0,    20,    21,     7,     0,   634,   633,    58,     0,
       0,    26,    27,     0,     0,     0,    58,     0,   647,   647,
     639,   630,   647,   655,   658,   656,   660,   661,   659,   631,
     647,   657,     0,     0,     0,     0,    48,    49,    50,    51,
      69,    56,     0,     0,   654,     0,   652,   615,     0,   647,
       0,    69,     0,    44,     8,   641,   637,   635,     0,     0,
       0,     0,     0,    68,     0,     0,    29,     0,    35,   626,
     626,    23,   647,    46,   616,    22,     0,   626,     0,   636,
       0,     0,     0,     0,   647,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   626,    71,    72,    73,    74,    75,    76,   231,
     232,   233,   234,   235,   236,   237,   238,    77,    78,    79,
      80,     0,   626,   626,    37,   626,     0,   624,   653,   647,
       0,   620,   640,   632,   619,   647,    53,    54,    52,    55,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    70,    65,
      67,     0,    57,    30,    39,    36,   626,    17,     0,     0,
       0,     0,   365,     0,   419,   420,     0,   577,   647,     0,
     327,     0,   239,     0,    81,   493,     0,   486,   647,     0,
     104,     0,   218,     0,   114,     0,   265,     0,   281,     0,
     300,     0,   469,     0,   439,     0,   125,   744,   623,     0,
     382,   647,   647,     0,   367,   647,   626,    60,   626,     0,
       0,   625,   626,   647,   647,   642,   647,   626,   369,   366,
     575,   626,   576,   418,   626,   329,   328,   626,   241,   240,
     626,    83,    82,   626,   492,   626,   106,   105,   626,   220,
     219,   626,   116,   115,   626,   626,   626,   626,   471,   470,
     626,   441,   440,   626,   626,   384,   383,   621,   622,   368,
      66,    62,    59,     0,   626,    34,    31,    32,    33,    38,
      42,    40,    41,    45,   641,   643,   638,   371,   422,   578,
     331,   243,    85,   488,   108,   222,   118,   266,   283,   302,
     473,   443,   127,   386,     0,     0,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    61,    63,    64,     0,   647,     0,
       0,     0,     0,     0,     0,   370,   380,   381,   379,   374,
     372,   377,   378,   375,     0,     0,   421,   427,   428,   426,
     425,   429,   435,   431,   433,   423,     0,   330,   339,   340,
     338,   334,   335,   345,   346,   347,   348,   351,   341,   343,
     344,   349,   350,   332,   336,   337,     0,   242,   252,   253,
     251,   246,   260,   254,   262,   256,   258,   244,   250,   249,
     247,     0,    84,    88,    89,    86,    87,     0,   487,   489,
     490,   107,   111,   112,   110,   109,   221,   224,   225,   223,
     626,   626,   626,   626,   626,     0,   117,   122,   123,   121,
     120,   119,     0,     0,   264,   279,   280,   278,   269,   270,
     272,   275,   267,   276,   277,   273,     0,     0,     0,   282,
     298,   299,   297,   286,   287,   290,   289,   284,   293,   294,
     295,   296,   291,     0,     0,   301,   315,   316,   314,   305,
     317,   319,   325,   321,   323,   303,   310,   311,   312,   313,
     306,   309,   308,   472,   478,   479,   477,   476,   480,   482,
     484,   474,   647,   647,   442,   446,   447,   445,   444,   448,
     450,   452,   454,   126,   132,   133,   131,   626,   129,   130,
       0,     0,   385,   391,   392,   390,   389,   393,   395,   387,
      43,   644,     0,     0,     0,     0,     0,     0,   626,   626,
       0,     0,   626,   626,   626,   626,   626,     0,   626,   626,
       0,   626,   626,   626,   626,   626,   626,   626,     0,     0,
     227,   226,   228,   229,   230,     0,     0,     0,   626,   626,
     626,     0,     0,     0,   626,   626,   626,     0,     0,   626,
     626,   626,   626,   626,   626,   626,   626,   626,   626,   626,
     456,   467,   626,   626,   626,   626,   128,     0,     0,   626,
     626,   626,   647,   647,   544,     0,     0,   647,   647,   548,
       0,     0,   561,   647,   647,   647,   103,     0,     0,   534,
     373,   376,     0,   352,     0,   437,   430,   436,   432,   434,
     424,     0,   342,   333,     0,   261,   255,   263,   257,   259,
     245,   248,    90,   647,   647,   647,   647,   491,   494,   647,
     497,   499,   498,     0,     0,   135,     0,   271,   268,   274,
     608,   647,   556,     0,   647,   609,   552,     0,   610,   647,
     647,   647,   560,     0,   288,   285,   292,   573,     0,   571,
       0,   565,     0,   579,   318,   320,   326,   322,   324,   304,
     307,   481,   483,   485,   475,     0,     0,   449,   451,   453,
     455,   415,   416,     0,   647,   398,   399,     0,   647,   394,
     396,   388,   543,   542,   541,   113,   547,   546,   545,   626,
     563,   562,   100,   102,   101,    99,   626,   536,   535,   626,
     354,   353,   438,   217,   549,     0,   500,   513,     0,   522,
     495,   124,   626,   136,   134,   216,   554,   553,   555,   550,
     551,   559,   558,   557,   570,   647,   567,   566,   569,     0,
     577,   626,   626,   626,   414,   626,   401,   400,   397,   158,
     538,   356,   626,    92,   626,     0,     0,   496,     0,   141,
     572,     0,   580,   458,   458,   422,   403,     0,     0,     0,
      94,    91,   626,   502,   501,   626,   515,   514,   626,   524,
     523,     0,     0,     0,   138,   139,   626,   140,   199,   568,
       0,     0,     0,     0,     0,     0,   564,   161,   162,   159,
     160,   537,   539,   540,   355,   361,   362,   360,   359,   363,
     357,     0,     0,   504,   517,   526,     0,     0,   137,     0,
       0,     0,     0,     0,     0,   144,     0,   147,   145,   146,
     626,   143,   142,   175,   198,   457,   461,   463,   465,   459,
     468,   417,   402,   412,   413,   405,   407,   408,   409,   406,
     410,   411,   626,     0,     0,   626,   626,     0,    93,     0,
       0,     0,     0,   202,     0,   178,     0,     0,   581,     0,
       0,     0,   626,   148,   174,   200,   626,   626,   626,   626,
     404,     0,     0,   364,   358,   647,   647,    98,     0,   503,
     505,   508,   509,   510,   511,   512,   626,   507,   516,   518,
     521,   626,   520,   525,   626,   528,   529,   530,   531,   532,
     533,   626,   203,   626,   626,   179,   177,   153,     0,   647,
       0,   155,   585,     0,     0,     0,     0,   164,     0,   574,
     647,   647,   197,     0,   149,   626,   462,   464,   466,   460,
     215,   214,    97,    96,    95,   506,   519,   527,   205,   201,
     181,   151,   152,   626,   156,   626,   193,     0,   588,   582,
       0,   584,   592,   626,   165,   626,   626,   191,   190,   196,
     195,   194,   176,     0,     0,   158,   154,   597,   587,   591,
       0,   167,   163,   158,     0,   204,   209,   210,   208,   206,
     207,     0,   180,   185,   186,   184,   182,   183,     0,   586,
     589,   593,   590,   595,     0,     0,     0,     0,   157,   650,
     594,     0,     0,   166,   171,   172,   168,   169,   170,   192,
     213,     0,   647,   613,   614,   611,   189,     0,   647,   612,
     651,     0,   596,     0,   211,   212,   187,   188,   650,   650,
       0,     0,     0,   173,   598,   650,     0,   650,   599,   650,
       0,     0,   650,   650,     0,     0,   607,   650,   600,   603,
       0,     0,   650,   604,   605,   602,   650,   601,     0,   650,
       0,   606
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1058, -1058, -1058,    44, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058,   273, -1058,   277, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058,    41, -1058, -1058,  -242,   288,
   -1058, -1058,   334,   726,   289, -1058, -1058, -1058, -1058,     1,
   -1058,   265, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058,  -411, -1058, -1058, -1058, -1058, -1058,  -387,
   -1058, -1058, -1058, -1058,  -463, -1058,  -178, -1058,  -445, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1057,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058,  -550, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
    -502, -1058, -1058, -1058, -1058, -1058,  -667,  -658,  -402,  -348,
   -1058,  -167, -1058, -1058,  -181, -1058,    37, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058,    46, -1058, -1058, -1058,
   -1058, -1058, -1058,    58, -1058, -1058, -1058, -1058, -1058,    64,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,    94,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058,   104, -1058, -1058,
   -1058, -1058, -1058,   119, -1058,   150,   172, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,  -356, -1058,
   -1058, -1058, -1058, -1058, -1058,  -421, -1058, -1058,  -176, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058,  -312, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058,  -144, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058, -1058,
   -1058,  -369, -1058, -1058,  -316, -1058,  -438, -1058,  -332,  -417,
   -1058,  -405, -1058,   151, -1058, -1058,  -373, -1058,  -849, -1058,
    -290, -1058, -1058, -1058, -1058,  -340,   329,  -256,  -694, -1058,
   -1058, -1058, -1058,  -486,  -512, -1058, -1058,  -476, -1058, -1058,
   -1058,  -498, -1058, -1058, -1058,  -555, -1058, -1058, -1058,  -673,
    -496, -1058, -1058, -1058,   142,  -192,  -395,   499,  1096, -1058,
   -1058,   375, -1058, -1058, -1058, -1058,   230, -1058,    -4,   -10,
     430,   222,  -100, -1058,   526,   377,  -138, -1058
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   109,   148,    99,     2,   114,   115,
     103,     3,   118,   119,   127,   128,   122,   123,   141,   195,
     349,   197,   142,   255,   350,   405,   182,   133,   129,   138,
     166,   167,   927,   928,   170,   291,   171,   348,   434,   292,
     345,   192,   193,   232,   313,   233,   372,   442,   525,   845,
     884,   942,  1018,   479,   727,   319,   234,   377,   444,   480,
     323,   235,   383,   446,   551,   335,   966,   452,   559,   764,
     854,   913,   914,   956,   957,  1048,   958,  1050,  1085,   897,
     959,  1056,  1095,  1134,  1146,   915,  1004,   916,   994,  1046,
    1104,  1127,  1157,   960,  1098,   961,   962,  1063,   917,   964,
     918,   992,  1043,  1103,  1120,  1151,   929,   930,   560,   492,
     321,   967,   380,   445,   481,   311,   239,   369,   441,   666,
     667,   662,   664,   665,   661,   663,   240,   325,   447,   679,
     678,   680,   327,   241,   448,   685,   684,   686,   329,   242,
     449,   694,   695,   689,   690,   692,   693,   691,   309,   243,
     366,   440,   659,   658,   498,   499,   732,   482,   841,   899,
     986,   985,   301,   244,   343,   245,   359,   438,   648,   649,
     339,   246,   396,   453,   711,   709,   710,   816,   817,   637,
     877,   923,   812,   813,   638,   305,   306,   247,   439,   656,
     652,   654,   655,   653,   734,   483,   333,   248,   392,   451,
     702,   703,   704,   705,   621,   805,   920,  1009,  1006,  1007,
    1008,   622,   806,   331,   484,   389,   450,   699,   696,   697,
     698,   316,   250,   443,   317,   757,   758,   759,   760,   885,
     904,   989,   761,   886,   907,   990,   762,   888,   910,   991,
     728,   485,   838,   898,   471,   715,   472,   720,   519,   579,
     777,   580,   773,   581,   783,   721,   969,   831,   601,   791,
     867,   602,   788,   865,  1058,   307,   308,   362,   792,   870,
    1053,  1054,  1055,  1088,  1089,  1108,  1091,  1092,  1110,  1132,
    1140,  1129,  1168,  1178,  1188,  1189,  1191,  1196,  1179,   778,
     779,  1158,  1159,   178,   100,   793,   340,   868,   110,   116,
     121,   135,   136,   159,   209,   151,   208,   355,   117,     4,
     137,  1161,   175,   202,   176,   101,   102,   342
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    14,   505,   574,    11,    11,   179,   627,   774,   564,
      10,    16,   238,   814,   818,   236,   263,     6,   298,     7,
     515,     5,   419,   113,   543,    16,   237,   468,   593,   490,
     510,   526,   597,   534,   539,   549,   557,   572,   588,   606,
     617,   626,   635,    16,   598,   180,   575,   131,  1128,   249,
     628,   469,   770,   491,   511,   106,  1135,   535,   108,   550,
     558,   573,   589,   607,   618,   473,   636,    16,   520,   470,
     112,   503,   517,   207,   565,   582,   600,   611,   562,   577,
     595,   194,    16,    16,   639,    16,   214,   105,   210,   211,
     212,   213,   107,   252,     6,     6,     7,     7,   486,   561,
     576,    10,   120,    10,   629,    16,   126,   406,   303,   780,
     259,   360,   781,   163,   770,   296,    10,    11,   352,   521,
      11,    11,    11,   462,   504,   518,    16,    11,    16,   771,
     163,   563,   578,   596,  -583,    11,    11,   163,   184,   185,
    1027,  1032,   186,   527,     8,   584,   553,   712,   341,   165,
     187,     6,   713,     7,     6,   104,     7,   113,    11,    11,
      11,    11,   717,  1015,     9,  1060,   165,   718,  1016,   204,
    1061,  1028,   463,   165,   163,   891,   924,   925,   139,   140,
      10,    10,    10,    10,   265,   454,    16,   174,   177,   174,
      11,   199,   258,   200,  1153,  1154,  1155,    16,  -597,  -597,
      10,   303,   522,   723,   270,   724,   770,   775,   725,   528,
     165,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,   361,   941,
     124,   373,   174,   174,   174,   174,  1087,  1139,   125,   163,
     630,   163,   384,   924,   925,   263,   926,   385,    16,   297,
     163,     6,   289,     7,    10,   300,   126,   113,   386,    11,
     514,   460,   393,   512,   542,   871,    10,   540,   592,   610,
     872,   590,   608,   619,   513,   165,   948,   165,   541,   403,
     763,   765,   591,   609,   620,   873,   165,   631,     6,   183,
       7,   163,   463,    10,   113,   290,    10,   516,   363,   988,
     464,   544,   552,   911,   215,   594,   912,  1142,   374,  1087,
     125,   409,  1090,  1143,   196,   198,   304,   486,   163,  1109,
     315,   459,   632,  1049,   486,   163,   217,   165,  1160,   218,
     338,   397,   398,   545,  1139,   400,   460,   149,  1174,   150,
     222,  1175,   163,   414,   415,   461,   416,   224,   225,   462,
     226,  1177,  1182,   460,   165,   462,   199,  1183,   200,    10,
     463,   165,   552,   931,  1187,   553,   753,   463,   754,   552,
      10,   755,   553,   756,  1186,   464,   229,  1192,   165,   230,
     253,     6,   254,     7,   463,  1199,     6,   113,     7,   134,
     623,   410,   132,    10,    10,   130,    10,   554,   231,    11,
     911,   163,     6,   965,     7,   356,   215,   346,   912,   347,
      10,    10,    10,   302,   546,   310,   312,   314,   146,   320,
     322,   324,   326,   328,   330,   332,   334,   336,   217,   344,
     353,   218,   354,   163,   125,   181,   457,   165,    12,    13,
     459,   403,   163,   486,   460,   357,   206,   358,   641,   224,
     225,   955,   226,   460,  1065,    11,    11,    11,    11,    11,
      11,   364,  1005,   365,   461,   566,   567,  1147,   462,   165,
      11,    11,   168,   404,  1114,   463,  1148,   493,   165,   290,
     168,   230,    11,   464,   463,   978,   494,   367,   937,   368,
     584,   553,   370,   162,   371,   789,   163,   375,   495,   376,
     231,   378,    11,   379,   496,   934,   980,   381,   387,   382,
     388,  1002,   938,   161,  1115,   399,  1019,    11,   981,   390,
     161,   391,   161,    11,   164,   394,   829,   395,   830,   836,
     940,   837,   165,   839,   497,   840,   975,   341,   341,   922,
     852,    11,   853,   716,   500,   722,  1034,   729,    11,    11,
     982,  1023,   419,  1038,   143,   144,   145,   741,   842,   501,
     744,   147,    11,    11,    11,   161,   161,   161,   161,   172,
     173,   875,  1024,   876,  1039,   976,   882,   766,   883,    11,
      11,   902,   921,   903,  1025,   264,  1040,  1020,  1029,  1035,
     502,   979,   188,   189,   190,   191,   163,   163,   924,   925,
     599,   919,   700,   701,   905,   318,   906,   977,   460,   908,
    1041,   909,  1042,  1044,   892,  1045,  1026,  1031,  1083,  1093,
    1084,  1094,  1130,  1096,   251,  1097,    11,    11,   163,  1162,
     924,   925,   165,   165,  1131,  1141,  1193,    10,   299,   463,
    1152,  1021,  1030,  1036,   437,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   160,     0,   165,     0,     0,     0,  1138,   531,
       0,     0,     0,  1022,   201,  1037,     0,   203,     0,   205,
     264,     0,     0,   407,   411,     0,   714,   174,   719,   174,
     726,   174,  1118,  1125,     0,     0,    10,    10,     0,     0,
    1149,   174,   822,   823,   174,     0,     0,   826,   827,   266,
     267,   268,   269,   832,   833,   834,  1119,  1126,   939,   338,
     338,   174,     0,     0,     0,   772,   776,   782,     0,     0,
       0,   787,   790,     0,     0,     0,     0,     0,     0,   968,
     968,     0,     0,   846,   847,   848,   849,     0,     0,   850,
       0,   811,   815,     0,     0,     0,     0,     0,     0,     0,
       0,   856,     0,     0,   858,     0,     0,     0,   455,   860,
     861,   862,   466,   477,   488,   508,   523,   529,   532,   537,
     547,   555,   570,   586,   604,   615,   624,   633,    11,     0,
       0,     0,   733,   735,     0,     0,     0,     0,     0,     0,
     459,     0,     0,   486,   874,     0,     0,     0,   878,     0,
     752,     0,     0,     0,   264,     0,     0,     0,    10,    10,
       0,     0,    10,    10,   461,   566,   567,     0,    10,    10,
      10,     0,     0,   435,     0,   459,     0,     0,   486,   163,
       0,     0,    10,    10,    10,    10,    10,   995,   552,     0,
     460,   553,    10,     0,    10,   890,    10,    10,    10,   461,
     566,   567,     0,   462,   169,     0,     0,     0,     0,     0,
      10,     0,   169,   506,    10,   165,  1033,     0,     0,     0,
       0,     0,     0,  1011,  1012,   545,    10,    11,     0,   642,
     643,   644,   645,   646,   647,  1107,     0,  1051,     0,  1057,
    1059,     0,   949,     0,   650,   651,     0,    11,    11,     0,
       0,   972,   950,     0,     0,     0,   657,   215,  1133,     0,
      11,    11,     0,     0,   951,   952,     0,     0,     0,   953,
       0,     0,   954,     0,     0,     0,   660,    11,  -150,   217,
     459,     0,   218,     0,   163,    11,    11,    11,    11,    11,
      11,   668,     0,   222,     0,   460,   640,   669,     0,   583,
     224,   225,     0,   226,     0,   566,   567,     0,   462,  1169,
     227,   568,     0,     0,     0,   675,   163,     0,     0,     0,
     165,     0,   676,   677,     0,     0,   463,   460,     0,   229,
     584,   174,   230,     0,   464,     0,   681,   682,   683,  1194,
       0,     0,     0,     0,  1197,  1072,  1073,     0,  1201,   475,
       0,   231,   165,   687,   688,     0,   585,     0,   463,     0,
     161,     0,   161,     0,   161,  1121,     0,   174,   174,     0,
       0,  1017,     0,     0,   161,     0,     0,   161,     0,  1082,
    1047,   174,  1052,   174,   174,  1062,     0,     0,  1122,     0,
    1099,  1100,     0,     0,   161,   215,     0,     0,     0,     0,
     707,   708,     0,  1170,   264,   264,     0,     0,    10,    10,
       0,     0,     0,     0,     0,   408,   412,   217,    10,   459,
     218,     0,   163,   163,   264,   264,     0,     0,   993,     0,
       0,   222,     0,     0,   460,    10,    10,     0,   224,   225,
       0,   226,     0,   461,     0,     0,     0,   462,   227,     0,
      11,     0,     0,     0,     0,     0,     0,    11,   165,   165,
       0,     0,     0,     0,   463,   463,     0,   229,     0,     0,
     230,     0,     0,   464,     0,     0,     0,     0,    11,     0,
       0,     0,  1165,     0,     0,     0,     0,   475,  1167,   231,
       0,     0,     0,     0,   536,   465,     0,     0,     0,     0,
     456,    10,     0,    10,   467,   478,   489,   509,   524,   530,
     533,   538,   548,   556,   571,   587,   605,   616,   625,   634,
    1150,  1156,   163,     0,     0,     0,     0,     0,     0,     0,
       0,   222,     0,   460,     0,     0,   612,     0,   215,     0,
       0,     0,     0,     0,   111,     0,     0,   174,     0,     0,
       0,   613,     0,     0,   824,   825,     0,     0,   165,   828,
     217,   459,   869,   218,   463,   163,   835,   229,     0,     0,
       0,     0,   932,   935,   222,     0,   460,     0,     0,     0,
     843,   224,   225,   844,   226,     0,   264,     0,     0,   462,
       0,   227,     0,     0,   614,     0,   477,   973,   215,   506,
       0,   165,   851,     0,     0,   855,     0,   463,     0,     0,
     229,     0,   857,   230,     0,   464,   859,     0,     0,     0,
     217,     0,   863,   218,     0,     0,     0,   864,     0,   866,
     475,     0,   231,     0,   222,   256,   257,   507,     0,     0,
     215,   224,   225,   260,   226,     0,     0,     0,     0,   462,
       0,     0,     0,     0,     0,     0,     0,   216,     0,     0,
       0,   869,   217,     0,   161,   218,     0,     0,   288,     0,
     229,   219,   220,   230,   221,     0,   222,   223,     0,     0,
       0,   946,   947,   224,   225,     0,   226,   887,   293,   294,
       0,   295,   231,   227,   983,   984,     0,   970,     0,     0,
     161,   161,     0,     0,     0,     0,     0,     0,   228,     0,
       0,   987,   229,     0,   161,   230,   161,   161,     0,   996,
     997,   998,   999,  1000,  1001,   459,     0,     0,   486,   163,
    1171,  1172,   351,     0,   231,     0,     0,  1176,     0,  1180,
     460,  1181,     0,     0,  1184,  1185,     0,     0,     0,  1190,
     566,   567,     0,   462,  1195,     0,   568,     0,  1198,     0,
       0,  1200,     0,     0,     0,   165,     0,     0,     0,   215,
       0,   463,     0,   552,     0,     0,   553,  1116,  1123,   464,
       0,     0,   401,     0,   402,     0,     0,     0,   413,     0,
     474,   217,   459,   417,   218,   486,   163,   418,     0,     0,
     420,   569,     0,   421,   264,     0,   422,   460,  1144,   423,
       0,   424,   224,   225,   425,   226,   461,   426,     0,     0,
     427,   428,   429,   430,     0,     0,   431,   264,     0,   432,
     433,     0,   165,     0,     0,     0,     0,     0,   463,     0,
     436,     0,     0,     0,     0,     0,   464,     0,     0,     0,
    1070,  1071,   215,     0,     0,     0,     0,  1074,     0,     0,
       0,     0,     0,   231,     0,     0,     0,     0,   487,     0,
       0,     0,     0,   474,   217,     0,     0,   218,   264,   163,
     161,     0,     0,     0,  1136,     0,     0,  1081,     0,     0,
     460,  1137,  1086,     0,     0,   224,   225,     0,   226,     0,
       0,     0,  1101,     0,     0,   227,     0,     0,   264,     0,
       0,     0,  1163,   264,     0,   165,     0,   264,     0,     0,
       0,   463,     0,     0,     0,     0,     0,   230,   215,   464,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   475,     0,   231,     0,     0,   474,
     217,   476,     0,   218,     0,   163,     0,     0,     0,     0,
       0,     0,     0,     0,   933,   936,   460,     0,     0,     0,
       0,   224,   225,     0,   226,     0,   670,   671,   672,   673,
     674,   227,     0,     0,     0,     0,     0,     0,   478,   974,
    1164,   165,     0,     0,     0,     0,  1166,   463,     0,     0,
       0,     0,     0,   230,     0,   464,     0,     0,     0,  1173,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
     475,     0,   231,     0,     0,     0,     0,   971,     0,     0,
       0,     0,     0,   217,     0,     0,   218,     0,   163,     0,
       0,     0,     0,     0,     0,     0,     0,   222,     0,   460,
       0,     0,     0,     0,   224,   225,     0,   226,     0,     0,
       0,     0,   462,   706,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,     0,     0,     0,     0,     0,
     463,     0,     0,   229,   730,   731,   230,     0,   736,   737,
     738,   739,   740,     0,   742,   743,     0,   745,   746,   747,
     748,   749,   750,   751,     0,   231,     0,     0,     0,     0,
     603,     0,     0,     0,   767,   768,   769,     0,     0,     0,
     784,   785,   786,     0,     0,   794,   795,   796,   797,   798,
     799,   800,   801,   802,   803,   804,     0,     0,   807,   808,
     809,   810,     0,     0,     0,   819,   820,   821,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1117,
    1124,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1145,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   879,     0,     0,     0,     0,
       0,     0,   880,     0,     0,   881,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   889,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   893,   894,   895,
       0,   896,     0,     0,     0,     0,     0,     0,   900,     0,
     901,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   943,     0,
       0,   944,     0,     0,   945,     0,     0,     0,     0,     0,
       0,     0,   963,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1003,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1010,     0,
       0,  1013,  1014,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1064,     0,
       0,     0,  1066,  1067,  1068,  1069,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1075,     0,     0,     0,     0,  1076,     0,     0,
    1077,     0,     0,     0,     0,     0,     0,  1078,     0,  1079,
    1080,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,     0,  1105,
       0,  1106,     0,    17,   261,     0,     0,     0,     0,  1111,
       0,  1112,  1113,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    15,     0,     0,     0,   262,     0,    17,   261,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,     0,    15,     0,
       0,   458,   152,   153,    17,   154,   155,     0,     0,     0,
     156,   157,   158,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,    16,   153,    17,
     154,   155,     0,     0,     0,   156,   157,   158,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,     0,
       0,     0,    16,     0,    17,   337,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,    16,     0,    17,
     261,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,     0,
       0,     0,     0,     0,    17,   261,     0,     0,  1090,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    15,     0,     0,     0,     0,     0,    17,
     261,     0,     0,  1187,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    15,  1160,
       0,     0,     0,     0,    17,   261,     0,     0,     0,     0,
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
       0,     0,     0,     0,    17,   261,     0,     0,     0,     0,
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
       4,    11,   440,   448,     8,     9,   144,   452,   681,   447,
      14,     8,   193,   707,   708,   193,   208,     5,   260,     7,
     441,     0,   362,    11,   445,     8,   193,   438,   449,   440,
     441,   442,   449,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     8,   449,   145,   448,    23,  1105,   193,
     452,   438,    17,   440,   441,   102,  1113,   444,   102,   446,
     447,   448,   449,   450,   451,   438,   453,     8,   441,   438,
     101,   440,   441,   102,   447,   448,   449,   450,   447,   448,
     449,    70,     8,     8,   453,     8,   103,    97,   188,   189,
     190,   191,   102,   102,     5,     5,     7,     7,    30,   447,
     448,   105,   112,   107,   452,     8,    82,   349,   105,    92,
     103,   303,    95,    31,    17,   103,   120,   121,   103,    20,
     124,   125,   126,    55,   440,   441,     8,   131,     8,    94,
      31,   447,   448,   449,    14,   139,   140,    31,   148,   149,
     989,   990,   152,    37,    54,    77,    78,    88,   286,    67,
     160,     5,    93,     7,     5,    13,     7,    11,   162,   163,
     164,   165,    88,    88,    74,    88,    67,    93,    93,   179,
      93,   103,    73,    67,    31,   869,    33,    34,    43,    44,
     184,   185,   186,   187,   101,   103,     8,   143,   144,   145,
     194,   102,   202,   104,    16,    17,    18,     8,   105,   106,
     204,   105,   103,    85,   214,    87,    17,    18,    90,   103,
      67,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   102,    83,
      56,   102,   188,   189,   190,   191,   105,   106,    64,    31,
      21,    31,   102,    33,    34,   437,   103,   102,     8,   259,
      31,     5,    12,     7,   258,   265,    82,    11,   102,   263,
     441,    42,   102,   441,   445,   102,   270,   445,   449,   450,
     102,   449,   450,   451,   441,    67,   103,    67,   445,    71,
     675,   676,   449,   450,   451,   102,    67,    68,     5,   147,
       7,    31,    73,   297,    11,   251,   300,   441,   308,   103,
      81,   445,    75,    24,     4,   449,    32,    97,   318,   105,
      64,   103,    14,   103,   172,   173,   272,    30,    31,   105,
     276,    27,   103,   996,    30,    31,    26,    67,     5,    29,
     286,   341,   342,    22,   106,   345,    42,   102,   107,   104,
      40,   108,    31,   353,   354,    51,   356,    47,    48,    55,
      50,    85,   109,    42,    67,    55,   102,   105,   104,   363,
      73,    67,    75,   103,    14,    78,    84,    73,    86,    75,
     374,    89,    78,    91,   110,    81,    76,   105,    67,    79,
     102,     5,   104,     7,    73,   105,     5,    11,     7,     8,
     103,   350,   119,   397,   398,   118,   400,   103,    98,   403,
      24,    31,     5,   103,     7,     8,     4,   102,    32,   104,
     414,   415,   416,   271,   103,   273,   274,   275,   130,   277,
     278,   279,   280,   281,   282,   283,   284,   285,    26,   287,
     102,    29,   104,    31,    64,   146,   435,    67,     8,     9,
      27,    71,    31,    30,    42,   102,   181,   104,   458,    47,
      48,   914,    50,    42,  1004,   459,   460,   461,   462,   463,
     464,   102,   964,   104,    51,    52,    53,  1134,    55,    67,
     474,   475,   138,   103,    63,    73,  1134,   440,    67,   435,
     146,    79,   486,    81,    73,   923,   440,   102,   899,   104,
      77,    78,   102,    28,   104,   687,    31,   102,   440,   104,
      98,   102,   506,   104,   440,   103,   923,   102,   102,   104,
     104,   956,   899,   136,   103,   343,   103,   521,   923,   102,
     143,   104,   145,   527,    59,   102,   102,   104,   104,   102,
     899,   104,    67,   102,   440,   104,   923,   675,   676,   895,
     102,   545,   104,   643,   440,   645,   991,   647,   552,   553,
     923,   989,   892,   991,   124,   125,   126,   657,   734,   440,
     660,   131,   566,   567,   568,   188,   189,   190,   191,   139,
     140,   102,   989,   104,   991,   923,   102,   677,   104,   583,
     584,   102,   894,   104,   989,   208,   991,   989,   990,   991,
     440,   923,   162,   163,   164,   165,    31,    31,    33,    34,
     449,   891,   612,   613,   102,   276,   104,   923,    42,   102,
     102,   104,   104,   102,   870,   104,   989,   990,   102,   102,
     104,   104,  1108,   102,   194,   104,   630,   631,    31,  1141,
      33,    34,    67,    67,  1110,  1133,  1191,   641,   263,    73,
    1136,   989,   990,   991,   414,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   136,    -1,    67,    -1,    -1,    -1,   103,   103,
      -1,    -1,    -1,   989,   175,   991,    -1,   178,    -1,   180,
     303,    -1,    -1,   349,   350,    -1,   642,   643,   644,   645,
     646,   647,  1103,  1104,    -1,    -1,   700,   701,    -1,    -1,
     103,   657,   712,   713,   660,    -1,    -1,   717,   718,   210,
     211,   212,   213,   723,   724,   725,  1103,  1104,   899,   675,
     676,   677,    -1,    -1,    -1,   681,   682,   683,    -1,    -1,
      -1,   687,   688,    -1,    -1,    -1,    -1,    -1,    -1,   920,
     921,    -1,    -1,   753,   754,   755,   756,    -1,    -1,   759,
      -1,   707,   708,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   771,    -1,    -1,   774,    -1,    -1,    -1,   434,   779,
     780,   781,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   792,    -1,
      -1,    -1,   650,   651,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    30,   814,    -1,    -1,    -1,   818,    -1,
     668,    -1,    -1,    -1,   437,    -1,    -1,    -1,   822,   823,
      -1,    -1,   826,   827,    51,    52,    53,    -1,   832,   833,
     834,    -1,    -1,   403,    -1,    27,    -1,    -1,    30,    31,
      -1,    -1,   846,   847,   848,   849,   850,   947,    75,    -1,
      42,    78,   856,    -1,   858,   865,   860,   861,   862,    51,
      52,    53,    -1,    55,   138,    -1,    -1,    -1,    -1,    -1,
     874,    -1,   146,    65,   878,    67,   103,    -1,    -1,    -1,
      -1,    -1,    -1,   983,   984,    22,   890,   891,    -1,   459,
     460,   461,   462,   463,   464,  1087,    -1,   997,    -1,   999,
    1000,    -1,    39,    -1,   474,   475,    -1,   911,   912,    -1,
      -1,   103,    49,    -1,    -1,    -1,   486,     4,  1110,    -1,
     924,   925,    -1,    -1,    61,    62,    -1,    -1,    -1,    66,
      -1,    -1,    69,    -1,    -1,    -1,   506,   941,    75,    26,
      27,    -1,    29,    -1,    31,   949,   950,   951,   952,   953,
     954,   521,    -1,    40,    -1,    42,   457,   527,    -1,    46,
      47,    48,    -1,    50,    -1,    52,    53,    -1,    55,  1161,
      57,    58,    -1,    -1,    -1,   545,    31,    -1,    -1,    -1,
      67,    -1,   552,   553,    -1,    -1,    73,    42,    -1,    76,
      77,   947,    79,    -1,    81,    -1,   566,   567,   568,  1191,
      -1,    -1,    -1,    -1,  1196,  1015,  1016,    -1,  1200,    96,
      -1,    98,    67,   583,   584,    -1,   103,    -1,    73,    -1,
     643,    -1,   645,    -1,   647,    80,    -1,   983,   984,    -1,
      -1,   987,    -1,    -1,   657,    -1,    -1,   660,    -1,  1049,
     996,   997,   998,   999,  1000,  1001,    -1,    -1,   103,    -1,
    1060,  1061,    -1,    -1,   677,     4,    -1,    -1,    -1,    -1,
     630,   631,    -1,  1163,   687,   688,    -1,    -1,  1072,  1073,
      -1,    -1,    -1,    -1,    -1,   349,   350,    26,  1082,    27,
      29,    -1,    31,    31,   707,   708,    -1,    -1,   946,    -1,
      -1,    40,    -1,    -1,    42,  1099,  1100,    -1,    47,    48,
      -1,    50,    -1,    51,    -1,    -1,    -1,    55,    57,    -1,
    1114,    -1,    -1,    -1,    -1,    -1,    -1,  1121,    67,    67,
      -1,    -1,    -1,    -1,    73,    73,    -1,    76,    -1,    -1,
      79,    -1,    -1,    81,    -1,    -1,    -1,    -1,  1142,    -1,
      -1,    -1,  1152,    -1,    -1,    -1,    -1,    96,  1158,    98,
      -1,    -1,    -1,    -1,   103,   103,    -1,    -1,    -1,    -1,
     434,  1165,    -1,  1167,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
    1136,  1137,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    42,    -1,    -1,    45,    -1,     4,    -1,
      -1,    -1,    -1,    -1,   108,    -1,    -1,  1163,    -1,    -1,
      -1,    60,    -1,    -1,   715,   716,    -1,    -1,    67,   720,
      26,    27,   792,    29,    73,    31,   727,    76,    -1,    -1,
      -1,    -1,   898,   899,    40,    -1,    42,    -1,    -1,    -1,
     741,    47,    48,   744,    50,    -1,   869,    -1,    -1,    55,
      -1,    57,    -1,    -1,   103,    -1,   922,   923,     4,    65,
      -1,    67,   763,    -1,    -1,   766,    -1,    73,    -1,    -1,
      76,    -1,   773,    79,    -1,    81,   777,    -1,    -1,    -1,
      26,    -1,   783,    29,    -1,    -1,    -1,   788,    -1,   790,
      96,    -1,    98,    -1,    40,   199,   200,   103,    -1,    -1,
       4,    47,    48,   207,    50,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    -1,    -1,
      -1,   891,    26,    -1,   947,    29,    -1,    -1,   232,    -1,
      76,    35,    36,    79,    38,    -1,    40,    41,    -1,    -1,
      -1,   911,   912,    47,    48,    -1,    50,   848,   252,   253,
      -1,   255,    98,    57,   924,   925,    -1,   103,    -1,    -1,
     983,   984,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      -1,   941,    76,    -1,   997,    79,   999,  1000,    -1,   949,
     950,   951,   952,   953,   954,    27,    -1,    -1,    30,    31,
    1168,  1169,   296,    -1,    98,    -1,    -1,  1175,    -1,  1177,
      42,  1179,    -1,    -1,  1182,  1183,    -1,    -1,    -1,  1187,
      52,    53,    -1,    55,  1192,    -1,    58,    -1,  1196,    -1,
      -1,  1199,    -1,    -1,    -1,    67,    -1,    -1,    -1,     4,
      -1,    73,    -1,    75,    -1,    -1,    78,  1103,  1104,    81,
      -1,    -1,   346,    -1,   348,    -1,    -1,    -1,   352,    -1,
      25,    26,    27,   357,    29,    30,    31,   361,    -1,    -1,
     364,   103,    -1,   367,  1087,    -1,   370,    42,  1134,   373,
      -1,   375,    47,    48,   378,    50,    51,   381,    -1,    -1,
     384,   385,   386,   387,    -1,    -1,   390,  1110,    -1,   393,
     394,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,
     404,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
    1011,  1012,     4,    -1,    -1,    -1,    -1,  1018,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,  1161,    31,
    1163,    -1,    -1,    -1,  1114,    -1,    -1,  1048,    -1,    -1,
      42,  1121,  1053,    -1,    -1,    47,    48,    -1,    50,    -1,
      -1,    -1,  1063,    -1,    -1,    57,    -1,    -1,  1191,    -1,
      -1,    -1,  1142,  1196,    -1,    67,    -1,  1200,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    79,     4,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,    -1,    98,    -1,    -1,    25,
      26,   103,    -1,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   898,   899,    42,    -1,    -1,    -1,
      -1,    47,    48,    -1,    50,    -1,   540,   541,   542,   543,
     544,    57,    -1,    -1,    -1,    -1,    -1,    -1,   922,   923,
    1151,    67,    -1,    -1,    -1,    -1,  1157,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,  1170,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      96,    -1,    98,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    42,
      -1,    -1,    -1,    -1,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    55,   627,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    76,   648,   649,    79,    -1,   652,   653,
     654,   655,   656,    -1,   658,   659,    -1,   661,   662,   663,
     664,   665,   666,   667,    -1,    98,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,   678,   679,   680,    -1,    -1,    -1,
     684,   685,   686,    -1,    -1,   689,   690,   691,   692,   693,
     694,   695,   696,   697,   698,   699,    -1,    -1,   702,   703,
     704,   705,    -1,    -1,    -1,   709,   710,   711,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1103,
    1104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   829,    -1,    -1,    -1,    -1,
      -1,    -1,   836,    -1,    -1,   839,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   852,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   871,   872,   873,
      -1,   875,    -1,    -1,    -1,    -1,    -1,    -1,   882,    -1,
     884,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   902,    -1,
      -1,   905,    -1,    -1,   908,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   916,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   960,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   982,    -1,
      -1,   985,   986,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1002,    -1,
      -1,    -1,  1006,  1007,  1008,  1009,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1026,    -1,    -1,    -1,    -1,  1031,    -1,    -1,
    1034,    -1,    -1,    -1,    -1,    -1,    -1,  1041,    -1,  1043,
    1044,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1065,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,  1083,
      -1,  1085,    -1,    10,    11,    -1,    -1,    -1,    -1,  1093,
      -1,  1095,  1096,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,   103,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
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
       0,   112,   118,   122,   420,     0,     5,     7,    54,    74,
     419,   419,   421,   421,   420,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   113,   114,   117,
     405,   426,   427,   121,   405,   420,   102,   420,   102,   115,
     409,   409,   101,    11,   119,   120,   410,   419,   123,   124,
     420,   411,   127,   128,    56,    64,    82,   125,   126,   139,
     127,    23,   125,   138,     8,   412,   413,   421,   140,    43,
      44,   129,   133,   421,   421,   421,   140,   421,   116,   102,
     104,   416,     8,     9,    11,    12,    16,    17,    18,   414,
     425,   426,    28,    31,    59,    67,   141,   142,   143,   144,
     145,   147,   421,   421,   114,   423,   425,   114,   404,   427,
     423,   145,   137,   405,   420,   420,   420,   420,   421,   421,
     421,   421,   152,   153,    70,   130,   405,   132,   405,   102,
     104,   408,   424,   408,   420,   408,   152,   102,   417,   415,
     423,   423,   423,   423,   103,     4,    21,    26,    29,    35,
      36,    38,    40,    41,    47,    48,    50,    57,    72,    76,
      79,    98,   154,   156,   167,   172,   177,   222,   225,   227,
     237,   244,   250,   260,   274,   276,   282,   298,   308,   325,
     333,   421,   102,   102,   104,   134,   409,   409,   420,   103,
     409,    11,   103,   406,   426,   101,   408,   408,   408,   408,
     420,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   421,   421,   421,   421,   421,   409,    12,
     114,   146,   150,   409,   409,   409,   103,   420,   139,   412,
     420,   273,   405,   105,   114,   296,   297,   376,   377,   259,
     405,   226,   405,   155,   405,   114,   332,   335,   377,   166,
     405,   221,   405,   171,   405,   238,   405,   243,   405,   249,
     405,   324,   405,   307,   405,   176,   405,    11,   114,   281,
     407,   427,   428,   275,   405,   151,   102,   104,   148,   131,
     135,   409,   103,   102,   104,   418,     8,   102,   104,   277,
     406,   102,   378,   420,   102,   104,   261,   102,   104,   228,
     102,   104,   157,   102,   420,   102,   104,   168,   102,   104,
     223,   102,   104,   173,   102,   102,   102,   102,   104,   326,
     102,   104,   309,   102,   102,   104,   283,   420,   420,   277,
     420,   409,   409,    71,   103,   136,   139,   143,   144,   103,
     136,   143,   144,   409,   420,   420,   420,   409,   409,   376,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   149,   421,   409,   417,   278,   299,
     262,   229,   158,   334,   169,   224,   174,   239,   245,   251,
     327,   310,   178,   284,   103,   143,   144,   150,   103,    27,
      42,    51,    55,    73,    81,   103,   143,   144,   164,   170,
     352,   355,   357,   367,    25,    96,   103,   143,   144,   164,
     170,   225,   268,   306,   325,   352,    30,   103,   143,   144,
     164,   170,   220,   227,   237,   244,   250,   260,   265,   266,
     268,   274,   276,   352,   355,   357,    65,   103,   143,   144,
     164,   170,   177,   222,   225,   306,   325,   352,   355,   359,
     367,    20,   103,   143,   144,   159,   164,    37,   103,   143,
     144,   103,   143,   144,   164,   170,   103,   143,   144,   164,
     177,   222,   225,   306,   325,    22,   103,   143,   144,   164,
     170,   175,    75,    78,   103,   143,   144,   164,   170,   179,
     219,   220,   352,   355,   357,   367,    52,    53,    58,   103,
     143,   144,   164,   170,   179,   219,   220,   352,   355,   360,
     362,   364,   367,    46,    77,   103,   143,   144,   164,   170,
     177,   222,   225,   306,   325,   352,   355,   360,   362,   364,
     367,   369,   372,   103,   143,   144,   164,   170,   177,   222,
     225,   367,    45,    60,   103,   143,   144,   164,   170,   177,
     222,   315,   322,   103,   143,   144,   164,   179,   219,   220,
      21,    68,   103,   143,   144,   164,   170,   290,   295,   352,
     408,   420,   421,   421,   421,   421,   421,   421,   279,   280,
     421,   421,   301,   304,   302,   303,   300,   421,   264,   263,
     421,   235,   232,   236,   233,   234,   230,   231,   421,   421,
     409,   409,   409,   409,   409,   421,   421,   421,   241,   240,
     242,   421,   421,   421,   247,   246,   248,   421,   421,   254,
     255,   258,   256,   257,   252,   253,   329,   330,   331,   328,
     420,   420,   311,   312,   313,   314,   409,   421,   421,   286,
     287,   285,    88,    93,   114,   356,   423,    88,    93,   114,
     358,   366,   423,    85,    87,    90,   114,   165,   351,   423,
     409,   409,   267,   405,   305,   405,   409,   409,   409,   409,
     409,   423,   409,   409,   423,   409,   409,   409,   409,   409,
     409,   409,   405,    84,    86,    89,    91,   336,   337,   338,
     339,   343,   347,   407,   180,   407,   423,   409,   409,   409,
      17,    94,   114,   363,   400,    18,   114,   361,   400,   401,
      92,    95,   114,   365,   409,   409,   409,   114,   373,   406,
     114,   370,   379,   406,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   316,   323,   409,   409,   409,
     409,   114,   293,   294,   379,   114,   288,   289,   379,   409,
     409,   409,   420,   420,   408,   408,   420,   420,   408,   102,
     104,   368,   420,   420,   420,   408,   102,   104,   353,   102,
     104,   269,   309,   408,   408,   160,   420,   420,   420,   420,
     420,   408,   102,   104,   181,   408,   420,   408,   420,   408,
     420,   420,   420,   408,   408,   374,   408,   371,   408,   421,
     380,   102,   102,   102,   420,   102,   104,   291,   420,   409,
     409,   409,   102,   104,   161,   340,   344,   408,   348,   409,
     420,   379,   378,   409,   409,   409,   409,   190,   354,   270,
     409,   409,   102,   104,   341,   102,   104,   345,   102,   104,
     349,    24,    32,   182,   183,   196,   198,   209,   211,   371,
     317,   317,   299,   292,    33,    34,   103,   143,   144,   217,
     218,   103,   143,   144,   103,   143,   144,   164,   170,   225,
     352,    83,   162,   409,   409,   409,   421,   421,   103,    39,
      49,    61,    62,    66,    69,   175,   184,   185,   187,   191,
     204,   206,   207,   409,   210,   103,   177,   222,   225,   367,
     103,   103,   103,   143,   144,   170,   220,   355,   357,   359,
     360,   362,   367,   421,   421,   272,   271,   421,   103,   342,
     346,   350,   212,   405,   199,   423,   421,   421,   421,   421,
     421,   421,   179,   409,   197,   211,   319,   320,   321,   318,
     409,   423,   423,   409,   409,    88,    93,   114,   163,   103,
     219,   220,   355,   357,   360,   362,   367,   369,   103,   219,
     220,   367,   369,   103,   179,   219,   220,   355,   357,   360,
     362,   102,   104,   213,   102,   104,   200,   114,   186,   400,
     188,   423,   114,   381,   382,   383,   192,   423,   375,   423,
      88,    93,   114,   208,   409,   198,   409,   409,   409,   409,
     408,   408,   420,   420,   408,   409,   409,   409,   409,   409,
     409,   408,   420,   102,   104,   189,   408,   105,   384,   385,
      14,   387,   388,   102,   104,   193,   102,   104,   205,   420,
     420,   408,   409,   214,   201,   409,   409,   406,   386,   105,
     389,   409,   409,   409,    63,   103,   143,   144,   164,   170,
     215,    80,   103,   143,   144,   164,   170,   202,   190,   392,
     384,   388,   390,   406,   194,   190,   421,   421,   103,   106,
     391,   392,    97,   103,   143,   144,   195,   217,   218,   103,
     114,   216,   401,    16,    17,    18,   114,   203,   402,   403,
       5,   422,   385,   421,   408,   420,   408,   420,   393,   406,
     423,   422,   422,   408,   107,   108,   422,    85,   394,   399,
     422,   422,   109,   105,   422,   422,   110,    14,   395,   396,
     422,   397,   105,   396,   406,   422,   398,   406,   422,   105,
     422,   406
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   111,   112,   112,   113,   114,   115,   116,   115,   117,
     118,   119,   120,   120,   120,   120,   121,   122,   123,   124,
     124,   124,   125,   126,   127,   128,   128,   128,   129,   130,
     131,   131,   131,   131,   131,   132,   133,   134,   134,   135,
     135,   135,   135,   136,   137,   138,   139,   140,   140,   140,
     140,   140,   141,   142,   143,   144,   145,   146,   147,   147,
     148,   148,   149,   149,   149,   151,   150,   150,   152,   153,
     153,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   155,   156,   157,   157,   158,   158,   158,   158,   158,
     160,   159,   161,   161,   162,   162,   163,   163,   163,   164,
     165,   165,   165,   165,   166,   167,   168,   168,   169,   169,
     169,   169,   169,   170,   171,   172,   173,   173,   174,   174,
     174,   174,   174,   174,   175,   176,   177,   178,   178,   178,
     178,   178,   178,   178,   179,   180,   181,   181,   182,   182,
     182,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     184,   185,   186,   186,   187,   188,   189,   189,   190,   190,
     190,   190,   190,   191,   192,   193,   193,   194,   194,   194,
     194,   194,   194,   195,   196,   197,   197,   198,   199,   200,
     200,   201,   201,   201,   201,   201,   201,   202,   203,   203,
     204,   205,   205,   206,   207,   208,   208,   208,   209,   210,
     210,   211,   212,   213,   213,   214,   214,   214,   214,   214,
     214,   215,   216,   216,   217,   218,   219,   220,   221,   222,
     223,   223,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   225,   225,   225,   225,   225,   225,   225,   225,   226,
     227,   228,   228,   229,   230,   229,   229,   231,   229,   229,
     229,   229,   229,   229,   232,   229,   233,   229,   234,   229,
     235,   229,   236,   229,   237,   238,   239,   240,   239,   239,
     241,   239,   239,   242,   239,   239,   239,   239,   239,   239,
     239,   243,   244,   245,   246,   245,   245,   247,   245,   245,
     245,   248,   245,   245,   245,   245,   245,   245,   245,   245,
     249,   250,   251,   252,   251,   251,   253,   251,   251,   251,
     251,   251,   251,   251,   251,   251,   251,   254,   251,   255,
     251,   256,   251,   257,   251,   258,   251,   259,   260,   261,
     261,   262,   263,   262,   262,   262,   262,   262,   262,   262,
     262,   264,   262,   265,   265,   266,   266,   266,   266,   266,
     266,   266,   267,   268,   269,   269,   270,   271,   270,   270,
     270,   270,   270,   272,   270,   273,   274,   275,   276,   277,
     277,   278,   279,   278,   278,   280,   278,   278,   278,   278,
     278,   278,   281,   282,   283,   283,   284,   285,   284,   284,
     284,   284,   284,   286,   284,   287,   284,   288,   288,   289,
     290,   291,   291,   292,   292,   292,   292,   292,   292,   292,
     292,   292,   292,   292,   293,   293,   294,   295,   296,   296,
     297,   298,   299,   300,   299,   299,   299,   299,   299,   301,
     299,   302,   299,   303,   299,   304,   299,   305,   306,   307,
     308,   309,   309,   310,   310,   310,   310,   310,   311,   310,
     312,   310,   313,   310,   314,   310,   316,   315,   317,   318,
     317,   319,   317,   320,   317,   321,   317,   323,   322,   324,
     325,   326,   326,   327,   328,   327,   327,   327,   327,   327,
     329,   327,   330,   327,   331,   327,   332,   333,   334,   334,
     334,   334,   335,   335,   336,   336,   337,   338,   338,   338,
     340,   339,   341,   341,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   344,   343,   345,   345,   346,   346,   346,
     346,   346,   348,   347,   349,   349,   350,   350,   350,   350,
     350,   350,   350,   350,   351,   352,   353,   353,   354,   354,
     354,   355,   356,   356,   356,   357,   358,   358,   358,   359,
     360,   361,   361,   362,   363,   363,   363,   364,   365,   365,
     365,   366,   367,   368,   368,   369,   370,   370,   371,   371,
     372,   374,   373,   373,   375,   376,   377,   378,   378,   380,
     379,   382,   381,   383,   381,   381,   384,   385,   386,   386,
     387,   388,   389,   389,   390,   391,   391,   392,   392,   393,
     394,   395,   396,   397,   397,   398,   398,   399,   400,   401,
     401,   402,   402,   403,   403,   404,   404,   405,   405,   406,
     406,   407,   407,   407,   408,   408,   409,   409,   409,   411,
     410,   412,   413,   413,   414,   414,   414,   415,   415,   416,
     416,   417,   417,   418,   418,   419,   419,   420,   420,   421,
     422,   422,   424,   423,   423,   425,   425,   425,   425,   425,
     425,   425,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   427,   428
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     0,     0,     6,     1,
      13,     1,     0,     2,     2,     2,     1,    13,     1,     0,
       2,     2,     4,     4,     1,     0,     2,     2,     8,     1,
       0,     2,     2,     2,     2,     1,     5,     1,     4,     0,
       2,     2,     2,     4,     1,     8,     4,     0,     2,     2,
       2,     2,     4,     4,     4,     4,     1,     1,     0,     6,
       1,     4,     0,     2,     2,     0,     3,     1,     1,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     4,     0,     2,     2,     2,     2,
       0,     6,     1,     4,     0,     4,     2,     2,     1,     4,
       2,     2,     2,     1,     1,     4,     1,     4,     0,     2,
       2,     2,     2,     4,     1,     4,     1,     4,     0,     2,
       2,     2,     2,     2,     4,     1,     7,     0,     3,     2,
       2,     2,     2,     2,     4,     1,     1,     4,     1,     1,
       1,     0,     2,     2,     2,     2,     2,     2,     3,     4,
       0,     4,     2,     1,     5,     1,     1,     4,     0,     2,
       2,     2,     2,     5,     1,     1,     4,     0,     2,     2,
       2,     2,     2,     4,     3,     0,     3,     4,     1,     1,
       4,     0,     2,     2,     2,     2,     2,     4,     2,     1,
       4,     1,     4,     4,     4,     2,     2,     1,     2,     0,
       2,     5,     1,     1,     4,     0,     2,     2,     2,     2,
       2,     4,     2,     1,     4,     4,     4,     4,     1,     4,
       1,     4,     0,     2,     2,     2,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     4,     0,     0,     4,     2,     0,     4,     2,
       2,     2,     2,     2,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     7,     1,     0,     0,     4,     2,
       0,     4,     2,     0,     4,     2,     2,     2,     2,     2,
       2,     1,     7,     0,     0,     4,     2,     0,     4,     2,
       2,     0,     4,     2,     2,     2,     2,     2,     2,     2,
       1,     7,     0,     0,     4,     2,     0,     4,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     1,     4,     1,
       4,     0,     0,     4,     2,     2,     2,     2,     2,     2,
       2,     0,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     4,     0,     0,     4,     2,
       2,     2,     2,     0,     4,     1,     4,     1,     4,     1,
       4,     0,     0,     4,     2,     0,     4,     2,     2,     2,
       2,     2,     1,     4,     1,     4,     0,     0,     4,     2,
       2,     2,     2,     0,     4,     0,     4,     2,     1,     1,
       4,     1,     4,     0,     3,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     7,     2,     1,
       1,     7,     0,     0,     4,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     4,     0,     4,     1,     4,     1,
       4,     1,     4,     0,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     7,     0,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     7,     1,
       4,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     1,     7,     0,     2,
       2,     4,     2,     1,     1,     2,     3,     1,     1,     1,
       0,     4,     1,     4,     0,     2,     3,     2,     2,     2,
       2,     2,     2,     0,     4,     1,     4,     0,     2,     3,
       2,     2,     0,     4,     1,     4,     0,     3,     2,     2,
       2,     2,     2,     2,     1,     4,     1,     4,     0,     2,
       2,     4,     2,     2,     1,     4,     2,     2,     1,     4,
       4,     2,     1,     4,     2,     2,     1,     4,     2,     2,
       1,     1,     4,     1,     4,     3,     2,     2,     3,     1,
       4,     0,     3,     1,     1,     2,     2,     0,     2,     0,
       3,     0,     2,     0,     2,     1,     3,     2,     0,     2,
       3,     2,     0,     2,     2,     0,     2,     0,     6,     5,
       5,     5,     4,     0,     2,     0,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     2,     2,     1,     2,     4,     0,     2,     2,     0,
       4,     2,     0,     1,     0,     2,     3,     0,     5,     1,
       4,     0,     4,     2,     5,     1,     1,     0,     2,     2,
       0,     1,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1
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

    case 192: /* pattern_arg_str  */

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

  case 24:

    { *param->remove_import = 0;
                              if (yang_check_imports(trg, param->unres)) {
                                YYABORT;
                              }
                            }

    break;

  case 29:

    { YANG_ADDELEM(trg->imp, trg->imp_size);
                                     /* HACK for unres */
                                     ((struct lys_import *)actual)->module = (struct lys_module *)s;
                                     s = NULL;
                                     actual_type = IMPORT_KEYWORD;
                                   }

    break;

  case 30:

    { (yyval.i) = 0; }

    break;

  case 32:

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

  case 33:

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

  case 34:

    { if ((yyvsp[-1].i)) {
                                            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "import");
                                            YYABORT;
                                          }
                                          (yyval.i) = 1;
                                        }

    break;

  case 35:

    { YANG_ADDELEM(trg->inc, trg->inc_size);
                                     /* HACK for unres */
                                     ((struct lys_include *)actual)->submodule = (struct lys_submodule *)s;
                                     s = NULL;
                                     actual_type = INCLUDE_KEYWORD;
                                   }

    break;

  case 39:

    { (yyval.i) = 0; }

    break;

  case 40:

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

  case 41:

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

  case 42:

    { if ((yyvsp[-1].i)) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "include");
                                             YYABORT;
                                           }
                                           (yyval.i) = 1;
                                         }

    break;

  case 43:

    { if (actual_type==IMPORT_KEYWORD) {
                                memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                              } else {                              // INCLUDE KEYWORD
                                memcpy(((struct lys_include *)actual)->rev, s, LY_REV_SIZE-1);
                              }
                              free(s);
                              s = NULL;
                            }

    break;

  case 44:

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

  case 46:

    { if (yang_read_prefix(trg, actual, s)) {
                                                           YYABORT;
                                                         }
                                                         s = NULL;
                                                       }

    break;

  case 48:

    { if (yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 49:

    { if (yang_read_common(trg, s, CONTACT_KEYWORD)) {
                                 YYABORT;
                               }
                               s = NULL;
                             }

    break;

  case 50:

    { if (yang_read_description(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 51:

    { if (yang_read_reference(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 56:

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

  case 57:

    { YANG_ADDELEM(trg->rev, trg->rev_size);
                                  yang_read_revision(trg, s, actual);
                                }

    break;

  case 63:

    { if (yang_read_description(trg, actual, s, "revision",REVISION_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 64:

    { if (yang_read_reference(trg, actual, s, "revision", REVISION_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 65:

    { s = strdup(yyget_text(scanner));
                              if (!s) {
                                LOGMEM;
                                YYABORT;
                              }
                            }

    break;

  case 67:

    { if (lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 68:

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

  case 69:

    { actual = NULL; }

    break;

  case 70:

    { actual = NULL; }

    break;

  case 81:

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

  case 85:

    { (yyval.uint) = 0; }

    break;

  case 86:

    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }

    break;

  case 87:

    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }

    break;

  case 88:

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

  case 89:

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

  case 90:

    { free(s); s = NULL; }

    break;

  case 98:

    { if (strcmp(s, "true") && strcmp(s, "false")) {
                 LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                 free(s);
                 YYABORT;
               }
               free(s);
               s = NULL;
             }

    break;

  case 99:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 100:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 101:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 102:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 103:

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

  case 104:

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

  case 107:

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

  case 109:

    { void *feature;

                                        feature = actual;
                                        YANG_ADDELEM(((struct lys_feature *)actual)->iffeature,
                                                     ((struct lys_feature *)actual)->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                                        s = NULL;
                                        actual = feature;
                                      }

    break;

  case 110:

    { if (((struct lys_feature *)actual)->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "feature");
                                      YYABORT;
                                    }
                                    ((struct lys_feature *)actual)->flags |= (yyvsp[0].i);
                                  }

    break;

  case 111:

    { if (yang_read_description(trg, actual, s, "feature", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 112:

    { if (yang_read_reference(trg, actual, s, "feature", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 114:

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

  case 117:

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

  case 119:

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

  case 120:

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

  case 121:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 122:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 123:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 125:

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

  case 126:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual = tpdf_parent;
                  }

    break;

  case 127:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 128:

    { actual = (yyvsp[-2].nodes).node.ptr_tpdf;
                                       actual_type = TYPEDEF_KEYWORD;
                                       (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 129:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 130:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 131:

    { if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 132:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 133:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 135:

    { if (!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 138:

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

  case 142:

    { if (yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 143:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 144:

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

  case 149:

    { actual = (yyvsp[-2].v); }

    break;

  case 150:

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

  case 151:

    { if (yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 152:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 153:

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

  case 154:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 155:

    { (yyval.v) = actual;
                         if (!(actual = yang_read_length(trg, actual, s))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 158:

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

  case 159:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 160:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 161:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 162:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 163:

    { if (yang_read_pattern(trg, actual, (yyvsp[-2].str), (yyvsp[-1].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual = yang_type;
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 164:

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

  case 165:

    { (yyval.ch) = 0x06; }

    break;

  case 166:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 167:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 168:

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

  case 169:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 170:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 171:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 172:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 173:

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

  case 174:

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

  case 177:

    { if (yang_check_enum(yang_type, actual, &cnt_val, actual_type)) {
               YYABORT;
             }
             actual = yang_type;
             actual_type = TYPE_KEYWORD;
           }

    break;

  case 178:

    { yang_type = actual;
                       YANG_ADDELEM(((struct yang_type *)actual)->type->info.enums.enm, ((struct yang_type *)actual)->type->info.enums.count);
                       if (yang_read_enum(trg, yang_type, actual, s)) {
                         YYABORT;
                       }
                       s = NULL;
                       actual_type = 0;
                     }

    break;

  case 180:

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

  case 182:

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

  case 183:

    { /* actual_type - it is used to check value of enum statement*/
                                if (actual_type) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                actual_type = 1;
                              }

    break;

  case 184:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 185:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 186:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 187:

    { ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                      /* keep the highest enum value for automatic increment */
                      if ((yyvsp[-1].i) >= cnt_val) {
                        cnt_val = (yyvsp[-1].i);
                        cnt_val++;
                      }
                    }

    break;

  case 188:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 189:

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

  case 190:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 194:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 195:

    { (yyval.i) = 1; }

    break;

  case 196:

    { (yyval.i) = -1; }

    break;

  case 197:

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

  case 198:

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

  case 201:

    { if (yang_check_bit(yang_type, actual, &cnt_val, actual_type)) {
                      YYABORT;
                    }
                    actual = yang_type;
                    actual_type = TYPE_KEYWORD;
                  }

    break;

  case 202:

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

  case 204:

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

  case 206:

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

  case 207:

    { /* actual_type - it is used to check position of bit statement*/
                                  if (actual_type) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  actual_type = 1;
                                }

    break;

  case 208:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags,
                                                     LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                  YYABORT;
                                }
                              }

    break;

  case 209:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 210:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 211:

    { ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                         /* keep the highest position value for automatic increment */
                         if ((yyvsp[-1].uint) >= cnt_val) {
                           cnt_val = (yyvsp[-1].uint);
                           cnt_val++;
                         }
                       }

    break;

  case 212:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 213:

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

  case 218:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_GROUPING, sizeof(struct lys_node_grp)))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       data_node = actual;
                                     }

    break;

  case 219:

    { LOGDBG("YANG: finished parsing grouping statement \"%s\"", data_node->name); }

    break;

  case 222:

    { (yyval.nodes).grouping = actual;
                                actual_type = GROUPING_KEYWORD;
                              }

    break;

  case 223:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 224:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 225:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
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
                                           }

    break;

  case 230:

    { actual = (yyvsp[-2].nodes).grouping;
                                                   actual_type = GROUPING_KEYWORD;
                                                   if (trg->version < 2) {
                                                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 239:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CONTAINER, sizeof(struct lys_node_container)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                      }

    break;

  case 240:

    { LOGDBG("YANG: finished parsing container statement \"%s\"", data_node->name); }

    break;

  case 242:

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

  case 243:

    { (yyval.nodes).container = actual;
                                 actual_type = CONTAINER_KEYWORD;
                               }

    break;

  case 244:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 246:

    { YANG_ADDELEM((yyvsp[-1].nodes).container->iffeature, (yyvsp[-1].nodes).container->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).container;
                                          s = NULL;
                                        }

    break;

  case 247:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 249:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 250:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 251:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 252:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 253:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
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

    { actual = (yyvsp[-1].nodes).container;
                                            actual_type = CONTAINER_KEYWORD;
                                            data_node = actual;
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "notification");
                                              YYABORT;
                                            }
                                          }

    break;

  case 260:

    { actual = (yyvsp[-1].nodes).container;
                                       actual_type = CONTAINER_KEYWORD;
                                     }

    break;

  case 262:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 264:

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

  case 265:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAF, sizeof(struct lys_node_leaf)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   (yyval.nodes).node.ptr_leaf = actual;
                                 }

    break;

  case 266:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LEAF_KEYWORD;
                          }

    break;

  case 267:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 269:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaf->iffeature, (yyvsp[-1].nodes).node.ptr_leaf->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                     s = NULL;
                                   }

    break;

  case 270:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                               (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                             }

    break;

  case 271:

    { (yyval.nodes) = (yyvsp[-3].nodes);}

    break;

  case 272:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 273:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 275:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 276:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 277:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 278:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 279:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 280:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 281:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAFLIST, sizeof(struct lys_node_leaflist)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                      }

    break;

  case 282:

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

  case 283:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                                 actual_type = LEAF_LIST_KEYWORD;
                               }

    break;

  case 284:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 286:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaflist->iffeature,
                                                       (yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                          s = NULL;
                                        }

    break;

  case 287:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                                  }

    break;

  case 288:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 289:

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

  case 290:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 291:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 293:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 294:

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

  case 295:

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

  case 296:

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

  case 297:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 298:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 299:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 300:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LIST, sizeof(struct lys_node_list)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 301:

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

  case 302:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LIST_KEYWORD;
                          }

    break;

  case 303:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 305:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->iffeature,
                                                  (yyvsp[-1].nodes).node.ptr_list->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_list;
                                     s = NULL;
                                   }

    break;

  case 306:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 308:

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

  case 309:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 310:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 311:

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

  case 312:

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

  case 313:

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

  case 314:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 315:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 316:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 317:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                  actual_type = LIST_KEYWORD;
                                }

    break;

  case 319:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
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
                                       if (trg->version < 2) {
                                         LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "notification");
                                         YYABORT;
                                       }
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

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CHOICE, sizeof(struct lys_node_choice)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                   }

    break;

  case 328:

    { LOGDBG("YANG: finished parsing choice statement \"%s\"", data_node->name); }

    break;

  case 330:

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

  case 331:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                              actual_type = CHOICE_KEYWORD;
                            }

    break;

  case 332:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                 actual_type = CHOICE_KEYWORD;
                               }

    break;

  case 333:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 334:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_choice->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_choice->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_choice;
                                       s = NULL;
                                     }

    break;

  case 335:

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

  case 336:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 337:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 338:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 339:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 340:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 341:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                          }

    break;

  case 342:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 351:

    { if (trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 352:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CASE, sizeof(struct lys_node_case)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 353:

    { LOGDBG("YANG: finished parsing case statement \"%s\"", data_node->name); }

    break;

  case 355:

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

  case 356:

    { (yyval.nodes).cs = actual;
                            actual_type = CASE_KEYWORD;
                          }

    break;

  case 357:

    { actual = (yyvsp[-1].nodes).cs;
                               actual_type = CASE_KEYWORD;
                             }

    break;

  case 359:

    { YANG_ADDELEM((yyvsp[-1].nodes).cs->iffeature, (yyvsp[-1].nodes).cs->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).cs;
                                     s = NULL;
                                   }

    break;

  case 360:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 361:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 362:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 363:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 365:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYXML, sizeof(struct lys_node_anydata)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ANYXML_KEYWORD;
                                   }

    break;

  case 366:

    { LOGDBG("YANG: finished parsing anyxml statement \"%s\"", data_node->name); }

    break;

  case 367:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYDATA, sizeof(struct lys_node_anydata)))) {
                                        YYABORT;
                                      }
                                      data_node = actual;
                                      s = NULL;
                                      actual_type = ANYDATA_KEYWORD;
                                    }

    break;

  case 368:

    { LOGDBG("YANG: finished parsing anydata statement \"%s\"", data_node->name); }

    break;

  case 370:

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

  case 371:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
                            }

    break;

  case 372:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 374:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_anydata->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_anydata->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                       s = NULL;
                                     }

    break;

  case 375:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 377:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "config",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 378:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_MAND_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "mandatory",
                                               ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                    }

    break;

  case 379:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "status",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 380:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 381:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 382:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_USES, sizeof(struct lys_node_uses)))) {
                                         YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                     }

    break;

  case 383:

    { LOGDBG("YANG: finished parsing uses statement \"%s\"", data_node->name); }

    break;

  case 385:

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

  case 386:

    { (yyval.nodes).uses = actual;
                            actual_type = USES_KEYWORD;
                          }

    break;

  case 387:

    { actual = (yyvsp[-1].nodes).uses;
                               actual_type = USES_KEYWORD;
                             }

    break;

  case 389:

    { YANG_ADDELEM((yyvsp[-1].nodes).uses->iffeature, (yyvsp[-1].nodes).uses->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).uses;
                                     s = NULL;
                                   }

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

  case 393:

    { actual = (yyvsp[-1].nodes).uses;
                                 actual_type = USES_KEYWORD;
                               }

    break;

  case 395:

    { actual = (yyvsp[-1].nodes).uses;
                                       actual_type = USES_KEYWORD;
                                       data_node = actual;
                                     }

    break;

  case 399:

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

  case 402:

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

  case 403:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 404:

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

  case 405:

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

  case 406:

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

  case 407:

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

  case 408:

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

  case 409:

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

  case 410:

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

  case 411:

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

  case 412:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 413:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 416:

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

  case 420:

    { YANG_ADDELEM(trg->augment, trg->augment_size);
                               if (yang_read_augment(trg, NULL, actual, s)) {
                                 YYABORT;
                               }
                               data_node = actual;
                               s = NULL;
                             }

    break;

  case 422:

    { (yyval.nodes).augment = actual;
                               actual_type = AUGMENT_KEYWORD;
                             }

    break;

  case 423:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                }

    break;

  case 425:

    { YANG_ADDELEM((yyvsp[-1].nodes).augment->iffeature, (yyvsp[-1].nodes).augment->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                        actual = (yyvsp[-1].nodes).augment;
                                        s = NULL;
                                      }

    break;

  case 426:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 427:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 428:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 429:

    { actual = (yyvsp[-1].nodes).augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 431:

    { actual = (yyvsp[-1].nodes).augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    data_node = actual;
                                  }

    break;

  case 433:

    { actual = (yyvsp[-1].nodes).augment;
                                          actual_type = AUGMENT_KEYWORD;
                                          data_node = actual;
                                          if (trg->version < 2) {
                                            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "notification");
                                            YYABORT;
                                          }
                                        }

    break;

  case 435:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 437:

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

  case 438:

    { config_inherit = (yyvsp[-1].i);
                                                         LOGDBG("YANG: finished parsing action statement \"%s\"", data_node->name);
                                                       }

    break;

  case 439:

    { if (!(actual = yang_read_node(trg, NULL, param->node, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                  (yyval.i) = config_inherit;
                                  config_inherit = CONFIG_IGNORE;
                                }

    break;

  case 440:

    { config_inherit = (yyvsp[-1].i);
                                                LOGDBG("YANG: finished parsing rpc statement \"%s\"", data_node->name);
                                              }

    break;

  case 442:

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

  case 443:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                           actual_type = RPC_KEYWORD;
                         }

    break;

  case 444:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_rpc->iffeature,
                                                 (yyvsp[-1].nodes).node.ptr_rpc->iffeature_size);
                                    ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                    actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                    s = NULL;
                                  }

    break;

  case 445:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 446:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 447:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 448:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                 actual_type = RPC_KEYWORD;
                               }

    break;

  case 450:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 452:

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

  case 453:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 454:

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

  case 455:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 456:

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

  case 457:

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

  case 458:

    { (yyval.nodes).inout = actual;
                                    actual_type = INPUT_KEYWORD;
                                  }

    break;

  case 459:

    { actual = (yyvsp[-1].nodes).inout;
                                       actual_type = INPUT_KEYWORD;
                                     }

    break;

  case 461:

    { actual = (yyvsp[-1].nodes).inout;
                                          actual_type = INPUT_KEYWORD;
                                        }

    break;

  case 463:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 465:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 467:

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

  case 468:

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

  case 469:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                           (yyval.i) = config_inherit;
                                           config_inherit = CONFIG_INHERIT_DISABLE;
                                         }

    break;

  case 470:

    { config_inherit = (yyvsp[-1].i); }

    break;

  case 472:

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

  case 473:

    { (yyval.nodes).notif = actual;
                                    actual_type = NOTIFICATION_KEYWORD;
                                  }

    break;

  case 474:

    { actual = (yyvsp[-1].nodes).notif;
                                       actual_type = NOTIFICATION_KEYWORD;
                                     }

    break;

  case 476:

    { YANG_ADDELEM((yyvsp[-1].nodes).notif->iffeature, (yyvsp[-1].nodes).notif->iffeature_size);
                                             ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                             actual = (yyvsp[-1].nodes).notif;
                                             s = NULL;
                                           }

    break;

  case 477:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 478:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 479:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 480:

    { actual = (yyvsp[-1].nodes).notif;
                                          actual_type = NOTIFICATION_KEYWORD;
                                        }

    break;

  case 482:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 484:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 486:

    { YANG_ADDELEM(trg->deviation, trg->deviation_size);
                                   ((struct lys_deviation *)actual)->target_name = transform_schema2json(trg, s);
                                   free(s);
                                   if (!((struct lys_deviation *)actual)->target_name) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 487:

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

  case 488:

    { (yyval.dev) = actual;
                                 actual_type = DEVIATION_KEYWORD;
                               }

    break;

  case 489:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 490:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 491:

    { actual = (yyvsp[-3].dev);
                                                                actual_type = DEVIATION_KEYWORD;
                                                                (yyval.dev) = (yyvsp[-3].dev);
                                                              }

    break;

  case 494:

    { if (yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 500:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_ADD))) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 503:

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

  case 504:

    { (yyval.deviate) = actual;
                                   actual_type = ADD_KEYWORD;
                                 }

    break;

  case 505:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 506:

    { actual = (yyvsp[-2].deviate);
                                              actual_type = ADD_KEYWORD;
                                              (yyval.deviate) = (yyvsp[-2].deviate);
                                            }

    break;

  case 507:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 508:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 509:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 510:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 511:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 512:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 513:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_DEL))) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 516:

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

  case 517:

    { (yyval.deviate) = actual;
                                      actual_type = DELETE_KEYWORD;
                                    }

    break;

  case 518:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 519:

    { actual = (yyvsp[-2].deviate);
                                                 actual_type = DELETE_KEYWORD;
                                                 (yyval.deviate) = (yyvsp[-2].deviate);
                                               }

    break;

  case 520:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 521:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 522:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_RPL))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 525:

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

  case 526:

    { (yyval.deviate) = actual;
                                       actual_type = REPLACE_KEYWORD;
                                     }

    break;

  case 527:

    { actual = (yyvsp[-2].deviate);
                                                  actual_type = REPLACE_KEYWORD;
                                                }

    break;

  case 528:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 529:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 530:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 531:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 532:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 533:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 534:

    { if (!(actual=yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type=WHEN_KEYWORD;
                      }

    break;

  case 539:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 540:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 541:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 542:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 543:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 544:

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

  case 545:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 546:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 547:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 548:

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

  case 550:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 551:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 552:

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

  case 553:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 554:

    { (yyval.uint) = 0; }

    break;

  case 555:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 556:

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

  case 557:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 558:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 559:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 560:

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

  case 561:

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

  case 571:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }

    break;

  case 574:

    { (yyval.v) = actual;
                        if (!(actual = yang_read_range(trg, actual, s))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 575:

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

  case 579:

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

  case 581:

    { tmp_s = yyget_text(scanner); }

    break;

  case 582:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM;
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 583:

    { tmp_s = yyget_text(scanner); }

    break;

  case 584:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM;
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 608:

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

  case 609:

    { (yyval.uint) = 0; }

    break;

  case 610:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 611:

    { (yyval.i) = 0; }

    break;

  case 612:

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

  case 618:

    { if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 623:

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

  case 629:

    { if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 652:

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

  case 744:

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
