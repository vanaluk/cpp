#define BOOST_TEST_MODULE Task1Tests
#include <boost/test/unit_test.hpp>
#include <thread>
#include <vector>
#include "task1_weak_ptr/custom_weak_ptr.hpp"

BOOST_AUTO_TEST_SUITE(Task1TestSuite)

BOOST_AUTO_TEST_CASE(test_shared_ptr_creation) {
    CustomSharedPtr<int> ptr(new int(42));
    BOOST_CHECK_EQUAL(ptr.use_count(), 1);
    BOOST_CHECK(ptr.get() != nullptr);
    BOOST_CHECK_EQUAL(*ptr, 42);
}

BOOST_AUTO_TEST_CASE(test_shared_ptr_copy) {
    CustomSharedPtr<int> ptr1(new int(42));
    BOOST_CHECK_EQUAL(ptr1.use_count(), 1);
    
    {
        CustomSharedPtr<int> ptr2(ptr1);
        BOOST_CHECK_EQUAL(ptr1.use_count(), 2);
        BOOST_CHECK_EQUAL(ptr2.use_count(), 2);
        BOOST_CHECK_EQUAL(*ptr1, 42);
        BOOST_CHECK_EQUAL(*ptr2, 42);
    }
    
    BOOST_CHECK_EQUAL(ptr1.use_count(), 1);
}

BOOST_AUTO_TEST_CASE(test_shared_ptr_move) {
    CustomSharedPtr<int> ptr1(new int(42));
    BOOST_CHECK_EQUAL(ptr1.use_count(), 1);
    
    CustomSharedPtr<int> ptr2(std::move(ptr1));
    BOOST_CHECK(!ptr1);  // moved-from should be null
    BOOST_CHECK_EQUAL(ptr1.use_count(), 0);
    BOOST_CHECK(ptr2.get() != nullptr);
    BOOST_CHECK_EQUAL(*ptr2, 42);
    BOOST_CHECK_EQUAL(ptr2.use_count(), 1);
}

BOOST_AUTO_TEST_CASE(test_weak_ptr_from_shared) {
    CustomSharedPtr<int> shared(new int(42));
    CustomWeakPtr<int> weak(shared);
    
    BOOST_CHECK(!weak.expired());
    BOOST_CHECK_EQUAL(weak.use_count(), 1);
}

BOOST_AUTO_TEST_CASE(test_lock_returns_valid_ptr) {
    CustomSharedPtr<int> shared(new int(42));
    CustomWeakPtr<int> weak(shared);
    
    BOOST_CHECK_EQUAL(shared.use_count(), 1);
    
    CustomSharedPtr<int> locked = weak.lock();
    BOOST_CHECK(locked);  // Should be valid
    BOOST_CHECK_EQUAL(*locked, 42);
    BOOST_CHECK_EQUAL(shared.use_count(), 2);
    BOOST_CHECK_EQUAL(locked.use_count(), 2);
}

BOOST_AUTO_TEST_CASE(test_lock_returns_null_after_destruction) {
    CustomWeakPtr<int> weak;
    {
        CustomSharedPtr<int> shared(new int(42));
        weak = CustomWeakPtr<int>(shared);
        BOOST_CHECK_EQUAL(weak.use_count(), 1);
        BOOST_CHECK(!weak.expired());
    }
    // shared is destroyed here
    
    BOOST_CHECK(weak.expired());
    CustomSharedPtr<int> locked = weak.lock();
    BOOST_CHECK(!locked);  // Should be null
}

BOOST_AUTO_TEST_CASE(test_multiple_weak_ptrs) {
    CustomWeakPtr<int> weak1;
    CustomWeakPtr<int> weak2;
    CustomWeakPtr<int> weak3;
    
    {
        CustomSharedPtr<int> shared(new int(42));
        weak1 = CustomWeakPtr<int>(shared);
        weak2 = CustomWeakPtr<int>(shared);
        weak3 = CustomWeakPtr<int>(shared);
        
        BOOST_CHECK_EQUAL(shared.use_count(), 1);
        
        // All weak_ptrs should be able to lock
        CustomSharedPtr<int> locked1 = weak1.lock();
        CustomSharedPtr<int> locked2 = weak2.lock();
        CustomSharedPtr<int> locked3 = weak3.lock();
        
        BOOST_CHECK(locked1);
        BOOST_CHECK(locked2);
        BOOST_CHECK(locked3);
        BOOST_CHECK_EQUAL(*locked1, 42);
        BOOST_CHECK_EQUAL(*locked2, 42);
        BOOST_CHECK_EQUAL(*locked3, 42);
        BOOST_CHECK_EQUAL(shared.use_count(), 4);
    }
    // shared is destroyed here, but locked pointers still exist
    
    // All weak_ptrs should be expired
    BOOST_CHECK(weak1.expired());
    BOOST_CHECK(weak2.expired());
    BOOST_CHECK(weak3.expired());
    
    // All locks should return null
    BOOST_CHECK(!weak1.lock());
    BOOST_CHECK(!weak2.lock());
    BOOST_CHECK(!weak3.lock());
}

BOOST_AUTO_TEST_CASE(test_multithread_lock) {
    auto shared = CustomSharedPtr<int>(new int(42));
    CustomWeakPtr<int> weak(shared);
    
    const int num_threads = 4;
    const int iterations = 1000;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&weak, iterations]() {
            for (int i = 0; i < iterations; ++i) {
                auto locked = weak.lock();
                if (locked) {
                    BOOST_CHECK_EQUAL(*locked, 42);
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // After all threads complete, use_count should be 1 (only 'shared')
    BOOST_CHECK_EQUAL(shared.use_count(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
