#pragma once

#include "Assignment.h"
#include "FileModule.h"

namespace CommentParser {

shared_ptr<Expression> parser(const char *text);
void collectParameters(const char *fulltext, FileModule *root_module);

}  // namespace CommentParser
