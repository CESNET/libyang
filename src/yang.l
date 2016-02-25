%option noyywrap yylineno
%{
#include "parser_yang.h"
#include "parser_yang_bis.h"
%}
%x COMMENT1
%x COMMENT2
%x RANGE
%x STRING1
%x STRING2
%x PATH
%%

"/*" {BEGIN COMMENT1; }
<COMMENT1>.  |
<COMMENT1>\n 
<COMMENT1>"*/" {BEGIN INITIAL; }
"//" {BEGIN COMMENT2;}
<COMMENT2>. 
<COMMENT2>\n {BEGIN INITIAL; return EOL; }
"anyxml" { return ANYXML_KEYWORD; }
"argument" { return ARGUMENT_KEYWORD; }
"augment" { BEGIN PATH; return AUGMENT_KEYWORD; }
"base" { return BASE_KEYWORD; }
"belongs-to" { return BELONGS_TO_KEYWORD; }
"bit" { return BIT_KEYWORD; }
"case" { return CASE_KEYWORD; }
"choice" { return CHOICE_KEYWORD; }
"config" { return CONFIG_KEYWORD; }
"contact" { return CONTACT_KEYWORD; }
"container" { return CONTAINER_KEYWORD; }
"default" { return DEFAULT_KEYWORD; }
"description" { return DESCRIPTION_KEYWORD; }
"enum" { return ENUM_KEYWORD; }
"error-app-tag" { return ERROR_APP_TAG_KEYWORD; }
"error-message" { return ERROR_MESSAGE_KEYWORD; }
"extension" { return EXTENSION_KEYWORD; }
"deviation" { BEGIN PATH; return DEVIATION_KEYWORD; }
"deviate" { return DEVIATE_KEYWORD; }
"feature" { return FEATURE_KEYWORD; }
"fraction-digits" { return FRACTION_DIGITS_KEYWORD; }
"grouping" { return GROUPING_KEYWORD; }
"identity" { return IDENTITY_KEYWORD; }
"if-feature" { return IF_FEATURE_KEYWORD; }
"import" { return IMPORT_KEYWORD; }
"include" { return INCLUDE_KEYWORD; }
"input" { return INPUT_KEYWORD; }
"key" { return KEY_KEYWORD; }
"leaf" { return LEAF_KEYWORD; }
"leaf-list" { return LEAF_LIST_KEYWORD; }
"length" { return LENGTH_KEYWORD; }
"list" { return LIST_KEYWORD; }
"mandatory" { return MANDATORY_KEYWORD; }
"max-elements" { return MAX_ELEMENTS_KEYWORD; }
"min-elements" { return MIN_ELEMENTS_KEYWORD; }
"module" { return MODULE_KEYWORD; }
"must" { return MUST_KEYWORD; }
"namespace" { return NAMESPACE_KEYWORD; }
"notification" { return NOTIFICATION_KEYWORD; }
"ordered-by" { return ORDERED_BY_KEYWORD; }
"organization" { return ORGANIZATION_KEYWORD; }
"output" { return OUTPUT_KEYWORD; }
"path" { BEGIN PATH; return PATH_KEYWORD; }
"pattern" { return PATTERN_KEYWORD; }
"position" { return POSITION_KEYWORD; }
"prefix" { return PREFIX_KEYWORD; }
"presence" { return PRESENCE_KEYWORD; }
"range" { return RANGE_KEYWORD; }
"reference" { return REFERENCE_KEYWORD; }
"refine" { BEGIN PATH; return REFINE_KEYWORD; }
"require-instance" { return REQUIRE_INSTANCE_KEYWORD; }
"revision" { return REVISION_KEYWORD; }
"revision-date" { return REVISION_DATE_KEYWORD; }
"rpc" { return RPC_KEYWORD; }
"status" { return STATUS_KEYWORD; }
"submodule" { return SUBMODULE_KEYWORD; }
"type" { return TYPE_KEYWORD; }
"typedef" { return TYPEDEF_KEYWORD; }
"unique" { BEGIN PATH; return UNIQUE_KEYWORD; }
"units" { return UNITS_KEYWORD; }
"uses" { return USES_KEYWORD; }
"value" { return VALUE_KEYWORD; }
"when" { return WHEN_KEYWORD; }
"yang-version" { return YANG_VERSION_KEYWORD; }
"yin-element" { return YIN_ELEMENT_KEYWORD; }
"add" { return ADD_KEYWORD; }
"current" { return CURRENT_KEYWORD; }
<PATH>"current" { return CURRENT_KEYWORD; }
"delete" { return DELETE_KEYWORD; }
"deprecated" { return DEPRECATED_KEYWORD; }
"false" { return FALSE_KEYWORD; }
<RANGE>"max" { return MAX_KEYWORD; }
<RANGE>"min" { return MIN_KEYWORD; }
"not-supported" { return NOT_SUPPORTED_KEYWORD; }
"obsolete" { return OBSOLETE_KEYWORD; }
"replace" { return REPLACE_KEYWORD; }
"system" { return SYSTEM_KEYWORD; }
"true" { return TRUE_KEYWORD; }
"unbounded" { return UNBOUNDED_KEYWORD; }
"user" { return USER_KEYWORD; }
"{" |
"}" |
";" | 
"+"  { return yytext[0];}  /* unsolved problem with concatenate string '+' */
\" {BEGIN STRING1;}
<STRING1>([\n\t !#-\[\]-~]|\\(\"|\\|n|t))* {return STRING;}
<STRING1>\" {BEGIN INITIAL;}
"'" {BEGIN STRING2;}
<STRING2>([\n\t -&\(-~]|\\(\l|\\|n|t))* {return STRING;}
<STRING2>"'" {BEGIN INITIAL;}
<RANGE>".." {return DOUBLEDOT;}
<RANGE>"|" {return yytext[0];}
"0" {return ZERO;}
<RANGE>"-"[1-9][0-9]* {return INTEGER;}
[1-9][0-9]* {return NON_NEGATIVE_INTEGER;}
<RANGE>[-]?[1-9][0-9]*\.[0-9]+ {return DECIMAL;}
<RANGE>";"  {BEGIN INITIAL; return yytext[0]; }
<RANGE>"{"  {BEGIN INITIAL; return yytext[0]; }
<RANGE>\n {return EOL;}
<RANGE>[ \t]+ { return WHITESPACE;}
<RANGE>\" {BEGIN STRING1;}
<RANGE>"'" {BEGIN STRING2;}
<RANGE>"//" {BEGIN COMMENT2;}
<RANGE>"/*" {BEGIN COMMENT1;}
<RANGE>. { printf("%d: unexpected character '%c'\n",yylineno,*yytext); return ERROR;}
<PATH>"/" |
<PATH>"[" |
<PATH>"]" |
<PATH>"=" |
<PATH>"(" |
<PATH>")"  {return yytext[0];}
<PATH>".." {return DOUBLEDOT;}
<PATH>\n { return EOL;}
<PATH>[ \t]+ { return WHITESPACE;}
<PATH>";" |
<PATH>"{" {BEGIN INITIAL; return yytext[0];}
<PATH>\" {BEGIN STRING1;}
<PATH>"'" {BEGIN STRING2;}
<PATH>"//" {BEGIN COMMENT2;}
<PATH>"/*" {BEGIN COMMENT1;}
<PATH>[A-Za-z_][-A-Za-z0-9_\.]* {return IDENTIFIER;}
<PATH>[A-Za-z_][-A-Za-z0-9_\.]*:[A-Za-z_][-A-Za-z0-9_\.]*   {return IDENTIFIERPREFIX;}
[0-9]{4}[-][0-9]{2}[-][0-9]{2} {return REVISION_DATE;}
[A-Za-z_][-A-Za-z0-9_\.]* {return IDENTIFIER;}
[A-Za-z_][-A-Za-z0-9_\.]*:[A-Za-z_][-A-Za-z0-9_\.]*   {return IDENTIFIERPREFIX;}
([!#-&\(-:<-\[\]-z|~])+ {if (yytext[0]=='/') {
                           switch(yytext[1]) { 
                             case '*': BEGIN COMMENT1; yyless(2); break;
                             case '/': yyless(2); BEGIN COMMENT2; break;
                             default:  return STRINGS;
                            }
                          } else {return STRINGS;} }
\n { return EOL;}
[ \t]+ { return WHITESPACE;}

. { printf("%d: unexpected character '%c'\n",yylineno,*yytext); return ERROR;}

%%