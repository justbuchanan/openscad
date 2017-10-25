#pragma once

#include <string>
#include "AST.h"

class Package : public ASTNode {
public:
    Package() {}
    virtual ~Package() {}

    void setPath(const std::string &path) { this->_path = path; }
    const std::string &path() const { return this->_path; }

    LocalScope scope;
    typedef std::unordered_set<std::string> ModuleContainer;
    ModuleContainer usedlibs;

private:
    std::string _path;
};
