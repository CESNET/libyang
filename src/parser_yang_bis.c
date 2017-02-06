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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3215

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  351
/* YYNRULES -- Number of rules.  */
#define YYNRULES  774
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1226

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
       0,   297,   297,   298,   300,   323,   326,   328,   327,   351,
     362,   372,   382,   383,   389,   394,   397,   408,   418,   431,
     432,   438,   440,   444,   446,   450,   452,   458,   459,   460,
     462,   470,   478,   479,   480,   491,   502,   513,   521,   526,
     527,   531,   532,   543,   554,   565,   569,   571,   587,   592,
     596,   602,   603,   608,   613,   618,   624,   628,   630,   634,
     636,   640,   642,   646,   648,   661,   667,   668,   670,   674,
     675,   679,   680,   685,   692,   692,   699,   705,   753,   754,
     757,   758,   759,   760,   761,   762,   763,   764,   765,   766,
     769,   784,   791,   792,   796,   797,   798,   804,   809,   815,
     826,   828,   829,   833,   838,   839,   841,   842,   843,   856,
     861,   863,   864,   865,   866,   881,   892,   894,   895,   910,
     911,   920,   926,   931,   937,   941,   943,   956,   958,   959,
     984,   985,   999,  1012,  1018,  1023,  1029,  1033,  1035,  1084,
    1094,  1097,  1102,  1103,  1109,  1113,  1118,  1125,  1127,  1133,
    1134,  1139,  1174,  1175,  1178,  1179,  1183,  1189,  1202,  1203,
    1204,  1205,  1206,  1208,  1225,  1230,  1236,  1237,  1253,  1257,
    1265,  1266,  1271,  1287,  1292,  1297,  1302,  1308,  1315,  1328,
    1329,  1333,  1334,  1344,  1349,  1354,  1359,  1365,  1369,  1380,
    1392,  1393,  1396,  1404,  1413,  1414,  1429,  1430,  1442,  1448,
    1452,  1457,  1463,  1468,  1478,  1479,  1494,  1499,  1500,  1505,
    1509,  1511,  1516,  1518,  1519,  1520,  1533,  1545,  1546,  1548,
    1556,  1566,  1567,  1582,  1583,  1595,  1601,  1606,  1611,  1617,
    1622,  1632,  1633,  1649,  1653,  1655,  1659,  1661,  1665,  1667,
    1671,  1673,  1683,  1689,  1690,  1694,  1695,  1701,  1706,  1711,
    1714,  1717,  1720,  1723,  1731,  1732,  1733,  1734,  1735,  1736,
    1737,  1738,  1741,  1751,  1758,  1759,  1782,  1783,  1783,  1787,
    1792,  1792,  1796,  1801,  1807,  1813,  1818,  1823,  1823,  1828,
    1828,  1833,  1833,  1842,  1842,  1846,  1846,  1852,  1892,  1902,
    1905,  1905,  1909,  1914,  1914,  1919,  1924,  1924,  1928,  1933,
    1939,  1945,  1951,  1956,  1962,  1972,  2027,  2030,  2030,  2034,
    2040,  2040,  2045,  2056,  2061,  2061,  2065,  2071,  2084,  2097,
    2107,  2113,  2118,  2124,  2134,  2181,  2184,  2184,  2188,  2194,
    2194,  2198,  2207,  2213,  2219,  2232,  2245,  2255,  2261,  2266,
    2271,  2271,  2275,  2275,  2280,  2280,  2285,  2285,  2294,  2294,
    2301,  2311,  2318,  2319,  2339,  2342,  2342,  2346,  2352,  2362,
    2369,  2376,  2383,  2389,  2395,  2395,  2401,  2402,  2405,  2406,
    2407,  2408,  2409,  2410,  2411,  2418,  2428,  2435,  2436,  2450,
    2451,  2451,  2455,  2460,  2466,  2471,  2476,  2476,  2483,  2493,
    2500,  2510,  2517,  2518,  2541,  2544,  2544,  2548,  2554,  2554,
    2558,  2565,  2572,  2579,  2584,  2590,  2600,  2607,  2608,  2640,
    2641,  2641,  2645,  2650,  2656,  2661,  2666,  2666,  2670,  2670,
    2676,  2677,  2679,  2689,  2691,  2692,  2726,  2729,  2743,  2767,
    2789,  2840,  2857,  2874,  2895,  2916,  2921,  2927,  2928,  2931,
    2946,  2955,  2956,  2958,  2969,  2978,  2979,  2979,  2983,  2988,
    2994,  2999,  3004,  3004,  3009,  3009,  3014,  3014,  3023,  3023,
    3029,  3044,  3051,  3061,  3068,  3069,  3093,  3096,  3102,  3108,
    3113,  3118,  3118,  3122,  3122,  3127,  3127,  3137,  3137,  3148,
    3163,  3192,  3195,  3195,  3199,  3199,  3203,  3203,  3208,  3208,
    3214,  3229,  3258,  3267,  3274,  3275,  3307,  3310,  3310,  3314,
    3319,  3325,  3330,  3335,  3335,  3339,  3339,  3344,  3344,  3350,
    3359,  3377,  3380,  3386,  3392,  3397,  3398,  3400,  3405,  3408,
    3412,  3414,  3415,  3416,  3418,  3418,  3424,  3425,  3457,  3460,
    3466,  3470,  3476,  3482,  3489,  3496,  3504,  3513,  3513,  3519,
    3520,  3552,  3555,  3561,  3565,  3571,  3578,  3578,  3584,  3585,
    3599,  3602,  3605,  3611,  3617,  3624,  3631,  3639,  3648,  3655,
    3657,  3658,  3662,  3663,  3668,  3674,  3679,  3681,  3682,  3683,
    3696,  3701,  3703,  3704,  3705,  3718,  3722,  3724,  3729,  3731,
    3732,  3752,  3757,  3759,  3760,  3761,  3781,  3786,  3788,  3789,
    3790,  3802,  3865,  3867,  3868,  3873,  3877,  3879,  3880,  3882,
    3883,  3885,  3889,  3891,  3891,  3898,  3901,  3909,  3928,  3930,
    3931,  3934,  3934,  3951,  3951,  3958,  3958,  3965,  3968,  3970,
    3972,  3973,  3975,  3977,  3979,  3980,  3982,  3984,  3985,  3987,
    3988,  3990,  3992,  3995,  3999,  4001,  4002,  4004,  4005,  4007,
    4009,  4020,  4021,  4024,  4025,  4036,  4037,  4039,  4040,  4042,
    4043,  4049,  4050,  4053,  4054,  4055,  4079,  4080,  4083,  4089,
    4093,  4098,  4099,  4100,  4103,  4105,  4111,  4113,  4114,  4116,
    4117,  4119,  4120,  4122,  4123,  4125,  4126,  4129,  4130,  4133,
    4135,  4136,  4139,  4139,  4146,  4148,  4149,  4150,  4151,  4152,
    4153,  4154,  4156,  4157,  4158,  4159,  4160,  4161,  4162,  4163,
    4164,  4165,  4166,  4167,  4168,  4169,  4170,  4171,  4172,  4173,
    4174,  4175,  4176,  4177,  4178,  4179,  4180,  4181,  4182,  4183,
    4184,  4185,  4186,  4187,  4188,  4189,  4190,  4191,  4192,  4193,
    4194,  4195,  4196,  4197,  4198,  4199,  4200,  4201,  4202,  4203,
    4204,  4205,  4206,  4207,  4208,  4209,  4210,  4211,  4212,  4213,
    4214,  4215,  4216,  4217,  4218,  4219,  4220,  4221,  4222,  4223,
    4224,  4225,  4226,  4227,  4228,  4229,  4230,  4231,  4232,  4233,
    4234,  4235,  4236,  4239,  4246
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
  "yang_version_arg", "yang_version_stmt", "namespace_arg_str",
  "namespace_stmt", "linkage_stmt", "linkage_stmts", "import_stmt",
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
  "if_feature_arg", "if_feature_stmt", "identity_arg_str", "identity_stmt",
  "identity_end", "identity_opt_stmt", "base_arg", "base_stmt",
  "typedef_arg_str", "typedef_stmt", "type_opt_stmt", "type_stmt",
  "type_arg_str", "type_end", "type_body_stmts", "some_restrictions",
  "union_spec", "fraction_digits_arg", "fraction_digits_stmt",
  "fraction_digits_arg_str", "length_stmt", "length_arg_str", "length_end",
  "message_opt_stmt", "pattern_stmt", "pattern_arg_str", "pattern_end",
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
  "container_stmt", "container_end", "container_opt_stmt", "$@3", "$@4",
  "$@5", "$@6", "$@7", "$@8", "$@9", "leaf_stmt", "leaf_arg_str",
  "leaf_opt_stmt", "$@10", "$@11", "$@12", "leaf_list_arg_str",
  "leaf_list_stmt", "leaf_list_opt_stmt", "$@13", "$@14", "$@15",
  "list_arg_str", "list_stmt", "list_opt_stmt", "$@16", "$@17", "$@18",
  "$@19", "$@20", "$@21", "$@22", "choice_arg_str", "choice_stmt",
  "choice_end", "choice_opt_stmt", "$@23", "$@24", "short_case_case_stmt",
  "short_case_stmt", "case_arg_str", "case_stmt", "case_end",
  "case_opt_stmt", "$@25", "$@26", "anyxml_arg_str", "anyxml_stmt",
  "anydata_arg_str", "anydata_stmt", "anyxml_end", "anyxml_opt_stmt",
  "$@27", "$@28", "uses_arg_str", "uses_stmt", "uses_end", "uses_opt_stmt",
  "$@29", "$@30", "$@31", "refine_args_str", "refine_arg_str",
  "refine_stmt", "refine_end", "refine_body_opt_stmts",
  "uses_augment_arg_str", "uses_augment_arg", "uses_augment_stmt",
  "augment_arg_str", "augment_arg", "augment_stmt", "augment_opt_stmt",
  "$@32", "$@33", "$@34", "$@35", "$@36", "action_arg_str", "action_stmt",
  "rpc_arg_str", "rpc_stmt", "rpc_end", "rpc_opt_stmt", "$@37", "$@38",
  "$@39", "$@40", "input_arg", "input_stmt", "input_output_opt_stmt",
  "$@41", "$@42", "$@43", "$@44", "output_arg", "output_stmt",
  "notification_arg_str", "notification_stmt", "notification_end",
  "notification_opt_stmt", "$@45", "$@46", "$@47", "$@48", "deviation_arg",
  "deviation_stmt", "deviation_opt_stmt", "deviation_arg_str",
  "deviate_body_stmt", "deviate_not_supported",
  "deviate_not_supported_stmt", "deviate_stmts", "deviate_add_stmt",
  "$@49", "deviate_add_end", "deviate_add_opt_stmt", "deviate_delete_stmt",
  "$@50", "deviate_delete_end", "deviate_delete_opt_stmt",
  "deviate_replace_stmt", "$@51", "deviate_replace_end",
  "deviate_replace_opt_stmt", "when_arg_str", "when_stmt", "when_end",
  "when_opt_stmt", "config_arg", "config_stmt", "config_arg_str",
  "mandatory_arg", "mandatory_stmt", "mandatory_arg_str", "presence_arg",
  "presence_stmt", "min_value_arg", "min_elements_stmt",
  "min_value_arg_str", "max_value_arg", "max_elements_stmt",
  "max_value_arg_str", "ordered_by_arg", "ordered_by_stmt",
  "ordered_by_arg_str", "must_agr_str", "must_stmt", "must_end",
  "unique_arg", "unique_stmt", "unique_arg_str", "unique_arg_opt",
  "key_arg", "key_stmt", "key_arg_str", "$@52", "range_arg_str",
  "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@53",
  "path_arg_str", "$@54", "$@55", "absolute_path", "absolute_paths",
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
  "string_opt_part1", "string_opt_part2", "unknown_statement_end",
  "unknown_statement2_opt", "unknown_statement2_end", "sep_stmt", "optsep",
  "sep", "whitespace_opt", "string", "$@56", "strings", "identifier",
  "identifiers", "identifiers_ref", YY_NULLPTR
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

#define YYPACT_NINF -969

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-969)))

#define YYTABLE_NINF -630

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -969,    29,  -969,  -969,   297,  -969,  -969,  -969,   120,   120,
    -969,  -969,  3022,  3022,   120,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   -46,
    -969,  -969,  -969,   -32,  -969,   120,  -969,   120,  -969,    -9,
     348,   348,  -969,  -969,  -969,   192,  -969,  -969,  -969,  -969,
      60,   205,  -969,    38,   120,   120,   120,  -969,  -969,  -969,
     -51,  2452,   120,  -969,   120,  -969,  -969,  -969,    41,   120,
     120,  -969,  -969,  2452,  3022,  2452,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
      41,  3022,   348,  -969,   120,   120,   120,   120,  -969,  -969,
    -969,  -969,  -969,    44,  3022,  3022,   348,   348,   242,  -969,
    -969,   242,  -969,  -969,   242,  -969,   120,  -969,  -969,    30,
    -969,   120,  2452,  2452,  2452,  2452,    40,  1865,   120,  -969,
      55,  -969,   262,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
     120,  -969,  2261,   120,    61,  -969,   242,  -969,   242,  -969,
     242,  -969,   242,  -969,  -969,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,   278,   348,  -969,  -969,  -969,  -969,   348,   104,  -969,
    -969,  -969,  -969,  -969,   294,  -969,  -969,  -969,  -969,   120,
    3022,     7,  3022,  3022,  3022,     7,  3022,  3022,  3022,  3022,
    3022,  3022,  3022,  3022,  3022,  2547,  3022,   348,  -969,  -969,
     270,  -969,   348,   348,  -969,  -969,   285,   120,    66,   288,
    -969,  3117,  -969,  -969,    73,  -969,  -969,   299,  -969,   305,
    -969,   306,  -969,  -969,    79,  -969,  -969,   336,  -969,   349,
    -969,   368,  -969,    91,  -969,    97,  -969,   113,  -969,   377,
    -969,   385,  -969,   144,  -969,  -969,   393,  -969,  -969,  -969,
     288,  -969,  -969,  -969,  -969,  -969,   147,   149,   348,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   154,   120,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
     120,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,   120,   120,  -969,   120,   348,   120,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   120,   120,
     348,   348,  -969,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   188,   278,  2356,
     757,   962,  1053,  1462,    71,   308,   346,  1602,   220,   927,
    1894,  1402,  1115,  1147,   251,   275,  -969,  -969,  -969,   242,
    -969,  -969,   120,   120,   120,   120,   120,   120,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,   120,   120,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   120,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   120,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,   120,  -969,  -969,  -969,  -969,  -969,
     120,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   120,  -969,
    -969,  -969,  -969,  -969,  -969,   120,   120,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   120,
     120,   120,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,   120,   120,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,   176,  -969,   181,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,   120,   120,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,   120,    17,  2452,    58,
    2452,    98,  2452,  -969,  -969,  3022,  3022,  -969,  -969,  -969,
    -969,  -969,  2452,  -969,  -969,  2452,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  3022,   348,   307,   348,   348,   348,   348,
     348,  2547,  2547,  2452,  -969,  -969,  -969,    19,   178,    42,
    -969,  -969,  -969,  2642,  2642,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,   120,   120,  -969,  -969,
    -969,  -969,  -969,  -969,   348,  2642,  2642,  -969,  -969,  -969,
    -969,  -969,  -969,   242,  -969,   242,  -969,  -969,  -969,  -969,
     242,  -969,   400,  -969,  -969,  -969,  -969,  -969,   242,  -969,
     404,  -969,   348,   348,   407,  -969,   385,  -969,   348,   348,
     348,   348,   348,   242,  -969,   348,   348,   242,  -969,   348,
     348,   348,   348,   348,   348,   348,   410,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   242,
    -969,   414,  -969,   242,  -969,   348,   348,   348,  -969,  -969,
    -969,   242,  -969,  -969,  -969,  -969,   242,  -969,  -969,  -969,
    -969,  -969,  -969,   242,  -969,   348,   348,   348,  -969,   242,
    -969,  -969,  -969,   242,  -969,  -969,  -969,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,  -969,  -969,   187,  -969,  -969,  -969,
     415,  -969,   348,   348,   348,   120,   120,  -969,  -969,   120,
     120,  -969,  -969,  -969,  -969,   120,   120,   120,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,   120,   120,   120,    52,   120,  -969,  -969,  -969,  -969,
    -969,   120,  -969,   120,  -969,   120,   120,   120,  -969,  -969,
    -969,  -969,   120,  -969,   100,  1297,  -969,   120,  -969,  -969,
    -969,   120,   348,   348,   348,   134,   428,   433,   438,  -969,
     464,   120,  3117,   154,  -969,  -969,  -969,  -969,  -969,  -969,
     348,   348,   490,   230,  1654,   120,   195,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,   120,   120,   197,  1328,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  1541,
     425,   120,   120,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,    77,  -969,
     348,   348,   348,  3022,  2452,  -969,   120,   120,   120,   120,
     120,   120,  -969,   239,  -969,  -969,  -969,  -969,  -969,  -969,
     348,   271,   348,   348,   348,   348,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  2452,  2452,
    -969,  -969,  -969,  -969,  -969,   242,  -969,   572,    94,   390,
     445,  -969,   446,  -969,    99,  2452,   145,  2452,  2452,    80,
    -969,   348,   293,  -969,   348,   242,  -969,   242,  -969,   348,
     348,   120,   120,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,   242,  -969,  -969,   467,  -969,  -969,   242,
    -969,   223,   318,   468,  -969,   486,  -969,  -969,  -969,  -969,
     242,  -969,   348,  -969,  -969,  -969,   348,   348,   348,   348,
     348,   348,  -969,   120,  -969,  -969,  -969,  -969,  3117,  -969,
    -969,   229,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
     120,   120,  -969,   348,   246,   212,   348,   348,  -969,   223,
    -969,  2737,   348,   348,   348,   120,  -969,  -969,  -969,  -969,
    -969,  -969,   120,  -969,  -969,  -969,  -969,  -969,  -969,   525,
     234,  -969,  -969,  -969,   160,   351,   567,   178,   184,  -969,
     347,  -969,   449,   120,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,   242,  -969,  -969,  -969,  -969,  -969,  -969,   242,
    -969,  -969,  -969,  -969,  3117,  -969,  2452,  -969,   120,  -969,
     120,   347,   347,   242,  -969,   250,   304,  -969,  -969,   347,
     301,   347,  -969,   347,   330,   274,   347,   347,   340,   402,
    -969,   347,  -969,  -969,   352,  2832,   347,  -969,  -969,  -969,
    2927,  -969,   361,   347,  3117,  -969
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     677,     0,     2,     3,     0,     1,   675,   676,     0,     0,
     678,   677,     0,     0,   679,   693,     4,   692,   694,   695,
     696,   697,   698,   699,   700,   701,   702,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   719,   720,   721,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   732,   733,   734,   735,
     736,   737,   738,   739,   740,   741,   742,   743,   744,   745,
     746,   747,   748,   749,   750,   751,   752,   753,   754,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,   768,   769,   770,   771,   772,   677,   650,     0,
       9,   773,   677,     0,    16,     6,   661,   649,   661,     5,
      12,    19,   677,   774,    27,    11,   663,   662,   677,    27,
      18,     0,    51,    26,     0,     0,     0,    13,    14,    15,
       0,   667,   666,    51,     0,    20,   661,     7,    66,     0,
       0,   661,   661,     0,     0,     0,   677,   669,   664,   685,
     688,   686,   690,   691,   689,   684,   665,   668,   682,   687,
      66,     0,    21,   677,     0,     0,     0,     0,    52,    53,
      54,    55,    78,    64,     0,     0,    28,    29,     0,    24,
     647,     0,    49,   677,     0,    22,   671,   677,    78,     0,
      47,     8,     0,     0,     0,     0,     0,    77,     0,   661,
       0,    31,     0,    37,   660,   658,    25,   661,   661,    50,
     648,    23,     0,   683,     0,   661,     0,    58,     0,    60,
       0,    56,     0,    62,   677,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   661,    80,    81,    82,    83,    84,    85,   254,
     255,   256,   257,   258,   259,   260,   261,    86,    87,    88,
      89,     0,    67,   661,   661,    39,    38,   656,     0,   652,
     670,   677,   651,   677,     0,    59,    61,    57,    63,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    79,    74,    76,
       0,    65,    32,    41,   659,   661,     0,    17,     0,     0,
     388,     0,   442,   443,     0,   609,   677,     0,   350,     0,
     262,     0,    90,   516,     0,   509,   677,     0,   115,     0,
     241,     0,   126,     0,   288,     0,   304,     0,   323,     0,
     492,     0,   462,     0,   138,   655,     0,   405,   677,   677,
       0,   390,   677,   661,    69,    68,     0,     0,   657,   677,
     677,   672,    48,   661,   392,   389,   607,   661,   608,   441,
     661,   352,   351,   661,   264,   263,   661,    92,    91,   661,
     515,   661,   117,   116,   661,   243,   242,   661,   128,   127,
     661,   661,   661,   661,   494,   493,   661,   464,   463,   661,
     661,   407,   406,   653,   654,   391,    75,    71,     0,    30,
      36,    33,    34,    35,    40,    44,    42,    43,   671,   673,
     394,   445,   610,   354,   266,    94,   511,   119,   245,   130,
     289,   306,   325,   496,   466,   140,   409,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    72,    73,     0,
      45,   677,     0,     0,     0,     0,     0,     0,   393,   403,
     404,   402,   397,   395,   400,   401,   398,     0,     0,   444,
     450,   451,   449,   448,   452,   458,   454,   456,   446,     0,
     353,   362,   363,   361,   357,   358,   368,   369,   370,   371,
     374,   364,   366,   367,   372,   373,   355,   359,   360,     0,
     265,   275,   276,   274,   269,   283,   277,   285,   279,   281,
     267,   273,   272,   270,     0,    93,    97,    98,   661,    96,
       0,   510,   512,   513,   118,   122,   123,   121,   120,   244,
     247,   248,   246,   661,   661,   661,   661,   661,     0,   129,
     134,   135,   133,   132,   131,     0,     0,   287,   302,   303,
     301,   292,   293,   295,   298,   290,   299,   300,   296,     0,
       0,     0,   305,   321,   322,   320,   309,   310,   313,   312,
     307,   316,   317,   318,   319,   314,     0,     0,   324,   338,
     339,   337,   328,   340,   342,   348,   344,   346,   326,   333,
     334,   335,   336,   329,   332,   331,   495,   501,   502,   500,
     499,   503,   505,   507,   497,   677,   677,   465,   469,   470,
     468,   467,   471,   473,     0,   475,     0,   477,   139,   145,
     146,   144,   661,   142,   143,     0,     0,   408,   414,   415,
     413,   412,   416,   418,   410,    46,   674,     0,     0,     0,
       0,     0,     0,   661,   661,     0,     0,   661,   661,   661,
     661,   661,     0,   661,   661,     0,   661,   661,   661,   661,
     661,   661,   661,     0,    95,     0,   250,   249,   251,   252,
     253,     0,     0,     0,   661,   661,   661,     0,     0,     0,
     661,   661,   661,     0,     0,   661,   661,   661,   661,   661,
     661,   661,   661,   661,   661,   661,   479,   490,   661,   661,
     661,   661,   661,   661,   141,     0,     0,   661,   661,   661,
     677,   677,   569,     0,   565,     0,   124,   677,   677,   574,
       0,   570,     0,   591,   677,   677,   677,   114,     0,   109,
       0,   558,   396,   399,     0,   375,     0,   460,   453,   459,
     455,   457,   447,     0,   239,   365,   356,     0,   575,   284,
     278,   286,   280,   282,   268,   271,     0,    99,   677,   677,
     519,   677,   514,   677,   517,   677,   521,   523,   522,     0,
     136,     0,   148,     0,   237,   294,   291,   297,   640,   677,
     585,     0,   581,   677,   641,   580,     0,   577,   642,   677,
     677,   677,   590,     0,   586,   311,   308,   315,   605,     0,
     601,   603,   598,     0,   595,   599,   611,   341,   343,   349,
     345,   347,   327,   330,   504,   506,   508,   498,   472,   474,
     481,   476,   481,   478,   438,   439,     0,   677,   421,   422,
       0,   677,   417,   419,   411,   568,   567,   566,   125,   573,
     572,   571,   661,   593,   592,   111,   113,   112,   110,   661,
     560,   559,   661,   377,   376,   461,   240,   576,   661,   101,
     100,   524,   537,   546,     0,   518,   137,   661,   149,   147,
     238,   583,   582,   584,   578,   579,   589,   588,   587,   602,
     677,   596,   597,   609,     0,     0,   661,   437,   661,   424,
     423,   420,   172,   562,   379,   104,     0,     0,     0,   520,
     154,   604,     0,   612,   480,   484,   486,   488,   482,   491,
     445,   426,     0,     0,     0,     0,     0,   661,   526,   525,
     661,   539,   538,   661,   548,   547,     0,     0,     0,   151,
     152,   661,   153,   217,   600,   661,   661,   661,   661,     0,
       0,     0,     0,   594,   175,   176,   173,   174,   561,   563,
     564,   378,   384,   385,   383,   382,   386,   380,     0,   102,
     528,   541,   550,     0,     0,   150,     0,     0,     0,     0,
       0,     0,   157,     0,   160,   158,   159,   661,   156,   155,
     190,   216,   485,   487,   489,   483,   440,   425,   435,   436,
     428,   430,   431,   432,   429,   433,   434,   661,     0,     0,
     661,   661,   677,   677,   108,     0,   103,     0,     0,     0,
       0,   220,     0,   193,     0,     0,   613,     0,     0,     0,
     661,   161,   189,   218,   427,     0,   235,     0,   233,   387,
     381,   107,   106,   105,   527,   529,   532,   533,   534,   535,
     536,   661,   531,   540,   542,   545,   661,   544,   549,   661,
     552,   553,   554,   555,   556,   557,   661,   221,   661,   661,
     194,   192,   167,     0,   164,   677,     0,   169,   617,     0,
     209,     0,     0,     0,   178,     0,   606,   677,   677,   215,
       0,   211,   162,   661,   236,   234,   530,   543,   551,   223,
     219,   196,   165,   166,   661,   170,   661,   210,     0,   620,
     614,     0,   616,   624,   661,   179,   661,   661,   207,   206,
     214,   213,   212,   191,     0,     0,   172,   168,   629,   619,
     623,     0,   181,   177,   172,     0,   222,   227,   228,   226,
     224,   225,     0,   195,   200,   201,   199,   197,   198,     0,
     618,   621,   625,   622,   627,     0,     0,     0,     0,   171,
     680,   626,     0,     0,   180,   185,   186,   182,   183,   184,
     208,   232,     0,   229,   677,   645,   646,   643,   205,     0,
     202,   677,   644,   681,     0,   628,     0,   230,   231,   203,
     204,   680,   680,     0,   187,     0,     0,   188,   630,   680,
       0,   680,   631,   680,     0,     0,   680,   680,     0,     0,
     639,   680,   632,   635,     0,     0,   680,   636,   637,   634,
     680,   633,     0,   680,     0,   638
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -969,  -969,  -969,    46,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,   363,  -969,  -969,   372,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   137,  -969,
    -969,  -969,  -225,   365,  -969,  -969,  -969,  -969,  -969,   793,
    -969,   841,   343,  -969,  -969,  -969,  -969,  -969,    75,  -969,
     337,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -409,  -969,  -969,  -969,  -969,  -969,
    -969,  -388,  -969,  -969,  -969,  -969,  -969,  -405,  -969,  -183,
    -969,  -438,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -968,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -491,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -431,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -594,  -969,  -582,  -969,
    -432,  -969,  -443,  -969,  -180,  -969,  -969,  -194,  -969,   138,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,   142,
    -969,  -969,  -969,  -969,  -969,  -969,   153,  -969,  -969,  -969,
    -969,  -969,   161,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,   164,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
     168,  -969,  -969,  -969,  -969,  -969,   180,  -969,   191,   276,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -300,  -969,  -969,  -969,  -969,  -969,  -969,  -135,  -969,
    -969,  -117,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -202,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -167,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,  -969,
    -969,  -969,  -969,  -969,  -969,  -356,  -969,  -969,  -969,  -353,
    -969,  -969,  -440,  -969,  -969,  -315,  -969,  -428,  -969,  -969,
    -423,  -969,  -969,   189,  -969,  -969,  -372,  -969,  -969,  -707,
    -969,  -969,  -969,  -969,  -969,  -969,  -969,  -360,   353,  -254,
    -668,  -969,  -969,  -969,  -969,  -488,  -519,  -969,  -969,  -487,
    -969,  -969,  -969,  -507,  -969,  -969,  -969,  -567,  -969,  -969,
    -969,  -674,  -506,  -969,  -969,  -969,   325,  -192,  -314,   589,
    -969,  -969,  -969,  1163,  -969,   381,  -969,  -969,  -969,   235,
    -969,    -4,    21,   196,   454,  -119,  -969,  -969,   252,  -143,
    -276
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   109,   163,    99,     2,   114,   115,
     103,     3,   119,   120,   184,   127,   178,   128,   122,   123,
     141,   200,   356,   202,   142,   266,   357,   459,   410,   189,
     136,   181,   129,   138,   220,   168,   216,   169,   218,   954,
     222,   955,   172,   300,   173,   199,   355,   437,   301,   352,
     196,   197,   242,   321,   243,   378,   444,   766,   528,   870,
    1015,   926,  1016,   738,   482,   739,   327,   244,   383,   446,
     725,   483,   331,   245,   389,   448,   779,   554,   343,   915,
     454,   562,   781,   879,   938,   939,   983,  1073,   984,  1074,
     985,  1076,  1106,   922,   986,  1083,  1116,  1155,  1193,  1167,
     940,  1032,   941,  1022,  1071,  1125,  1179,  1148,  1180,   987,
    1119,  1079,   988,  1090,   989,  1091,   942,   991,   943,  1020,
    1068,  1124,  1172,  1141,  1173,  1037,   956,  1035,   957,   783,
     563,   753,   495,   329,   916,   386,   447,   484,   319,   249,
     375,   443,   671,   672,   667,   669,   670,   666,   668,   250,
     333,   449,   685,   684,   686,   335,   251,   450,   691,   690,
     692,   337,   252,   451,   700,   701,   695,   696,   698,   699,
     697,   317,   253,   372,   442,   664,   663,   501,   502,   744,
     485,   864,   924,  1011,  1010,   309,   254,   350,   255,   365,
     440,   653,   654,   346,   256,   402,   455,   719,   717,   718,
     839,   840,   642,   900,   950,   835,   836,   643,   313,   314,
     257,   441,   661,   657,   659,   660,   658,   746,   486,   341,
     258,   398,   453,   708,   709,   711,   713,   624,   625,   894,
     948,   945,   946,   947,   626,   627,   339,   487,   395,   452,
     705,   702,   703,   704,   324,   260,   445,   325,   772,   773,
     774,   775,   776,   906,   929,  1017,   777,   907,   932,  1018,
     778,   908,   935,  1019,   740,   488,   861,   923,   723,   474,
     724,   730,   475,   731,   757,   522,   796,   582,   797,   791,
     583,   792,   803,   584,   804,   732,   918,   854,   813,   604,
     814,   892,   809,   605,   810,   890,  1085,   315,   316,   368,
     815,   893,  1080,  1081,  1082,  1109,  1110,  1129,  1112,  1113,
    1131,  1153,  1161,  1150,  1191,  1202,  1212,  1213,  1215,  1220,
    1203,   798,   799,  1181,  1182,   182,   100,   816,   347,   206,
     207,   305,   208,   110,   116,   130,   131,   156,   148,   212,
     361,   117,   132,    12,  1184,   157,   187,   158,   101,   102,
     118
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,   183,   508,   248,    11,    11,   564,   579,   422,   567,
      10,   634,   577,   793,   246,    16,   632,   247,   578,   349,
     271,     4,   633,   600,   179,    16,   185,    16,   601,     5,
     259,   471,    14,   493,   513,   529,   788,   537,   542,   552,
     560,   575,   591,   609,   620,   631,   640,   837,   841,   308,
      16,   146,   472,   147,   494,   514,   106,     6,   538,     7,
     553,   561,   576,   592,   610,   621,    16,   641,   476,   164,
     108,   523,   165,   217,   219,   221,   223,   568,   585,   603,
     614,   139,   140,   134,   473,    16,   506,   520,    16,   507,
     521,   524,   112,   565,   580,   598,   566,   581,   599,   644,
     166,    10,   165,    10,   225,   720,    16,    16,   167,     6,
     721,     7,   311,   789,   198,   113,   788,    10,   105,   366,
      11,    11,    11,   107,   489,     6,   227,     7,    10,   228,
      11,   411,   215,   121,   800,    11,    11,   801,   167,     6,
     232,     7,   126,   224,   466,   113,   727,   234,   235,   465,
     236,   728,   348,    16,   204,   465,   205,   263,  1149,  -615,
      11,    11,    11,    11,   273,  1012,  1156,   186,  1087,   362,
    1013,   587,   556,  1088,   525,   367,   239,   155,   165,   240,
     165,   379,    10,   734,   191,   735,    16,    10,   736,   155,
     180,   155,    16,   390,    11,   788,   794,  1053,   241,   391,
    1175,  1176,  1177,   914,   210,    13,    10,   304,   213,    10,
       6,   125,     7,   137,   167,   392,   167,   925,   408,   165,
     408,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,   155,   155,
     155,   155,   548,   165,   944,   279,   399,   271,   124,   517,
     409,   165,   414,   545,   463,   167,   125,   595,   613,   311,
     515,   165,   463,   516,   543,  -629,  -629,   544,   593,   611,
     622,   594,   612,   623,   126,    10,   519,   165,   710,   167,
     547,   489,   165,   712,   597,   466,    16,   167,   463,   896,
     298,   456,  1142,   466,   307,   936,   635,   167,   969,     6,
     975,     7,     6,    10,     7,   113,   165,   299,   518,  1135,
    1052,  1057,   546,   167,   555,  1143,   596,   463,   167,   466,
     143,   144,   145,   549,   466,   937,   555,   312,  1108,   556,
     161,   323,  1111,   958,  1130,   174,   175,   369,   104,   165,
    1160,   345,   167,   636,   204,   530,   205,   380,   466,  1136,
    1075,     8,  1183,     6,   628,     7,   467,  1198,   125,   113,
     192,   193,   194,   195,   264,    10,   265,   780,   782,   403,
     404,     9,   353,   406,   354,   167,    10,   165,   637,  1207,
     418,   419,   165,   159,   951,   952,  1201,   359,   463,   360,
     363,   768,   364,   769,   261,   159,   770,   159,   771,    10,
      10,   370,    10,   371,    11,   349,   349,   373,   376,   374,
     377,   531,  1199,   167,    10,    10,  1211,   462,   167,   466,
     489,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   381,  1206,
     382,   464,   569,   570,   159,   159,   159,   159,  1163,   534,
    1210,   384,   462,   385,  1164,   489,   165,  1216,    11,    11,
      11,    11,    11,    11,   272,   555,  1223,   463,   556,     6,
     387,     7,   388,    11,    11,   113,   464,   569,   570,   393,
     465,   394,   646,   135,   299,    11,   190,   396,   936,   397,
     509,   133,   167,  1058,   415,   400,   937,   401,   160,   201,
     203,   811,   852,   188,   853,    11,   859,  1001,   860,   862,
    1003,   863,   868,   460,   869,   964,   877,   898,   878,   899,
      11,   165,  1005,   951,   952,   214,    11,  1006,   997,   726,
     927,   733,   928,   741,   982,   930,   965,   931,   348,   348,
     933,  1093,   934,   754,    11,  1030,   758,  1066,  1069,  1067,
    1070,    11,    11,   422,  1108,  1160,   165,   167,   951,   952,
    1033,  1168,  1000,   272,   784,    11,    11,    11,   967,  1104,
    1114,  1105,  1115,  1169,  1046,  1055,  1061,  1048,  1007,  1063,
     496,  1059,    11,    11,   497,  1045,  1054,  1060,  1117,  1049,
    1118,  1064,   167,   953,  1050,   498,  1065,  1002,   165,   462,
     951,   952,   489,   499,   438,   310,   500,   318,   320,   322,
     503,   328,   330,   332,   334,   336,   338,   340,   342,   344,
     949,   351,   504,   464,   569,   570,   405,   465,  1159,   865,
     895,    11,    11,   505,   167,  1004,   706,   707,   326,   913,
     602,  1151,    10,  1185,  1152,  1051,  1056,  1162,  1217,   587,
     556,  1174,   306,   439,     0,     0,     0,     0,   647,   648,
     649,   650,   651,   652,  1047,     0,  1062,     0,     0,     0,
    1170,     0,     0,   655,   656,  1044,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   662,     0,     0,     0,     0,
       0,   272,     0,   722,   155,   729,   155,   737,   155,     0,
     917,   917,    10,    10,     0,   665,     0,     0,   155,     0,
       0,   155,     0,     0,     0,  1139,  1146,     0,     0,     0,
     673,     0,     0,     0,     0,     0,   675,   345,   345,   155,
     966,     0,     0,   790,   795,   802,  1140,  1147,     0,   808,
     812,   845,   846,     0,   681,     0,     0,     0,   849,   850,
       0,   682,   683,     0,     0,   855,   856,   857,     0,     0,
       0,   834,   838,     0,     0,   687,   688,   689,     0,     0,
     209,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,   693,   694,   462,     0,     0,     0,   165,   871,
     872,     0,   873,     0,   874,     0,   875,     0,     0,   463,
       0,     0,     0,     0,     0,   275,     0,   276,   464,   277,
     881,   278,   465,     0,   883,     0,     0,     0,     0,     0,
     885,   886,   887,     0,   167,     0,     0,     0,     0,     0,
     466,   715,   716,     0,     0,     0,     0,     0,   467,     0,
       0,    10,    10,     0,     0,    10,    10,     0,     0,     0,
       0,    10,    10,    10,     0,  1023,     0,     0,   897,     0,
     468,     0,   901,     0,     0,     0,     0,    10,    10,    10,
      10,    10,     0,     0,     0,     0,     0,    10,     0,    10,
       0,    10,    10,    10,     0,     0,     0,     0,    11,  1036,
    1038,     0,     0,    10,     0,     0,     0,    10,     0,     0,
     159,     0,   159,     0,   159,     0,  1077,    10,  1084,  1086,
       0,   911,     0,     0,   159,     0,  1128,   159,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   170,    11,    11,     0,   159,     0,     0,     0,  1154,
       0,     0,     0,     0,     0,   272,   272,    11,    11,     0,
       0,     0,     0,   170,   462,     0,     0,   489,   165,     0,
       0,     0,     0,     0,     0,     0,   225,   272,   272,   463,
       0,     0,    11,    11,    11,    11,    11,    11,   464,   171,
     745,   747,   465,     0,     0,     0,     0,   477,   227,     0,
       0,   228,  1192,   165,   167,     0,     0,     0,   767,     0,
     466,   171,   555,     0,   463,   556,     0,     0,   467,   234,
     235,     0,   236,     0,  1014,     0,     0,     0,     0,   237,
     155,     0,     0,  1218,     0,     0,     0,     0,  1221,   167,
     557,     0,  1225,  1041,  1042,   466,     0,    10,    10,     0,
       0,   240,     0,   467,     0,     0,     0,     0,   645,     0,
       0,     0,     0,     0,   155,   155,     0,   225,   478,     0,
     241,     0,     0,     0,     0,   479,     0,  1194,     0,     0,
    1072,   155,  1078,   155,   155,  1089,     0,     0,   477,   227,
     462,     0,   228,   489,   165,     0,     0,     0,   912,     0,
       0,     0,     0,     0,     0,   463,  1103,     0,     0,    10,
     234,   235,     0,   236,   464,     0,     0,     0,  1120,  1121,
       0,     0,     0,     0,     0,     0,    10,    10,     0,   225,
     167,   968,     0,     0,     0,     0,   466,     0,     0,     0,
       0,    11,   973,   974,   467,     0,     0,     0,    11,     0,
       0,   227,     0,     0,   228,     0,   165,  1008,  1009,   412,
     416,   241,     0,     0,     0,   232,   490,   463,     0,    11,
       0,     0,   234,   235,   272,   236,     0,     0,     0,     0,
     465,     0,  1024,  1025,  1026,  1027,  1028,  1029,   165,     0,
       0,     0,   167,     0,    10,     0,    10,   232,   466,   463,
       0,   239,   615,     0,   240,  1188,     0,   413,   417,     0,
       0,     0,  1190,  1171,  1178,     0,     0,   616,     0,     0,
       0,     0,     0,   241,   167,     0,     0,     0,   606,     0,
     466,     0,     0,   239,     0,     0,   159,     0,     0,     0,
     457,     0,   155,   469,   480,   491,   511,   526,   532,   535,
     540,   550,   558,   573,   589,   607,   618,   629,   638,     0,
     617,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     159,   159,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   111,     0,     0,     0,     0,     0,   159,   458,   159,
     159,   470,   481,   492,   512,   527,   533,   536,   541,   551,
     559,   574,   590,   608,   619,   630,   639,     0,  1021,   162,
       0,   225,     0,     0,   176,   177,     0,     0,     0,     0,
       0,     0,   847,     0,   848,     0,     0,     0,     0,   851,
       0,     0,     0,   227,     0,     0,   228,   858,     0,     0,
       0,  1157,     0,     0,     0,     0,     0,   232,  1158,     0,
       0,     0,   866,     0,   234,   235,   867,   236,     0,     0,
     548,     0,   465,     0,     0,     0,     0,     0,     0,  1186,
     272,     0,   262,     0,     0,     0,     0,   976,   876,     0,
     267,   268,   880,   239,     0,     0,   240,   977,   274,     0,
     882,     0,     0,   272,     0,   884,     0,     0,     0,   978,
     979,     0,   888,     0,   980,   241,     0,   981,   889,     0,
     919,     0,   891,  -163,     0,   297,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   302,   303,   227,   462,
       0,   228,     0,   165,     0,     0,   272,     0,   159,     0,
       0,     0,   232,     0,   463,     0,     0,     0,   586,   234,
     235,     0,   236,     0,   569,   570,     0,   465,     0,   237,
     571,     0,     0,   909,     0,     0,   225,   272,   358,   167,
       0,     0,   272,     0,     0,   466,   272,     0,   239,   587,
       0,   240,     0,   467,     0,     0,     0,     0,   227,   462,
       0,   228,     0,   165,     0,     0,     0,     0,   478,     0,
     241,     0,   232,     0,   463,   588,     0,     0,     0,   234,
     235,     0,   236,     0,     0,     0,   407,   465,     0,   237,
       0,     0,     0,     0,     0,     0,   420,   509,     0,   167,
     421,     0,     0,   423,     0,   466,   424,     0,   239,   425,
       0,   240,   426,   467,   427,   225,     0,   428,     0,     0,
     429,     0,     0,   430,   431,   432,   433,     0,   478,   434,
     241,     0,   435,   436,     0,   510,   477,   227,     0,     0,
     228,     0,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   463,     0,     0,     0,     0,   234,   235,
       0,   236,     0,     0,     0,     0,     0,     0,   237,     0,
       0,     0,     0,     0,  1043,     0,   225,     0,   167,     0,
       0,     0,     0,     0,   466,     0,     0,     0,     0,     0,
     240,     0,   467,     0,  1094,     0,  1095,     0,   227,     0,
       0,   228,     0,   165,     0,     0,     0,   478,     0,   241,
       0,     0,   232,     0,   996,  1195,  1196,     0,     0,   234,
     235,     0,   236,  1200,     0,  1204,     0,  1205,   225,   237,
    1208,  1209,  1102,     0,     0,  1214,     0,     0,  1107,   167,
    1219,     0,     0,     0,  1222,   466,     0,  1224,   239,  1122,
     227,   240,     0,   228,     0,   165,     0,     0,     0,     0,
       0,   674,     0,     0,     0,     0,   463,     0,   478,     0,
     241,   234,   235,     0,   236,   539,   676,   677,   678,   679,
     680,     0,     0,     0,     0,     0,   959,   962,     0,     0,
       0,   167,     0,     0,     0,     0,     0,   466,     0,     0,
       0,     0,     0,   240,     0,   467,     0,     0,     0,     0,
       0,     0,   480,   998,     0,     0,     0,     0,     0,     0,
       0,     0,   241,     0,     0,     0,     0,   961,     0,     0,
       0,  1187,     0,     0,   960,   963,     0,     0,  1189,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1197,     0,     0,     0,     0,     0,     0,     0,
     481,   999,     0,     0,     0,   714,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   742,   743,     0,     0,
     748,   749,   750,   751,   752,     0,   755,   756,     0,   759,
     760,   761,   762,   763,   764,   765,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   785,   786,   787,
       0,     0,     0,   805,   806,   807,     0,     0,   817,   818,
     819,   820,   821,   822,   823,   824,   825,   826,   827,   225,
       0,   828,   829,   830,   831,   832,   833,     0,     0,     0,
     842,   843,   844,     0,     0,     0,   226,     0,     0,     0,
       0,   227,     0,     0,   228,     0,     0,     0,     0,     0,
     229,   230,     0,   231,     0,   232,   233,     0,     0,     0,
       0,     0,   234,   235,     0,   236,     0,  1137,  1144,     0,
       0,   462,   237,     0,   489,   165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   463,   238,     0,     0,
       0,   239,     0,     0,   240,     0,   569,   570,  1165,   465,
       0,     0,   571,     0,     0,     0,     0,     0,     0,     0,
       0,   167,     0,   241,     0,  1138,  1145,   466,     0,   555,
       0,     0,   556,     0,     0,   467,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1166,   572,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   902,     0,     0,     0,     0,
       0,     0,   903,     0,     0,   904,     0,     0,     0,     0,
       0,   905,     0,     0,     0,     0,     0,     0,     0,     0,
     910,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   920,
       0,   921,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     970,     0,     0,   971,     0,     0,   972,     0,     0,     0,
       0,     0,     0,     0,   990,     0,     0,     0,   992,   993,
     994,   995,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1031,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1034,     0,     0,  1039,  1040,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1092,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1096,     0,     0,     0,     0,  1097,
       0,     0,  1098,     0,     0,     0,     0,     0,     0,  1099,
       0,  1100,  1101,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1123,     0,     0,     0,
       0,     0,     0,     0,     0,    15,     0,  1126,     0,  1127,
       0,    17,   269,     0,     0,     0,     0,  1132,     0,  1133,
    1134,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      15,     0,     0,     0,   270,     0,    17,   269,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,     0,    15,     0,     0,   461,
      16,   149,    17,   150,   151,     0,     0,     0,   152,   153,
     154,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    15,     0,     0,     0,    16,     0,    17,   113,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    15,     0,     0,     0,
      16,     0,    17,   269,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    15,     0,     0,     0,     0,     0,    17,   269,     0,
       0,  1111,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    15,     0,     0,     0,
       0,     0,    17,   269,     0,     0,  1211,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    15,  1183,     0,     0,     0,     0,    17,   269,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    15,     0,     0,     0,
      16,     0,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    15,     0,     0,     0,     0,     0,    17,   269,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96
};

static const yytype_int16 yycheck[] =
{
       4,   144,   442,   197,     8,     9,   449,   450,   368,   449,
      14,   454,   450,   687,   197,     8,   454,   197,   450,   295,
     212,     0,   454,   451,   143,     8,   145,     8,   451,     0,
     197,   440,    11,   442,   443,   444,    17,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   715,   716,   274,
       8,   102,   440,   104,   442,   443,   102,     5,   446,     7,
     448,   449,   450,   451,   452,   453,     8,   455,   440,    28,
     102,   443,    31,   192,   193,   194,   195,   449,   450,   451,
     452,    43,    44,    23,   440,     8,   442,   443,     8,   442,
     443,    20,   101,   449,   450,   451,   449,   450,   451,   455,
      59,   105,    31,   107,     4,    88,     8,     8,    67,     5,
      93,     7,   105,    94,    70,    11,    17,   121,    97,   311,
     124,   125,   126,   102,    30,     5,    26,     7,   132,    29,
     134,   356,   102,   112,    92,   139,   140,    95,    67,     5,
      40,     7,    82,   103,    73,    11,    88,    47,    48,    55,
      50,    93,   295,     8,   102,    55,   104,   102,  1126,    14,
     164,   165,   166,   167,   103,    88,  1134,   146,    88,   103,
      93,    77,    78,    93,   103,   102,    76,   131,    31,    79,
      31,   102,   186,    85,   163,    87,     8,   191,    90,   143,
     144,   145,     8,   102,   198,    17,    18,   103,    98,   102,
      16,    17,    18,   103,   183,     9,   210,   103,   187,   213,
       5,    64,     7,     8,    67,   102,    67,    83,    71,    31,
      71,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   192,   193,
     194,   195,    22,    31,   912,   224,   102,   439,    56,   443,
     103,    31,   103,   447,    42,    67,    64,   451,   452,   105,
     443,    31,    42,   443,   447,   105,   106,   447,   451,   452,
     453,   451,   452,   453,    82,   279,   443,    31,   102,    67,
     447,    30,    31,   102,   451,    73,     8,    67,    42,   102,
      12,   103,    80,    73,   273,    24,    21,    67,   103,     5,
     103,     7,     5,   307,     7,    11,    31,   261,   443,    63,
    1017,  1018,   447,    67,    75,   103,   451,    42,    67,    73,
     124,   125,   126,   103,    73,    32,    75,   281,   105,    78,
     134,   285,    14,   103,   105,   139,   140,   316,    13,    31,
     106,   295,    67,    68,   102,    37,   104,   326,    73,   103,
    1024,    54,     5,     5,   103,     7,    81,   107,    64,    11,
     164,   165,   166,   167,   102,   369,   104,   681,   682,   348,
     349,    74,   102,   352,   104,    67,   380,    31,   103,   105,
     359,   360,    31,   131,    33,    34,    85,   102,    42,   104,
     102,    84,   104,    86,   198,   143,    89,   145,    91,   403,
     404,   102,   406,   104,   408,   681,   682,   102,   102,   104,
     104,   103,   108,    67,   418,   419,    14,    27,    67,    73,
      30,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   102,   109,
     104,    51,    52,    53,   192,   193,   194,   195,    97,   103,
     110,   102,    27,   104,   103,    30,    31,   105,   462,   463,
     464,   465,   466,   467,   212,    75,   105,    42,    78,     5,
     102,     7,   104,   477,   478,    11,    51,    52,    53,   102,
      55,   104,   461,   120,   438,   489,   161,   102,    24,   104,
      65,   119,    67,   103,   357,   102,    32,   104,   133,   174,
     175,   693,   102,   160,   104,   509,   102,   950,   104,   102,
     950,   104,   102,   438,   104,   924,   102,   102,   104,   104,
     524,    31,   950,    33,    34,   188,   530,   950,   103,   648,
     102,   650,   104,   652,   939,   102,   924,   104,   681,   682,
     102,  1032,   104,   662,   548,   983,   665,   102,   102,   104,
     104,   555,   556,   913,   105,   106,    31,    67,    33,    34,
     991,  1155,   950,   311,   683,   569,   570,   571,   924,   102,
     102,   104,   104,  1155,  1017,  1018,  1019,  1017,   950,  1019,
     442,  1019,   586,   587,   442,  1017,  1018,  1019,   102,  1017,
     104,  1019,    67,   103,  1017,   442,  1019,   950,    31,    27,
      33,    34,    30,   442,   408,   280,   442,   282,   283,   284,
     442,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     920,   296,   442,    51,    52,    53,   350,    55,   103,   746,
     832,   635,   636,   442,    67,   950,   615,   616,   285,   893,
     451,  1129,   646,  1162,  1131,  1017,  1018,  1154,  1215,    77,
      78,  1157,   271,   418,    -1,    -1,    -1,    -1,   462,   463,
     464,   465,   466,   467,  1017,    -1,  1019,    -1,    -1,    -1,
     103,    -1,    -1,   477,   478,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   489,    -1,    -1,    -1,    -1,
      -1,   439,    -1,   647,   648,   649,   650,   651,   652,    -1,
     894,   895,   706,   707,    -1,   509,    -1,    -1,   662,    -1,
      -1,   665,    -1,    -1,    -1,  1124,  1125,    -1,    -1,    -1,
     524,    -1,    -1,    -1,    -1,    -1,   530,   681,   682,   683,
     924,    -1,    -1,   687,   688,   689,  1124,  1125,    -1,   693,
     694,   720,   721,    -1,   548,    -1,    -1,    -1,   727,   728,
      -1,   555,   556,    -1,    -1,   734,   735,   736,    -1,    -1,
      -1,   715,   716,    -1,    -1,   569,   570,   571,    -1,    -1,
     181,    -1,    -1,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   586,   587,    27,    -1,    -1,    -1,    31,   768,
     769,    -1,   771,    -1,   773,    -1,   775,    -1,    -1,    42,
      -1,    -1,    -1,    -1,    -1,   216,    -1,   218,    51,   220,
     789,   222,    55,    -1,   793,    -1,    -1,    -1,    -1,    -1,
     799,   800,   801,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      73,   635,   636,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,   845,   846,    -1,    -1,   849,   850,    -1,    -1,    -1,
      -1,   855,   856,   857,    -1,   974,    -1,    -1,   837,    -1,
     103,    -1,   841,    -1,    -1,    -1,    -1,   871,   872,   873,
     874,   875,    -1,    -1,    -1,    -1,    -1,   881,    -1,   883,
      -1,   885,   886,   887,    -1,    -1,    -1,    -1,   892,  1008,
    1009,    -1,    -1,   897,    -1,    -1,    -1,   901,    -1,    -1,
     648,    -1,   650,    -1,   652,    -1,  1025,   911,  1027,  1028,
      -1,   890,    -1,    -1,   662,    -1,  1108,   665,    -1,    -1,
      -1,   925,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   936,   937,    -1,   683,    -1,    -1,    -1,  1131,
      -1,    -1,    -1,    -1,    -1,   693,   694,   951,   952,    -1,
      -1,    -1,    -1,   160,    27,    -1,    -1,    30,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,   715,   716,    42,
      -1,    -1,   976,   977,   978,   979,   980,   981,    51,   138,
     655,   656,    55,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,  1184,    31,    67,    -1,    -1,    -1,   673,    -1,
      73,   160,    75,    -1,    42,    78,    -1,    -1,    81,    47,
      48,    -1,    50,    -1,   968,    -1,    -1,    -1,    -1,    57,
     974,    -1,    -1,  1215,    -1,    -1,    -1,    -1,  1220,    67,
     103,    -1,  1224,  1012,  1013,    73,    -1,  1041,  1042,    -1,
      -1,    79,    -1,    81,    -1,    -1,    -1,    -1,   459,    -1,
      -1,    -1,    -1,    -1,  1008,  1009,    -1,     4,    96,    -1,
      98,    -1,    -1,    -1,    -1,   103,    -1,  1186,    -1,    -1,
    1024,  1025,  1026,  1027,  1028,  1029,    -1,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    -1,    -1,   892,    -1,
      -1,    -1,    -1,    -1,    -1,    42,  1075,    -1,    -1,  1103,
      47,    48,    -1,    50,    51,    -1,    -1,    -1,  1087,  1088,
      -1,    -1,    -1,    -1,    -1,    -1,  1120,  1121,    -1,     4,
      67,   925,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,  1135,   936,   937,    81,    -1,    -1,    -1,  1142,    -1,
      -1,    26,    -1,    -1,    29,    -1,    31,   951,   952,   356,
     357,    98,    -1,    -1,    -1,    40,   103,    42,    -1,  1163,
      -1,    -1,    47,    48,   912,    50,    -1,    -1,    -1,    -1,
      55,    -1,   976,   977,   978,   979,   980,   981,    31,    -1,
      -1,    -1,    67,    -1,  1188,    -1,  1190,    40,    73,    42,
      -1,    76,    45,    -1,    79,  1174,    -1,   356,   357,    -1,
      -1,    -1,  1181,  1157,  1158,    -1,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    98,    67,    -1,    -1,    -1,   103,    -1,
      73,    -1,    -1,    76,    -1,    -1,   974,    -1,    -1,    -1,
     437,    -1,  1186,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1008,  1009,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,  1025,   437,  1027,
    1028,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,    -1,   973,   136,
      -1,     4,    -1,    -1,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   723,    -1,   725,    -1,    -1,    -1,    -1,   730,
      -1,    -1,    -1,    26,    -1,    -1,    29,   738,    -1,    -1,
      -1,  1135,    -1,    -1,    -1,    -1,    -1,    40,  1142,    -1,
      -1,    -1,   753,    -1,    47,    48,   757,    50,    -1,    -1,
      22,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,  1163,
    1108,    -1,   199,    -1,    -1,    -1,    -1,    39,   779,    -1,
     207,   208,   783,    76,    -1,    -1,    79,    49,   215,    -1,
     791,    -1,    -1,  1131,    -1,   796,    -1,    -1,    -1,    61,
      62,    -1,   803,    -1,    66,    98,    -1,    69,   809,    -1,
     103,    -1,   813,    75,    -1,   242,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   263,   264,    26,    27,
      -1,    29,    -1,    31,    -1,    -1,  1184,    -1,  1186,    -1,
      -1,    -1,    40,    -1,    42,    -1,    -1,    -1,    46,    47,
      48,    -1,    50,    -1,    52,    53,    -1,    55,    -1,    57,
      58,    -1,    -1,   874,    -1,    -1,     4,  1215,   305,    67,
      -1,    -1,  1220,    -1,    -1,    73,  1224,    -1,    76,    77,
      -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    26,    27,
      -1,    29,    -1,    31,    -1,    -1,    -1,    -1,    96,    -1,
      98,    -1,    40,    -1,    42,   103,    -1,    -1,    -1,    47,
      48,    -1,    50,    -1,    -1,    -1,   353,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,   363,    65,    -1,    67,
     367,    -1,    -1,   370,    -1,    73,   373,    -1,    76,   376,
      -1,    79,   379,    81,   381,     4,    -1,   384,    -1,    -1,
     387,    -1,    -1,   390,   391,   392,   393,    -1,    96,   396,
      98,    -1,   399,   400,    -1,   103,    25,    26,    -1,    -1,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,  1015,    -1,     4,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    81,    -1,  1035,    -1,  1037,    -1,    26,    -1,
      -1,    29,    -1,    31,    -1,    -1,    -1,    96,    -1,    98,
      -1,    -1,    40,    -1,   103,  1191,  1192,    -1,    -1,    47,
      48,    -1,    50,  1199,    -1,  1201,    -1,  1203,     4,    57,
    1206,  1207,  1073,    -1,    -1,  1211,    -1,    -1,  1079,    67,
    1216,    -1,    -1,    -1,  1220,    73,    -1,  1223,    76,  1090,
      26,    79,    -1,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,   528,    -1,    -1,    -1,    -1,    42,    -1,    96,    -1,
      98,    47,    48,    -1,    50,   103,   543,   544,   545,   546,
     547,    -1,    -1,    -1,    -1,    -1,   923,   924,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,   949,   950,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,  1172,    -1,    -1,   923,   924,    -1,    -1,  1179,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     949,   950,    -1,    -1,    -1,   632,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   653,   654,    -1,    -1,
     657,   658,   659,   660,   661,    -1,   663,   664,    -1,   666,
     667,   668,   669,   670,   671,   672,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   684,   685,   686,
      -1,    -1,    -1,   690,   691,   692,    -1,    -1,   695,   696,
     697,   698,   699,   700,   701,   702,   703,   704,   705,     4,
      -1,   708,   709,   710,   711,   712,   713,    -1,    -1,    -1,
     717,   718,   719,    -1,    -1,    -1,    21,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    -1,    40,    41,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    50,    -1,  1124,  1125,    -1,
      -1,    27,    57,    -1,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    72,    -1,    -1,
      -1,    76,    -1,    -1,    79,    -1,    52,    53,  1155,    55,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    98,    -1,  1124,  1125,    73,    -1,    75,
      -1,    -1,    78,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1155,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   852,    -1,    -1,    -1,    -1,
      -1,    -1,   859,    -1,    -1,   862,    -1,    -1,    -1,    -1,
      -1,   868,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     877,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   896,
      -1,   898,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     927,    -1,    -1,   930,    -1,    -1,   933,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   941,    -1,    -1,    -1,   945,   946,
     947,   948,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     987,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1007,    -1,    -1,  1010,  1011,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1030,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1051,    -1,    -1,    -1,    -1,  1056,
      -1,    -1,  1059,    -1,    -1,    -1,    -1,    -1,    -1,  1066,
      -1,  1068,  1069,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1093,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,  1104,    -1,  1106,
      -1,    10,    11,    -1,    -1,    -1,    -1,  1114,    -1,  1116,
    1117,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       4,    -1,    -1,    -1,   103,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    -1,     4,    -1,    -1,   103,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    16,    17,
      18,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
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
       8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
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
      98,     4,     5,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,     4,    -1,    -1,    -1,
       8,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   112,   118,   122,   453,     0,     5,     7,    54,    74,
     452,   452,   454,   454,   453,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   113,   114,   117,
     437,   459,   460,   121,   437,   453,   102,   453,   102,   115,
     444,   444,   101,    11,   119,   120,   445,   452,   461,   123,
     124,   453,   129,   130,    56,    64,    82,   126,   128,   143,
     446,   447,   453,   129,    23,   126,   141,     8,   144,    43,
      44,   131,   135,   454,   454,   454,   102,   104,   449,     9,
      11,    12,    16,    17,    18,   114,   448,   456,   458,   459,
     144,   454,   444,   116,    28,    31,    59,    67,   146,   148,
     150,   152,   153,   155,   454,   454,   444,   444,   127,   456,
     114,   142,   436,   460,   125,   456,   453,   457,   153,   140,
     437,   453,   454,   454,   454,   454,   161,   162,    70,   156,
     132,   437,   134,   437,   102,   104,   440,   441,   443,   440,
     453,   440,   450,   453,   161,   102,   147,   456,   149,   456,
     145,   456,   151,   456,   103,     4,    21,    26,    29,    35,
      36,    38,    40,    41,    47,    48,    50,    57,    72,    76,
      79,    98,   163,   165,   178,   184,   190,   245,   248,   250,
     260,   267,   273,   283,   297,   299,   305,   321,   331,   348,
     356,   454,   444,   102,   102,   104,   136,   444,   444,    11,
     103,   438,   459,   103,   444,   440,   440,   440,   440,   453,
     454,   454,   454,   454,   454,   454,   454,   454,   454,   454,
     454,   454,   454,   454,   454,   454,   454,   444,    12,   114,
     154,   159,   444,   444,   103,   442,   446,   453,   143,   296,
     437,   105,   114,   319,   320,   408,   409,   282,   437,   249,
     437,   164,   437,   114,   355,   358,   409,   177,   437,   244,
     437,   183,   437,   261,   437,   266,   437,   272,   437,   347,
     437,   330,   437,   189,   437,   114,   304,   439,   460,   461,
     298,   437,   160,   102,   104,   157,   133,   137,   444,   102,
     104,   451,   103,   102,   104,   300,   438,   102,   410,   453,
     102,   104,   284,   102,   104,   251,   102,   104,   166,   102,
     453,   102,   104,   179,   102,   104,   246,   102,   104,   185,
     102,   102,   102,   102,   104,   349,   102,   104,   332,   102,
     102,   104,   306,   453,   453,   300,   453,   444,    71,   103,
     139,   143,   150,   152,   103,   139,   150,   152,   453,   453,
     444,   444,   408,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   158,   454,   450,
     301,   322,   285,   252,   167,   357,   180,   247,   186,   262,
     268,   274,   350,   333,   191,   307,   103,   150,   152,   138,
     159,   103,    27,    42,    51,    55,    73,    81,   103,   150,
     152,   175,   182,   376,   380,   383,   397,    25,    96,   103,
     150,   152,   175,   182,   248,   291,   329,   348,   376,    30,
     103,   150,   152,   175,   182,   243,   250,   260,   267,   273,
     283,   288,   289,   291,   297,   299,   376,   380,   383,    65,
     103,   150,   152,   175,   182,   190,   245,   248,   329,   348,
     376,   380,   386,   397,    20,   103,   150,   152,   169,   175,
      37,   103,   150,   152,   103,   150,   152,   175,   182,   103,
     150,   152,   175,   190,   245,   248,   329,   348,    22,   103,
     150,   152,   175,   182,   188,    75,    78,   103,   150,   152,
     175,   182,   192,   241,   243,   376,   380,   383,   397,    52,
      53,    58,   103,   150,   152,   175,   182,   192,   241,   243,
     376,   380,   388,   391,   394,   397,    46,    77,   103,   150,
     152,   175,   182,   190,   245,   248,   329,   348,   376,   380,
     388,   391,   394,   397,   400,   404,   103,   150,   152,   175,
     182,   190,   245,   248,   397,    45,    60,   103,   150,   152,
     175,   182,   190,   245,   338,   339,   345,   346,   103,   150,
     152,   175,   192,   241,   243,    21,    68,   103,   150,   152,
     175,   182,   313,   318,   376,   440,   453,   454,   454,   454,
     454,   454,   454,   302,   303,   454,   454,   324,   327,   325,
     326,   323,   454,   287,   286,   454,   258,   255,   259,   256,
     257,   253,   254,   454,   444,   454,   444,   444,   444,   444,
     444,   454,   454,   454,   264,   263,   265,   454,   454,   454,
     270,   269,   271,   454,   454,   277,   278,   281,   279,   280,
     275,   276,   352,   353,   354,   351,   453,   453,   334,   335,
     102,   336,   102,   337,   444,   454,   454,   309,   310,   308,
      88,    93,   114,   379,   381,   181,   456,    88,    93,   114,
     382,   384,   396,   456,    85,    87,    90,   114,   174,   176,
     375,   456,   444,   444,   290,   437,   328,   437,   444,   444,
     444,   444,   444,   242,   456,   444,   444,   385,   456,   444,
     444,   444,   444,   444,   444,   444,   168,   437,    84,    86,
      89,    91,   359,   360,   361,   362,   363,   367,   371,   187,
     439,   193,   439,   240,   456,   444,   444,   444,    17,    94,
     114,   390,   392,   432,    18,   114,   387,   389,   432,   433,
      92,    95,   114,   393,   395,   444,   444,   444,   114,   403,
     405,   438,   114,   399,   401,   411,   438,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   114,   316,   317,   411,   114,   311,
     312,   411,   444,   444,   444,   453,   453,   440,   440,   453,
     453,   440,   102,   104,   398,   453,   453,   453,   440,   102,
     104,   377,   102,   104,   292,   332,   440,   440,   102,   104,
     170,   453,   453,   453,   453,   453,   440,   102,   104,   194,
     440,   453,   440,   453,   440,   453,   453,   453,   440,   440,
     406,   440,   402,   412,   340,   340,   102,   453,   102,   104,
     314,   453,   444,   444,   444,   444,   364,   368,   372,   440,
     444,   453,   454,   410,   103,   190,   245,   248,   397,   103,
     444,   444,   204,   378,   293,    83,   172,   102,   104,   365,
     102,   104,   369,   102,   104,   373,    24,    32,   195,   196,
     211,   213,   227,   229,   411,   342,   343,   344,   341,   322,
     315,    33,    34,   103,   150,   152,   237,   239,   103,   150,
     152,   103,   150,   152,   175,   182,   248,   376,   454,   103,
     444,   444,   444,   454,   454,   103,    39,    49,    61,    62,
      66,    69,   188,   197,   199,   201,   205,   220,   223,   225,
     444,   228,   444,   444,   444,   444,   103,   103,   150,   152,
     182,   243,   380,   383,   386,   388,   391,   397,   454,   454,
     295,   294,    88,    93,   114,   171,   173,   366,   370,   374,
     230,   437,   214,   456,   454,   454,   454,   454,   454,   454,
     192,   444,   212,   229,   444,   238,   456,   236,   456,   444,
     444,   453,   453,   440,   103,   241,   243,   380,   383,   388,
     391,   397,   400,   103,   241,   243,   397,   400,   103,   192,
     241,   243,   380,   383,   388,   391,   102,   104,   231,   102,
     104,   215,   114,   198,   200,   432,   202,   456,   114,   222,
     413,   414,   415,   206,   456,   407,   456,    88,    93,   114,
     224,   226,   444,   213,   440,   440,   444,   444,   444,   444,
     444,   444,   440,   453,   102,   104,   203,   440,   105,   416,
     417,    14,   419,   420,   102,   104,   207,   102,   104,   221,
     453,   453,   440,   444,   232,   216,   444,   444,   438,   418,
     105,   421,   444,   444,   444,    63,   103,   150,   152,   175,
     182,   234,    80,   103,   150,   152,   175,   182,   218,   204,
     424,   416,   420,   422,   438,   208,   204,   454,   454,   103,
     106,   423,   424,    97,   103,   150,   152,   210,   237,   239,
     103,   114,   233,   235,   433,    16,    17,    18,   114,   217,
     219,   434,   435,     5,   455,   417,   454,   440,   453,   440,
     453,   425,   438,   209,   456,   455,   455,   440,   107,   108,
     455,    85,   426,   431,   455,   455,   109,   105,   455,   455,
     110,    14,   427,   428,   455,   429,   105,   428,   438,   455,
     430,   438,   455,   105,   455,   438
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   111,   112,   112,   113,   114,   115,   116,   115,   117,
     118,   119,   120,   120,   120,   120,   121,   122,   123,   124,
     124,   124,   125,   126,   127,   128,   129,   130,   130,   130,
     131,   132,   133,   133,   133,   133,   133,   134,   135,   136,
     136,   137,   137,   137,   137,   138,   139,   140,   141,   142,
     143,   144,   144,   144,   144,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   155,   156,   157,
     157,   158,   158,   158,   160,   159,   159,   161,   162,   162,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     164,   165,   166,   166,   167,   167,   167,   167,   167,   168,
     169,   170,   170,   171,   172,   172,   173,   173,   173,   174,
     175,   176,   176,   176,   176,   177,   178,   179,   179,   180,
     180,   180,   180,   180,   181,   182,   183,   184,   185,   185,
     186,   186,   186,   186,   186,   186,   187,   188,   189,   190,
     191,   191,   191,   191,   191,   191,   191,   192,   193,   194,
     194,   195,   195,   195,   196,   196,   196,   196,   196,   196,
     196,   196,   196,   197,   198,   199,   200,   200,   201,   202,
     203,   203,   204,   204,   204,   204,   204,   205,   206,   207,
     207,   208,   208,   208,   208,   208,   208,   209,   210,   211,
     212,   212,   213,   214,   215,   215,   216,   216,   216,   216,
     216,   216,   217,   218,   219,   219,   220,   221,   221,   222,
     223,   224,   225,   226,   226,   226,   227,   228,   228,   229,
     230,   231,   231,   232,   232,   232,   232,   232,   232,   233,
     234,   235,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   246,   247,   247,   247,   247,   247,
     247,   247,   247,   247,   248,   248,   248,   248,   248,   248,
     248,   248,   249,   250,   251,   251,   252,   253,   252,   252,
     254,   252,   252,   252,   252,   252,   252,   255,   252,   256,
     252,   257,   252,   258,   252,   259,   252,   260,   261,   262,
     263,   262,   262,   264,   262,   262,   265,   262,   262,   262,
     262,   262,   262,   262,   266,   267,   268,   269,   268,   268,
     270,   268,   268,   268,   271,   268,   268,   268,   268,   268,
     268,   268,   268,   272,   273,   274,   275,   274,   274,   276,
     274,   274,   274,   274,   274,   274,   274,   274,   274,   274,
     277,   274,   278,   274,   279,   274,   280,   274,   281,   274,
     282,   283,   284,   284,   285,   286,   285,   285,   285,   285,
     285,   285,   285,   285,   287,   285,   288,   288,   289,   289,
     289,   289,   289,   289,   289,   290,   291,   292,   292,   293,
     294,   293,   293,   293,   293,   293,   295,   293,   296,   297,
     298,   299,   300,   300,   301,   302,   301,   301,   303,   301,
     301,   301,   301,   301,   301,   304,   305,   306,   306,   307,
     308,   307,   307,   307,   307,   307,   309,   307,   310,   307,
     311,   311,   312,   313,   314,   314,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   316,   316,   317,
     318,   319,   319,   320,   321,   322,   323,   322,   322,   322,
     322,   322,   324,   322,   325,   322,   326,   322,   327,   322,
     328,   329,   330,   331,   332,   332,   333,   333,   333,   333,
     333,   334,   333,   335,   333,   336,   333,   337,   333,   338,
     339,   340,   341,   340,   342,   340,   343,   340,   344,   340,
     345,   346,   347,   348,   349,   349,   350,   351,   350,   350,
     350,   350,   350,   352,   350,   353,   350,   354,   350,   355,
     356,   357,   357,   357,   357,   358,   358,   359,   359,   360,
     361,   362,   362,   362,   364,   363,   365,   365,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   368,   367,   369,
     369,   370,   370,   370,   370,   370,   372,   371,   373,   373,
     374,   374,   374,   374,   374,   374,   374,   374,   375,   376,
     377,   377,   378,   378,   378,   379,   380,   381,   381,   381,
     382,   383,   384,   384,   384,   385,   386,   387,   388,   389,
     389,   390,   391,   392,   392,   392,   393,   394,   395,   395,
     395,   396,   397,   398,   398,   399,   400,   401,   401,   402,
     402,   403,   404,   406,   405,   405,   407,   408,   409,   410,
     410,   412,   411,   414,   413,   415,   413,   413,   416,   417,
     418,   418,   419,   420,   421,   421,   422,   423,   423,   424,
     424,   425,   426,   427,   428,   429,   429,   430,   430,   431,
     432,   433,   433,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   439,   440,   440,   441,   442,
     443,   444,   444,   444,   445,   446,   447,   448,   448,   449,
     449,   450,   450,   451,   451,   452,   452,   453,   453,   454,
     455,   455,   457,   456,   456,   458,   458,   458,   458,   458,
     458,   458,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   460,   461
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     0,     0,     6,     1,
      13,     1,     0,     2,     2,     2,     1,    13,     1,     0,
       2,     3,     1,     4,     1,     4,     1,     0,     3,     3,
       7,     1,     0,     2,     2,     2,     2,     1,     4,     1,
       4,     0,     2,     2,     2,     1,     4,     1,     7,     1,
       4,     0,     2,     2,     2,     2,     1,     4,     1,     4,
       1,     4,     1,     4,     1,     1,     0,     3,     4,     1,
       4,     0,     2,     2,     0,     3,     1,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     4,     0,     3,     2,     2,     2,     1,
       4,     1,     4,     1,     0,     4,     2,     2,     1,     1,
       4,     2,     2,     2,     1,     1,     4,     1,     4,     0,
       2,     2,     2,     2,     1,     4,     1,     4,     1,     4,
       0,     2,     2,     2,     2,     2,     1,     4,     1,     7,
       0,     3,     2,     2,     2,     2,     2,     4,     1,     1,
       4,     1,     1,     1,     0,     2,     2,     2,     2,     2,
       2,     3,     4,     0,     1,     4,     2,     1,     5,     1,
       1,     4,     0,     2,     2,     2,     2,     5,     1,     1,
       4,     0,     2,     2,     2,     2,     2,     1,     4,     3,
       0,     3,     4,     1,     1,     4,     0,     2,     2,     2,
       2,     2,     1,     4,     2,     1,     4,     1,     4,     1,
       4,     1,     4,     2,     2,     1,     2,     0,     2,     5,
       1,     1,     4,     0,     2,     2,     2,     2,     2,     1,
       4,     2,     1,     1,     4,     1,     4,     1,     4,     1,
       4,     1,     4,     1,     4,     0,     2,     2,     2,     3,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     4,     0,     0,     4,     2,
       0,     4,     2,     2,     2,     2,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     7,     1,     0,
       0,     4,     2,     0,     4,     2,     0,     4,     2,     2,
       2,     2,     2,     2,     1,     7,     0,     0,     4,     2,
       0,     4,     2,     2,     0,     4,     2,     2,     2,     2,
       2,     2,     2,     1,     7,     0,     0,     4,     2,     0,
       4,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       1,     4,     1,     4,     0,     0,     4,     2,     2,     2,
       2,     2,     2,     2,     0,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     4,     0,
       0,     4,     2,     2,     2,     2,     0,     4,     1,     4,
       1,     4,     1,     4,     0,     0,     4,     2,     0,     4,
       2,     2,     2,     2,     2,     1,     4,     1,     4,     0,
       0,     4,     2,     2,     2,     2,     0,     4,     0,     4,
       2,     1,     1,     4,     1,     4,     0,     3,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       7,     2,     1,     1,     7,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     4,     0,     4,
       1,     4,     1,     4,     1,     4,     0,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     4,     2,
       5,     0,     0,     4,     0,     4,     0,     4,     0,     4,
       2,     5,     1,     4,     1,     4,     0,     0,     4,     2,
       2,     2,     2,     0,     4,     0,     4,     0,     4,     1,
       7,     0,     2,     2,     4,     2,     1,     1,     2,     1,
       3,     1,     1,     1,     0,     4,     1,     4,     0,     2,
       3,     2,     2,     2,     2,     2,     2,     0,     4,     1,
       4,     0,     2,     3,     2,     2,     0,     4,     1,     4,
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
       1,     0,     2,     2,     3,     2,     1,     0,     1,     1,
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

    case 206: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 441: /* semicolom  */

      { free(((*yyvaluep).str)); }

        break;

    case 443: /* curly_bracket_open  */

      { free(((*yyvaluep).str)); }

        break;

    case 447: /* string_opt_part1  */

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
                  param->value = &s;
                  param->data_node = (void **)&data_node;
                  param->actual_node = &actual;
                  backup_type = NODE;
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
                                     actual_type = MODULE_KEYWORD;
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
                                         s = NULL;
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
                                        actual_type = SUBMODULE_KEYWORD;
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

  case 22:

    { backup_type = actual_type;
                           actual_type = YANG_VERSION_KEYWORD;
                         }

    break;

  case 24:

    { backup_type = actual_type;
                            actual_type = NAMESPACE_KEYWORD;
                          }

    break;

  case 26:

    { *param->remove_import = 0;
                              if (yang_check_imports(trg, param->unres)) {
                                YYABORT;
                              }
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
                                         s = NULL;
                                         actual_type = BELONGS_TO_KEYWORD;
                                       }

    break;

  case 48:

    { actual_type = (yyvsp[-4].token); }

    break;

  case 49:

    { backup_type = actual_type;
                             actual_type = PREFIX_KEYWORD;
                           }

    break;

  case 50:

    { if (yang_read_prefix(trg, actual, s)) {
                                                       YYABORT;
                                                     }
                                                     s = NULL;
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

    { actual = NULL; }

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
                                      YANG_ADDELEM(trg->features, trg->features_size);
                                      ((struct lys_feature *)actual)->name = lydict_insert_zc(trg->ctx, s);
                                      ((struct lys_feature *)actual)->module = trg;
                                      s = NULL;
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

  case 120:

    { void *feature;

                                        feature = actual;
                                        YANG_ADDELEM(((struct lys_feature *)actual)->iffeature,
                                                     ((struct lys_feature *)actual)->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                                        s = NULL;
                                        actual = feature;
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

    { backup_type = actual_type;
                         actual_type = IF_FEATURE_KEYWORD;
                       }

    break;

  case 126:

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

  case 129:

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

  case 131:

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

  case 132:

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

  case 133:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 134:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 135:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 136:

    { backup_type = actual_type;
                                   actual_type = BASE_KEYWORD;
                                 }

    break;

  case 138:

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

  case 139:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual = tpdf_parent;
                  }

    break;

  case 140:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 141:

    { actual = (yyvsp[-2].nodes).node.ptr_tpdf;
                                       actual_type = TYPEDEF_KEYWORD;
                                       (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 142:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 143:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 144:

    { if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 145:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 146:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 148:

    { if (!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 151:

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

  case 155:

    { if (yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 156:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 157:

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

    { actual = (yyvsp[-2].v); }

    break;

  case 163:

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

  case 164:

    { (yyval.uint) = (yyvsp[0].uint);
                                               backup_type = actual_type;
                                               actual_type = FRACTION_DIGITS_KEYWORD;
                                             }

    break;

  case 165:

    { if (yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 166:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 167:

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

  case 168:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 169:

    { (yyval.v) = actual;
                         if (!(actual = yang_read_length(trg, actual, s))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 172:

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

  case 173:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 174:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 175:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 176:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 177:

    { if (yang_read_pattern(trg, actual, (yyvsp[-2].str), (yyvsp[-1].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual = yang_type;
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 178:

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

  case 179:

    { (yyval.ch) = 0x06; }

    break;

  case 180:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 181:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 182:

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

  case 183:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 184:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 185:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 186:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 187:

    { backup_type = actual_type;
                       actual_type = MODIFIER_KEYWORD;
                     }

    break;

  case 188:

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

  case 189:

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

  case 192:

    { if (yang_check_enum(yang_type, actual, &cnt_val, is_value)) {
               YYABORT;
             }
             actual = yang_type;
             actual_type = TYPE_KEYWORD;
           }

    break;

  case 193:

    { yang_type = actual;
                       YANG_ADDELEM(((struct yang_type *)actual)->type->info.enums.enm, ((struct yang_type *)actual)->type->info.enums.count);
                       if (yang_read_enum(trg, yang_type, actual, s)) {
                         YYABORT;
                       }
                       s = NULL;
                       is_value = 0;
                     }

    break;

  case 195:

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

  case 197:

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

  case 198:

    { if (is_value) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                is_value = 1;
                              }

    break;

  case 199:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 200:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 201:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 202:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = VALUE_KEYWORD;
                                 }

    break;

  case 203:

    { ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                      /* keep the highest enum value for automatic increment */
                      if ((yyvsp[-1].i) >= cnt_val) {
                        cnt_val = (yyvsp[-1].i);
                        cnt_val++;
                      }
                    }

    break;

  case 204:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 205:

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

  case 206:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 209:

    { backup_type = actual_type;
                         actual_type = PATH_KEYWORD;
                       }

    break;

  case 211:

    { (yyval.i) = (yyvsp[0].i);
                                                 backup_type = actual_type;
                                                 actual_type = REQUIRE_INSTANCE_KEYWORD;
                                               }

    break;

  case 212:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 213:

    { (yyval.i) = 1; }

    break;

  case 214:

    { (yyval.i) = -1; }

    break;

  case 215:

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

  case 216:

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

  case 219:

    { if (yang_check_bit(yang_type, actual, &cnt_val, is_value)) {
                      YYABORT;
                    }
                    actual = yang_type;
                    actual_type = TYPE_KEYWORD;
                  }

    break;

  case 220:

    { yang_type = actual;
                                  YANG_ADDELEM(((struct yang_type *)actual)->type->info.bits.bit,
                                               ((struct yang_type *)actual)->type->info.bits.count);
                                  if (yang_read_bit(trg, yang_type, actual, s)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  is_value = 0;
                                }

    break;

  case 222:

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

  case 224:

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

  case 225:

    { if (is_value) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  is_value = 1;
                                }

    break;

  case 226:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags,
                                                     LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                  YYABORT;
                                }
                              }

    break;

  case 227:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 228:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 229:

    { (yyval.uint) = (yyvsp[0].uint);
                                             backup_type = actual_type;
                                             actual_type = POSITION_KEYWORD;
                                           }

    break;

  case 230:

    { ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                         /* keep the highest position value for automatic increment */
                         if ((yyvsp[-1].uint) >= cnt_val) {
                           cnt_val = (yyvsp[-1].uint);
                           cnt_val++;
                         }
                       }

    break;

  case 231:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 232:

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

  case 233:

    { backup_type = actual_type;
                            actual_type = ERROR_MESSAGE_KEYWORD;
                          }

    break;

  case 235:

    { backup_type = actual_type;
                            actual_type = ERROR_APP_TAG_KEYWORD;
                          }

    break;

  case 237:

    { backup_type = actual_type;
                    actual_type = UNITS_KEYWORD;
                  }

    break;

  case 239:

    { backup_type = actual_type;
                      actual_type = DEFAULT_KEYWORD;
                    }

    break;

  case 241:

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

  case 242:

    { LOGDBG("YANG: finished parsing grouping statement \"%s\"", data_node->name);
                 actual_type = (yyvsp[-1].backup_token).token;
                 actual = (yyvsp[-1].backup_token).actual;
               }

    break;

  case 245:

    { (yyval.nodes).grouping = actual; }

    break;

  case 246:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 247:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 248:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 249:

    { actual = (yyvsp[-2].nodes).grouping;
                                               actual_type = GROUPING_KEYWORD;
                                             }

    break;

  case 250:

    { actual = (yyvsp[-2].nodes).grouping;
                                              actual_type = GROUPING_KEYWORD;
                                            }

    break;

  case 251:

    { actual = (yyvsp[-2].nodes).grouping;
                                               actual_type = GROUPING_KEYWORD;
                                             }

    break;

  case 252:

    { actual = (yyvsp[-2].nodes).grouping;
                                             actual_type = GROUPING_KEYWORD;
                                           }

    break;

  case 253:

    { actual = (yyvsp[-2].nodes).grouping;
                                                   actual_type = GROUPING_KEYWORD;
                                                   if (trg->version < 2) {
                                                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 262:

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

  case 263:

    { LOGDBG("YANG: finished parsing container statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                  data_node = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 265:

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

  case 266:

    { (yyval.nodes).container = actual; }

    break;

  case 267:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 269:

    { YANG_ADDELEM((yyvsp[-1].nodes).container->iffeature, (yyvsp[-1].nodes).container->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).container;
                                          s = NULL;
                                        }

    break;

  case 270:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 272:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 273:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 274:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 275:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 276:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 277:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 279:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 281:

    { actual = (yyvsp[-1].nodes).container;
                                            actual_type = CONTAINER_KEYWORD;
                                            data_node = actual;
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "notification");
                                              YYABORT;
                                            }
                                          }

    break;

  case 283:

    { actual = (yyvsp[-1].nodes).container;
                                       actual_type = CONTAINER_KEYWORD;
                                     }

    break;

  case 285:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 287:

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

  case 288:

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

  case 289:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 290:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 292:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaf->iffeature, (yyvsp[-1].nodes).node.ptr_leaf->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                     s = NULL;
                                   }

    break;

  case 293:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                               (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                             }

    break;

  case 294:

    { (yyval.nodes) = (yyvsp[-3].nodes);}

    break;

  case 295:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 296:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 298:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 299:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 300:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 301:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 302:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 303:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 304:

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

  case 305:

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

  case 306:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                               }

    break;

  case 307:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 309:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaflist->iffeature,
                                                       (yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                          s = NULL;
                                        }

    break;

  case 310:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                                  }

    break;

  case 311:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 312:

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

  case 313:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 314:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 316:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 317:

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

  case 318:

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

  case 319:

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

  case 320:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 321:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 322:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 323:

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

  case 324:

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

  case 325:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 326:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 328:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->iffeature,
                                                  (yyvsp[-1].nodes).node.ptr_list->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_list;
                                     s = NULL;
                                   }

    break;

  case 329:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 331:

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

  case 332:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 333:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 334:

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

  case 335:

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

  case 336:

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

  case 337:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 338:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 339:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 340:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                  actual_type = LIST_KEYWORD;
                                }

    break;

  case 342:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 344:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 346:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                       actual_type = LIST_KEYWORD;
                                       data_node = actual;
                                       if (trg->version < 2) {
                                         LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "notification");
                                         YYABORT;
                                       }
                                     }

    break;

  case 348:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 349:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 350:

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

  case 351:

    { LOGDBG("YANG: finished parsing choice statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 353:

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

  case 354:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                            }

    break;

  case 355:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                 actual_type = CHOICE_KEYWORD;
                               }

    break;

  case 356:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 357:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_choice->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_choice->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_choice;
                                       s = NULL;
                                     }

    break;

  case 358:

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

  case 359:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 360:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 361:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 362:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 363:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 364:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                          }

    break;

  case 365:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 374:

    { if (trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 375:

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

  case 376:

    { LOGDBG("YANG: finished parsing case statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 378:

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

  case 379:

    { (yyval.nodes).cs = actual; }

    break;

  case 380:

    { actual = (yyvsp[-1].nodes).cs;
                               actual_type = CASE_KEYWORD;
                             }

    break;

  case 382:

    { YANG_ADDELEM((yyvsp[-1].nodes).cs->iffeature, (yyvsp[-1].nodes).cs->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).cs;
                                     s = NULL;
                                   }

    break;

  case 383:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 384:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 385:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 386:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 388:

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

  case 389:

    { LOGDBG("YANG: finished parsing anyxml statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 390:

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

  case 391:

    { LOGDBG("YANG: finished parsing anydata statement \"%s\"", data_node->name);
                actual_type = (yyvsp[-1].backup_token).token;
                actual = (yyvsp[-1].backup_token).actual;
                data_node = (yyvsp[-1].backup_token).actual;
              }

    break;

  case 393:

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

  case 394:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
                            }

    break;

  case 395:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 397:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_anydata->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_anydata->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                       s = NULL;
                                     }

    break;

  case 398:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 400:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "config",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 401:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_MAND_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "mandatory",
                                               ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                    }

    break;

  case 402:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "status",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 403:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 404:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 405:

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

  case 406:

    { LOGDBG("YANG: finished parsing uses statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 408:

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

  case 409:

    { (yyval.nodes).uses = actual; }

    break;

  case 410:

    { actual = (yyvsp[-1].nodes).uses;
                               actual_type = USES_KEYWORD;
                             }

    break;

  case 412:

    { YANG_ADDELEM((yyvsp[-1].nodes).uses->iffeature, (yyvsp[-1].nodes).uses->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).uses;
                                     s = NULL;
                                   }

    break;

  case 413:

    { if ((yyvsp[-1].nodes).uses->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).uses, "status", "uses");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).uses->flags |= (yyvsp[0].i);
                               }

    break;

  case 414:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 415:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 416:

    { actual = (yyvsp[-1].nodes).uses;
                                 actual_type = USES_KEYWORD;
                               }

    break;

  case 418:

    { actual = (yyvsp[-1].nodes).uses;
                                       actual_type = USES_KEYWORD;
                                       data_node = actual;
                                     }

    break;

  case 422:

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

  case 425:

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

  case 426:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 427:

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

  case 428:

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

  case 429:

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

  case 430:

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

  case 431:

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

  case 432:

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

  case 433:

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

  case 434:

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

  case 435:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 436:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 439:

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

  case 440:

    { LOGDBG("YANG: finished parsing augment statement \"%s\"", data_node->name);
                         actual_type = (yyvsp[-4].backup_token).token;
                         actual = (yyvsp[-4].backup_token).actual;
                         data_node = (yyvsp[-4].backup_token).actual;
                       }

    break;

  case 443:

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

  case 444:

    { LOGDBG("YANG: finished parsing augment statement \"%s\"", data_node->name);
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                    data_node = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 445:

    { (yyval.nodes).augment = actual; }

    break;

  case 446:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                }

    break;

  case 448:

    { YANG_ADDELEM((yyvsp[-1].nodes).augment->iffeature, (yyvsp[-1].nodes).augment->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                        actual = (yyvsp[-1].nodes).augment;
                                        s = NULL;
                                      }

    break;

  case 449:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 450:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 451:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 452:

    { actual = (yyvsp[-1].nodes).augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 454:

    { actual = (yyvsp[-1].nodes).augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    data_node = actual;
                                  }

    break;

  case 456:

    { actual = (yyvsp[-1].nodes).augment;
                                          actual_type = AUGMENT_KEYWORD;
                                          data_node = actual;
                                          if (trg->version < 2) {
                                            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "notification");
                                            YYABORT;
                                          }
                                        }

    break;

  case 458:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 460:

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

  case 461:

    { LOGDBG("YANG: finished parsing action statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 462:

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

  case 463:

    { LOGDBG("YANG: finished parsing rpc statement \"%s\"", data_node->name);
            actual_type = (yyvsp[-1].backup_token).token;
            actual = (yyvsp[-1].backup_token).actual;
            data_node = (yyvsp[-1].backup_token).actual;
          }

    break;

  case 465:

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

  case 466:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                         }

    break;

  case 467:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_rpc->iffeature,
                                                 (yyvsp[-1].nodes).node.ptr_rpc->iffeature_size);
                                    ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                    actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                    s = NULL;
                                  }

    break;

  case 468:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 469:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 470:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 471:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                 actual_type = RPC_KEYWORD;
                               }

    break;

  case 473:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 475:

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

  case 476:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 477:

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

  case 478:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 479:

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

  case 480:

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
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 481:

    { (yyval.nodes).inout = actual;
                                    actual_type = INPUT_KEYWORD;
                                  }

    break;

  case 482:

    { actual = (yyvsp[-1].nodes).inout;
                                       actual_type = INPUT_KEYWORD;
                                     }

    break;

  case 484:

    { actual = (yyvsp[-1].nodes).inout;
                                          actual_type = INPUT_KEYWORD;
                                        }

    break;

  case 486:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 488:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 490:

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

  case 491:

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
                   actual_type = (yyvsp[-4].backup_token).token;
                   actual = (yyvsp[-4].backup_token).actual;
                   data_node = (yyvsp[-4].backup_token).actual;
                 }

    break;

  case 492:

    { (yyval.backup_token).token = actual_type;
                                           (yyval.backup_token).actual = actual;
                                           if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                           actual_type = NOTIFICATION_KEYWORD;
                                         }

    break;

  case 493:

    { LOGDBG("YANG: finished parsing notification statement \"%s\"", data_node->name);
                     actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                     data_node = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 495:

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

  case 496:

    { (yyval.nodes).notif = actual;
                                    actual_type = NOTIFICATION_KEYWORD;
                                  }

    break;

  case 497:

    { actual = (yyvsp[-1].nodes).notif;
                                       actual_type = NOTIFICATION_KEYWORD;
                                     }

    break;

  case 499:

    { YANG_ADDELEM((yyvsp[-1].nodes).notif->iffeature, (yyvsp[-1].nodes).notif->iffeature_size);
                                             ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                             actual = (yyvsp[-1].nodes).notif;
                                             s = NULL;
                                           }

    break;

  case 500:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 501:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 502:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 503:

    { actual = (yyvsp[-1].nodes).notif;
                                          actual_type = NOTIFICATION_KEYWORD;
                                        }

    break;

  case 505:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 507:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 509:

    { YANG_ADDELEM(trg->deviation, trg->deviation_size);
                                   ((struct lys_deviation *)actual)->target_name = transform_schema2json(trg, s);
                                   free(s);
                                   if (!((struct lys_deviation *)actual)->target_name) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 510:

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

  case 511:

    { (yyval.dev) = actual;
                                 actual_type = DEVIATION_KEYWORD;
                               }

    break;

  case 512:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 513:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 514:

    { actual = (yyvsp[-3].dev);
                                                                actual_type = DEVIATION_KEYWORD;
                                                                (yyval.dev) = (yyvsp[-3].dev);
                                                              }

    break;

  case 517:

    { if (yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 519:

    {  backup_type = actual_type;
                                                actual_type = NOT_SUPPORTED_KEYWORD;
                                             }

    break;

  case 524:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_ADD))) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 527:

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

  case 528:

    { (yyval.deviate) = actual;
                                   actual_type = ADD_KEYWORD;
                                 }

    break;

  case 529:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 530:

    { actual = (yyvsp[-2].deviate);
                                              actual_type = ADD_KEYWORD;
                                              (yyval.deviate) = (yyvsp[-2].deviate);
                                            }

    break;

  case 531:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 532:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 533:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 534:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 535:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 536:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 537:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_DEL))) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 540:

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

  case 541:

    { (yyval.deviate) = actual;
                                      actual_type = DELETE_KEYWORD;
                                    }

    break;

  case 542:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 543:

    { actual = (yyvsp[-2].deviate);
                                                 actual_type = DELETE_KEYWORD;
                                                 (yyval.deviate) = (yyvsp[-2].deviate);
                                               }

    break;

  case 544:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 545:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 546:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_RPL))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 549:

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

  case 550:

    { (yyval.deviate) = actual;
                                       actual_type = REPLACE_KEYWORD;
                                     }

    break;

  case 551:

    { actual = (yyvsp[-2].deviate);
                                                  actual_type = REPLACE_KEYWORD;
                                                }

    break;

  case 552:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 553:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 554:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 555:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 556:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 557:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 558:

    { if (!(actual=yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type=WHEN_KEYWORD;
                      }

    break;

  case 563:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 564:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 565:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = CONFIG_KEYWORD;
                           }

    break;

  case 566:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 567:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 568:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 569:

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

  case 570:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = MANDATORY_KEYWORD;
                                 }

    break;

  case 571:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 572:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 573:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 574:

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

  case 575:

    { backup_type = actual_type;
                       actual_type = PRESENCE_KEYWORD;
                     }

    break;

  case 577:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MIN_ELEMENTS_KEYWORD;
                                 }

    break;

  case 578:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 579:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 580:

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

  case 581:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MAX_ELEMENTS_KEYWORD;
                                 }

    break;

  case 582:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 583:

    { (yyval.uint) = 0; }

    break;

  case 584:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 585:

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

  case 586:

    { (yyval.i) = (yyvsp[0].i);
                                     backup_type = actual_type;
                                     actual_type = ORDERED_BY_KEYWORD;
                                   }

    break;

  case 587:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 588:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 589:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 590:

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

  case 591:

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

  case 595:

    { backup_type = actual_type;
                             actual_type = UNIQUE_KEYWORD;
                           }

    break;

  case 601:

    { backup_type = actual_type;
                       actual_type = KEY_KEYWORD;
                     }

    break;

  case 603:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }

    break;

  case 606:

    { (yyval.v) = actual;
                        if (!(actual = yang_read_range(trg, actual, s))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 607:

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

  case 611:

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

  case 613:

    { tmp_s = yyget_text(scanner); }

    break;

  case 614:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM;
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 615:

    { tmp_s = yyget_text(scanner); }

    break;

  case 616:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM;
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 640:

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

  case 641:

    { (yyval.uint) = 0; }

    break;

  case 642:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 643:

    { (yyval.i) = 0; }

    break;

  case 644:

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

  case 650:

    { if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 655:

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

  case 656:

    { s = (yyvsp[-1].str); }

    break;

  case 657:

    { s = (yyvsp[-3].str); }

    break;

  case 658:

    { actual_type = backup_type;
                 backup_type = NODE;
                 (yyval.str) = s;
                 s = NULL;
               }

    break;

  case 659:

    { actual_type = backup_type;
                           backup_type = NODE;
                         }

    break;

  case 660:

    { (yyval.str) = s;
                          s = NULL;
                        }

    break;

  case 665:

    { if (!yang_read_ext(trg, actual, (yyvsp[-1].str), s, actual_type, backup_type)) {
                                                  YYABORT;
                                                }
                                                s = NULL;
                                              }

    break;

  case 666:

    { (yyval.str) = s; s = NULL; }

    break;

  case 682:

    { s = strdup(yyget_text(scanner));
                  if (!s) {
                    LOGMEM;
                    YYABORT;
                  }
                }

    break;

  case 773:

    { s = strdup(yyget_text(scanner));
                          if (!s) {
                            LOGMEM;
                            YYABORT;
                          }
                        }

    break;

  case 774:

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
