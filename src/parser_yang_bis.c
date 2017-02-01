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
#define YYLAST   3098

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  350
/* YYNRULES -- Number of rules.  */
#define YYNRULES  773
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1225

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
       0,   271,   271,   272,   274,   297,   300,   302,   301,   325,
     336,   346,   356,   357,   363,   368,   371,   382,   392,   405,
     406,   412,   414,   418,   420,   424,   426,   432,   433,   434,
     437,   442,   449,   450,   451,   462,   473,   480,   487,   489,
     490,   494,   495,   506,   517,   524,   528,   538,   554,   561,
     565,   571,   572,   577,   582,   587,   593,   597,   599,   603,
     605,   609,   611,   615,   617,   630,   634,   635,   637,   638,
     643,   644,   649,   656,   656,   663,   669,   717,   718,   721,
     722,   723,   724,   725,   726,   727,   728,   729,   730,   733,
     744,   749,   750,   754,   755,   756,   762,   767,   773,   782,
     784,   785,   789,   794,   795,   797,   798,   799,   812,   817,
     819,   820,   821,   822,   837,   848,   850,   851,   866,   867,
     876,   882,   887,   893,   897,   899,   912,   914,   915,   940,
     941,   955,   968,   974,   979,   985,   989,   991,  1040,  1050,
    1053,  1058,  1059,  1065,  1069,  1074,  1081,  1083,  1089,  1090,
    1095,  1130,  1131,  1134,  1135,  1139,  1145,  1158,  1159,  1160,
    1161,  1162,  1164,  1181,  1186,  1192,  1193,  1209,  1213,  1221,
    1222,  1227,  1239,  1244,  1249,  1254,  1260,  1267,  1280,  1281,
    1285,  1286,  1296,  1301,  1306,  1311,  1317,  1321,  1332,  1344,
    1345,  1348,  1356,  1365,  1366,  1381,  1382,  1394,  1400,  1404,
    1409,  1415,  1420,  1430,  1431,  1446,  1451,  1452,  1457,  1461,
    1463,  1468,  1470,  1471,  1472,  1485,  1497,  1498,  1500,  1508,
    1518,  1519,  1534,  1535,  1547,  1553,  1558,  1563,  1569,  1574,
    1584,  1585,  1601,  1605,  1607,  1611,  1613,  1617,  1619,  1623,
    1625,  1632,  1635,  1636,  1641,  1644,  1650,  1655,  1660,  1663,
    1666,  1669,  1672,  1680,  1681,  1682,  1683,  1684,  1685,  1686,
    1687,  1690,  1697,  1700,  1701,  1724,  1727,  1727,  1731,  1736,
    1736,  1740,  1745,  1751,  1757,  1762,  1767,  1767,  1772,  1772,
    1777,  1777,  1786,  1786,  1790,  1790,  1796,  1833,  1841,  1845,
    1845,  1849,  1854,  1854,  1859,  1864,  1864,  1868,  1873,  1879,
    1885,  1891,  1896,  1902,  1909,  1961,  1965,  1965,  1969,  1975,
    1975,  1980,  1991,  1996,  1996,  2000,  2006,  2019,  2032,  2042,
    2048,  2053,  2059,  2066,  2110,  2114,  2114,  2118,  2124,  2124,
    2128,  2137,  2143,  2149,  2162,  2175,  2185,  2191,  2196,  2201,
    2201,  2205,  2205,  2210,  2210,  2215,  2215,  2224,  2224,  2231,
    2238,  2241,  2242,  2262,  2266,  2266,  2270,  2276,  2286,  2293,
    2300,  2307,  2313,  2319,  2319,  2325,  2326,  2329,  2330,  2331,
    2332,  2333,  2334,  2335,  2342,  2349,  2352,  2353,  2367,  2370,
    2370,  2374,  2379,  2385,  2390,  2395,  2395,  2402,  2410,  2413,
    2421,  2424,  2425,  2448,  2451,  2451,  2455,  2461,  2461,  2465,
    2472,  2479,  2486,  2491,  2497,  2504,  2507,  2508,  2540,  2543,
    2543,  2547,  2552,  2558,  2563,  2568,  2568,  2572,  2572,  2578,
    2579,  2581,  2591,  2593,  2594,  2628,  2631,  2645,  2669,  2691,
    2742,  2759,  2776,  2797,  2818,  2823,  2829,  2830,  2833,  2845,
    2850,  2851,  2853,  2861,  2866,  2869,  2869,  2873,  2878,  2884,
    2889,  2894,  2894,  2899,  2899,  2904,  2904,  2913,  2913,  2919,
    2931,  2934,  2941,  2944,  2945,  2969,  2973,  2979,  2985,  2990,
    2995,  2995,  2999,  2999,  3004,  3004,  3014,  3014,  3025,  3025,
    3061,  3064,  3064,  3068,  3068,  3072,  3072,  3077,  3077,  3083,
    3083,  3119,  3125,  3127,  3128,  3162,  3165,  3165,  3169,  3174,
    3180,  3185,  3190,  3190,  3194,  3194,  3199,  3199,  3205,  3214,
    3232,  3235,  3241,  3247,  3252,  3253,  3255,  3260,  3263,  3267,
    3269,  3270,  3271,  3273,  3273,  3279,  3280,  3312,  3315,  3321,
    3325,  3331,  3337,  3344,  3351,  3359,  3368,  3368,  3374,  3375,
    3407,  3410,  3416,  3420,  3426,  3433,  3433,  3439,  3440,  3454,
    3457,  3460,  3466,  3472,  3479,  3486,  3494,  3503,  3510,  3512,
    3513,  3517,  3518,  3523,  3529,  3534,  3536,  3537,  3538,  3551,
    3556,  3558,  3559,  3560,  3573,  3577,  3579,  3584,  3586,  3587,
    3607,  3612,  3614,  3615,  3616,  3636,  3641,  3643,  3644,  3645,
    3657,  3720,  3722,  3723,  3728,  3732,  3734,  3735,  3737,  3738,
    3740,  3744,  3746,  3746,  3753,  3756,  3764,  3783,  3785,  3786,
    3789,  3789,  3806,  3806,  3813,  3813,  3820,  3823,  3825,  3827,
    3828,  3830,  3832,  3834,  3835,  3837,  3839,  3840,  3842,  3843,
    3845,  3847,  3850,  3854,  3856,  3857,  3859,  3860,  3862,  3864,
    3875,  3876,  3879,  3880,  3891,  3892,  3894,  3895,  3897,  3898,
    3904,  3905,  3908,  3909,  3910,  3934,  3935,  3938,  3944,  3948,
    3953,  3954,  3955,  3958,  3960,  3966,  3968,  3969,  3971,  3972,
    3974,  3975,  3977,  3978,  3980,  3981,  3984,  3985,  3988,  3990,
    3991,  3994,  3994,  4001,  4003,  4004,  4005,  4006,  4007,  4008,
    4009,  4011,  4012,  4013,  4014,  4015,  4016,  4017,  4018,  4019,
    4020,  4021,  4022,  4023,  4024,  4025,  4026,  4027,  4028,  4029,
    4030,  4031,  4032,  4033,  4034,  4035,  4036,  4037,  4038,  4039,
    4040,  4041,  4042,  4043,  4044,  4045,  4046,  4047,  4048,  4049,
    4050,  4051,  4052,  4053,  4054,  4055,  4056,  4057,  4058,  4059,
    4060,  4061,  4062,  4063,  4064,  4065,  4066,  4067,  4068,  4069,
    4070,  4071,  4072,  4073,  4074,  4075,  4076,  4077,  4078,  4079,
    4080,  4081,  4082,  4083,  4084,  4085,  4086,  4087,  4088,  4089,
    4090,  4091,  4094,  4101
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
  "revision_arg_stmt", "revision_stmt", "revision_end",
  "revision_opt_stmt", "date_arg_str", "$@2", "body_stmts_end",
  "body_stmts", "body_stmt", "extension_arg_str", "extension_stmt",
  "extension_end", "extension_opt_stmt", "argument_str", "argument_stmt",
  "argument_end", "yin_element_arg", "yin_element_stmt",
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
  "$@39", "$@40", "input_stmt", "$@41", "input_output_opt_stmt", "$@42",
  "$@43", "$@44", "$@45", "output_stmt", "$@46", "notification_arg_str",
  "notification_stmt", "notification_end", "notification_opt_stmt", "$@47",
  "$@48", "$@49", "$@50", "deviation_arg", "deviation_stmt",
  "deviation_opt_stmt", "deviation_arg_str", "deviate_body_stmt",
  "deviate_not_supported", "deviate_not_supported_stmt", "deviate_stmts",
  "deviate_add_stmt", "$@51", "deviate_add_end", "deviate_add_opt_stmt",
  "deviate_delete_stmt", "$@52", "deviate_delete_end",
  "deviate_delete_opt_stmt", "deviate_replace_stmt", "$@53",
  "deviate_replace_end", "deviate_replace_opt_stmt", "when_arg_str",
  "when_stmt", "when_end", "when_opt_stmt", "config_arg", "config_stmt",
  "config_arg_str", "mandatory_arg", "mandatory_stmt", "mandatory_arg_str",
  "presence_arg", "presence_stmt", "min_value_arg", "min_elements_stmt",
  "min_value_arg_str", "max_value_arg", "max_elements_stmt",
  "max_value_arg_str", "ordered_by_arg", "ordered_by_stmt",
  "ordered_by_arg_str", "must_agr_str", "must_stmt", "must_end",
  "unique_arg", "unique_stmt", "unique_arg_str", "unique_arg_opt",
  "key_arg", "key_stmt", "key_arg_str", "$@54", "range_arg_str",
  "absolute_schema_nodeid", "absolute_schema_nodeids",
  "absolute_schema_nodeid_opt", "descendant_schema_nodeid", "$@55",
  "path_arg_str", "$@56", "$@57", "absolute_path", "absolute_paths",
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
  "sep", "whitespace_opt", "string", "$@58", "strings", "identifier",
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

#define YYPACT_NINF -1002

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1002)))

#define YYTABLE_NINF -629

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1002,    61, -1002, -1002,   316, -1002, -1002, -1002,   181,   181,
   -1002, -1002,  2905,  2905,   181, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   -31,
   -1002, -1002, -1002,   -25, -1002,   181, -1002,   181, -1002,   -13,
     249,   249, -1002, -1002, -1002,   115, -1002, -1002, -1002, -1002,
      77,   234, -1002,    49,   181,   181,   181, -1002, -1002, -1002,
     103,  2335,   181, -1002,   181, -1002, -1002, -1002,   325,   181,
     181, -1002, -1002,  2335,  2905,  2335, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
     325,  2905,   249, -1002,   181,   181,   181,   181, -1002, -1002,
   -1002, -1002, -1002,    36,  2905,  2905,   166, -1002, -1002,   166,
   -1002, -1002,   166, -1002,   181, -1002, -1002,     8, -1002,   181,
    2335,  2335,  2335,  2335,    20,  1935,   181,    42, -1002,   177,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181, -1002,  1840,
     181,    75, -1002,   166, -1002,   166, -1002,   166, -1002,   166,
   -1002, -1002,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,    64, -1002,
   -1002, -1002, -1002,   249,    63, -1002, -1002, -1002, -1002, -1002,
     184, -1002, -1002, -1002, -1002,   181,  2905,    29,  2905,  2905,
    2905,    29,  2905,  2905,  2905,  2905,  2905,  2905,  2905,  2905,
    2905,  2430,  2905,   249, -1002, -1002,   244, -1002,   249,   249,
     249, -1002, -1002,   250,   181,    80,   263, -1002,  3000, -1002,
   -1002,    92, -1002, -1002,   270, -1002,   285, -1002,   296, -1002,
   -1002,    97, -1002, -1002,   334, -1002,   352, -1002,   367, -1002,
     100, -1002,   156, -1002,   163, -1002,   377, -1002,   381, -1002,
     167, -1002, -1002,   391, -1002, -1002, -1002,   263, -1002, -1002,
   -1002, -1002, -1002,   146,    66,   249, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002,    44,   181, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
     181,   181, -1002,   181,   249,   249,   181, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002,   181,   181,   249,   249,
   -1002,   249,   249,   249,   249,   249,   249,   249,   249,   249,
     249,   249,   249,   249,   249,    12,    64,   249,  2239,   236,
    1533,  1416,  1358,   109,   471,   231,   754,   213,   364,  1249,
    1134,   963,   652,   421,   215, -1002, -1002, -1002,   166, -1002,
   -1002,   181,   181,   181,   181,   181,   181, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002,   181,   181, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002,   181, -1002, -1002, -1002, -1002, -1002,   181,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181, -1002, -1002,
   -1002, -1002, -1002, -1002,   181,   181, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181,   181,
     181, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002,   181,   181, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002,   181,   181, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002,   181,    54,  2335,    55,  2335,    83,  2335,
   -1002, -1002,  2905,  2905, -1002, -1002, -1002, -1002, -1002,  2335,
   -1002, -1002,  2335, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
    2905,   277,   249,   249,   249,   249,   249,  2430,  2430,  2335,
   -1002, -1002, -1002,    52,    87,    46, -1002, -1002, -1002,  2525,
    2525, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002,   181,   181, -1002, -1002, -1002, -1002,   249,  2525,
    2525, -1002, -1002, -1002, -1002, -1002, -1002,   166, -1002,   166,
   -1002, -1002, -1002, -1002,   166, -1002,   403, -1002, -1002, -1002,
   -1002, -1002,   166, -1002,   443, -1002,   249,   249,   451, -1002,
     381, -1002,   249,   249,   249,   249,   249,   166, -1002,   249,
     249,   166, -1002,   249,   249,   249,   249,   249,   249,   249,
     474, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002,   166, -1002,   482, -1002,   166, -1002,   249,
     249,   249, -1002, -1002, -1002,   166, -1002, -1002, -1002, -1002,
     166, -1002, -1002, -1002, -1002, -1002, -1002,   166, -1002,   249,
     249,   249, -1002,   166, -1002, -1002, -1002,   166, -1002, -1002,
   -1002,   249,   249,   249,   249,   249,   249,   249,   249,   249,
     249,   249,   204,   212,   249,   249,   249,   249, -1002, -1002,
     238, -1002, -1002, -1002,   483, -1002,   249,   249,   249,   181,
     181, -1002, -1002,   181,   181, -1002, -1002, -1002, -1002,   181,
     181,   181, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002,   181,   181,   181,    50,   181,
   -1002, -1002, -1002, -1002, -1002,   181, -1002,   181, -1002,   181,
     181,   181, -1002, -1002, -1002, -1002,   181, -1002, -1002, -1002,
   -1002,   181, -1002, -1002, -1002,   181,   249,   249,   249,    91,
     249,   486,   487,   491, -1002,   522,   181,  3000,    44,   249,
     249,   249,   249,    78,    47,  1630,   181,   233, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181,   181,   248,
     879, -1002, -1002, -1002, -1002, -1002,   161,   331,  1626,   413,
     181,   181, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002,    65, -1002,   249,
     249,   249,  2905,  2335, -1002,   181,   181,   181,   181,   181,
     181, -1002,   272, -1002, -1002, -1002, -1002, -1002, -1002,   249,
     335, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,  2335,
    2335, -1002, -1002, -1002, -1002, -1002,   166, -1002,   765,   247,
     423,   492, -1002,   503, -1002,   176,  2335,    99,  2335,  2335,
      79, -1002,   249,   326, -1002, -1002, -1002, -1002, -1002,   249,
     166, -1002,   166, -1002,   249,   249,   181,   181, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   166, -1002,
   -1002,   533, -1002, -1002,   166, -1002,   271,   361,   534, -1002,
     539, -1002, -1002, -1002, -1002,   166, -1002,   249, -1002,   249,
     249,   249,   249, -1002, -1002,   249,   249,   249,   249,   249,
     249, -1002,   181, -1002, -1002, -1002, -1002,  3000, -1002, -1002,
     278, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   181,
     181, -1002,   249,   338,   746,   249,   249, -1002,   271, -1002,
    2620,   249,   249,   249,   181, -1002, -1002, -1002, -1002, -1002,
   -1002,   181, -1002, -1002, -1002, -1002, -1002, -1002,   415,   276,
   -1002, -1002, -1002,    51,   241,   662,    87,   235, -1002,   383,
   -1002,   179,   181, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002,   166, -1002, -1002, -1002, -1002, -1002, -1002,   166, -1002,
   -1002, -1002, -1002,  3000, -1002,  2335, -1002,   181, -1002,   181,
     383,   383,   166, -1002,   286,   300, -1002, -1002,   383,   324,
     383, -1002,   383,   323,   330,   383,   383,   337,   449, -1002,
     383, -1002, -1002,   365,  2715,   383, -1002, -1002, -1002,  2810,
   -1002,   392,   383,  3000, -1002
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     676,     0,     2,     3,     0,     1,   674,   675,     0,     0,
     677,   676,     0,     0,   678,   692,     4,   691,   693,   694,
     695,   696,   697,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   713,   714,
     715,   716,   717,   718,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   729,   730,   731,   732,   733,   734,
     735,   736,   737,   738,   739,   740,   741,   742,   743,   744,
     745,   746,   747,   748,   749,   750,   751,   752,   753,   754,
     755,   756,   757,   758,   759,   760,   761,   762,   763,   764,
     765,   766,   767,   768,   769,   770,   771,   676,   649,     0,
       9,   772,   676,     0,    16,     6,   660,   648,   660,     5,
      12,    19,   676,   773,    27,    11,   662,   661,   676,    27,
      18,     0,    51,    26,     0,     0,     0,    13,    14,    15,
       0,   666,   665,    51,     0,    20,   660,     7,    66,     0,
       0,    28,    29,     0,     0,     0,   676,   668,   663,   684,
     687,   685,   689,   690,   688,   683,   664,   667,   681,   686,
      66,     0,    21,   676,     0,     0,     0,     0,    52,    53,
      54,    55,    77,    64,     0,     0,     0,    24,   646,     0,
      49,   676,     0,    22,   670,   676,    77,     0,    47,     8,
       0,     0,     0,     0,     0,    76,     0,     0,    31,     0,
      37,   659,   657,    25,   660,   660,    50,   647,    23,     0,
     682,     0,   660,     0,    58,     0,    60,     0,    56,     0,
      62,   676,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   660,
      79,    80,    81,    82,    83,    84,   253,   254,   255,   256,
     257,   258,   259,   260,    85,    86,    87,    88,     0,   660,
     660,    39,   660,   655,     0,   651,   669,   676,   650,   676,
       0,    59,    61,    57,    63,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    78,    73,    75,     0,    65,    32,    41,
      38,   658,   660,     0,    17,     0,     0,   387,     0,   441,
     442,     0,   608,   676,     0,   349,     0,   261,     0,    89,
     515,     0,   508,   676,     0,   114,     0,   240,     0,   125,
       0,   287,     0,   303,     0,   322,     0,   491,     0,   461,
       0,   137,   654,     0,   404,   676,   676,     0,   389,   676,
     660,    68,   660,     0,     0,   656,   676,   676,   671,    48,
     660,   391,   388,   606,   660,   607,   440,   660,   351,   350,
     660,   263,   262,   660,    91,    90,   660,   514,   660,   116,
     115,   660,   242,   241,   660,   127,   126,   660,   660,   660,
     660,   493,   492,   660,   463,   462,   660,   660,   406,   405,
     652,   653,   390,    74,    70,    67,     0,   660,    36,    33,
      34,    35,    40,    44,    42,    43,   670,   672,   393,   444,
     609,   353,   265,    93,   510,   118,   244,   129,   288,   305,
     324,   495,   465,   139,   408,     0,     0,    30,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,    71,    72,     0,    45,
     676,     0,     0,     0,     0,     0,     0,   392,   402,   403,
     401,   396,   394,   399,   400,   397,     0,     0,   443,   449,
     450,   448,   447,   451,   457,   453,   455,   445,     0,   352,
     361,   362,   360,   356,   357,   367,   368,   369,   370,   373,
     363,   365,   366,   371,   372,   354,   358,   359,     0,   264,
     274,   275,   273,   268,   282,   276,   284,   278,   280,   266,
     272,   271,   269,     0,    92,    96,    97,    94,    95,     0,
     509,   511,   512,   117,   121,   122,   120,   119,   243,   246,
     247,   245,   660,   660,   660,   660,   660,     0,   128,   133,
     134,   132,   131,   130,     0,     0,   286,   301,   302,   300,
     291,   292,   294,   297,   289,   298,   299,   295,     0,     0,
       0,   304,   320,   321,   319,   308,   309,   312,   311,   306,
     315,   316,   317,   318,   313,     0,     0,   323,   337,   338,
     336,   327,   339,   341,   347,   343,   345,   325,   332,   333,
     334,   335,   328,   331,   330,   494,   500,   501,   499,   498,
     502,   504,   506,   496,   676,   676,   464,   468,   469,   467,
     466,   470,   472,   474,   476,   138,   144,   145,   143,   660,
     141,   142,     0,     0,   407,   413,   414,   412,   411,   415,
     417,   409,    46,   673,     0,     0,     0,     0,     0,     0,
     660,   660,     0,     0,   660,   660,   660,   660,   660,     0,
     660,   660,     0,   660,   660,   660,   660,   660,   660,   660,
       0,     0,   249,   248,   250,   251,   252,     0,     0,     0,
     660,   660,   660,     0,     0,     0,   660,   660,   660,     0,
       0,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   478,   489,   660,   660,   660,   660,   140,     0,
       0,   660,   660,   660,   676,   676,   568,     0,   564,     0,
     123,   676,   676,   573,     0,   569,     0,   590,   676,   676,
     676,   113,     0,   108,     0,   557,   395,   398,     0,   374,
       0,   459,   452,   458,   454,   456,   446,     0,   238,   364,
     355,     0,   574,   283,   277,   285,   279,   281,   267,   270,
       0,    98,   676,   676,   518,   676,   513,   676,   516,   676,
     520,   522,   521,     0,   135,     0,   147,     0,   236,   293,
     290,   296,   639,   676,   584,     0,   580,   676,   640,   579,
       0,   576,   641,   676,   676,   676,   589,     0,   585,   310,
     307,   314,   604,     0,   600,   602,   597,     0,   594,   598,
     610,   340,   342,   348,   344,   346,   326,   329,   503,   505,
     507,   497,     0,     0,   471,   473,   475,   477,   437,   438,
       0,   676,   420,   421,     0,   676,   416,   418,   410,   567,
     566,   565,   124,   572,   571,   570,   660,   592,   591,   110,
     112,   111,   109,   660,   559,   558,   660,   376,   375,   460,
     239,   575,   660,   100,   660,   523,   536,   545,     0,   517,
     136,   660,   148,   146,   237,   582,   581,   583,   577,   578,
     588,   587,   586,   601,   676,   595,   596,   608,   660,   660,
     660,   436,   660,   423,   422,   419,   171,   561,   378,   103,
      99,     0,     0,     0,   519,   153,   603,     0,   611,   480,
     480,   444,   425,     0,     0,     0,     0,     0,   660,   525,
     524,   660,   538,   537,   660,   547,   546,     0,     0,     0,
     150,   151,   660,   152,   216,   599,     0,     0,     0,     0,
       0,     0,   593,   174,   175,   172,   173,   560,   562,   563,
     377,   383,   384,   382,   381,   385,   379,     0,   101,   527,
     540,   549,     0,     0,   149,     0,     0,     0,     0,     0,
       0,   156,     0,   159,   157,   158,   660,   155,   154,   189,
     215,   479,   483,   485,   487,   481,   490,   439,   424,   434,
     435,   427,   429,   430,   431,   428,   432,   433,   660,     0,
       0,   660,   660,   676,   676,   107,     0,   102,     0,     0,
       0,     0,   219,     0,   192,     0,     0,   612,     0,     0,
       0,   660,   160,   188,   217,   660,   660,   660,   660,   426,
       0,   234,     0,   232,   386,   380,   106,   105,   104,   526,
     528,   531,   532,   533,   534,   535,   660,   530,   539,   541,
     544,   660,   543,   548,   660,   551,   552,   553,   554,   555,
     556,   660,   220,   660,   660,   193,   191,   166,     0,   163,
     676,     0,   168,   616,     0,   208,     0,     0,     0,   177,
       0,   605,   676,   676,   214,     0,   210,   161,   660,   484,
     486,   488,   482,   235,   233,   529,   542,   550,   222,   218,
     195,   164,   165,   660,   169,   660,   209,     0,   619,   613,
       0,   615,   623,   660,   178,   660,   660,   206,   205,   213,
     212,   211,   190,     0,     0,   171,   167,   628,   618,   622,
       0,   180,   176,   171,     0,   221,   226,   227,   225,   223,
     224,     0,   194,   199,   200,   198,   196,   197,     0,   617,
     620,   624,   621,   626,     0,     0,     0,     0,   170,   679,
     625,     0,     0,   179,   184,   185,   181,   182,   183,   207,
     231,     0,   228,   676,   644,   645,   642,   204,     0,   201,
     676,   643,   680,     0,   627,     0,   229,   230,   202,   203,
     679,   679,     0,   186,     0,     0,   187,   629,   679,     0,
     679,   630,   679,     0,     0,   679,   679,     0,     0,   638,
     679,   631,   634,     0,     0,   679,   635,   636,   633,   679,
     632,     0,   679,     0,   637
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1002, -1002, -1002,   579, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002,   366, -1002, -1002,   387, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   157, -1002,
   -1002, -1002,  -214,   382, -1002, -1002, -1002, -1002, -1002,   169,
   -1002,   217,   340, -1002, -1002, -1002, -1002,    95, -1002,   328,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002,  -423, -1002, -1002, -1002, -1002, -1002, -1002,
    -403, -1002, -1002, -1002, -1002, -1002,  -402, -1002,  -120, -1002,
    -442, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1001, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002,  -489, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002,  -440, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002,  -610, -1002,  -606, -1002,  -359,
   -1002,  -364, -1002,  -109, -1002, -1002,  -131, -1002,   116, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   117, -1002,
   -1002, -1002, -1002, -1002, -1002,   122, -1002, -1002, -1002, -1002,
   -1002,   126, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002,   136, -1002, -1002, -1002, -1002, -1002, -1002, -1002,   165,
   -1002, -1002, -1002, -1002, -1002,   185, -1002,   189,   251, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
    -314, -1002, -1002, -1002, -1002, -1002, -1002,  -246, -1002, -1002,
    -138, -1002, -1002, -1002, -1002, -1002, -1002, -1002,  -279, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002,  -145, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002, -1002,
   -1002, -1002, -1002, -1002,  -323, -1002, -1002, -1002,  -383, -1002,
   -1002,  -407, -1002, -1002,  -299, -1002,  -449, -1002, -1002,  -448,
   -1002, -1002,   203, -1002, -1002,  -436, -1002, -1002,  -794, -1002,
   -1002, -1002, -1002, -1002, -1002, -1002,  -325,   373,  -232,  -657,
   -1002, -1002, -1002, -1002,  -452,  -484, -1002, -1002,  -451, -1002,
   -1002, -1002,  -475, -1002, -1002, -1002,  -534, -1002, -1002, -1002,
    -651,  -474, -1002, -1002, -1002,   138,  -200,  -388,   535, -1002,
   -1002, -1002,  1142, -1002,   414, -1002, -1002, -1002,   268, -1002,
      -4,   164,   508,   158,   102, -1002, -1002,  -110,  -136,  -274
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    97,    98,   109,   163,    99,     2,   114,   115,
     103,     3,   119,   120,   182,   127,   176,   128,   122,   123,
     141,   197,   353,   199,   142,   262,   354,   458,   408,   187,
     136,   179,   129,   138,   217,   168,   213,   169,   215,   943,
     219,   944,   172,   296,   173,   352,   435,   297,   349,   194,
     195,   239,   318,   240,   375,   443,   760,   527,   864,  1006,
     917,  1007,   732,   481,   733,   324,   241,   380,   445,   719,
     482,   328,   242,   386,   447,   773,   553,   340,   982,   453,
     561,   775,   873,   929,   930,   972,  1068,   973,  1069,   974,
    1071,  1105,   913,   975,  1078,  1115,  1154,  1192,  1166,   931,
    1023,   932,  1013,  1066,  1124,  1178,  1147,  1179,   976,  1118,
    1074,   977,  1085,   978,  1086,   933,   980,   934,  1011,  1063,
    1123,  1171,  1140,  1172,  1032,   945,  1030,   946,   777,   562,
     747,   494,   326,   983,   383,   446,   483,   316,   246,   372,
     442,   668,   669,   664,   666,   667,   663,   665,   247,   330,
     448,   681,   680,   682,   332,   248,   449,   687,   686,   688,
     334,   249,   450,   696,   697,   691,   692,   694,   695,   693,
     314,   250,   369,   441,   661,   660,   500,   501,   738,   484,
     858,   915,  1002,  1001,   306,   251,   347,   252,   362,   439,
     650,   651,   343,   253,   399,   454,   713,   711,   712,   833,
     834,   639,   894,   939,   829,   830,   640,   310,   311,   254,
     440,   658,   654,   656,   657,   655,   740,   485,   338,   255,
     395,   452,   704,   705,   706,   707,   623,   822,   936,  1028,
    1025,  1026,  1027,   624,   823,   336,   486,   392,   451,   701,
     698,   699,   700,   321,   257,   444,   322,   766,   767,   768,
     769,   770,   901,   920,  1008,   771,   902,   923,  1009,   772,
     903,   926,  1010,   734,   487,   855,   914,   717,   473,   718,
     724,   474,   725,   751,   521,   790,   581,   791,   785,   582,
     786,   797,   583,   798,   726,   985,   848,   807,   603,   808,
     886,   803,   604,   804,   884,  1080,   312,   313,   365,   809,
     887,  1075,  1076,  1077,  1108,  1109,  1128,  1111,  1112,  1130,
    1152,  1160,  1149,  1190,  1201,  1211,  1212,  1214,  1219,  1202,
     792,   793,  1180,  1181,   180,   100,   810,   344,   203,   204,
     302,   205,   110,   116,   130,   131,   156,   148,   209,   358,
     117,   132,    12,  1183,   157,   185,   158,   101,   102,   118
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      10,   599,   600,   475,    11,    11,   522,   576,   181,   267,
      10,   629,   567,   584,   602,   613,   470,   346,   492,   512,
     528,   159,   536,   541,   551,   559,   574,   590,   608,   619,
     628,   637,   787,   159,   507,   159,   471,    16,   493,   513,
     420,   566,   537,   165,   552,   560,   575,   591,   609,   620,
     256,   638,   831,   835,    16,     6,   305,     7,   506,   520,
      16,     5,    16,    16,   245,   565,   580,   598,     6,   782,
       7,   106,    16,    16,   113,   243,   294,   108,   165,   167,
     159,   159,   159,   159,   563,   578,   244,    16,   112,   631,
     577,    16,   139,   140,   630,    16,     6,   165,     7,   268,
     134,    10,   113,    10,   782,   788,   196,    16,   363,   165,
     212,   940,   941,  -614,   167,   455,   472,    10,   505,   519,
      11,    11,    11,   221,  1148,   564,   579,   597,    10,   523,
      11,   641,  1155,   167,   308,    11,    11,   406,   794,   409,
     165,   795,   714,   721,   259,   167,   783,   715,   722,   308,
     947,   104,   201,  1003,   202,   345,  -628,  -628,  1004,   126,
      11,    11,    11,    11,     4,   222,   301,  1082,   728,   412,
     729,   124,  1083,   730,   916,    14,   167,   165,   269,   125,
      10,   942,   465,   359,    16,    10,     6,   224,     7,     6,
     225,     7,    11,   782,   364,   113,   517,   126,   268,   376,
     545,   229,   387,    10,   595,   146,    10,   147,   231,   232,
     125,   233,   524,   167,  1047,  1052,   464,   406,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,   547,   632,   236,   267,     6,
     237,     7,   137,    16,   165,   177,   165,   183,   125,   407,
     935,  1174,  1175,  1176,     6,   462,     7,   462,   388,   238,
     113,   105,   165,   461,   981,   389,   107,   165,   201,   396,
     202,    10,   165,   462,   940,   941,   121,   488,   462,   260,
     167,   261,   167,   633,  1107,  1159,   465,   463,   465,   774,
     776,   464,   214,   216,   218,   220,   466,   518,   167,   188,
      10,   546,   464,   167,   465,   596,   888,   170,   167,   465,
     184,   516,   198,   200,   889,   544,   548,   466,   634,   594,
     612,     6,   514,     7,   586,   555,   542,   189,   268,   170,
     592,   610,   621,   515,   533,   222,   958,   543,  1162,   467,
     890,   593,   611,   622,  1163,   207,   350,   554,   351,   210,
    1048,   964,   356,   164,   357,   171,   165,   224,   928,   927,
     225,   762,    10,   763,  1070,   360,   764,   361,   765,   165,
       8,   229,   367,    10,   368,  1110,  1107,   171,   231,   232,
     462,   233,  1159,  1129,   166,   275,   464,   370,  1182,   371,
       9,   461,   167,  1197,   488,   165,    10,    10,   373,    10,
     374,  1134,    11,   346,   346,   167,   462,   236,  1198,  1200,
     237,   465,    10,    10,   307,   463,   315,   317,   319,   464,
     325,   327,   329,   331,   333,   335,   337,   339,   341,   238,
     348,   167,  1205,   304,   986,  1206,   378,   465,   379,   554,
     461,  1135,   555,   488,   165,   466,   165,  1209,   940,   941,
     461,   488,   165,   488,   381,   462,   382,    11,    11,    11,
      11,    11,    11,  1210,   463,   568,   569,   556,   464,   384,
    1215,   385,    11,    11,   463,   568,   569,   366,   508,   390,
     167,   391,   167,   393,    11,   394,   135,   377,   167,   805,
     996,   997,   953,   397,   465,   398,   554,  1222,   554,   555,
     186,   555,   165,   998,    11,   846,   133,   847,   529,   400,
     401,   413,   954,   403,   211,   160,   988,    13,  1158,    11,
     416,   417,   410,   414,   625,    11,  1053,     6,   971,     7,
    1021,   459,   994,   113,  1088,   159,   991,   159,   167,   159,
    1024,   345,   345,    11,  1167,   853,   927,   854,  1168,   159,
      11,    11,   159,   856,   928,   857,   993,   495,   496,  1044,
    1045,  1059,  1060,   497,    11,    11,    11,   498,  1054,   159,
     411,   415,  1046,  1051,   530,   992,   862,   499,   863,   268,
     268,    11,    11,   420,   871,   892,   872,   893,   918,   921,
     919,   922,   956,   924,  1061,   925,  1062,   938,   402,   268,
     268,  1043,   859,  1058,   456,  1064,   502,  1065,   468,   479,
     490,   510,   525,   531,   534,   539,   549,   557,   572,   588,
     606,   617,   626,   635,   643,  1042,   503,  1057,    11,    11,
     504,   937,   143,   144,   145,  1103,  1113,  1104,  1114,    10,
     995,  1116,   161,  1117,  1041,  1050,  1056,   174,   175,  1040,
    1049,  1055,   457,   601,   323,   908,   469,   480,   491,   511,
     526,   532,   535,   540,   550,   558,   573,   589,   607,   618,
     627,   636,   190,   191,   192,   193,  1150,  1184,  1161,  1151,
    1216,   303,  1173,   165,   438,     0,     0,     0,     0,     0,
       0,     0,   229,   165,   462,   940,   941,   614,    10,    10,
    1138,  1145,     0,     0,   258,     0,     0,     0,     0,     0,
     155,     0,   615,     0,   206,     0,     0,   208,     0,   167,
    1139,  1146,   155,   178,   155,   465,     0,     0,   236,   167,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   720,   271,   727,
     272,   735,   273,     0,   274,   616,     0,     0,   222,     0,
       0,   748,     0,     0,   752,  1169,     0,     0,     0,   155,
     155,   155,   155,     0,     0,     0,     0,   165,   702,   703,
     224,   778,     0,   225,   955,   165,     0,     0,   462,     0,
     739,   741,   461,     0,   229,   488,     0,   268,     0,     0,
       0,   231,   232,     0,   233,   984,   984,     0,   761,     0,
       0,   234,     0,   167,     0,     0,   463,   568,   569,   465,
     464,   167,     0,     0,     0,     0,  1141,   465,     0,     0,
     236,     0,     0,   237,     0,    10,    10,   295,     0,    10,
      10,     0,   586,   555,     0,    10,    10,    10,     0,  1142,
     477,     0,   238,   159,     0,     0,   309,   538,     0,     0,
     320,    10,    10,    10,    10,    10,     0,     0,  1039,     0,
     342,    10,     0,    10,     0,    10,    10,    10,   839,   840,
       0,     0,    11,     0,     0,   843,   844,    10,     0,   159,
     159,    10,   849,   850,   851,     0,     0,     0,     0,     0,
       0,   547,    10,     0,     0,     0,   159,  1127,   159,   159,
       0,     0,    11,     0,   436,     0,     0,     0,   965,     0,
       0,     0,     0,    11,    11,     0,   865,   866,   966,   867,
    1153,   868,     0,   869,     0,     0,    11,    11,     0,     0,
     967,   968,     0,     0,     0,   969,     0,   875,   970,     0,
       0,   877,     0,     0,  -162,     0,     0,   879,   880,   881,
       0,    11,    11,    11,    11,    11,    11,   222,     0,   644,
     645,   646,   647,   648,   649,     0,     0,     0,     0,     0,
       0,     0,     0,  1191,   652,   653,     0,     0,     0,   224,
       0,     0,   225,   642,   165,   891,   659,   268,     0,   895,
       0,     0,     0,   229,     0,   462,     0,     0,     0,     0,
     231,   232,     0,   233,  1217,   295,   662,     0,   464,  1220,
     268,     0,     0,  1224,     0,     0,     0,     0,     0,     0,
     167,   670,    10,    10,     0,     0,   465,   671,     0,   236,
       0,     0,   237,     0,     0,     0,     0,     0,   906,     0,
       0,     0,     0,     0,     0,   677,     0,     0,     0,     0,
       0,   238,   678,   679,     0,  1014,   605,     0,     0,     0,
       0,     0,     0,   268,     0,   159,   683,   684,   685,     0,
       0,     0,     0,   948,   951,     0,     0,     0,     0,     0,
       0,     0,     0,   689,   690,     0,     0,     0,    10,     0,
    1012,  1031,  1033,     0,   268,     0,     0,   479,   989,   268,
       0,     0,     0,   268,     0,    10,    10,     0,  1072,     0,
    1079,  1081,     0,     0,     0,     0,     0,     0,     0,     0,
      11,   949,   952,     0,     0,     0,     0,    11,   222,     0,
     709,   710,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   480,   990,     0,    11,     0,
     224,   461,     0,   225,     0,   165,     0,  1036,  1037,     0,
       0,     0,     0,     0,   229,     0,   462,     0,     0,     0,
     585,   231,   232,    10,   233,    10,   568,   569,     0,   464,
       0,   234,   570,     0,     0,     0,     0,     0,     0,     0,
       0,   167,     0,     0,     0,     0,     0,   465,     0,     0,
     236,   586,     0,   237,     0,   466,     0,     0,     0,     0,
       0,     0,     0,   716,   155,   723,   155,   731,   155,     0,
     477,     0,   238,     0,  1102,     0,     0,   587,   155,     0,
       0,   155,     0,     0,     0,     0,  1119,  1120,     0,     0,
     111,     0,   841,     0,   842,     0,   342,   342,   155,   845,
       0,     0,   784,   789,   796,     0,     0,   852,   802,   806,
       0,     0,     0,     0,     0,     0,   461,     0,   162,   488,
     165,     0,   860,     0,     0,     0,   861,  1193,   828,   832,
       0,   462,  1136,  1143,     0,     0,     0,     0,     0,     0,
       0,   568,   569,     0,   464,     0,     0,   570,   870,     0,
       0,     0,   874,     0,     0,     0,   167,     0,     0,     0,
     876,     0,   465,  1164,   554,   878,     0,   555,     0,     0,
     466,     0,   882,     0,     0,     0,     0,  1187,   883,     0,
    1137,  1144,   885,     0,  1189,     0,   263,   264,  1194,  1195,
       0,     0,   571,     0,   270,     0,  1199,     0,  1203,     0,
    1204,     0,   222,  1207,  1208,     0,     0,     0,  1213,     0,
       0,  1165,     0,  1218,     0,     0,     0,  1221,     0,     0,
    1223,   293,     0,     0,   224,   461,     0,   225,     0,   165,
       0,     0,     0,     0,   907,     0,     0,     0,   229,     0,
     462,   298,   299,   904,   300,   231,   232,     0,   233,     0,
       0,     0,     0,   464,     0,   234,     0,     0,     0,     0,
     222,     0,     0,   508,   957,   167,     0,     0,     0,     0,
       0,   465,     0,     0,   236,   962,   963,   237,     0,   466,
       0,   476,   224,   461,   355,   225,   488,   165,   999,  1000,
       0,     0,     0,     0,   477,     0,   238,     0,   462,     0,
       0,   509,     0,   231,   232,     0,   233,   463,     0,     0,
       0,     0,     0,  1015,  1016,  1017,  1018,  1019,  1020,     0,
       0,     0,     0,   167,     0,     0,     0,     0,     0,   465,
       0,     0,   404,     0,   405,     0,     0,   466,     0,     0,
       0,     0,   418,     0,     0,     0,   419,     0,     0,   421,
       0,     0,   422,     0,   238,   423,     0,     0,   424,   489,
     425,     0,     0,   426,     0,     0,   427,     0,     0,   428,
     429,   430,   431,     0,     0,   432,  1005,   222,   433,   434,
       0,  1038,   155,     0,     0,     0,     0,     0,     0,   437,
       0,     0,     0,     0,     0,     0,     0,     0,   476,   224,
       0,     0,   225,     0,   165,  1093,     0,  1094,     0,     0,
       0,     0,     0,     0,     0,   462,     0,     0,   155,   155,
     231,   232,     0,   233,     0,     0,     0,     0,     0,     0,
     234,     0,     0,     0,  1067,   155,  1073,   155,   155,  1084,
     167,     0,     0,  1101,     0,     0,   465,     0,     0,  1106,
       0,     0,   237,     0,   466,     0,     0,     0,     0,     0,
    1121,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     222,   238,     0,     0,   222,     0,   478,     0,     0,     0,
       0,     0,  1156,     0,     0,     0,     0,     0,     0,  1157,
       0,   476,   224,     0,     0,   225,   224,   165,     0,   225,
       0,   165,     0,     0,     0,     0,     0,     0,   462,     0,
    1185,     0,   462,   231,   232,     0,   233,   231,   232,     0,
     233,     0,     0,   234,   672,   673,   674,   675,   676,     0,
       0,     0,     0,   167,     0,     0,     0,   167,     0,   465,
       0,     0,     0,   465,     0,   237,  1186,   466,     0,   237,
       0,   466,     0,  1188,     0,     0,     0,     0,     0,     0,
       0,     0,   477,     0,   238,     0,     0,  1196,   238,   987,
       0,     0,     0,   950,     0,  1170,  1177,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   155,     0,     0,     0,     0,     0,
       0,   708,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   736,   737,     0,     0,   742,   743,   744,   745,
     746,     0,   749,   750,     0,   753,   754,   755,   756,   757,
     758,   759,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   779,   780,   781,     0,     0,     0,   799,   800,
     801,     0,     0,   811,   812,   813,   814,   815,   816,   817,
     818,   819,   820,   821,    15,     0,   824,   825,   826,   827,
      17,   265,     0,   836,   837,   838,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,   222,
       0,     0,     0,   266,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,   224,     0,     0,   225,     0,     0,     0,     0,     0,
     226,   227,     0,   228,     0,   229,   230,     0,     0,     0,
       0,     0,   231,   232,     0,   233,     0,     0,   896,     0,
       0,     0,   234,     0,     0,   897,     0,     0,   898,     0,
       0,     0,     0,     0,   899,     0,   900,   235,     0,     0,
       0,   236,     0,   905,   237,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     909,   910,   911,   238,   912,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     959,     0,     0,   960,     0,     0,   961,     0,     0,     0,
       0,     0,     0,     0,   979,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1022,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1029,     0,     0,  1034,  1035,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1087,     0,     0,     0,  1089,  1090,  1091,
    1092,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1095,     0,
       0,     0,     0,  1096,     0,     0,  1097,     0,     0,     0,
       0,     0,     0,  1098,     0,  1099,  1100,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1122,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,     0,  1125,     0,  1126,     0,    17,
     265,     0,     0,     0,     0,  1131,     0,  1132,  1133,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,     0,    15,
       0,     0,   460,    16,   149,    17,   150,   151,     0,     0,
       0,   152,   153,   154,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,    16,     0,
      17,   113,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,    16,     0,    17,   265,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,     0,     0,     0,     0,     0,
      17,   265,     0,     0,  1110,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    15,
       0,     0,     0,     0,     0,    17,   265,     0,     0,  1210,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    15,  1182,     0,     0,     0,     0,
      17,   265,     0,     0,     0,     0,     0,     0,     0,     0,
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
      17,   265,     0,     0,     0,     0,     0,     0,     0,     0,
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
       4,   450,   450,   439,     8,     9,   442,   449,   144,   209,
      14,   453,   448,   449,   450,   451,   439,   291,   441,   442,
     443,   131,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   683,   143,   441,   145,   439,     8,   441,   442,
     365,   448,   445,    31,   447,   448,   449,   450,   451,   452,
     195,   454,   709,   710,     8,     5,   270,     7,   441,   442,
       8,     0,     8,     8,   195,   448,   449,   450,     5,    17,
       7,   102,     8,     8,    11,   195,    12,   102,    31,    67,
     190,   191,   192,   193,   448,   449,   195,     8,   101,   453,
     449,     8,    43,    44,   453,     8,     5,    31,     7,   209,
      23,   105,    11,   107,    17,    18,    70,     8,   308,    31,
     102,    33,    34,    14,    67,   103,   439,   121,   441,   442,
     124,   125,   126,   103,  1125,   448,   449,   450,   132,    20,
     134,   454,  1133,    67,   105,   139,   140,    71,    92,   353,
      31,    95,    88,    88,   102,    67,    94,    93,    93,   105,
     103,    13,   102,    88,   104,   291,   105,   106,    93,    82,
     164,   165,   166,   167,     0,     4,   103,    88,    85,   103,
      87,    56,    93,    90,    83,    11,    67,    31,   103,    64,
     184,   103,    73,   103,     8,   189,     5,    26,     7,     5,
      29,     7,   196,    17,   102,    11,   442,    82,   308,   102,
     446,    40,   102,   207,   450,   102,   210,   104,    47,    48,
      64,    50,   103,    67,  1008,  1009,    55,    71,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,    22,    21,    76,   438,     5,
      79,     7,     8,     8,    31,   143,    31,   145,    64,   103,
     907,    16,    17,    18,     5,    42,     7,    42,   102,    98,
      11,    97,    31,    27,   103,   102,   102,    31,   102,   102,
     104,   275,    31,    42,    33,    34,   112,    30,    42,   102,
      67,   104,    67,    68,   105,   106,    73,    51,    73,   677,
     678,    55,   190,   191,   192,   193,    81,   442,    67,   161,
     304,   446,    55,    67,    73,   450,   102,   138,    67,    73,
     146,   442,   174,   175,   102,   446,   103,    81,   103,   450,
     451,     5,   442,     7,    77,    78,   446,   163,   438,   160,
     450,   451,   452,   442,   103,     4,   103,   446,    97,   103,
     102,   450,   451,   452,   103,   181,   102,    75,   104,   185,
     103,   103,   102,    28,   104,   138,    31,    26,    32,    24,
      29,    84,   366,    86,  1015,   102,    89,   104,    91,    31,
      54,    40,   102,   377,   104,    14,   105,   160,    47,    48,
      42,    50,   106,   105,    59,   221,    55,   102,     5,   104,
      74,    27,    67,   107,    30,    31,   400,   401,   102,   403,
     104,    63,   406,   677,   678,    67,    42,    76,   108,    85,
      79,    73,   416,   417,   276,    51,   278,   279,   280,    55,
     282,   283,   284,   285,   286,   287,   288,   289,   290,    98,
     292,    67,   109,   269,   103,   105,   102,    73,   104,    75,
      27,   103,    78,    30,    31,    81,    31,   110,    33,    34,
      27,    30,    31,    30,   102,    42,   104,   461,   462,   463,
     464,   465,   466,    14,    51,    52,    53,   103,    55,   102,
     105,   104,   476,   477,    51,    52,    53,   313,    65,   102,
      67,   104,    67,   102,   488,   104,   120,   323,    67,   689,
     939,   939,   915,   102,    73,   104,    75,   105,    75,    78,
     160,    78,    31,   939,   508,   102,   119,   104,    37,   345,
     346,   354,   915,   349,   186,   133,   103,     9,   103,   523,
     356,   357,   353,   354,   103,   529,   103,     5,   930,     7,
     972,   436,   939,    11,  1023,   645,   939,   647,    67,   649,
     980,   677,   678,   547,  1154,   102,    24,   104,  1154,   659,
     554,   555,   662,   102,    32,   104,   939,   441,   441,  1008,
    1008,  1010,  1010,   441,   568,   569,   570,   441,  1010,   679,
     353,   354,  1008,  1009,   103,   939,   102,   441,   104,   689,
     690,   585,   586,   908,   102,   102,   104,   104,   102,   102,
     104,   104,   915,   102,   102,   104,   104,   911,   347,   709,
     710,  1008,   740,  1010,   435,   102,   441,   104,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   460,  1008,   441,  1010,   632,   633,
     441,   910,   124,   125,   126,   102,   102,   104,   104,   643,
     939,   102,   134,   104,  1008,  1009,  1010,   139,   140,  1008,
    1009,  1010,   435,   450,   281,   887,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   164,   165,   166,   167,  1128,  1161,  1153,  1130,
    1214,   267,  1156,    31,   416,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    31,    42,    33,    34,    45,   702,   703,
    1123,  1124,    -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,
     131,    -1,    60,    -1,   179,    -1,    -1,   182,    -1,    67,
    1123,  1124,   143,   144,   145,    73,    -1,    -1,    76,    67,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   645,   213,   647,
     215,   649,   217,    -1,   219,   103,    -1,    -1,     4,    -1,
      -1,   659,    -1,    -1,   662,   103,    -1,    -1,    -1,   190,
     191,   192,   193,    -1,    -1,    -1,    -1,    31,   614,   615,
      26,   679,    -1,    29,   915,    31,    -1,    -1,    42,    -1,
     652,   653,    27,    -1,    40,    30,    -1,   907,    -1,    -1,
      -1,    47,    48,    -1,    50,   936,   937,    -1,   670,    -1,
      -1,    57,    -1,    67,    -1,    -1,    51,    52,    53,    73,
      55,    67,    -1,    -1,    -1,    -1,    80,    73,    -1,    -1,
      76,    -1,    -1,    79,    -1,   839,   840,   258,    -1,   843,
     844,    -1,    77,    78,    -1,   849,   850,   851,    -1,   103,
      96,    -1,    98,   963,    -1,    -1,   277,   103,    -1,    -1,
     281,   865,   866,   867,   868,   869,    -1,    -1,   103,    -1,
     291,   875,    -1,   877,    -1,   879,   880,   881,   714,   715,
      -1,    -1,   886,    -1,    -1,   721,   722,   891,    -1,   999,
    1000,   895,   728,   729,   730,    -1,    -1,    -1,    -1,    -1,
      -1,    22,   906,    -1,    -1,    -1,  1016,  1107,  1018,  1019,
      -1,    -1,   916,    -1,   406,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,   927,   928,    -1,   762,   763,    49,   765,
    1130,   767,    -1,   769,    -1,    -1,   940,   941,    -1,    -1,
      61,    62,    -1,    -1,    -1,    66,    -1,   783,    69,    -1,
      -1,   787,    -1,    -1,    75,    -1,    -1,   793,   794,   795,
      -1,   965,   966,   967,   968,   969,   970,     4,    -1,   461,
     462,   463,   464,   465,   466,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1183,   476,   477,    -1,    -1,    -1,    26,
      -1,    -1,    29,   458,    31,   831,   488,  1107,    -1,   835,
      -1,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,    -1,
      47,    48,    -1,    50,  1214,   436,   508,    -1,    55,  1219,
    1130,    -1,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,
      67,   523,  1036,  1037,    -1,    -1,    73,   529,    -1,    76,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,   884,    -1,
      -1,    -1,    -1,    -1,    -1,   547,    -1,    -1,    -1,    -1,
      -1,    98,   554,   555,    -1,   963,   103,    -1,    -1,    -1,
      -1,    -1,    -1,  1183,    -1,  1185,   568,   569,   570,    -1,
      -1,    -1,    -1,   914,   915,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   585,   586,    -1,    -1,    -1,  1102,    -1,
     962,   999,  1000,    -1,  1214,    -1,    -1,   938,   939,  1219,
      -1,    -1,    -1,  1223,    -1,  1119,  1120,    -1,  1016,    -1,
    1018,  1019,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1134,   914,   915,    -1,    -1,    -1,    -1,  1141,     4,    -1,
     632,   633,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   938,   939,    -1,  1162,    -1,
      26,    27,    -1,    29,    -1,    31,    -1,  1003,  1004,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,
      46,    47,    48,  1187,    50,  1189,    52,    53,    -1,    55,
      -1,    57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      76,    77,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   644,   645,   646,   647,   648,   649,    -1,
      96,    -1,    98,    -1,  1070,    -1,    -1,   103,   659,    -1,
      -1,   662,    -1,    -1,    -1,    -1,  1082,  1083,    -1,    -1,
     108,    -1,   717,    -1,   719,    -1,   677,   678,   679,   724,
      -1,    -1,   683,   684,   685,    -1,    -1,   732,   689,   690,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,   136,    30,
      31,    -1,   747,    -1,    -1,    -1,   751,  1185,   709,   710,
      -1,    42,  1123,  1124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    53,    -1,    55,    -1,    -1,    58,   773,    -1,
      -1,    -1,   777,    -1,    -1,    -1,    67,    -1,    -1,    -1,
     785,    -1,    73,  1154,    75,   790,    -1,    78,    -1,    -1,
      81,    -1,   797,    -1,    -1,    -1,    -1,  1173,   803,    -1,
    1123,  1124,   807,    -1,  1180,    -1,   204,   205,  1190,  1191,
      -1,    -1,   103,    -1,   212,    -1,  1198,    -1,  1200,    -1,
    1202,    -1,     4,  1205,  1206,    -1,    -1,    -1,  1210,    -1,
      -1,  1154,    -1,  1215,    -1,    -1,    -1,  1219,    -1,    -1,
    1222,   239,    -1,    -1,    26,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,   886,    -1,    -1,    -1,    40,    -1,
      42,   259,   260,   868,   262,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    65,   916,    67,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    76,   927,   928,    79,    -1,    81,
      -1,    25,    26,    27,   302,    29,    30,    31,   940,   941,
      -1,    -1,    -1,    -1,    96,    -1,    98,    -1,    42,    -1,
      -1,   103,    -1,    47,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    -1,   965,   966,   967,   968,   969,   970,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,   350,    -1,   352,    -1,    -1,    81,    -1,    -1,
      -1,    -1,   360,    -1,    -1,    -1,   364,    -1,    -1,   367,
      -1,    -1,   370,    -1,    98,   373,    -1,    -1,   376,   103,
     378,    -1,    -1,   381,    -1,    -1,   384,    -1,    -1,   387,
     388,   389,   390,    -1,    -1,   393,   957,     4,   396,   397,
      -1,  1006,   963,    -1,    -1,    -1,    -1,    -1,    -1,   407,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    -1,    31,  1030,    -1,  1032,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,   999,  1000,
      47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,  1015,  1016,  1017,  1018,  1019,  1020,
      67,    -1,    -1,  1068,    -1,    -1,    73,    -1,    -1,  1074,
      -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,
    1085,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,
       4,    98,    -1,    -1,     4,    -1,   103,    -1,    -1,    -1,
      -1,    -1,  1134,    -1,    -1,    -1,    -1,    -1,    -1,  1141,
      -1,    25,    26,    -1,    -1,    29,    26,    31,    -1,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
    1162,    -1,    42,    47,    48,    -1,    50,    47,    48,    -1,
      50,    -1,    -1,    57,   542,   543,   544,   545,   546,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    67,    -1,    73,
      -1,    -1,    -1,    73,    -1,    79,  1171,    81,    -1,    79,
      -1,    81,    -1,  1178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    -1,    98,    -1,    -1,  1192,    98,   103,
      -1,    -1,    -1,   103,    -1,  1156,  1157,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1185,    -1,    -1,    -1,    -1,    -1,
      -1,   629,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   650,   651,    -1,    -1,   654,   655,   656,   657,
     658,    -1,   660,   661,    -1,   663,   664,   665,   666,   667,
     668,   669,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   680,   681,   682,    -1,    -1,    -1,   686,   687,
     688,    -1,    -1,   691,   692,   693,   694,   695,   696,   697,
     698,   699,   700,   701,     4,    -1,   704,   705,   706,   707,
      10,    11,    -1,   711,   712,   713,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,     4,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    21,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    -1,    40,    41,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    50,    -1,    -1,   846,    -1,
      -1,    -1,    57,    -1,    -1,   853,    -1,    -1,   856,    -1,
      -1,    -1,    -1,    -1,   862,    -1,   864,    72,    -1,    -1,
      -1,    76,    -1,   871,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     888,   889,   890,    98,   892,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     918,    -1,    -1,   921,    -1,    -1,   924,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   932,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   976,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     998,    -1,    -1,  1001,  1002,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1021,    -1,    -1,    -1,  1025,  1026,  1027,
    1028,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1046,    -1,
      -1,    -1,    -1,  1051,    -1,    -1,  1054,    -1,    -1,    -1,
      -1,    -1,    -1,  1061,    -1,  1063,  1064,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1088,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,    -1,  1103,    -1,  1105,    -1,    10,
      11,    -1,    -1,    -1,    -1,  1113,    -1,  1115,  1116,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    -1,     4,
      -1,    -1,   103,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    -1,    20,    21,    22,    23,    24,
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
       0,   112,   118,   122,   452,     0,     5,     7,    54,    74,
     451,   451,   453,   453,   452,     4,     8,    10,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   113,   114,   117,
     436,   458,   459,   121,   436,   452,   102,   452,   102,   115,
     443,   443,   101,    11,   119,   120,   444,   451,   460,   123,
     124,   452,   129,   130,    56,    64,    82,   126,   128,   143,
     445,   446,   452,   129,    23,   126,   141,     8,   144,    43,
      44,   131,   135,   453,   453,   453,   102,   104,   448,     9,
      11,    12,    16,    17,    18,   114,   447,   455,   457,   458,
     144,   453,   443,   116,    28,    31,    59,    67,   146,   148,
     150,   152,   153,   155,   453,   453,   127,   455,   114,   142,
     435,   459,   125,   455,   452,   456,   153,   140,   436,   452,
     453,   453,   453,   453,   160,   161,    70,   132,   436,   134,
     436,   102,   104,   439,   440,   442,   439,   452,   439,   449,
     452,   160,   102,   147,   455,   149,   455,   145,   455,   151,
     455,   103,     4,    21,    26,    29,    35,    36,    38,    40,
      41,    47,    48,    50,    57,    72,    76,    79,    98,   162,
     164,   177,   183,   189,   244,   247,   249,   259,   266,   272,
     282,   296,   298,   304,   320,   330,   347,   355,   453,   102,
     102,   104,   136,   443,   443,    11,   103,   437,   458,   103,
     443,   439,   439,   439,   439,   452,   453,   453,   453,   453,
     453,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     453,   453,   453,   443,    12,   114,   154,   158,   443,   443,
     443,   103,   441,   445,   452,   143,   295,   436,   105,   114,
     318,   319,   407,   408,   281,   436,   248,   436,   163,   436,
     114,   354,   357,   408,   176,   436,   243,   436,   182,   436,
     260,   436,   265,   436,   271,   436,   346,   436,   329,   436,
     188,   436,   114,   303,   438,   459,   460,   297,   436,   159,
     102,   104,   156,   133,   137,   443,   102,   104,   450,   103,
     102,   104,   299,   437,   102,   409,   452,   102,   104,   283,
     102,   104,   250,   102,   104,   165,   102,   452,   102,   104,
     178,   102,   104,   245,   102,   104,   184,   102,   102,   102,
     102,   104,   348,   102,   104,   331,   102,   102,   104,   305,
     452,   452,   299,   452,   443,   443,    71,   103,   139,   143,
     150,   152,   103,   139,   150,   152,   452,   452,   443,   443,
     407,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   443,   157,   453,   443,   449,   300,
     321,   284,   251,   166,   356,   179,   246,   185,   261,   267,
     273,   349,   332,   190,   306,   103,   150,   152,   138,   158,
     103,    27,    42,    51,    55,    73,    81,   103,   150,   152,
     174,   181,   375,   379,   382,   396,    25,    96,   103,   150,
     152,   174,   181,   247,   290,   328,   347,   375,    30,   103,
     150,   152,   174,   181,   242,   249,   259,   266,   272,   282,
     287,   288,   290,   296,   298,   375,   379,   382,    65,   103,
     150,   152,   174,   181,   189,   244,   247,   328,   347,   375,
     379,   385,   396,    20,   103,   150,   152,   168,   174,    37,
     103,   150,   152,   103,   150,   152,   174,   181,   103,   150,
     152,   174,   189,   244,   247,   328,   347,    22,   103,   150,
     152,   174,   181,   187,    75,    78,   103,   150,   152,   174,
     181,   191,   240,   242,   375,   379,   382,   396,    52,    53,
      58,   103,   150,   152,   174,   181,   191,   240,   242,   375,
     379,   387,   390,   393,   396,    46,    77,   103,   150,   152,
     174,   181,   189,   244,   247,   328,   347,   375,   379,   387,
     390,   393,   396,   399,   403,   103,   150,   152,   174,   181,
     189,   244,   247,   396,    45,    60,   103,   150,   152,   174,
     181,   189,   244,   337,   344,   103,   150,   152,   174,   191,
     240,   242,    21,    68,   103,   150,   152,   174,   181,   312,
     317,   375,   439,   452,   453,   453,   453,   453,   453,   453,
     301,   302,   453,   453,   323,   326,   324,   325,   322,   453,
     286,   285,   453,   257,   254,   258,   255,   256,   252,   253,
     453,   453,   443,   443,   443,   443,   443,   453,   453,   453,
     263,   262,   264,   453,   453,   453,   269,   268,   270,   453,
     453,   276,   277,   280,   278,   279,   274,   275,   351,   352,
     353,   350,   452,   452,   333,   334,   335,   336,   443,   453,
     453,   308,   309,   307,    88,    93,   114,   378,   380,   180,
     455,    88,    93,   114,   381,   383,   395,   455,    85,    87,
      90,   114,   173,   175,   374,   455,   443,   443,   289,   436,
     327,   436,   443,   443,   443,   443,   443,   241,   455,   443,
     443,   384,   455,   443,   443,   443,   443,   443,   443,   443,
     167,   436,    84,    86,    89,    91,   358,   359,   360,   361,
     362,   366,   370,   186,   438,   192,   438,   239,   455,   443,
     443,   443,    17,    94,   114,   389,   391,   431,    18,   114,
     386,   388,   431,   432,    92,    95,   114,   392,   394,   443,
     443,   443,   114,   402,   404,   437,   114,   398,   400,   410,
     437,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   338,   345,   443,   443,   443,   443,   114,   315,
     316,   410,   114,   310,   311,   410,   443,   443,   443,   452,
     452,   439,   439,   452,   452,   439,   102,   104,   397,   452,
     452,   452,   439,   102,   104,   376,   102,   104,   291,   331,
     439,   439,   102,   104,   169,   452,   452,   452,   452,   452,
     439,   102,   104,   193,   439,   452,   439,   452,   439,   452,
     452,   452,   439,   439,   405,   439,   401,   411,   102,   102,
     102,   452,   102,   104,   313,   452,   443,   443,   443,   443,
     443,   363,   367,   371,   439,   443,   452,   453,   409,   443,
     443,   443,   443,   203,   377,   292,    83,   171,   102,   104,
     364,   102,   104,   368,   102,   104,   372,    24,    32,   194,
     195,   210,   212,   226,   228,   410,   339,   339,   321,   314,
      33,    34,   103,   150,   152,   236,   238,   103,   150,   152,
     103,   150,   152,   174,   181,   247,   375,   453,   103,   443,
     443,   443,   453,   453,   103,    39,    49,    61,    62,    66,
      69,   187,   196,   198,   200,   204,   219,   222,   224,   443,
     227,   103,   189,   244,   247,   396,   103,   103,   103,   150,
     152,   181,   242,   379,   382,   385,   387,   390,   396,   453,
     453,   294,   293,    88,    93,   114,   170,   172,   365,   369,
     373,   229,   436,   213,   455,   453,   453,   453,   453,   453,
     453,   191,   443,   211,   228,   341,   342,   343,   340,   443,
     237,   455,   235,   455,   443,   443,   452,   452,   439,   103,
     240,   242,   379,   382,   387,   390,   396,   399,   103,   240,
     242,   396,   399,   103,   191,   240,   242,   379,   382,   387,
     390,   102,   104,   230,   102,   104,   214,   114,   197,   199,
     431,   201,   455,   114,   221,   412,   413,   414,   205,   455,
     406,   455,    88,    93,   114,   223,   225,   443,   212,   443,
     443,   443,   443,   439,   439,   443,   443,   443,   443,   443,
     443,   439,   452,   102,   104,   202,   439,   105,   415,   416,
      14,   418,   419,   102,   104,   206,   102,   104,   220,   452,
     452,   439,   443,   231,   215,   443,   443,   437,   417,   105,
     420,   443,   443,   443,    63,   103,   150,   152,   174,   181,
     233,    80,   103,   150,   152,   174,   181,   217,   203,   423,
     415,   419,   421,   437,   207,   203,   453,   453,   103,   106,
     422,   423,    97,   103,   150,   152,   209,   236,   238,   103,
     114,   232,   234,   432,    16,    17,    18,   114,   216,   218,
     433,   434,     5,   454,   416,   453,   439,   452,   439,   452,
     424,   437,   208,   455,   454,   454,   439,   107,   108,   454,
      85,   425,   430,   454,   454,   109,   105,   454,   454,   110,
      14,   426,   427,   454,   428,   105,   427,   437,   454,   429,
     437,   454,   105,   454,   437
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
     149,   150,   151,   152,   153,   154,   155,   155,   156,   156,
     157,   157,   157,   159,   158,   158,   160,   161,   161,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   163,
     164,   165,   165,   166,   166,   166,   166,   166,   167,   168,
     169,   169,   170,   171,   171,   172,   172,   172,   173,   174,
     175,   175,   175,   175,   176,   177,   178,   178,   179,   179,
     179,   179,   179,   180,   181,   182,   183,   184,   184,   185,
     185,   185,   185,   185,   185,   186,   187,   188,   189,   190,
     190,   190,   190,   190,   190,   190,   191,   192,   193,   193,
     194,   194,   194,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   196,   197,   198,   199,   199,   200,   201,   202,
     202,   203,   203,   203,   203,   203,   204,   205,   206,   206,
     207,   207,   207,   207,   207,   207,   208,   209,   210,   211,
     211,   212,   213,   214,   214,   215,   215,   215,   215,   215,
     215,   216,   217,   218,   218,   219,   220,   220,   221,   222,
     223,   224,   225,   225,   225,   226,   227,   227,   228,   229,
     230,   230,   231,   231,   231,   231,   231,   231,   232,   233,
     234,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   245,   246,   246,   246,   246,   246,   246,
     246,   246,   246,   247,   247,   247,   247,   247,   247,   247,
     247,   248,   249,   250,   250,   251,   252,   251,   251,   253,
     251,   251,   251,   251,   251,   251,   254,   251,   255,   251,
     256,   251,   257,   251,   258,   251,   259,   260,   261,   262,
     261,   261,   263,   261,   261,   264,   261,   261,   261,   261,
     261,   261,   261,   265,   266,   267,   268,   267,   267,   269,
     267,   267,   267,   270,   267,   267,   267,   267,   267,   267,
     267,   267,   271,   272,   273,   274,   273,   273,   275,   273,
     273,   273,   273,   273,   273,   273,   273,   273,   273,   276,
     273,   277,   273,   278,   273,   279,   273,   280,   273,   281,
     282,   283,   283,   284,   285,   284,   284,   284,   284,   284,
     284,   284,   284,   286,   284,   287,   287,   288,   288,   288,
     288,   288,   288,   288,   289,   290,   291,   291,   292,   293,
     292,   292,   292,   292,   292,   294,   292,   295,   296,   297,
     298,   299,   299,   300,   301,   300,   300,   302,   300,   300,
     300,   300,   300,   300,   303,   304,   305,   305,   306,   307,
     306,   306,   306,   306,   306,   308,   306,   309,   306,   310,
     310,   311,   312,   313,   313,   314,   314,   314,   314,   314,
     314,   314,   314,   314,   314,   314,   315,   315,   316,   317,
     318,   318,   319,   320,   321,   322,   321,   321,   321,   321,
     321,   323,   321,   324,   321,   325,   321,   326,   321,   327,
     328,   329,   330,   331,   331,   332,   332,   332,   332,   332,
     333,   332,   334,   332,   335,   332,   336,   332,   338,   337,
     339,   340,   339,   341,   339,   342,   339,   343,   339,   345,
     344,   346,   347,   348,   348,   349,   350,   349,   349,   349,
     349,   349,   351,   349,   352,   349,   353,   349,   354,   355,
     356,   356,   356,   356,   357,   357,   358,   358,   359,   360,
     361,   361,   361,   363,   362,   364,   364,   365,   365,   365,
     365,   365,   365,   365,   365,   365,   367,   366,   368,   368,
     369,   369,   369,   369,   369,   371,   370,   372,   372,   373,
     373,   373,   373,   373,   373,   373,   373,   374,   375,   376,
     376,   377,   377,   377,   378,   379,   380,   380,   380,   381,
     382,   383,   383,   383,   384,   385,   386,   387,   388,   388,
     389,   390,   391,   391,   391,   392,   393,   394,   394,   394,
     395,   396,   397,   397,   398,   399,   400,   400,   401,   401,
     402,   403,   405,   404,   404,   406,   407,   408,   409,   409,
     411,   410,   413,   412,   414,   412,   412,   415,   416,   417,
     417,   418,   419,   420,   420,   421,   422,   422,   423,   423,
     424,   425,   426,   427,   428,   428,   429,   429,   430,   431,
     432,   432,   433,   433,   434,   434,   435,   435,   436,   436,
     437,   437,   438,   438,   438,   439,   439,   440,   441,   442,
     443,   443,   443,   444,   445,   446,   447,   447,   448,   448,
     449,   449,   450,   450,   451,   451,   452,   452,   453,   454,
     454,   456,   455,   455,   457,   457,   457,   457,   457,   457,
     457,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     458,   458,   459,   460
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     0,     0,     6,     1,
      13,     1,     0,     2,     2,     2,     1,    13,     1,     0,
       2,     3,     1,     4,     1,     4,     1,     0,     2,     2,
       8,     1,     0,     2,     2,     2,     2,     1,     5,     1,
       4,     0,     2,     2,     2,     1,     4,     1,     7,     1,
       4,     0,     2,     2,     2,     2,     1,     4,     1,     4,
       1,     4,     1,     4,     1,     1,     0,     6,     1,     4,
       0,     2,     2,     0,     3,     1,     1,     0,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     4,     0,     2,     2,     2,     2,     1,     5,
       1,     4,     1,     0,     4,     2,     2,     1,     1,     4,
       2,     2,     2,     1,     1,     4,     1,     4,     0,     2,
       2,     2,     2,     1,     4,     1,     4,     1,     4,     0,
       2,     2,     2,     2,     2,     1,     4,     1,     7,     0,
       3,     2,     2,     2,     2,     2,     4,     1,     1,     4,
       1,     1,     1,     0,     2,     2,     2,     2,     2,     2,
       3,     4,     0,     1,     4,     2,     1,     5,     1,     1,
       4,     0,     2,     2,     2,     2,     5,     1,     1,     4,
       0,     2,     2,     2,     2,     2,     1,     4,     3,     0,
       3,     4,     1,     1,     4,     0,     2,     2,     2,     2,
       2,     1,     4,     2,     1,     4,     1,     4,     1,     4,
       1,     4,     2,     2,     1,     2,     0,     2,     5,     1,
       1,     4,     0,     2,     2,     2,     2,     2,     1,     4,
       2,     1,     1,     4,     1,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     0,     2,     2,     2,     3,     3,
       3,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     4,     0,     0,     4,     2,     0,
       4,     2,     2,     2,     2,     2,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     7,     1,     0,     0,
       4,     2,     0,     4,     2,     0,     4,     2,     2,     2,
       2,     2,     2,     1,     7,     0,     0,     4,     2,     0,
       4,     2,     2,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     1,     7,     0,     0,     4,     2,     0,     4,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     1,
       4,     1,     4,     0,     0,     4,     2,     2,     2,     2,
       2,     2,     2,     0,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     4,     0,     0,
       4,     2,     2,     2,     2,     0,     4,     1,     4,     1,
       4,     1,     4,     0,     0,     4,     2,     0,     4,     2,
       2,     2,     2,     2,     1,     4,     1,     4,     0,     0,
       4,     2,     2,     2,     2,     0,     4,     0,     4,     2,
       1,     1,     4,     1,     4,     0,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     7,
       2,     1,     1,     7,     0,     0,     4,     2,     2,     2,
       2,     0,     4,     0,     4,     0,     4,     0,     4,     1,
       4,     1,     4,     1,     4,     0,     2,     2,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     7,
       0,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       7,     1,     4,     1,     4,     0,     0,     4,     2,     2,
       2,     2,     0,     4,     0,     4,     0,     4,     1,     7,
       0,     2,     2,     4,     2,     1,     1,     2,     1,     3,
       1,     1,     1,     0,     4,     1,     4,     0,     2,     3,
       2,     2,     2,     2,     2,     2,     0,     4,     1,     4,
       0,     2,     3,     2,     2,     0,     4,     1,     4,     0,
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
       0,     2,     2,     3,     2,     1,     0,     1,     1,     4,
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

    case 205: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 440: /* semicolom  */

      { free(((*yyvaluep).str)); }

        break;

    case 442: /* curly_bracket_open  */

      { free(((*yyvaluep).str)); }

        break;

    case 446: /* string_opt_part1  */

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

  case 31:

    { YANG_ADDELEM(trg->imp, trg->imp_size);
                                     /* HACK for unres */
                                     ((struct lys_import *)actual)->module = (struct lys_module *)s;
                                     s = NULL;
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
                                            YYABORT;
                                          }
                                          (yyval.i) = 1;
                                        }

    break;

  case 37:

    { YANG_ADDELEM(trg->inc, trg->inc_size);
                                     /* HACK for unres */
                                     ((struct lys_include *)actual)->submodule = (struct lys_submodule *)s;
                                     s = NULL;
                                     actual_type = INCLUDE_KEYWORD;
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
                                             YYABORT;
                                           }
                                           (yyval.i) = 1;
                                         }

    break;

  case 45:

    { backup_type = actual_type;
                                  actual_type = REVISION_DATE_KEYWORD;
                                }

    break;

  case 46:

    { if (actual_type==IMPORT_KEYWORD) {
                                memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                              } else {                              // INCLUDE KEYWORD
                                memcpy(((struct lys_include *)actual)->rev, s, LY_REV_SIZE-1);
                              }
                              free(s);
                              s = NULL;
                            }

    break;

  case 47:

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
                                         backup_type = actual_type;
                                         actual_type = BELONGS_TO_KEYWORD;
                                       }

    break;

  case 48:

    { actual_type = backup_type;
                       backup_type = NODE;
                     }

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
                                  yang_read_revision(trg, s, actual);
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

    { actual = NULL; }

    break;

  case 78:

    { actual = NULL; }

    break;

  case 89:

    { YANG_ADDELEM(trg->extensions, trg->extensions_size);
                                        trg->extensions_size--;
                                        ((struct lys_ext *)actual)->name = lydict_insert_zc(param->module->ctx, s);
                                        ((struct lys_ext *)actual)->module = trg;
                                        if (lyp_check_identifier(((struct lys_ext *)actual)->name, LY_IDENT_EXTENSION, trg, NULL)) {
                                          YYABORT;
                                        }
                                        trg->extensions_size++;
                                        s = NULL;
                                      }

    break;

  case 90:

    { struct lys_ext *ext = actual;
                  ext->plugin = ext_get_plugin(ext->name, ext->module->name, ext->module->rev ? ext->module->rev[0].date : NULL);
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

    { if (((struct lys_ext *)actual)->argument) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                      free(s);
                                      YYABORT;
                                   }
                                   ((struct lys_ext *)actual)->argument = lydict_insert_zc(param->module->ctx, s);
                                   s = NULL;
                                 }

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
                                      YANG_ADDELEM(trg->features, trg->features_size);
                                      ((struct lys_feature *)actual)->name = lydict_insert_zc(trg->ctx, s);
                                      ((struct lys_feature *)actual)->module = trg;
                                      s = NULL;
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

  case 119:

    { void *feature;

                                        feature = actual;
                                        YANG_ADDELEM(((struct lys_feature *)actual)->iffeature,
                                                     ((struct lys_feature *)actual)->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                                        s = NULL;
                                        actual = feature;
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

    { backup_type = actual_type;
                         actual_type = IF_FEATURE_KEYWORD;
                       }

    break;

  case 125:

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

  case 128:

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

  case 130:

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

  case 131:

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

  case 132:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 133:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 134:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 135:

    { backup_type = actual_type;
                                   actual_type = BASE_KEYWORD;
                                 }

    break;

  case 137:

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

  case 138:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual = tpdf_parent;
                  }

    break;

  case 139:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 140:

    { actual = (yyvsp[-2].nodes).node.ptr_tpdf;
                                       actual_type = TYPEDEF_KEYWORD;
                                       (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 141:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {YYABORT;} s = NULL; }

    break;

  case 142:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  (yyval.nodes) = (yyvsp[-1].nodes);
                                }

    break;

  case 143:

    { if (yang_check_flags((uint16_t*)&(yyvsp[-1].nodes).node.ptr_tpdf->flags, LYS_STATUS_MASK, "status", "typedef", (yyvsp[0].i), 0)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 144:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 145:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 147:

    { if (!(actual = yang_read_type(trg, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 150:

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

  case 154:

    { if (yang_read_require_instance(actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 155:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 156:

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

  case 161:

    { actual = (yyvsp[-2].v); }

    break;

  case 162:

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

  case 163:

    { (yyval.uint) = (yyvsp[0].uint);
                                               backup_type = actual_type;
                                               actual_type = FRACTION_DIGITS_KEYWORD;
                                             }

    break;

  case 164:

    { if (yang_read_fraction(actual, (yyvsp[-1].uint))) {
                                  YYABORT;
                                }
                              }

    break;

  case 165:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 166:

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

  case 167:

    { actual = (yyvsp[-2].v);
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

    break;

  case 168:

    { (yyval.v) = actual;
                         if (!(actual = yang_read_length(trg, actual, s))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 171:

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

  case 172:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 173:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 174:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 175:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 176:

    { if (yang_read_pattern(trg, actual, (yyvsp[-2].str), (yyvsp[-1].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual = yang_type;
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

    break;

  case 177:

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

  case 178:

    { (yyval.ch) = 0x06; }

    break;

  case 179:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 180:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 181:

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

  case 182:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 183:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 184:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 185:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 186:

    { backup_type = actual_type;
                       actual_type = MODIFIER_KEYWORD;
                     }

    break;

  case 187:

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

  case 188:

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

  case 191:

    { if (yang_check_enum(yang_type, actual, &cnt_val, is_value)) {
               YYABORT;
             }
             actual = yang_type;
             actual_type = TYPE_KEYWORD;
           }

    break;

  case 192:

    { yang_type = actual;
                       YANG_ADDELEM(((struct yang_type *)actual)->type->info.enums.enm, ((struct yang_type *)actual)->type->info.enums.count);
                       if (yang_read_enum(trg, yang_type, actual, s)) {
                         YYABORT;
                       }
                       s = NULL;
                       is_value = 0;
                     }

    break;

  case 194:

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

  case 196:

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

  case 197:

    { if (is_value) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                is_value = 1;
                              }

    break;

  case 198:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_enum *)actual)->flags, LYS_STATUS_MASK, "status", "enum", (yyvsp[0].i), 1)) {
                                   YYABORT;
                                 }
                               }

    break;

  case 199:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 200:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 201:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = VALUE_KEYWORD;
                                 }

    break;

  case 202:

    { ((struct lys_type_enum *)actual)->value = (yyvsp[-1].i);

                      /* keep the highest enum value for automatic increment */
                      if ((yyvsp[-1].i) >= cnt_val) {
                        cnt_val = (yyvsp[-1].i);
                        cnt_val++;
                      }
                    }

    break;

  case 203:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 204:

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

  case 205:

    { actual = (yyvsp[-1].v);
                                                        actual_type = RANGE_KEYWORD;
                                                      }

    break;

  case 208:

    { backup_type = actual_type;
                         actual_type = PATH_KEYWORD;
                       }

    break;

  case 210:

    { (yyval.i) = (yyvsp[0].i);
                                                 backup_type = actual_type;
                                                 actual_type = REQUIRE_INSTANCE_KEYWORD;
                                               }

    break;

  case 211:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 212:

    { (yyval.i) = 1; }

    break;

  case 213:

    { (yyval.i) = -1; }

    break;

  case 214:

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

  case 215:

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

  case 218:

    { if (yang_check_bit(yang_type, actual, &cnt_val, is_value)) {
                      YYABORT;
                    }
                    actual = yang_type;
                    actual_type = TYPE_KEYWORD;
                  }

    break;

  case 219:

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

  case 221:

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

  case 223:

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

  case 224:

    { if (is_value) {
                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  is_value = 1;
                                }

    break;

  case 225:

    { if (yang_check_flags((uint16_t*)&((struct lys_type_bit *)actual)->flags,
                                                     LYS_STATUS_MASK, "status", "bit", (yyvsp[0].i), 1)) {
                                  YYABORT;
                                }
                              }

    break;

  case 226:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 227:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 228:

    { (yyval.uint) = (yyvsp[0].uint);
                                             backup_type = actual_type;
                                             actual_type = POSITION_KEYWORD;
                                           }

    break;

  case 229:

    { ((struct lys_type_bit *)actual)->pos = (yyvsp[-1].uint);

                         /* keep the highest position value for automatic increment */
                         if ((yyvsp[-1].uint) >= cnt_val) {
                           cnt_val = (yyvsp[-1].uint);
                           cnt_val++;
                         }
                       }

    break;

  case 230:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 231:

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

  case 232:

    { backup_type = actual_type;
                            actual_type = ERROR_MESSAGE_KEYWORD;
                          }

    break;

  case 234:

    { backup_type = actual_type;
                            actual_type = ERROR_APP_TAG_KEYWORD;
                          }

    break;

  case 236:

    { backup_type = actual_type;
                    actual_type = UNITS_KEYWORD;
                  }

    break;

  case 238:

    { backup_type = actual_type;
                      actual_type = DEFAULT_KEYWORD;
                    }

    break;

  case 240:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_GROUPING, sizeof(struct lys_node_grp)))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       data_node = actual;
                                     }

    break;

  case 241:

    { LOGDBG("YANG: finished parsing grouping statement \"%s\"", data_node->name); }

    break;

  case 244:

    { (yyval.nodes).grouping = actual;
                                actual_type = GROUPING_KEYWORD;
                              }

    break;

  case 245:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 246:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 247:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 248:

    { actual = (yyvsp[-2].nodes).grouping;
                                               actual_type = GROUPING_KEYWORD;
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
                                                   if (trg->version < 2) {
                                                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 261:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CONTAINER, sizeof(struct lys_node_container)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                      }

    break;

  case 262:

    { LOGDBG("YANG: finished parsing container statement \"%s\"", data_node->name); }

    break;

  case 264:

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

  case 265:

    { (yyval.nodes).container = actual;
                                 actual_type = CONTAINER_KEYWORD;
                               }

    break;

  case 266:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 268:

    { YANG_ADDELEM((yyvsp[-1].nodes).container->iffeature, (yyvsp[-1].nodes).container->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).container;
                                          s = NULL;
                                        }

    break;

  case 269:

    { actual = (yyvsp[-1].nodes).container;
                                    actual_type = CONTAINER_KEYWORD;
                                  }

    break;

  case 271:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 272:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 273:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 274:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 275:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 276:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 278:

    { actual = (yyvsp[-1].nodes).container;
                                      actual_type = CONTAINER_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 280:

    { actual = (yyvsp[-1].nodes).container;
                                            actual_type = CONTAINER_KEYWORD;
                                            data_node = actual;
                                            if (trg->version < 2) {
                                              LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "notification");
                                              YYABORT;
                                            }
                                          }

    break;

  case 282:

    { actual = (yyvsp[-1].nodes).container;
                                       actual_type = CONTAINER_KEYWORD;
                                     }

    break;

  case 284:

    { actual = (yyvsp[-1].nodes).container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }

    break;

  case 286:

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

  case 287:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAF, sizeof(struct lys_node_leaf)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   (yyval.nodes).node.ptr_leaf = actual;
                                 }

    break;

  case 288:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LEAF_KEYWORD;
                          }

    break;

  case 289:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 291:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaf->iffeature, (yyvsp[-1].nodes).node.ptr_leaf->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_leaf;
                                     s = NULL;
                                   }

    break;

  case 292:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                               (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                             }

    break;

  case 293:

    { (yyval.nodes) = (yyvsp[-3].nodes);}

    break;

  case 294:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 295:

    { actual = (yyvsp[-1].nodes).node.ptr_leaf;
                               actual_type = LEAF_KEYWORD;
                             }

    break;

  case 297:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 298:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 299:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 300:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 301:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 302:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 303:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAFLIST, sizeof(struct lys_node_leaflist)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                      }

    break;

  case 304:

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

  case 305:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                                 actual_type = LEAF_LIST_KEYWORD;
                               }

    break;

  case 306:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 308:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaflist->iffeature,
                                                       (yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size);
                                          ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                          actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                          s = NULL;
                                        }

    break;

  case 309:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                    (yyvsp[-1].nodes).node.flag |= LYS_TYPE_DEF;
                                  }

    break;

  case 310:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 311:

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

  case 312:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 313:

    { actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                    actual_type = LEAF_LIST_KEYWORD;
                                  }

    break;

  case 315:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 316:

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

  case 317:

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

  case 318:

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

  case 319:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 320:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 321:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 322:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LIST, sizeof(struct lys_node_list)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 323:

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

  case 324:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                            actual_type = LIST_KEYWORD;
                          }

    break;

  case 325:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 327:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->iffeature,
                                                  (yyvsp[-1].nodes).node.ptr_list->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).node.ptr_list;
                                     s = NULL;
                                   }

    break;

  case 328:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                               actual_type = LIST_KEYWORD;
                             }

    break;

  case 330:

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

  case 331:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 332:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 333:

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

  case 334:

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

  case 335:

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

  case 336:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 337:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 338:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 339:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                  actual_type = LIST_KEYWORD;
                                }

    break;

  case 341:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 343:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                 actual_type = LIST_KEYWORD;
                                 data_node = actual;
                               }

    break;

  case 345:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                       actual_type = LIST_KEYWORD;
                                       data_node = actual;
                                       if (trg->version < 2) {
                                         LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "notification");
                                         YYABORT;
                                       }
                                     }

    break;

  case 347:

    { actual = (yyvsp[-1].nodes).node.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   (yyvsp[-1].nodes).node.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }

    break;

  case 348:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 349:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CHOICE, sizeof(struct lys_node_choice)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                   }

    break;

  case 350:

    { LOGDBG("YANG: finished parsing choice statement \"%s\"", data_node->name); }

    break;

  case 352:

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

  case 353:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                              actual_type = CHOICE_KEYWORD;
                            }

    break;

  case 354:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                 actual_type = CHOICE_KEYWORD;
                               }

    break;

  case 355:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 356:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_choice->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_choice->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_choice;
                                       s = NULL;
                                     }

    break;

  case 357:

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

  case 358:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 359:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 360:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 361:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 362:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 363:

    { actual = (yyvsp[-1].nodes).node.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                          }

    break;

  case 364:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 373:

    { if (trg->version < 2 ) {
                     LOGVAL(LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 374:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CASE, sizeof(struct lys_node_case)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                 }

    break;

  case 375:

    { LOGDBG("YANG: finished parsing case statement \"%s\"", data_node->name); }

    break;

  case 377:

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

  case 378:

    { (yyval.nodes).cs = actual;
                            actual_type = CASE_KEYWORD;
                          }

    break;

  case 379:

    { actual = (yyvsp[-1].nodes).cs;
                               actual_type = CASE_KEYWORD;
                             }

    break;

  case 381:

    { YANG_ADDELEM((yyvsp[-1].nodes).cs->iffeature, (yyvsp[-1].nodes).cs->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).cs;
                                     s = NULL;
                                   }

    break;

  case 382:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 383:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 384:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 385:

    { actual = (yyvsp[-1].nodes).cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }

    break;

  case 387:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYXML, sizeof(struct lys_node_anydata)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ANYXML_KEYWORD;
                                   }

    break;

  case 388:

    { LOGDBG("YANG: finished parsing anyxml statement \"%s\"", data_node->name); }

    break;

  case 389:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYDATA, sizeof(struct lys_node_anydata)))) {
                                        YYABORT;
                                      }
                                      data_node = actual;
                                      s = NULL;
                                      actual_type = ANYDATA_KEYWORD;
                                    }

    break;

  case 390:

    { LOGDBG("YANG: finished parsing anydata statement \"%s\"", data_node->name); }

    break;

  case 392:

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

  case 393:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
                            }

    break;

  case 394:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 396:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_anydata->iffeature,
                                                    (yyvsp[-1].nodes).node.ptr_anydata->iffeature_size);
                                       ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                       actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                       s = NULL;
                                     }

    break;

  case 397:

    { actual = (yyvsp[-1].nodes).node.ptr_anydata;
                                 actual_type = (yyvsp[-1].nodes).node.flag;
                               }

    break;

  case 399:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "config",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 400:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_MAND_MASK) {
                                        LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "mandatory",
                                               ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                    }

    break;

  case 401:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_STATUS_MASK) {
                                     LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "status",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 402:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 403:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 404:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_USES, sizeof(struct lys_node_uses)))) {
                                         YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                     }

    break;

  case 405:

    { LOGDBG("YANG: finished parsing uses statement \"%s\"", data_node->name); }

    break;

  case 407:

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

  case 408:

    { (yyval.nodes).uses = actual;
                            actual_type = USES_KEYWORD;
                          }

    break;

  case 409:

    { actual = (yyvsp[-1].nodes).uses;
                               actual_type = USES_KEYWORD;
                             }

    break;

  case 411:

    { YANG_ADDELEM((yyvsp[-1].nodes).uses->iffeature, (yyvsp[-1].nodes).uses->iffeature_size);
                                     ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                     actual = (yyvsp[-1].nodes).uses;
                                     s = NULL;
                                   }

    break;

  case 412:

    { if ((yyvsp[-1].nodes).uses->flags & LYS_STATUS_MASK) {
                                   LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).uses, "status", "uses");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).uses->flags |= (yyvsp[0].i);
                               }

    break;

  case 413:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 414:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 415:

    { actual = (yyvsp[-1].nodes).uses;
                                 actual_type = USES_KEYWORD;
                               }

    break;

  case 417:

    { actual = (yyvsp[-1].nodes).uses;
                                       actual_type = USES_KEYWORD;
                                       data_node = actual;
                                     }

    break;

  case 421:

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

  case 424:

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

  case 425:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 426:

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

  case 427:

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

  case 428:

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

  case 429:

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

  case 430:

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

  case 431:

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

  case 432:

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

  case 433:

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

  case 434:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 435:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 438:

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

  case 442:

    { YANG_ADDELEM(trg->augment, trg->augment_size);
                               if (yang_read_augment(trg, NULL, actual, s)) {
                                 YYABORT;
                               }
                               data_node = actual;
                               s = NULL;
                             }

    break;

  case 444:

    { (yyval.nodes).augment = actual;
                               actual_type = AUGMENT_KEYWORD;
                             }

    break;

  case 445:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                }

    break;

  case 447:

    { YANG_ADDELEM((yyvsp[-1].nodes).augment->iffeature, (yyvsp[-1].nodes).augment->iffeature_size);
                                        ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                        actual = (yyvsp[-1].nodes).augment;
                                        s = NULL;
                                      }

    break;

  case 448:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 449:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 450:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 451:

    { actual = (yyvsp[-1].nodes).augment;
                                      actual_type = AUGMENT_KEYWORD;
                                      data_node = actual;
                                    }

    break;

  case 453:

    { actual = (yyvsp[-1].nodes).augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    data_node = actual;
                                  }

    break;

  case 455:

    { actual = (yyvsp[-1].nodes).augment;
                                          actual_type = AUGMENT_KEYWORD;
                                          data_node = actual;
                                          if (trg->version < 2) {
                                            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "notification");
                                            YYABORT;
                                          }
                                        }

    break;

  case 457:

    { actual = (yyvsp[-1].nodes).augment;
                                  actual_type = AUGMENT_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 459:

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
                                   }

    break;

  case 460:

    { LOGDBG("YANG: finished parsing action statement \"%s\"", data_node->name); }

    break;

  case 461:

    { if (!(actual = yang_read_node(trg, NULL, param->node, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                }

    break;

  case 462:

    { LOGDBG("YANG: finished parsing rpc statement \"%s\"", data_node->name); }

    break;

  case 464:

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

  case 465:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                           actual_type = RPC_KEYWORD;
                         }

    break;

  case 466:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_rpc->iffeature,
                                                 (yyvsp[-1].nodes).node.ptr_rpc->iffeature_size);
                                    ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                    actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                    s = NULL;
                                  }

    break;

  case 467:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 468:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 469:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 470:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                 actual_type = RPC_KEYWORD;
                               }

    break;

  case 472:

    { actual = (yyvsp[-1].nodes).node.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }

    break;

  case 474:

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

  case 475:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 476:

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

  case 477:

    { (yyval.nodes) = (yyvsp[-3].nodes); }

    break;

  case 478:

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

  case 479:

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

  case 480:

    { (yyval.nodes).inout = actual;
                                    actual_type = INPUT_KEYWORD;
                                  }

    break;

  case 481:

    { actual = (yyvsp[-1].nodes).inout;
                                       actual_type = INPUT_KEYWORD;
                                     }

    break;

  case 483:

    { actual = (yyvsp[-1].nodes).inout;
                                          actual_type = INPUT_KEYWORD;
                                        }

    break;

  case 485:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 487:

    { actual = (yyvsp[-1].nodes).inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 489:

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

  case 490:

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

  case 491:

    { if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                         }

    break;

  case 494:

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

  case 495:

    { (yyval.nodes).notif = actual;
                                    actual_type = NOTIFICATION_KEYWORD;
                                  }

    break;

  case 496:

    { actual = (yyvsp[-1].nodes).notif;
                                       actual_type = NOTIFICATION_KEYWORD;
                                     }

    break;

  case 498:

    { YANG_ADDELEM((yyvsp[-1].nodes).notif->iffeature, (yyvsp[-1].nodes).notif->iffeature_size);
                                             ((struct lys_iffeature *)actual)->features = (struct lys_feature**)s;
                                             actual = (yyvsp[-1].nodes).notif;
                                             s = NULL;
                                           }

    break;

  case 499:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 500:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 501:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 502:

    { actual = (yyvsp[-1].nodes).notif;
                                          actual_type = NOTIFICATION_KEYWORD;
                                        }

    break;

  case 504:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 506:

    { actual = (yyvsp[-1].nodes).notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }

    break;

  case 508:

    { YANG_ADDELEM(trg->deviation, trg->deviation_size);
                                   ((struct lys_deviation *)actual)->target_name = transform_schema2json(trg, s);
                                   free(s);
                                   if (!((struct lys_deviation *)actual)->target_name) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 509:

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

  case 510:

    { (yyval.dev) = actual;
                                 actual_type = DEVIATION_KEYWORD;
                               }

    break;

  case 511:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 512:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 513:

    { actual = (yyvsp[-3].dev);
                                                                actual_type = DEVIATION_KEYWORD;
                                                                (yyval.dev) = (yyvsp[-3].dev);
                                                              }

    break;

  case 516:

    { if (yang_read_deviate_unsupported(actual)) {
                       YYABORT;
                     }
                   }

    break;

  case 518:

    {  backup_type = actual_type;
                                                actual_type = NOT_SUPPORTED_KEYWORD;
                                             }

    break;

  case 523:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_ADD))) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 526:

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

  case 527:

    { (yyval.deviate) = actual;
                                   actual_type = ADD_KEYWORD;
                                 }

    break;

  case 528:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 529:

    { actual = (yyvsp[-2].deviate);
                                              actual_type = ADD_KEYWORD;
                                              (yyval.deviate) = (yyvsp[-2].deviate);
                                            }

    break;

  case 530:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 531:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 532:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 533:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 534:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 535:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 536:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_DEL))) {
                                               YYABORT;
                                             }
                                           }

    break;

  case 539:

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

  case 540:

    { (yyval.deviate) = actual;
                                      actual_type = DELETE_KEYWORD;
                                    }

    break;

  case 541:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 542:

    { actual = (yyvsp[-2].deviate);
                                                 actual_type = DELETE_KEYWORD;
                                                 (yyval.deviate) = (yyvsp[-2].deviate);
                                               }

    break;

  case 543:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 544:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 545:

    { if (!(actual = yang_read_deviate(actual, LY_DEVIATE_RPL))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 548:

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

  case 549:

    { (yyval.deviate) = actual;
                                       actual_type = REPLACE_KEYWORD;
                                     }

    break;

  case 550:

    { actual = (yyvsp[-2].deviate);
                                                  actual_type = REPLACE_KEYWORD;
                                                }

    break;

  case 551:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 552:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 553:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 554:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 555:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 556:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 557:

    { if (!(actual=yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type=WHEN_KEYWORD;
                      }

    break;

  case 562:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 563:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 564:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = CONFIG_KEYWORD;
                           }

    break;

  case 565:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 566:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 567:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 568:

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

  case 569:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = MANDATORY_KEYWORD;
                                 }

    break;

  case 570:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 571:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 572:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 573:

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

  case 574:

    { backup_type = actual_type;
                       actual_type = PRESENCE_KEYWORD;
                     }

    break;

  case 576:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MIN_ELEMENTS_KEYWORD;
                                 }

    break;

  case 577:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 578:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 579:

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

  case 580:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MAX_ELEMENTS_KEYWORD;
                                 }

    break;

  case 581:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 582:

    { (yyval.uint) = 0; }

    break;

  case 583:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 584:

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

  case 585:

    { (yyval.i) = (yyvsp[0].i);
                                     backup_type = actual_type;
                                     actual_type = ORDERED_BY_KEYWORD;
                                   }

    break;

  case 586:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 587:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 588:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 589:

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

  case 590:

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

  case 594:

    { backup_type = actual_type;
                             actual_type = UNIQUE_KEYWORD;
                           }

    break;

  case 600:

    { backup_type = actual_type;
                       actual_type = KEY_KEYWORD;
                     }

    break;

  case 602:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM;
                                 YYABORT;
                               }
                             }

    break;

  case 605:

    { (yyval.v) = actual;
                        if (!(actual = yang_read_range(trg, actual, s))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 606:

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

  case 610:

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

  case 612:

    { tmp_s = yyget_text(scanner); }

    break;

  case 613:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM;
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 614:

    { tmp_s = yyget_text(scanner); }

    break;

  case 615:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM;
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 639:

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

  case 640:

    { (yyval.uint) = 0; }

    break;

  case 641:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 642:

    { (yyval.i) = 0; }

    break;

  case 643:

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

  case 649:

    { if (lyp_check_identifier(s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 654:

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

  case 655:

    { s = (yyvsp[-1].str); }

    break;

  case 656:

    { s = (yyvsp[-3].str); }

    break;

  case 657:

    { actual_type = backup_type;
                 backup_type = NODE;
                 (yyval.str) = s;
                 s = NULL;
               }

    break;

  case 658:

    { actual_type = backup_type;
                           backup_type = NODE;
                         }

    break;

  case 659:

    { (yyval.str) = s;
                          s = NULL;
                        }

    break;

  case 664:

    { if (!yang_read_ext(trg, actual, (yyvsp[-1].str), s, actual_type, backup_type)) {
                                                  YYABORT;
                                                }
                                                s = NULL;
                                              }

    break;

  case 665:

    { (yyval.str) = s; s = NULL; }

    break;

  case 681:

    { s = strdup(yyget_text(scanner));
                  if (!s) {
                    LOGMEM;
                    YYABORT;
                  }
                }

    break;

  case 772:

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
