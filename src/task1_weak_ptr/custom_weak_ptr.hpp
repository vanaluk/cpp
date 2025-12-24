#pragma once

#include <atomic>

// Function declarations for demonstration and benchmarks
void demonstrate_weak_ptr_lock();
long long benchmark_weak_ptr_lock(int iterations, int thread_count);

/**
 * Custom implementation of weak_ptr demonstrating lock() operation
 *
 * Implementation shows how reference counting works in weak_ptr::lock():
 * 1. Uses atomic counter for thread safety
 * 2. lock() checks that the object is still alive (use_count > 0)
 * 3. If object is alive, increases use_count and returns shared_ptr
 * 4. If object is dead, returns nullptr
 */
template<typename T>
class CustomWeakPtr;

template<typename T>
class CustomSharedPtr {
public:
	explicit CustomSharedPtr(T* ptr= nullptr) :
		ptr_(ptr),
		ref_count_(new std::atomic<int>(1)),
		weak_count_(new std::atomic<int>(0)) {
	}

	// Internal constructor for lock() - ref_count and weak_count are related counters
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	CustomSharedPtr(T* ptr, std::atomic<int>* ref_count, std::atomic<int>* weak_count) :
		ptr_(ptr),
		ref_count_(ref_count),
		weak_count_(weak_count) {
		// ref_count already incremented in lock(), do nothing
	}

	~CustomSharedPtr() {
		if(ptr_ != nullptr && ref_count_ != nullptr) {
			// Decrease reference counter
			int count= --(*ref_count_);
			if(count == 0) {
				delete ptr_;
				ptr_= nullptr;

				// If no weak_ptr, delete counters
				if(weak_count_->load() == 0) {
					delete ref_count_;
					delete weak_count_;
				}
			}
		}
	}

	// Copy constructor
	CustomSharedPtr(const CustomSharedPtr& other) :
		ptr_(other.ptr_),
		ref_count_(other.ref_count_),
		weak_count_(other.weak_count_) {
		if(ref_count_ != nullptr) {
			++(*ref_count_);
		}
	}

	CustomSharedPtr& operator=(const CustomSharedPtr& other) {
		if(this != &other) {
			// Decrease old counter
			if(ref_count_ != nullptr) {
				int count= --(*ref_count_);
				if(count == 0) {
					delete ptr_;
					if(weak_count_->load() == 0) {
						delete ref_count_;
						delete weak_count_;
					}
				}
			}

			// Copy new values
			ptr_= other.ptr_;
			ref_count_= other.ref_count_;
			weak_count_= other.weak_count_;

			if(ref_count_ != nullptr) {
				++(*ref_count_);
			}
		}
		return *this;
	}

	// Move constructor
	CustomSharedPtr(CustomSharedPtr&& other) noexcept
		:
		ptr_(other.ptr_),
		ref_count_(other.ref_count_),
		weak_count_(other.weak_count_) {
		other.ptr_= nullptr;
		other.ref_count_= nullptr;
		other.weak_count_= nullptr;
	}

	T* get() const {
		return ptr_;
	}
	T& operator*() const {
		return *ptr_;
	}
	T* operator->() const {
		return ptr_;
	}
	int use_count() const {
		return ref_count_ != nullptr ? ref_count_->load() : 0;
	}
	bool expired() const {
		return use_count() == 0;
	}

	explicit operator bool() const noexcept {
		return ptr_ != nullptr;
	}

	bool operator!() const noexcept {
		return ptr_ == nullptr;
	}

private:
	T* ptr_;
	std::atomic<int>* ref_count_;
	std::atomic<int>* weak_count_;

	friend class CustomWeakPtr<T>;
};

template<typename T>
class CustomWeakPtr {
public:
	CustomWeakPtr() :
		ptr_(nullptr),
		ref_count_(nullptr),
		weak_count_(nullptr) {}

	explicit CustomWeakPtr(const CustomSharedPtr<T>& shared) :
		ptr_(shared.ptr_),
		ref_count_(shared.ref_count_),
		weak_count_(shared.weak_count_) {
		if(weak_count_ != nullptr) {
			++(*weak_count_);
		}
	}

	~CustomWeakPtr() {
		if(weak_count_ != nullptr) {
			int count= --(*weak_count_);
			if(count == 0 && ref_count_ != nullptr && ref_count_->load() == 0) {
				// No more shared_ptr and weak_ptr - can delete counters
				delete ref_count_;
				delete weak_count_;
			}
		}
	}

	// Copy constructor
	CustomWeakPtr(const CustomWeakPtr& other) :
		ptr_(other.ptr_),
		ref_count_(other.ref_count_),
		weak_count_(other.weak_count_) {
		if(weak_count_ != nullptr) {
			++(*weak_count_);
		}
	}

	CustomWeakPtr& operator=(const CustomWeakPtr& other) {
		if(this != &other) {
			// Decrease old weak_count
			if(weak_count_ != nullptr) {
				int count= --(*weak_count_);
				if(count == 0 && ref_count_ != nullptr && ref_count_->load() == 0) {
					delete ref_count_;
					delete weak_count_;
				}
			}

			ptr_= other.ptr_;
			ref_count_= other.ref_count_;
			weak_count_= other.weak_count_;

			if(weak_count_ != nullptr) {
				++(*weak_count_);
			}
		}
		return *this;
	}

	/**
	 * Main method - lock()
	 *
	 * Algorithm:
	 * 1. Check that ref_count_ exists and is not zero
	 * 2. Try to atomically increase ref_count_ (if it > 0)
	 * 3. If successful - create and return CustomSharedPtr
	 * 4. If object is already deleted - return nullptr
	 */
	CustomSharedPtr<T> lock() const {
		if(ref_count_ == nullptr || weak_count_ == nullptr) {
			return CustomSharedPtr<T>();
		}

		// Atomically check and increase reference counter
		int expected= ref_count_->load();
		while(expected > 0) {
			// Try to atomically increase counter
			if(ref_count_->compare_exchange_weak(expected, expected + 1)) {
				// Successfully increased - object is alive, create shared_ptr
				return CustomSharedPtr<T>(ptr_, ref_count_, weak_count_);
			}
			// compare_exchange_weak may have changed expected, retry
			expected= ref_count_->load();
		}

		// Object is already deleted
		return CustomSharedPtr<T>();
	}

	bool expired() const {
		return ref_count_ == nullptr || ref_count_->load() == 0;
	}

	int use_count() const {
		return ref_count_ != nullptr ? ref_count_->load() : 0;
	}

private:
	T* ptr_;
	std::atomic<int>* ref_count_;
	std::atomic<int>* weak_count_;
};
