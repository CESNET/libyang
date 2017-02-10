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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3363

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  313
/* YYNRULES -- Number of rules.  */
#define YYNRULES  743
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1218

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
       0,   312,   312,   313,   315,   338,   341,   343,   342,   366,
     377,   387,   397,   398,   404,   409,   412,   423,   433,   446,
     447,   453,   455,   459,   461,   465,   467,   468,   469,   471,
     479,   487,   488,   489,   500,   511,   522,   530,   535,   536,
     540,   541,   552,   563,   574,   578,   580,   596,   601,   605,
     611,   612,   617,   622,   627,   633,   637,   639,   643,   645,
     649,   651,   655,   657,   670,   676,   677,   679,   683,   684,
     688,   689,   694,   701,   701,   708,   714,   762,   781,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   796,
     811,   818,   819,   823,   824,   825,   831,   836,   842,   853,
     855,   856,   860,   865,   866,   868,   869,   870,   883,   888,
     890,   891,   892,   893,   908,   922,   927,   928,   943,   944,
     945,   951,   956,   962,  1016,  1021,  1022,  1024,  1040,  1045,
    1046,  1071,  1072,  1086,  1087,  1093,  1098,  1104,  1108,  1110,
    1160,  1171,  1174,  1177,  1182,  1187,  1193,  1198,  1204,  1209,
    1218,  1219,  1223,  1258,  1259,  1261,  1262,  1266,  1272,  1285,
    1286,  1287,  1288,  1289,  1291,  1295,  1313,  1318,  1324,  1325,
    1341,  1346,  1355,  1356,  1360,  1376,  1381,  1386,  1391,  1397,
    1402,  1409,  1422,  1423,  1427,  1428,  1438,  1443,  1448,  1453,
    1459,  1463,  1474,  1486,  1487,  1490,  1498,  1509,  1510,  1525,
    1526,  1527,  1533,  1537,  1542,  1548,  1553,  1563,  1564,  1579,
    1584,  1585,  1590,  1594,  1596,  1601,  1603,  1604,  1605,  1618,
    1630,  1631,  1633,  1641,  1653,  1654,  1669,  1670,  1671,  1677,
    1682,  1687,  1693,  1698,  1708,  1709,  1725,  1729,  1731,  1735,
    1737,  1741,  1743,  1747,  1749,  1759,  1766,  1767,  1771,  1772,
    1778,  1783,  1788,  1789,  1790,  1791,  1792,  1798,  1799,  1800,
    1801,  1802,  1803,  1804,  1805,  1808,  1818,  1825,  1826,  1849,
    1850,  1851,  1852,  1853,  1858,  1864,  1870,  1875,  1880,  1881,
    1882,  1887,  1888,  1890,  1930,  1940,  1943,  1944,  1945,  1948,
    1953,  1954,  1959,  1965,  1971,  1977,  1982,  1988,  1998,  2053,
    2056,  2057,  2058,  2061,  2072,  2077,  2078,  2084,  2097,  2110,
    2120,  2126,  2131,  2137,  2147,  2194,  2197,  2198,  2199,  2200,
    2209,  2215,  2221,  2234,  2247,  2257,  2263,  2268,  2273,  2274,
    2275,  2276,  2281,  2283,  2293,  2300,  2301,  2321,  2324,  2325,
    2326,  2336,  2343,  2350,  2357,  2363,  2369,  2371,  2372,  2374,
    2375,  2376,  2377,  2378,  2379,  2380,  2386,  2396,  2403,  2404,
    2418,  2419,  2420,  2421,  2427,  2432,  2437,  2440,  2450,  2457,
    2467,  2474,  2475,  2498,  2501,  2502,  2503,  2504,  2511,  2518,
    2525,  2530,  2536,  2546,  2553,  2554,  2586,  2587,  2588,  2589,
    2595,  2600,  2605,  2606,  2608,  2609,  2611,  2624,  2629,  2630,
    2662,  2665,  2679,  2695,  2717,  2768,  2785,  2802,  2823,  2844,
    2849,  2855,  2856,  2859,  2874,  2883,  2884,  2886,  2897,  2906,
    2907,  2908,  2909,  2915,  2920,  2925,  2926,  2927,  2932,  2934,
    2949,  2956,  2966,  2973,  2974,  2998,  3001,  3002,  3008,  3013,
    3018,  3019,  3020,  3027,  3035,  3050,  3080,  3081,  3082,  3083,
    3084,  3086,  3101,  3131,  3140,  3147,  3148,  3180,  3181,  3182,
    3183,  3189,  3194,  3199,  3200,  3201,  3203,  3215,  3235,  3236,
    3242,  3248,  3250,  3251,  3253,  3254,  3257,  3265,  3270,  3271,
    3273,  3274,  3275,  3277,  3285,  3290,  3291,  3323,  3324,  3330,
    3331,  3337,  3343,  3350,  3357,  3365,  3374,  3382,  3387,  3388,
    3420,  3421,  3427,  3428,  3434,  3441,  3449,  3454,  3455,  3469,
    3470,  3471,  3477,  3483,  3490,  3497,  3505,  3514,  3523,  3528,
    3529,  3533,  3534,  3539,  3545,  3550,  3552,  3553,  3554,  3567,
    3572,  3574,  3575,  3576,  3589,  3593,  3595,  3600,  3602,  3603,
    3623,  3628,  3630,  3631,  3632,  3652,  3657,  3659,  3660,  3661,
    3673,  3738,  3743,  3744,  3748,  3752,  3754,  3755,  3757,  3758,
    3760,  3764,  3766,  3766,  3773,  3776,  3785,  3804,  3806,  3807,
    3810,  3810,  3827,  3827,  3834,  3834,  3841,  3844,  3846,  3848,
    3849,  3851,  3853,  3855,  3856,  3858,  3860,  3861,  3863,  3864,
    3866,  3868,  3871,  3875,  3877,  3878,  3880,  3881,  3883,  3885,
    3896,  3897,  3900,  3901,  3912,  3913,  3915,  3916,  3918,  3919,
    3925,  3926,  3929,  3930,  3931,  3955,  3956,  3959,  3965,  3969,
    3974,  3975,  3976,  3979,  3984,  3993,  3995,  3996,  3998,  3999,
    4001,  4002,  4003,  4005,  4006,  4008,  4009,  4011,  4012,  4016,
    4017,  4019,  4020,  4022,  4030,  4031,  4034,  4035,  4038,  4040,
    4041,  4044,  4044,  4051,  4053,  4054,  4055,  4056,  4057,  4058,
    4059,  4061,  4062,  4063,  4064,  4065,  4066,  4067,  4068,  4069,
    4070,  4071,  4072,  4073,  4074,  4075,  4076,  4077,  4078,  4079,
    4080,  4081,  4082,  4083,  4084,  4085,  4086,  4087,  4088,  4089,
    4090,  4091,  4092,  4093,  4094,  4095,  4096,  4097,  4098,  4099,
    4100,  4101,  4102,  4103,  4104,  4105,  4106,  4107,  4108,  4109,
    4110,  4111,  4112,  4113,  4114,  4115,  4116,  4117,  4118,  4119,
    4120,  4121,  4122,  4123,  4124,  4125,  4126,  4127,  4128,  4129,
    4130,  4131,  4132,  4133,  4134,  4135,  4136,  4137,  4138,  4139,
    4140,  4141,  4144,  4151
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
  "unknown_statement2_identifier", "sep_stmt", "optsep", "sep",
  "whitespace_opt", "string", "$@7", "strings", "identifier",
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
     355,   356,    43,   123,   125,    59,    47,    91,    93,    61,
      40,    41
};
# endif

#define YYPACT_NINF -1001

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1001)))

#define YYTABLE_NINF -589

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1001,    69, -1001, -1001,   250, -1001, -1001, -1001,   150,   150,
   -1001, -1001,  3170,  3170,   150, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   -29,
   -1001, -1001, -1001,   -18, -1001,   150, -1001,   150, -1001,   -14,
     191,   191, -1001, -1001, -1001,    59, -1001, -1001, -1001, -1001,
      11,   363,   340,   150,   150,   150, -1001, -1001, -1001,    80,
    2505,   150,   340,   150, -1001, -1001, -1001,   150,   150, -1001,
   -1001,   423,  2505,  3170,  2505, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   423,
    3170,   191, -1001,  3170,  3170,   191,   191,   150,   150,   150,
     150, -1001, -1001, -1001, -1001, -1001,    43,    85, -1001, -1001,
      85, -1001, -1001,    85, -1001,   150, -1001, -1001,    76, -1001,
     150,   110, -1001,    96, -1001,  2505,  2505,  2505,  2505,    -5,
    1343,   150, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   150,
   -1001,  2313,   150,    28, -1001, -1001, -1001, -1001, -1001,    85,
   -1001,    85, -1001,    85, -1001,    85, -1001, -1001,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001,    54,   191,   191,    12, -1001, -1001,
   -1001,   150, -1001, -1001,   209,   191,   191, -1001, -1001, -1001,
   -1001,   150,  3170,     6,  3170,  3170,  3170,     6,  3170,  3170,
    3170,  3170,  3170,  3170,  3170,  3170,  3170,  2695,  3170,   191,
   -1001, -1001,   116, -1001, -1001, -1001,   150,   195,  2600,   150,
     114,   248,    83,   213, -1001,  3265, -1001, -1001,   138, -1001,
   -1001,   228, -1001,   345, -1001,   361, -1001, -1001,   143, -1001,
   -1001,   386, -1001,   392, -1001,   400, -1001,   147, -1001,   158,
   -1001,   161, -1001,   403, -1001,   431, -1001,   168, -1001, -1001,
     468, -1001, -1001, -1001,   213, -1001, -1001, -1001, -1001, -1001,
     191, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   150, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001,   175,   150, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001,   150, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
     150,   150, -1001,   150,   191,   150,   150,   150,    54,   191,
     191, -1001,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,    -6,  2408,   151,    85,
   -1001,   493,  1269,   961,  1085,   234,   358,   382,  1182,   272,
     426,  1832,  1416,  1456,   779,   687,   580, -1001, -1001, -1001,
   -1001, -1001, -1001,   150,   150,   150,   150,   150,   150, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   150,   150,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
     150, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
     150, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001,   150, -1001, -1001, -1001, -1001,
   -1001,   150, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   150,
   -1001, -1001, -1001, -1001, -1001, -1001,   150,   150, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
     150,   150,   150, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001,   150,   150, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001,   182, -1001,   196, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001,   150,   150, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001,   421,    65,  2505,   118,
    2505,   122,  2505,   191,   191,   191,  3170,  3170,   191,   191,
     191,   191,   191,   191,  2505,   191,   191,   191,  2505,   191,
     191,   191,   191,   191,   191,   191,   191,  3170,   191,   374,
     191,   191,   191,   191,   191,   191,  2695,   191,  2695,  2505,
     191,   191,   191,   191,    62,   251,    36,   191,   191,   191,
     191,  2790,  2790,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   150,   150,   191,   191,
     191, -1001,   191, -1001,   191,   191,  2790,  2790,   191,   191,
     191,   191, -1001, -1001, -1001, -1001,    85, -1001,   482, -1001,
   -1001, -1001, -1001,    85, -1001,   495, -1001, -1001, -1001, -1001,
   -1001,    85, -1001,   499, -1001,   507, -1001,   431, -1001,    85,
   -1001,    85, -1001,   515, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,    85,
   -1001,   516, -1001,    85, -1001, -1001, -1001, -1001,    85, -1001,
   -1001, -1001, -1001,    85, -1001, -1001, -1001, -1001, -1001, -1001,
      85, -1001, -1001,    85, -1001, -1001, -1001,    85, -1001, -1001,
   -1001,   191,   191, -1001, -1001,   203, -1001, -1001, -1001,   527,
   -1001,   150,   150,   150, -1001, -1001, -1001, -1001,   150,   150,
   -1001, -1001, -1001, -1001,   150,   150,   150, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
     191,    75,    79,    89,    90, -1001, -1001, -1001, -1001, -1001,
     150, -1001,   150, -1001,   150,   150,   150, -1001, -1001, -1001,
   -1001,   150, -1001,   282,  1186, -1001,   150, -1001, -1001, -1001,
     150,    99,   191,   191,   191,    60, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   316,   150,
    3265,   175, -1001, -1001, -1001, -1001, -1001, -1001,   191,   191,
   -1001,   445,   106,  1638,   150,   205,   100,   191,   191,   191,
     150,   150,   231,  1021, -1001, -1001, -1001, -1001, -1001,   191,
     191,   191,   191,  1810,  1491,   150,   150, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001,   159, -1001, -1001,   314,   145,   562,  3170,  2505,
   -1001,   150,   150,   150,   150,   150,   150, -1001, -1001,   238,
   -1001, -1001, -1001, -1001, -1001, -1001,   191,   326, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
    2505,  2505,   191,   191,   191, -1001, -1001, -1001,    85, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001,   530, -1001,   531, -1001,    92,  2505,    29,  2505,
   -1001,  2505,   163,   191, -1001,   191,   191,   191,   328, -1001,
     191,   191,    85, -1001,    85, -1001,   150,   150, -1001,   191,
     191,   191, -1001, -1001, -1001, -1001, -1001, -1001, -1001,    85,
   -1001, -1001,   536, -1001, -1001,    85, -1001,   257,   360,   541,
   -1001,   546, -1001, -1001, -1001, -1001,    85, -1001, -1001, -1001,
   -1001,   191,   191,   191, -1001,   150, -1001, -1001, -1001, -1001,
    3265, -1001, -1001,   273, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001,   150,   150, -1001,   191,   286,   305,   191, -1001,
     257, -1001,  2885,   191,   191,   150, -1001, -1001, -1001, -1001,
   -1001, -1001,   150, -1001, -1001, -1001, -1001, -1001, -1001,   704,
     292, -1001, -1001, -1001,   219,   781,   856,   251,   191,    73,
     191, -1001,   410, -1001,   281,   150, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001,    85, -1001, -1001, -1001, -1001, -1001,
   -1001,    85, -1001, -1001, -1001, -1001,  3265, -1001,  2505, -1001,
     150, -1001,   150,   410,   410,    85, -1001,   312,   343, -1001,
   -1001,   410,   390,   410, -1001,   410,   349,   377,   410,   410,
     369,   473, -1001,   410, -1001, -1001,   396,  2980,   410, -1001,
   -1001, -1001,  3075, -1001,   404,   410,  3265, -1001
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
     735,   736,   737,   738,   739,   740,   741,   646,   609,     0,
       9,   742,   646,     0,    16,     6,   620,   608,   620,     5,
      12,    19,   646,   743,    26,    11,   622,   621,   646,    26,
      18,     0,    50,     0,     0,     0,    13,    14,    15,     0,
     626,   625,    50,     0,    20,   620,     7,     0,     0,   620,
     620,    65,     0,     0,     0,   646,   635,   623,   654,   657,
     655,   659,   660,   658,   653,   624,   627,   651,   656,    65,
       0,    21,   646,     0,     0,    27,    28,     0,     0,     0,
       0,    51,    52,    53,    54,    77,    63,     0,    24,   606,
       0,    48,   646,     0,    22,   637,   646,    77,     0,    46,
       8,     0,    30,     0,    36,     0,     0,     0,     0,     0,
      76,     0,   620,   619,   617,    25,   620,   620,    49,   607,
      23,     0,   652,     0,   620,   620,   620,    38,    37,     0,
      57,     0,    59,     0,    55,     0,    61,   646,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   620,    79,    80,    81,    82,
      83,    84,   257,   258,   259,   260,   261,   262,   263,   264,
      85,    86,    87,    88,     0,    66,   615,     0,   636,   639,
     646,   628,   643,   646,     0,    31,    40,    58,    60,    56,
      62,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      73,    75,     0,    64,   618,   620,   638,     0,   630,    17,
       0,     0,     0,     0,   367,     0,   416,   417,     0,   568,
     646,     0,   333,     0,   265,     0,    89,   473,     0,   466,
     646,     0,   114,     0,   244,     0,   127,     0,   284,     0,
     297,     0,   313,     0,   453,     0,   431,     0,   139,   614,
       0,   382,   646,   646,     0,   369,   646,   620,    68,    67,
     616,   646,   641,   640,   646,   629,   646,    47,     0,    29,
      35,    32,    33,    34,    39,    43,    41,    42,   620,   371,
     368,   611,   566,   610,   620,   567,   415,   620,   335,   334,
     620,   267,   266,   620,    91,    90,   620,   472,   620,   116,
     115,   620,   246,   245,   620,   129,   128,   620,   620,   620,
     620,   455,   454,   620,   433,   432,   620,   620,   384,   383,
     612,   613,   370,    74,    70,   637,   633,   631,     0,   373,
     419,   569,   337,   269,    93,   468,   118,   248,   131,   285,
     299,   315,   457,   435,   141,   386,     0,     0,   632,     0,
      44,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,    71,    72,
     642,   646,    45,     0,     0,     0,     0,     0,     0,   372,
     380,   381,   379,   620,   620,   377,   378,   620,     0,     0,
     418,   423,   424,   422,   620,   620,   620,   620,   620,   620,
       0,   336,   344,   345,   343,   620,   340,   349,   350,   351,
     352,   355,   620,   347,   348,   353,   354,   620,   341,   342,
       0,   268,   276,   277,   275,   620,   620,   620,   620,   620,
     620,   620,   274,   273,   620,     0,    92,    96,    97,   620,
      95,     0,   467,   469,   470,   117,   121,   122,   120,   620,
     247,   250,   251,   249,   620,   620,   620,   620,   620,     0,
     130,   135,   136,   134,   620,   132,     0,     0,   283,   295,
     296,   294,   620,   620,   289,   291,   620,   292,   293,   620,
       0,     0,     0,   298,   311,   312,   310,   620,   620,   304,
     303,   620,   306,   307,   308,   309,   620,     0,     0,   314,
     326,   327,   325,   620,   620,   620,   620,   620,   620,   620,
     321,   322,   323,   324,   620,   320,   319,   456,   461,   462,
     460,   620,   620,   620,   620,   620,   646,   646,   434,   438,
     439,   437,   620,   620,   620,     0,   620,     0,   620,   140,
     146,   147,   145,   620,   143,   144,     0,     0,   385,   390,
     391,   389,   620,   620,   620,   620,     0,     0,     0,     0,
       0,     0,     0,   375,   374,   376,     0,     0,   421,   425,
     428,   426,   427,   420,     0,   339,   346,   338,     0,   271,
     281,   278,   282,   279,   280,   270,   272,     0,    94,     0,
     119,   253,   252,   254,   255,   256,     0,   133,     0,     0,
     287,   288,   286,   290,     0,     0,     0,   301,   302,   300,
     305,     0,     0,   317,   328,   329,   332,   330,   331,   316,
     318,   459,   463,   464,   465,   458,   444,   451,   436,   440,
     441,   620,   442,   620,   443,   142,     0,     0,   388,   392,
     393,   387,   646,   646,   646,   528,     0,   524,     0,   123,
     646,   646,   533,     0,   529,     0,   550,   646,   646,   646,
     113,     0,   108,     0,   517,     0,   356,     0,   429,     0,
     242,     0,   534,     0,    98,   483,   496,   476,   505,   620,
     646,   474,   475,   646,   480,   646,   482,   646,   481,     0,
     137,     0,   149,     0,   240,   599,   646,   544,     0,   540,
     646,   600,   539,     0,   536,   601,   646,   646,   646,   549,
       0,   545,   564,     0,   560,   562,   557,     0,   554,   558,
     570,   446,   446,   412,   413,     0,   646,   395,   396,     0,
     646,   634,   527,   526,   525,   620,   125,   124,   532,   531,
     530,   620,   552,   551,   110,   112,   111,   109,   620,   519,
     518,   620,   358,   357,   430,   243,   535,   620,   100,    99,
     471,     0,     0,     0,     0,   138,   620,   150,   148,   241,
     542,   541,   543,   537,   538,   548,   547,   546,   561,   646,
     555,   556,   568,     0,     0,   620,   411,   620,   398,   397,
     394,     0,   174,   521,   360,   103,   620,   478,   477,   620,
     485,   484,   620,   498,   497,   620,   507,   506,   155,   563,
       0,   571,   445,   620,   620,   620,   620,   452,   419,   400,
     126,     0,     0,     0,     0,     0,     0,   487,   500,   509,
       0,     0,     0,   152,   153,   620,   154,   220,   559,   448,
     449,   450,   447,     0,     0,     0,     0,   553,   177,   178,
     175,   176,   520,   522,   523,   359,   364,   365,   363,   620,
     620,   620,     0,   101,   479,     0,     0,     0,     0,     0,
     151,     0,     0,     0,     0,     0,     0,   158,   620,     0,
     161,   620,   620,   620,   157,   156,   193,   219,   414,   399,
     409,   410,   620,   404,   405,   406,   403,   407,   408,   620,
       0,     0,   362,   366,   361,   646,   646,   107,     0,   102,
     486,   488,   491,   492,   493,   494,   495,   620,   490,   499,
     501,   504,   620,   503,   508,   620,   511,   512,   513,   514,
     515,   516,     0,   223,     0,   196,     0,     0,   572,     0,
     179,     0,     0,   163,   164,   159,   160,   162,   192,   221,
     402,   401,     0,   238,     0,   236,   106,   105,   104,   489,
     502,   510,   620,   224,   620,   620,   197,   195,   169,     0,
     166,   646,     0,   171,   576,     0,   212,     0,     0,     0,
     181,     0,   565,   646,   646,   218,     0,   214,   620,   239,
     237,   226,   222,   199,   167,   168,   620,   172,   170,   213,
       0,   579,   573,     0,   575,   583,   620,   182,   180,   620,
     210,   209,   217,   216,   215,   194,     0,     0,   174,   588,
     578,   582,     0,   184,   174,     0,   225,   230,   231,   229,
     620,   228,     0,   198,   203,   204,   202,   620,   201,     0,
     577,   580,   584,   581,   586,     0,     0,     0,   227,     0,
     200,   173,   649,   585,     0,     0,   183,   188,   189,   185,
     186,   187,   211,   235,     0,   232,   646,   604,   605,   602,
     208,     0,   205,   646,   603,   650,     0,   587,     0,   233,
     234,   206,   207,   649,   649,     0,   190,     0,     0,   191,
     589,   649,     0,   649,   590,   649,     0,     0,   649,   649,
       0,     0,   598,   649,   591,   594,     0,     0,   649,   595,
     596,   593,   649,   592,     0,   649,     0,   597
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1001, -1001, -1001,   609, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001,   368, -1001, -1001,   373, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001,   186, -1001, -1001,
   -1001,  -239,   383, -1001, -1001, -1001, -1001, -1001,   -19, -1001,
     216,   350, -1001, -1001, -1001, -1001, -1001,    86, -1001,   332,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001,  -406, -1001, -1001, -1001, -1001, -1001, -1001,
    -315, -1001, -1001, -1001, -1001, -1001, -1001,  -407, -1001,  -174,
   -1001,  -454, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1000, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001,  -502, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
    -434, -1001, -1001, -1001, -1001, -1001, -1001, -1001,  -589, -1001,
    -588, -1001,  -415, -1001,  -397, -1001,  -167, -1001, -1001,  -180,
   -1001,   119, -1001, -1001,   127, -1001, -1001, -1001,   131, -1001,
   -1001,   137, -1001, -1001,   146, -1001, -1001, -1001, -1001, -1001,
     153, -1001, -1001, -1001,   170, -1001,   171,   221, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001,  -280, -1001,   -81, -1001, -1001,  -122,
   -1001, -1001, -1001,  -172, -1001, -1001, -1001,  -192, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001,
   -1001, -1001,  -292, -1001, -1001, -1001,  -422, -1001, -1001,  -444,
   -1001, -1001,  -289, -1001,  -426, -1001, -1001,  -379, -1001, -1001,
     198, -1001, -1001,  -433, -1001, -1001,  -535, -1001, -1001, -1001,
   -1001, -1001, -1001, -1001,  -382,   370,  -224,  -695, -1001, -1001,
   -1001, -1001,  -457,  -490, -1001, -1001,  -439, -1001, -1001, -1001,
    -479, -1001, -1001, -1001,  -522, -1001, -1001, -1001,  -702,  -441,
   -1001, -1001, -1001,   112,  -300,   -42,   384, -1001, -1001, -1001,
    1195,  -199, -1001, -1001, -1001, -1001, -1001, -1001, -1001,   283,
   -1001, -1001, -1001,    -4,   -10,   458,   441,  -120, -1001,   405,
      47,  -137,  -284
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   109,   162,    99,     2,   114,   115,
     103,     3,   119,   120,   183,   126,   177,   127,   122,   139,
     191,   311,   193,   140,   218,   312,   449,   370,   188,   135,
     180,   128,   141,   223,   171,   219,   172,   221,   948,   225,
     949,   175,   302,   176,   202,   359,   446,   303,   356,   199,
     200,   245,   325,   246,   395,   455,   773,   539,   859,  1008,
     925,  1009,   761,   493,   762,   331,   247,   400,   457,   748,
     494,   837,   335,   248,   406,   459,   789,   565,   347,   913,
     465,   573,   791,   868,   932,   933,   978,   979,  1069,   980,
    1070,   981,  1072,  1098,   921,  1039,   982,  1079,  1108,  1145,
    1185,  1159,   934,  1048,   935,  1034,  1067,  1117,  1171,  1138,
    1172,   983,  1111,  1075,   984,  1086,   985,  1087,   936,   987,
     937,  1032,  1064,  1116,  1164,  1131,  1165,  1054,   950,  1052,
     951,   793,   574,   769,   506,   333,   914,   403,   458,   495,
     323,   252,   392,   454,   253,   337,   460,   339,   254,   461,
     341,   255,   462,   321,   256,   389,   453,   512,   513,   765,
     496,   853,   923,   313,   257,   354,   258,   380,   451,   350,
     259,   419,   466,   828,   829,   653,   889,   944,   824,   825,
     654,   317,   318,   260,   452,   767,   497,   345,   261,   415,
     464,   635,   636,   883,   637,   638,   343,   498,   412,   463,
     328,   263,   456,   329,   779,   780,   781,   898,   782,   783,
     784,   901,   965,   785,   786,   904,   966,   787,   788,   907,
     967,   763,   499,   850,   922,   746,   485,   747,   753,   486,
     754,   771,   533,   803,   593,   804,   798,   594,   799,   810,
     595,   811,   755,   916,   843,   817,   615,   818,   881,   813,
     616,   814,   879,  1081,   319,   320,   385,   819,   882,  1076,
    1077,  1078,  1101,  1102,  1120,  1104,  1105,  1122,  1143,  1153,
    1140,  1183,  1194,  1204,  1205,  1207,  1212,  1195,   805,   806,
    1173,  1174,   181,   100,   820,   351,   205,   206,   305,   207,
     110,   116,   129,   130,   155,   307,   365,   448,   147,   211,
     270,   363,   271,   117,     4,    12,  1176,   156,   186,   157,
     101,   102,   118
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,    14,   800,   431,    11,    11,   182,   588,   262,   519,
      10,   643,   269,   353,    16,   382,   578,     6,   487,     7,
     251,   534,   178,   113,   184,   168,   249,   579,   596,   614,
     625,   518,   532,   250,   133,   310,   611,    16,   577,   592,
     610,   826,   830,  -574,    16,   482,   589,   504,   524,   540,
     644,   548,   553,   563,   571,   586,   602,   620,   631,   642,
     651,   170,    16,   575,   590,     6,   300,     7,   645,     5,
      16,   113,   371,    16,   106,   220,   222,   224,   226,   795,
       6,    16,     7,   612,     6,   108,     7,   105,   112,  1167,
    1168,  1169,   107,   125,     6,     6,     7,     7,   467,   227,
      16,    10,   121,    10,     6,     6,     7,     7,   131,   795,
     113,   113,   315,   201,   168,   123,   304,    10,  1139,    11,
      11,    11,   173,   124,  1146,   104,    16,    10,   807,    11,
      16,   808,   273,    11,    11,   185,   483,   168,   505,   525,
     173,   125,   549,   924,   564,   572,   587,   603,   621,   632,
     170,   652,   190,   743,   368,     6,   796,     7,   744,   484,
     352,   517,   531,    11,    11,    11,    11,    16,   576,   591,
     609,    16,   209,   170,   655,   500,   212,   158,   896,   214,
     897,    10,   899,   145,   900,   146,    10,   374,   203,   158,
     204,   158,   902,   905,   903,   906,     6,    11,     7,   216,
     476,   217,   113,   920,   964,    10,   750,   757,    10,   758,
     952,   751,   759,   215,     6,   938,     7,   281,   367,   357,
     113,   358,   598,   567,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,   384,   158,   158,   158,   158,   396,  1005,   269,  1019,
     407,  1083,  1006,   471,   535,     6,  1084,     7,   272,    16,
     306,   408,   530,   309,   409,   168,   558,    11,   795,   801,
     608,   416,   189,   124,   528,   192,   194,    10,   556,   168,
     526,   315,   606,   624,   554,   731,   228,   527,   604,   622,
     633,   555,   372,   376,   559,   605,   623,   634,   361,   733,
     362,   170,    10,   168,     8,    10,   885,   477,   230,   963,
     386,   231,   124,   566,   474,   170,   378,   168,   379,   368,
     397,     6,   235,     7,     9,  -588,  -588,   113,   474,   237,
     238,   387,   239,   388,  1071,   970,   168,   476,   536,   170,
     930,   473,   420,   421,   500,   477,   423,   474,   931,  1125,
     930,   425,   369,   170,   426,   158,   427,   174,   242,   477,
     931,   243,   383,  1100,    11,   475,   580,   581,     6,   476,
       7,   136,   170,   529,  1103,   174,   560,   557,   477,  1121,
     244,   607,    10,   137,   138,  1132,   912,  1100,  1152,   168,
    1126,   598,   567,    10,   314,   541,   322,   324,   326,  1152,
     332,   334,   336,   338,   340,   342,   344,   346,   348,  1133,
     355,   815,   353,   168,   353,  1175,    10,    10,  1010,    10,
    1190,    10,    10,    10,   474,   170,     6,   468,     7,   742,
    1018,  1023,   480,   491,   502,   522,   537,   543,   546,   551,
     561,   569,   584,   600,   618,   629,   640,   649,   390,   170,
     391,   167,  1191,   473,   168,   477,   500,   168,   775,  1198,
     776,   656,   542,   777,   393,   778,   394,    13,   474,    11,
      11,    11,    11,    11,    11,  1193,   168,   475,   945,   946,
    1202,   476,   169,  1199,    11,    11,   545,  1203,   134,   398,
     170,   399,   132,   170,   272,   401,    11,   402,   375,   477,
     995,   566,  1208,   404,   567,   405,   410,   478,   411,   187,
    1215,   999,   170,  1025,   450,   159,    11,   958,   997,   213,
     473,  1014,   994,  1029,   168,  1044,   977,   373,   377,   431,
     568,    11,  1017,  1022,   413,   474,   414,    11,   749,  1015,
     756,  1030,   764,  1013,   475,  1028,  1088,   993,   476,   947,
    1011,  1020,  1026,  1049,   770,    11,  1160,  1161,   772,   352,
     170,   352,    11,    11,   208,   998,   477,   210,  1012,  1021,
    1027,   417,   507,   418,   478,   422,    11,    11,    11,   794,
     508,   142,   143,   144,   509,   835,  1016,   836,  1031,   473,
     510,   160,   500,    11,    11,   163,   164,   479,   841,   511,
     842,   646,   848,   277,   849,   278,   514,   279,   959,   280,
     851,   168,   852,   475,   580,   581,   726,   727,   857,   866,
     858,   867,   474,   515,   516,   195,   196,   197,   198,   992,
     887,   961,   888,  1062,  1065,  1063,  1066,   566,   943,  1096,
     567,  1097,    11,    11,  1106,   854,  1107,   170,   647,  1109,
     884,  1110,    10,   477,   790,   996,   792,   330,   911,   264,
     613,   478,   469,  1141,  1177,  1154,  1024,   481,   492,   503,
     523,   538,   544,   547,   552,   562,   570,   585,   601,   619,
     630,   641,   650,  1142,   648,  1209,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   915,   915,   158,  1166,   158,   447,   158,
    1129,  1136,     0,   366,     0,     0,     0,   500,   168,     0,
       0,   158,    10,    10,     0,   158,     0,     0,     0,   308,
       0,     0,   831,   832,   833,   168,     0,   945,   946,   154,
     838,   839,     0,   960,     0,     0,   158,   844,   845,   846,
       0,   154,   179,   154,   170,     0,     0,     0,   383,   383,
     477,     0,   566,     0,     0,   567,     0,     0,     0,     0,
     861,   170,     0,   862,     0,   863,     0,   864,   766,   768,
       0,     0,     0,   383,   383,     0,   870,     0,     0,     0,
     872,   639,     0,     0,     0,     0,   874,   875,   876,   774,
    1119,  1130,  1137,     0,   154,   154,   154,   154,  1151,     0,
     168,     0,   168,     0,   945,   946,   886,     0,     0,   235,
     890,   474,  1144,     0,   626,     0,   428,    10,    10,    10,
       0,     0,     0,   472,    10,    10,     0,     0,     0,   627,
      10,    10,    10,     0,     0,     0,   170,     0,   170,  1035,
       0,     0,   477,     0,     0,   242,     0,    10,    10,    10,
      10,     0,     0,     0,     0,     0,    10,     0,    10,   909,
      10,    10,    10,   301,     0,     0,  1184,    11,  1155,     0,
    1053,  1055,    10,   628,     0,  1156,    10,   168,     0,   945,
     946,     0,   316,     0,     0,     0,   327,     0,     0,     0,
       0,     0,     0,   953,   956,    10,   349,  1210,     0,     0,
       0,     0,  1213,     0,     0,     0,  1217,  1073,     0,  1080,
      11,  1082,     0,   170,   491,   990,    11,    11,     0,     0,
       0,   657,   658,   659,   660,   661,   662,     0,     0,     0,
       0,    11,    11,     0,     0,     0,   666,   667,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   383,   674,     0,
    1162,     0,     0,     0,     0,   228,     0,    11,    11,    11,
      11,    11,    11,     0,     0,     0,     0,     0,   678,     0,
       0,     0,     0,     0,     0,     0,   488,   230,   473,     0,
     231,   500,   168,   687,     0,  1056,  1057,     0,     0,   689,
       0,     0,     0,   474,     0,     0,     0,     0,   237,   238,
       0,   239,   475,     0,     0,     0,   158,   696,     0,     0,
       0,     0,     0,     0,   698,   699,     0,     0,   170,     0,
       0,     0,     0,     0,   477,     0,     0,   301,   704,   705,
     706,     0,   478,   559,     0,     0,     0,   158,   158,     0,
       0,     0,    10,    10,     0,   711,   712,     0,  1186,   244,
     971,  1095,     0,     0,     0,   501,     0,     0,     0,     0,
     972,     0,     0,  1112,  1113,     0,     0,     0,     0,     0,
    1033,     0,   973,   974,   158,     0,   158,   975,   158,   228,
     976,    10,     0,     0,     0,     0,  -165,  1127,  1134,     0,
       0,     0,     0,     0,   736,   737,     0,     0,    10,    10,
       0,   230,   473,     0,   231,     0,   168,     0,     0,     0,
       0,    11,     0,     0,     0,   235,  1157,   474,    11,     0,
     834,     0,   237,   238,     0,   239,     0,   840,   954,   957,
     476,     0,   240,     0,     0,   847,     0,   383,     0,     0,
     520,    11,   170,   855,     0,   856,  1180,     0,   477,   492,
     991,   242,     0,  1182,   243,     0,   478,     0,     0,   383,
       0,     0,     0,   865,     0,     0,    10,   869,    10,     0,
       0,   489,   871,   244,     0,     0,   228,   873,     0,   521,
     228,     0,     0,     0,   877,     0,     0,   878,     0,     0,
       0,   880,     0,     0,     0,     0,     0,     0,   230,     0,
       0,   231,   230,   168,     0,   231,     0,     0,     0,     0,
       0,     0,   235,   383,     0,   158,   235,     0,     0,   237,
     238,     0,   239,   237,   238,     0,   239,     0,     0,   240,
       0,   476,     0,     0,     0,     0,     0,     0,     0,   170,
       0,     0,     0,     0,   383,   477,     0,     0,   242,   383,
       0,   243,   242,   383,     0,   243,   745,   154,   752,   154,
     760,   154,     0,   228,     0,     0,     0,     0,   489,     0,
     244,     0,     0,   154,   244,     0,   550,   154,     0,     0,
     917,     0,     0,     0,   488,   230,     0,     0,   231,     0,
     168,     0,     0,   111,     0,   349,     0,   349,   154,     0,
       0,   474,     0,   797,   802,   809,   237,   238,     0,   239,
     812,   816,     0,     0,     0,     0,   240,     0,     0,     0,
     161,     0,  1128,  1135,   165,   166,   170,     0,     0,   910,
       0,     0,   477,     0,     0,   823,   827,   228,   243,     0,
     478,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1158,     0,     0,   229,   489,     0,   244,     0,   230,
       0,     0,   231,   490,     0,     0,     0,     0,   232,   233,
       0,   234,   962,   235,   236,     0,     0,     0,   968,   969,
     237,   238,  1058,   239,     0,     0,     0,   265,     0,     0,
     240,   266,   267,  1000,  1001,     0,     0,     0,     0,   274,
     275,   276,     0,     0,     0,   241,     0,     0,     0,   242,
     228,     0,   243,     0,     0,     0,     0,     0,     0,  1036,
    1037,  1038,  1040,  1041,  1042,     0,  1089,     0,  1090,     0,
     299,   244,   230,   473,     0,   231,     0,   168,     0,     0,
       0,     0,     0,  1094,     0,     0,   235,     0,   474,  1099,
     228,     0,   597,   237,   238,     0,   239,     0,   580,   581,
    1114,   476,     0,   240,   582,     0,     0,     0,     0,     0,
       0,     0,   230,   170,     0,   231,     0,   168,     0,   477,
       0,     0,   242,   598,     0,   243,   235,   478,   474,     0,
     360,     0,     0,   237,   238,     0,   239,     0,     0,     0,
       0,   476,   489,     0,   244,     0,     0,     0,   473,     0,
     599,   500,   168,   170,     0,     0,     0,     0,     0,   477,
       0,     0,   242,   474,     0,   243,     0,     0,     0,     0,
       0,     0,   475,   580,   581,     0,   476,     0,  1179,     0,
       0,     0,   424,     0,   244,  1181,   520,     0,   170,     0,
     617,     0,     0,     0,     0,     0,     0,     0,     0,  1189,
       0,  1007,     0,   429,     0,     0,     0,     0,   154,   430,
       0,     0,   432,  1147,     0,   433,     0,     0,   434,     0,
    1149,   435,     0,   436,     0,   989,   437,     0,     0,   438,
       0,     0,   439,   440,   441,   442,     0,     0,   443,   154,
     154,   444,   445,  1178,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1187,  1188,     0,     0,     0,     0,
       0,     0,  1192,     0,  1196,     0,  1197,     0,     0,  1200,
    1201,     0,   228,     0,  1206,  1068,   154,  1074,   154,  1211,
     154,  1085,     0,  1214,     0,     0,  1216,     0,     0,     0,
       0,     0,     0,     0,   230,     0,     0,   231,     0,   168,
       0,     0,     0,     0,     0,     0,     0,     0,   663,   664,
     474,     0,   665,     0,     0,   237,   238,     0,   239,   668,
     669,   670,   671,   672,   673,     0,     0,     0,     0,     0,
     675,     0,     0,     0,     0,   170,     0,   676,     0,     0,
       0,   477,   677,     0,     0,     0,     0,   243,     0,   478,
     679,   680,   681,   682,   683,   684,   685,     0,     0,   686,
       0,     0,     0,     0,   688,     0,   244,     0,     0,     0,
       0,     0,   955,     0,   690,     0,     0,     0,     0,   691,
     692,   693,   694,   695,     0,     0,  1163,     0,  1170,   697,
       0,     0,     0,     0,     0,     0,     0,   700,   701,     0,
       0,   702,     0,     0,   703,     0,     0,     0,     0,     0,
       0,     0,   707,   708,     0,     0,   709,   154,     0,     0,
       0,   710,     0,     0,     0,     0,     0,     0,   713,   714,
     715,   716,   717,   718,   719,     0,     0,     0,     0,   720,
       0,     0,     0,     0,   228,     0,   721,   722,   723,   724,
     725,     0,     0,     0,     0,     0,     0,   728,   729,   730,
       0,   732,     0,   734,     0,   488,   230,     0,   735,   231,
       0,   168,     0,     0,     0,     0,     0,   738,   739,   740,
     741,     0,   474,     0,     0,     0,     0,   237,   238,   473,
     239,     0,   500,   168,     0,     0,     0,   240,     0,     0,
       0,     0,     0,     0,   474,     0,     0,   170,     0,     0,
       0,     0,     0,   477,   580,   581,     0,   476,     0,   243,
     582,   478,     0,     0,     0,     0,     0,     0,     0,   170,
       0,     0,     0,     0,     0,   477,   489,   566,   244,     0,
     567,     0,     0,   478,   988,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   821,     0,   822,     0,
       0,     0,     0,     0,     0,     0,   583,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   860,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     891,     0,     0,     0,     0,     0,   892,     0,     0,     0,
       0,     0,     0,   893,     0,     0,   894,     0,     0,     0,
       0,     0,   895,     0,     0,     0,     0,     0,     0,     0,
       0,   908,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     918,     0,   919,     0,     0,     0,     0,     0,     0,     0,
       0,   926,     0,     0,   927,     0,     0,   928,     0,     0,
     929,     0,     0,     0,     0,     0,     0,     0,   939,   940,
     941,   942,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     986,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1002,  1003,  1004,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1043,     0,     0,  1045,  1046,  1047,     0,
       0,     0,     0,     0,     0,     0,     0,  1050,     0,     0,
       0,     0,     0,     0,  1051,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1059,     0,     0,     0,     0,  1060,     0,     0,
    1061,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1091,     0,  1092,
    1093,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1115,     0,     0,     0,     0,     0,     0,
       0,  1118,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1123,     0,     0,  1124,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,     0,     0,
       0,     0,     0,    17,   113,  1148,     0,     0,     0,     0,
       0,     0,  1150,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    15,     0,     0,     0,     0,   268,    17,   113,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,     0,     0,    15,
       0,     0,   470,    16,   148,    17,   149,   150,     0,     0,
       0,   151,   152,   153,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,   364,   148,
      17,   149,   150,     0,     0,     0,   151,   152,   153,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,    16,     0,    17,   113,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,     0,
      17,   381,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,     0,     0,    17,   381,     0,     0,  1103,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,     0,     0,
      17,   381,     0,     0,  1203,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
    1175,     0,     0,     0,     0,    17,   381,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,     0,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,     0,     0,    17,   381,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96
};

static const yytype_int16 yycheck[] =
{
       4,    11,   704,   385,     8,     9,   143,   461,   200,   453,
      14,   465,   211,   297,     8,   315,   460,     5,   451,     7,
     200,   454,   142,    11,   144,    31,   200,   460,   461,   462,
     463,   453,   454,   200,    23,   274,   462,     8,   460,   461,
     462,   736,   737,    14,     8,   451,   461,   453,   454,   455,
     465,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,    67,     8,   460,   461,     5,    12,     7,   465,     0,
       8,    11,   311,     8,   103,   195,   196,   197,   198,    17,
       5,     8,     7,   462,     5,   103,     7,    97,   102,    16,
      17,    18,   102,    82,     5,     5,     7,     7,   104,   104,
       8,   105,   112,   107,     5,     5,     7,     7,   118,    17,
      11,    11,   106,    70,    31,    56,   104,   121,  1118,   123,
     124,   125,   141,    64,  1124,    13,     8,   131,    92,   133,
       8,    95,   104,   137,   138,   145,   451,    31,   453,   454,
     159,    82,   457,    83,   459,   460,   461,   462,   463,   464,
      67,   466,   162,    88,    71,     5,    94,     7,    93,   451,
     297,   453,   454,   167,   168,   169,   170,     8,   460,   461,
     462,     8,   182,    67,   466,    30,   186,   130,   103,   103,
     105,   185,   103,   103,   105,   105,   190,   104,   103,   142,
     105,   144,   103,   103,   105,   105,     5,   201,     7,   103,
      55,   105,    11,   104,   104,   209,    88,    85,   212,    87,
     104,    93,    90,   103,     5,   910,     7,   227,   104,   103,
      11,   105,    77,    78,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   103,   195,   196,   197,   198,   103,    88,   447,   104,
     103,    88,    93,   102,    20,     5,    93,     7,   211,     8,
     270,   103,   454,   273,   103,    31,   458,   271,    17,    18,
     462,   103,   160,    64,   454,   163,   164,   281,   458,    31,
     454,   106,   462,   463,   458,   103,     4,   454,   462,   463,
     464,   458,   311,   312,    22,   462,   463,   464,   103,   103,
     105,    67,   306,    31,    54,   309,   103,    73,    26,   104,
     320,    29,    64,    75,    42,    67,   103,    31,   105,    71,
     330,     5,    40,     7,    74,   106,   107,    11,    42,    47,
      48,   103,    50,   105,  1036,   104,    31,    55,   104,    67,
      24,    27,   352,   353,    30,    73,   356,    42,    32,    63,
      24,   361,   104,    67,   364,   308,   366,   141,    76,    73,
      32,    79,   315,   106,   368,    51,    52,    53,     5,    55,
       7,     8,    67,   454,    14,   159,   104,   458,    73,   106,
      98,   462,   386,    43,    44,    80,   104,   106,   107,    31,
     104,    77,    78,   397,   282,    37,   284,   285,   286,   107,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   104,
     298,   711,   696,    31,   698,     5,   420,   421,   104,   423,
     108,   425,   426,   427,    42,    67,     5,   446,     7,     8,
     965,   966,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   103,    67,
     105,    28,   109,    27,    31,    73,    30,    31,    84,   110,
      86,   471,   104,    89,   103,    91,   105,     9,    42,   473,
     474,   475,   476,   477,   478,    85,    31,    51,    33,    34,
     111,    55,    59,   106,   488,   489,   104,    14,   120,   103,
      67,   105,   119,    67,   447,   103,   500,   105,   312,    73,
     944,    75,   106,   103,    78,   105,   103,    81,   105,   159,
     106,   944,    67,   967,   428,   132,   520,   923,   944,   187,
      27,   965,   944,   967,    31,   979,   933,   311,   312,   911,
     104,   535,   965,   966,   103,    42,   105,   541,   658,   965,
     660,   967,   662,   965,    51,   967,  1048,   944,    55,   104,
     965,   966,   967,   987,   674,   559,  1145,  1145,   678,   696,
      67,   698,   566,   567,   180,   944,    73,   183,   965,   966,
     967,   103,   453,   105,    81,   354,   580,   581,   582,   699,
     453,   123,   124,   125,   453,   103,   965,   105,   967,    27,
     453,   133,    30,   597,   598,   137,   138,   104,   103,   453,
     105,    21,   103,   219,   105,   221,   453,   223,   923,   225,
     103,    31,   105,    51,    52,    53,   626,   627,   103,   103,
     105,   105,    42,   453,   453,   167,   168,   169,   170,   944,
     103,   923,   105,   103,   103,   105,   105,    75,   918,   103,
      78,   105,   646,   647,   103,   767,   105,    67,    68,   103,
     822,   105,   656,    73,   696,   944,   698,   287,   882,   201,
     462,    81,   446,  1120,  1154,  1144,   104,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,  1122,   104,  1207,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   883,   884,   658,  1147,   660,   425,   662,
    1116,  1117,    -1,   308,    -1,    -1,    -1,    30,    31,    -1,
      -1,   674,   726,   727,    -1,   678,    -1,    -1,    -1,   271,
      -1,    -1,   742,   743,   744,    31,    -1,    33,    34,   130,
     750,   751,    -1,   923,    -1,    -1,   699,   757,   758,   759,
      -1,   142,   143,   144,    67,    -1,    -1,    -1,   711,   712,
      73,    -1,    75,    -1,    -1,    78,    -1,    -1,    -1,    -1,
     780,    67,    -1,   783,    -1,   785,    -1,   787,   666,   667,
      -1,    -1,    -1,   736,   737,    -1,   796,    -1,    -1,    -1,
     800,   104,    -1,    -1,    -1,    -1,   806,   807,   808,   687,
    1100,  1116,  1117,    -1,   195,   196,   197,   198,   104,    -1,
      31,    -1,    31,    -1,    33,    34,   826,    -1,    -1,    40,
     830,    42,  1122,    -1,    45,    -1,   368,   831,   832,   833,
      -1,    -1,    -1,   449,   838,   839,    -1,    -1,    -1,    60,
     844,   845,   846,    -1,    -1,    -1,    67,    -1,    67,   969,
      -1,    -1,    73,    -1,    -1,    76,    -1,   861,   862,   863,
     864,    -1,    -1,    -1,    -1,    -1,   870,    -1,   872,   879,
     874,   875,   876,   264,    -1,    -1,  1176,   881,    97,    -1,
    1000,  1001,   886,   104,    -1,   104,   890,    31,    -1,    33,
      34,    -1,   283,    -1,    -1,    -1,   287,    -1,    -1,    -1,
      -1,    -1,    -1,   922,   923,   909,   297,  1207,    -1,    -1,
      -1,    -1,  1212,    -1,    -1,    -1,  1216,  1037,    -1,  1039,
     924,  1041,    -1,    67,   943,   944,   930,   931,    -1,    -1,
      -1,   473,   474,   475,   476,   477,   478,    -1,    -1,    -1,
      -1,   945,   946,    -1,    -1,    -1,   488,   489,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   910,   500,    -1,
     104,    -1,    -1,    -1,    -1,     4,    -1,   971,   972,   973,
     974,   975,   976,    -1,    -1,    -1,    -1,    -1,   520,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    -1,
      29,    30,    31,   535,    -1,  1005,  1006,    -1,    -1,   541,
      -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,
      -1,    50,    51,    -1,    -1,    -1,   969,   559,    -1,    -1,
      -1,    -1,    -1,    -1,   566,   567,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,   428,   580,   581,
     582,    -1,    81,    22,    -1,    -1,    -1,  1000,  1001,    -1,
      -1,    -1,  1056,  1057,    -1,   597,   598,    -1,  1178,    98,
      39,  1071,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      49,    -1,    -1,  1083,  1084,    -1,    -1,    -1,    -1,    -1,
     968,    -1,    61,    62,  1037,    -1,  1039,    66,  1041,     4,
      69,  1095,    -1,    -1,    -1,    -1,    75,  1116,  1117,    -1,
      -1,    -1,    -1,    -1,   646,   647,    -1,    -1,  1112,  1113,
      -1,    26,    27,    -1,    29,    -1,    31,    -1,    -1,    -1,
      -1,  1125,    -1,    -1,    -1,    40,  1145,    42,  1132,    -1,
     746,    -1,    47,    48,    -1,    50,    -1,   753,   922,   923,
      55,    -1,    57,    -1,    -1,   761,    -1,  1100,    -1,    -1,
      65,  1155,    67,   769,    -1,   771,  1166,    -1,    73,   943,
     944,    76,    -1,  1173,    79,    -1,    81,    -1,    -1,  1122,
      -1,    -1,    -1,   789,    -1,    -1,  1180,   793,  1182,    -1,
      -1,    96,   798,    98,    -1,    -1,     4,   803,    -1,   104,
       4,    -1,    -1,    -1,   810,    -1,    -1,   813,    -1,    -1,
      -1,   817,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    29,    26,    31,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    40,  1176,    -1,  1178,    40,    -1,    -1,    47,
      48,    -1,    50,    47,    48,    -1,    50,    -1,    -1,    57,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,  1207,    73,    -1,    -1,    76,  1212,
      -1,    79,    76,  1216,    -1,    79,   657,   658,   659,   660,
     661,   662,    -1,     4,    -1,    -1,    -1,    -1,    96,    -1,
      98,    -1,    -1,   674,    98,    -1,   104,   678,    -1,    -1,
     104,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    -1,
      31,    -1,    -1,   108,    -1,   696,    -1,   698,   699,    -1,
      -1,    42,    -1,   704,   705,   706,    47,    48,    -1,    50,
     711,   712,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
     135,    -1,  1116,  1117,   139,   140,    67,    -1,    -1,   881,
      -1,    -1,    73,    -1,    -1,   736,   737,     4,    79,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1145,    -1,    -1,    21,    96,    -1,    98,    -1,    26,
      -1,    -1,    29,   104,    -1,    -1,    -1,    -1,    35,    36,
      -1,    38,   924,    40,    41,    -1,    -1,    -1,   930,   931,
      47,    48,  1008,    50,    -1,    -1,    -1,   202,    -1,    -1,
      57,   206,   207,   945,   946,    -1,    -1,    -1,    -1,   214,
     215,   216,    -1,    -1,    -1,    72,    -1,    -1,    -1,    76,
       4,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,   971,
     972,   973,   974,   975,   976,    -1,  1052,    -1,  1054,    -1,
     245,    98,    26,    27,    -1,    29,    -1,    31,    -1,    -1,
      -1,    -1,    -1,  1069,    -1,    -1,    40,    -1,    42,  1075,
       4,    -1,    46,    47,    48,    -1,    50,    -1,    52,    53,
    1086,    55,    -1,    57,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    67,    -1,    29,    -1,    31,    -1,    73,
      -1,    -1,    76,    77,    -1,    79,    40,    81,    42,    -1,
     305,    -1,    -1,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    55,    96,    -1,    98,    -1,    -1,    -1,    27,    -1,
     104,    30,    31,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    76,    42,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    53,    -1,    55,    -1,  1164,    -1,
      -1,    -1,   357,    -1,    98,  1171,    65,    -1,    67,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1185,
      -1,   962,    -1,   378,    -1,    -1,    -1,    -1,   969,   384,
      -1,    -1,   387,  1125,    -1,   390,    -1,    -1,   393,    -1,
    1132,   396,    -1,   398,    -1,   104,   401,    -1,    -1,   404,
      -1,    -1,   407,   408,   409,   410,    -1,    -1,   413,  1000,
    1001,   416,   417,  1155,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1183,  1184,    -1,    -1,    -1,    -1,
      -1,    -1,  1191,    -1,  1193,    -1,  1195,    -1,    -1,  1198,
    1199,    -1,     4,    -1,  1203,  1036,  1037,  1038,  1039,  1208,
    1041,  1042,    -1,  1212,    -1,    -1,  1215,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   483,   484,
      42,    -1,   487,    -1,    -1,    47,    48,    -1,    50,   494,
     495,   496,   497,   498,   499,    -1,    -1,    -1,    -1,    -1,
     505,    -1,    -1,    -1,    -1,    67,    -1,   512,    -1,    -1,
      -1,    73,   517,    -1,    -1,    -1,    -1,    79,    -1,    81,
     525,   526,   527,   528,   529,   530,   531,    -1,    -1,   534,
      -1,    -1,    -1,    -1,   539,    -1,    98,    -1,    -1,    -1,
      -1,    -1,   104,    -1,   549,    -1,    -1,    -1,    -1,   554,
     555,   556,   557,   558,    -1,    -1,  1147,    -1,  1149,   564,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   572,   573,    -1,
      -1,   576,    -1,    -1,   579,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   587,   588,    -1,    -1,   591,  1178,    -1,    -1,
      -1,   596,    -1,    -1,    -1,    -1,    -1,    -1,   603,   604,
     605,   606,   607,   608,   609,    -1,    -1,    -1,    -1,   614,
      -1,    -1,    -1,    -1,     4,    -1,   621,   622,   623,   624,
     625,    -1,    -1,    -1,    -1,    -1,    -1,   632,   633,   634,
      -1,   636,    -1,   638,    -1,    25,    26,    -1,   643,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,   652,   653,   654,
     655,    -1,    42,    -1,    -1,    -1,    -1,    47,    48,    27,
      50,    -1,    30,    31,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    52,    53,    -1,    55,    -1,    79,
      58,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    96,    75,    98,    -1,
      78,    -1,    -1,    81,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   731,    -1,   733,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   779,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     835,    -1,    -1,    -1,    -1,    -1,   841,    -1,    -1,    -1,
      -1,    -1,    -1,   848,    -1,    -1,   851,    -1,    -1,    -1,
      -1,    -1,   857,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   866,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     885,    -1,   887,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   896,    -1,    -1,   899,    -1,    -1,   902,    -1,    -1,
     905,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   913,   914,
     915,   916,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     935,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   959,   960,   961,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   978,    -1,    -1,   981,   982,   983,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   992,    -1,    -1,
      -1,    -1,    -1,    -1,   999,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1017,    -1,    -1,    -1,    -1,  1022,    -1,    -1,
    1025,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1062,    -1,  1064,
    1065,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1088,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1096,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1106,    -1,    -1,  1109,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    10,    11,  1130,    -1,    -1,    -1,    -1,
      -1,    -1,  1137,    20,    21,    22,    23,    24,    25,    26,
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
       0,   113,   119,   123,   416,     0,     5,     7,    54,    74,
     415,   415,   417,   417,   416,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   114,   115,   118,
     395,   422,   423,   122,   395,   416,   103,   416,   103,   116,
     402,   402,   102,    11,   120,   121,   403,   415,   424,   124,
     125,   416,   130,    56,    64,    82,   127,   129,   143,   404,
     405,   416,   130,    23,   127,   141,     8,    43,    44,   131,
     135,   144,   417,   417,   417,   103,   105,   410,     9,    11,
      12,    16,    17,    18,   115,   406,   419,   421,   422,   144,
     417,   402,   117,   417,   417,   402,   402,    28,    31,    59,
      67,   146,   148,   150,   152,   153,   155,   128,   419,   115,
     142,   394,   423,   126,   419,   416,   420,   153,   140,   395,
     416,   132,   395,   134,   395,   417,   417,   417,   417,   161,
     162,    70,   156,   103,   105,   398,   399,   401,   398,   416,
     398,   411,   416,   161,   103,   103,   103,   105,   136,   147,
     419,   149,   419,   145,   419,   151,   419,   104,     4,    21,
      26,    29,    35,    36,    38,    40,    41,    47,    48,    50,
      57,    72,    76,    79,    98,   163,   165,   178,   185,   191,
     248,   251,   253,   256,   260,   263,   266,   276,   278,   282,
     295,   300,   309,   313,   417,   402,   402,   402,   104,   403,
     412,   414,   422,   104,   402,   402,   402,   398,   398,   398,
     398,   416,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   402,
      12,   115,   154,   159,   104,   400,   416,   407,   417,   416,
     143,   133,   137,   275,   395,   106,   115,   293,   294,   366,
     367,   265,   395,   252,   395,   164,   395,   115,   312,   315,
     367,   177,   395,   247,   395,   184,   395,   257,   395,   259,
     395,   262,   395,   308,   395,   299,   395,   190,   395,   115,
     281,   397,   423,   424,   277,   395,   160,   103,   105,   157,
     402,   103,   105,   413,     8,   408,   421,   104,    71,   104,
     139,   143,   150,   152,   104,   139,   150,   152,   103,   105,
     279,    11,   396,   422,   103,   368,   416,   103,   105,   267,
     103,   105,   254,   103,   105,   166,   103,   416,   103,   105,
     179,   103,   105,   249,   103,   105,   186,   103,   103,   103,
     103,   105,   310,   103,   105,   301,   103,   103,   105,   283,
     416,   416,   279,   416,   402,   416,   416,   416,   417,   402,
     402,   366,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   158,   411,   409,   138,
     159,   280,   296,   268,   255,   167,   314,   180,   250,   187,
     258,   261,   264,   311,   302,   192,   284,   104,   150,   152,
     104,   102,   398,    27,    42,    51,    55,    73,    81,   104,
     150,   152,   175,   182,   334,   338,   341,   355,    25,    96,
     104,   150,   152,   175,   182,   251,   272,   298,   309,   334,
      30,   104,   150,   152,   175,   182,   246,   253,   256,   260,
     263,   266,   269,   270,   272,   276,   278,   334,   338,   341,
      65,   104,   150,   152,   175,   182,   191,   248,   251,   298,
     309,   334,   338,   344,   355,    20,   104,   150,   152,   169,
     175,    37,   104,   150,   152,   104,   150,   152,   175,   182,
     104,   150,   152,   175,   191,   248,   251,   298,   309,    22,
     104,   150,   152,   175,   182,   189,    75,    78,   104,   150,
     152,   175,   182,   193,   244,   246,   334,   338,   341,   355,
      52,    53,    58,   104,   150,   152,   175,   182,   193,   244,
     246,   334,   338,   346,   349,   352,   355,    46,    77,   104,
     150,   152,   175,   182,   191,   248,   251,   298,   309,   334,
     338,   346,   349,   352,   355,   358,   362,   104,   150,   152,
     175,   182,   191,   248,   251,   355,    45,    60,   104,   150,
     152,   175,   182,   191,   248,   303,   304,   306,   307,   104,
     150,   152,   175,   193,   244,   246,    21,    68,   104,   150,
     152,   175,   182,   287,   292,   334,   416,   417,   417,   417,
     417,   417,   417,   402,   402,   402,   417,   417,   402,   402,
     402,   402,   402,   402,   417,   402,   402,   402,   417,   402,
     402,   402,   402,   402,   402,   402,   402,   417,   402,   417,
     402,   402,   402,   402,   402,   402,   417,   402,   417,   417,
     402,   402,   402,   402,   417,   417,   417,   402,   402,   402,
     402,   417,   417,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   416,   416,   402,   402,
     402,   103,   402,   103,   402,   402,   417,   417,   402,   402,
     402,   402,     8,    88,    93,   115,   337,   339,   181,   419,
      88,    93,   115,   340,   342,   354,   419,    85,    87,    90,
     115,   174,   176,   333,   419,   271,   395,   297,   395,   245,
     419,   343,   419,   168,   395,    84,    86,    89,    91,   316,
     317,   318,   320,   321,   322,   325,   326,   329,   330,   188,
     397,   194,   397,   243,   419,    17,    94,   115,   348,   350,
     390,    18,   115,   345,   347,   390,   391,    92,    95,   115,
     351,   353,   115,   361,   363,   396,   115,   357,   359,   369,
     396,   402,   402,   115,   290,   291,   369,   115,   285,   286,
     369,   416,   416,   416,   398,   103,   105,   183,   416,   416,
     398,   103,   105,   356,   416,   416,   416,   398,   103,   105,
     335,   103,   105,   273,   301,   398,   398,   103,   105,   170,
     402,   416,   416,   416,   416,   398,   103,   105,   195,   398,
     416,   398,   416,   398,   416,   416,   416,   398,   398,   364,
     398,   360,   370,   305,   305,   103,   416,   103,   105,   288,
     416,   402,   402,   402,   402,   402,   103,   105,   319,   103,
     105,   323,   103,   105,   327,   103,   105,   331,   402,   416,
     417,   368,   104,   191,   248,   251,   355,   104,   402,   402,
     104,   206,   336,   274,    83,   172,   402,   402,   402,   402,
      24,    32,   196,   197,   214,   216,   230,   232,   369,   402,
     402,   402,   402,   296,   289,    33,    34,   104,   150,   152,
     240,   242,   104,   150,   152,   104,   150,   152,   175,   182,
     251,   334,   417,   104,   104,   324,   328,   332,   417,   417,
     104,    39,    49,    61,    62,    66,    69,   189,   198,   199,
     201,   203,   208,   223,   226,   228,   402,   231,   104,   104,
     150,   152,   182,   246,   338,   341,   344,   346,   349,   355,
     417,   417,   402,   402,   402,    88,    93,   115,   171,   173,
     104,   244,   246,   338,   341,   346,   349,   355,   358,   104,
     244,   246,   355,   358,   104,   193,   244,   246,   338,   341,
     346,   349,   233,   395,   217,   419,   417,   417,   417,   207,
     417,   417,   417,   402,   193,   402,   402,   402,   215,   232,
     402,   402,   241,   419,   239,   419,   416,   416,   398,   402,
     402,   402,   103,   105,   234,   103,   105,   218,   115,   200,
     202,   390,   204,   419,   115,   225,   371,   372,   373,   209,
     419,   365,   419,    88,    93,   115,   227,   229,   216,   398,
     398,   402,   402,   402,   398,   416,   103,   105,   205,   398,
     106,   374,   375,    14,   377,   378,   103,   105,   210,   103,
     105,   224,   416,   416,   398,   402,   235,   219,   402,   396,
     376,   106,   379,   402,   402,    63,   104,   150,   152,   175,
     182,   237,    80,   104,   150,   152,   175,   182,   221,   206,
     382,   374,   378,   380,   396,   211,   206,   417,   402,   417,
     402,   104,   107,   381,   382,    97,   104,   150,   152,   213,
     240,   242,   104,   115,   236,   238,   391,    16,    17,    18,
     115,   220,   222,   392,   393,     5,   418,   375,   417,   398,
     416,   398,   416,   383,   396,   212,   419,   418,   418,   398,
     108,   109,   418,    85,   384,   389,   418,   418,   110,   106,
     418,   418,   111,    14,   385,   386,   418,   387,   106,   386,
     396,   418,   388,   396,   418,   106,   418,   396
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   112,   113,   113,   114,   115,   116,   117,   116,   118,
     119,   120,   121,   121,   121,   121,   122,   123,   124,   125,
     125,   125,   126,   127,   128,   129,   130,   130,   130,   131,
     132,   133,   133,   133,   133,   133,   134,   135,   136,   136,
     137,   137,   137,   137,   138,   139,   140,   141,   142,   143,
     144,   144,   144,   144,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   155,   156,   157,   157,
     158,   158,   158,   160,   159,   159,   161,   162,   162,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   164,
     165,   166,   166,   167,   167,   167,   167,   167,   168,   169,
     170,   170,   171,   172,   172,   173,   173,   173,   174,   175,
     176,   176,   176,   176,   177,   178,   179,   179,   180,   180,
     180,   180,   180,   181,   182,   183,   183,   184,   185,   186,
     186,   187,   187,   187,   187,   187,   187,   188,   189,   190,
     191,   192,   192,   192,   192,   192,   192,   192,   193,   194,
     195,   195,   196,   196,   196,   197,   197,   197,   197,   197,
     197,   197,   197,   197,   198,   199,   200,   201,   202,   202,
     203,   204,   205,   205,   206,   206,   206,   206,   206,   207,
     208,   209,   210,   210,   211,   211,   211,   211,   211,   211,
     212,   213,   214,   215,   215,   216,   217,   218,   218,   219,
     219,   219,   219,   219,   219,   220,   221,   222,   222,   223,
     224,   224,   225,   226,   227,   228,   229,   229,   229,   230,
     231,   231,   232,   233,   234,   234,   235,   235,   235,   235,
     235,   235,   236,   237,   238,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   249,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   251,   251,   251,
     251,   251,   251,   251,   251,   252,   253,   254,   254,   255,
     255,   255,   255,   255,   255,   255,   255,   255,   255,   255,
     255,   255,   255,   256,   257,   258,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   259,   260,   261,
     261,   261,   261,   261,   261,   261,   261,   261,   261,   261,
     261,   261,   261,   262,   263,   264,   264,   264,   264,   264,
     264,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     264,   264,   264,   265,   266,   267,   267,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   269,   269,   270,
     270,   270,   270,   270,   270,   270,   271,   272,   273,   273,
     274,   274,   274,   274,   274,   274,   274,   275,   276,   277,
     278,   279,   279,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   281,   282,   283,   283,   284,   284,   284,   284,
     284,   284,   284,   284,   285,   285,   286,   287,   288,   288,
     289,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   290,   290,   291,   292,   293,   293,   294,   295,   296,
     296,   296,   296,   296,   296,   296,   296,   296,   296,   297,
     298,   299,   300,   301,   301,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   303,   304,   305,   305,   305,   305,
     305,   306,   307,   308,   309,   310,   310,   311,   311,   311,
     311,   311,   311,   311,   311,   311,   312,   313,   314,   314,
     314,   314,   315,   315,   316,   316,   317,   318,   319,   319,
     320,   320,   320,   321,   322,   323,   323,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   325,   326,   327,   327,
     328,   328,   328,   328,   328,   329,   330,   331,   331,   332,
     332,   332,   332,   332,   332,   332,   332,   333,   334,   335,
     335,   336,   336,   336,   337,   338,   339,   339,   339,   340,
     341,   342,   342,   342,   343,   344,   345,   346,   347,   347,
     348,   349,   350,   350,   350,   351,   352,   353,   353,   353,
     354,   355,   356,   356,   357,   358,   359,   359,   360,   360,
     361,   362,   364,   363,   363,   365,   366,   367,   368,   368,
     370,   369,   372,   371,   373,   371,   371,   374,   375,   376,
     376,   377,   378,   379,   379,   380,   381,   381,   382,   382,
     383,   384,   385,   386,   387,   387,   388,   388,   389,   390,
     391,   391,   392,   392,   393,   393,   394,   394,   395,   395,
     396,   396,   397,   397,   397,   398,   398,   399,   400,   401,
     402,   402,   402,   403,   404,   405,   406,   406,   407,   407,
     408,   408,   408,   409,   409,   410,   410,   411,   411,   412,
     412,   413,   413,   414,   415,   415,   416,   416,   417,   418,
     418,   420,   419,   419,   421,   421,   421,   421,   421,   421,
     421,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   423,   424
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     0,     0,     6,     1,
      13,     1,     0,     2,     2,     2,     1,    13,     1,     0,
       2,     3,     1,     4,     1,     4,     0,     3,     3,     7,
       1,     0,     2,     2,     2,     2,     1,     4,     1,     4,
       0,     2,     2,     2,     1,     4,     1,     7,     1,     4,
       0,     2,     2,     2,     2,     1,     4,     1,     4,     1,
       4,     1,     4,     1,     1,     0,     3,     4,     1,     4,
       0,     2,     2,     0,     3,     1,     1,     0,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     4,     0,     3,     2,     2,     2,     1,     4,
       1,     4,     1,     0,     4,     2,     2,     1,     1,     4,
       2,     2,     2,     1,     1,     4,     1,     4,     0,     3,
       2,     2,     2,     1,     4,     1,     3,     1,     4,     1,
       4,     0,     2,     3,     2,     2,     2,     1,     4,     1,
       7,     0,     3,     2,     2,     2,     2,     2,     4,     1,
       1,     4,     1,     1,     1,     0,     2,     2,     2,     3,
       3,     2,     3,     3,     2,     0,     1,     4,     2,     1,
       4,     1,     1,     4,     0,     2,     2,     2,     2,     1,
       4,     1,     1,     4,     0,     2,     2,     2,     2,     2,
       1,     4,     3,     0,     3,     4,     1,     1,     4,     0,
       3,     2,     2,     2,     2,     1,     4,     2,     1,     4,
       1,     4,     1,     4,     1,     4,     2,     2,     1,     2,
       0,     2,     5,     1,     1,     4,     0,     3,     2,     2,
       2,     2,     1,     4,     2,     1,     1,     4,     1,     4,
       1,     4,     1,     4,     1,     4,     1,     4,     0,     2,
       2,     2,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     4,     0,
       3,     3,     3,     2,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     7,     1,     0,     3,     3,     3,     2,
       3,     2,     2,     2,     2,     2,     2,     1,     7,     0,
       3,     3,     3,     2,     2,     3,     2,     2,     2,     2,
       2,     2,     2,     1,     7,     0,     3,     3,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     1,     4,     1,     4,     0,     3,     3,
       2,     2,     2,     2,     2,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     4,
       0,     3,     3,     2,     2,     2,     3,     1,     4,     1,
       4,     1,     4,     0,     3,     3,     3,     2,     2,     2,
       2,     2,     1,     4,     1,     4,     0,     3,     3,     2,
       2,     2,     3,     3,     2,     1,     1,     4,     1,     4,
       0,     3,     3,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     7,     2,     1,     1,     7,     0,
       3,     3,     2,     2,     2,     3,     3,     3,     3,     1,
       4,     1,     4,     1,     4,     0,     3,     2,     2,     2,
       3,     3,     3,     3,     2,     5,     0,     3,     3,     3,
       3,     2,     5,     1,     4,     1,     4,     0,     3,     3,
       2,     2,     2,     3,     3,     3,     1,     7,     0,     2,
       2,     5,     2,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     1,     3,     1,     4,     0,     2,     3,
       2,     2,     2,     2,     2,     2,     1,     3,     1,     4,
       0,     2,     3,     2,     2,     1,     3,     1,     4,     0,
       3,     2,     2,     2,     2,     2,     2,     1,     4,     1,
       4,     0,     2,     2,     1,     4,     2,     2,     1,     1,
       4,     2,     2,     1,     1,     4,     1,     4,     2,     1,
       1,     4,     2,     2,     1,     1,     4,     2,     2,     1,
       1,     4,     1,     4,     1,     4,     2,     1,     0,     3,
       1,     4,     0,     3,     1,     1,     2,     2,     0,     2,
       0,     3,     0,     2,     0,     2,     1,     3,     2,     0,
       2,     3,     2,     0,     2,     2,     0,     2,     0,     6,
       5,     5,     5,     4,     0,     2,     0,     5,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     1,
       1,     1,     2,     2,     1,     2,     4,     1,     1,     1,
       0,     2,     2,     3,     2,     1,     0,     1,     0,     2,
       0,     2,     3,     0,     5,     1,     4,     0,     3,     1,
       3,     1,     4,     1,     1,     1,     0,     2,     2,     0,
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

  case 29:

    { actual_type = (yyvsp[-4].token);
                   backup_type = NODE;
                   actual = NULL;
                 }

    break;

  case 30:

    { YANG_ADDELEM(trg->imp, trg->imp_size);
                                     /* HACK for unres */
                                     ((struct lys_import *)actual)->module = (struct lys_module *)s;
                                     s = NULL;
                                     (yyval.token) = actual_type;
                                     actual_type = IMPORT_KEYWORD;
                                   }

    break;

  case 31:

    { (yyval.i) = 0; }

    break;

  case 33:

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

  case 34:

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

  case 35:

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

  case 36:

    { YANG_ADDELEM(trg->inc, trg->inc_size);
                                     /* HACK for unres */
                                     ((struct lys_include *)actual)->submodule = (struct lys_submodule *)s;
                                     s = NULL;
                                     (yyval.token) = actual_type;
                                     actual_type = INCLUDE_KEYWORD;
                                   }

    break;

  case 37:

    { actual_type = (yyvsp[-1].token);
                                                                backup_type = NODE;
                                                                actual = NULL;
                                                              }

    break;

  case 40:

    { (yyval.i) = 0; }

    break;

  case 41:

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

  case 42:

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

  case 43:

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

  case 44:

    { backup_type = actual_type;
                                  actual_type = REVISION_DATE_KEYWORD;
                                }

    break;

  case 46:

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

  case 47:

    { actual_type = (yyvsp[-4].token); }

    break;

  case 48:

    { backup_type = actual_type;
                             actual_type = PREFIX_KEYWORD;
                           }

    break;

  case 49:

    { if (yang_read_prefix(trg, actual, s)) {
                                                       YYABORT;
                                                     }
                                                     s = NULL;
                                                   }

    break;

  case 51:

    { if (yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 52:

    { if (yang_read_common(trg, s, CONTACT_KEYWORD)) {
                                 YYABORT;
                               }
                               s = NULL;
                             }

    break;

  case 53:

    { if (yang_read_description(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 54:

    { if (yang_read_reference(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 55:

    { backup_type = actual_type;
                           actual_type = ORGANIZATION_KEYWORD;
                         }

    break;

  case 57:

    { backup_type = actual_type;
                      actual_type = CONTACT_KEYWORD;
                    }

    break;

  case 59:

    { backup_type = actual_type;
                          actual_type = DESCRIPTION_KEYWORD;
                        }

    break;

  case 61:

    { backup_type = actual_type;
                        actual_type = REFERENCE_KEYWORD;
                      }

    break;

  case 63:

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

  case 64:

    { YANG_ADDELEM(trg->rev, trg->rev_size);
                                  actual = yang_read_revision(trg, s, actual);
                                  (yyval.token) = actual_type;
                                  actual_type = REVISION_KEYWORD;
                                }

    break;

  case 67:

    { actual_type = (yyvsp[-1].token);
                                                                     actual = NULL;
                                                                   }

    break;

  case 71:

    { if (yang_read_description(trg, actual, s, "revision",REVISION_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 72:

    { if (yang_read_reference(trg, actual, s, "revision", REVISION_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 73:

    { s = strdup(yyget_text(scanner));
                              if (!s) {
                                LOGMEM;
                                YYABORT;
                              }
                            }

    break;

  case 75:

    { if (lyp_check_date(s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 76:

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

  case 77:

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

  case 78:

    { actual = NULL; }

    break;

  case 89:

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

  case 90:

    { struct lys_ext *ext = actual;
                  ext->plugin = ext_get_plugin(ext->name, ext->module->name, ext->module->rev ? ext->module->rev[0].date : NULL);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 95:

    { if (((struct lys_ext *)actual)->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      ((struct lys_ext *)actual)->flags |= (yyvsp[0].i);
                                    }

    break;

  case 96:

    { if (yang_read_description(trg, actual, s, "extension", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 97:

    { if (yang_read_reference(trg, actual, s, "extension", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 98:

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

  case 99:

    { actual_type = (yyvsp[-1].token); }

    break;

  case 102:

    { (yyval.uint) = (yyvsp[0].uint);
                                       backup_type = actual_type;
                                       actual_type = YIN_ELEMENT_KEYWORD;
                                     }

    break;

  case 104:

    { ((struct lys_ext *)actual)->flags |= (yyvsp[-1].uint); }

    break;

  case 105:

    { (yyval.uint) = LYS_YINELEM; }

    break;

  case 106:

    { (yyval.uint) = 0; }

    break;

  case 107:

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

  case 108:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = STATUS_KEYWORD;
                           }

    break;

  case 109:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 110:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 111:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 112:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 113:

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

  case 114:

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

  case 115:

    { actual = (yyvsp[-1].backup_token).actual;
                actual_type = (yyvsp[-1].backup_token).token;
              }

    break;

  case 117:

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

    { if (((struct lys_feature *)actual)->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "feature");
                                      YYABORT;
                                    }
                                    ((struct lys_feature *)actual)->flags |= (yyvsp[0].i);
                                  }

    break;

  case 121:

    { if (yang_read_description(trg, actual, s, "feature", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 122:

    { if (yang_read_reference(trg, actual, s, "feature", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 123:

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

  case 124:

    { actual = (yyvsp[-1].backup_token).actual;
                   actual_type = (yyvsp[-1].backup_token).token;
                 }

    break;

  case 127:

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

  case 128:

    { actual = (yyvsp[-1].backup_token).actual;
                 actual_type = (yyvsp[-1].backup_token).token;
               }

    break;

  case 130:

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

  case 132:

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

  case 134:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 135:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 136:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 137:

    { backup_type = actual_type;
                                   actual_type = BASE_KEYWORD;
                                 }

    break;

  case 139:

    { tpdf_parent = actual;
                                      (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
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
                                    }

    break;

  case 140:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 141:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 142:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 143:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 144:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 145:

    { if ((yyvsp[-1].nodes).node.ptr_tpdf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "typedef");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_tpdf->flags |= (yyvsp[0].i);
                               }

    break;

  case 146:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 147:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 148:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 149:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       actual_type = TYPE_KEYWORD;
                                     }

    break;

  case 152:

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

  case 156:

    { if (yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 157:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 158:

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

  case 164:

    { actual_type = (yyvsp[-1].backup_token).token;
                                   actual = (yyvsp[-1].backup_token).actual;
                                 }

    break;

  case 165:

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

  case 166:

    { (yyval.uint) = (yyvsp[0].uint);
                                               backup_type = actual_type;
                                               actual_type = FRACTION_DIGITS_KEYWORD;
                                             }

    break;

  case 167:

    { if (yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 168:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 169:

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

  case 170:

    { actual = (yyvsp[-1].backup_token).actual;
               actual_type = (yyvsp[-1].backup_token).token;
             }

    break;

  case 171:

    { (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         if (!(actual = yang_read_length(trg, actual, s))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 174:

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

  case 175:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 176:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 177:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 178:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 179:

    { (yyval.backup_token).token = actual_type;
                   (yyval.backup_token).actual = actual;
                   actual_type = PATTERN_KEYWORD;
                 }

    break;

  case 180:

    { if (yang_read_pattern(trg, actual, (yyvsp[-1].str), (yyvsp[0].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual_type = (yyvsp[-2].backup_token).token;
                                                                        actual = (yyvsp[-2].backup_token).actual;
                                                                      }

    break;

  case 181:

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

  case 182:

    { (yyval.ch) = 0x06; }

    break;

  case 183:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 184:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 185:

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

  case 186:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 187:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 188:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 189:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 190:

    { backup_type = actual_type;
                       actual_type = MODIFIER_KEYWORD;
                     }

    break;

  case 191:

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

  case 192:

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

  case 195:

    { if (yang_check_enum(yang_type, actual, &cnt_val, is_value)) {
               YYABORT;
             }
             actual = (yyvsp[-1].backup_token).actual;
             actual_type = (yyvsp[-1].backup_token).token;
           }

    break;

  case 196:

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

  case 198:

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

  case 201:

    { if (is_value) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                is_value = 1;
                              }

    break;

  case 202:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 203:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 204:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 205:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = VALUE_KEYWORD;
                                 }

    break;

  case 206:

    { ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                      /* keep the highest enum value for automatic increment */
                      if ((yyvsp[-1].i) >= cnt_val) {
                        cnt_val = (yyvsp[-1].i);
                        cnt_val++;
                      }
                    }

    break;

  case 207:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 208:

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

  case 209:

    { actual_type = (yyvsp[-1].backup_token).token;
                                                        actual = (yyvsp[-1].backup_token).actual;
                                                      }

    break;

  case 212:

    { backup_type = actual_type;
                         actual_type = PATH_KEYWORD;
                       }

    break;

  case 214:

    { (yyval.i) = (yyvsp[0].i);
                                                 backup_type = actual_type;
                                                 actual_type = REQUIRE_INSTANCE_KEYWORD;
                                               }

    break;

  case 215:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 216:

    { (yyval.i) = 1; }

    break;

  case 217:

    { (yyval.i) = -1; }

    break;

  case 218:

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

  case 219:

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

  case 222:

    { if (yang_check_bit(yang_type, actual, &cnt_val, is_value)) {
                      YYABORT;
                    }
                    actual = (yyvsp[-2].backup_token).actual;
                    actual_type = (yyvsp[-2].backup_token).token;
                  }

    break;

  case 223:

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

  case 225:

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

  case 228:

    { if (is_value) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  is_value = 1;
                                }

    break;

  case 229:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags,
                                                     LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                  YYABORT;
                                }
                              }

    break;

  case 230:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 231:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 232:

    { (yyval.uint) = (yyvsp[0].uint);
                                             backup_type = actual_type;
                                             actual_type = POSITION_KEYWORD;
                                           }

    break;

  case 233:

    { ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                         /* keep the highest position value for automatic increment */
                         if ((yyvsp[-1].uint) >= cnt_val) {
                           cnt_val = (yyvsp[-1].uint);
                           cnt_val++;
                         }
                       }

    break;

  case 234:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 235:

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

  case 236:

    { backup_type = actual_type;
                            actual_type = ERROR_MESSAGE_KEYWORD;
                          }

    break;

  case 238:

    { backup_type = actual_type;
                            actual_type = ERROR_APP_TAG_KEYWORD;
                          }

    break;

  case 240:

    { backup_type = actual_type;
                    actual_type = UNITS_KEYWORD;
                  }

    break;

  case 242:

    { backup_type = actual_type;
                      actual_type = DEFAULT_KEYWORD;
                    }

    break;

  case 244:

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

  case 245:

    { LOGDBG("YANG: finished parsing grouping statement \"%s\"", data_node->name);
                 actual_type = (yyvsp[-1].backup_token).token;
                 actual = (yyvsp[-1].backup_token).actual;
                 data_node = (yyvsp[-1].backup_token).actual;
               }

    break;

  case 248:

    { (yyval.nodes).grouping = actual; }

    break;

  case 249:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 250:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 251:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 256:

    { if (trg->version < 2) {
                                                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 265:

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

  case 266:

    { LOGDBG("YANG: finished parsing container statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                  data_node = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 268:

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

  case 269:

    { (yyval.nodes).container = actual; }

    break;

  case 273:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 274:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 275:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 276:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 277:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 280:

    { if (trg->version < 2) {
                                                      LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).container, "notification");
                                                      YYABORT;
                                                    }
                                                  }

    break;

  case 283:

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

  case 284:

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

  case 285:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 288:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 289:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 291:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 292:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 293:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 294:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 295:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 296:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 297:

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

  case 298:

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

  case 299:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                               }

    break;

  case 302:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                            (yyval.nodes) = (yyvsp[-2].nodes);
                                          }

    break;

  case 303:

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

  case 304:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 306:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 307:

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

  case 308:

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

  case 309:

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

  case 310:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 311:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 312:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 313:

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

  case 314:

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

  case 315:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 319:

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

  case 320:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 321:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 322:

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

  case 323:

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

  case 324:

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

  case 325:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 326:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 327:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 331:

    { if (trg->version < 2) {
                                                 LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_list, "notification");
                                                 YYABORT;
                                               }
                                             }

    break;

  case 333:

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

  case 334:

    { LOGDBG("YANG: finished parsing choice statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 336:

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

  case 337:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                            }

    break;

  case 340:

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

  case 341:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 342:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 343:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 344:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 345:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 355:

    { if (trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 356:

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

  case 357:

    { LOGDBG("YANG: finished parsing case statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 359:

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

  case 360:

    { (yyval.nodes).cs = actual; }

    break;

  case 363:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 364:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 365:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 367:

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

  case 368:

    { LOGDBG("YANG: finished parsing anyxml statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 369:

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

  case 370:

    { LOGDBG("YANG: finished parsing anydata statement \"%s\"", data_node->name);
                actual_type = (yyvsp[-1].backup_token).token;
                actual = (yyvsp[-1].backup_token).actual;
                data_node = (yyvsp[-1].backup_token).actual;
              }

    break;

  case 372:

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

  case 373:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
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

  case 383:

    { LOGDBG("YANG: finished parsing uses statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

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

    { (yyval.nodes).uses = actual; }

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

  case 396:

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

  case 397:

    { actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 399:

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

  case 400:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 401:

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

  case 402:

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

  case 403:

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

  case 404:

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

  case 405:

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

  case 406:

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

  case 407:

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

  case 408:

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

  case 409:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 410:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 413:

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

  case 414:

    { LOGDBG("YANG: finished parsing augment statement \"%s\"", data_node->name);
                         actual_type = (yyvsp[-4].backup_token).token;
                         actual = (yyvsp[-4].backup_token).actual;
                         data_node = (yyvsp[-4].backup_token).actual;
                       }

    break;

  case 417:

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

  case 418:

    { LOGDBG("YANG: finished parsing augment statement \"%s\"", data_node->name);
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                    data_node = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 419:

    { (yyval.nodes).augment = actual; }

    break;

  case 422:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 423:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 424:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 427:

    { if (trg->version < 2) {
                                                    LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).augment, "notification");
                                                    YYABORT;
                                                  }
                                                }

    break;

  case 429:

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

  case 430:

    { LOGDBG("YANG: finished parsing action statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 431:

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

  case 432:

    { LOGDBG("YANG: finished parsing rpc statement \"%s\"", data_node->name);
            actual_type = (yyvsp[-1].backup_token).token;
            actual = (yyvsp[-1].backup_token).actual;
            data_node = (yyvsp[-1].backup_token).actual;
          }

    break;

  case 434:

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

  case 435:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                         }

    break;

  case 437:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 438:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 439:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 442:

    { if ((yyvsp[-2].nodes).node.flag & LYS_RPC_INPUT) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_rpc, "input", "rpc");
                                         YYABORT;
                                       }
                                       (yyvsp[-2].nodes).node.flag |= LYS_RPC_INPUT;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 443:

    { if ((yyvsp[-2].nodes).node.flag & LYS_RPC_OUTPUT) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_rpc, "output", "rpc");
                                          YYABORT;
                                        }
                                        (yyvsp[-2].nodes).node.flag |= LYS_RPC_OUTPUT;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      }

    break;

  case 444:

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

  case 445:

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

  case 451:

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

  case 452:

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

  case 453:

    { (yyval.backup_token).token = actual_type;
                                           (yyval.backup_token).actual = actual;
                                           if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                           actual_type = NOTIFICATION_KEYWORD;
                                         }

    break;

  case 454:

    { LOGDBG("YANG: finished parsing notification statement \"%s\"", data_node->name);
                     actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                     data_node = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 456:

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

  case 457:

    { (yyval.nodes).notif = actual; }

    break;

  case 460:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 461:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 462:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 466:

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

  case 467:

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

  case 468:

    { (yyval.dev) = actual; }

    break;

  case 469:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 470:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 476:

    { (yyval.backup_token).token = actual_type;
                                               (yyval.backup_token).actual = actual;
                                               if (!(actual = yang_read_deviate_unsupported(actual))) {
                                                 YYABORT;
                                               }
                                               actual_type = NOT_SUPPORTED_KEYWORD;
                                             }

    break;

  case 477:

    { actual_type = (yyvsp[-2].backup_token).token;
                              actual = (yyvsp[-2].backup_token).actual;
                            }

    break;

  case 483:

    { (yyval.backup_token).token = actual_type;
                           (yyval.backup_token).actual = actual;
                           if (!(actual = yang_read_deviate(actual, LY_DEVIATE_ADD))) {
                             YYABORT;
                           }
                           actual_type = ADD_KEYWORD;
                         }

    break;

  case 484:

    { actual_type = (yyvsp[-2].backup_token).token;
                    actual = (yyvsp[-2].backup_token).actual;
                  }

    break;

  case 486:

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

  case 487:

    { (yyval.deviate) = actual; }

    break;

  case 488:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 490:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 491:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 492:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 493:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 494:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 495:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 496:

    { (yyval.backup_token).token = actual_type;
                                 (yyval.backup_token).actual = actual;
                                 if (!(actual = yang_read_deviate(actual, LY_DEVIATE_DEL))) {
                                   YYABORT;
                                 }
                                 actual_type = DELETE_KEYWORD;
                               }

    break;

  case 497:

    { actual_type = (yyvsp[-2].backup_token).token;
                       actual = (yyvsp[-2].backup_token).actual;
                     }

    break;

  case 499:

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

  case 500:

    { (yyval.deviate) = actual; }

    break;

  case 501:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 503:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 504:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 505:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_deviate(actual, LY_DEVIATE_RPL))) {
                                     YYABORT;
                                   }
                                   actual_type = REPLACE_KEYWORD;
                                 }

    break;

  case 506:

    { actual_type = (yyvsp[-2].backup_token).token;
                        actual = (yyvsp[-2].backup_token).actual;
                      }

    break;

  case 508:

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

  case 509:

    { (yyval.deviate) = actual; }

    break;

  case 511:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 512:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 513:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 514:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 515:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 516:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 517:

    { (yyval.backup_token).token = actual_type;
                        (yyval.backup_token).actual = actual;
                        if (!(actual = yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type = WHEN_KEYWORD;
                      }

    break;

  case 518:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 522:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 523:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 524:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = CONFIG_KEYWORD;
                           }

    break;

  case 525:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 526:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 527:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 528:

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

  case 529:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = MANDATORY_KEYWORD;
                                 }

    break;

  case 530:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 531:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 532:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 533:

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

  case 534:

    { backup_type = actual_type;
                       actual_type = PRESENCE_KEYWORD;
                     }

    break;

  case 536:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MIN_ELEMENTS_KEYWORD;
                                 }

    break;

  case 537:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 538:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 539:

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

  case 540:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MAX_ELEMENTS_KEYWORD;
                                 }

    break;

  case 541:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 542:

    { (yyval.uint) = 0; }

    break;

  case 543:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 544:

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

  case 545:

    { (yyval.i) = (yyvsp[0].i);
                                     backup_type = actual_type;
                                     actual_type = ORDERED_BY_KEYWORD;
                                   }

    break;

  case 546:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 547:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 548:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 549:

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

  case 550:

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

  case 551:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 554:

    { backup_type = actual_type;
                             actual_type = UNIQUE_KEYWORD;
                           }

    break;

  case 560:

    { backup_type = actual_type;
                       actual_type = KEY_KEYWORD;
                     }

    break;

  case 562:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }

    break;

  case 565:

    { (yyval.backup_token).token = actual_type;
                        (yyval.backup_token).actual = actual;
                        if (!(actual = yang_read_range(trg, actual, s))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 566:

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

  case 570:

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

  case 572:

    { tmp_s = yyget_text(scanner); }

    break;

  case 573:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM;
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 574:

    { tmp_s = yyget_text(scanner); }

    break;

  case 575:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM;
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 599:

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

  case 600:

    { (yyval.uint) = 0; }

    break;

  case 601:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 602:

    { (yyval.i) = 0; }

    break;

  case 603:

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

  case 609:

    { if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 614:

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

  case 615:

    { s = (yyvsp[-1].str); }

    break;

  case 616:

    { s = (yyvsp[-3].str); }

    break;

  case 617:

    { actual_type = backup_type;
                 backup_type = NODE;
                 (yyval.str) = s;
                 s = NULL;
               }

    break;

  case 618:

    { actual_type = backup_type;
                           backup_type = NODE;
                         }

    break;

  case 619:

    { (yyval.str) = s;
                          s = NULL;
                        }

    break;

  case 623:

    { actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 624:

    { (yyval.backup_token).token = actual_type;
                                                (yyval.backup_token).actual = actual;
                                                if (!(actual = yang_read_ext(trg, actual, (yyvsp[-1].str), s, actual_type, backup_type))) {
                                                  YYABORT;
                                                }
                                                s = NULL;
                                                actual_type = EXTENSION_INSTANCE;
                                              }

    break;

  case 625:

    { (yyval.str) = s; s = NULL; }

    break;

  case 643:

    { (yyval.str) = yyget_text(scanner); }

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
