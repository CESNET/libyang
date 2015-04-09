/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML parser implementation for libyang
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

#include <unistd.h>

#include "../common.h"
#include "xml.h"


struct lyxml_elem *lyxml_read(const char *UNUSED(data), int UNUSED(options))
{
	return NULL;
}

struct lyxml_elem *lyxml_read_fd(int UNUSED(fd), int UNUSED(options))
{
	return NULL;
}

struct lyxml_elem *lyxml_read_file(const char *UNUSED(filename), int UNUSED(options))
{
	return NULL;
}

void lyxml_free_elem(struct lyxml_elem *UNUSED(elem), int UNUSED(recursive))
{

}
