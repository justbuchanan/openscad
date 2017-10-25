#pragma once

#include "linalg.h"
#include "node.h"

class TransformNode : public AbstractNode {
public:
    VISITABLE();
    TransformNode(const ModuleInstantiation *mi);
    virtual std::string toString() const;
    virtual std::string name() const;

    Transform3d matrix;
};
