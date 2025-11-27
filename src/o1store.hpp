#pragma once
//
// implements a O(1) store of objects
//

// template parameters:
// * 'type' is object type. 'type' must contain public field 'type **alloc_ptr'
// * 'size' is number of preallocated objects
// * 'store_id' is used for debugging
// * 'instance_size_B' is custom size of instance to fit largest object in an
//   object hierarchy or 0 if 'type' sizeof is used
//
// note: no destructor since life-time is program life-time
//

// reviewed: 2024-05-01
// reviewed: 2024-05-22
// reviewed: 2025-11-27

#include <cstdio>
#include <cstdlib>

template <typename type, int32_t const size, int32_t const store_id = 0,
          int32_t const instance_size_B = 0>
class o1store {
    type* all_{};
    type** free_bgn_{};
    type** free_ptr_{};
    type** free_end_{};
    type** alloc_bgn_{};
    type** alloc_ptr_{};
    type** del_bgn_{};
    type** del_ptr_{};
    type** del_end_{};

  public:
    o1store() {
        all_ = static_cast<type*>(
            calloc(size, instance_size_B ? instance_size_B : sizeof(type)));
        free_ptr_ = free_bgn_ =
            static_cast<type**>(calloc(size, sizeof(type*)));
        alloc_ptr_ = alloc_bgn_ =
            static_cast<type**>(calloc(size, sizeof(type*)));
        del_ptr_ = del_bgn_ = static_cast<type**>(calloc(size, sizeof(type*)));

        if (!all_ || !free_bgn_ || !alloc_bgn_ || !del_bgn_) {
            printf("!!! o1store %d: could not allocate arrays\n", store_id);
            exit(1);
        }

        free_end_ = free_bgn_ + size;
        del_end_ = del_bgn_ + size;

        // write pointers to instances in the 'free' list
        type* all_it = all_;
        for (type** free_it = free_bgn_; free_it < free_end_; ++free_it) {
            *free_it = all_it;
            if (instance_size_B) {
                all_it = reinterpret_cast<type*>(
                    reinterpret_cast<char*>(all_it) + instance_size_B);
            } else {
                ++all_it;
            }
        }
    }

    // allocates an instance
    // returns nullptr if instance could not be allocated
    auto alloc() -> type* {
        if (free_ptr_ >= free_end_) {
            return nullptr;
        }
        type* inst = *free_ptr_;
        ++free_ptr_;
        *alloc_ptr_ = inst;
        inst->alloc_ptr = alloc_ptr_;
        // note: needs compiler flag -flifetime-dse=1 for inst->alloc_ptr to be
        //       written when inlined
        //       see: https://github.com/espressif/crosstool-NG/issues/55
        ++alloc_ptr_;
        return inst;
    }

    // adds instance to list of instances to be freed with 'apply_free()'
    auto free(type* inst) -> void {
        if (del_ptr_ >= del_end_) {
            printf("!!! o1store %d: free overrun\n", store_id);
            exit(1);
        }
        *del_ptr_ = inst;
        ++del_ptr_;
    }

    // deallocates the instances that have been freed
    auto apply_free() -> void {
        for (type** it = del_bgn_; it < del_ptr_; ++it) {
            type* inst_deleted = *it;
            --alloc_ptr_;
            type* inst_to_move = *alloc_ptr_;
            inst_to_move->alloc_ptr = inst_deleted->alloc_ptr;
            *(inst_deleted->alloc_ptr) = inst_to_move;
            --free_ptr_;
            *free_ptr_ = inst_deleted;
        }
        del_ptr_ = del_bgn_;
    }

    // returns list of allocated instances
    inline auto allocated_list() const -> type** { return alloc_bgn_; }

    // returns length of list of allocated instances
    inline auto allocated_list_len() const -> int32_t {
        return int32_t(alloc_ptr_ - alloc_bgn_);
    }

    // returns one past the end of allocated instances list
    inline auto allocated_list_end() const -> type** { return alloc_ptr_; }

    // returns the list with all preallocated instances
    inline auto all_list() const -> type* { return all_; }

    // returns the length of 'all' list
    auto constexpr all_list_len() const -> int32_t { return size; }

    // returns instance at index 'ix' from 'all' list
    inline auto instance(int32_t ix) const -> type* {
        if (!instance_size_B) {
            return &all_[ix];
        }
        // note: if instance size is specified do pointer shenanigans
        return reinterpret_cast<type*>(reinterpret_cast<char*>(all_) +
                                       instance_size_B * ix);
    }

    // returns the size of allocated heap memory in bytes
    auto constexpr allocated_data_size_B() const -> int32_t {
        return instance_size_B
                   ? (size * instance_size_B + 3 * size * sizeof(type*))
                   : (size * sizeof(type) + 3 * size * sizeof(type*));
    }
};
