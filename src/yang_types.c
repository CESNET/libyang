/**
 * @file yang_types.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Static YANG built-in-types
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <stdlib.h>

#include "tree.h"

struct ly_tpdf ly_type_binary = {
		.name = "binary",
		.module = NULL,
		.dsc = "Any binary data",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_BINARY,
		.type = {0}
};

struct ly_tpdf ly_type_bits = {
		.name = "bits",
		.module = NULL,
		.dsc = "A set of bits or flags",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_BITS,
		.type = {0}
};

struct ly_tpdf ly_type_bool = {
		.name = "boolean",
		.module = NULL,
		.dsc = "true or false",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_BOOL,
		.type = {0}
};

struct ly_tpdf ly_type_dec64 = {
		.name = "decimal64",
		.module = NULL,
		.dsc = "64-bit signed decimal number",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_DEC64,
		.type = {0}
};

struct ly_tpdf ly_type_empty = {
		.name = "empty",
		.module = NULL,
		.dsc = "A leaf that does not have any value",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_EMPTY,
		.type = {0}
};

struct ly_tpdf ly_type_enum = {
		.name = "enumeration",
		.module = NULL,
		.dsc = "Enumerated strings",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_ENUM,
		.type = {0}
};

struct ly_tpdf ly_type_ident = {
		.name = "identityref",
		.module = NULL,
		.dsc = "A reference to an abstract identity",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_IDENT,
		.type = {0}
};

struct ly_tpdf ly_type_inst = {
		.name = "instance-identifier",
		.module = NULL,
		.dsc = "References a data tree node",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_INST,
		.type = {0}
};

struct ly_tpdf ly_type_int8 = {
		.name = "int8",
		.module = NULL,
		.dsc = "8-bit signed integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_INT8,
		.type = {0}
};

struct ly_tpdf ly_type_int16 = {
		.name = "int16",
		.module = NULL,
		.dsc = "16-bit signed integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_INT16,
		.type = {0}
};

struct ly_tpdf ly_type_int32 = {
		.name = "int32",
		.module = NULL,
		.dsc = "32-bit signed integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_INT32,
		.type = {0}
};

struct ly_tpdf ly_type_int64 = {
		.name = "int64",
		.module = NULL,
		.dsc = "64-bit signed integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_INT64,
		.type = {0}
};

struct ly_tpdf ly_type_leafref = {
		.name = "leafref",
		.module = NULL,
		.dsc = "A reference to a leaf instance",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_LEAFREF,
		.type = {0}
};

struct ly_tpdf ly_type_string = {
		.name = "string",
		.module = NULL,
		.dsc = "Human-readable string",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_STRING,
		.type = {0}
};

struct ly_tpdf ly_type_uint8 = {
		.name = "uint8",
		.module = NULL,
		.dsc = "8-bit unsigned integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_UINT8,
		.type = {0}
};

struct ly_tpdf ly_type_uint16 = {
		.name = "uint16",
		.module = NULL,
		.dsc = "16-bit unsigned integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_UINT16,
		.type = {0}
};

struct ly_tpdf ly_type_uint32 = {
		.name = "uint32",
		.module = NULL,
		.dsc = "32-bit unsigned integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_UINT32,
		.type = {0}
};

struct ly_tpdf ly_type_uint64 = {
		.name = "uint64",
		.module = NULL,
		.dsc = "64-bit unsigned integer",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_UINT64,
		.type = {0}
};

struct ly_tpdf ly_type_union = {
		.name = "union",
		.module = NULL,
		.dsc = "Choice of member types",
		.ref = "RFC 6020, section 4.2.4",
		.flags = 0,
		.base = LY_TYPE_UNION,
		.type = {0}
};

struct ly_types ly_types[LY_DATA_TYPE_COUNT] = {
		{LY_TYPE_DEF, NULL},
		{LY_TYPE_BINARY, &ly_type_binary},
		{LY_TYPE_BITS, &ly_type_bits},
		{LY_TYPE_BOOL, &ly_type_bool},
		{LY_TYPE_DEC64, &ly_type_dec64},
		{LY_TYPE_EMPTY, &ly_type_empty},
		{LY_TYPE_ENUM, &ly_type_enum},
		{LY_TYPE_IDENT, &ly_type_ident},
		{LY_TYPE_INST, &ly_type_inst},
		{LY_TYPE_INT8, &ly_type_int8},
		{LY_TYPE_INT16, &ly_type_int16},
		{LY_TYPE_INT32, &ly_type_int32},
		{LY_TYPE_INT64, &ly_type_int64},
		{LY_TYPE_LEAFREF, &ly_type_leafref},
		{LY_TYPE_STRING, &ly_type_string},
		{LY_TYPE_UINT8, &ly_type_uint8},
		{LY_TYPE_UINT16, &ly_type_uint16},
		{LY_TYPE_UINT32, &ly_type_uint32},
		{LY_TYPE_UINT64, &ly_type_uint64},
		{LY_TYPE_UNION, &ly_type_union}
};
