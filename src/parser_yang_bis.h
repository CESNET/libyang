/* A Bison parser, made by GNU Bison 3.6.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_YANG_BIS_H_INCLUDED
# define YY_YY_PARSER_YANG_BIS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    UNION_KEYWORD = 258,           /* UNION_KEYWORD  */
    ANYXML_KEYWORD = 259,          /* ANYXML_KEYWORD  */
    WHITESPACE = 260,              /* WHITESPACE  */
    TOKEN_ERROR = 261,             /* TOKEN_ERROR  */
    EOL = 262,                     /* EOL  */
    STRING = 263,                  /* STRING  */
    STRINGS = 264,                 /* STRINGS  */
    IDENTIFIER = 265,              /* IDENTIFIER  */
    IDENTIFIERPREFIX = 266,        /* IDENTIFIERPREFIX  */
    REVISION_DATE = 267,           /* REVISION_DATE  */
    TAB = 268,                     /* TAB  */
    DOUBLEDOT = 269,               /* DOUBLEDOT  */
    URI = 270,                     /* URI  */
    INTEGER = 271,                 /* INTEGER  */
    NON_NEGATIVE_INTEGER = 272,    /* NON_NEGATIVE_INTEGER  */
    ZERO = 273,                    /* ZERO  */
    TOKEN_DECIMAL = 274,           /* TOKEN_DECIMAL  */
    ARGUMENT_KEYWORD = 275,        /* ARGUMENT_KEYWORD  */
    AUGMENT_KEYWORD = 276,         /* AUGMENT_KEYWORD  */
    BASE_KEYWORD = 277,            /* BASE_KEYWORD  */
    BELONGS_TO_KEYWORD = 278,      /* BELONGS_TO_KEYWORD  */
    BIT_KEYWORD = 279,             /* BIT_KEYWORD  */
    CASE_KEYWORD = 280,            /* CASE_KEYWORD  */
    CHOICE_KEYWORD = 281,          /* CHOICE_KEYWORD  */
    CONFIG_KEYWORD = 282,          /* CONFIG_KEYWORD  */
    CONTACT_KEYWORD = 283,         /* CONTACT_KEYWORD  */
    CONTAINER_KEYWORD = 284,       /* CONTAINER_KEYWORD  */
    DEFAULT_KEYWORD = 285,         /* DEFAULT_KEYWORD  */
    DESCRIPTION_KEYWORD = 286,     /* DESCRIPTION_KEYWORD  */
    ENUM_KEYWORD = 287,            /* ENUM_KEYWORD  */
    ERROR_APP_TAG_KEYWORD = 288,   /* ERROR_APP_TAG_KEYWORD  */
    ERROR_MESSAGE_KEYWORD = 289,   /* ERROR_MESSAGE_KEYWORD  */
    EXTENSION_KEYWORD = 290,       /* EXTENSION_KEYWORD  */
    DEVIATION_KEYWORD = 291,       /* DEVIATION_KEYWORD  */
    DEVIATE_KEYWORD = 292,         /* DEVIATE_KEYWORD  */
    FEATURE_KEYWORD = 293,         /* FEATURE_KEYWORD  */
    FRACTION_DIGITS_KEYWORD = 294, /* FRACTION_DIGITS_KEYWORD  */
    GROUPING_KEYWORD = 295,        /* GROUPING_KEYWORD  */
    IDENTITY_KEYWORD = 296,        /* IDENTITY_KEYWORD  */
    IF_FEATURE_KEYWORD = 297,      /* IF_FEATURE_KEYWORD  */
    IMPORT_KEYWORD = 298,          /* IMPORT_KEYWORD  */
    INCLUDE_KEYWORD = 299,         /* INCLUDE_KEYWORD  */
    INPUT_KEYWORD = 300,           /* INPUT_KEYWORD  */
    KEY_KEYWORD = 301,             /* KEY_KEYWORD  */
    LEAF_KEYWORD = 302,            /* LEAF_KEYWORD  */
    LEAF_LIST_KEYWORD = 303,       /* LEAF_LIST_KEYWORD  */
    LENGTH_KEYWORD = 304,          /* LENGTH_KEYWORD  */
    LIST_KEYWORD = 305,            /* LIST_KEYWORD  */
    MANDATORY_KEYWORD = 306,       /* MANDATORY_KEYWORD  */
    MAX_ELEMENTS_KEYWORD = 307,    /* MAX_ELEMENTS_KEYWORD  */
    MIN_ELEMENTS_KEYWORD = 308,    /* MIN_ELEMENTS_KEYWORD  */
    MODULE_KEYWORD = 309,          /* MODULE_KEYWORD  */
    MUST_KEYWORD = 310,            /* MUST_KEYWORD  */
    NAMESPACE_KEYWORD = 311,       /* NAMESPACE_KEYWORD  */
    NOTIFICATION_KEYWORD = 312,    /* NOTIFICATION_KEYWORD  */
    ORDERED_BY_KEYWORD = 313,      /* ORDERED_BY_KEYWORD  */
    ORGANIZATION_KEYWORD = 314,    /* ORGANIZATION_KEYWORD  */
    OUTPUT_KEYWORD = 315,          /* OUTPUT_KEYWORD  */
    PATH_KEYWORD = 316,            /* PATH_KEYWORD  */
    PATTERN_KEYWORD = 317,         /* PATTERN_KEYWORD  */
    POSITION_KEYWORD = 318,        /* POSITION_KEYWORD  */
    PREFIX_KEYWORD = 319,          /* PREFIX_KEYWORD  */
    PRESENCE_KEYWORD = 320,        /* PRESENCE_KEYWORD  */
    RANGE_KEYWORD = 321,           /* RANGE_KEYWORD  */
    REFERENCE_KEYWORD = 322,       /* REFERENCE_KEYWORD  */
    REFINE_KEYWORD = 323,          /* REFINE_KEYWORD  */
    REQUIRE_INSTANCE_KEYWORD = 324, /* REQUIRE_INSTANCE_KEYWORD  */
    REVISION_KEYWORD = 325,        /* REVISION_KEYWORD  */
    REVISION_DATE_KEYWORD = 326,   /* REVISION_DATE_KEYWORD  */
    RPC_KEYWORD = 327,             /* RPC_KEYWORD  */
    STATUS_KEYWORD = 328,          /* STATUS_KEYWORD  */
    SUBMODULE_KEYWORD = 329,       /* SUBMODULE_KEYWORD  */
    TYPE_KEYWORD = 330,            /* TYPE_KEYWORD  */
    TYPEDEF_KEYWORD = 331,         /* TYPEDEF_KEYWORD  */
    UNIQUE_KEYWORD = 332,          /* UNIQUE_KEYWORD  */
    UNITS_KEYWORD = 333,           /* UNITS_KEYWORD  */
    USES_KEYWORD = 334,            /* USES_KEYWORD  */
    VALUE_KEYWORD = 335,           /* VALUE_KEYWORD  */
    WHEN_KEYWORD = 336,            /* WHEN_KEYWORD  */
    YANG_VERSION_KEYWORD = 337,    /* YANG_VERSION_KEYWORD  */
    YIN_ELEMENT_KEYWORD = 338,     /* YIN_ELEMENT_KEYWORD  */
    ADD_KEYWORD = 339,             /* ADD_KEYWORD  */
    CURRENT_KEYWORD = 340,         /* CURRENT_KEYWORD  */
    DELETE_KEYWORD = 341,          /* DELETE_KEYWORD  */
    DEPRECATED_KEYWORD = 342,      /* DEPRECATED_KEYWORD  */
    FALSE_KEYWORD = 343,           /* FALSE_KEYWORD  */
    NOT_SUPPORTED_KEYWORD = 344,   /* NOT_SUPPORTED_KEYWORD  */
    OBSOLETE_KEYWORD = 345,        /* OBSOLETE_KEYWORD  */
    REPLACE_KEYWORD = 346,         /* REPLACE_KEYWORD  */
    SYSTEM_KEYWORD = 347,          /* SYSTEM_KEYWORD  */
    TRUE_KEYWORD = 348,            /* TRUE_KEYWORD  */
    UNBOUNDED_KEYWORD = 349,       /* UNBOUNDED_KEYWORD  */
    USER_KEYWORD = 350,            /* USER_KEYWORD  */
    ACTION_KEYWORD = 351,          /* ACTION_KEYWORD  */
    MODIFIER_KEYWORD = 352,        /* MODIFIER_KEYWORD  */
    ANYDATA_KEYWORD = 353,         /* ANYDATA_KEYWORD  */
    NODE = 354,                    /* NODE  */
    NODE_PRINT = 355,              /* NODE_PRINT  */
    EXTENSION_INSTANCE = 356,      /* EXTENSION_INSTANCE  */
    SUBMODULE_EXT_KEYWORD = 357    /* SUBMODULE_EXT_KEYWORD  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

  int64_t i;
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
