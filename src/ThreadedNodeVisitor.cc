#include "ThreadedNodeVisitor.h"

#include "printutils.h"
#include <iostream>

using namespace std;

int ThreadedNodeVisitor::Parallelism = 0;


namespace {

constexpr bool THREAD_DEBUG = true;


// This is the main function for each of the worker threads. It reads items from
// the work queue and processes them. It exits when the context signals to abort
// or that it is finished.
void ProcessWorkItems(ProcessingContext*ctx, NodeVisitor*visitor) {
    std::shared_ptr<WorkItem> nextWorkItem;

    while (!ctx->exitNow()) {
        std::shared_ptr<WorkItem> workItem;

        if (nextWorkItem) {
            workItem = nextWorkItem;
            nextWorkItem = nullptr;
        } else {
            // Wait for a work item to process
            std::unique_lock<std::mutex> lk(ctx->queueMutex);
            ctx->cv.wait(lk, [ctx]() {
                return !ctx->workQueue.empty() || ctx->exitNow();
            });

            if (ctx->exitNow()) {
                return;
            }

            if (ctx->workQueue.empty()) {
                continue;
            }

            // Get available work item
            workItem = ctx->workQueue.front();
            ctx->workQueue.pop();
            lk.unlock();
        }


        if (THREAD_DEBUG){
            // cout << "Processing item" << endl;
            cout << '*';
            fflush(stdout);
        }

        // Run postfix
        try {
            Response response = workItem->node->accept(workItem->state, *visitor);
            if (response == Response::AbortTraversal) {
                ctx->abort();
                return;
            }
        } catch (ProgressCancelException) {
            ctx->cancel();
            return;
        }

        if (workItem->parentWork) {
            // decrement remaining child count for pending parent work item. If this
            // was the last one, push the parent work item onto the queue.
            int x = workItem->parentWork->pendingChildren.fetch_sub(1);
            if (x == 1) {
                if (THREAD_DEBUG) {
                    // cout << "Pushing parent work item" << endl;
                    cout << '^';
                    fflush(stdout);
                }

                nextWorkItem = workItem->parentWork;
            }
        } else {
            // A parentless item is the root
            if (THREAD_DEBUG) {
                cout << "Finished traversing root item" << endl;
            }
            ctx->finish();
        }
    }
}

} // namespace

void ProcessingContext::start(NodeVisitor* visitor) {
    // Start worker threads. Use Parallism if nonzero, otherwise use as many
    // threads as the system has cores.
    const size_t numThreads = ThreadedNodeVisitor::Parallelism != 0 ?
                    ThreadedNodeVisitor::Parallelism :
                    std::thread::hardware_concurrency();

    for (int i = 0; i < numThreads; i++) {
        // TODO: emplace_back instead of move()?
        std::thread t([this, visitor](){
            try {
                ProcessWorkItems(this, visitor);
            } catch (...) {
                std::cerr << "UNHANDLED EXCEPTION IN WORKER THREAD" << std::endl;
                exit(1);
            }
        });
        _workerThreads.push_back(std::move(t));
    }
    PRINTB("Started %d worker threads", numThreads);
}

void ProcessingContext::wait() {
    // Wait for threads to finish processing all postfix traversals.
    for (auto& t : _workerThreads) t.join();

    if (THREAD_DEBUG) {
        std::cout << "JOINED THREADS" << std::endl;
    }

    // Re-throw exception if we ended early due to a user-requested cancel.
    if (isCanceled()) {
        throw ProgressCancelException();
    }
}

bool ProcessingContext::exitNow() {
    return _abort || _finished | _canceled;
}

void ProcessingContext::cancel() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        _canceled = true;
    }
    cv.notify_all();
}

bool ProcessingContext::isCanceled() const {
    return _canceled;
}

void ProcessingContext::abort() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        _abort = true;
    }
    cv.notify_all();
}

bool ProcessingContext::isAborted() const {
    return _abort;
}

void ProcessingContext::finish() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        _finished = true;
    }
    cv.notify_all();
}

void ProcessingContext::pushWorkItem(std::shared_ptr<WorkItem> item) {
    {
        std::lock_guard<std::mutex> lk(queueMutex);
        workQueue.push(std::move(item));
    }
    cv.notify_one();
}


void ThreadedNodeVisitor::traverseThreadedRecursive(ProcessingContext*ctx,  NodeVisitor*visitor,
    std::shared_ptr<WorkItem> parentWorkItem, const AbstractNode &node, const class State &state) {

    if (ctx->exitNow()) return; // Abort immediately

    // Run prefix
    State newstate = state;
    newstate.setNumChildren(node.getChildren().size());
    newstate.setPrefix(true);
    newstate.setParent(state.parent());
    Response response;
    try {
        response = node.accept(newstate, *visitor);
    } catch (ProgressCancelException) {
        ctx->cancel();
        return;
    }

    if (response == Response::AbortTraversal) {
        ctx->abort();
        return;
    }

    // build postfix work item
    std::shared_ptr<WorkItem> postfixWorkItem = std::make_shared<WorkItem>(node.getChildren().size());
    postfixWorkItem->state = newstate;
    postfixWorkItem->state.setPrefix(false);
    postfixWorkItem->state.setPostfix(true);
    postfixWorkItem->node = &node;
    postfixWorkItem->parentWork = parentWorkItem;

    if (response == Response::PruneTraversal || node.getChildren().empty()) {
        // leaf node - queue parent work directly
        ctx->pushWorkItem(postfixWorkItem);
        return;
    }

    // Recurse
    newstate.setParent(&node);
    for(const auto &chnode : node.getChildren()) {
        _traverseThreadedRecursive(ctx, visitor, postfixWorkItem, *chnode, newstate);
        if (ctx->exitNow()) return; // Abort immediately
    }
}

// This begins by starting a fixed number of worker threads at the start of
// traversal and scheduling work amongst them as it becomes available. A
// condition variable is used to efficiently sleep threads while work is
// unavailable.
//
// The geometry tree is traversed top-to-bottom recursively, running prefix
// traversal for each of the nodes on the way down. This happens serially,
// ensuring that each node's prefix traversal happens before any of it's
// children's.
//
// The postfix traversals are usually (much) more expensive to run than the
// prefix traversals and these are what we run in parallel. The important
// constraint on running these in parallel is that a given node's postfix
// traversal can not be run until the postfix traversals for each of the child
// nodes is run.
//
// As the tree is recursively traversed top-to-bottom, a tree of pending postfix
// traversals is built. In order to keep track of which nodes are ready to run,
// each node keeps a counter of how many child nodes have yet to complete their
// postfix traversals. Each time a postfix traversal is completed, the pending
// child count of it's parent node is decremented. When the counter gets to
// zero, that node is pushed onto the work queue and is executed as soon as a
// thread is available.
Response ThreadedNodeVisitor::traverseThreaded(const AbstractNode &node, const class State &state) {
    // Create the context that will be passed to all recursive calls
    ProcessingContext ctx;

    // Start threads
    ctx.start(this);

    // Recursively do all prefix traversals and schedule postfix traversals to
    // happen later.
    traverseThreadedRecursive(&ctx, this, nullptr, node, state);

    ctx.wait();

    return ctx.isAborted() ? Response::AbortTraversal : Response::ContinueTraversal;
}
