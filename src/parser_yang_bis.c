/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 26 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:339  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include "resolve.h"
#include "common.h"
#include "parser_yang.h"
#include "parser_yang_bis.h"
#include "parser_yang_lex.h"
#include "parser.h"

/* only syntax rules */
#define EXTENSION_ARG 0x01
#define EXTENSION_STA 0x02
#define EXTENSION_DSC 0x04
#define EXTENSION_REF 0x08
#define DISABLE_INHERIT 0
#define ENABLE_INHERIT 0x01

void yyerror(void *scanner, ...);
char *s, *tmp_s;
char rev[LY_REV_SIZE];
struct lys_module *trg;
/* temporary pointer for the check extension nacm */
struct lys_node *data_node;
/* pointer on the current parsed element*/
void *actual;
int config_inherit;
int actual_type;
int64_t cnt_val;

#line 100 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:339  */

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
#ifndef YY_YY_HOME_XVICAN01_DOCUMENTS_BC_LIBYANG_SRC_PARSER_YANG_BIS_H_INCLUDED
# define YY_YY_HOME_XVICAN01_DOCUMENTS_BC_LIBYANG_SRC_PARSER_YANG_BIS_H_INCLUDED
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
    USER_KEYWORD = 350
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 60 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:355  */

  int32_t i;
  uint32_t uint;
  char *str;
  void *v;
  struct lys_module *inc;
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

#line 257 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all);

#endif /* !YY_YY_HOME_XVICAN01_DOCUMENTS_BC_LIBYANG_SRC_PARSER_YANG_BIS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 271 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:358  */

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
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYLAST   2952

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  336
/* YYNRULES -- Number of rules.  */
#define YYNRULES  730
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1184

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   350

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
     104,   105,     2,    96,     2,     2,     2,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    99,
       2,   103,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   101,     2,   102,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    97,     2,    98,     2,     2,     2,     2,
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
      95
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   244,   244,   245,   251,   251,   262,   264,   263,   276,
     288,   276,   297,   298,   299,   300,   303,   315,   303,   326,
     327,   333,   333,   342,   344,   344,   348,   356,   358,   381,
     382,   385,   393,   385,   410,   412,   412,   432,   433,   438,
     439,   441,   443,   452,   463,   452,   472,   474,   475,   476,
     477,   482,   488,   490,   492,   494,   496,   504,   507,   507,
     516,   517,   517,   522,   523,   528,   535,   535,   544,   550,
     591,   594,   595,   596,   597,   598,   599,   600,   606,   607,
     608,   610,   610,   629,   630,   634,   635,   642,   649,   660,
     672,   672,   674,   675,   679,   681,   682,   683,   694,   696,
     697,   696,   700,   701,   702,   703,   720,   720,   729,   730,
     735,   754,   761,   767,   772,   778,   780,   780,   789,   791,
     790,   798,   799,   810,   816,   822,   829,   833,   831,   853,
     863,   863,   870,   873,   873,   885,   886,   892,   898,   903,
     909,   910,   916,   917,   918,   925,   933,   938,   942,   972,
     973,   977,   978,   979,   987,  1000,  1006,  1007,  1025,  1029,
    1039,  1040,  1045,  1060,  1065,  1070,  1075,  1081,  1085,  1095,
    1096,  1101,  1101,  1119,  1120,  1123,  1135,  1145,  1146,  1151,
    1152,  1161,  1167,  1172,  1178,  1190,  1191,  1209,  1214,  1215,
    1220,  1222,  1224,  1228,  1232,  1247,  1247,  1265,  1266,  1268,
    1279,  1289,  1290,  1295,  1296,  1305,  1311,  1316,  1322,  1334,
    1335,  1350,  1352,  1354,  1356,  1358,  1358,  1365,  1366,  1371,
    1391,  1397,  1402,  1407,  1408,  1415,  1418,  1419,  1420,  1421,
    1422,  1423,  1424,  1427,  1427,  1435,  1436,  1441,  1475,  1475,
    1477,  1484,  1484,  1492,  1493,  1499,  1505,  1510,  1515,  1515,
    1520,  1520,  1528,  1528,  1535,  1542,  1535,  1567,  1595,  1595,
    1597,  1604,  1609,  1604,  1616,  1617,  1617,  1625,  1628,  1634,
    1640,  1646,  1651,  1657,  1664,  1657,  1688,  1716,  1716,  1718,
    1725,  1730,  1725,  1737,  1738,  1738,  1746,  1752,  1766,  1780,
    1792,  1798,  1803,  1809,  1816,  1809,  1841,  1882,  1882,  1884,
    1891,  1891,  1899,  1909,  1917,  1923,  1937,  1951,  1963,  1969,
    1974,  1979,  1979,  1987,  1987,  1992,  1992,  2000,  2000,  2011,
    2013,  2012,  2030,  2051,  2051,  2053,  2066,  2078,  2086,  2094,
    2102,  2111,  2120,  2120,  2130,  2131,  2134,  2135,  2136,  2137,
    2138,  2141,  2141,  2149,  2150,  2154,  2174,  2174,  2176,  2183,
    2189,  2194,  2199,  2199,  2206,  2206,  2217,  2218,  2222,  2249,
    2249,  2251,  2258,  2258,  2266,  2272,  2278,  2284,  2289,  2295,
    2295,  2311,  2312,  2316,  2351,  2351,  2353,  2360,  2366,  2371,
    2376,  2376,  2384,  2384,  2399,  2399,  2408,  2409,  2414,  2415,
    2418,  2438,  2445,  2469,  2493,  2512,  2531,  2556,  2581,  2586,
    2592,  2601,  2592,  2608,  2609,  2612,  2621,  2612,  2633,  2654,
    2654,  2656,  2663,  2669,  2674,  2679,  2679,  2687,  2687,  2697,
    2698,  2701,  2701,  2712,  2713,  2718,  2746,  2753,  2759,  2764,
    2769,  2769,  2777,  2777,  2782,  2782,  2792,  2792,  2803,  2811,
    2803,  2818,  2838,  2838,  2846,  2846,  2851,  2851,  2861,  2869,
    2861,  2876,  2876,  2886,  2890,  2895,  2922,  2929,  2935,  2940,
    2945,  2945,  2953,  2953,  2958,  2958,  2965,  2974,  2965,  2987,
    3006,  3013,  3020,  3030,  3031,  3033,  3038,  3040,  3041,  3042,
    3045,  3047,  3047,  3053,  3054,  3058,  3080,  3088,  3096,  3112,
    3120,  3127,  3134,  3145,  3157,  3157,  3163,  3164,  3168,  3190,
    3198,  3209,  3219,  3228,  3228,  3234,  3235,  3239,  3244,  3250,
    3258,  3266,  3273,  3280,  3291,  3303,  3303,  3306,  3307,  3311,
    3312,  3317,  3323,  3325,  3326,  3325,  3329,  3330,  3331,  3346,
    3348,  3349,  3348,  3352,  3353,  3354,  3369,  3371,  3373,  3374,
    3395,  3397,  3398,  3399,  3420,  3422,  3423,  3424,  3436,  3436,
    3444,  3445,  3450,  3452,  3453,  3455,  3456,  3458,  3460,  3460,
    3469,  3472,  3472,  3483,  3486,  3496,  3517,  3519,  3520,  3523,
    3523,  3542,  3542,  3551,  3551,  3560,  3563,  3565,  3567,  3568,
    3570,  3572,  3574,  3575,  3577,  3579,  3580,  3582,  3583,  3585,
    3587,  3590,  3594,  3596,  3597,  3599,  3600,  3602,  3604,  3615,
    3616,  3619,  3620,  3632,  3633,  3635,  3636,  3638,  3639,  3645,
    3646,  3649,  3650,  3651,  3677,  3678,  3681,  3682,  3683,  3686,
    3686,  3694,  3696,  3697,  3699,  3700,  3702,  3703,  3705,  3706,
    3708,  3709,  3711,  3712,  3714,  3715,  3718,  3719,  3722,  3724,
    3725,  3729,  3729,  3738,  3740,  3741,  3742,  3743,  3744,  3745,
    3746,  3748,  3749,  3750,  3751,  3752,  3753,  3754,  3755,  3756,
    3757,  3758,  3759,  3760,  3761,  3762,  3763,  3764,  3765,  3766,
    3767,  3768,  3769,  3770,  3771,  3772,  3773,  3774,  3775,  3776,
    3777,  3778,  3779,  3780,  3781,  3782,  3783,  3784,  3785,  3786,
    3787,  3788,  3789,  3790,  3791,  3792,  3793,  3794,  3795,  3796,
    3797,  3798,  3799,  3800,  3801,  3802,  3803,  3804,  3805,  3806,
    3807,  3808,  3809,  3810,  3811,  3812,  3813,  3814,  3815,  3816,
    3817,  3818,  3819,  3820,  3821,  3822,  3823,  3824,  3825,  3828,
    3837
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
  "USER_KEYWORD", "'+'", "'{'", "'}'", "';'", "'/'", "'['", "']'", "'='",
  "'('", "')'", "$accept", "start", "string_1", "$@1", "string_2", "$@2",
  "module_stmt", "$@3", "$@4", "module_header_stmts", "submodule_stmt",
  "$@5", "$@6", "submodule_header_stmts", "$@7", "yang_version_stmt",
  "yang_version_arg_str", "$@8", "namespace_stmt", "linkage_stmts",
  "import_stmt", "$@9", "$@10", "tmp_identifier_arg_str", "include_stmt",
  "$@11", "include_end", "revision_date_opt", "revision_date_stmt",
  "revision_date", "belongs_to_stmt", "$@12", "$@13", "prefix_stmt",
  "meta_stmts", "organization_stmt", "contact_stmt", "description_stmt",
  "reference_stmt", "revision_stmts", "revision_stmt", "$@14",
  "revision_end", "$@15", "revision_opt_stmt", "date_arg_str", "$@16",
  "body_stmts", "body_stmt", "extension_stmt", "$@17", "extension_end",
  "extension_opt_stmt", "argument_stmt", "$@18", "argument_end",
  "yin_element_stmt", "yin_element_arg_str", "status_stmt",
  "status_read_stmt", "$@19", "$@20", "status_arg_str", "feature_stmt",
  "$@21", "feature_end", "feature_opt_stmt", "if_feature_stmt",
  "identity_stmt", "$@22", "identity_end", "$@23", "identity_opt_stmt",
  "base_stmt", "typedef_stmt", "$@24", "typedef_arg_str", "type_stmt",
  "$@25", "type_opt_stmt", "$@26", "type_end", "type_body_stmts",
  "decimal_string_restrictions", "union_spec", "fraction_digits_stmt",
  "fraction_digits_arg_str", "length_stmt", "length_arg_str", "length_end",
  "message_opt_stmt", "pattern_stmt", "pattern_arg_str", "pattern_end",
  "enum_specification", "$@27", "enum_stmts", "enum_stmt", "enum_arg_str",
  "enum_end", "enum_opt_stmt", "value_stmt", "integer_value_arg_str",
  "range_stmt", "range_end", "path_stmt", "require_instance_stmt",
  "require_instance_arg_str", "bits_specification", "$@28", "bit_stmts",
  "bit_stmt", "bit_arg_str", "bit_end", "bit_opt_stmt", "position_stmt",
  "position_value_arg_str", "error_message_stmt", "error_app_tag_stmt",
  "units_stmt", "default_stmt", "grouping_stmt", "$@29", "grouping_end",
  "grouping_opt_stmt", "data_def_stmt", "container_stmt", "$@30",
  "container_end", "container_opt_stmt", "$@31", "$@32", "$@33", "$@34",
  "$@35", "leaf_stmt", "$@36", "$@37", "leaf_opt_stmt", "$@38", "$@39",
  "$@40", "$@41", "leaf_list_stmt", "$@42", "$@43", "leaf_list_opt_stmt",
  "$@44", "$@45", "$@46", "$@47", "list_stmt", "$@48", "$@49",
  "list_opt_stmt", "$@50", "$@51", "$@52", "$@53", "$@54", "choice_stmt",
  "$@55", "choice_end", "$@56", "choice_opt_stmt", "$@57", "$@58",
  "short_case_case_stmt", "short_case_stmt", "case_stmt", "$@59",
  "case_end", "case_opt_stmt", "$@60", "$@61", "anyxml_stmt", "$@62",
  "anyxml_end", "anyxml_opt_stmt", "$@63", "$@64", "uses_stmt", "$@65",
  "uses_end", "uses_opt_stmt", "$@66", "$@67", "$@68", "refine_stmt",
  "$@69", "refine_end", "refine_arg_str", "refine_body_opt_stmts",
  "uses_augment_stmt", "$@70", "$@71", "uses_augment_arg_str",
  "augment_stmt", "$@72", "$@73", "augment_opt_stmt", "$@74", "$@75",
  "$@76", "augment_arg_str", "rpc_stmt", "$@77", "rpc_end", "rpc_opt_stmt",
  "$@78", "$@79", "$@80", "$@81", "input_stmt", "$@82", "$@83",
  "input_output_opt_stmt", "$@84", "$@85", "$@86", "output_stmt", "$@87",
  "$@88", "notification_stmt", "$@89", "notification_end",
  "notification_opt_stmt", "$@90", "$@91", "$@92", "deviation_stmt",
  "$@93", "$@94", "deviation_opt_stmt", "deviation_arg_str",
  "deviate_body_stmt", "deviate_stmts", "deviate_not_supported_stmt",
  "deviate_add_stmt", "$@95", "deviate_add_end", "deviate_add_opt_stmt",
  "deviate_delete_stmt", "$@96", "deviate_delete_end",
  "deviate_delete_opt_stmt", "deviate_replace_stmt", "$@97",
  "deviate_replace_end", "deviate_replace_opt_stmt", "when_stmt", "$@98",
  "when_end", "when_opt_stmt", "config_stmt", "config_read_stmt", "$@99",
  "$@100", "config_arg_str", "mandatory_stmt", "mandatory_read_stmt",
  "$@101", "$@102", "mandatory_arg_str", "presence_stmt",
  "min_elements_stmt", "min_value_arg_str", "max_elements_stmt",
  "max_value_arg_str", "ordered_by_stmt", "ordered_by_arg_str",
  "must_stmt", "$@103", "must_end", "unique_stmt", "unique_arg_str",
  "unique_arg", "key_stmt", "key_arg_str", "$@104", "key_opt", "$@105",
  "range_arg_str", "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@106",
  "path_arg_str", "$@107", "$@108", "absolute_path", "absolute_paths",
  "absolute_path_opt", "relative_path", "relative_path_part1",
  "relative_path_part1_opt", "descendant_path", "descendant_path_opt",
  "path_predicate", "path_equality_expr", "path_key_expr",
  "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "stmtsep", "unknown_statement",
  "$@109", "string_opt", "string_opt_part1", "string_opt_part2",
  "string_opt_part3", "unknown_statement_end", "unknown_statement2_opt",
  "unknown_statement2_end", "sep_stmt", "optsep", "sep", "whitespace_opt",
  "string", "$@110", "strings", "identifier", "identifiers",
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
     345,   346,   347,   348,   349,   350,    43,   123,   125,    59,
      47,    91,    93,    61,    40,    41
};
# endif

#define YYPACT_NINF -985

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-985)))

#define YYTABLE_NINF -588

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -985,    51,  -985,  -985,   201,  -985,  -985,  -985,   180,   180,
    -985,  -985,  2765,  2765,   180,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
     -10,   180,    23,   180,  -985,  -985,    13,   194,   194,  -985,
    -985,   182,  -985,  -985,     4,   235,   180,   180,   180,   180,
    -985,  -985,  -985,  -985,  -985,    69,  -985,  -985,   193,   120,
    -985,  1956,  2765,    18,   204,   204,   180,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   202,  -985,  -985,  -985,   202,  -985,  -985,  -985,   202,
     180,   180,  -985,  -985,    70,    70,  2765,   180,  1747,   180,
    -985,  -985,  -985,  -985,  -985,   180,  -985,  -985,  2765,  2765,
     180,   180,   180,   180,  -985,  -985,  -985,  -985,    64,    64,
    -985,  -985,   180,    33,  -985,    48,    29,   194,   180,   180,
    -985,  -985,  -985,  1956,  1956,  1956,  1956,   180,  -985,  1049,
    1138,    44,   226,  -985,  -985,  -985,    75,   240,   202,   202,
     202,   202,     7,   194,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,   272,   194,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  2765,    -5,  2765,
    2765,  2765,    -5,  2765,  2765,  2765,  2765,  2765,  2765,  2765,
    2765,  2765,  2305,   180,   194,   180,   234,  1747,   180,  -985,
     234,   331,   194,  -985,   253,  -985,  2857,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   110,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,   121,   180,  -985,   180,   146,  -985,   180,
    -985,  -985,  -985,   258,  -985,   157,   163,   180,   286,   289,
     292,   160,   180,   315,   343,   355,   170,   179,   185,   359,
     369,  -985,   372,   180,   180,   190,  -985,   224,   329,  -985,
     194,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,   194,  -985,  -985,  -985,  -985,   180,
     275,  -985,  -985,  -985,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   631,   194,
     194,  -985,    84,   242,   670,  1195,   368,   327,   102,    41,
      12,   660,   400,  1433,  1266,  1378,    76,   722,   180,   180,
     180,  -985,  -985,  -985,   281,   276,  -985,  -985,   303,   194,
    -985,  -985,  -985,  -985,   180,   180,   180,   180,   180,   180,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   180,
    -985,  -985,  -985,  -985,  -985,  -985,   298,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   335,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   180,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   180,  -985,
    -985,  -985,  -985,  -985,   180,  -985,  -985,   336,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
     180,  -985,  -985,  -985,   339,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,   357,   276,  -985,  -985,  -985,  -985,   180,   180,
     180,  -985,  -985,  -985,  -985,  -985,   361,   276,  -985,  -985,
    -985,  -985,  -985,  -985,   180,   180,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,   386,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  1956,    61,  1956,  -985,   180,  -985,   180,   180,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  2305,  -985,
    1956,  -985,  1956,  -985,  -985,  2765,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  1956,  -985,  -985,  -985,  -985,  -985,  2765,
     526,  -985,   194,   194,   194,  2305,  -985,  -985,  -985,  -985,
    -985,     6,   196,    -2,  -985,  -985,  -985,  -985,  2397,  2397,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   180,
     180,  -985,  -985,  -985,  -985,   202,  -985,  -985,  -985,  -985,
     202,   202,  2305,   194,  2397,  2397,  -985,  -985,  -985,    37,
     202,    52,  -985,    61,  -985,   194,   194,  -985,   194,   194,
     194,   194,   194,   202,   194,   194,   194,   194,   194,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
     202,  -985,   194,   194,  -985,  -985,  -985,   202,  -985,  -985,
    -985,   202,  -985,  -985,  -985,  -985,  -985,   202,  -985,   194,
     194,   202,  -985,  -985,   202,  -985,    84,  -985,   194,   194,
     194,   194,   194,   194,   194,   194,   316,   330,   194,   194,
     194,   194,  -985,   180,   180,   180,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   194,   194,   194,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   381,  -985,   420,
     448,  -985,   460,   180,   180,    84,   180,   180,  -985,   194,
     180,  -985,   180,  -985,   180,   180,   180,  -985,   194,  -985,
      84,  -985,  -985,  -985,  2857,  -985,  -985,  -985,   507,   409,
     180,   519,   180,   180,   180,   202,   180,   180,   202,  -985,
    -985,  -985,   202,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,   524,   531,  -985,   555,  -985,  -985,  2857,    84,
     163,   194,   194,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,   194,  -985,   266,   194,   191,   194,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   403,
     403,   723,   194,   194,   313,  -985,  -985,  -985,   792,   180,
     404,   194,   194,   194,    84,  -985,  -985,  -985,   407,   414,
     180,   180,  -985,   415,   408,  -985,   486,  -985,  -985,  -985,
     508,  1195,   937,   180,   180,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,    73,  -985,  1056,
     138,   251,  -985,  -985,  -985,  -985,  -985,  -985,    57,    74,
    -985,   180,   180,   180,   180,   276,  -985,  -985,  -985,  -985,
     180,  -985,   180,  -985,   437,   180,   180,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  1956,  1956,  -985,
    -985,  -985,  -985,  -985,   202,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   194,   194,   194,
    -985,   202,   440,   527,  -985,  -985,  -985,   202,    60,  1956,
    1956,  1956,  -985,   194,  1956,   194,  2765,   508,  -985,    37,
      52,   194,   202,   202,   194,   194,   180,   180,  -985,   194,
     194,   194,  -985,  2857,  -985,  -985,   453,  -985,  -985,   180,
     180,  -985,  -985,   202,  -985,   556,  -985,   566,  -985,   575,
    -985,   194,   606,  -985,   486,   617,  -985,  -985,   202,   202,
    -985,  -985,  -985,   440,  -985,  2489,  -985,   180,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,   457,  -985,  -985,  -985,
       3,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     560,  -985,   261,   405,   529,   533,   350,   559,  -985,  2857,
    -985,  -985,  -985,  -985,   180,  -985,  -985,  -985,  -985,  -985,
     180,  -985,  -985,  -985,  -985,  -985,   560,   560,    45,   196,
     459,   465,  -985,  -985,  -985,  -985,   202,  -985,  -985,  -985,
     202,  -985,  -985,   560,  -985,   180,  -985,   180,   506,   560,
    -985,   560,   503,   520,   560,   560,   528,   594,  -985,   560,
    -985,  -985,   525,  2581,   560,  -985,  -985,  -985,  2673,  -985,
     567,   560,  2857,  -985
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     636,     0,     2,     3,     0,     1,   634,   635,     0,     0,
     637,   636,     0,     0,   638,   652,     4,   651,   653,   654,
     655,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     685,   686,   687,   688,   689,   690,   691,   692,   693,   694,
     695,   696,   697,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   713,   714,
     715,   716,   717,   718,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   608,     9,   729,   636,    16,   636,
       0,   607,     0,     6,   616,   616,     5,    12,    19,   636,
     619,    10,   618,   617,    17,     0,   622,     0,     0,     0,
      28,    13,    14,    15,    28,     0,    20,     7,     0,   624,
     623,     0,     0,     0,    47,    47,     0,    22,   636,   636,
     628,   620,   636,   621,   644,   647,   645,   649,   650,   648,
     643,     0,   641,   646,   605,     0,   636,    24,    26,     0,
       0,     0,    29,    30,    56,    56,     0,     8,   630,   626,
     616,   616,    27,   636,    46,   606,   636,    23,     0,     0,
       0,     0,     0,     0,    48,    49,    50,    51,    69,    69,
      43,   610,   622,     0,   609,   625,     0,   614,   642,    25,
      31,    34,    35,     0,     0,     0,     0,     0,   616,     0,
       0,     0,     0,   629,   636,   616,     0,     0,     0,     0,
       0,     0,     0,    57,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     636,   616,    71,    72,    73,    74,    75,    76,   226,   227,
     228,   229,   230,   231,   232,    77,    78,    79,    80,   636,
     616,   636,   636,   631,     0,   615,   616,   616,    37,   616,
      53,    54,    52,    55,    66,    68,    58,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    70,    18,     0,   630,   632,   636,
       0,    39,    36,   636,     0,   354,     0,   420,   405,   567,
     636,   317,   233,    81,   474,   466,   636,   106,   215,   116,
     254,   273,   293,   451,   421,     0,   129,   730,   613,   369,
     636,   636,    44,     0,   627,    32,     0,     0,    40,    67,
     616,    60,    59,     0,   565,     0,   566,   419,     0,     0,
       0,     0,   473,     0,     0,     0,     0,     0,     0,     0,
       0,   616,     0,   611,   612,     0,   636,    39,     0,    38,
      61,   616,   356,   355,   616,   568,   616,   319,   318,   616,
     235,   234,   616,    83,    82,   616,   616,   108,   107,   616,
     217,   216,   616,   118,   117,   616,   616,   616,   616,   453,
     452,   616,   423,   422,   132,   616,   371,   370,   616,   633,
       0,    42,   636,    63,   358,   408,   322,   237,    85,   469,
     110,   219,   121,   257,   276,   296,   455,   425,   127,   373,
      45,   616,     0,     0,     0,   406,   320,     0,     0,   467,
       0,     0,   119,   255,   274,   294,     0,     0,     0,     0,
       0,   138,   139,   137,     0,     0,   135,   136,     0,    33,
      41,    62,    64,    65,     0,     0,     0,     0,     0,     0,
     357,   367,   368,   366,   361,   359,   364,   365,   362,     0,
     413,   414,   412,   411,   415,   417,     0,   409,   330,   331,
     329,   325,   326,   336,   337,   338,   339,     0,   332,   334,
     335,   340,   323,   327,   328,     0,   236,   246,   247,   245,
     240,   250,   248,   252,   238,   244,   243,   241,     0,    84,
      88,    89,    86,    87,     0,   470,   471,     0,   109,   113,
     114,   112,   111,   218,   221,   222,   220,   616,   616,   616,
       0,   124,   125,   123,     0,   122,   271,   272,   270,   260,
     264,   267,     0,     0,   258,   268,   269,   265,     0,     0,
       0,   291,   292,   290,   279,   283,     0,     0,   277,   286,
     287,   288,   289,   284,     0,     0,   309,   310,   308,   299,
     311,   313,   315,     0,   297,   304,   305,   306,   307,   300,
     303,   302,   454,   458,   459,   457,   456,   460,   462,   464,
     636,   636,   424,   428,   429,   427,   426,   430,   432,   434,
     436,     0,     0,     0,   128,     0,   616,     0,     0,   372,
     378,   379,   377,   376,   380,   382,   374,   523,     0,   530,
       0,    99,     0,   616,   616,     0,   616,   616,   407,   616,
     321,   616,   616,     0,   616,   616,   616,   616,   616,     0,
       0,   468,   224,   223,   225,     0,   120,   256,   262,   616,
     616,     0,     0,     0,   275,   281,   616,   616,     0,     0,
     616,   616,   616,   295,   616,   616,   616,   616,   616,   438,
     448,   616,   616,   616,   616,     0,   636,   636,   636,   105,
       0,     0,     0,   134,     0,     0,   616,   616,   616,     0,
       0,     0,   548,     0,   515,   360,   363,   341,   416,   418,
     410,   333,   324,     0,   251,   249,   253,   239,   242,    90,
     636,   636,   636,   636,   472,   636,   475,   477,   479,   478,
       0,   616,   259,   266,   598,   636,   543,     0,   636,   599,
     539,     0,   600,   636,   636,   636,   547,     0,   616,   278,
     285,     0,   557,   558,     0,   552,     0,   569,   312,   314,
     316,   298,   301,   461,   463,   465,     0,     0,   431,   433,
     435,   437,   214,   102,   104,   103,    98,   213,   130,   404,
     400,   636,   389,   384,   636,   381,   383,   375,   636,   636,
     528,   524,   115,   636,   636,   535,   531,     0,   100,     0,
       0,   536,     0,   481,   494,     0,   503,   476,   126,   263,
     541,   540,   542,   537,   538,   546,   545,   544,   282,   560,
       0,   554,   553,   556,     0,   567,   616,   616,     0,     0,
     403,     0,   388,   527,   526,     0,   534,   533,     0,   616,
     550,   549,     0,   519,   517,   516,   616,   343,   342,   616,
      92,   616,     0,     0,   480,     0,   559,   563,     0,     0,
     570,   441,   441,   616,   140,   131,   616,   616,   386,   385,
     525,   532,   162,   101,     0,   345,     0,    91,   616,   483,
     482,   616,   496,   495,   616,   505,   504,   561,   555,   439,
     449,   148,   408,   390,     0,   518,   520,   521,     0,     0,
       0,   485,   498,   507,     0,   442,   444,   446,     0,     0,
       0,     0,   145,     0,   142,   143,     0,   144,   146,   147,
       0,   401,     0,     0,     0,   551,   165,   166,   163,   164,
     344,   350,   351,   349,   348,   352,   346,     0,    93,     0,
       0,     0,   562,   616,   616,   616,   440,   450,   571,     0,
     141,     0,     0,     0,     0,     0,   151,   149,   150,   616,
       0,   616,     0,   197,     0,     0,     0,   387,   398,   399,
     393,   394,   395,   392,   396,   397,   616,     0,     0,   616,
     616,   636,   636,    97,     0,   484,   486,   489,   490,   491,
     492,   493,   616,   488,   497,   499,   502,   616,   501,   506,
     616,   509,   510,   511,   512,   513,   514,   443,   445,   447,
     575,     0,     0,     0,   636,   636,   194,     0,     0,     0,
       0,     0,   616,   152,     0,   173,     0,   196,   402,     0,
       0,   391,     0,     0,   353,   347,    96,    95,    94,   487,
     500,   508,   190,     0,   578,   572,     0,   574,   582,   193,
     192,   191,   157,     0,   636,     0,   159,     0,   168,     0,
     564,   153,     0,   176,   172,     0,   200,   198,     0,     0,
     212,   211,   587,   577,   581,     0,   155,   156,   616,   160,
     616,   616,   169,   616,   616,   188,   187,   616,   177,   175,
     616,   616,   201,   616,   522,   529,   576,   579,   583,   580,
     585,   162,   158,   162,   167,   162,   179,   174,   203,   199,
     639,   584,     0,     0,     0,     0,     0,     0,   640,     0,
     586,   161,   170,   189,     0,   178,   182,   183,   181,   180,
       0,   202,   206,   207,   205,   204,   639,   639,     0,     0,
       0,     0,   603,   604,   601,   186,     0,   636,   602,   210,
       0,   636,   588,   639,   184,   185,   208,   209,     0,   639,
     589,   639,     0,     0,   639,   639,     0,     0,   597,   639,
     590,   593,     0,     0,   639,   594,   595,   592,   639,   591,
       0,   639,     0,   596
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -985,  -985,   718,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   510,  -985,  -985,  -985,   505,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   306,  -985,  -985,
    -985,  -985,  -985,  -116,   521,  -985,  -985,    53,   141,   494,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   489,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -421,  -175,
    -985,  -985,   -22,  -985,  -985,  -985,  -985,  -388,  -985,  -985,
    -985,  -985,  -985,  -209,  -287,  -985,  -985,  -551,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -984,  -985,  -985,  -985,  -985,  -985,  -985,  -377,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -339,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -402,
    -418,  -255,  -985,  -985,  -985,  -413,   257,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   279,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,   290,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   302,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   308,
    -985,  -985,  -985,  -985,  -985,   310,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -190,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -108,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -370,  -985,  -985,  -985,  -515,   106,  -985,  -985,  -273,  -210,
    -360,  -985,  -985,  -271,  -165,  -415,  -985,  -395,  -985,   318,
    -985,  -406,  -985,  -985,  -574,  -985,   -99,  -985,  -985,  -985,
    -143,  -985,  -985,  -345,   483,   -59,  -682,  -985,  -985,  -985,
    -985,  -301,  -338,  -985,  -985,  -302,  -985,  -985,  -985,  -323,
    -985,  -985,  -985,  -394,  -985,  -985,  -985,  -650,  -359,  -985,
    -985,  -985,    31,  -159,  -608,   450,  1227,  -985,  -985,   591,
    -985,  -985,  -985,  -985,   490,  -985,    -4,    -3,   412,    66,
      47,  -985,  -985,    81,  -103,  -985
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    94,    99,   106,   138,     2,   100,   120,   111,
       3,   102,   124,   114,   125,   121,   159,   176,   122,   134,
     162,   216,   367,   200,   163,   217,   269,   337,   338,   412,
     137,   211,   365,   123,   164,   184,   185,   926,   927,   188,
     208,   304,   342,   413,   433,   276,   303,   209,   241,   242,
     350,   384,   438,   522,   802,   851,   900,   984,   482,   473,
     703,   842,   690,   243,   353,   388,   440,   483,   244,   355,
     394,   544,   442,   545,   245,   454,   325,   616,   828,   428,
     455,   865,   913,   914,   955,   956,  1053,   957,  1055,  1080,
     894,   958,  1057,  1083,   915,   916,  1064,   961,  1062,  1089,
    1116,  1129,  1146,   959,  1086,   917,   918,  1017,   919,   920,
    1027,   963,  1065,  1093,  1117,  1135,  1150,   928,   929,   456,
     457,   246,   354,   391,   441,   247,   248,   349,   381,   437,
     647,   648,   645,   644,   646,   249,   356,   552,   443,   659,
     553,   731,   660,   250,   357,   566,   444,   666,   567,   748,
     667,   251,   358,   583,   445,   674,   675,   670,   671,   672,
     252,   348,   378,   497,   436,   642,   641,   498,   499,   485,
     800,   848,   898,   980,   979,   253,   343,   373,   434,   633,
     634,   254,   362,   407,   458,   698,   696,   697,   624,   831,
     869,   783,   922,   625,   829,   964,   780,   255,   345,   486,
     435,   639,   636,   637,   308,   256,   360,   403,   447,   681,
     682,   683,   684,   609,   766,   908,   889,   943,   944,   945,
     610,   767,   909,   257,   359,   400,   446,   676,   677,   678,
     258,   351,   527,   439,   315,   724,   725,   726,   727,   852,
     880,   939,   728,   853,   883,   940,   729,   855,   886,   941,
     487,   799,   845,   874,   971,   476,   699,   835,   791,   972,
     477,   701,   838,   796,   516,   570,   741,   571,   737,   572,
     747,   478,   797,   841,   590,   755,   822,   591,   752,   820,
     856,   904,  1059,   309,   310,   346,   756,   825,  1011,  1012,
    1013,  1044,  1045,  1073,  1047,  1048,  1075,  1099,  1111,  1096,
    1136,  1160,  1170,  1171,  1173,  1178,  1161,   742,   743,  1147,
    1148,   155,    95,   757,   329,   823,   107,   112,   116,   128,
     129,   143,   195,   141,   193,   263,   113,     4,   130,  1119,
     151,   173,   152,    96,    97,   331
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,   375,   658,    16,    11,    11,    16,   453,    14,   192,
      10,   738,   781,   784,    16,    16,   665,   523,   492,   274,
     700,   543,   484,   734,   513,   551,    16,   -21,   539,   156,
     586,   517,   582,   599,     6,   157,     7,   557,   573,   589,
     110,   550,   565,   181,    98,    16,   474,   730,   491,   510,
     587,     5,   532,    16,   465,   549,   564,   579,   596,   606,
      16,  1142,  1143,  1144,   475,    16,   502,   514,    16,    16,
     623,  -573,   181,   554,   568,   584,   504,   734,   524,   183,
     224,    16,    16,   556,   778,   468,   119,   104,   626,     6,
     744,     7,   136,   745,   101,   306,   103,    10,   180,    10,
     735,   181,   226,  -587,  -587,   227,   115,   181,   183,   109,
     528,    10,    11,    11,    11,    11,   231,  1113,   465,  1114,
     105,  1115,   518,   233,   234,   788,   235,   215,   142,   182,
     789,   213,    11,   181,   207,   167,   168,   183,   192,   169,
     793,   260,   859,   183,   214,   794,   686,   344,   687,   468,
     511,   688,   238,   175,   537,   239,    11,    11,   580,   597,
     607,   981,  1014,    10,    10,    10,   982,  1015,   448,   183,
     198,    10,   266,   199,   592,   449,    11,    11,    11,    11,
     332,   170,   512,   171,   335,     6,   538,     7,    11,   330,
     581,   598,   608,   467,    10,    10,     6,   190,     7,     6,
     519,     7,   110,    11,    16,   110,     6,   361,     7,   201,
     202,   264,   153,   734,   739,   575,   450,   186,   186,   366,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,   994,   293,   117,     6,
       6,     7,     7,   127,   369,   110,   118,   160,   161,   194,
     218,   219,   220,   221,   374,     8,   295,   385,   297,   298,
      10,   490,   509,   306,   119,   531,   536,   395,   548,   563,
     578,   595,   605,   181,   899,     9,   396,     6,   965,     7,
     299,   448,   397,   622,   153,   153,   153,   153,   408,    10,
     139,    10,   140,    10,    10,   336,   334,   181,   118,   170,
     339,   171,   966,   558,   559,   187,   187,   347,   305,   183,
     311,   312,   313,   352,   317,   318,   319,   320,   321,   322,
     323,   324,   326,   261,   617,   262,   615,   363,   364,   450,
      10,   224,    11,   183,   181,    10,     6,   267,     7,   268,
     461,   411,   110,    10,   181,   465,   923,   924,    10,   999,
     340,   615,   341,   226,   464,   371,   227,   372,   181,    10,
      10,  1043,  1110,   409,   895,   993,   998,   231,  1054,   465,
     183,   618,   224,   431,   233,   234,   468,   235,   194,   614,
     183,   181,   467,   376,   469,   377,   379,   194,   380,   382,
    1000,   383,   505,   479,   183,   464,   638,   227,   448,   181,
     468,   619,   336,   238,  1022,    10,   239,   224,   469,   432,
     465,   925,   386,   826,   387,   233,   234,   183,   235,   466,
      12,    13,   540,   449,   988,   506,  1003,   827,    10,   226,
    1124,   181,   227,   640,   651,   183,   181,   656,   923,   924,
     389,   468,   390,   231,    11,    11,    11,   951,  1125,   469,
     233,   234,   392,   235,   393,   657,   398,   952,   399,   664,
      11,    11,    11,    11,    11,    11,   401,   183,   402,   405,
     953,   406,   183,   449,   954,    11,   907,   907,   839,   238,
     840,   451,   239,  -154,   673,   935,   462,   471,   480,   488,
     507,   520,   525,   529,   534,   541,   546,   561,   576,   593,
     603,    11,   938,  1121,   970,   946,   866,   974,   484,   753,
     934,   620,   947,   950,    11,   375,   976,   843,   960,   844,
      11,   987,   996,  1002,   990,   330,  1005,   975,   936,   131,
     132,   133,   962,   992,   997,  1028,    11,   986,   995,  1001,
    1043,  1046,   503,   515,   991,   846,  1006,   847,   166,   555,
     569,   585,   330,  1074,    11,    11,    11,   849,  1110,   850,
     181,  1152,   923,   924,   181,  1118,   923,   924,  1153,   452,
      11,    11,   178,   179,   463,   472,   481,   489,   508,   521,
     526,   530,   535,   542,   547,   562,   577,   594,   604,   330,
     181,  1159,   203,   204,   205,   206,   183,   679,   680,   621,
     183,   172,   905,   905,   863,   174,   864,  1164,  1169,   177,
     720,    11,   721,    11,    11,   722,   867,   723,   868,   222,
    1165,   878,  1130,   879,   126,  1174,   183,  1122,   881,   135,
     882,  1123,   449,  1168,   906,   906,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   884,  1078,   885,  1079,   165,  1131,   685,   189,
     691,   448,   181,  1081,   224,  1082,   707,  1181,   270,   271,
     272,   273,  1084,   410,  1085,    10,    10,   702,   210,   704,
     719,   798,   912,   773,   774,   775,   226,  1090,  1067,   227,
     713,   181,   153,   493,   153,  1128,  1134,   464,   183,   887,
     231,   181,   921,  1087,   449,  1088,  -133,   233,   234,   450,
     235,   153,   465,   153,  1091,   494,  1092,   803,   804,   805,
     806,   466,   807,   933,   153,   467,   495,   183,     6,   989,
       7,  1004,   810,   468,   110,   812,   238,   183,   496,   239,
     814,   815,   816,   468,   500,   540,   501,  -195,   368,   194,
     194,   469,    11,   181,   890,  -171,  1068,   973,   533,  1069,
     888,   942,   231,   588,   465,   316,   860,   600,   470,    10,
      10,    10,  1097,  1098,  1120,   194,   194,  1112,   830,  1175,
    1151,   832,   601,   212,   910,   833,   834,   333,     0,   183,
     836,   837,   911,     0,     0,   468,   224,     0,   238,    10,
      10,    10,    10,    10,     0,     0,    10,     0,    10,     0,
      10,    10,    10,     0,     0,     0,    11,     0,   226,     0,
     602,   227,     0,   181,     0,     0,    10,     0,    10,    10,
      10,     0,    10,    10,   465,     0,     0,     0,     0,   233,
     234,     0,   235,     0,     0,     0,     0,     0,     0,   150,
     154,   158,     0,     0,     0,    11,     0,     0,     0,   183,
     611,   612,   613,     0,     0,   468,     0,     0,     0,     0,
       0,   239,     0,   469,     0,     0,   627,   628,   629,   630,
     631,   632,   460,     0,  1072,     0,     0,     0,     0,     0,
     930,   635,     0,     0,     0,    11,     0,     0,     0,     0,
      11,     0,     0,     0,     0,   194,    11,    11,     0,     0,
       0,     0,     0,     0,     0,     0,  1100,   643,     0,    11,
      11,   150,   150,   150,   150,     0,     0,   896,     0,     0,
     649,     0,     0,     0,     0,     0,   650,     0,     0,   194,
     275,     0,     0,     0,     0,     0,     0,    11,    11,    11,
      11,   931,   655,     0,     0,     0,    11,     0,    11,     0,
    1137,    11,    11,     0,   965,     0,     0,   448,   181,     0,
     661,   662,   663,     0,   480,   968,     0,     0,  1036,  1037,
       0,     0,     0,     0,     0,     0,   668,   669,   966,   558,
     559,     0,   467,     0,     0,     0,   307,     0,     0,     0,
     314,     0,   505,     0,   183,     0,     0,     0,     0,     0,
     328,  1049,  1050,     0,  1176,   897,     0,     0,     0,  1179,
       0,     0,     0,  1183,  1032,  1033,     0,   692,     0,   694,
     695,     0,    10,    10,     0,   967,     0,     0,     0,   932,
       0,     0,     0,     0,     0,    10,    10,     0,     0,     0,
       0,  1077,     0,   224,     0,     0,     0,  1066,   153,   153,
       0,     0,   481,   969,     0,     0,  1056,  1058,  1060,     0,
     225,  1063,     0,    10,     0,   226,     0,     0,   227,     0,
       0,     0,     0,   965,   228,   229,   448,   230,     0,   231,
     232,     0,     0,     0,     0,     0,   233,   234,     0,   235,
     153,   153,   153,     0,     0,   153,   236,   966,   558,   559,
       0,   467,     0,     0,     0,     0,     0,     0,     0,     0,
      11,   237,     0,     0,   194,   238,    11,     0,   239,     0,
       0,     0,     0,   575,   450,   772,     0,     0,     0,     0,
     776,   777,   224,     0,  1155,     0,     0,   240,  1157,     0,
     792,    10,     0,    10,   985,     0,   194,     0,     0,   225,
       0,     0,     0,   801,   226,     0,     0,   227,   824,  1126,
    1132,     0,     0,   228,   229,     0,   230,     0,   231,   232,
     808,     0,     0,     0,     0,   233,   234,   811,   235,     0,
       0,   813,     0,     0,     0,   236,     0,   817,     0,   224,
     194,   819,  1140,  1141,   821,     0,     0,     0,     0,     0,
     237,     0,     0,     0,   238,     0,     0,   239,     0,  1158,
     479,   226,     0,     0,   227,  1162,   181,  1163,     0,     0,
    1166,  1167,   858,     0,     0,  1172,   259,   465,     0,     0,
    1177,     0,   233,   234,  1180,   235,     0,  1182,     0,     0,
       0,     0,     0,     0,   194,   854,     0,  1127,  1133,   194,
       0,     0,   183,   194,     0,     0,     0,     0,   449,     0,
     857,   824,     0,     0,   239,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,   870,     0,     0,   871,     0,
       0,     0,   873,   464,     0,     0,     0,   181,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   465,     0,
       0,   937,     0,     0,     0,     0,   858,     0,   558,   559,
       0,   467,   948,   949,   560,     0,     0,     0,     0,   150,
     689,   150,   108,   183,     0,   977,   978,     0,     0,   468,
       0,  -280,     0,     0,   450,     0,   328,   469,   150,     0,
     150,     0,     0,     0,   857,     0,     0,     0,     0,     0,
       0,   150,     0,  1018,  1019,  1020,  1021,     0,     0,     0,
       0,     0,  1024,   328,  1026,     0,     0,  1029,  1030,   736,
     740,   746,   224,     0,     0,     0,   751,   754,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   196,   197,     0,
       0,     0,     0,     0,   226,   464,     0,   227,     0,   181,
     328,     0,   779,   782,     0,     0,     0,   790,   231,   795,
     465,   689,     0,     0,   574,   233,   234,     0,   235,     0,
     558,   559,     0,   467,  1038,   223,   560,     0,     0,     0,
       0,     0,   265,     0,     0,   183,     0,     0,     0,     0,
       0,   468,     0,     0,   238,   575,     0,   239,     0,   469,
     464,  1042,     0,   448,   181,     0,     0,  1051,   294,     0,
       0,     0,     0,     0,     0,   465,     0,     0,     0,     0,
       0,     0,  1070,  1071,   466,     0,     0,   296,   467,     0,
       0,     0,     0,   300,   301,     0,   302,     0,     0,     0,
     183,     0,     0,  1076,     0,     0,   468,     0,  -261,     0,
       0,   450,     0,     0,   469,     0,     0,     0,  1094,  1095,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1138,     0,     0,     0,
       0,     0,  1139,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   404,     0,
       0,     0,     0,     0,     0,     0,  1154,     0,   414,     0,
    1156,   415,     0,   416,     0,     0,   417,     0,     0,   418,
       0,     0,   419,   420,     0,     0,   421,     0,     0,   422,
       0,     0,   423,   424,   425,   426,     0,     0,   427,     0,
       0,     0,   429,     0,     0,   430,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   983,     0,     0,   459,     0,
       0,     0,     0,     0,     0,     0,  1010,  1016,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   150,   150,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1052,   150,   150,   150,
       0,     0,   150,     0,     0,     0,     0,   790,   795,     0,
       0,    15,     6,     0,     7,     0,     0,    17,   191,     0,
       0,     0,     0,     0,   652,   653,   654,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,   693,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1145,  1149,     0,     0,
     705,   706,     0,   708,   709,     0,   710,     0,   711,   712,
       0,   714,   715,   716,   717,   718,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   732,   733,     0,     0,
       0,     0,     0,   749,   750,     0,     0,   758,   759,   760,
       0,   761,   762,   763,   764,   765,     0,     0,   768,   769,
     770,   771,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   785,   786,   787,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   809,     0,
      15,     0,     0,     0,    16,   144,    17,   145,   146,     0,
       0,     0,   147,   148,   149,   818,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,     0,   861,   862,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   872,     0,     0,     0,
       0,     0,     0,   875,     0,     0,   876,     0,   877,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     891,     0,     0,   892,   893,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   901,     0,     0,   902,     0,
       0,   903,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1007,  1008,  1009,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1023,     0,  1025,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1031,     0,     0,  1034,  1035,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1039,
       0,     0,     0,     0,  1040,     0,     0,  1041,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1061,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1101,     0,  1102,  1103,    15,
    1104,  1105,     0,    16,  1106,    17,   327,  1107,  1108,     0,
    1109,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    15,     0,     0,     0,    16,     0,    17,   191,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    15,     0,     0,     0,     0,     0,    17,
     191,     0,     0,  1046,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    15,     0,     0,     0,     0,
       0,    17,   191,     0,     0,  1169,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    15,  1118,     0,
       0,     0,     0,    17,   191,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    15,
       0,     0,     0,    16,     0,    17,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    15,     0,     0,     0,     0,     0,    17,   191,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93
};

static const yytype_int16 yycheck[] =
{
       4,   346,   553,     8,     8,     9,     8,   428,    11,   168,
      14,   661,   694,   695,     8,     8,   567,   438,   436,    12,
     628,   442,   435,    17,   437,   443,     8,    23,   441,   132,
     445,   437,   445,   446,     5,    17,     7,   443,   444,   445,
      11,   443,   444,    31,    13,     8,   434,   655,   436,   437,
     445,     0,   440,     8,    42,   443,   444,   445,   446,   447,
       8,    16,    17,    18,   434,     8,   436,   437,     8,     8,
     458,    14,    31,   443,   444,   445,   436,    17,    37,    67,
       4,     8,     8,   443,   692,    73,    82,    97,   458,     5,
      92,     7,    23,    95,    97,   100,    99,   101,    28,   103,
      94,    31,    26,   100,   101,    29,   109,    31,    67,    96,
      98,   115,   116,   117,   118,   119,    40,  1101,    42,  1103,
      97,  1105,    20,    47,    48,    88,    50,    98,     8,    59,
      93,    98,   136,    31,    70,   138,   139,    67,   297,   142,
      88,    97,   824,    67,    96,    93,    85,   306,    87,    73,
     437,    90,    76,   156,   441,    79,   160,   161,   445,   446,
     447,    88,    88,   167,   168,   169,    93,    93,    30,    67,
     173,   175,    97,   176,    98,    73,   180,   181,   182,   183,
     296,    97,   437,    99,   300,     5,   441,     7,   192,   292,
     445,   446,   447,    55,   198,   199,     5,   166,     7,     5,
      98,     7,    11,   207,     8,    11,     5,    97,     7,   178,
     179,   214,   131,    17,    18,    77,    78,   164,   165,    98,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,    98,   240,    56,     5,
       5,     7,     7,     8,    98,    11,    64,    43,    44,   168,
     203,   204,   205,   206,    97,    54,   259,    97,   261,   262,
     264,   436,   437,   100,    82,   440,   441,    97,   443,   444,
     445,   446,   447,    31,    83,    74,    97,     5,    27,     7,
       8,    30,    97,   458,   203,   204,   205,   206,    98,   293,
      97,   295,    99,   297,   298,    71,   299,    31,    64,    97,
     303,    99,    51,    52,    53,   164,   165,   310,   277,    67,
     279,   280,   281,   316,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    97,    21,    99,    75,   330,   331,    78,
     334,     4,   336,    67,    31,   339,     5,    97,     7,    99,
      98,    12,    11,   347,    31,    42,    33,    34,   352,    98,
      97,    75,    99,    26,    27,    97,    29,    99,    31,   363,
     364,   100,   101,   366,    98,   939,   940,    40,  1018,    42,
      67,    68,     4,    98,    47,    48,    73,    50,   297,    98,
      67,    31,    55,    97,    81,    99,    97,   306,    99,    97,
     941,    99,    65,    25,    67,    27,    98,    29,    30,    31,
      73,    98,    71,    76,   955,   409,    79,     4,    81,   412,
      42,    98,    97,    97,    99,    47,    48,    67,    50,    51,
       8,     9,    22,    73,   939,    98,   941,    97,   432,    26,
      80,    31,    29,    98,    98,    67,    31,    98,    33,    34,
      97,    73,    99,    40,   448,   449,   450,    39,    98,    81,
      47,    48,    97,    50,    99,    98,    97,    49,    99,    98,
     464,   465,   466,   467,   468,   469,    97,    67,    99,    97,
      62,    99,    67,    73,    66,   479,   889,   890,    97,    76,
      99,   428,    79,    75,    98,   898,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   505,    98,    98,   922,    98,    97,   922,   921,   668,
     898,   458,    98,    98,   518,   860,   922,    97,    32,    99,
     524,   939,   940,   941,   939,   628,   941,   922,   898,   117,
     118,   119,    24,   939,   940,    98,   540,   939,   940,   941,
     100,    14,   436,   437,   939,    97,   941,    99,   136,   443,
     444,   445,   655,   100,   558,   559,   560,    97,   101,    99,
      31,   102,    33,    34,    31,     5,    33,    34,   103,   428,
     574,   575,   160,   161,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   692,
      31,    85,   180,   181,   182,   183,    67,   600,   601,   458,
      67,   151,   889,   890,    97,   155,    99,   104,    14,   159,
      84,   615,    86,   617,   618,    89,    97,    91,    99,   207,
     100,    97,    63,    99,   114,   100,    67,    98,    97,   124,
      99,    98,    73,   105,   889,   890,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,    97,    97,    99,    99,   135,    98,   611,   165,
     613,    30,    31,    97,     4,    99,   635,   100,   218,   219,
     220,   221,    97,   367,    99,   679,   680,   630,   189,   632,
     649,   703,   891,   686,   687,   688,    26,  1064,  1027,    29,
     643,    31,   611,   436,   613,  1116,  1117,    27,    67,   858,
      40,    31,   892,    97,    73,    99,    75,    47,    48,    78,
      50,   630,    42,   632,    97,   436,    99,   720,   721,   722,
     723,    51,   725,   898,   643,    55,   436,    67,     5,   939,
       7,   941,   735,    73,    11,   738,    76,    67,   436,    79,
     743,   744,   745,    73,   436,    22,   436,    24,   336,   668,
     669,    81,   756,    31,   862,    32,  1029,   922,    98,  1030,
     859,   904,    40,   445,    42,   282,   825,    45,    98,   773,
     774,   775,  1073,  1075,  1112,   694,   695,  1100,   781,  1173,
    1139,   784,    60,   192,    61,   788,   789,   297,    -1,    67,
     793,   794,    69,    -1,    -1,    73,     4,    -1,    76,   803,
     804,   805,   806,   807,    -1,    -1,   810,    -1,   812,    -1,
     814,   815,   816,    -1,    -1,    -1,   820,    -1,    26,    -1,
      98,    29,    -1,    31,    -1,    -1,   830,    -1,   832,   833,
     834,    -1,   836,   837,    42,    -1,    -1,    -1,    -1,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   859,    -1,    -1,    -1,    67,
     448,   449,   450,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    81,    -1,    -1,   464,   465,   466,   467,
     468,   469,   432,    -1,  1043,    -1,    -1,    -1,    -1,    -1,
      98,   479,    -1,    -1,    -1,   899,    -1,    -1,    -1,    -1,
     904,    -1,    -1,    -1,    -1,   824,   910,   911,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1075,   505,    -1,   923,
     924,   203,   204,   205,   206,    -1,    -1,   874,    -1,    -1,
     518,    -1,    -1,    -1,    -1,    -1,   524,    -1,    -1,   858,
     222,    -1,    -1,    -1,    -1,    -1,    -1,   951,   952,   953,
     954,   898,   540,    -1,    -1,    -1,   960,    -1,   962,    -1,
    1119,   965,   966,    -1,    27,    -1,    -1,    30,    31,    -1,
     558,   559,   560,    -1,   921,   922,    -1,    -1,   981,   982,
      -1,    -1,    -1,    -1,    -1,    -1,   574,   575,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,   278,    -1,    -1,    -1,
     282,    -1,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,
     292,  1014,  1015,    -1,  1173,   874,    -1,    -1,    -1,  1178,
      -1,    -1,    -1,  1182,   977,   978,    -1,   615,    -1,   617,
     618,    -1,  1036,  1037,    -1,    98,    -1,    -1,    -1,   898,
      -1,    -1,    -1,    -1,    -1,  1049,  1050,    -1,    -1,    -1,
      -1,  1054,    -1,     4,    -1,    -1,    -1,  1026,   977,   978,
      -1,    -1,   921,   922,    -1,    -1,  1019,  1020,  1021,    -1,
      21,  1024,    -1,  1077,    -1,    26,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    27,    35,    36,    30,    38,    -1,    40,
      41,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    50,
    1019,  1020,  1021,    -1,    -1,  1024,    57,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1124,    72,    -1,    -1,  1043,    76,  1130,    -1,    79,    -1,
      -1,    -1,    -1,    77,    78,   685,    -1,    -1,    -1,    -1,
     690,   691,     4,    -1,  1147,    -1,    -1,    98,  1151,    -1,
     700,  1155,    -1,  1157,    98,    -1,  1075,    -1,    -1,    21,
      -1,    -1,    -1,   713,    26,    -1,    -1,    29,   756,  1116,
    1117,    -1,    -1,    35,    36,    -1,    38,    -1,    40,    41,
     730,    -1,    -1,    -1,    -1,    47,    48,   737,    50,    -1,
      -1,   741,    -1,    -1,    -1,    57,    -1,   747,    -1,     4,
    1119,   751,  1136,  1137,   754,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    -1,    -1,    76,    -1,    -1,    79,    -1,  1153,
      25,    26,    -1,    -1,    29,  1159,    31,  1161,    -1,    -1,
    1164,  1165,   820,    -1,    -1,  1169,    98,    42,    -1,    -1,
    1174,    -1,    47,    48,  1178,    50,    -1,  1181,    -1,    -1,
      -1,    -1,    -1,    -1,  1173,   805,    -1,  1116,  1117,  1178,
      -1,    -1,    67,  1182,    -1,    -1,    -1,    -1,    73,    -1,
     820,   859,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   835,    -1,    -1,   838,    -1,
      -1,    -1,   842,    27,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,   899,    -1,    -1,    -1,    -1,   904,    -1,    52,    53,
      -1,    55,   910,   911,    58,    -1,    -1,    -1,    -1,   611,
     612,   613,   105,    67,    -1,   923,   924,    -1,    -1,    73,
      -1,    75,    -1,    -1,    78,    -1,   628,    81,   630,    -1,
     632,    -1,    -1,    -1,   904,    -1,    -1,    -1,    -1,    -1,
      -1,   643,    -1,   951,   952,   953,   954,    -1,    -1,    -1,
      -1,    -1,   960,   655,   962,    -1,    -1,   965,   966,   661,
     662,   663,     4,    -1,    -1,    -1,   668,   669,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    26,    27,    -1,    29,    -1,    31,
     692,    -1,   694,   695,    -1,    -1,    -1,   699,    40,   701,
      42,   703,    -1,    -1,    46,    47,    48,    -1,    50,    -1,
      52,    53,    -1,    55,   984,   208,    58,    -1,    -1,    -1,
      -1,    -1,   215,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    -1,    79,    -1,    81,
      27,  1011,    -1,    30,    31,    -1,    -1,  1017,   241,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    -1,  1032,  1033,    51,    -1,    -1,   260,    55,    -1,
      -1,    -1,    -1,   266,   267,    -1,   269,    -1,    -1,    -1,
      67,    -1,    -1,  1053,    -1,    -1,    73,    -1,    75,    -1,
      -1,    78,    -1,    -1,    81,    -1,    -1,    -1,  1068,  1069,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1124,    -1,    -1,    -1,
      -1,    -1,  1130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   340,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   361,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1146,    -1,   371,    -1,
    1150,   374,    -1,   376,    -1,    -1,   379,    -1,    -1,   382,
      -1,    -1,   385,   386,    -1,    -1,   389,    -1,    -1,   392,
      -1,    -1,   395,   396,   397,   398,    -1,    -1,   401,    -1,
      -1,    -1,   405,    -1,    -1,   408,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   937,    -1,    -1,   431,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   948,   949,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   977,   978,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1018,  1019,  1020,  1021,
      -1,    -1,  1024,    -1,    -1,    -1,    -1,  1029,  1030,    -1,
      -1,     4,     5,    -1,     7,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,   537,   538,   539,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,   616,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1138,  1139,    -1,    -1,
     633,   634,    -1,   636,   637,    -1,   639,    -1,   641,   642,
      -1,   644,   645,   646,   647,   648,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   659,   660,    -1,    -1,
      -1,    -1,    -1,   666,   667,    -1,    -1,   670,   671,   672,
      -1,   674,   675,   676,   677,   678,    -1,    -1,   681,   682,
     683,   684,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   696,   697,   698,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   731,    -1,
       4,    -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    16,    17,    18,   748,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,   826,   827,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   839,    -1,    -1,    -1,
      -1,    -1,    -1,   846,    -1,    -1,   849,    -1,   851,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     863,    -1,    -1,   866,   867,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   878,    -1,    -1,   881,    -1,
      -1,   884,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     943,   944,   945,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   959,    -1,   961,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   976,    -1,    -1,   979,   980,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   992,
      -1,    -1,    -1,    -1,   997,    -1,    -1,  1000,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1022,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1078,    -1,  1080,  1081,     4,
    1083,  1084,    -1,     8,  1087,    10,    11,  1090,  1091,    -1,
    1093,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     4,    -1,    -1,    -1,     8,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     4,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,     4,    -1,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,     4,     5,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     4,
      -1,    -1,    -1,     8,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   107,   112,   116,   433,     0,     5,     7,    54,    74,
     432,   432,   434,   434,   433,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   108,   418,   439,   440,   418,   109,
     113,   433,   117,   433,    97,    97,   110,   422,   422,    96,
      11,   115,   423,   432,   119,   433,   424,    56,    64,    82,
     114,   121,   124,   139,   118,   120,   121,     8,   425,   426,
     434,   434,   434,   434,   125,   125,    23,   136,   111,    97,
      99,   429,     8,   427,     9,    11,    12,    16,    17,    18,
     108,   436,   438,   439,   108,   417,   440,    17,   108,   122,
      43,    44,   126,   130,   140,   140,   434,   433,   433,   433,
      97,    99,   421,   437,   421,   433,   123,   421,   434,   434,
      28,    31,    59,    67,   141,   142,   143,   144,   145,   145,
     418,    11,   419,   430,   439,   428,   422,   422,   433,   433,
     129,   418,   418,   434,   434,   434,   434,    70,   146,   153,
     153,   137,   425,    98,    96,    98,   127,   131,   436,   436,
     436,   436,   434,   422,     4,    21,    26,    29,    35,    36,
      38,    40,    41,    47,    48,    50,    57,    72,    76,    79,
      98,   154,   155,   169,   174,   180,   227,   231,   232,   241,
     249,   257,   266,   281,   287,   303,   311,   329,   336,    98,
      97,    97,    99,   431,   433,   422,    97,    97,    99,   132,
     421,   421,   421,   421,    12,   108,   151,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   433,   422,   433,   422,   433,   433,     8,
     422,   422,   422,   152,   147,   418,   100,   108,   310,   389,
     390,   418,   418,   418,   108,   340,   390,   418,   418,   418,
     418,   418,   418,   418,   418,   182,   418,    11,   108,   420,
     440,   441,   139,   430,   433,   139,    71,   133,   134,   433,
      97,    99,   148,   282,   419,   304,   391,   433,   267,   233,
     156,   337,   433,   170,   228,   175,   242,   250,   258,   330,
     312,    97,   288,   433,   433,   138,    98,   128,   434,    98,
     422,    97,    99,   283,    97,   389,    97,    99,   268,    97,
      99,   234,    97,    99,   157,    97,    97,    99,   171,    97,
      99,   229,    97,    99,   176,    97,    97,    97,    97,    99,
     331,    97,    99,   313,   422,    97,    99,   289,    98,   433,
     133,    12,   135,   149,   422,   422,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   422,   185,   422,
     422,    98,   433,   150,   284,   306,   270,   235,   158,   339,
     172,   230,   178,   244,   252,   260,   332,   314,    30,    73,
      78,   143,   144,   164,   181,   186,   225,   226,   290,   422,
     421,    98,   143,   144,    27,    42,    51,    55,    73,    81,
      98,   143,   144,   165,   173,   356,   361,   366,   377,    25,
     143,   144,   164,   173,   231,   275,   305,   356,   143,   144,
     165,   173,   226,   232,   241,   249,   257,   269,   273,   274,
     275,   281,   356,   361,   366,    65,    98,   143,   144,   165,
     173,   180,   227,   231,   356,   361,   370,   377,    20,    98,
     143,   144,   159,   164,    37,   143,   144,   338,    98,   143,
     144,   165,   173,    98,   143,   144,   165,   180,   227,   231,
      22,   143,   144,   164,   177,   179,   143,   144,   165,   173,
     225,   226,   243,   246,   356,   361,   366,   377,    52,    53,
      58,   143,   144,   165,   173,   225,   251,   254,   356,   361,
     371,   373,   375,   377,    46,    77,   143,   144,   165,   173,
     180,   227,   231,   259,   356,   361,   371,   373,   375,   377,
     380,   383,    98,   143,   144,   165,   173,   180,   227,   231,
      45,    60,    98,   143,   144,   165,   173,   180,   227,   319,
     326,   434,   434,   434,    98,    75,   183,    21,    68,    98,
     143,   144,   165,   173,   294,   299,   356,   434,   434,   434,
     434,   434,   434,   285,   286,   434,   308,   309,    98,   307,
      98,   272,   271,   434,   239,   238,   240,   236,   237,   434,
     434,    98,   422,   422,   422,   434,    98,    98,   183,   245,
     248,   434,   434,   434,    98,   183,   253,   256,   434,   434,
     263,   264,   265,    98,   261,   262,   333,   334,   335,   433,
     433,   315,   316,   317,   318,   436,    85,    87,    90,   108,
     168,   436,   434,   422,   434,   434,   292,   293,   291,   362,
     420,   367,   436,   166,   436,   422,   422,   418,   422,   422,
     422,   422,   422,   436,   422,   422,   422,   422,   422,   418,
      84,    86,    89,    91,   341,   342,   343,   344,   348,   352,
     420,   247,   422,   422,    17,    94,   108,   374,   413,    18,
     108,   372,   413,   414,    92,    95,   108,   376,   255,   422,
     422,   108,   384,   419,   108,   381,   392,   419,   422,   422,
     422,   422,   422,   422,   422,   422,   320,   327,   422,   422,
     422,   422,   421,   433,   433,   433,   421,   421,   420,   108,
     302,   392,   108,   297,   392,   422,   422,   422,    88,    93,
     108,   364,   421,    88,    93,   108,   369,   378,   168,   357,
     276,   421,   160,   433,   433,   433,   433,   433,   421,   422,
     433,   421,   433,   421,   433,   433,   433,   421,   422,   421,
     385,   421,   382,   421,   434,   393,    97,    97,   184,   300,
     433,   295,   433,   433,   433,   363,   433,   433,   368,    97,
      99,   379,   167,    97,    99,   358,    97,    99,   277,    97,
      99,   161,   345,   349,   421,   353,   386,   421,   434,   392,
     391,   422,   422,    97,    99,   187,    97,    97,    99,   296,
     421,   421,   422,   421,   359,   422,   422,   422,    97,    99,
     346,    97,    99,   350,    97,    99,   354,   419,   382,   322,
     322,   422,   422,   422,   196,    98,   143,   144,   278,    83,
     162,   422,   422,   422,   387,   180,   227,   231,   321,   328,
      61,    69,   179,   188,   189,   200,   201,   211,   212,   214,
     215,   306,   298,    33,    34,    98,   143,   144,   223,   224,
      98,   143,   144,   165,   173,   231,   356,   434,    98,   347,
     351,   355,   386,   323,   324,   325,    98,    98,   434,   434,
      98,    39,    49,    62,    66,   190,   191,   193,   197,   209,
      32,   203,    24,   217,   301,    27,    51,    98,   143,   144,
     226,   360,   365,   370,   371,   373,   377,   434,   434,   280,
     279,    88,    93,   108,   163,    98,   225,   226,   360,   365,
     371,   373,   377,   380,    98,   225,   226,   377,   380,    98,
     183,   225,   226,   360,   365,   371,   373,   422,   422,   422,
     108,   394,   395,   396,    88,    93,   108,   213,   434,   434,
     434,   434,   183,   422,   434,   422,   434,   216,    98,   434,
     434,   422,   436,   436,   422,   422,   433,   433,   421,   422,
     422,   422,   421,   100,   397,   398,    14,   400,   401,   433,
     433,   421,   108,   192,   413,   194,   436,   198,   436,   388,
     436,   422,   204,   436,   202,   218,   418,   217,   364,   369,
     421,   421,   419,   399,   100,   402,   421,   433,    97,    99,
     195,    97,    99,   199,    97,    99,   210,    97,    99,   205,
     203,    97,    99,   219,   421,   421,   405,   397,   401,   403,
     419,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     101,   404,   405,   196,   196,   196,   206,   220,     5,   435,
     398,    98,    98,    98,    80,    98,   143,   144,   164,   207,
      63,    98,   143,   144,   164,   221,   406,   419,   434,   434,
     435,   435,    16,    17,    18,   108,   208,   415,   416,   108,
     222,   414,   102,   103,   421,   433,   421,   433,   435,    85,
     407,   412,   435,   435,   104,   100,   435,   435,   105,    14,
     408,   409,   435,   410,   100,   409,   419,   435,   411,   419,
     435,   100,   435,   419
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   106,   107,   107,   109,   108,   110,   111,   110,   113,
     114,   112,   115,   115,   115,   115,   117,   118,   116,   119,
     119,   120,   119,   121,   123,   122,   122,   124,   125,   125,
     125,   127,   128,   126,   129,   131,   130,   132,   132,   133,
     133,   134,   135,   137,   138,   136,   139,   140,   140,   140,
     140,   140,   141,   142,   143,   144,   145,   145,   147,   146,
     148,   149,   148,   150,   150,   150,   152,   151,   151,   153,
     153,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   156,   155,   157,   157,   158,   158,   158,   158,   158,
     160,   159,   161,   161,   162,   163,   163,   163,   164,   166,
     167,   165,   168,   168,   168,   168,   170,   169,   171,   171,
     172,   172,   172,   172,   172,   173,   175,   174,   176,   177,
     176,   178,   178,   178,   178,   178,   179,   181,   180,   182,
     184,   183,   185,   186,   185,   185,   185,   185,   185,   185,
     187,   187,   188,   188,   188,   188,   188,   188,   189,   189,
     189,   189,   189,   189,   190,   191,   192,   192,   193,   194,
     195,   195,   196,   196,   196,   196,   196,   197,   198,   199,
     199,   201,   200,   202,   202,   203,   204,   205,   205,   206,
     206,   206,   206,   206,   207,   208,   208,   209,   210,   210,
     211,   212,   213,   213,   213,   215,   214,   216,   216,   217,
     218,   219,   219,   220,   220,   220,   220,   220,   221,   222,
     222,   223,   224,   225,   226,   228,   227,   229,   229,   230,
     230,   230,   230,   230,   230,   230,   231,   231,   231,   231,
     231,   231,   231,   233,   232,   234,   234,   235,   236,   235,
     235,   237,   235,   235,   235,   235,   235,   235,   238,   235,
     239,   235,   240,   235,   242,   243,   241,   244,   245,   244,
     244,   246,   247,   244,   244,   248,   244,   244,   244,   244,
     244,   244,   244,   250,   251,   249,   252,   253,   252,   252,
     254,   255,   252,   252,   256,   252,   252,   252,   252,   252,
     252,   252,   252,   258,   259,   257,   260,   261,   260,   260,
     262,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   263,   260,   264,   260,   265,   260,   267,   266,   268,
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
     310,   312,   311,   313,   313,   314,   314,   314,   314,   314,
     315,   314,   316,   314,   317,   314,   318,   314,   320,   321,
     319,   322,   323,   322,   324,   322,   325,   322,   327,   328,
     326,   330,   329,   331,   331,   332,   332,   332,   332,   332,
     333,   332,   334,   332,   335,   332,   337,   338,   336,   339,
     339,   339,   339,   340,   340,   341,   341,   342,   342,   342,
     343,   345,   344,   346,   346,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   349,   348,   350,   350,   351,   351,
     351,   351,   351,   353,   352,   354,   354,   355,   355,   355,
     355,   355,   355,   355,   355,   357,   356,   358,   358,   359,
     359,   359,   360,   362,   363,   361,   364,   364,   364,   365,
     367,   368,   366,   369,   369,   369,   370,   371,   372,   372,
     373,   374,   374,   374,   375,   376,   376,   376,   378,   377,
     379,   379,   380,   381,   381,   382,   382,   383,   385,   384,
     384,   387,   386,   386,   388,   389,   390,   391,   391,   393,
     392,   395,   394,   396,   394,   394,   397,   398,   399,   399,
     400,   401,   402,   402,   403,   404,   404,   405,   405,   406,
     407,   408,   409,   410,   410,   411,   411,   412,   413,   414,
     414,   415,   415,   416,   416,   417,   417,   418,   418,   419,
     419,   420,   420,   420,   421,   421,   422,   422,   422,   424,
     423,   425,   426,   426,   427,   427,   428,   428,   429,   429,
     430,   430,   431,   431,   432,   432,   433,   433,   434,   435,
     435,   437,   436,   436,   438,   438,   438,   438,   438,   438,
     438,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   440,
     441
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     4,     0,     0,     6,     0,
       0,    15,     0,     2,     2,     2,     0,     0,    15,     0,
       2,     0,     3,     4,     0,     3,     1,     4,     0,     2,
       2,     0,     0,    11,     1,     0,     6,     1,     4,     0,
       1,     5,     1,     0,     0,    10,     4,     0,     2,     2,
       2,     2,     4,     4,     4,     4,     0,     3,     0,     5,
       1,     0,     5,     0,     2,     2,     0,     3,     1,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     2,     2,     2,     2,
       0,     6,     1,     4,     4,     2,     2,     1,     4,     0,
       0,     6,     2,     2,     2,     1,     0,     5,     1,     4,
       0,     2,     2,     2,     2,     4,     0,     5,     1,     0,
       5,     0,     2,     2,     2,     2,     4,     0,     8,     1,
       0,     5,     0,     0,     4,     2,     2,     2,     2,     2,
       1,     4,     1,     1,     1,     1,     1,     1,     0,     2,
       2,     2,     3,     4,     0,     4,     2,     1,     5,     1,
       1,     4,     0,     2,     2,     2,     2,     5,     1,     1,
       4,     0,     4,     0,     3,     4,     1,     1,     4,     0,
       2,     2,     2,     2,     4,     2,     1,     4,     1,     4,
       4,     4,     2,     2,     1,     0,     3,     0,     2,     5,
       1,     1,     4,     0,     2,     2,     2,     2,     4,     2,
       1,     4,     4,     4,     4,     0,     5,     1,     4,     0,
       2,     2,     2,     3,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     0,     5,     1,     4,     0,     0,     4,
       2,     0,     4,     2,     2,     2,     2,     2,     0,     4,
       0,     4,     0,     4,     0,     0,     9,     0,     0,     4,
       2,     0,     0,     5,     2,     0,     4,     2,     2,     2,
       2,     2,     2,     0,     0,     9,     0,     0,     4,     2,
       0,     0,     5,     2,     0,     4,     2,     2,     2,     2,
       2,     2,     2,     0,     0,     9,     0,     0,     4,     2,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     5,     1,
       0,     5,     0,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     0,     4,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     5,     1,     4,     0,     0,
       4,     2,     0,     4,     2,     2,     2,     2,     2,     0,
       5,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     5,     1,     4,     2,     1,
       0,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     0,     9,     2,     1,     0,     0,     9,     0,     0,
       4,     2,     2,     2,     2,     0,     4,     0,     4,     2,
       1,     0,     5,     1,     4,     0,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     0,
       8,     0,     0,     4,     0,     4,     0,     4,     0,     0,
       8,     0,     5,     1,     4,     0,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     0,     9,     0,
       2,     2,     4,     2,     1,     1,     2,     1,     1,     1,
       3,     0,     4,     1,     4,     0,     2,     3,     2,     2,
       2,     2,     2,     2,     0,     4,     1,     4,     0,     2,
       3,     2,     2,     0,     4,     1,     4,     0,     3,     2,
       2,     2,     2,     2,     2,     0,     5,     1,     3,     0,
       2,     2,     4,     0,     0,     6,     2,     2,     1,     4,
       0,     0,     6,     2,     2,     1,     4,     4,     2,     1,
       4,     2,     2,     1,     4,     2,     2,     1,     0,     5,
       1,     4,     3,     2,     2,     3,     1,     3,     0,     3,
       2,     0,     4,     1,     1,     2,     2,     0,     2,     0,
       3,     0,     2,     0,     2,     1,     3,     2,     0,     2,
       3,     2,     0,     2,     2,     0,     2,     0,     6,     5,
       5,     5,     4,     0,     2,     0,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     2,     2,     1,     2,     4,     0,     2,     2,     0,
       4,     2,     0,     1,     0,     3,     0,     5,     1,     4,
       0,     3,     2,     5,     1,     1,     0,     2,     2,     0,
       1,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
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
      yyerror (scanner, module, submodule, unres, size_arrays, read_all, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner, module, submodule, unres, size_arrays, read_all); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner, module, submodule, unres, size_arrays, read_all);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
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
                                              , scanner, module, submodule, unres, size_arrays, read_all);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner, module, submodule, unres, size_arrays, read_all); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner, struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all)
{
  YYUSE (yyvaluep);
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
          case 129: /* tmp_identifier_arg_str  */
#line 233 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1257  */
      { free(((*yyvaluep).str)); }
#line 2446 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1257  */
        break;

    case 270: /* choice_opt_stmt  */
#line 234 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1257  */
      { if (read_all && ((*yyvaluep).nodes).choice.s) { free(((*yyvaluep).nodes).choice.s); } }
#line 2452 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1257  */
        break;

    case 339: /* deviation_opt_stmt  */
#line 235 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1257  */
      { if (read_all) {
                free(((*yyvaluep).nodes).deviation);
              }
            }
#line 2461 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1257  */
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


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

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
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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
      yychar = yylex (&yylval, scanner);
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 245 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && lyp_propagate_submodule(module, submodule)) {
                       YYERROR;
                     }
                   }
#line 2732 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 4:
#line 251 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                      s = strdup(yyget_text(scanner));
                      if (!s) {
                        LOGMEM;
                        YYERROR;
                      }
                    }
                  }
#line 2745 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 7:
#line 264 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all){
                s = realloc(s, yyget_leng(scanner) + strlen(s) + 1);
                if (s) {
                  strcat(s, yyget_text(scanner));
                } else {
                  LOGMEM;
                  YYERROR;
                }
              }
            }
#line 2760 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 9:
#line 276 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                              if (submodule) {
                                                                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "module");
                                                                YYERROR;
                                                              }
                                                              trg = module;
                                                              yang_read_common(trg,s,MODULE_KEYWORD);
                                                              s = NULL;
                                                              config_inherit = ENABLE_INHERIT;
                                                            }
                                                          }
#line 2776 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 10:
#line 288 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !module->ns) { LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "namespace", "module"); YYERROR; }
                                       if (read_all && !module->prefix) { LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "module"); YYERROR; }
                                     }
#line 2784 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 12:
#line 297 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 2790 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 13:
#line 298 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].i)) { LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "module"); YYERROR; } (yyval.i) = 1; }
#line 2796 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 14:
#line 299 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_common(module, s, NAMESPACE_KEYWORD)) {YYERROR;} s=NULL; }
#line 2802 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 15:
#line 300 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_prefix(module, NULL, s, MODULE_KEYWORD)) {YYERROR;} s=NULL; }
#line 2808 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 16:
#line 303 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                                    if (!submodule) {
                                                                      LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "submodule");
                                                                      YYERROR;
                                                                    }
                                                                    trg = (struct lys_module *)submodule;
                                                                    yang_read_common(trg,s,MODULE_KEYWORD);
                                                                    s = NULL;
                                                                    config_inherit = ENABLE_INHERIT;
                                                                  }
                                                                }
#line 2824 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 17:
#line 315 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !submodule->prefix) {
                                                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                                YYERROR;
                                              }
                                            }
#line 2834 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 19:
#line 326 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 2840 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 20:
#line 327 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].i)) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "submodule");
                                                  YYERROR;
                                                }
                                                (yyval.i) = 1;
                                              }
#line 2851 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 21:
#line 333 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                if (submodule->prefix) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                  YYERROR;
                                }
                              }
                            }
#line 2863 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 24:
#line 344 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (strlen(yyget_text(scanner))!=1 || yyget_text(scanner)[0]!='1') {
                                               YYERROR;
                                             }
                                           }
#line 2872 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 26:
#line 348 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                 if (strlen(s)!=1 || s[0]!='1') {
                   free(s);
                   YYERROR;
                 }
               }
             }
#line 2884 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 28:
#line 358 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                          if (size_arrays->imp) {
                            trg->imp = calloc(size_arrays->imp, sizeof *trg->imp);
                            if (!trg->imp) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->inc) {
                            trg->inc = calloc(size_arrays->inc, sizeof *trg->inc);
                            if (!trg->inc) {
                              LOGMEM;
                              YYERROR;
                            }
                            trg->inc_size = size_arrays->inc;
                            size_arrays->inc = 0;
                            /* trg->inc_size can be updated by the included submodules,
                             * so we will use size_arrays->inc here, trg->inc_size stores the
                             * target size of the array
                             */
                          }
                        }
                      }
#line 2912 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 31:
#line 385 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    {
                 if (!read_all) {
                   size_arrays->imp++;
                 } else {
                   actual = &trg->imp[trg->imp_size];
                 }
             }
#line 2924 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 32:
#line 393 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                 if (yang_read_prefix(trg, actual, s, IMPORT_KEYWORD)) {YYERROR;}
                                 s=NULL;
                                 actual_type=IMPORT_KEYWORD;
                               }
                             }
#line 2935 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 33:
#line 400 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                             (yyval.inc) = trg;
                             if (yang_fill_import(trg, actual, (yyvsp[-8].str))) {
                               YYERROR;
                             }
                             trg = (yyval.inc);
                             config_inherit = ENABLE_INHERIT;
                           }
                         }
#line 2949 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 34:
#line 410 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.str) = s; s = NULL; }
#line 2955 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 35:
#line 412 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                             memset(rev, 0, LY_REV_SIZE);
                                                             actual_type = INCLUDE_KEYWORD;
                                                           }
                                                           else {
                                                             size_arrays->inc++;
                                                           }
                                                         }
#line 2968 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 36:
#line 420 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      (yyval.inc) = trg;
                                      if (yang_fill_include(module, submodule, s, rev, size_arrays->inc, unres)) {
                                        YYERROR;
                                      }
                                      size_arrays->inc++;
                                      s = NULL;
                                      trg = (yyval.inc);
                                      config_inherit = ENABLE_INHERIT;
                                    }
                                  }
#line 2984 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 42:
#line 443 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                 if (actual_type==IMPORT_KEYWORD) {
                                     memcpy(((struct lys_import *)actual)->rev, yyget_text(scanner), LY_REV_SIZE-1);
                                 } else {                              // INCLUDE KEYWORD
                                     memcpy(rev, yyget_text(scanner), LY_REV_SIZE - 1);
                                 }
                               }
                             }
#line 2997 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 43:
#line 452 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                               if (!ly_strequal(s, submodule->belongsto->name, 0)) {
                                                                 LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "belongs-to");
                                                                 free(s);
                                                                 YYERROR;
                                                               }
                                                               free(s);
                                                               s = NULL;
                                                             }
                                                           }
#line 3012 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 44:
#line 463 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                     if (yang_read_prefix(trg, NULL, s, MODULE_KEYWORD)) {
                                       YYERROR;
                                     }
                                     s = NULL;
                                   }
                                 }
#line 3024 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 48:
#line 475 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {YYERROR;} s=NULL; }
#line 3030 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 49:
#line 476 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_common(trg, s, CONTACT_KEYWORD)) {YYERROR;} s=NULL; }
#line 3036 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 50:
#line 477 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, NULL, s, NULL)) {
                                     YYERROR;
                                   }
                                   s = NULL;
                                 }
#line 3046 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 51:
#line 482 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, NULL, s, NULL)) {
                                   YYERROR;
                                 }
                                 s=NULL;
                               }
#line 3056 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 56:
#line 496 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && size_arrays->rev) {
                           trg->rev = calloc(size_arrays->rev, sizeof *trg->rev);
                           if (!trg->rev) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                       }
#line 3069 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 58:
#line 507 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                     if(!(actual=yang_read_revision(trg,s))) {YYERROR;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }
#line 3081 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 61:
#line 517 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual_type = REVISION_KEYWORD; }
#line 3087 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 64:
#line 523 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, "revision")) {
                                            YYERROR;
                                          }
                                          s = NULL;
                                        }
#line 3097 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 65:
#line 528 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, "revision")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }
#line 3107 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 66:
#line 535 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                s = strdup(yyget_text(scanner));
                                if (!s) {
                                  LOGMEM;
                                  YYERROR;
                                }
                              }
                            }
#line 3120 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 68:
#line 544 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && lyp_check_date(s)) {
                   free(s);
                   YYERROR;
               }
             }
#line 3130 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 69:
#line 550 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                       if (size_arrays->features) {
                         trg->features = calloc(size_arrays->features,sizeof *trg->features);
                         if (!trg->features) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->ident) {
                         trg->ident = calloc(size_arrays->ident,sizeof *trg->ident);
                         if (!trg->ident) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->augment) {
                         trg->augment = calloc(size_arrays->augment,sizeof *trg->augment);
                         if (!trg->augment) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->tpdf) {
                         trg->tpdf = calloc(size_arrays->tpdf, sizeof *trg->tpdf);
                         if (!trg->tpdf) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                       if (size_arrays->deviation) {
                         trg->deviation = calloc(size_arrays->deviation, sizeof *trg->deviation);
                         if (!trg->deviation) {
                           LOGMEM;
                           YYERROR;
                         }
                         /* module with deviation - must be implemented (description of /ietf-yang-library:modules-state/module/deviation) */
                         module->implemented = 1;
                       }
                       actual = NULL;
                     }
                   }
#line 3176 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 70:
#line 591 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = NULL; }
#line 3182 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 74:
#line 597 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) { size_arrays->tpdf++; } }
#line 3188 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 77:
#line 600 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                     size_arrays->augment++;
                   } else {
                     config_inherit = ENABLE_INHERIT;
                   }
                 }
#line 3199 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 80:
#line 608 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) { size_arrays->deviation++; } }
#line 3205 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 81:
#line 610 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
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
#line 3227 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 85:
#line 634 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = 0; }
#line 3233 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 86:
#line 635 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].uint) & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYERROR;
                                        }
                                        (yyvsp[-1].uint) |= EXTENSION_ARG;
                                        (yyval.uint) = (yyvsp[-1].uint);
                                      }
#line 3245 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 87:
#line 642 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].uint) & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYERROR;
                                      }
                                      (yyvsp[-1].uint) |= EXTENSION_STA;
                                      (yyval.uint) = (yyvsp[-1].uint);
                                    }
#line 3257 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 88:
#line 649 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             free(s);
                                             s= NULL;
                                           }
                                           if ((yyvsp[-1].uint) & EXTENSION_DSC) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "description", "extension");
                                             YYERROR;
                                           }
                                           (yyvsp[-1].uint) |= EXTENSION_DSC;
                                           (yyval.uint) = (yyvsp[-1].uint);
                                         }
#line 3273 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 89:
#line 660 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                           free(s);
                                           s = NULL;
                                         }
                                         if ((yyvsp[-1].uint) & EXTENSION_REF) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "reference", "extension");
                                           YYERROR;
                                         }
                                         (yyvsp[-1].uint) |= EXTENSION_REF;
                                         (yyval.uint) = (yyvsp[-1].uint);
                                       }
#line 3289 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 90:
#line 672 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { free(s); s = NULL; }
#line 3295 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 97:
#line 683 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                 if (strcmp(s, "true") && strcmp(s, "false")) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                    free(s);
                    YYERROR;
                 }
                 free(s);
                 s = NULL;
               }
             }
#line 3310 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 98:
#line 694 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 3316 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 99:
#line 696 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }
#line 3322 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 100:
#line 697 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }
#line 3328 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 101:
#line 698 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-2].i); }
#line 3334 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 102:
#line 700 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_STATUS_CURR; }
#line 3340 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 103:
#line 701 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_STATUS_OBSLT; }
#line 3346 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 104:
#line 702 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_STATUS_DEPRC; }
#line 3352 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 105:
#line 703 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
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
                   YYERROR;
                 }
                 free(s);
                 s = NULL;
               }
             }
#line 3373 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 106:
#line 720 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                         if (!(actual = yang_read_feature(trg, s))) {YYERROR;}
                                                         s=NULL; 
                                                       } else {
                                                         size_arrays->features++;
                                                       }
                                                     }
#line 3385 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 110:
#line 735 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                             if (size_arrays->node[size_arrays->next].if_features) {
                               ((struct lys_feature*)actual)->features = calloc(size_arrays->node[size_arrays->next].if_features,
                                                                                sizeof *((struct lys_feature*)actual)->features);
                               if (!((struct lys_feature*)actual)->features) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                             store_flags((struct lys_node *)actual, size_arrays->node[size_arrays->next].flags, 0);
                             size_arrays->next++;
                           } else {
                             (yyval.i) = size_arrays->size;
                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                               LOGMEM;
                               YYERROR;
                             }
                           } 
                         }
#line 3409 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 111:
#line 754 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                          if (yang_read_if_feature(trg, actual, s, unres, FEATURE_KEYWORD)) {YYERROR;}
                                          s=NULL; 
                                        } else {
                                          size_arrays->node[(yyvsp[-1].i)].if_features++;
                                        }
                                      }
#line 3421 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 112:
#line 761 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].i)].flags, LYS_STATUS_MASK, "status", "feature", (yyvsp[0].i))) {
                                             YYERROR;
                                           }
                                         }
                                       }
#line 3432 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 113:
#line 767 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, "feature")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }
#line 3442 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 114:
#line 772 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, "feature")) {
                                         YYERROR;
                                       }
                                       s = NULL;
                                     }
#line 3452 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 116:
#line 780 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                           if (!(actual = yang_read_identity(trg,s))) {YYERROR;}
                                                           s = NULL;
                                                         } else {
                                                           size_arrays->ident++;
                                                         }
                                                       }
#line 3464 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 119:
#line 791 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_base(module, actual, (yyvsp[0].str), unres)) {
                               YYERROR;
                             }
                           }
#line 3473 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 121:
#line 798 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.str) = NULL; }
#line 3479 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 122:
#line 799 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                     if ((yyvsp[-1].str)) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                                       free(s);
                                       free((yyvsp[-1].str));
                                       YYERROR;
                                     }
                                     (yyval.str) = s;
                                     s = NULL;
                                   }
                                 }
#line 3495 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 123:
#line 810 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (yang_check_flags(&((struct lys_ident *)actual)->flags, LYS_STATUS_MASK, "status", "identity", (yyvsp[0].i))) {
                                         YYERROR;
                                       }
                                     }
                                   }
#line 3506 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 124:
#line 816 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, "identity")) {
                                            free((yyvsp[-1].str));
                                            YYERROR;
                                          }
                                          s = NULL;
                                        }
#line 3517 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 125:
#line 822 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, "identity")) {
                                          free((yyvsp[-1].str));
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }
#line 3528 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 127:
#line 833 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                    if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                                      YYERROR;
                                    }
                                    if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DER,(struct lys_node *) (yyvsp[-3].v))) {
                                      YYERROR;
                                    }
                                    actual = (yyvsp[-3].v);

                                    /* check default value */
                                    if ((yyvsp[0].nodes).node.ptr_tpdf->dflt) {
                                      if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_tpdf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_tpdf->dflt) == -1) {
                                        YYERROR;
                                      }
                                    }
                                  }
                                }
#line 3551 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 129:
#line 853 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                        (yyval.v) = actual;
                                        if (!(actual = yang_read_typedef(trg, actual, s))) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                        actual_type = TYPEDEF_KEYWORD;
                                      }
                                    }
#line 3565 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 130:
#line 863 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !(actual = yang_read_type(trg, actual, s, actual_type))) {
                                                       YYERROR;
                                                     }
                                                     s = NULL;
                                                   }
#line 3575 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 132:
#line 870 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes).node.ptr_tpdf = actual;
                        (yyval.nodes).node.flag = 0;
                      }
#line 3583 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 133:
#line 873 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, (yyvsp[0].nodes).node.ptr_tpdf, "type", "typedef");
                       YYERROR;
                     }
                   }
#line 3593 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 134:
#line 878 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                   actual = (yyvsp[-3].nodes).node.ptr_tpdf;
                   actual_type = TYPEDEF_KEYWORD;
                   (yyvsp[-3].nodes).node.flag |= LYS_TYPE_DEF;
                   (yyval.nodes) = (yyvsp[-3].nodes);
                 }
               }
#line 3605 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 135:
#line 885 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYERROR;} s = NULL; }
#line 3611 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 136:
#line 886 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYERROR;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }
#line 3622 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 137:
#line 892 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                   if (yang_check_flags(&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i))) {
                                     YYERROR;
                                   }
                                 }
                               }
#line 3633 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 138:
#line 898 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 3643 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 139:
#line 903 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }
#line 3653 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 144:
#line 918 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /*leafref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->type->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }
#line 3665 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 145:
#line 925 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /*identityref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->flags |= LYS_TYPE_BASE;
                   ((struct yang_type *)actual)->type->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }
#line 3678 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 146:
#line 933 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /*instance_identifier_specification */
                             if (read_all) {
                               ((struct yang_type *)actual)->type->base = LY_TYPE_INST;
                             }
                           }
#line 3688 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 148:
#line 942 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if (size_arrays->node[size_arrays->next].uni && size_arrays->node[size_arrays->next].pattern) {
                                           LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", ((struct yang_type *)actual)->type->parent->name);
                                           YYERROR;
                                         }
                                         if (size_arrays->node[size_arrays->next].pattern) {
                                           ((struct yang_type *)actual)->type->info.str.patterns = calloc(size_arrays->node[size_arrays->next].pattern, sizeof(struct lys_restr));
                                           if (!((struct yang_type *)actual)->type->info.str.patterns) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           ((struct yang_type *)actual)->type->base = LY_TYPE_STRING;
                                         }
                                         if (size_arrays->node[size_arrays->next].uni) {
                                           ((struct yang_type *)actual)->type->info.uni.types = calloc(size_arrays->node[size_arrays->next].uni, sizeof(struct lys_type));
                                           if (!((struct yang_type *)actual)->type->info.uni.types) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           ((struct yang_type *)actual)->type->base = LY_TYPE_UNION;
                                         }
                                         size_arrays->next++;
                                       } else {
                                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                           LOGMEM;
                                           YYERROR;
                                         }
                                         (yyval.uint) = size_arrays->size-1;
                                       }
                                     }
#line 3723 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 150:
#line 973 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                                  size_arrays->node[(yyvsp[-1].uint)].pattern++; /* count of pattern*/
                                                }
                                              }
#line 3732 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 153:
#line 979 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                                  actual = (yyvsp[-2].v);
                                                                } else {
                                                                  size_arrays->node[(yyvsp[-3].uint)].uni++; /* count of union*/
                                                                }
                                                              }
#line 3743 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 154:
#line 987 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
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
#line 3760 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1001 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYERROR;
                                }
                              }
#line 3769 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1006 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[-1].uint); }
#line 3775 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1007 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                 char *endptr = NULL;
                 unsigned long val;

                 val = strtoul(s, &endptr, 10);
                 if (*endptr || s[0] == '-' || val == 0 || val > UINT32_MAX) {
                   LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "fraction-digits");
                   free(s);
                   s = NULL;
                   YYERROR;
                 }
                 (yyval.uint) = (uint32_t) val;
                 free(s);
                 s =NULL;
               }
             }
#line 3796 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1025 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }
#line 3804 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1029 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                           (yyval.v) = actual;
                           if (!(actual = yang_read_length(trg, actual, s))) {
                             YYERROR;
                           }
                           actual_type = LENGTH_KEYWORD;
                           s = NULL;
                         }
                       }
#line 3818 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1045 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
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
#line 3838 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1060 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }
#line 3848 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1065 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }
#line 3858 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1070 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, (yyvsp[-1].str))) {
                                           YYERROR;
                                          }
                                          s = NULL;
                                        }
#line 3868 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1075 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, (yyvsp[-1].str))) {
                                         YYERROR;
                                       }
                                       s = NULL;
                                     }
#line 3878 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1081 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-2].v);
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }
#line 3886 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1085 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                            (yyval.v) = actual;
                            if (!(actual = yang_read_pattern(trg, actual, s))) {
                              YYERROR;
                            }
                            actual_type = PATTERN_KEYWORD;
                            s = NULL;
                          }
                        }
#line 3900 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1101 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                        if (size_arrays->node[size_arrays->next].enm) {
                          ((struct yang_type *)actual)->type->info.enums.enm = calloc(size_arrays->node[size_arrays->next++].enm, sizeof(struct lys_type_enum));
                          if (!((struct yang_type *)actual)->type->info.enums.enm) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                        ((struct yang_type *)actual)->type->base = LY_TYPE_ENUM;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYERROR;
                        }
                      }
                    }
#line 3922 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1124 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
               if (yang_check_enum((yyvsp[-1].v), actual, &cnt_val, actual_type)) {
                 YYERROR;
               }
               actual = (yyvsp[-1].v);
               actual_type = TYPE_KEYWORD;
             } else {
               size_arrays->node[size_arrays->size-1].enm++; /* count of enum*/
             }
           }
#line 3937 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1135 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                         (yyval.v) = actual;
                         if (!(actual = yang_read_enum(trg, actual, s))) {
                           YYERROR;
                         }
                         s = NULL;
                         actual_type = 0;
                       }
                     }
#line 3951 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1152 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /* actual_type - it is used to check value of enum statement*/
                                if (read_all) {
                                  if (actual_type) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                    YYERROR;
                                  }
                                  actual_type = 1;
                                }
                              }
#line 3965 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1161 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                   if (yang_check_flags(&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i))) {
                                     YYERROR;
                                   }
                                 }
                               }
#line 3976 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1167 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, "enum")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 3986 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1172 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, "enum")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }
#line 3996 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1179 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                        ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                        /* keep the highest enum value for automatic increment */
                        if ((yyvsp[-1].i) > cnt_val) {
                          cnt_val = (yyvsp[-1].i);
                        }
                        cnt_val++;
                      }
                    }
#line 4011 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1190 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 4017 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1191 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                  /* convert it to int32_t */
                  int64_t val;
                  char *endptr;

                  val = strtoll(s, &endptr, 10);
                  if (val < INT32_MIN || val > INT32_MAX || *endptr) {
                      LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "value");
                      free(s);
                      YYERROR;
                  }
                  free(s);
                  s = NULL;
                  (yyval.i) = (int32_t) val;
               }
             }
#line 4038 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1209 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }
#line 4046 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1224 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }
#line 4055 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1228 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }
#line 4064 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1232 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                  if (!strcmp(s,"true")) {
                    ((struct yang_type *)actual)->type->info.inst.req = 1;
                  } else if (!strcmp(s,"false")) {
                    ((struct yang_type *)actual)->type->info.inst.req = -1;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "require-instance");
                    free(s);
                    YYERROR;
                  }
                  free(s);
                }
              }
#line 4082 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1247 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                        if (size_arrays->node[size_arrays->next].bit) {
                          ((struct yang_type *)actual)->type->info.bits.bit = calloc(size_arrays->node[size_arrays->next++].bit, sizeof(struct lys_type_bit));
                          if (!((struct yang_type *)actual)->type->info.bits.bit) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                        ((struct yang_type *)actual)->type->base = LY_TYPE_BITS;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYERROR;
                        }
                      }
                    }
#line 4104 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1269 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                      if (yang_check_bit((yyvsp[-2].v), actual, &cnt_val, actual_type)) {
                        YYERROR;
                      }
                      actual = (yyvsp[-2].v);
                    } else {
                      size_arrays->node[size_arrays->size-1].bit++; /* count of bit*/
                    }
                  }
#line 4118 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1279 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                    (yyval.v) = actual;
                                    if (!(actual = yang_read_bit(trg, actual, s))) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                    actual_type = 0;
                                  }
                                }
#line 4132 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1296 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /* actual_type - it is used to check position of bit statement*/
                                  if (read_all) {
                                    if (actual_type) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                      YYERROR;
                                    }
                                    actual_type = 1;
                                  }
                                }
#line 4146 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1305 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                  if (yang_check_flags(&((struct lys_type_bit *)actual)->flags, LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i))) {
                                    YYERROR;
                                  }
                                }
                              }
#line 4157 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1311 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, "bit")) {
                                       YYERROR;
                                     }
                                     s = NULL;
                                   }
#line 4167 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1316 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, "bit")) {
                                     YYERROR;
                                   }
                                   s = NULL;
                                 }
#line 4177 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1323 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                           ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                           /* keep the highest position value for automatic increment */
                           if ((yyvsp[-1].uint) > cnt_val) {
                             cnt_val = (yyvsp[-1].uint);
                           }
                           cnt_val++;
                         }
                       }
#line 4192 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1334 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[-1].uint); }
#line 4198 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1335 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /* convert it to uint32_t */
                unsigned long val;
                char *endptr;

                val = strtoul(s, &endptr, 10);
                if (val > UINT32_MAX || s[0] == '-' || *endptr) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "position");
                    free(s);
                    YYERROR;
                }
                free(s);
                s = NULL;
                (yyval.uint) = (uint32_t) val;
              }
#line 4217 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1358 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYERROR;}
                                                           s=NULL;
                                                         }
                                                       }
#line 4227 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1371 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                               (yyval.nodes).grouping = actual;
                               actual_type = GROUPING_KEYWORD;
                               if (size_arrays->node[size_arrays->next].tpdf) {
                                 (yyval.nodes).grouping->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).grouping->tpdf);
                                 if (!(yyval.nodes).grouping->tpdf) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).grouping, size_arrays->node[size_arrays->next].flags, 0);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }
#line 4252 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1391 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                            if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "grouping", (yyvsp[0].i))) {
                                              YYERROR;
                                            }
                                          }
                                        }
#line 4263 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1397 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                            YYERROR;
                                          }
                                          s = NULL;
                                        }
#line 4273 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1402 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }
#line 4283 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1407 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }
#line 4289 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1408 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                actual = (yyvsp[-2].nodes).grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[(yyvsp[-2].nodes).index].tpdf++;
                                              }
                                            }
#line 4301 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1415 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-2].nodes).grouping; actual_type = GROUPING_KEYWORD; }
#line 4307 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1427 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYERROR;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           }
                                                         }
#line 4318 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1441 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                               (yyval.nodes).container = actual;
                               actual_type = CONTAINER_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 (yyval.nodes).container->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).container->features);
                                 if (!(yyval.nodes).container->features) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 (yyval.nodes).container->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).container->must);
                                 if (!(yyval.nodes).container->must) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].tpdf) {
                                 (yyval.nodes).container->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).container->tpdf);
                                 if (!(yyval.nodes).container->tpdf) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).container, size_arrays->node[size_arrays->next].flags, config_inherit);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }
#line 4357 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1475 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).container; actual_type = CONTAINER_KEYWORD; }
#line 4363 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1477 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).container, s, unres, CONTAINER_KEYWORD)) {YYERROR;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }
#line 4375 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1484 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }
#line 4387 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1492 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {YYERROR;} s=NULL; }
#line 4393 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1493 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "container", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }
#line 4404 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1499 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "container", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }
#line 4415 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1505 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }
#line 4425 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1510 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }
#line 4435 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1515 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }
#line 4444 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1520 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                 actual = (yyvsp[-1].nodes).container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                               }
                                             }
#line 4456 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1528 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }
#line 4465 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1535 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }
#line 4476 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1542 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                  if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                                    YYERROR;
                                  } else {
                                      if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *) (yyvsp[0].nodes).node.ptr_leaf)) {
                                        (yyvsp[0].nodes).node.ptr_leaf->type.der = NULL;
                                        YYERROR;
                                      }
                                  }
                                  if ((yyvsp[0].nodes).node.ptr_leaf->dflt) {
                                    if ((yyvsp[0].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE) {
                                      /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                                      LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "mandatory", "list");
                                      LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                                      YYERROR;
                                    }
                                    if (unres_schema_add_str(trg, unres, &(yyvsp[0].nodes).node.ptr_leaf->type, UNRES_TYPE_DFLT, (yyvsp[0].nodes).node.ptr_leaf->dflt) == -1) {
                                      YYERROR;
                                    }
                                  }
                                }
                              }
#line 4504 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1567 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                            (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LEAF_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).node.ptr_leaf->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_leaf->features);
                              if (!(yyval.nodes).node.ptr_leaf->features) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).node.ptr_leaf->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_leaf->must);
                              if (!(yyval.nodes).node.ptr_leaf->must) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaf, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }
#line 4537 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1595 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_leaf; actual_type = LEAF_KEYWORD; }
#line 4543 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1597 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, unres, LEAF_KEYWORD)) {YYERROR;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }
#line 4555 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1604 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaf, "type", "leaf");
                         YYERROR;
                       }
                     }
#line 4565 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1609 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                     actual = (yyvsp[-2].nodes).node.ptr_leaf;
                     actual_type = LEAF_KEYWORD;
                     (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                   }
                 }
#line 4576 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1615 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-4].nodes);}
#line 4582 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1616 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYERROR;} s = NULL; }
#line 4588 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1617 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                   actual_type = LEAF_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }
#line 4600 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1625 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {YYERROR;}
                                    s = NULL;
                                  }
#line 4608 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1628 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                               if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf", (yyvsp[0].i))) {
                                                 YYERROR;
                                               }
                                             }
                                           }
#line 4619 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1634 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "leaf", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }
#line 4630 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1640 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        }
                                      }
#line 4641 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1646 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }
#line 4651 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1651 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 4661 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1657 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                               if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYERROR;}
                                                               data_node = actual;
                                                               s=NULL;
                                                             }
                                                           }
#line 4672 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1664 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             if ((yyvsp[0].nodes).node.ptr_leaflist->flags & LYS_CONFIG_R) {
                                               /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                                * ignore oredering MASK - 0x7F
                                                */
                                             (yyvsp[0].nodes).node.ptr_leaflist->flags &= 0x7F;
                                           }
                                           if ((yyvsp[0].nodes).node.ptr_leaflist->max && (yyvsp[0].nodes).node.ptr_leaflist->min > (yyvsp[0].nodes).node.ptr_leaflist->max) {
                                             LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                                             YYERROR;
                                           }
                                           if (!((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                                             LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                                             YYERROR;
                                           } else {
                                             if (unres_schema_add_node(trg, unres, &(yyvsp[0].nodes).node.ptr_leaflist->type, UNRES_TYPE_DER,
                                                                      (struct lys_node *) (yyvsp[0].nodes).node.ptr_leaflist)) {
                                               YYERROR;
                                             }
                                           }
                                         }
                                       }
#line 4699 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 276:
#line 1688 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                               (yyval.nodes).node.ptr_leaflist = actual;
                               (yyval.nodes).node.flag = 0;
                               actual_type = LEAF_LIST_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 (yyval.nodes).node.ptr_leaflist->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_leaflist->features);
                                 if (!(yyval.nodes).node.ptr_leaflist->features) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 (yyval.nodes).node.ptr_leaflist->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_leaflist->must);
                                 if (!(yyval.nodes).node.ptr_leaflist->must) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               store_flags((struct lys_node *)(yyval.nodes).node.ptr_leaflist, size_arrays->node[size_arrays->next].flags, config_inherit);
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }
#line 4732 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1716 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }
#line 4738 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1718 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, unres, LEAF_LIST_KEYWORD)) {YYERROR;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                          }
                                        }
#line 4750 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1725 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && ((yyvsp[0].nodes).node.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_leaflist, "type", "leaf-list");
                            YYERROR;
                          }
                        }
#line 4760 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1730 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                   actual = (yyvsp[-2].nodes).node.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                 }
               }
#line 4771 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1736 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-4].nodes); }
#line 4777 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1737 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {YYERROR;} s = NULL; }
#line 4783 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1738 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                    }
                                  }
#line 4795 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1746 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "leaf-list", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }
#line 4806 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1752 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                              if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                                                YYERROR;
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
#line 4825 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 288:
#line 1766 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                              if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "max-elements", "leaf-list");
                                                YYERROR;
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
#line 4844 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 289:
#line 1780 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "ordered by", "leaf-list");
                                              YYERROR;
                                            }
                                            if ((yyvsp[0].i) & LYS_USERORDERED) {
                                              (yyvsp[-1].nodes).node.ptr_leaflist->flags |= LYS_USERORDERED;
                                            }
                                            (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }
#line 4861 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 290:
#line 1792 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "leaf-list", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }
#line 4872 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 291:
#line 1798 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                         }
#line 4882 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 292:
#line 1803 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }
#line 4892 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 293:
#line 1809 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }
#line 4903 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 294:
#line 1816 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                 if ((yyvsp[0].nodes).node.ptr_list->flags & LYS_CONFIG_R) {
                                   /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                    * ignore oredering MASK - 0x7F
                                    */
                                   (yyvsp[0].nodes).node.ptr_list->flags &= 0x7F;
                                 }
                                 if (((yyvsp[0].nodes).node.ptr_list->flags & LYS_CONFIG_W) && !(yyvsp[0].nodes).node.ptr_list->keys) {
                                   LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_list, "key", "list");
                                   YYERROR;
                                 }
                                 if ((yyvsp[0].nodes).node.ptr_list->keys && yang_read_key(trg, (yyvsp[0].nodes).node.ptr_list, unres)) {
                                   YYERROR;
                                 }
                                 if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                   LOGVAL(LYE_SPEC, LY_VLOG_LYS, (yyvsp[0].nodes).node.ptr_list, "data-def statement missing.");
                                   YYERROR;
                                 }
                                 if (yang_read_unique(trg, (yyvsp[0].nodes).node.ptr_list, unres)) {
                                   YYERROR;
                                 }
                               }
                             }
#line 4931 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 296:
#line 1841 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                          (yyval.nodes).node.ptr_list = actual;
                          (yyval.nodes).node.flag = 0;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).node.ptr_list->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_list->features);
                            if (!(yyval.nodes).node.ptr_list->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].must) {
                            (yyval.nodes).node.ptr_list->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).node.ptr_list->must);
                            if (!(yyval.nodes).node.ptr_list->must) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].tpdf) {
                            (yyval.nodes).node.ptr_list->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_list->tpdf);
                            if (!(yyval.nodes).node.ptr_list->tpdf) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].unique) {
                            (yyval.nodes).node.ptr_list->unique = calloc(size_arrays->node[size_arrays->next].unique, sizeof *(yyval.nodes).node.ptr_list->unique);
                            if (!(yyval.nodes).node.ptr_list->unique) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).node.ptr_list, size_arrays->node[size_arrays->next].flags, config_inherit);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                      }
#line 4977 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 297:
#line 1882 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_list; actual_type = LIST_KEYWORD; }
#line 4983 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 299:
#line 1884 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_list, s, unres, LIST_KEYWORD)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }
#line 4995 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 300:
#line 1891 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[(yyvsp[-1].nodes).index].must++;
                               }
                             }
#line 5007 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1899 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                if ((yyvsp[-1].nodes).node.ptr_list->keys) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "key", "list");
                                  YYERROR;
                                }
                                (yyvsp[-1].nodes).node.ptr_list->keys = (struct lys_node_leaf **)s;
                                (yyval.nodes) = (yyvsp[-1].nodes);
                                s=NULL;
                              }
                            }
#line 5022 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1909 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                   (yyvsp[-1].nodes).node.ptr_list->unique[(yyvsp[-1].nodes).node.ptr_list->unique_size++].expr = (const char **)s;
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                 }
                               }
#line 5035 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1917 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "list", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }
#line 5046 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1923 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "min-elements", "list");
                                           YYERROR;
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
#line 5065 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1937 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "max-elements", "list");
                                           YYERROR;
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
#line 5084 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1951 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                         LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "ordered by", "list");
                                         YYERROR;
                                       }
                                       if ((yyvsp[0].i) & LYS_USERORDERED) {
                                         (yyvsp[-1].nodes).node.ptr_list->flags |= LYS_USERORDERED;
                                       }
                                       (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                       (yyval.nodes) = (yyvsp[-1].nodes);
                                     }
                                   }
#line 5101 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 308:
#line 1963 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "list", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }
#line 5112 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 309:
#line 1969 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 5122 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 310:
#line 1974 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }
#line 5132 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 311:
#line 1979 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            actual = (yyvsp[-1].nodes).node.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                          }
                                        }
#line 5144 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 313:
#line 1987 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }
#line 5153 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 315:
#line 1992 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }
#line 5163 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 316:
#line 1997 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 5169 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 317:
#line 2000 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_CHOICE,sizeof(struct lys_node_choice)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }
#line 5183 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 320:
#line 2013 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                              if ((yyvsp[0].nodes).choice.s && ((yyvsp[0].nodes).choice.ptr_choice->flags & LYS_MAND_TRUE)) {
                                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "choice");
                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"default\" statement is forbidden on choices with \"mandatory\".");
                                YYERROR;
                              }
                              /* link default with the case */
                              if ((yyvsp[0].nodes).choice.s) {
                                if (unres_schema_add_str(trg, unres, (yyvsp[0].nodes).choice.ptr_choice, UNRES_CHOICE_DFLT, (yyvsp[0].nodes).choice.s) == -1) {
                                  YYERROR;
                                }
                                free((yyvsp[0].nodes).choice.s);
                              }
                            }
                          }
#line 5203 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 322:
#line 2030 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                            (yyval.nodes).choice.ptr_choice = actual;
                            (yyval.nodes).choice.s = NULL;
                            actual_type = CHOICE_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).choice.ptr_choice->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).choice.ptr_choice->features);
                              if (!(yyval.nodes).choice.ptr_choice->features) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).choice.ptr_choice, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }
#line 5229 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 323:
#line 2051 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).choice.ptr_choice; actual_type = CHOICE_KEYWORD; }
#line 5235 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 324:
#line 2052 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 5241 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 325:
#line 2053 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).choice.ptr_choice,s, unres, CHOICE_KEYWORD)) {
                                           if ((yyvsp[-1].nodes).choice.s) {
                                             free((yyvsp[-1].nodes).choice.s);
                                           }
                                           YYERROR;
                                         }
                                         s=NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }
#line 5259 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 326:
#line 2066 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      if ((yyvsp[-1].nodes).choice.s) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).choice.ptr_choice, "default", "choice");
                                        free((yyvsp[-1].nodes).choice.s);
                                        free(s);
                                        YYERROR;
                                      }
                                      (yyvsp[-1].nodes).choice.s = s;
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }
#line 5276 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 327:
#line 2078 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                           if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "choice", (yyvsp[0].i))) {
                                             YYERROR;
                                           }
                                         } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }
#line 5289 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 328:
#line 2086 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                      if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "choice", (yyvsp[0].i))) {
                                        YYERROR;
                                      }
                                    } else {
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }
                                  }
#line 5302 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 329:
#line 2094 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "choice", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        } else {
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }
#line 5315 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 330:
#line 2102 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                          if (yang_read_description(trg, (yyvsp[-1].nodes).choice.ptr_choice, s, "choice")) {
                                            free((yyvsp[-1].nodes).choice.s);
                                            YYERROR;
                                          }
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }
#line 5329 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 331:
#line 2111 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                        if (yang_read_reference(trg, (yyvsp[-1].nodes).choice.ptr_choice, s, "choice")) {
                                          free((yyvsp[-1].nodes).choice.s);
                                          YYERROR;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }
                                    }
#line 5343 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 332:
#line 2120 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }
#line 5355 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 333:
#line 2127 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 5361 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 341:
#line 2141 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 }
                                               }
#line 5372 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 345:
#line 2154 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                          (yyval.nodes).cs = actual;
                          actual_type = CASE_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).cs->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).cs->features);
                            if (!(yyval.nodes).cs->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).cs, size_arrays->node[size_arrays->next].flags, 1);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                      }
#line 5397 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 346:
#line 2174 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).cs; actual_type = CASE_KEYWORD; }
#line 5403 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 348:
#line 2176 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).cs, s, unres, CASE_KEYWORD)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }
#line 5415 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 349:
#line 2183 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "case", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }
#line 5426 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 350:
#line 2189 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 5436 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 351:
#line 2194 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }
#line 5446 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 352:
#line 2199 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }
#line 5455 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 354:
#line 2206 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_ANYXML,sizeof(struct lys_node_anyxml)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }
#line 5469 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 358:
#line 2222 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                            (yyval.nodes).anyxml = actual;
                            actual_type = ANYXML_KEYWORD;
                            if (size_arrays->node[size_arrays->next].if_features) {
                              (yyval.nodes).anyxml->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).anyxml->features);
                              if (!(yyval.nodes).anyxml->features) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            if (size_arrays->node[size_arrays->next].must) {
                              (yyval.nodes).anyxml->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).anyxml->must);
                              if (!(yyval.nodes).anyxml->features || !(yyval.nodes).anyxml->must) {
                                LOGMEM;
                                YYERROR;
                              }
                            }
                            store_flags((struct lys_node *)(yyval.nodes).anyxml, size_arrays->node[size_arrays->next].flags, config_inherit);
                            size_arrays->next++;
                          } else {
                            (yyval.nodes).index = size_arrays->size;
                            if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }
#line 5501 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 359:
#line 2249 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).anyxml; actual_type = ANYXML_KEYWORD; }
#line 5507 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 361:
#line 2251 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if (yang_read_if_feature(trg, (yyvsp[-1].nodes).anyxml, s, unres, ANYXML_KEYWORD)) {YYERROR;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                       }
                                     }
#line 5519 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 362:
#line 2258 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].must++;
                                 }
                               }
#line 5531 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 364:
#line 2266 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_CONFIG_MASK, "config", "anyxml", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        }
                                      }
#line 5542 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 365:
#line 2272 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                             if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_MAND_MASK, "mandatory", "anyxml", (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                           }
                                         }
#line 5553 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 366:
#line 2278 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                          if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "anyxml", (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                        }
                                      }
#line 5564 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 367:
#line 2284 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                      }
#line 5574 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 368:
#line 2289 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).anyxml, s, "anyxml")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 5584 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 369:
#line 2295 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_USES,sizeof(struct lys_node_uses)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     }
                                                   }
#line 5598 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 370:
#line 2304 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL) == -1) {
                          YYERROR;
                        }
                      }
                    }
#line 5609 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 373:
#line 2316 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                          (yyval.nodes).uses.ptr_uses = actual;
                          (yyval.nodes).uses.config_inherit = config_inherit;
                          actual_type = USES_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            (yyval.nodes).uses.ptr_uses->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).uses.ptr_uses->features);
                            if (!(yyval.nodes).uses.ptr_uses->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].refine) {
                            (yyval.nodes).uses.ptr_uses->refine = calloc(size_arrays->node[size_arrays->next].refine, sizeof *(yyval.nodes).uses.ptr_uses->refine);
                            if (!(yyval.nodes).uses.ptr_uses->refine) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].augment) {
                            (yyval.nodes).uses.ptr_uses->augment = calloc(size_arrays->node[size_arrays->next].augment, sizeof *(yyval.nodes).uses.ptr_uses->augment);
                            if (!(yyval.nodes).uses.ptr_uses->augment) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          store_flags((struct lys_node *)(yyval.nodes).uses.ptr_uses, size_arrays->node[size_arrays->next].flags, config_inherit);
                          size_arrays->next++;
                        } else {
                          (yyval.nodes).index = size_arrays->size;
                          if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                            LOGMEM;
                            YYERROR;
                          }
                        }
                      }
#line 5649 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 374:
#line 2351 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).uses.ptr_uses; actual_type = USES_KEYWORD; }
#line 5655 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 376:
#line 2353 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (yang_read_if_feature(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, unres, USES_KEYWORD)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                     }
                                   }
#line 5667 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 377:
#line 2360 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                        if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "uses", (yyvsp[0].i))) {
                                          YYERROR;
                                        }
                                      }
                                    }
#line 5678 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 378:
#line 2366 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 5688 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 379:
#line 2371 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).uses.ptr_uses, s, "uses")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }
#line 5698 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 380:
#line 2376 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                   actual = (yyvsp[-1].nodes).uses.ptr_uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[(yyvsp[-1].nodes).index].refine++;
                                 }
                               }
#line 5710 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 382:
#line 2384 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
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
#line 5727 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 384:
#line 2399 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s))) {
                                                     YYERROR;
                                                   }
                                                   s = NULL;
                                                 }
                                               }
#line 5739 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 390:
#line 2418 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                  (yyval.nodes).refine = actual;
                                  actual_type = REFINE_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].must) {
                                    (yyval.nodes).refine->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *(yyval.nodes).refine->must);
                                    if (!(yyval.nodes).refine->must) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                    (yyval.nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
                                  }
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYERROR;
                                  }
                                }
                              }
#line 5764 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 391:
#line 2438 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         actual = (yyvsp[-2].nodes).refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                       }
                                     }
#line 5776 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 392:
#line 2445 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             if ((yyvsp[-1].nodes).refine->target_type) {
                                               if ((yyvsp[-1].nodes).refine->target_type & LYS_CONTAINER) {
                                                 if ((yyvsp[-1].nodes).refine->mod.presence) {
                                                   LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "presence", "refine");
                                                   free(s);
                                                   YYERROR;
                                                 }
                                                 (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                                 (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                               } else {
                                                 free(s);
                                                 LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "presence", "refine");
                                                 LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                 YYERROR;
                                               }
                                             } else {
                                               (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                               (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }
#line 5805 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 393:
#line 2469 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if ((yyvsp[-1].nodes).refine->target_type) {
                                              if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                                if ((yyvsp[-1].nodes).refine->mod.dflt) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "default", "refine");
                                                  free(s);
                                                  YYERROR;
                                                }
                                                (yyvsp[-1].nodes).refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                              } else {
                                                free(s);
                                                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                YYERROR;
                                              }
                                            } else {
                                              (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE;
                                              (yyvsp[-1].nodes).refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }
#line 5834 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 394:
#line 2493 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                           if ((yyvsp[-1].nodes).refine->target_type) {
                                             if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST)) {
                                               (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST);
                                               if (yang_check_flags(&(yyvsp[-1].nodes).refine->flags, LYS_CONFIG_MASK, "config", "refine", (yyvsp[0].i))) {
                                                 YYERROR;
                                               }
                                             } else {
                                               LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "config", "refine");
                                               LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                               YYERROR;
                                             }
                                           } else {
                                             (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST;
                                             (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                           }
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }
#line 5858 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 395:
#line 2512 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                              if ((yyvsp[-1].nodes).refine->target_type) {
                                                if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML)) {
                                                  (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML);
                                                  if (yang_check_flags(&(yyvsp[-1].nodes).refine->flags, LYS_MAND_MASK, "mandatory", "refine", (yyvsp[0].i))) {
                                                    YYERROR;
                                                  }
                                                } else {
                                                  LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "refine");
                                                  LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                  YYERROR;
                                                }
                                              } else {
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYXML;
                                                (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }
#line 5882 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 396:
#line 2531 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                 if ((yyvsp[-1].nodes).refine->target_type) {
                                                   if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     /* magic - bit 3 in flags means min set */
                                                     if ((yyvsp[-1].nodes).refine->flags & 0x04) {
                                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                       YYERROR;
                                                     }
                                                     (yyvsp[-1].nodes).refine->flags |= 0x04;
                                                     (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                   } else {
                                                     LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                     LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                     YYERROR;
                                                   }
                                                 } else {
                                                   (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   /* magic - bit 3 in flags means min set */
                                                   (yyvsp[-1].nodes).refine->flags |= 0x04;
                                                   (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }
#line 5912 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 397:
#line 2556 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                 if ((yyvsp[-1].nodes).refine->target_type) {
                                                   if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     /* magic - bit 4 in flags means max set */
                                                     if ((yyvsp[-1].nodes).refine->flags & 0x08) {
                                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                       YYERROR;
                                                     }
                                                     (yyvsp[-1].nodes).refine->flags |= 0x08;
                                                     (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                   } else {
                                                     LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                     LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                     YYERROR;
                                                   }
                                                 } else {
                                                   (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   /* magic - bit 4 in flags means max set */
                                                   (yyvsp[-1].nodes).refine->flags |= 0x08;
                                                   (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }
#line 5942 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 398:
#line 2581 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                            }
#line 5952 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 399:
#line 2586 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine")) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                          }
#line 5962 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 400:
#line 2592 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s))) {
                                                                  YYERROR;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              }
                                                            }
#line 5975 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 401:
#line 2601 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYERROR;
                                          }
                                        }
#line 5985 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 405:
#line 2612 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s))) {
                                                        YYERROR;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    }
                                                  }
#line 5998 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 406:
#line 2621 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (!((yyvsp[0].nodes).node.flag & LYS_DATADEF)){
                                         LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "data-def or case", "augment");
                                         YYERROR;
                                       }
                                       if (unres_schema_add_node(trg, unres, actual, UNRES_AUGMENT, NULL) == -1) {
                                         YYERROR;
                                       }
                                     }
                                   }
#line 6013 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 408:
#line 2633 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                             (yyval.nodes).node.ptr_augment = actual;
                             (yyval.nodes).node.flag = 0;
                             actual_type = AUGMENT_KEYWORD;
                             if (size_arrays->node[size_arrays->next].if_features) {
                               (yyval.nodes).node.ptr_augment->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_augment->features);
                               if (!(yyval.nodes).node.ptr_augment->features) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                             config_inherit = DISABLE_INHERIT;
                             size_arrays->next++;
                           } else {
                             (yyval.nodes).index = size_arrays->size;
                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                               LOGMEM;
                               YYERROR;
                             }
                           }
                         }
#line 6039 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 409:
#line 2654 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_augment; actual_type = AUGMENT_KEYWORD; }
#line 6045 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 411:
#line 2656 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                          if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_augment, s, unres, AUGMENT_KEYWORD)) {YYERROR;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                        }
                                      }
#line 6057 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 412:
#line 2663 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      if (yang_check_flags(&(yyvsp[-1].nodes).node.ptr_augment->flags, LYS_STATUS_MASK, "status", "augment", (yyvsp[0].i))) {
                                        YYERROR;
                                      }
                                    }
                                  }
#line 6068 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 413:
#line 2669 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                       }
#line 6078 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 414:
#line 2674 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_augment, s, "augment")) {
                                         YYERROR;
                                       }
                                       s = NULL;
                                     }
#line 6088 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 415:
#line 2679 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                        actual = (yyvsp[-1].nodes).node.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }
#line 6100 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 416:
#line 2686 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 6106 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 417:
#line 2687 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                    actual = (yyvsp[-1].nodes).node.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }
#line 6118 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 418:
#line 2694 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 6124 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 421:
#line 2701 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                 if (!(actual = yang_read_node(trg, NULL, s, LYS_RPC, sizeof(struct lys_node_rpc)))) {
                                                   YYERROR;
                                                 }
                                                 data_node = actual;
                                                 s = NULL;
                                               }
                                               config_inherit = DISABLE_INHERIT;
                                             }
#line 6138 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 422:
#line 2710 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { config_inherit = ENABLE_INHERIT; }
#line 6144 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 425:
#line 2718 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                         (yyval.nodes).node.ptr_rpc = actual;
                         (yyval.nodes).node.flag = 0;
                         actual_type = RPC_KEYWORD;
                         if (size_arrays->node[size_arrays->next].if_features) {
                           (yyval.nodes).node.ptr_rpc->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).node.ptr_rpc->features);
                           if (!(yyval.nodes).node.ptr_rpc->features) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                         if (size_arrays->node[size_arrays->next].tpdf) {
                           (yyval.nodes).node.ptr_rpc->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_rpc->tpdf);
                           if (!(yyval.nodes).node.ptr_rpc->tpdf) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                         store_flags((struct lys_node *)(yyval.nodes).node.ptr_rpc, size_arrays->node[size_arrays->next].flags, 0);
                         size_arrays->next++;
                       } else {
                         (yyval.nodes).index = size_arrays->size;
                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                           LOGMEM;
                           YYERROR;
                         }
                       }
                     }
#line 6177 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 426:
#line 2746 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      if (yang_read_if_feature(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, unres, RPC_KEYWORD)) {YYERROR;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                    }
                                  }
#line 6189 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 427:
#line 2753 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                       if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "rpc", (yyvsp[0].i))) {
                                         YYERROR;
                                       }
                                     }
                                   }
#line 6200 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 428:
#line 2759 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                       YYERROR;
                                     }
                                     s = NULL;
                                   }
#line 6210 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 429:
#line 2764 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc")) {
                                     YYERROR;
                                   }
                                   s = NULL;
                                 }
#line 6220 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 430:
#line 2769 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                           actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                         }
                                       }
#line 6232 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 432:
#line 2777 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }
#line 6241 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 434:
#line 2782 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].nodes).node.flag & LYS_RPC_INPUT) {
                                 LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "input", "rpc");
                                 YYERROR;
                               }
                               (yyvsp[-1].nodes).node.flag |= LYS_RPC_INPUT;
                               actual = (yyvsp[-1].nodes).node.ptr_rpc;
                               actual_type = RPC_KEYWORD;
                               data_node = actual;
                             }
#line 6255 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 435:
#line 2791 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 6261 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 436:
#line 2792 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].nodes).node.flag & LYS_RPC_OUTPUT) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "output", "rpc");
                                  YYERROR;
                                }
                                (yyvsp[-1].nodes).node.flag |= LYS_RPC_OUTPUT;
                                actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                actual_type = RPC_KEYWORD;
                                data_node = actual;
                              }
#line 6275 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 437:
#line 2801 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 6281 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 438:
#line 2803 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                     if (!(actual = yang_read_node(trg, actual, NULL, LYS_INPUT, sizeof(struct lys_node_rpc_inout)))) {
                                      YYERROR;
                                     }
                                     data_node = actual;
                                   }
                                 }
#line 6293 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 439:
#line 2811 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "input");
                                          YYERROR;
                                        }
                                      }
#line 6303 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 441:
#line 2818 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                  (yyval.nodes).node.ptr_inout = actual;
                                  (yyval.nodes).node.flag = 0;
                                  actual_type = INPUT_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).node.ptr_inout->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).node.ptr_inout->tpdf);
                                    if (!(yyval.nodes).node.ptr_inout->tpdf) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYERROR;
                                  }
                                }
                              }
#line 6328 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 442:
#line 2838 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).node.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }
#line 6340 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 444:
#line 2846 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).node.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }
#line 6349 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 446:
#line 2851 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             actual = (yyvsp[-1].nodes).node.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }
#line 6361 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 447:
#line 2858 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.nodes) = (yyvsp[-3].nodes); }
#line 6367 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 448:
#line 2861 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (!(actual = yang_read_node(trg, actual, NULL, LYS_OUTPUT, sizeof(struct lys_node_rpc_inout)))) {
                                        YYERROR;
                                       }
                                       data_node = actual;
                                     }
                                   }
#line 6379 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 449:
#line 2869 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !((yyvsp[0].nodes).node.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, "data-def", "output");
                                           YYERROR;
                                         }
                                       }
#line 6389 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 451:
#line 2876 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYERROR;
                                                                   }
                                                                   data_node = actual;
                                                                 }
                                                                 config_inherit = DISABLE_INHERIT;
                                                               }
#line 6402 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 452:
#line 2884 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { config_inherit = ENABLE_INHERIT; }
#line 6408 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 453:
#line 2886 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                          size_arrays->next++;
                        }
                      }
#line 6417 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 455:
#line 2895 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                  (yyval.nodes).notif = actual;
                                  actual_type = NOTIFICATION_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].if_features) {
                                    (yyval.nodes).notif->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *(yyval.nodes).notif->features);
                                    if (!(yyval.nodes).notif->features) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    (yyval.nodes).notif->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *(yyval.nodes).notif->tpdf);
                                    if (!(yyval.nodes).notif->tpdf) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  store_flags((struct lys_node *)(yyval.nodes).notif, size_arrays->node[size_arrays->next].flags, 0);
                                  size_arrays->next++;
                                } else {
                                  (yyval.nodes).index = size_arrays->size;
                                  if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                    LOGMEM;
                                    YYERROR;
                                  }
                                }
                              }
#line 6449 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 456:
#line 2922 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                               if (yang_read_if_feature(trg, (yyvsp[-1].nodes).notif, s, unres, NOTIFICATION_KEYWORD)) {YYERROR;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[(yyvsp[-1].nodes).index].if_features++;
                                             }
                                           }
#line 6461 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 457:
#line 2929 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!read_all) {
                                                if (yang_check_flags(&size_arrays->node[(yyvsp[-1].nodes).index].flags, LYS_STATUS_MASK, "status", "notification", (yyvsp[0].i))) {
                                                  YYERROR;
                                                }
                                              }
                                            }
#line 6472 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 458:
#line 2935 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                            }
#line 6482 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 459:
#line 2940 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification")) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                          }
#line 6492 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 460:
#line 2945 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                    actual = (yyvsp[-1].nodes).notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[(yyvsp[-1].nodes).index].tpdf++;
                                                  }
                                                }
#line 6504 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 462:
#line 2953 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }
#line 6513 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 464:
#line 2958 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }
#line 6522 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 466:
#line 2965 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s))) {
                                                              YYERROR;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            }
                                                        }
#line 6535 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 467:
#line 2974 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if (actual_type == DEVIATION_KEYWORD) {
                                              LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                                              YYERROR;
                                            }
                                            if (yang_check_deviation(trg, actual, unres)) {
                                              YYERROR;
                                            }
                                            free((yyvsp[0].nodes).deviation);
                                          }
                                        }
#line 6551 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 469:
#line 2987 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                               (yyval.nodes).deviation = actual;
                               actual_type = DEVIATION_KEYWORD;
                               if (size_arrays->node[size_arrays->next].deviate) {
                                 (yyval.nodes).deviation->deviation->deviate = calloc(size_arrays->node[size_arrays->next].deviate, sizeof *(yyval.nodes).deviation->deviation->deviate);
                                 if (!(yyval.nodes).deviation->deviation->deviate) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               size_arrays->next++;
                             } else {
                               (yyval.nodes).index = size_arrays->size;
                               if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                           }
#line 6575 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 470:
#line 3006 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                             free((yyvsp[-1].nodes).deviation);
                                             YYERROR;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
#line 6587 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 471:
#line 3013 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, (yyvsp[-1].nodes).deviation->deviation, s, "deviation")) {
                                           free((yyvsp[-1].nodes).deviation);
                                           YYERROR;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
#line 6599 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 472:
#line 3020 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                                  actual = (yyvsp[-3].nodes).deviation;
                                                                  actual_type = DEVIATE_KEYWORD;
                                                                  (yyval.nodes) = (yyvsp[-3].nodes);
                                                                } else {
                                                                  /* count of deviate statemenet */
                                                                  size_arrays->node[(yyvsp[-3].nodes).index].deviate++;
                                                                }
                                                              }
#line 6613 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 475:
#line 3034 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_deviate_unsupported(actual)) {
                       YYERROR;
                     }
                   }
#line 6622 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 481:
#line 3047 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD)) {
                                         YYERROR;
                                       }
                                     }
#line 6631 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 485:
#line 3058 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                 (yyval.nodes).deviation = actual;
                                 actual_type = ADD_KEYWORD;
                                 if (size_arrays->node[size_arrays->next].must) {
                                    if (yang_read_deviate_must(actual, size_arrays->node[size_arrays->next].must)) {
                                      YYERROR;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].unique) {
                                    if (yang_read_deviate_unique(actual, size_arrays->node[size_arrays->next].unique)) {
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                               } else {
                                 (yyval.nodes).index = size_arrays->size;
                                 if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                             }
#line 6658 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 486:
#line 3080 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }
                                     }
#line 6671 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 487:
#line 3088 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                        actual = (yyvsp[-2].nodes).deviation;
                                        actual_type = ADD_KEYWORD;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      } else {
                                        size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                      }
                                    }
#line 6684 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 488:
#line 3096 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                          struct lys_node_list *list;

                                          list = (struct lys_node_list *)(yyvsp[-1].nodes).deviation->target;
                                          if (yang_fill_unique(trg, list, &list->unique[list->unique_size], s, NULL)) {
                                            list->unique_size++;
                                            YYERROR;
                                          }
                                          list->unique_size++;
                                          free(s);
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        } else {
                                          size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                        }
                                      }
#line 6705 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 489:
#line 3112 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }
                                       }
#line 6718 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 490:
#line 3120 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                          if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                            YYERROR;
                                          }
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }
                                      }
#line 6730 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 491:
#line 3127 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                               YYERROR;
                                             }
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }
#line 6742 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 492:
#line 3134 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                if ((yyvsp[-1].nodes).deviation->deviate->min_set) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                  YYERROR;
                                                }
                                                if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 0)) {
                                                  YYERROR;
                                                }
                                                (yyval.nodes) =  (yyvsp[-1].nodes);
                                              }
                                            }
#line 6758 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 493:
#line 3145 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                if ((yyvsp[-1].nodes).deviation->deviate->max_set) {
                                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                  YYERROR;
                                                }
                                                if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 1)) {
                                                  YYERROR;
                                                }
                                                (yyval.nodes) =  (yyvsp[-1].nodes);
                                              }
                                            }
#line 6774 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 494:
#line 3157 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL)) {
                                               YYERROR;
                                             }
                                           }
#line 6783 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 498:
#line 3168 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = DELETE_KEYWORD;
                                    if (size_arrays->node[size_arrays->next].must) {
                                      if (yang_read_deviate_must(actual, size_arrays->node[size_arrays->next].must)) {
                                        YYERROR;
                                      }
                                    }
                                    if (size_arrays->node[size_arrays->next].unique) {
                                      if (yang_read_deviate_unique(actual, size_arrays->node[size_arrays->next].unique)) {
                                        YYERROR;
                                      }
                                    }
                                    size_arrays->next++;
                                  } else {
                                    (yyval.nodes).index = size_arrays->size;
                                    if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                }
#line 6810 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 499:
#line 3190 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }
                                        }
#line 6823 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 500:
#line 3198 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                           if (yang_check_deviate_must(trg->ctx, (yyvsp[-2].nodes).deviation)) {
                                             YYERROR;
                                           }
                                           actual = (yyvsp[-2].nodes).deviation;
                                           actual_type = DELETE_KEYWORD;
                                           (yyval.nodes) = (yyvsp[-2].nodes);
                                         } else {
                                           size_arrays->node[(yyvsp[-2].nodes).index].must++;
                                         }
                                       }
#line 6839 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 501:
#line 3209 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             if (yang_check_deviate_unique(trg, (yyvsp[-1].nodes).deviation, s)) {
                                               YYERROR;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           } else {
                                             size_arrays->node[(yyvsp[-1].nodes).index].unique++;
                                           }
                                         }
#line 6854 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 502:
#line 3219 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }
#line 6867 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 503:
#line 3228 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL)) {
                                                 YYERROR;
                                               }
                                             }
#line 6876 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 507:
#line 3239 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                    (yyval.nodes).deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }
#line 6886 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 508:
#line 3244 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, (yyvsp[-2].nodes).deviation->deviate->type, UNRES_TYPE_DER, (yyvsp[-2].nodes).deviation->target)) {
                                              YYERROR;
                                            }
                                          }
                                        }
#line 6897 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 509:
#line 3250 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                               YYERROR;
                                             }
                                             s = NULL;
                                             (yyval.nodes) = (yyvsp[-1].nodes);
                                           }
                                         }
#line 6910 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 510:
#line 3258 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                               if (yang_read_deviate_default(trg->ctx, (yyvsp[-1].nodes).deviation, s)) {
                                                 YYERROR;
                                               }
                                               s = NULL;
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }
                                           }
#line 6923 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 511:
#line 3266 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                              if (yang_read_deviate_config((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                YYERROR;
                                              }
                                              (yyval.nodes) = (yyvsp[-1].nodes);
                                            }
                                          }
#line 6935 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 512:
#line 3273 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                 if (yang_read_deviate_mandatory((yyvsp[-1].nodes).deviation, (yyvsp[0].i))) {
                                                   YYERROR;
                                                 }
                                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                               }
                                             }
#line 6947 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 513:
#line 3280 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                    if ((yyvsp[-1].nodes).deviation->deviate->min_set) {
                                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviate");
                                                      YYERROR;
                                                    }
                                                    if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 0)) {
                                                      YYERROR;
                                                    }
                                                    (yyval.nodes) =  (yyvsp[-1].nodes);
                                                  }
                                                }
#line 6963 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 514:
#line 3291 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                    if ((yyvsp[-1].nodes).deviation->deviate->max_set) {
                                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviate");
                                                      YYERROR;
                                                    }
                                                    if (yang_read_deviate_minmax((yyvsp[-1].nodes).deviation, (yyvsp[0].uint), 1)) {
                                                      YYERROR;
                                                    }
                                                    (yyval.nodes) =  (yyvsp[-1].nodes);
                                                  }
                                                }
#line 6979 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 515:
#line 3303 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && !(actual=yang_read_when(trg, actual, actual_type, s))) {YYERROR;} s=NULL; actual_type=WHEN_KEYWORD;}
#line 6985 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 520:
#line 3312 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_description(trg, actual, s, "when")) {
                                        YYERROR;
                                      }
                                      s = NULL;
                                    }
#line 6995 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 521:
#line 3317 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && yang_read_reference(trg, actual, s, "when")) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                  }
#line 7005 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 522:
#line 3323 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 7011 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 523:
#line 3325 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }
#line 7017 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 524:
#line 3326 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }
#line 7023 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 525:
#line 3327 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-2].i); }
#line 7029 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 526:
#line 3329 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_CONFIG_W; }
#line 7035 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 527:
#line 3330 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_CONFIG_R; }
#line 7041 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 528:
#line 3331 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                  if (!strcmp(s, "true")) {
                    (yyval.i) = LYS_CONFIG_W;
                  } else if (!strcmp(s, "false")) {
                    (yyval.i) = LYS_CONFIG_R;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "config");
                    free(s);
                    YYERROR;
                  }
                  free(s);
                  s = NULL;
                }
              }
#line 7060 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 529:
#line 3346 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 7066 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 530:
#line 3348 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }
#line 7072 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 531:
#line 3349 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { read_all = (read_all) ? LY_READ_ONLY_SIZE : LY_READ_ALL; }
#line 7078 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 532:
#line 3350 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-2].i); }
#line 7084 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 533:
#line 3352 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_MAND_TRUE; }
#line 7090 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 534:
#line 3353 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_MAND_FALSE; }
#line 7096 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 535:
#line 3354 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                  if (!strcmp(s, "true")) {
                    (yyval.i) = LYS_MAND_TRUE;
                  } else if (!strcmp(s, "false")) {
                    (yyval.i) = LYS_MAND_FALSE;
                  } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "mandatory");
                    free(s);
                    YYERROR;
                  }
                  free(s);
                  s = NULL;
                }
              }
#line 7115 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 537:
#line 3371 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[-1].uint); }
#line 7121 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 538:
#line 3373 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[-1].uint); }
#line 7127 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 539:
#line 3374 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                  if (strlen(s) == 1 && s[0] == '0') {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(yyget_text(scanner), &endptr, 10);
                    if (val > UINT32_MAX || *endptr) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "min-elements");
                        free(s);
                        YYERROR;
                    }
                    (yyval.uint) = (uint32_t) val;
                  }
                  free(s);
                  s = NULL;
                }
              }
#line 7152 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 540:
#line 3395 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[-1].uint); }
#line 7158 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 541:
#line 3397 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = 0; }
#line 7164 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 542:
#line 3398 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[-1].uint); }
#line 7170 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 543:
#line 3399 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                  if (!strcmp(s, "unbounded")) {
                    (yyval.uint) = 0;
                  } else {
                    /* convert it to uint32_t */
                    uint64_t val;
                    char *endptr;

                    val = strtoul(yyget_text(scanner), &endptr, 10);
                    if (val > UINT32_MAX || *endptr) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, s, "max-elements");
                        free(s);
                        YYERROR;
                    }
                    (yyval.uint) = (uint32_t) val;
                  }
                  free(s);
                  s = NULL;
                }
              }
#line 7195 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 544:
#line 3420 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 7201 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 545:
#line 3422 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_USERORDERED; }
#line 7207 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 546:
#line 3423 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = LYS_SYSTEMORDERED; }
#line 7213 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 547:
#line 3424 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (!strcmp(s, "user")) {
                  (yyval.i) = LYS_USERORDERED;
                } else if (!strcmp(s, "system")) {
                  (yyval.i) = LYS_SYSTEMORDERED;
                } else {
                  free(s);
                  YYERROR;
                }
                free(s);
                s=NULL;
              }
#line 7229 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 548:
#line 3436 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                       if (!(actual=yang_read_must(trg, actual, s, actual_type))) {YYERROR;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }
#line 7240 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 558:
#line 3460 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all){
                                 s = strdup(yyget_text(scanner));
                                 if (!s) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                             }
#line 7253 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 561:
#line 3472 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                 s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                 if (!s) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                                 strcat(s," ");
                                 strcat(s, yyget_text(scanner));
                                }
                             }
#line 7268 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 564:
#line 3486 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                          (yyval.v) = actual;
                          if (!(actual = yang_read_range(trg, actual, s))) {
                             YYERROR;
                          }
                          actual_type = RANGE_KEYWORD;
                          s = NULL;
                        }
                      }
#line 7282 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 565:
#line 3496 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                if (s) {
                                                  s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYERROR;
                                                  }
                                                  strcat(s,"/");
                                                  strcat(s, yyget_text(scanner));
                                                } else {
                                                  s = malloc(yyget_leng(scanner) + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYERROR;
                                                  }
                                                  s[0]='/';
                                                  memcpy(s + 1, yyget_text(scanner), yyget_leng(scanner) + 1);
                                                }
                                              }
                                            }
#line 7307 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 569:
#line 3523 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all)  {
                                              if (s) {
                                                s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 1);
                                                if (!s) {
                                                  LOGMEM;
                                                  YYERROR;
                                                }
                                                strcat(s, yyget_text(scanner));
                                              } else {
                                                s = strdup(yyget_text(scanner));
                                                if (!s) {
                                                  LOGMEM;
                                                  YYERROR;
                                                }
                                              }
                                            }
                                          }
#line 7329 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 571:
#line 3542 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { tmp_s = yyget_text(scanner); }
#line 7335 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 572:
#line 3542 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                                     s = strdup(tmp_s);
                                                     if (!s) {
                                                       LOGMEM;
                                                       YYERROR;
                                                     }
                                                     s[strlen(s) - 1] = '\0';
                                                   }
                                                 }
#line 7349 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 573:
#line 3551 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { tmp_s = yyget_text(scanner); }
#line 7355 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 574:
#line 3551 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                           s = strdup(tmp_s);
                                           if (!s) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           s[strlen(s) - 1] = '\0';
                                         }
                                       }
#line 7369 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 598:
#line 3604 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /* convert it to uint32_t */
                                                unsigned long val;

                                                val = strtoul(yyget_text(scanner), NULL, 10);
                                                if (val > UINT32_MAX) {
                                                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Converted number is very long.");
                                                    YYERROR;
                                                }
                                                (yyval.uint) = (uint32_t) val;
                                             }
#line 7384 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 599:
#line 3615 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = 0; }
#line 7390 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 600:
#line 3616 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[0].uint); }
#line 7396 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 601:
#line 3619 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 7402 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 602:
#line 3620 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { /* convert it to int32_t */
               int64_t val;

               val = strtoll(yyget_text(scanner), NULL, 10);
               if (val < INT32_MIN || val > INT32_MAX) {
                   LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The number is not in the correct range (INT32_MIN..INT32_MAX): \"%d\"",val);
                   YYERROR;
               }
               (yyval.i) = (int32_t) val;
             }
#line 7417 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 608:
#line 3639 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all && lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYERROR;
                }
              }
#line 7427 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 613:
#line 3651 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                 char *tmp;

                 if ((tmp = strchr(s, ':'))) {
                   *tmp = '\0';
                   /* check prefix */
                   if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYERROR;
                   }
                   /* check identifier */
                   if (lyp_check_identifier(tmp + 1, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYERROR;
                   }
                   *tmp = ':';
                 } else {
                   /* check identifier */
                   if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                     free(s);
                     YYERROR;
                   }
                 }
               }
             }
#line 7457 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 619:
#line 3686 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, yyget_text(scanner))) {
                                         YYERROR;
                                       }
                                     }
                                   }
#line 7468 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 641:
#line 3729 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all){
                    s = strdup(yyget_text(scanner));
                    if (!s) {
                      LOGMEM;
                      YYERROR;
                    }
                  }
                }
#line 7481 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 729:
#line 3828 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                            s = strdup(yyget_text(scanner));
                            if (!s) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }
#line 7494 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;

  case 730:
#line 3837 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1646  */
    { if (read_all) {
                                      s = strdup(yyget_text(scanner));
                                      if (!s) {
                                        LOGMEM;
                                        YYERROR;
                                      }
                                    }
                                  }
#line 7507 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
    break;


#line 7511 "/home/xvican01/Documents/bc/libyang/src/parser_yang_bis.c" /* yacc.c:1646  */
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
      yyerror (scanner, module, submodule, unres, size_arrays, read_all, YY_("syntax error"));
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
        yyerror (scanner, module, submodule, unres, size_arrays, read_all, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



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
                      yytoken, &yylval, scanner, module, submodule, unres, size_arrays, read_all);
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, scanner, module, submodule, unres, size_arrays, read_all);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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
  yyerror (scanner, module, submodule, unres, size_arrays, read_all, YY_("memory exhausted"));
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
                  yytoken, &yylval, scanner, module, submodule, unres, size_arrays, read_all);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, scanner, module, submodule, unres, size_arrays, read_all);
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
#line 3846 "/home/xvican01/Documents/bc/libyang/src/yang.y" /* yacc.c:1906  */


void yyerror(void *scanner, ...){

  LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
  free(s);
}
