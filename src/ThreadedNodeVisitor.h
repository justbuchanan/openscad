#pragma once

#include "NodeVisitor.h"
#include "progress.h"
#include "Tree.h"
#include "CGAL_Nef_polyhedron.h"

#include <thread>
#include <condition_variable>


class ProcessingContext;

class WorkItem {
public:
    WorkItem(int numChildren) : pendingChildren(numChildren) {}
    typedef std::function<void(ProcessingContext* ctx)> WorkFunction;
    WorkFunction func = nullptr;
    // State state;
    // const AbstractNode *node = nullptr;
    std::atomic<int> pendingChildren;
    std::shared_ptr<WorkItem> parentWork;
};

class ProcessingContext {
public:
    ProcessingContext() {}

    void start(NodeVisitor* visitor);

    void wait();

    bool exitNow();

    void cancel();
    bool isCanceled() const;

    void abort();
    bool isAborted() const;

    void finish();

    void pushWorkItem(std::shared_ptr<WorkItem> item);

    std::queue<std::shared_ptr<WorkItem>> workQueue;
    // This lock is required when reading or writing the workQueue
    std::mutex queueMutex;
    // The condition variable is signaled whenever a new item is added to the queue.
    std::condition_variable cv;

private:
    bool _abort = false;
    bool _finished = false;
    bool _canceled = false;

    std::vector<std::thread> _workerThreads;
};



class ThreadedNodeVisitor : public NodeVisitor {
  const Tree &_tree;

public:
  ThreadedNodeVisitor(const Tree &tree) : _tree(tree) {}

  Response traverseThreaded(const AbstractNode &node, const class State &state = NodeVisitor::nullstate);


  void traverseThreadedRecursive(ProcessingContext*ctx,  NodeVisitor*visitor,
        std::shared_ptr<WorkItem> parentWorkItem, const AbstractNode &node, const class State &state);

  // Number of threads to spawn for parallel traversal. If 0 (the default), uses
  // the value returned from std::thread::hardware_concurrency().
  static int Parallelism;

  std::shared_ptr<ProcessingContext> processingContext;
};
