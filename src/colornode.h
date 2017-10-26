#pragma once

#include "linalg.h"
#include "node.h"

class ColorNode : public AbstractNode {
public:
	VISITABLE();
	ColorNode(const ModuleInstantiation *mi)
	    : AbstractNode(mi), color(-1.0f, -1.0f, -1.0f, 1.0f) {}
	virtual std::string toString() const;
	virtual std::string name() const;

	Color4f color;
};
