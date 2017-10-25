#ifndef CSGTEXTCACHE_H_
#define CSGTEXTCACHE_H_

#include <boost/unordered_map.hpp>
#include <string>
#include "Tree.h"

using std::string;

class CSGTextCache {
public:
    CSGTextCache(const Tree &tree) : tree(tree) {}
    ~CSGTextCache() {}

    bool contains(const AbstractNode &node) const;
    string operator[](const AbstractNode &node) const;
    void insert(const class AbstractNode &node, const string &value);
    void remove(const class AbstractNode &node);
    void clear();

private:
    boost::unordered_map<string, string> cache;
    const Tree &tree;
};

#endif
