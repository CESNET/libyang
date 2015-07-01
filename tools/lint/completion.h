#ifndef COMPLETION_H_
#define COMPLETION_H_

#include "../../linenoise/linenoise.h"

void complete_cmd(const char *buf, linenoiseCompletions *lc);

#endif /* COMPLETION_H_ */