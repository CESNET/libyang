%{
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include "resolve.h"
#include "common.h"
#include "parser_yang.h"
#include "parser.h"

/* only syntax rules */
#define EXTENSION_ARG 0x01
#define EXTENSION_STA 0x02
#define EXTENSION_DSC 0x04
#define EXTENSION_REF 0x08

extern int yylineno;
extern int yyleng;
void yyerror(struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all, char *str, ...);   //parameter is in directive parse-param
int yylex(void);
extern char *yytext;
char *s, *tmp_s;
char rev[LY_REV_SIZE];
struct lys_module *trg;
struct lys_node *data_node;
void *actual;
int actual_type;
int tmp_line;
int64_t cnt_val;
%}

%parse-param {struct lys_module *module} {struct lys_submodule *submodule} {struct unres_schema *unres} {struct lys_array_size *size_arrays} {int read_all}

%union {
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
    struct lys_node_case *cs;
    struct lys_node_grp *grouping;
    struct type_leaf leaf;
    struct type_leaflist leaflist;
    struct type_list list;
    struct type_tpdf tpdf;
    struct lys_node_uses *uses;
    struct lys_refine *refine;
    struct type_augment augment;
    struct type_rpc rpc;
    struct type_inout inout;
    struct lys_node_notif *notif;
    struct type_deviation *deviation;
  } nodes;
}

%token UNION_KEYWORD
%token ANYXML_KEYWORD
%token WHITESPACE
%token ERROR
%token EOL
%token STRING
%token STRINGS
%token IDENTIFIER
%token IDENTIFIERPREFIX
%token REVISION_DATE
%token TAB
%token DOUBLEDOT
%token URI
%token INTEGER
%token NON_NEGATIVE_INTEGER
%token ZERO
%token DECIMAL
%token ARGUMENT_KEYWORD
%token AUGMENT_KEYWORD
%token BASE_KEYWORD
%token BELONGS_TO_KEYWORD
%token BIT_KEYWORD
%token CASE_KEYWORD
%token CHOICE_KEYWORD
%token CONFIG_KEYWORD
%token CONTACT_KEYWORD
%token CONTAINER_KEYWORD
%token DEFAULT_KEYWORD
%token DESCRIPTION_KEYWORD
%token ENUM_KEYWORD
%token ERROR_APP_TAG_KEYWORD
%token ERROR_MESSAGE_KEYWORD
%token EXTENSION_KEYWORD
%token DEVIATION_KEYWORD
%token DEVIATE_KEYWORD
%token FEATURE_KEYWORD
%token FRACTION_DIGITS_KEYWORD
%token GROUPING_KEYWORD
%token IDENTITY_KEYWORD
%token IF_FEATURE_KEYWORD
%token IMPORT_KEYWORD
%token INCLUDE_KEYWORD
%token INPUT_KEYWORD
%token KEY_KEYWORD
%token LEAF_KEYWORD
%token LEAF_LIST_KEYWORD
%token LENGTH_KEYWORD
%token LIST_KEYWORD
%token MANDATORY_KEYWORD
%token MAX_ELEMENTS_KEYWORD
%token MIN_ELEMENTS_KEYWORD
%token MODULE_KEYWORD
%token MUST_KEYWORD
%token NAMESPACE_KEYWORD
%token NOTIFICATION_KEYWORD
%token ORDERED_BY_KEYWORD
%token ORGANIZATION_KEYWORD
%token OUTPUT_KEYWORD
%token PATH_KEYWORD
%token PATTERN_KEYWORD
%token POSITION_KEYWORD
%token PREFIX_KEYWORD
%token PRESENCE_KEYWORD
%token RANGE_KEYWORD
%token REFERENCE_KEYWORD
%token REFINE_KEYWORD
%token REQUIRE_INSTANCE_KEYWORD
%token REVISION_KEYWORD
%token REVISION_DATE_KEYWORD
%token RPC_KEYWORD
%token STATUS_KEYWORD
%token SUBMODULE_KEYWORD
%token TYPE_KEYWORD
%token TYPEDEF_KEYWORD
%token UNIQUE_KEYWORD
%token UNITS_KEYWORD
%token USES_KEYWORD
%token VALUE_KEYWORD
%token WHEN_KEYWORD
%token YANG_VERSION_KEYWORD
%token YIN_ELEMENT_KEYWORD
%token ADD_KEYWORD
%token CURRENT_KEYWORD
%token DELETE_KEYWORD
%token DEPRECATED_KEYWORD
%token FALSE_KEYWORD
%token MAX_KEYWORD
%token MIN_KEYWORD
%token NOT_SUPPORTED_KEYWORD
%token OBSOLETE_KEYWORD
%token REPLACE_KEYWORD
%token SYSTEM_KEYWORD
%token TRUE_KEYWORD
%token UNBOUNDED_KEYWORD
%token USER_KEYWORD

%type <uint> positive_integer_value
%type <uint> non_negative_integer_value
%type <uint> max_value_arg_str
%type <uint> max_elements_stmt
%type <uint> min_value_arg_str
%type <uint> min_elements_stmt
%type <uint> decimal_string_restrictions
%type <uint> fraction_digits_arg_str
%type <uint> position_value_arg_str
%type <uint> extension_opt_stmt
%type <i> module_header_stmts
%type <i> submodule_header_stmts
%type <str> tmp_identifier_arg_str
%type <i> status_stmt
%type <i> status_arg_str
%type <i> config_stmt
%type <i> config_arg_str
%type <i> mandatory_stmt
%type <i> mandatory_arg_str
%type <i> ordered_by_stmt
%type <i> ordered_by_arg_str
%type <i> integer_value_arg_str
%type <i> integer_value
%type <v> length_arg_str
%type <v> pattern_arg_str
%type <v> range_arg_str
%type <v> enum_arg_str
%type <v> bit_arg_str
%type <v> union_spec
%type <v> typedef_arg_str
%type <nodes> container_opt_stmt
%type <nodes> anyxml_opt_stmt
%type <nodes> choice_opt_stmt
%type <nodes> case_opt_stmt
%type <nodes> grouping_opt_stmt
%type <nodes> leaf_opt_stmt
%type <nodes> leaf_list_opt_stmt
%type <nodes> list_opt_stmt
%type <nodes> type_opt_stmt
%type <nodes> uses_opt_stmt
%type <nodes> refine_body_opt_stmts
%type <nodes> augment_opt_stmt
%type <nodes> rpc_opt_stmt
%type <nodes> input_output_opt_stmt
%type <nodes> notification_opt_stmt
%type <nodes> deviation_opt_stmt
%type <nodes> deviate_add_opt_stmt
%type <nodes> deviate_delete_opt_stmt
%type <nodes> deviate_replace_opt_stmt
%type <inc> include_stmt

%destructor { free($$); } tmp_identifier_arg_str
%destructor { if (read_all && $$.choice.s) { free($$.choice.s); } } choice_opt_stmt
%destructor { if (read_all) {
                free($$.deviation);
              }
            } deviation_opt_stmt

%%

/* to simplify code, store the module/submodule being processed as trg */

start: module_stmt 
 |  submodule_stmt { if (read_all && lyp_propagate_submodule(module, submodule, 1)) {
                       YYERROR;
                     }
                   }


string_1: STRING  { if (read_all) {
                      s = strdup(yytext);
                      if (!s) {
                        LOGMEM;
                        YYERROR;
                      }
                    }
                  }
optsep string_2


string_2: %empty
  |  string_2 '+' optsep 
     STRING { if (read_all){
                s = realloc(s,yyleng+strlen(s)+1);
                if (s) {
                  strcat(s,yytext);
                } else {
                  LOGMEM;
                  YYERROR;
                }
              }
            }
     optsep;

module_stmt: optsep MODULE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                              if (submodule) {
                                                                LOGVAL(LYE_INSTMT, yylineno, LY_VLOG_NONE, NULL, "module");
                                                                YYERROR;
                                                              }
                                                              trg = module;
                                                              yang_read_common(trg,s,MODULE_KEYWORD,0);
                                                              s = NULL;
                                                            }
                                                          }
             '{' stmtsep
                 module_header_stmts { if (read_all && !module->ns) { LOGVAL(LYE_MISSSTMT2,yylineno,LY_VLOG_NONE,NULL,"namespace", "module"); YYERROR; }
                                       if (read_all && !module->prefix) { LOGVAL(LYE_MISSSTMT2,yylineno,LY_VLOG_NONE,NULL,"prefix", "module"); YYERROR; }
                                     }
                 linkage_stmts
                 meta_stmts
                 revision_stmts
                 body_stmts
             '}' optsep

module_header_stmts: %empty  { $$ = 0; }
  |  module_header_stmts yang_version_stmt { if ($1) { LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "yang version", "module"); YYERROR; } $$ = 1; }
  |  module_header_stmts namespace_stmt { if (read_all && yang_read_common(module,s,NAMESPACE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  module_header_stmts prefix_stmt { if (read_all && yang_read_prefix(module,NULL,s,MODULE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  ;

submodule_stmt: optsep SUBMODULE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                                    if (!submodule) {
                                                                      LOGVAL(LYE_INSTMT, yylineno, LY_VLOG_NONE, NULL, "submodule");
                                                                      YYERROR;
                                                                    }
                                                                    trg = (struct lys_module *)submodule;
                                                                    yang_read_common(trg,s,MODULE_KEYWORD,0);
                                                                    s = NULL;
                                                                  }
                                                                }
                '{' stmtsep
                    submodule_header_stmts  { if (read_all && !submodule->prefix) {
                                                LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                                YYERROR;
                                              }
                                            }
                    linkage_stmts
                    meta_stmts
                    revision_stmts
                    body_stmts
                '}' optsep

submodule_header_stmts: %empty { $$ = 0; }
  |  submodule_header_stmts yang_version_stmt { if ($1) {
                                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "yang version", "submodule");
                                                  YYERROR;
                                                }
                                                $$ = 1;
                                              }
  |  submodule_header_stmts { if (read_all) {
                                if (submodule->prefix) {
                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                  YYERROR;
                                }
                              }
                            }
     belongs_to_stmt
  
yang_version_stmt: YANG_VERSION_KEYWORD sep yang_version_arg_str stmtend;

yang_version_arg_str: NON_NEGATIVE_INTEGER { if (strlen(yytext)!=1 || yytext[0]!='1') {YYERROR;} } optsep
  | string_1;

namespace_stmt: NAMESPACE_KEYWORD sep uri_str stmtend;

uri_str: string_1
  /*|  URI*/
  ;

linkage_stmts: %empty { if (read_all) {
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
 |  linkage_stmts import_stmt 
 |  linkage_stmts include_stmt
 ;

import_stmt: IMPORT_KEYWORD sep tmp_identifier_arg_str {
                 if (!read_all) {
                   size_arrays->imp++;
                 } else {
                   actual = &trg->imp[trg->imp_size];
                 }
             }
             '{' stmtsep
                 prefix_stmt { if (read_all) {
                                 if (yang_read_prefix(trg,actual,s,IMPORT_KEYWORD,yylineno)) {YYERROR;}
                                 s=NULL;
                                 actual_type=IMPORT_KEYWORD;
                               }
                             }
                 revision_date_opt
             '}' stmtsep { if (read_all && yang_fill_import(trg,actual,$3,yylineno)) {YYERROR;} }

tmp_identifier_arg_str: identifier_arg_str { $$ = s; s = NULL; }

include_stmt: INCLUDE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                             memset(rev, 0, LY_REV_SIZE);
                                                             actual_type = INCLUDE_KEYWORD;
                                                           }
                                                           else {
                                                             size_arrays->inc++;
                                                           }
                                                         }
              include_end stmtsep { if (read_all) {
                                      $$ = trg;
                                      if (yang_fill_include(module, submodule, s, rev, size_arrays->inc, unres, yylineno)) {
                                        YYERROR;
                                      }
                                      size_arrays->inc++;
                                      s = NULL;
                                      trg = $$;
                                    }
                                  }

include_end: ';'
  | '{' stmtsep
     revision_date_opt 
    '}'
  ;  

revision_date_opt: %empty 
  | revision_date_stmt;

revision_date_stmt: REVISION_DATE_KEYWORD sep revision_date optsep stmtend;

revision_date: REVISION_DATE { if (read_all) {
                                 if (actual_type==IMPORT_KEYWORD) {
                                     memcpy(((struct lys_import *)actual)->rev,yytext,LY_REV_SIZE-1);
                                 } else {                              // INCLUDE KEYWORD
                                     memcpy(rev,yytext,LY_REV_SIZE - 1);
                                 }
                               }
                             }

belongs_to_stmt: BELONGS_TO_KEYWORD sep identifier_arg_str { if (read_all) {
                                                               if (!ly_strequal(s, submodule->belongsto->name, 1)) {
                                                                 LOGVAL(LYE_INARG, yylineno, LY_VLOG_NONE, NULL, s, "belongs-to");
                                                                 free(s);
                                                                 YYERROR;
                                                               }
                                                               free(s);
                                                               s = NULL;
                                                             }
                                                           }
                 '{' stmtsep
                     prefix_stmt { if (read_all) {
                                     if (lyp_check_identifier(s, LY_IDENT_NAME, yylineno, NULL, NULL)) {
                                       YYERROR;
                                     }
                                     submodule->prefix = lydict_insert_zc(module->ctx, s);
                                     s = NULL;
                                   }
                                 }
                 '}' stmtsep

prefix_stmt: PREFIX_KEYWORD sep prefix_arg_str stmtend;

meta_stmts: %empty 
  |  meta_stmts organization_stmt { if (read_all && yang_read_common(trg,s,ORGANIZATION_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  meta_stmts contact_stmt { if (read_all && yang_read_common(trg,s,CONTACT_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  meta_stmts description_stmt { if (read_all && yang_read_description(trg,NULL,s,MODULE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  meta_stmts reference_stmt { if (read_all && yang_read_reference(trg,NULL,s,MODULE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  ;

organization_stmt: ORGANIZATION_KEYWORD sep string stmtend;

contact_stmt: CONTACT_KEYWORD sep string stmtend;

description_stmt: DESCRIPTION_KEYWORD sep string stmtend;

reference_stmt: REFERENCE_KEYWORD sep string stmtend;

revision_stmts: %empty { if (read_all) {
                           trg->rev = calloc(size_arrays->rev, sizeof *trg->rev);
                         }
                       }
  | revision_stmts revision_stmt stmtsep;


revision_stmt: REVISION_KEYWORD sep date_arg_str { if (read_all) {
                                                     if(!(actual=yang_read_revision(trg,s))) {YYERROR;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }
               revision_end;

revision_end: ';'
  | '{' stmtsep { actual_type = REVISION_KEYWORD; }
        description_reference_stmt
     '}'
  ;

description_reference_stmt: %empty 
  |  description_reference_stmt description_stmt { if (read_all && yang_read_description(trg,actual,s,actual_type,yylineno)) {YYERROR;} s=NULL; }
  |  description_reference_stmt reference_stmt { if (read_all && yang_read_reference(trg,actual,s,actual_type,yylineno)) {YYERROR;} s=NULL; }
  ;

date_arg_str: REVISION_DATE { if (read_all) {
                                s = strdup(yytext);
                                if (!s) {
                                  LOGMEM;
                                  YYERROR;
                                }
                              }
                            }
              optsep
  | string_1
  ;

body_stmts: %empty { if (read_all) {
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
  | body_stmts body_stmt stmtsep { actual = NULL; }


body_stmt: extension_stmt
  | feature_stmt
  | identity_stmt
  | typedef_stmt { if (!read_all) { size_arrays->tpdf++; } }
  | grouping_stmt
  | data_def_stmt
  | augment_stmt { if (!read_all) { size_arrays->augment++; } }
  | rpc_stmt 
  | notification_stmt 
  | deviation_stmt { if (!read_all) { size_arrays->deviation++; } }

extension_stmt: EXTENSION_KEYWORD sep identifier_arg_str { if (read_all) {
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
                extension_end

extension_end: ';' 
  | '{' stmtsep
        extension_opt_stmt
    '}'

extension_opt_stmt: %empty { $$ = 0; }
  |  extension_opt_stmt argument_stmt { if ($1 & EXTENSION_ARG) {
                                          LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "argument", "extension");
                                          YYERROR;
                                        }
                                        $1 |= EXTENSION_ARG;
                                        $$ = $1;
                                      }
  |  extension_opt_stmt status_stmt { if ($1 & EXTENSION_STA) {
                                        LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYERROR;
                                      }
                                      $1 |= EXTENSION_STA;
                                      $$ = $1;
                                    }
  |  extension_opt_stmt description_stmt { if (read_all) {
                                             free(s);
                                             s= NULL;
                                           }
                                           if ($1 & EXTENSION_DSC) {
                                             LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "description", "extension");
                                             YYERROR;
                                           }
                                           $1 |= EXTENSION_DSC;
                                           $$ = $1;
                                         }
  |  extension_opt_stmt reference_stmt { if (read_all) {
                                           free(s);
                                           s = NULL;
                                         }
                                         if ($1 & EXTENSION_REF) {
                                           LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "reference", "extension");
                                           YYERROR;
                                         }
                                         $1 |= EXTENSION_REF;
                                         $$ = $1;
                                       }

argument_stmt: ARGUMENT_KEYWORD sep identifier_arg_str { free(s); s = NULL; } argument_end stmtsep;
                     
argument_end: ';'
  | '{' stmtsep
        yin_element_stmt
    '}'

yin_element_stmt: YIN_ELEMENT_KEYWORD sep yin_element_arg_str stmtend;

yin_element_arg_str: TRUE_KEYWORD optsep
  | FALSE_KEYWORD optsep
  | string_1 { if (read_all) {
                 if (strcmp(s, "true") && strcmp(s, "false")) {
                    LOGVAL(LYE_INSTMT, yylineno, LY_VLOG_NONE, NULL, s);
                    free(s);
                    YYERROR;
                 }
                 free(s);
                 s = NULL;
               }
             }

status_stmt:  STATUS_KEYWORD sep status_arg_str stmtend { $$ = $3; }

status_arg_str: CURRENT_KEYWORD optsep { $$ = LYS_STATUS_CURR; }
  | OBSOLETE_KEYWORD optsep { $$ = LYS_STATUS_OBSLT; }
  | DEPRECATED_KEYWORD optsep { $$ = LYS_STATUS_DEPRC; }
  | string_1 { if (read_all) {
                 if (!strcmp(s, "current")) {
                   $$ = LYS_STATUS_CURR;
                 } else if (!strcmp(s, "obsolete")) {
                   $$ = LYS_STATUS_OBSLT;
                 } else if (!strcmp(s, "deprecated")) {
                   $$ = LYS_STATUS_DEPRC;
                 } else {
                   LOGVAL(LYE_INSTMT, yylineno, LY_VLOG_NONE, NULL, s);
                   free(s);
                   YYERROR;
                 }
                 free(s);
                 s = NULL;
               }
             }

feature_stmt: FEATURE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                         if (!(actual = yang_read_feature(trg,s,yylineno))) {YYERROR;}
                                                         s=NULL; 
                                                       } else {
                                                         size_arrays->features++;
                                                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                           LOGMEM;
                                                           YYERROR;
                                                         }
                                                       }
                                                     }
              feature_end { if (read_all) { size_arrays->next++; } }

feature_end: ';' 
  | '{' stmtsep
        feature_opt_stmt
    '}' 
  ;

feature_opt_stmt: %empty { if (read_all) {
                             ((struct lys_feature*)actual)->features = calloc(size_arrays->node[size_arrays->next].if_features, 
                                                                              sizeof *((struct lys_feature*)actual)->features);
                             if (!((struct lys_feature*)actual)->features) {
                               LOGMEM;
                               YYERROR;
                             }
                           } 
                         }
  |  feature_opt_stmt if_feature_stmt { if (read_all) {
                                          if (yang_read_if_feature(trg,actual,s,unres,FEATURE_KEYWORD,yylineno)) {YYERROR;}
                                          s=NULL; 
                                        } else {
                                          size_arrays->node[size_arrays->size-1].if_features++;
                                        }
                                      }
  |  feature_opt_stmt status_stmt { if (read_all && yang_read_status(actual,$2,FEATURE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  feature_opt_stmt description_stmt { if (read_all && yang_read_description(trg,actual,s,FEATURE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  feature_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,actual,s,FEATURE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  ;

if_feature_stmt: IF_FEATURE_KEYWORD sep identifier_ref_arg_str stmtend;

identity_stmt: IDENTITY_KEYWORD sep identifier_arg_str { if (read_all) {
                                                           if (!(actual = yang_read_identity(trg,s))) {YYERROR;}
                                                           s = NULL;
                                                         } else {
                                                           size_arrays->ident++;
                                                         }
                                                       }
               identity_end;

identity_end: ';' 
  |  '{' stmtsep
         identity_opt_stmt
      '}'
  ;

identity_opt_stmt: %empty 
  |  identity_opt_stmt base_stmt { if (read_all && yang_read_base(trg,actual,s,unres,yylineno)) {YYERROR;} s = NULL; }
  |  identity_opt_stmt status_stmt { if (read_all && yang_read_status(actual,$2,IDENTITY_KEYWORD,yylineno)) {YYERROR;} }
  |  identity_opt_stmt description_stmt { if (read_all && yang_read_description(trg,actual,s,IDENTITY_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  identity_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,actual,s,IDENTITY_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

base_stmt: BASE_KEYWORD sep identifier_ref_arg_str stmtend;

typedef_stmt: TYPEDEF_KEYWORD sep typedef_arg_str
              '{' stmtsep
                  type_opt_stmt { if (read_all) {
                                    if (!($6.tpdf.flag & LYS_TYPE_DEF)) {
                                      LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, NULL, "type", "typedef");
                                      YYERROR;
                                    }
                                    if (unres_schema_add_node(trg, unres, &$6.tpdf.ptr_tpdf->type, UNRES_TYPE_DER,(struct lys_node *) $3, 0)) {
                                      YYERROR;
                                    }
                                    actual = $3;

                                    /* check default value */
                                    if ($6.tpdf.ptr_tpdf->dflt) {
                                      if (unres_schema_add_str(trg, unres, &$6.tpdf.ptr_tpdf->type, UNRES_TYPE_DFLT, $6.tpdf.ptr_tpdf->dflt, $6.tpdf.line) == -1) {
                                        YYERROR;
                                      }
                                    }
                                  }
                                }
               '}' ;

typedef_arg_str: identifier_arg_str { if (read_all) {
                                        $$ = actual;
                                        if (!(actual = yang_read_typedef(trg, actual, s, yylineno))) {
                                          YYERROR;
                                        }
                                        s = NULL;
                                        actual_type = TYPEDEF_KEYWORD;
                                      }
                                    }
  
type_stmt: TYPE_KEYWORD sep identifier_ref_arg_str { if (read_all && !(actual = yang_read_type(trg, actual, s, actual_type, yylineno))) {
                                                       YYERROR;
                                                     }
                                                     s = NULL;
                                                   }
           type_end;

type_opt_stmt: %empty { $$.tpdf.ptr_tpdf = actual; }
  |  type_opt_stmt { if (read_all && ($1.tpdf.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, $1.tpdf.ptr_tpdf, "type", "typedef");
                       YYERROR;
                     }
                   }
     type_stmt stmtsep { if (read_all) {
                   actual = $1.tpdf.ptr_tpdf;
                   actual_type = TYPEDEF_KEYWORD;
                   $1.tpdf.flag |= LYS_TYPE_DEF;
                   $$ = $1;
                 }
               }
  |  type_opt_stmt units_stmt { if (read_all && yang_read_units(trg, $1.tpdf.ptr_tpdf, s, TYPEDEF_KEYWORD, yylineno)) {YYERROR;} s = NULL; }
  |  type_opt_stmt default_stmt { if (read_all && yang_read_default(trg, $1.tpdf.ptr_tpdf, s, TYPEDEF_KEYWORD, yylineno)) {
                                    YYERROR;
                                  }
                                  s = NULL;
                                  $1.tpdf.line = yylineno;
                                  $$ = $1;
                                }
  |  type_opt_stmt status_stmt { if (read_all && yang_read_status($1.tpdf.ptr_tpdf, $2, TYPEDEF_KEYWORD, yylineno)) {YYERROR;} }
  |  type_opt_stmt description_stmt { if (read_all && yang_read_description(trg, $1.tpdf.ptr_tpdf, s, TYPEDEF_KEYWORD, yylineno)) {YYERROR;} s = NULL; }
  |  type_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg, $1.tpdf.ptr_tpdf, s, TYPEDEF_KEYWORD, yylineno)) {YYERROR;} s = NULL; }
  ;

type_end: ';' 
  |  '{' stmtsep
         type_body_stmts
      '}'
  ;


type_body_stmts: decimal_string_restrictions
  | enum_specification 
  | path_stmt  { /*leafref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->type->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }
  | base_stmt  { /*identityref_specification */
                 if (read_all) {
                   ((struct yang_type *)actual)->flags |= LYS_TYPE_BASE;
                   ((struct yang_type *)actual)->type->base = LY_TYPE_LEAFREF;
                   ((struct yang_type *)actual)->type->info.lref.path = lydict_insert_zc(trg->ctx, s);
                   s = NULL;
                 }
               }
  | require_instance_stmt  { /*instance_identifier_specification */
                             if (read_all) {
                               ((struct yang_type *)actual)->type->base = LY_TYPE_INST;
                             }
                           }
  | bits_specification 
  ;


decimal_string_restrictions: %empty  { if (read_all) {
                                         if (size_arrays->node[size_arrays->next].refine && size_arrays->node[size_arrays->next].augment) {
                                           LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", ((struct yang_type *)actual)->type->parent->name);
                                           YYERROR;
                                         }
                                         if (size_arrays->node[size_arrays->next].refine) {
                                           ((struct yang_type *)actual)->type->info.str.patterns = calloc(size_arrays->node[size_arrays->next].refine, sizeof(struct lys_restr));
                                           if (!((struct yang_type *)actual)->type->info.str.patterns) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           ((struct yang_type *)actual)->type->base = LY_TYPE_STRING;
                                         }
                                         /* augment - count of union*/
                                         if (size_arrays->node[size_arrays->next].augment) {
                                           ((struct yang_type *)actual)->type->info.uni.types = calloc(size_arrays->node[size_arrays->next].augment, sizeof(struct lys_type));
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
                                         $$ = size_arrays->size-1;
                                       }
                                     }
  |  decimal_string_restrictions length_stmt
  |  decimal_string_restrictions pattern_stmt { if (!read_all) {
                                                  size_arrays->node[$1].refine++; /* count of pattern*/
                                                }
                                              }
  |  decimal_string_restrictions fraction_digits_stmt
  |  decimal_string_restrictions range_stmt stmtsep
  |  decimal_string_restrictions union_spec type_stmt stmtsep { if (read_all) {
                                                          actual = $2;
                                                        } else {
                                                          size_arrays->node[$1].augment++; /* count of union*/
                                                        }
                                                      }
  ;

  union_spec: %empty { if (read_all) {
                         struct yang_type *typ;
                         struct lys_type *type;

                         typ = (struct yang_type *)actual;
                         $$ = actual;
                         type = &typ->type->info.uni.types[typ->type->info.uni.count++];
                         type->parent = typ->type->parent;
                         actual = type;
                         actual_type = UNION_KEYWORD;
                       }
                     }

fraction_digits_stmt: FRACTION_DIGITS_KEYWORD sep fraction_digits_arg_str
                      stmtend { if (read_all && yang_read_fraction(actual, $3, yylineno)) {
                                  YYERROR;
                                }
                              }

fraction_digits_arg_str: positive_integer_value optsep { $$ = $1; }
  | string_1 { if (read_all) {
                 int errno = 0;
                 char *endptr = NULL;
                 unsigned long val;

                 val = strtoul(s, &endptr, 10);
                 if (*endptr || s[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                   LOGVAL(LYE_INARG, yylineno, LY_VLOG_NONE, NULL, s, "fraction-digits");
                   free(s);
                   s = NULL;
                   YYERROR;
                 }
                 $$ = (uint32_t) val;
                 free(s);
                 s =NULL;
               }
             }
  ;

length_stmt: LENGTH_KEYWORD sep length_arg_str length_end stmtsep { actual = $3;
                                                                    actual_type = TYPE_KEYWORD;
                                                                  }

length_arg_str: string { if (read_all) {
                           $$ = actual;
                           if (!(actual = yang_read_length(trg, actual, s, yylineno))) {
                             YYERROR;
                           }
                           actual_type = LENGTH_KEYWORD;
                           s = NULL;
                         }
                       }

length_end: ';' 
  |  '{' stmtsep
         message_opt_stmt
      '}'
  ;    

message_opt_stmt: %empty
  |  message_opt_stmt error_message_stmt { if (read_all && yang_read_message(trg,actual,s,actual_type, ERROR_MESSAGE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  message_opt_stmt error_app_tag_stmt { if (yang_read_message(trg,actual,s,actual_type, ERROR_APP_TAG_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  message_opt_stmt description_stmt { if (read_all && yang_read_description(trg,actual,s,actual_type,yylineno)) {YYERROR;} s = NULL; }
  |  message_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,actual,s,actual_type,yylineno)) {YYERROR;} s = NULL; }
  ;

pattern_stmt: PATTERN_KEYWORD sep pattern_arg_str pattern_end stmtsep { actual = $3;
                                                                        actual_type = TYPE_KEYWORD;
                                                                      }

pattern_arg_str: string { if (read_all) {
                            $$ = actual;
                            if (!(actual = yang_read_pattern(trg, actual, s, yylineno))) {
                              YYERROR;
                            }
                            actual_type = PATTERN_KEYWORD;
                            s = NULL;
                          }
                        }

pattern_end: ';'
  |  '{' stmtsep
         message_opt_stmt
     '}'
  ;  

enum_specification: { if (read_all) {
                        ((struct yang_type *)actual)->type->info.enums.enm = calloc(size_arrays->node[size_arrays->next++].refine, sizeof(struct lys_type_enum));
                        if (!((struct yang_type *)actual)->type->info.enums.enm) {
                          LOGMEM;
                          YYERROR;
                        }
                        ((struct yang_type *)actual)->type->base = LY_TYPE_ENUM;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYERROR;
                        }
                      }
                    } enum_stmt stmtsep enum_stmts;

enum_stmts: %empty
  | enum_stmts enum_stmt stmtsep;


enum_stmt: ENUM_KEYWORD sep enum_arg_str enum_end
           { if (read_all) {
               if (yang_check_enum($3, actual, &cnt_val, actual_type, yylineno)) {
                 YYERROR;
               }
               actual = $3;
               actual_type = TYPE_KEYWORD;
             } else {
               size_arrays->node[size_arrays->size-1].refine++; /* count of enum*/
             }
           }

enum_arg_str: string { if (read_all) {
                         $$ = actual;
                         if (!(actual = yang_read_enum(trg, actual, s, yylineno))) {
                           YYERROR;
                         }
                         s = NULL;
                         actual_type = 0;
                       }
                     }

enum_end: ';'
  |  '{' stmtsep
         enum_opt_stmt
     '}'
  ;

enum_opt_stmt: %empty 
  |  enum_opt_stmt value_stmt { /* actual_type - it is used to check value of enum statement*/
                                if (read_all) {
                                  if (actual_type) {
                                    LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "value", "enum");
                                    YYERROR;
                                  }
                                  actual_type = 1;
                                }
                              }
  |  enum_opt_stmt status_stmt { if (read_all && yang_read_status(actual,$2,ENUM_KEYWORD,yylineno)) {YYERROR;} }
  |  enum_opt_stmt description_stmt { if (read_all && yang_read_description(trg,actual,s,ENUM_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  enum_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,actual,s,ENUM_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

value_stmt: VALUE_KEYWORD sep integer_value_arg_str
            stmtend { if (read_all) {
                        ((struct lys_type_enum *)actual)->value = $3;

                        /* keep the highest enum value for automatic increment */
                        if ($3 > cnt_val) {
                          cnt_val = $3;
                        }
                        cnt_val++;
                      }
                    }

integer_value_arg_str: integer_value optsep { $$ = $1; }
  |  string_1 { if (read_all) {
                  /* convert it to int32_t */
                  int64_t val;
                  char *endptr;

                  val = strtoll(s, &endptr, 10);
                  if (val < INT32_MIN || val > INT32_MAX || *endptr) {
                      LOGVAL(LYE_INARG, yylineno, LY_VLOG_NONE, NULL, s, "value");
                      free(s);
                      YYERROR;
                  }
                  free(s);
                  s = NULL;
                  $$ = (int32_t) val;
               }
             }
  ;

range_stmt: RANGE_KEYWORD sep range_arg_str range_end { actual = $3;
                                                        actual_type = RANGE_KEYWORD;
                                                      }


range_end: ';'
  |  '{' stmtsep
         message_opt_stmt
      '}'
   ;

path_stmt: PATH_KEYWORD sep path_arg_str stmtend; 

require_instance_stmt: REQUIRE_INSTANCE_KEYWORD sep require_instance_arg_str stmtend;

require_instance_arg_str: TRUE_KEYWORD optsep { if (read_all) {
                                                  ((struct yang_type *)actual)->type->info.inst.req = 1;
                                                }
                                              }
  |  FALSE_KEYWORD optsep { if (read_all) {
                              ((struct yang_type *)actual)->type->info.inst.req = -1;
                            }
                          }
  |  string_1 { if (read_all) {
                  if (!strcmp(s,"true")) {
                    ((struct yang_type *)actual)->type->info.inst.req = 1;
                  } else if (!strcmp(s,"false")) {
                    ((struct yang_type *)actual)->type->info.inst.req = -1;
                  } else {
                    LOGVAL(LYE_INARG, yylineno, LY_VLOG_NONE, NULL, s, "require-instance");
                    free(s);
                    YYERROR;
                  }
                  free(s);
                }
              }
  ;

bits_specification: { if (read_all) {
                        ((struct yang_type *)actual)->type->info.bits.bit = calloc(size_arrays->node[size_arrays->next++].refine, sizeof(struct lys_type_bit));
                        if (!((struct yang_type *)actual)->type->info.bits.bit) {
                          LOGMEM;
                          YYERROR;
                        }
                        ((struct yang_type *)actual)->type->base = LY_TYPE_BITS;
                        cnt_val = 0;
                      } else {
                        if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                          LOGMEM;
                          YYERROR;
                        }
                      }
                    } bit_stmt bit_stmts

bit_stmts: %empty 
  | bit_stmts bit_stmt;

bit_stmt: BIT_KEYWORD sep bit_arg_str bit_end
          stmtsep { if (read_all) {
                      if (yang_check_bit($3, actual, &cnt_val, actual_type, yylineno)) {
                        YYERROR;
                      }
                      actual = $3;
                    } else {
                      size_arrays->node[size_arrays->size-1].refine++; /* count of bit*/
                    }
                  }

bit_arg_str: identifier_arg_str { if (read_all) {
                                    $$ = actual;
                                    if (!(actual = yang_read_bit(trg, actual, s, yylineno))) {
                                      YYERROR;
                                    }
                                    s = NULL;
                                    actual_type = 0;
                                  }
                                }

bit_end: ';'
  |  '{' stmtsep
         bit_opt_stmt
     '}'
  ;

bit_opt_stmt: %empty 
  |  bit_opt_stmt position_stmt { /* actual_type - it is used to check position of bit statement*/
                                  if (read_all) {
                                    if (actual_type) {
                                      LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "position", "bit");
                                      YYERROR;
                                    }
                                    actual_type = 1;
                                  }
                                }
  |  bit_opt_stmt status_stmt { if (read_all && yang_read_status(actual,$2,BIT_KEYWORD,yylineno)) {YYERROR;} }
  |  bit_opt_stmt description_stmt { if (read_all && yang_read_description(trg,actual,s,BIT_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  bit_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,actual,s,BIT_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

position_stmt: POSITION_KEYWORD sep position_value_arg_str
               stmtend { if (read_all) {
                           ((struct lys_type_bit *)actual)->pos = $3;

                           /* keep the highest position value for automatic increment */
                           if ($3 > cnt_val) {
                             cnt_val = $3;
                           }
                           cnt_val++;
                         }
                       }

position_value_arg_str: non_negative_integer_value optsep { $$ = $1; }
  |  string_1 { /* convert it to uint32_t */
                unsigned long val;
                char *endptr;

                val = strtoul(s, &endptr, 10);
                if (val > UINT32_MAX || s[0] == '-' || *endptr) {
                    LOGVAL(LYE_INARG, yylineno, LY_VLOG_NONE, NULL, s, "position");
                    free(s);
                    YYERROR;
                }
                free(s);
                s = NULL;
                $$ = (uint32_t) val;
              }

error_message_stmt: ERROR_MESSAGE_KEYWORD sep string stmtend;

error_app_tag_stmt: ERROR_APP_TAG_KEYWORD sep string stmtend;

units_stmt: UNITS_KEYWORD sep string stmtend;

default_stmt: DEFAULT_KEYWORD sep string stmtend;

grouping_stmt: GROUPING_KEYWORD sep identifier_arg_str { if (read_all) {
                                                           if (!(actual = yang_read_node(trg,actual,s,LYS_GROUPING,sizeof(struct lys_node_grp)))) {YYERROR;}
                                                           s=NULL;
                                                         } else {
                                                           if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                             LOGMEM;
                                                             YYERROR;
                                                           }
                                                         }
                                                       }
               grouping_end;

grouping_end: ';' { if (read_all) { size_arrays->next++; } }
  |  '{' stmtsep
         grouping_opt_stmt
     '}'
  ;

grouping_opt_stmt: %empty { if (read_all) {
                               $$.grouping = actual;
                               actual_type = GROUPING_KEYWORD;
                               $$.grouping->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *$$.grouping->tpdf);
                               if (!$$.grouping->tpdf) {
                                 LOGMEM;
                                 YYERROR;
                               }
                               size_arrays->next++;
                             } else {
                               $$.index = size_arrays->size-1;
                             }
                           }
  |  grouping_opt_stmt status_stmt { if (read_all && yang_read_status($1.grouping,$2,GROUPING_KEYWORD,yylineno)) {YYERROR;} }
  |  grouping_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.grouping,s,GROUPING_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  grouping_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.grouping,s,GROUPING_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  grouping_opt_stmt grouping_stmt stmtsep { actual = $1.grouping; actual_type = GROUPING_KEYWORD; }
  |  grouping_opt_stmt typedef_stmt stmtsep { if (read_all) {
                                                actual = $1.grouping;
                                                actual_type = GROUPING_KEYWORD;
                                              } else {
                                                size_arrays->node[$1.index].tpdf++;
                                              }
                                            }
  |  grouping_opt_stmt data_def_stmt stmtsep { actual = $1.grouping; actual_type = GROUPING_KEYWORD; }
  ;

data_def_stmt: container_stmt
  |  leaf_stmt
  |  leaf_list_stmt 
  |  list_stmt 
  |  choice_stmt 
  |  anyxml_stmt
  |  uses_stmt
  ;

container_stmt: CONTAINER_KEYWORD sep identifier_arg_str { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_CONTAINER,sizeof(struct lys_node_container)))) {YYERROR;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           } else {
                                                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                               LOGMEM;
                                                               YYERROR;
                                                             }
                                                           }
                                                         }
                container_end ;

container_end: ';' { if (read_all) { size_arrays->next++; } }
  |  '{' stmtsep
         container_opt_stmt
      '}'
  ;  

container_opt_stmt: %empty { if (read_all) {
                               $$.container = actual;
                               actual_type = CONTAINER_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 $$.container->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.container->features);
                                 if (!$$.container->features) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 $$.container->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *$$.container->must);
                                 if (!$$.container->must) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].tpdf) {
                                 $$.container->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *$$.container->tpdf);
                                 if (!$$.container->tpdf) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               size_arrays->next++;
                             } else {
                               $$.index = size_arrays->size-1;
                             }
                           }
  |  container_opt_stmt when_stmt { actual = $1.container; actual_type = CONTAINER_KEYWORD; }
     stmtsep
  |  container_opt_stmt if_feature_stmt { if (read_all) {
                                            if (yang_read_if_feature(trg,$1.container,s,unres,CONTAINER_KEYWORD,yylineno)) {YYERROR;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[$1.index].if_features++;
                                          }
                                        }
  |  container_opt_stmt must_stmt { if (read_all) {
                                      actual = $1.container;
                                      actual_type = CONTAINER_KEYWORD;
                                    } else {
                                      size_arrays->node[$1.index].must++;
                                    }
                                  }
     stmtsep
  |  container_opt_stmt presence_stmt { if (read_all && yang_read_presence(trg,$1.container,s,yylineno)) {YYERROR;} s=NULL; }
  |  container_opt_stmt config_stmt { if (read_all && yang_read_config($1.container,$2,CONTAINER_KEYWORD,yylineno)) {YYERROR;} }
  |  container_opt_stmt status_stmt { if (read_all && yang_read_status($1.container,$2,CONTAINER_KEYWORD,yylineno)) {YYERROR;} }
  |  container_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.container,s,CONTAINER_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  container_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.container,s,CONTAINER_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  container_opt_stmt grouping_stmt { actual = $1.container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }
     stmtsep
  |  container_opt_stmt typedef_stmt { if (read_all) {
                                                 actual = $1.container;
                                                 actual_type = CONTAINER_KEYWORD;
                                               } else {
                                                 size_arrays->node[$1.index].tpdf++;
                                               }
                                             }
     stmtsep
  |  container_opt_stmt data_def_stmt { actual = $1.container;
                                        actual_type = CONTAINER_KEYWORD;
                                        data_node = actual;
                                      }
     stmtsep
  ;

leaf_stmt: LEAF_KEYWORD sep identifier_arg_str { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LEAF,sizeof(struct lys_node_leaf)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 } else {
                                                   if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                     LOGMEM;
                                                     YYERROR;
                                                   }
                                                 }
                                               }
           '{' stmtsep
               leaf_opt_stmt  { if (read_all) {
                                  if (!($7.leaf.flag & LYS_TYPE_DEF)) {
                                    LOGVAL(LYE_SPEC, yylineno, LY_VLOG_LYS, $7.leaf.ptr_leaf, "type statement missing.");
                                    YYERROR;
                                  } else {
                                      if (unres_schema_add_node(trg, unres, &$7.leaf.ptr_leaf->type, UNRES_TYPE_DER,(struct lys_node *) $7.leaf.ptr_leaf, $7.leaf.line)) {
                                        $7.leaf.ptr_leaf->type.der = NULL;
                                        YYERROR;
                                      }
                                  }
                                  if ($7.leaf.ptr_leaf->dflt && unres_schema_add_str(trg, unres, &$7.leaf.ptr_leaf->type, UNRES_TYPE_DFLT, $7.leaf.ptr_leaf->dflt, tmp_line) == -1) {
                                    YYERROR;
                                  }
                                }
                              }
           '}' ;

leaf_opt_stmt: %empty { if (read_all) {
                          $$.leaf.ptr_leaf = actual;
                          $$.leaf.flag = 0;
                          actual_type = LEAF_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            $$.leaf.ptr_leaf->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.leaf.ptr_leaf->features);
                            if (!$$.leaf.ptr_leaf->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].must) {
                            $$.leaf.ptr_leaf->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *$$.leaf.ptr_leaf->must);
                            if (!$$.leaf.ptr_leaf->must) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          size_arrays->next++;
                        } else {
                          $$.index = size_arrays->size-1;
                        }
                      }
  |  leaf_opt_stmt when_stmt { actual = $1.leaf.ptr_leaf; actual_type = LEAF_KEYWORD; }
     stmtsep
  |  leaf_opt_stmt if_feature_stmt { if (read_all) {
                                       if (yang_read_if_feature(trg,$1.leaf.ptr_leaf,s,unres,LEAF_KEYWORD,yylineno)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[$1.index].if_features++;
                                     }
                                   }
  |  leaf_opt_stmt { if (read_all && ($1.leaf.flag & LYS_TYPE_DEF)) {
                       LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.leaf.ptr_leaf, "type", "leaf");
                       YYERROR;
                     }
                   }
     type_stmt { if (read_all) {
                   actual = $1.leaf.ptr_leaf;
                   actual_type = LEAF_KEYWORD;
                   $1.leaf.flag |= LYS_TYPE_DEF;
                   $1.leaf.line = yylineno;
                 }
               }
     stmtsep { $$ = $1;}
  |  leaf_opt_stmt units_stmt { if (read_all && yang_read_units(trg,$1.leaf.ptr_leaf,s,LEAF_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  leaf_opt_stmt must_stmt { if (read_all) {
                                 actual = $1.leaf.ptr_leaf;
                                 actual_type = LEAF_KEYWORD;
                               } else {
                                 size_arrays->node[$1.index].must++;
                               }
                             }
     stmtsep
  |  leaf_opt_stmt default_stmt { if (read_all && yang_read_default(trg,$1.leaf.ptr_leaf,s,LEAF_KEYWORD,yylineno)) {YYERROR;}
                                  s = NULL;
                                  tmp_line = yylineno;
                                }
  |  leaf_opt_stmt config_stmt { if (read_all && yang_read_config($1.leaf.ptr_leaf,$2,LEAF_KEYWORD,yylineno)) {YYERROR;} }
  |  leaf_opt_stmt mandatory_stmt { if (read_all && yang_read_mandatory($1.leaf.ptr_leaf,$2,LEAF_KEYWORD,yylineno)) {YYERROR;} }
  |  leaf_opt_stmt status_stmt { if (read_all && yang_read_status($1.leaf.ptr_leaf,$2,LEAF_KEYWORD,yylineno)) {YYERROR;} }
  |  leaf_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.leaf.ptr_leaf,s,LEAF_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  leaf_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.leaf.ptr_leaf,s,LEAF_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

leaf_list_stmt: LEAF_LIST_KEYWORD sep identifier_arg_str { if (read_all) {
                                                             if (!(actual = yang_read_node(trg,actual,s,LYS_LEAFLIST,sizeof(struct lys_node_leaflist)))) {YYERROR;}
                                                             data_node = actual;
                                                             s=NULL;
                                                           } else {
                                                             if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                               LOGMEM;
                                                               YYERROR;
                                                             }
                                                           }
                                                         }
                '{' stmtsep
                    leaf_list_opt_stmt { if (read_all) {
                                           if ($7.leaflist.ptr_leaflist->flags & LYS_CONFIG_R) {
                                             /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                              * ignore oredering MASK - 0x7F
                                              */
                                             $7.leaflist.ptr_leaflist->flags &= 0x7F;
                                           }
                                           if ($7.leaflist.ptr_leaflist->max && $7.leaflist.ptr_leaflist->min > $7.leaflist.ptr_leaflist->max) {
                                             LOGVAL(LYE_SPEC, yylineno, LY_VLOG_LYS, $7.leaflist.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                                             YYERROR;
                                           }
                                           if (!($7.leaflist.flag & LYS_TYPE_DEF)) {
                                             LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_LYS, $7.leaflist.ptr_leaflist, "type", "leaflist");
                                             YYERROR;
                                           } else {
                                             if (unres_schema_add_node(trg, unres, &$7.leaflist.ptr_leaflist->type, UNRES_TYPE_DER,
                                                                      (struct lys_node *) $7.leaflist.ptr_leaflist, $7.leaflist.line)) {
                                               YYERROR;
                                             }
                                           }
                                         }
                                       }
                '}' ;

leaf_list_opt_stmt: %empty { if (read_all) {
                               $$.leaflist.ptr_leaflist = actual;
                               $$.leaflist.flag = 0;
                               actual_type = LEAF_LIST_KEYWORD;
                               if (size_arrays->node[size_arrays->next].if_features) {
                                 $$.leaflist.ptr_leaflist->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.leaflist.ptr_leaflist->features);
                                 if (!$$.leaflist.ptr_leaflist->features) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               if (size_arrays->node[size_arrays->next].must) {
                                 $$.leaflist.ptr_leaflist->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *$$.leaflist.ptr_leaflist->must);
                                 if (!$$.leaflist.ptr_leaflist->must) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               size_arrays->next++;
                             } else {
                               $$.index = size_arrays->size-1;
                             }
                           }
  |  leaf_list_opt_stmt when_stmt { actual = $1.leaflist.ptr_leaflist; actual_type = LEAF_LIST_KEYWORD; }
     stmtsep
  |  leaf_list_opt_stmt if_feature_stmt { if (read_all) {
                                            if (yang_read_if_feature(trg,$1.leaflist.ptr_leaflist,s,unres,LEAF_LIST_KEYWORD,yylineno)) {YYERROR;}
                                            s=NULL;
                                          } else {
                                            size_arrays->node[$1.index].if_features++;
                                          }
                                        }
  |  leaf_list_opt_stmt { if (read_all && ($1.leaflist.flag & LYS_TYPE_DEF)) {
                            LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.leaflist.ptr_leaflist, "type", "leaflist");
                            YYERROR;
                          }
                        }
     type_stmt { if (read_all) {
                   actual = $1.leaflist.ptr_leaflist;
                   actual_type = LEAF_LIST_KEYWORD;
                   $1.leaflist.flag |= LYS_TYPE_DEF;
                   $1.leaflist.line = yylineno;
                 }
               }
     stmtsep { $$ = $1; }
  |  leaf_list_opt_stmt units_stmt { if (read_all && yang_read_units(trg,$1.leaflist.ptr_leaflist,s,LEAF_LIST_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  leaf_list_opt_stmt must_stmt { if (read_all) {
                                      actual = $1.leaflist.ptr_leaflist;
                                      actual_type = LEAF_LIST_KEYWORD;
                                    } else {
                                      size_arrays->node[$1.index].must++;
                                    }
                                  }
     stmtsep
  |  leaf_list_opt_stmt config_stmt { if (read_all && yang_read_config($1.leaflist.ptr_leaflist,$2,LEAF_LIST_KEYWORD,yylineno)) {YYERROR;} }
  |  leaf_list_opt_stmt min_elements_stmt { if (read_all) {
                                              if ($1.leaflist.flag & LYS_MIN_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.leaflist.ptr_leaflist, "min-elements", "leaflist");
                                                YYERROR;
                                              }
                                              $1.leaflist.ptr_leaflist->min = $2;
                                              $1.leaflist.flag |= LYS_MIN_ELEMENTS;
                                              $$ = $1;
                                            }
                                          }
  |  leaf_list_opt_stmt max_elements_stmt { if (read_all) {
                                              if ($1.leaflist.flag & LYS_MAX_ELEMENTS) {
                                                LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.leaflist.ptr_leaflist, "max-elements", "leaflist");
                                                YYERROR;
                                              }
                                              $1.leaflist.ptr_leaflist->max = $2;
                                              $1.leaflist.flag |= LYS_MAX_ELEMENTS;
                                              $$ = $1;
                                            }
                                          }
  |  leaf_list_opt_stmt ordered_by_stmt { if (read_all) {
                                            if ($1.leaflist.flag & LYS_ORDERED_MASK) {
                                              LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.leaflist.ptr_leaflist, "ordered by", "leaflist");
                                              YYERROR;
                                            }
                                            if ($2 & LYS_USERORDERED) {
                                              $1.leaflist.ptr_leaflist->flags |= LYS_USERORDERED;
                                            }
                                            $1.leaflist.flag |= $2;
                                            $$ = $1;
                                          }
                                        }
  |  leaf_list_opt_stmt status_stmt { if (read_all && yang_read_status($1.leaflist.ptr_leaflist,$2,LEAF_LIST_KEYWORD,yylineno)) {YYERROR;} }
  |  leaf_list_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.leaflist.ptr_leaflist,s,LEAF_LIST_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  leaf_list_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.leaflist.ptr_leaflist,s,LEAF_LIST_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

list_stmt: LIST_KEYWORD sep identifier_arg_str { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_LIST,sizeof(struct lys_node_list)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 } else {
                                                   if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                     LOGMEM;
                                                     YYERROR;
                                                   }
                                                 }
                                               }
           '{' stmtsep
               list_opt_stmt { if (read_all) {
                                 if ($7.list.ptr_list->flags & LYS_CONFIG_R) {
                                   /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                                    * ignore oredering MASK - 0x7F
                                    */
                                   $7.list.ptr_list->flags &= 0x7F;
                                 }
                                 if ($7.list.ptr_list->max && $7.list.ptr_list->min > $7.list.ptr_list->max) {
                                   LOGVAL(LYE_SPEC, yylineno, LY_VLOG_LYS, $7.list.ptr_list, "\"min-elements\" is bigger than \"max-elements\".");
                                   YYERROR;
                                 }
                                 if (($7.list.ptr_list->flags & LYS_CONFIG_W) && !$7.list.ptr_list->keys) {
                                   LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_LYS, $7.list.ptr_list, "key", "list");
                                   YYERROR;
                                 }
                                 if ($7.list.ptr_list->keys && yang_read_key(trg, $7.list.ptr_list, unres, $7.list.line)) {
                                   YYERROR;
                                 }
                                 if (!($7.list.flag & LYS_DATADEF)) {
                                   LOGVAL(LYE_SPEC, yylineno, LY_VLOG_LYS, $7.list.ptr_list, "data-def statement missing.");
                                   YYERROR;
                                 }
                                 if (yang_read_unique(trg, $7.list.ptr_list, unres)) {
                                   YYERROR;
                                 }
                               }
                             }
            '}' ;

list_opt_stmt: %empty { if (read_all) {
                          $$.list.ptr_list = actual;
                          $$.list.flag = 0;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            $$.list.ptr_list->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.list.ptr_list->features);
                            if (!$$.list.ptr_list->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].must) {
                            $$.list.ptr_list->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *$$.list.ptr_list->must);
                            if (!$$.list.ptr_list->must) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].tpdf) {
                            $$.list.ptr_list->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *$$.list.ptr_list->tpdf);
                            if (!$$.list.ptr_list->tpdf) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].unique) {
                            $$.list.ptr_list->unique = calloc(size_arrays->node[size_arrays->next].unique, sizeof *$$.list.ptr_list->unique);
                            if (!$$.list.ptr_list->unique) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          size_arrays->next++;
                        } else {
                          $$.index = size_arrays->size-1;
                        }
                      }
  |  list_opt_stmt when_stmt { actual = $1.list.ptr_list; actual_type = LIST_KEYWORD; }
     stmtsep
  |  list_opt_stmt if_feature_stmt { if (read_all) {
                                       if (yang_read_if_feature(trg,$1.list.ptr_list,s,unres,LIST_KEYWORD,yylineno)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[$1.index].if_features++;
                                     }
                                   }
  |  list_opt_stmt must_stmt { if (read_all) {
                                 actual = $1.list.ptr_list;
                                 actual_type = LIST_KEYWORD;
                               } else {
                                 size_arrays->node[$1.index].must++;
                               }
                             }
     stmtsep
  |  list_opt_stmt key_stmt { if (read_all) {
                                if ($1.list.ptr_list->keys) {
                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.list.ptr_list, "key", "list");
                                  YYERROR;
                                }
                                $1.list.ptr_list->keys = (struct lys_node_leaf **)s;
                                $1.list.line = yylineno;
                                $$ = $1;
                                s=NULL;
                              }
                            }
  |  list_opt_stmt unique_stmt { if (read_all) {
                                   struct type_ident *ident;
                                   ident = malloc(sizeof(struct type_ident) + strlen(s) + 1);
                                   if (!ident) {
                                     LOGMEM;
                                     YYERROR;
                                   }
                                   ident->line = yylineno;
                                   memcpy(ident->s,s,strlen(s)+1);
                                   $1.list.ptr_list->unique[$1.list.ptr_list->unique_size++].expr = (const char **)ident;
                                   $$ = $1;
                                   free(s);
                                   s = NULL;
                                 } else {
                                   size_arrays->node[$1.index].unique++;
                                 }
                               }
  |  list_opt_stmt config_stmt { if (read_all && yang_read_config($1.list.ptr_list,$2,LIST_KEYWORD,yylineno)) {YYERROR;} }
  |  list_opt_stmt min_elements_stmt { if (read_all) {
                                         if ($1.list.flag & LYS_MIN_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.list.ptr_list, "min-elements", "list");
                                           YYERROR;
                                         }
                                         $1.list.ptr_list->min = $2;
                                         $1.list.flag |= LYS_MIN_ELEMENTS;
                                         $$ = $1;
                                       }
                                     }
  |  list_opt_stmt max_elements_stmt { if (read_all) {
                                         if ($1.list.flag & LYS_MAX_ELEMENTS) {
                                           LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.list.ptr_list, "max-elements", "list");
                                           YYERROR;
                                         }
                                         $1.list.ptr_list->max = $2;
                                         $1.list.flag |= LYS_MAX_ELEMENTS;
                                         $$ = $1;
                                       }
                                     }
  |  list_opt_stmt ordered_by_stmt { if (read_all) {
                                       if ($1.list.flag & LYS_ORDERED_MASK) {
                                         LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.list.ptr_list, "ordered by", "list");
                                         YYERROR;
                                       }
                                       if ($2 & LYS_USERORDERED) {
                                         $1.list.ptr_list->flags |= LYS_USERORDERED;
                                       }
                                       $1.list.flag |= $2;
                                       $$ = $1;
                                     }
                                   }
  |  list_opt_stmt status_stmt { if (read_all && yang_read_status($1.list.ptr_list,$2,LIST_KEYWORD,yylineno)) {YYERROR;} }
  |  list_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.list.ptr_list,s,LIST_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  list_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.list.ptr_list,s,LIST_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  list_opt_stmt typedef_stmt { if (read_all) {
                                            actual = $1.list.ptr_list;
                                            actual_type = LIST_KEYWORD;
                                          } else {
                                            size_arrays->node[$1.index].tpdf++;
                                          }
                                        }
     stmtsep
  |  list_opt_stmt grouping_stmt { actual = $1.list.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   data_node = actual;
                                 }
     stmtsep
  |  list_opt_stmt data_def_stmt { actual = $1.list.ptr_list;
                                   actual_type = LIST_KEYWORD;
                                   $1.list.flag |= LYS_DATADEF;
                                   data_node = actual;
                                 }
     stmtsep { $$ = $1; }
  ;

choice_stmt: CHOICE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_CHOICE,sizeof(struct lys_node_choice)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     } else {
                                                       if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                         LOGMEM;
                                                         YYERROR;
                                                       }
                                                     }
                                                   }
             choice_end;

choice_end: ';' { if (read_all) { size_arrays->next++; } }
  |  '{' stmtsep
         choice_opt_stmt  { if (read_all) {
                              if ($3.choice.s && ($3.choice.ptr_choice->flags & LYS_MAND_TRUE)) {
                                LOGVAL(LYE_SPEC, yylineno, LY_VLOG_LYS, $3.choice.ptr_choice, "The \"default\" statement MUST NOT be present on choices where \"mandatory\" is true.");
                                YYERROR;
                              }
                              /* link default with the case */
                              if ($3.choice.s) {
                                if (unres_schema_add_str(trg, unres, $3.choice.ptr_choice, UNRES_CHOICE_DFLT, $3.choice.s, yylineno) == -1) {
                                  YYERROR;
                                }
                                free($3.choice.s);
                              }
                            }
                          }
     '}' ;

choice_opt_stmt: %empty { if (read_all) {
                            $$.choice.ptr_choice = actual;
                            $$.choice.s = NULL;
                            actual_type = CHOICE_KEYWORD;
                            $$.choice.ptr_choice->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.choice.ptr_choice->features);
                            if (!$$.choice.ptr_choice->features) {
                              LOGMEM;
                              YYERROR;
                            }
                            size_arrays->next++;
                          } else {
                            $$.index = size_arrays->size-1;
                          }
                        }
  |  choice_opt_stmt when_stmt { actual = $1.choice.ptr_choice; actual_type = CHOICE_KEYWORD; }
     stmtsep { $$ = $1; }
  |  choice_opt_stmt if_feature_stmt { if (read_all) {
                                         if (yang_read_if_feature(trg,$1.choice.ptr_choice,s,unres,CHOICE_KEYWORD,yylineno)) {
                                           if ($1.choice.s) {
                                             free($1.choice.s);
                                           }
                                           YYERROR;
                                         }
                                         s=NULL;
                                         $$ = $1;
                                       } else {
                                         size_arrays->node[$1.index].if_features++;
                                       }
                                     }
  |  choice_opt_stmt default_stmt { if (read_all) {
                                      if ($1.choice.s) {
                                        LOGVAL(LYE_TOOMANY,yylineno,LY_VLOG_LYS,$1.choice.ptr_choice,"default","choice");
                                        free($1.choice.s);
                                        free(s);
                                        YYERROR;
                                      }
                                      $1.choice.s = s;
                                      s = NULL;
                                      $$ = $1;
                                    }
                                  }
  |  choice_opt_stmt config_stmt { if (read_all && yang_read_config($1.choice.ptr_choice,$2,CHOICE_KEYWORD,yylineno)) {
                                     if ($1.choice.s) {
                                       free($1.choice.s);
                                     }
                                     YYERROR;
                                     $$ = $1;
                                   }
                                 }
|  choice_opt_stmt mandatory_stmt { if (read_all && yang_read_mandatory($1.choice.ptr_choice,$2,CHOICE_KEYWORD,yylineno)) {
                                      if ($1.choice.s) {
                                        free($1.choice.s);
                                      }
                                      YYERROR;
                                      $$ = $1;
                                    }
                                  }
  |  choice_opt_stmt status_stmt { if (read_all && yang_read_status($1.choice.ptr_choice,$2,CHOICE_KEYWORD,yylineno)) {
                                     if ($1.choice.s) {
                                       free($1.choice.s);
                                       YYERROR;
                                     }
                                     $$ = $1;
                                   }
                                 }
  |  choice_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.choice.ptr_choice,s,CHOICE_KEYWORD,yylineno)) {
                                          if ($1.choice.s) {
                                            free($1.choice.s);
                                            YYERROR;
                                          }
                                          s = NULL;
                                          $$ = $1;
                                        }
                                      }
  |  choice_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.choice.ptr_choice,s,CHOICE_KEYWORD,yylineno)) {
                                        if ($1.choice.s) {
                                          free($1.choice.s);
                                          YYERROR;
                                        }
                                        s = NULL;
                                        $$ = $1;
                                      }
                                    }
  |  choice_opt_stmt short_case_case_stmt { actual = $1.choice.ptr_choice;
                                            actual_type = CHOICE_KEYWORD;
                                            data_node = actual;
                                            if (read_all && data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                              data_node = NULL;
                                            }
                                          }
     stmtsep { $$ = $1; }
  ;

short_case_case_stmt:  short_case_stmt
  |  case_stmt
  ;

short_case_stmt: container_stmt
  |  leaf_stmt
  |  leaf_list_stmt
  |  list_stmt
  |  anyxml_stmt
  ;

case_stmt: CASE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                   if (!(actual = yang_read_node(trg,actual,s,LYS_CASE,sizeof(struct lys_node_case)))) {YYERROR;}
                                                   data_node = actual;
                                                   s=NULL;
                                                 } else {
                                                   if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                     LOGMEM;
                                                     YYERROR;
                                                   }
                                                 }
                                               }
           case_end;

case_end: ';'
  |  '{' stmtsep
         case_opt_stmt
      '}' ;

case_opt_stmt: %empty { if (read_all) {
                          $$.cs = actual;
                          actual_type = CASE_KEYWORD;
                          $$.cs->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.cs->features);
                          if (!$$.cs->features) {
                            LOGMEM;
                            YYERROR;
                          }
                          size_arrays->next++;
                        } else {
                          $$.index = size_arrays->size-1;
                        }
                      }
  |  case_opt_stmt when_stmt { actual = $1.cs; actual_type = CASE_KEYWORD; }
     stmtsep
  |  case_opt_stmt if_feature_stmt { if (read_all) {
                                       if (yang_read_if_feature(trg,$1.cs,s,unres,CASE_KEYWORD,yylineno)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[$1.index].if_features++;
                                     }
                                   }
  |  case_opt_stmt status_stmt { if (read_all && yang_read_status($1.cs,$2,CASE_KEYWORD,yylineno)) {YYERROR;} }
  |  case_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.cs,s,CASE_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  case_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.cs,s,CASE_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  case_opt_stmt data_def_stmt { actual = $1.cs;
                                   actual_type = CASE_KEYWORD;
                                   data_node = actual;
                                 }
     stmtsep
  ;

anyxml_stmt: ANYXML_KEYWORD sep identifier_arg_str { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_ANYXML,sizeof(struct lys_node_anyxml)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     } else {
                                                       if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                         LOGMEM;
                                                         YYERROR;
                                                       }
                                                     }
                                                   }
             anyxml_end;

anyxml_end: ';' { if (read_all) { size_arrays->next++; } }
  |  '{' stmtsep
         anyxml_opt_stmt
     '}' ;

anyxml_opt_stmt: %empty { if (read_all) {
                            $$.anyxml = actual;
                            actual_type = ANYXML_KEYWORD;
                            $$.anyxml->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.anyxml->features);
                            $$.anyxml->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *$$.anyxml->must);
                            if (!$$.anyxml->features || !$$.anyxml->must) {
                              LOGMEM;
                              YYERROR;
                            }
                            size_arrays->next++;
                          } else {
                            $$.index = size_arrays->size-1;
                          }
                        }
  |  anyxml_opt_stmt when_stmt { actual = $1.anyxml; actual_type = ANYXML_KEYWORD; }
     stmtsep
  |  anyxml_opt_stmt if_feature_stmt { if (read_all) {
                                         if (yang_read_if_feature(trg,$1.anyxml,s,unres,ANYXML_KEYWORD,yylineno)) {YYERROR;}
                                         s=NULL;
                                       } else {
                                         size_arrays->node[$1.index].if_features++;
                                       }
                                     }
  |  anyxml_opt_stmt must_stmt { if (read_all) {
                                   actual = $1.anyxml;
                                   actual_type = ANYXML_KEYWORD;
                                 } else {
                                   size_arrays->node[$1.index].must++;
                                 }
                               }
     stmtsep
  |  anyxml_opt_stmt config_stmt { if (read_all && yang_read_config($1.anyxml,$2,ANYXML_KEYWORD,yylineno)) {YYERROR;} }
  |  anyxml_opt_stmt mandatory_stmt { if (read_all && yang_read_mandatory($1.anyxml,$2,ANYXML_KEYWORD,yylineno)) {YYERROR;} }
  |  anyxml_opt_stmt status_stmt { if (read_all && yang_read_status($1.anyxml,$2,ANYXML_KEYWORD,yylineno)) {YYERROR;} }
  |  anyxml_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.anyxml,s,ANYXML_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  anyxml_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.anyxml,s,ANYXML_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

uses_stmt: USES_KEYWORD sep identifier_ref_arg_str { if (read_all) {
                                                       if (!(actual = yang_read_node(trg,actual,s,LYS_USES,sizeof(struct lys_node_uses)))) {YYERROR;}
                                                       data_node = actual;
                                                       if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                                         data_node = NULL;
                                                       }
                                                       s=NULL;
                                                     } else {
                                                       if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                         LOGMEM;
                                                         YYERROR;
                                                       }
                                                     }
                                                   }
           uses_end { if (read_all) {
                        if (unres_schema_add_node(trg, unres, actual, UNRES_USES, NULL, yylineno) == -1) {
                          YYERROR;
                        }
                      }
                    }

uses_end: ';' { if (read_all) { size_arrays->next++; } }
  |  '{' stmtsep
         uses_opt_stmt
     '}' ;

uses_opt_stmt: %empty { if (read_all) {
                          $$.uses = actual;
                          actual_type = USES_KEYWORD;
                          if (size_arrays->node[size_arrays->next].if_features) {
                            $$.uses->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.uses->features);
                            if (!$$.uses->features) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].refine) {
                            $$.uses->refine = calloc(size_arrays->node[size_arrays->next].refine, sizeof *$$.uses->refine);
                            if (!$$.uses->refine) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          if (size_arrays->node[size_arrays->next].augment) {
                            $$.uses->augment = calloc(size_arrays->node[size_arrays->next].augment, sizeof *$$.uses->augment);
                            if (!$$.uses->augment) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                          size_arrays->next++;
                        } else {
                          $$.index = size_arrays->size-1;
                        }
                      }
  |  uses_opt_stmt when_stmt { actual = $1.uses; actual_type = USES_KEYWORD; }
     stmtsep
  |  uses_opt_stmt if_feature_stmt { if (read_all) {
                                       if (yang_read_if_feature(trg,$1.uses,s,unres,USES_KEYWORD,yylineno)) {YYERROR;}
                                       s=NULL;
                                     } else {
                                       size_arrays->node[$1.index].if_features++;
                                     }
                                   }
  |  uses_opt_stmt status_stmt { if (read_all && yang_read_status($1.uses,$2,USES_KEYWORD,yylineno)) {YYERROR;} }
  |  uses_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.uses,s,USES_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  uses_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.uses,s,USES_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  uses_opt_stmt refine_stmt { if (read_all) {
                                   actual = $1.uses;
                                   actual_type = USES_KEYWORD;
                                 } else {
                                   size_arrays->node[$1.index].refine++;
                                 }
                               }
     stmtsep
  |  uses_opt_stmt uses_augment_stmt { if (read_all) {
                                         actual = $1.uses;
                                         actual_type = USES_KEYWORD;
                                         data_node = actual;
                                         if (data_node->parent && (data_node->parent->nodetype == LYS_GROUPING)) {
                                           data_node = NULL;
                                         }
                                       } else {
                                         size_arrays->node[$1.index].augment++;
                                       }
                                     }
     stmtsep
  ;

refine_stmt: REFINE_KEYWORD sep refine_arg_str { if (read_all) {
                                                   if (!(actual = yang_read_refine(trg, actual, s, yylineno))) {
                                                     YYERROR;
                                                   }
                                                   s = NULL;
                                                 } else {
                                                   if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                     LOGMEM;
                                                     YYERROR;
                                                   }
                                                 }
                                               }
             refine_end;

refine_end: ';'
  |  '{' stmtsep
         refine_body_opt_stmts
     '}' ;


refine_arg_str: descendant_schema_nodeid optsep 
  | string_1
  ;

refine_body_opt_stmts: %empty { if (read_all) {
                                  $$.refine = actual;
                                  actual_type = REFINE_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].must) {
                                    $$.refine->must = calloc(size_arrays->node[size_arrays->next].must, sizeof *$$.refine->must);
                                    if (!$$.refine->must) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                    $$.refine->target_type = LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
                                  }
                                  size_arrays->next++;
                                } else {
                                  $$.index = size_arrays->size-1;
                                }
                              }
  |  refine_body_opt_stmts must_stmt stmtsep { if (read_all) {
                                         actual = $1.refine;
                                         actual_type = REFINE_KEYWORD;
                                       } else {
                                         size_arrays->node[$1.index].must++;
                                       }
                                     }
  |  refine_body_opt_stmts presence_stmt { if (read_all) {
                                             if ($1.refine->target_type) {
                                               if ($1.refine->target_type & LYS_CONTAINER) {
                                                 if ($1.refine->mod.presence) {
                                                   LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "presence", "refine");
                                                   free(s);
                                                   YYERROR;
                                                 }
                                                 $1.refine->target_type = LYS_CONTAINER;
                                                 $1.refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                               } else {
                                                 free(s);
                                                 LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "invalid combination of refine substatements");
                                                 YYERROR;
                                               }
                                             } else {
                                               $1.refine->target_type = LYS_CONTAINER;
                                               $1.refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                             }
                                             s = NULL;
                                             $$ = $1;
                                           }
                                         }
  |  refine_body_opt_stmts default_stmt { if (read_all) {
                                            if ($1.refine->target_type) {
                                              if ($1.refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
                                                $1.refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                                if ($1.refine->mod.dflt) {
                                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "default", "refine");
                                                  free(s);
                                                  YYERROR;
                                                }
                                                $1.refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                              } else {
                                                free(s);
                                                LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "invalid combination of refine substatements");
                                                YYERROR;
                                              }
                                            } else {
                                              $1.refine->target_type = LYS_LEAF | LYS_CHOICE;
                                              $1.refine->mod.dflt = lydict_insert_zc(trg->ctx, s);
                                            }
                                            s = NULL;
                                            $$ = $1;
                                          }
                                        }
  |  refine_body_opt_stmts config_stmt { if (read_all) {
                                           if ($1.refine->target_type) {
                                             if ($1.refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST)) {
                                               $1.refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST);
                                               if (yang_read_config($1.refine, $2, REFINE_KEYWORD, yylineno)) {
                                                 YYERROR;
                                               }
                                             } else {
                                               LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "invalid combination of refine substatements");
                                               YYERROR;
                                             }
                                           } else {
                                             $1.refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST;
                                             $1.refine->flags |= $2;
                                           }
                                           $$ = $1;
                                         }
                                       }
  |  refine_body_opt_stmts mandatory_stmt { if (read_all) {
                                              if ($1.refine->target_type) {
                                                if ($1.refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML)) {
                                                  $1.refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML);
                                                  if (yang_read_mandatory($1.refine, $2, REFINE_KEYWORD, yylineno)) {
                                                    YYERROR;
                                                  }
                                                } else {
                                                  LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "invalid combination of refine substatements");
                                                  YYERROR;
                                                }
                                              } else {
                                                $1.refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYXML;
                                                $1.refine->flags |= $2;
                                              }
                                              $$ = $1;
                                            }
                                          }
  |  refine_body_opt_stmts min_elements_stmt { if (read_all) {
                                                 if ($1.refine->target_type) {
                                                   if ($1.refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     $1.refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     /* magic - bit 3 in flags means min set */
                                                     if ($1.refine->flags & 0x04) {
                                                       LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                       YYERROR;
                                                     }
                                                     $1.refine->flags |= 0x04;
                                                     $1.refine->mod.list.min = $2;
                                                   } else {
                                                     LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "invalid combination of refine substatements");
                                                     YYERROR;
                                                   }
                                                 } else {
                                                   $1.refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   /* magic - bit 3 in flags means min set */
                                                   $1.refine->flags |= 0x04;
                                                   $1.refine->mod.list.min = $2;
                                                 }
                                                 $$ = $1;
                                               }
                                             }
  |  refine_body_opt_stmts max_elements_stmt { if (read_all) {
                                                 if ($1.refine->target_type) {
                                                   if ($1.refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                     $1.refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                     /* magic - bit 4 in flags means max set */
                                                     if ($1.refine->flags & 0x08) {
                                                       LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                       YYERROR;
                                                     }
                                                     $1.refine->flags |= 0x08;
                                                     $1.refine->mod.list.max = $2;
                                                   } else {
                                                     LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "invalid combination of refine substatements");
                                                     YYERROR;
                                                   }
                                                 } else {
                                                   $1.refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                   /* magic - bit 4 in flags means max set */
                                                   $1.refine->flags |= 0x08;
                                                   $1.refine->mod.list.max = $2;
                                                 }
                                                 $$ = $1;
                                               }
                                             }
  |  refine_body_opt_stmts description_stmt { if (read_all && yang_read_description(trg,$1.refine,s,REFINE_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  refine_body_opt_stmts reference_stmt { if (read_all && yang_read_reference(trg,$1.refine,s,REFINE_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  ;

uses_augment_stmt: AUGMENT_KEYWORD sep uses_augment_arg_str { if (read_all) {
                                                                if (!(actual = yang_read_augment(trg, actual, s, yylineno))) {
                                                                  YYERROR;
                                                                }
                                                                data_node = actual;
                                                                s = NULL;
                                                              } else {
                                                                if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                                  LOGMEM;
                                                                  YYERROR;
                                                                }
                                                              }
                                                            }
                   '{' stmtsep
                       augment_opt_stmt { if (read_all && !($7.augment.flag & LYS_DATADEF)){
                                            LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, NULL, "data-def or case", "uses/augment");
                                            YYERROR;
                                          }
                                        }
                   '}' ;

uses_augment_arg_str: descendant_schema_nodeid optsep 
  |  string_1 
  ; 

augment_stmt: AUGMENT_KEYWORD sep augment_arg_str { if (read_all) {
                                                      if (!(actual = yang_read_augment(trg, NULL, s, yylineno))) {
                                                        YYERROR;
                                                      }
                                                      data_node = actual;
                                                      s = NULL;
                                                    } else {
                                                      if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                        LOGMEM;
                                                        YYERROR;
                                                      }
                                                    }
                                                  }
              '{' stmtsep
                  augment_opt_stmt { if (read_all) {
                                       if (!($7.augment.flag & LYS_DATADEF)){
                                         LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, NULL, "data-def or case", "augment");
                                         YYERROR;
                                       }
                                       if (unres_schema_add_node(trg, unres, actual, UNRES_AUGMENT, NULL, yylineno) == -1) {
                                         YYERROR;
                                       }
                                     }
                                   }
               '}' ;

augment_opt_stmt: %empty { if (read_all) {
                             $$.augment.ptr_augment = actual;
                             $$.augment.flag = 0;
                             actual_type = AUGMENT_KEYWORD;
                             if (size_arrays->node[size_arrays->next].if_features) {
                               $$.augment.ptr_augment->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.augment.ptr_augment->features);
                               if (!$$.augment.ptr_augment->features) {
                                 LOGMEM;
                                 YYERROR;
                               }
                             }
                             size_arrays->next++;
                           } else {
                             $$.index = size_arrays->size-1;
                           }
                         }
  |  augment_opt_stmt when_stmt { actual = $1.augment.ptr_augment; actual_type = AUGMENT_KEYWORD; }
     stmtsep
  |  augment_opt_stmt if_feature_stmt { if (read_all) {
                                          if (yang_read_if_feature(trg,$1.augment.ptr_augment,s,unres,AUGMENT_KEYWORD,yylineno)) {YYERROR;}
                                          s=NULL;
                                        } else {
                                          size_arrays->node[$1.index].if_features++;
                                        }
                                      }
  |  augment_opt_stmt status_stmt { if (read_all && yang_read_status($1.augment.ptr_augment,$2,AUGMENT_KEYWORD,yylineno)) {YYERROR;} }
  |  augment_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.augment.ptr_augment,s,AUGMENT_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  augment_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.augment.ptr_augment,s,AUGMENT_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  augment_opt_stmt data_def_stmt { if (read_all) {
                                        actual = $1.augment.ptr_augment;
                                        actual_type = AUGMENT_KEYWORD;
                                        $1.augment.flag |= LYS_DATADEF;
                                        data_node = actual;
                                      }
                                    }
     stmtsep { $$ = $1; }
  |  augment_opt_stmt case_stmt { if (read_all) {
                                    actual = $1.augment.ptr_augment;
                                    actual_type = AUGMENT_KEYWORD;
                                    $1.augment.flag |= LYS_DATADEF;
                                    data_node = actual;
                                  }
                                }
     stmtsep { $$ = $1; }
  ;

augment_arg_str: absolute_schema_nodeids optsep
  |  string_1
  ; 

rpc_stmt: RPC_KEYWORD sep identifier_arg_str { if (read_all) {
                                                 if (!(actual = yang_read_node(trg, NULL, s, LYS_RPC, sizeof(struct lys_node_rpc)))) {
                                                   YYERROR;
                                                 }
                                                 data_node = actual;
                                                 s = NULL;
                                               } else {
                                                 if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                   LOGMEM;
                                                   YYERROR;
                                                 }
                                               }
                                             }
          rpc_end;

rpc_end: ';' { if (read_all) {
                 size_arrays->next++;
               }
             }
  |  '{' stmtsep
         rpc_opt_stmt
      '}'


rpc_opt_stmt: %empty { if (read_all) {
                         $$.rpc.ptr_rpc = actual;
                         $$.rpc.flag = 0;
                         actual_type = RPC_KEYWORD;
                         if (size_arrays->node[size_arrays->next].if_features) {
                           $$.rpc.ptr_rpc->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.rpc.ptr_rpc->features);
                           if (!$$.rpc.ptr_rpc->features) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                         if (size_arrays->node[size_arrays->next].tpdf) {
                           $$.rpc.ptr_rpc->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *$$.rpc.ptr_rpc->tpdf);
                           if (!$$.rpc.ptr_rpc->tpdf) {
                             LOGMEM;
                             YYERROR;
                           }
                         }
                         size_arrays->next++;
                       } else {
                         $$.index = size_arrays->size-1;
                       }
                     }
  |  rpc_opt_stmt if_feature_stmt { if (read_all) {
                                      if (yang_read_if_feature(trg,$1.rpc.ptr_rpc,s,unres,RPC_KEYWORD,yylineno)) {YYERROR;}
                                      s=NULL;
                                    } else {
                                      size_arrays->node[$1.index].if_features++;
                                    }
                                  }
  |  rpc_opt_stmt status_stmt { if (read_all && yang_read_status($1.rpc.ptr_rpc,$2,RPC_KEYWORD,yylineno)) {YYERROR;} }
  |  rpc_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.rpc.ptr_rpc,s,RPC_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  rpc_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.rpc.ptr_rpc,s,RPC_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  rpc_opt_stmt typedef_stmt { if (read_all) {
                                           actual = $1.rpc.ptr_rpc;
                                           actual_type = RPC_KEYWORD;
                                         } else {
                                           size_arrays->node[$1.index].tpdf++;
                                         }
                                       }
     stmtsep
  |  rpc_opt_stmt grouping_stmt { actual = $1.rpc.ptr_rpc;
                                  actual_type = RPC_KEYWORD;
                                  data_node = actual;
                                }
     stmtsep
  |  rpc_opt_stmt input_stmt { if ($1.rpc.flag & LYS_RPC_INPUT) {
                                 LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.rpc.ptr_rpc, "input", "rpc");
                                 YYERROR;
                               }
                               $1.rpc.flag |= LYS_RPC_INPUT;
                               actual = $1.rpc.ptr_rpc;
                               actual_type = RPC_KEYWORD;
                               data_node = actual;
                             }
     stmtsep { $$ = $1; }
  |  rpc_opt_stmt output_stmt { if ($1.rpc.flag & LYS_RPC_OUTPUT) {
                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_LYS, $1.rpc.ptr_rpc, "output", "rpc");
                                  YYERROR;
                                }
                                $1.rpc.flag |= LYS_RPC_OUTPUT;
                                actual = $1.rpc.ptr_rpc;
                                actual_type = RPC_KEYWORD;
                                data_node = actual;
                              }
     stmtsep { $$ = $1; }

input_stmt: INPUT_KEYWORD optsep { if (read_all) {
                                     if (!(actual = yang_read_node(trg, actual, NULL, LYS_INPUT, sizeof(struct lys_node_rpc_inout)))) {
                                      YYERROR;
                                     }
                                     data_node = actual;
                                   } else {
                                     if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                       LOGMEM;
                                       YYERROR;
                                     }
                                   }
                                 }
            '{' stmtsep
                input_output_opt_stmt { if (read_all && !($6.inout.flag & LYS_DATADEF)) {
                                          LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, "data-def", "input");
                                          YYERROR;
                                        }
                                      }
            '}'

input_output_opt_stmt: %empty { if (read_all) {
                                  $$.inout.ptr_inout = actual;
                                  $$.inout.flag = 0;
                                  actual_type = INPUT_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    $$.inout.ptr_inout->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *$$.inout.ptr_inout->tpdf);
                                    if (!$$.inout.ptr_inout->tpdf) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                                } else {
                                  $$.index = size_arrays->size-1;
                                }
                              }
  |  input_output_opt_stmt typedef_stmt { if (read_all) {
                                                    actual = $1.inout.ptr_inout;
                                                    actual_type = INPUT_KEYWORD;
                                                  } else {
                                                    size_arrays->node[$1.index].tpdf++;
                                                  }
                                                }
     stmtsep
  |  input_output_opt_stmt grouping_stmt { actual = $1.inout.ptr_inout;
                                           actual_type = INPUT_KEYWORD;
                                           data_node = actual;
                                         }
     stmtsep
  |  input_output_opt_stmt data_def_stmt { if (read_all) {
                                             actual = $1.inout.ptr_inout;
                                             actual_type = INPUT_KEYWORD;
                                             $1.inout.flag |= LYS_DATADEF;
                                             data_node = actual;
                                           }
                                         }
     stmtsep { $$ = $1; }
  ;

output_stmt: OUTPUT_KEYWORD optsep { if (read_all) {
                                       if (!(actual = yang_read_node(trg, actual, NULL, LYS_OUTPUT, sizeof(struct lys_node_rpc_inout)))) {
                                        YYERROR;
                                       }
                                       data_node = actual;
                                     } else {
                                       if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                         LOGMEM;
                                         YYERROR;
                                       }
                                     }
                                   }
             '{' stmtsep
                 input_output_opt_stmt { if (read_all && !($6.inout.flag & LYS_DATADEF)) {
                                           LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, "data-def", "output");
                                           YYERROR;
                                         }
                                       }
             '}'

notification_stmt: NOTIFICATION_KEYWORD sep identifier_arg_str { if (read_all) {
                                                                   if (!(actual = yang_read_node(trg, NULL, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                                                    YYERROR;
                                                                   }
                                                                   data_node = actual;
                                                                 } else {
                                                                   if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                                     LOGMEM;
                                                                     YYERROR;
                                                                   }
                                                                 }
                                                               }
                   notification_end;

notification_end: ';' { if (read_all) {
                          size_arrays->next++;
                        }
                      }
  |  '{' stmtsep
         notification_opt_stmt
      '}' ;


notification_opt_stmt: %empty { if (read_all) {
                                  $$.notif = actual;
                                  actual_type = NOTIFICATION_KEYWORD;
                                  if (size_arrays->node[size_arrays->next].if_features) {
                                    $$.notif->features = calloc(size_arrays->node[size_arrays->next].if_features, sizeof *$$.notif->features);
                                    if (!$$.notif->features) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].tpdf) {
                                    $$.notif->tpdf = calloc(size_arrays->node[size_arrays->next].tpdf, sizeof *$$.notif->tpdf);
                                    if (!$$.notif->tpdf) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                                } else {
                                  $$.index = size_arrays->size-1;
                                }
                              }
  |  notification_opt_stmt if_feature_stmt { if (read_all) {
                                               if (yang_read_if_feature(trg,$1.notif,s,unres,NOTIFICATION_KEYWORD,yylineno)) {YYERROR;}
                                               s=NULL;
                                             } else {
                                               size_arrays->node[$1.index].if_features++;
                                             }
                                           }
  |  notification_opt_stmt status_stmt { if (read_all && yang_read_status($1.notif,$2,NOTIFICATION_KEYWORD,yylineno)) {YYERROR;} }
  |  notification_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.notif,s,NOTIFICATION_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  notification_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.notif,s,NOTIFICATION_KEYWORD,yylineno)) {YYERROR;} s = NULL; }
  |  notification_opt_stmt typedef_stmt { if (read_all) {
                                                    actual = $1.notif;
                                                    actual_type = NOTIFICATION_KEYWORD;
                                                  } else {
                                                    size_arrays->node[$1.index].tpdf++;
                                                  }
                                                }
     stmtsep
  |  notification_opt_stmt grouping_stmt { actual = $1.notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }
     stmtsep
  |  notification_opt_stmt data_def_stmt { actual = $1.notif;
                                           actual_type = NOTIFICATION_KEYWORD;
                                           data_node = actual;
                                         }
     stmtsep
  ;

deviation_stmt: DEVIATION_KEYWORD sep deviation_arg_str { if (read_all) {
                                                            if (!(actual = yang_read_deviation(trg, s, yylineno))) {
                                                              YYERROR;
                                                            }
                                                            s = NULL;
                                                            trg->deviation_size++;
                                                            } else {
                                                              if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                                LOGMEM;
                                                                YYERROR;
                                                              }
                                                            }

                                                        }
                '{' stmtsep
                    deviation_opt_stmt  { if (read_all) {
                                            if (actual_type == DEVIATION_KEYWORD) {
                                              LOGVAL(LYE_MISSSTMT2, yylineno, LY_VLOG_NONE, NULL, "deviate", "deviation");
                                              YYERROR;
                                            }
                                            if (yang_check_deviation(trg, actual, unres, yylineno)) {
                                              YYERROR;
                                            }
                                            free($7.deviation);
                                          }
                                        }
                '}'

deviation_opt_stmt: %empty { if (read_all) {
                               $$.deviation = actual;
                               actual_type = DEVIATION_KEYWORD;
                               if (size_arrays->node[size_arrays->next].unique) {
                                 $$.deviation->deviation->deviate = calloc(size_arrays->node[size_arrays->next].unique, sizeof *$$.deviation->deviation->deviate);
                                 if (!$$.deviation->deviation->deviate) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                               size_arrays->next++;
                             } else {
                               $$.index = size_arrays->size -1;
                             }
                           }
  |  deviation_opt_stmt description_stmt { if (read_all && yang_read_description(trg,$1.deviation->deviation,s,DEVIATION_KEYWORD,yylineno)) {
                                             free($1.deviation);
                                             YYERROR;
                                           }
                                           s = NULL;
                                           $$ = $1;
                                         }
  |  deviation_opt_stmt reference_stmt { if (read_all && yang_read_reference(trg,$1.deviation->deviation,s,DEVIATION_KEYWORD,yylineno)) {
                                           free($1.deviation);
                                           YYERROR;
                                         }
                                         s = NULL;
                                         $$ = $1;
                                       }
  |  deviation_opt_stmt DEVIATE_KEYWORD sep deviate_body_stmt { if (read_all) {
                                                                  actual = $1.deviation;
                                                                  actual_type = DEVIATE_KEYWORD;
                                                                  $$ = $1;
                                                                } else {
                                                                  /* count of deviate statemenet */
                                                                  size_arrays->node[$1.index].unique++;
                                                                }
                                                              }

deviation_arg_str: absolute_schema_nodeids optsep
  | string_1

deviate_body_stmt: deviate_not_supported_stmt
                   { if (read_all && yang_read_deviate_unsupported(actual, yylineno)) {
                       YYERROR;
                     }
                   }
  |  deviate_stmts optsep

deviate_stmts: deviate_add_stmt
  |  deviate_replace_stmt
  |  deviate_delete_stmt
  ;

deviate_not_supported_stmt: NOT_SUPPORTED_KEYWORD optsep stmtend;

deviate_add_stmt: ADD_KEYWORD optsep { if (read_all && yang_read_deviate(actual, LY_DEVIATE_ADD, yylineno)) {
                                         YYERROR;
                                       }
                                     }
                  deviate_add_end

deviate_add_end: ';'
  |  '{' stmtsep
         deviate_add_opt_stmt
     '}'

deviate_add_opt_stmt: %empty { if (read_all) {
                                 $$.deviation = actual;
                                 actual_type = ADD_KEYWORD;
                                 if (size_arrays->node[size_arrays->next].must) {
                                    if (yang_read_deviate_must(trg->ctx, actual, size_arrays->node[size_arrays->next].must, yylineno)) {
                                      YYERROR;
                                    }
                                  }
                                  if (size_arrays->node[size_arrays->next].unique) {
                                    if (yang_read_deviate_unique(trg->ctx, actual, size_arrays->node[size_arrays->next].unique, yylineno)) {
                                      YYERROR;
                                    }
                                  }
                                  size_arrays->next++;
                               } else {
                                 $$.index = size_arrays->size;
                                 if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                             }
  |  deviate_add_opt_stmt units_stmt { if (read_all) {
                                         if (yang_read_deviate_units(trg->ctx, $1.deviation, s, yylineno)) {
                                           YYERROR;
                                         }
                                         s = NULL;
                                         $$ = $1;
                                       }
                                     }
  |  deviate_add_opt_stmt must_stmt stmtsep { if (read_all) {
                                        actual = $1.deviation;
                                        actual_type = ADD_KEYWORD;
                                        $$ = $1;
                                      } else {
                                        size_arrays->node[$1.index].must++;
                                      }
                                    }
  |  deviate_add_opt_stmt unique_stmt { if (read_all) {
                                          struct lys_node_list *list;

                                          list = (struct lys_node_list *)$1.deviation->target;
                                          if (yang_fill_unique(trg, list, &list->unique[list->unique_size], s, NULL, yylineno)) {
                                            list->unique_size++;
                                            YYERROR;
                                          }
                                          list->unique_size++;
                                          free(s);
                                          s = NULL;
                                          $$ = $1;
                                        } else {
                                          size_arrays->node[$1.index].unique++;
                                        }
                                      }
  |  deviate_add_opt_stmt default_stmt { if (read_all) {
                                           if (yang_read_deviate_default(trg->ctx, $1.deviation, s, yylineno)) {
                                             YYERROR;
                                           }
                                           s = NULL;
                                           $$ = $1;
                                         }
                                       }
  |  deviate_add_opt_stmt config_stmt { if (read_all) {
                                          if (yang_read_deviate_config($1.deviation, $2, yylineno)) {
                                            YYERROR;
                                          }
                                          $$ = $1;
                                        }
                                      }
  |  deviate_add_opt_stmt mandatory_stmt { if (read_all) {
                                             if (yang_read_deviate_mandatory($1.deviation, $2, yylineno)) {
                                               YYERROR;
                                             }
                                             $$ = $1;
                                           }
                                         }
  |  deviate_add_opt_stmt min_elements_stmt { if (read_all) {
                                                if ($1.deviation->deviate->min_set) {
                                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                  YYERROR;
                                                }
                                                if (yang_read_deviate_minmax($1.deviation, $2, 0, yylineno)) {
                                                  YYERROR;
                                                }
                                                $$ =  $1;
                                              }
                                            }
  |  deviate_add_opt_stmt max_elements_stmt { if (read_all) {
                                                if ($1.deviation->deviate->max_set) {
                                                  LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                  YYERROR;
                                                }
                                                if (yang_read_deviate_minmax($1.deviation, $2, 1, yylineno)) {
                                                  YYERROR;
                                                }
                                                $$ =  $1;
                                              }
                                            }

deviate_delete_stmt: DELETE_KEYWORD optsep { if (read_all && yang_read_deviate(actual, LY_DEVIATE_DEL, yylineno)) {
                                               YYERROR;
                                             }
                                           }
                     deviate_delete_end

deviate_delete_end: ';'
  |  '{' stmtsep
         deviate_delete_opt_stmt
      '}' ;

deviate_delete_opt_stmt: %empty { if (read_all) {
                                    $$.deviation = actual;
                                    actual_type = DELETE_KEYWORD;
                                    if (size_arrays->node[size_arrays->next].must) {
                                      if (yang_read_deviate_must(trg->ctx, actual, size_arrays->node[size_arrays->next].must, yylineno)) {
                                        YYERROR;
                                      }
                                    }
                                    if (size_arrays->node[size_arrays->next].unique) {
                                      if (yang_read_deviate_unique(trg->ctx, actual, size_arrays->node[size_arrays->next].unique, yylineno)) {
                                        YYERROR;
                                      }
                                    }
                                    size_arrays->next++;
                                  } else {
                                    $$.index = size_arrays->size;
                                    if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                      LOGMEM;
                                      YYERROR;
                                    }
                                  }
                                }
  |  deviate_delete_opt_stmt units_stmt { if (read_all) {
                                            if (yang_read_deviate_units(trg->ctx, $1.deviation, s, yylineno)) {
                                              YYERROR;
                                            }
                                            s = NULL;
                                            $$ = $1;
                                          }
                                        }
  |  deviate_delete_opt_stmt must_stmt stmtsep { if (read_all) {
                                           if (yang_check_deviate_must(trg->ctx, $1.deviation, yylineno)) {
                                             YYERROR;
                                           }
                                           actual = $1.deviation;
                                           actual_type = DELETE_KEYWORD;
                                           $$ = $1;
                                         } else {
                                           size_arrays->node[$1.index].must++;
                                         }
                                       }
  |  deviate_delete_opt_stmt unique_stmt { if (read_all) {
                                             if (yang_check_deviate_unique(trg, $1.deviation, s, yylineno)) {
                                               YYERROR;
                                             }
                                             s = NULL;
                                             $$ = $1;
                                           } else {
                                             size_arrays->node[$1.index].unique++;
                                           }
                                         }
  |  deviate_delete_opt_stmt default_stmt { if (read_all) {
                                              if (yang_read_deviate_default(trg->ctx, $1.deviation, s, yylineno)) {
                                                YYERROR;
                                              }
                                              s = NULL;
                                              $$ = $1;
                                            }
                                          }

deviate_replace_stmt: REPLACE_KEYWORD optsep { if (read_all && yang_read_deviate(actual, LY_DEVIATE_RPL, yylineno)) {
                                                 YYERROR;
                                               }
                                             }
                      deviate_replace_end

deviate_replace_end: ';'
  |  '{' stmtsep
         deviate_replace_opt_stmt
     '}' ;

deviate_replace_opt_stmt: %empty { if (read_all) {
                                    $$.deviation = actual;
                                    actual_type = REPLACE_KEYWORD;
                                  }
                                }
  |  deviate_replace_opt_stmt type_stmt stmtsep { if (read_all) {
                                            if (unres_schema_add_node(trg, unres, $1.deviation->deviate->type, UNRES_TYPE_DER, $1.deviation->target, yylineno)) {
                                              YYERROR;
                                            }
                                          }
                                        }
  |  deviate_replace_opt_stmt units_stmt { if (read_all) {
                                             if (yang_read_deviate_units(trg->ctx, $1.deviation, s, yylineno)) {
                                               YYERROR;
                                             }
                                             s = NULL;
                                             $$ = $1;
                                           }
                                         }
  |  deviate_replace_opt_stmt default_stmt { if (read_all) {
                                               if (yang_read_deviate_default(trg->ctx, $1.deviation, s, yylineno)) {
                                                 YYERROR;
                                               }
                                               s = NULL;
                                               $$ = $1;
                                             }
                                           }
  |  deviate_replace_opt_stmt config_stmt { if (read_all) {
                                              if (yang_read_deviate_config($1.deviation, $2, yylineno)) {
                                                YYERROR;
                                              }
                                              $$ = $1;
                                            }
                                          }
  |  deviate_replace_opt_stmt mandatory_stmt { if (read_all) {
                                                 if (yang_read_deviate_mandatory($1.deviation, $2, yylineno)) {
                                                   YYERROR;
                                                 }
                                                 $$ = $1;
                                               }
                                             }
  |  deviate_replace_opt_stmt min_elements_stmt { if (read_all) {
                                                    if ($1.deviation->deviate->min_set) {
                                                      LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "min-elements", "deviate");
                                                      YYERROR;
                                                    }
                                                    if (yang_read_deviate_minmax($1.deviation, $2, 0, yylineno)) {
                                                      YYERROR;
                                                    }
                                                    $$ =  $1;
                                                  }
                                                }
  |  deviate_replace_opt_stmt max_elements_stmt { if (read_all) {
                                                    if ($1.deviation->deviate->max_set) {
                                                      LOGVAL(LYE_TOOMANY, yylineno, LY_VLOG_NONE, NULL, "max-elements", "deviate");
                                                      YYERROR;
                                                    }
                                                    if (yang_read_deviate_minmax($1.deviation, $2, 1, yylineno)) {
                                                      YYERROR;
                                                    }
                                                    $$ =  $1;
                                                  }
                                                }

when_stmt: WHEN_KEYWORD sep string  { if (read_all && !(actual=yang_read_when(trg,actual,actual_type,s,yylineno))) {YYERROR;} s=NULL; actual_type=WHEN_KEYWORD;}
           when_end;

when_end: ';'
  |  '{'
         description_reference_stmt
     '}'
  ;

config_stmt: CONFIG_KEYWORD sep config_arg_str stmtend { $$ = $3; }

config_arg_str: TRUE_KEYWORD optsep { $$ = LYS_CONFIG_W; }
  |  FALSE_KEYWORD optsep { $$ = LYS_CONFIG_R; }
  |  string_1  //not implement
  ;

mandatory_stmt: MANDATORY_KEYWORD sep mandatory_arg_str stmtend { $$ = $3; }

mandatory_arg_str: TRUE_KEYWORD optsep { $$ = LYS_MAND_TRUE; }
  |  FALSE_KEYWORD optsep { $$ = LYS_MAND_FALSE; }
  |  string_1 //not implement
  ;

presence_stmt: PRESENCE_KEYWORD sep string stmtend;

min_elements_stmt: MIN_ELEMENTS_KEYWORD sep min_value_arg_str stmtend { $$ = $3; }

min_value_arg_str: non_negative_integer_value optsep { $$ = $1; }
  |  string_1 // not implement
  ;

max_elements_stmt: MAX_ELEMENTS_KEYWORD sep max_value_arg_str stmtend { $$ = $3; }

max_value_arg_str: UNBOUNDED_KEYWORD optsep //not implement
  |  positive_integer_value optsep { $$ = $1; }
  |  string_1 // not implement
  ;

ordered_by_stmt: ORDERED_BY_KEYWORD sep ordered_by_arg_str stmtend { $$ = $3; }

ordered_by_arg_str: USER_KEYWORD optsep { $$ = LYS_USERORDERED; }
  |  SYSTEM_KEYWORD optsep { $$ = LYS_SYSTEMORDERED; }
  |  string_1 { if (!strcmp(s, "user")) {
                  $$ = LYS_USERORDERED;
                } else if (!strcmp(s, "system")) {
                  $$ = LYS_SYSTEMORDERED;
                } else {
                  free(s);
                  YYERROR;
                }
                free(s);
                s=NULL;
              }

must_stmt: MUST_KEYWORD sep string { if (read_all) {
                                       if (!(actual=yang_read_must(trg,actual,s,actual_type,yylineno))) {YYERROR;}
                                       s=NULL;
                                       actual_type=MUST_KEYWORD;
                                     }
                                   }
           must_end;

must_end: ';'
  |  '{' stmtsep
         message_opt_stmt
     '}'
  ;

unique_stmt: UNIQUE_KEYWORD sep unique_arg_str; 

unique_arg_str: descendant_schema_nodeid unique_arg 
  |  string_1 stmtend;

unique_arg: sep descendant_schema_nodeid unique_arg
  |  stmtend;

key_stmt: KEY_KEYWORD sep key_arg_str; 

key_arg_str: node_identifier { if (read_all){
                                 s = strdup(yytext);
                                 if (!s) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                               }
                             }
             key_opt
  |  string_1 stmtend
  ;

key_opt: sep node_identifier { if (read_all) {
                                 s = ly_realloc(s,strlen(s) + yyleng + 2);
                                 if (!s) {
                                   LOGMEM;
                                   YYERROR;
                                 }
                                 strcat(s," ");
                                 strcat(s,yytext);
                                }
                             }
         key_opt
  | stmtend 
  ;

range_arg_str: string { if (read_all) {
                          $$ = actual;
                          if (!(actual = yang_read_range(trg, actual, s, yylineno))) {
                             YYERROR;
                          }
                          actual_type = RANGE_KEYWORD;
                          s = NULL;
                        }
                      }

absolute_schema_nodeid: '/' node_identifier { if (read_all) {
                                                if (s) {
                                                  s = ly_realloc(s,strlen(s) + yyleng + 2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYERROR;
                                                  }
                                                  strcat(s,"/");
                                                  strcat(s,yytext);
                                                } else {
                                                  s = malloc(yyleng+2);
                                                  if (!s) {
                                                    LOGMEM;
                                                    YYERROR;
                                                  }
                                                  s[0]='/';
                                                  memcpy(s+1,yytext,yyleng+1);
                                                }
                                              }
                                            }

absolute_schema_nodeids: absolute_schema_nodeid absolute_schema_nodeid_opt;

absolute_schema_nodeid_opt: %empty 
  |  absolute_schema_nodeid_opt absolute_schema_nodeid
  ;

descendant_schema_nodeid: node_identifier { if (read_all)  {
                                              if (s) {
                                                s = ly_realloc(s,strlen(s) + yyleng + 1);
                                                if (!s) {
                                                  LOGMEM;
                                                  YYERROR;
                                                }
                                                strcat(s,yytext);
                                              } else {
                                                s = strdup(yytext);
                                                if (!s) {
                                                  LOGMEM;
                                                  YYERROR;
                                                }
                                              }
                                            }
                                          }
                          absolute_schema_nodeid_opt;

path_arg_str: { tmp_s = yytext; } absolute_paths { if (read_all) {
                                                     s = strdup(tmp_s);
                                                     if (!s) {
                                                       LOGMEM;
                                                       YYERROR;
                                                     }
                                                     s[strlen(s) - 1] = '\0';
                                                   }
                                                 }
  |  { tmp_s = yytext; } relative_path { if (read_all) {
                                           s = strdup(tmp_s);
                                           if (!s) {
                                             LOGMEM;
                                             YYERROR;
                                           }
                                           s[strlen(s) - 1] = '\0';
                                         }
                                       }
  |  string_1
  ;

absolute_path: '/' node_identifier path_predicate

absolute_paths: absolute_path absolute_path_opt

absolute_path_opt: %empty 
  |  absolute_path_opt absolute_path;

relative_path: relative_path_part1 relative_path_part1_opt descendant_path

relative_path_part1: DOUBLEDOT '/';

relative_path_part1_opt: %empty 
  |  relative_path_part1_opt relative_path_part1;

descendant_path: node_identifier descendant_path_opt

descendant_path_opt: %empty 
  |  path_predicate absolute_paths;

path_predicate: %empty 
  | path_predicate '[' whitespace_opt path_equality_expr whitespace_opt ']'

path_equality_expr: node_identifier whitespace_opt '=' whitespace_opt path_key_expr

path_key_expr: current_function_invocation whitespace_opt '/' whitespace_opt
                     rel_path_keyexpr

rel_path_keyexpr: rel_path_keyexpr_part1 rel_path_keyexpr_part1_opt
                    node_identifier rel_path_keyexpr_part2
                     node_identifier

rel_path_keyexpr_part1: DOUBLEDOT whitespace_opt '/' whitespace_opt;

rel_path_keyexpr_part1_opt: %empty 
  |  rel_path_keyexpr_part1_opt rel_path_keyexpr_part1;

rel_path_keyexpr_part2: %empty 
  | rel_path_keyexpr_part2 whitespace_opt '/' whitespace_opt node_identifier;

current_function_invocation: CURRENT_KEYWORD whitespace_opt '(' whitespace_opt ')'

positive_integer_value: NON_NEGATIVE_INTEGER { /* convert it to uint32_t */
                                                unsigned long val;

                                                val = strtoul(yytext, NULL, 10);
                                                if (val > UINT32_MAX) {
                                                    LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "Converted number is very long.");
                                                    YYERROR;
                                                }
                                                $$ = (uint32_t) val;
                                             }

non_negative_integer_value: ZERO { $$ = 0; }
  |  positive_integer_value { $$ = $1; }
  ;

integer_value: ZERO { $$ = 0; }
  |  integer_value_convert { /* convert it to int32_t */
               int64_t val;

               val = strtoll(yytext, NULL, 10);
               if (val < INT32_MIN || val > INT32_MAX) {
                   LOGVAL(LYE_SPEC, yylineno, LY_VLOG_NONE, NULL, "The number is not in the correct range (INT32_MIN..INT32_MAX): \"%d\"",val);
                   YYERROR;
               }
               $$ = (int32_t) val;
             }
  ;

integer_value_convert: INTEGER
  |  NON_NEGATIVE_INTEGER

prefix_arg_str: string_1
  |  identifiers optsep;

identifier_arg_str: identifiers optsep 
  |  string_1 
  ;

node_identifier: identifier
  |  IDENTIFIERPREFIX
  ; 

identifier_ref_arg_str: identifiers optsep
  | identifiers_ref optsep
  | string_1
  ; 

stmtend: ';' stmtsep
  | '{' stmtsep '}' stmtsep
  ;

stmtsep: %empty 
  | stmtsep sep_stmt 
  | stmtsep unknown_statement
  ;

unknown_statement: identifiers_ref { if (read_all ) {
                                       if (yang_use_extension(trg, data_node, actual, s, yylineno)) {
                                         free(s);
                                         YYERROR;
                                       }
                                       free(s);
                                       s = NULL;
                                     }
                                   }
                   string_opt unknown_statement_end

string_opt: %empty //Soptsep
  |  sep string_1;

unknown_statement_end: ';' 
  |  '{' optsep unknown_statement2_opt '}'

unknown_statement2_opt: %empty 
  |  node_identifier string_opt unknown_statement2_end;

unknown_statement2_end: ';' optsep
  |  '{' optsep unknown_statement2_opt '}' optsep

sep_stmt: WHITESPACE
  | EOL
  ;

optsep: %empty 
  | optsep sep_stmt
  ; 

sep: sep_stmt optsep;

whitespace_opt: %empty 
  | WHITESPACE
  ;


string: strings { if (read_all){
                    s = strdup(yytext);
                    if (!s) {
                      LOGMEM;
                      YYERROR;
                    }
                  }
                }
        optsep
  |  string_1

strings: STRINGS
  |  REVISION_DATE
  |  identifier
  |  IDENTIFIERPREFIX
  |  ZERO
  |  INTEGER
  |  NON_NEGATIVE_INTEGER

identifier: IDENTIFIER
  |  ANYXML_KEYWORD
  |  ARGUMENT_KEYWORD
  |  AUGMENT_KEYWORD
  |  BASE_KEYWORD
  |  BELONGS_TO_KEYWORD
  |  BIT_KEYWORD
  |  CASE_KEYWORD
  |  CHOICE_KEYWORD
  |  CONFIG_KEYWORD
  |  CONTACT_KEYWORD
  |  CONTAINER_KEYWORD
  |  DEFAULT_KEYWORD
  |  DESCRIPTION_KEYWORD
  |  ENUM_KEYWORD
  |  ERROR_APP_TAG_KEYWORD
  |  ERROR_MESSAGE_KEYWORD
  |  EXTENSION_KEYWORD
  |  DEVIATION_KEYWORD
  |  DEVIATE_KEYWORD
  |  FEATURE_KEYWORD
  |  FRACTION_DIGITS_KEYWORD
  |  GROUPING_KEYWORD
  |  IDENTITY_KEYWORD
  |  IF_FEATURE_KEYWORD
  |  IMPORT_KEYWORD
  |  INCLUDE_KEYWORD
  |  INPUT_KEYWORD
  |  KEY_KEYWORD
  |  LEAF_KEYWORD
  |  LEAF_LIST_KEYWORD
  |  LENGTH_KEYWORD
  |  LIST_KEYWORD
  |  MANDATORY_KEYWORD
  |  MAX_ELEMENTS_KEYWORD
  |  MIN_ELEMENTS_KEYWORD
  |  MODULE_KEYWORD
  |  MUST_KEYWORD
  |  NAMESPACE_KEYWORD
  |  NOTIFICATION_KEYWORD
  |  ORDERED_BY_KEYWORD
  |  ORGANIZATION_KEYWORD
  |  OUTPUT_KEYWORD
  |  PATH_KEYWORD
  |  PATTERN_KEYWORD
  |  POSITION_KEYWORD
  |  PREFIX_KEYWORD
  |  PRESENCE_KEYWORD
  |  RANGE_KEYWORD
  |  REFERENCE_KEYWORD
  |  REFINE_KEYWORD
  |  REQUIRE_INSTANCE_KEYWORD
  |  REVISION_KEYWORD
  |  REVISION_DATE_KEYWORD
  |  RPC_KEYWORD
  |  STATUS_KEYWORD
  |  SUBMODULE_KEYWORD
  |  TYPE_KEYWORD
  |  TYPEDEF_KEYWORD
  |  UNIQUE_KEYWORD
  |  UNITS_KEYWORD
  |  USES_KEYWORD
  |  VALUE_KEYWORD
  |  WHEN_KEYWORD
  |  YANG_VERSION_KEYWORD
  |  YIN_ELEMENT_KEYWORD
  |  ADD_KEYWORD
  |  CURRENT_KEYWORD
  |  DELETE_KEYWORD
  |  DEPRECATED_KEYWORD
  |  FALSE_KEYWORD
  |  MAX_KEYWORD
  |  MIN_KEYWORD
  |  NOT_SUPPORTED_KEYWORD
  |  OBSOLETE_KEYWORD
  |  REPLACE_KEYWORD
  |  SYSTEM_KEYWORD
  |  TRUE_KEYWORD
  |  UNBOUNDED_KEYWORD
  |  USER_KEYWORD
  ;

identifiers: identifier { if (read_all) {
                            s = strdup(yytext);
                            if (!s) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }

identifiers_ref: IDENTIFIERPREFIX { if (read_all) {
                                      s = strdup(yytext);
                                      if (!s) {
                                        LOGMEM;
                                        YYERROR;
                                      }
                                    }
                                  }

%%

void yyerror(struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all, char *str, ...){

  LOGVAL(LYE_INSTMT, yylineno, LY_VLOG_NONE, NULL, yytext);
  free(s);
}
