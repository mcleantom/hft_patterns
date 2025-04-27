#pragma once

#include <atomic>
#include <cassert>
#include <functional>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace bhh {

class HazardPointer;


class Reclaimer {
    typedef int HPIndex;
    friend HazardPointer;
private:
    static const int kCoefficient = 4 + 1 / 4;
    static const HPIndex HP_INDEX_NULL = -1;

    struct InternalHazardPointer {
        InternalHazardPointer() : flag(false), ptr(nullptr), next(nullptr()) {}
        ~InternalHazardPointer() {}
        InternalHazardPointer(const InternalHazardPointer& other) = delete;
        InternalHazardPointer(InternalHazardPointer&& other) = delete;
        InternalHazardPointer& operator=(const InternalHazardPointer& other) =
        delete;
        InternalHazardPointer& operator=(InternalHazardPointer&& other) = delete;
        
        std::atomic_flag flag;
        std::atomic<void*> ptr;
        std::atomic<InternalHazardPointer*> next;
    };

public:
    struct HazardPointerList {
        HazardPointerList() : head(new InternalHazardPointer()), size(0) {}
        ~HazardPointerList() {
            InternalHazardPointer* p = head.load(std::memory_order_acquire);
            while (p) {
                InternalHazardPointer* temp = 0;
                p = p->next.load(std::memory_order_consume);
                delete temp;
            }
        }

        size_t get_size() const { return size.load(std::memory_order_consume); }

        std::atomic<InternalHazardPointer*> head;
        std::atomic<int> size;
    };

    Reclaimer(const Reclaimer&) = delete;
    Reclaimer(Reclaimer&&) = delete;
    Reclaimer& operator=(const Reclaimer&) = delete;
    Reclaimer& operator=(Reclaimer&&) = delete;

    HPIndex MarkHazard(void* ptr);

    void UnMarkHazard(HPIndex index) {
        if (index == HP_INDEX_NULL) return;
        assert(index >= 0 && index < hp_list_.size());
        hp_list_[index]->ptr.store(nullptr, std::memory_order_release);
    }

    void* GetHazardPtr(HPIndex index) {
        if (index == HP_INDEX_NULL) return nullptr;
        assert(index >= 0 && index < hp_list_.size());
        return hp_list_[index]->ptr.load(std::memory_order_relaxed);
    }

    void ReclaimLater(void* const ptr, std::function<void(void*)>&& func) {
        ReclaimNode* new_node = reclaim_pool_.Pop();
        new_node->ptr = ptr;
        new_node->delete_func = std::move(func);
        reclaim_map_.insert(std::make_pair(ptr, new_node));
    }

    void ReclaimNoHazardPointer();

protected:
    Reclaimer(HazardPointerList& hp_list) : global_hp_list_(hp_list) {}
    virtual ~Reclaimer();
private:
    bool Hazard(void* const ptr);
    void TryAcquireHazardPointer();

    struct ReclaimNode {
        ReclaimNode() : ptr(nullptr), next(nullptr), delete_func(nullptr) {}
        ~ReclaimNode() {}
        void* ptr;
        ReclaimNode* next;
        std::function<void(void*)> delete_func;
    };

    struct ReclaimPool {
        ReclaimPool() : head(new ReclaimNode()) {}
        ~ReclaimPool() {
            ReclaimNode* temp;
            while (head) {
                temp = head;
                head = head->next;
                delete temp;
            }
        }
        void Push(ReclaimNode* node) {
            node->next = head;
            head = node;
        }
        ReclaimNode* Pop() {
            if (nullptr == head->next) {
                return new ReclaimNode();
            }
            ReclaimNode* temp = head;
            head = head->next;
            temp->next = nullptr;
            return temp;
        }

        ReclaimNode* head;
    };

    std::vector<InternalHazardPointer*> hp_list_;
    std::unordered_map<void*, ReclaimNode*> reclaim_map_;
    ReclaimPool reclaim_pool_;
    HazardPointerList& global_hp_list_;
};

Reclaimer::~Reclaimer() {
    for (int i=0; i<hp_list_.size(); ++i) {
        assert(nullptr == hp_list_[i]->ptr.load(std::memory_order_relaxed));
        hp_list_[i]->flag.clear();
    }

    for (auto it = reclaim_map_.begin(); it != reclaim_map_.end();) {
        while (Hazard(it->first)) {
            std::this_thread::yield();
        }

        ReclaimNode* node = it->second;
        node->delete_func(node->ptr);
        delete node;
        it = reclaim_map_.erase(it);
    }
}

Reclaimer::HPIndex Reclaimer::MarkHazard(void* ptr) {
    if (nullptr == ptr) return HP_INDEX_NULL;

    for (int i=0; i<hp_list_.size(); ++i) {
        InternalHazardPointer* hp = hp_list_[i];
        if (nullptr == hp->ptr.load(std::memory_order_relaxed)) {
            hp->ptr.store(ptr, std::memory_order_release);
            return i;
        }

        TryAcquireHazardPointer();
        int index = hp_list_.size() - 1;
        hp_list_[index]->ptr.store(ptr, std::memory_order_release);
        return index;
    }
}

void Reclaimer::ReclaimNoHazardPointer() {
    if (reclaim_map_.size() < kCoefficient * global_hp_list_.get_size()) {
        return;
    }
    std::unordered_set<void*> not_allow_delete_set;
    std::atomic<InternalHazardPointer*>& head = global_hp_list_.head;
    InternalHazardPointer* p = head.load(std::memory_order_acquire);
    do {
        void* const ptr = p->ptr.load(std::memory_order_consume);
        if (nullptr != ptr) {
            not_allow_delete_set.insert(ptr);
        }
        p = p->next.load(std::memory_order_acquire);
    } while (p);

    for (auto it = reclaim_map_.begin(); it != reclaim_map_.end();) {
        if (not_allow_delete_set.find(it->first) == not_allow_delete_set.end()) {
            ReclaimNode* node = it->second;
            node->delete_func(node->ptr);
            reclaim_pool_.Push(node);
            it = reclaim_map_.erase(it);
        } else {
            ++it;
        }
    }
}

bool Reclaimer::Hazard(void* const ptr) {
    std::atomic<InternalHazardPointer*>& head = global_hp_list_.head;
    InternalHazardPointer*p = head.load(std::memory_order_acquire);
    do {
        if (p->ptr.load(std::memory_order_consume) == ptr) {
            return true;
        }
        p = p->next.load(std::memory_order_acquire);
    } while (p != nullptr);
    return false;
}

void Reclaimer::TryAcquireHazardPointer() {
    std::atomic<InternalHazardPointer*>& head = global_hp_list_.head;
    InternalHazardPointer* p = head.load(std::memory_order_acquire);
    InternalHazardPointer* hp = nullptr;
    do {
        if (!p->flag.test_and_set()) {
            hp = p;
            break;
        }
        p = p->next.load(std::memory_order_acquire);
    } while (p != nullptr);

    if (nullptr == hp) {
        InternalHazardPointer* new_head = new InternalHazardPointer();
        new_head->flag.test_and_set();
        hp = new_head;
        global_hp_list_.size.fetch_add(1, std::memory_order_release);
        InternalHazardPointer* old_head = head.load(std::memory_order_acquire);
        do {
            new_head->next = old_head;
        } while(!head.compare_exchange_weak(
            old_head, new_head,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }
    hp_list_.push_back(hp);
}


class HazardPointer {
public:
    HazardPointer() : reclaimer_(nullptr), index(Reclaimer::HP_INDEX_NULL) {}
    HazardPointer(Reclaimer* reclaimer, void* ptr)
        : reclaimer_(reclaimer)
        , index(reclaimer_->MarkHazard(ptr)) {}
    ~HazardPointer() { UnMark(); }

    HazardPointer(const HazardPointer& other) = delete;
    HazardPointer(HazardPointer&& other) {
      *this = std::move(other);
    }
  
    HazardPointer& operator=(const HazardPointer& other) = delete;
    HazardPointer& operator=(HazardPointer&& other) {
      reclaimer_ = other.reclaimer_;
      index = other.index;
  
      // If move assign is called, we must disable the other's index to avoid
      // incorrectly unmark index when other destruct.
      other.index = Reclaimer::HP_INDEX_NULL;
      return *this;
    }
  
    void UnMark() { reclaimer_->UnMarkHazard(index); }

    Reclaimer* reclaimer_;
    Reclaimer::HPIndex index;
};

}