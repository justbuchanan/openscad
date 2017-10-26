#ifndef CSGTEXTRENDERER_H_
#define CSGTEXTRENDERER_H_

#include "CSGTextCache.h"
#include "NodeVisitor.h"
#include "enums.h"

#include <list>
#include <map>

using std::list;
using std::map;
using std::string;

class CSGTextRenderer : public NodeVisitor {
public:
	CSGTextRenderer(CSGTextCache &cache) : cache(cache) {}
	virtual ~CSGTextRenderer() {}

	virtual Response visit(State &state, const AbstractNode &node);
	virtual Response visit(State &state, const AbstractIntersectionNode &node);
	virtual Response visit(State &state, const CsgOpNode &node);
	virtual Response visit(State &state, const TransformNode &node);
	virtual Response visit(State &state, const AbstractPolyNode &node);

private:
	void addToParent(const State &state, const AbstractNode &node);
	bool isCached(const AbstractNode &node);
	void process(string &target, const string &src, OpenSCADOperator op);
	void applyToChildren(const AbstractNode &node, OpenSCADOperator op);

	string currindent;
	typedef list<const AbstractNode *> ChildList;
	map<int, ChildList> visitedchildren;

	CSGTextCache &cache;
};

#endif
