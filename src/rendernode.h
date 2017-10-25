#pragma once

#include <string>
#include "node.h"

class RenderNode : public AbstractNode {
public:
    VISITABLE();
    RenderNode(const ModuleInstantiation *mi)
        : AbstractNode(mi), convexity(1) {}
    virtual std::string toString() const;
    virtual std::string name() const { return "render"; }

    int convexity;
};
