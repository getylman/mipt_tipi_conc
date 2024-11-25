#include <unistd.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

class spin_lock_TAS {
 public:
  spin_lock_TAS() : m_spin(0) {}
  ~spin_lock_TAS() { assert(m_spin.load(std::memory_order_relaxed) == 0); }

  void lock() {
    uint32_t expected;
    const int kYieldConst = 5;

    for (int i = 0; i < kYieldConst; ++i) {
      for (int j = 0; j <= i; ++j) {
        expected = 0;
        if (m_spin.compare_exchange_weak(expected, 1,
                                         std::memory_order_acquire)) {
          return;
        }
      }
      std::this_thread::yield();
    }

    uint32_t sleep_time = 100;  // microseconds

    do {
      sleep_time <<= 1;
      int32_t jitter = rand() % 20 - 10;

      std::this_thread::sleep_for(
          std::chrono::microseconds(sleep_time + jitter));

      expected = 0;
    } while (
        !m_spin.compare_exchange_weak(expected, 1, std::memory_order_acquire));
  }

  void unlock() { m_spin.store(0, std::memory_order_release); }

 private:
  std::atomic<uint32_t> m_spin;
};

class spin_lock_TTAS {
 public:
  spin_lock_TTAS() : m_spin(0) {}
  ~spin_lock_TTAS() { assert(m_spin.load(std::memory_order_relaxed) == 0); }

  void lock() {
    uint32_t expected;
    const int kYieldConst = 5;

    for (int i = 0; i < kYieldConst; ++i) {
      for (int j = 0; j <= i; ++j) {
        expected = 0;
        while (m_spin.load(std::memory_order_relaxed)) {
          __asm volatile("pause" ::: "memory");
        }
        if (m_spin.compare_exchange_weak(expected, 1,
                                         std::memory_order_acquire)) {
          return;
        }
      }
      std::this_thread::yield();
    }

    uint32_t sleep_time = 100;  // microseconds

    do {
      sleep_time <<= 1;
      int32_t jitter = rand() % 20 - 10;

      std::this_thread::sleep_for(
          std::chrono::microseconds(sleep_time + jitter));

      while (m_spin.load(std::memory_order_relaxed)) {
        __asm volatile("pause" ::: "memory");
      }

      expected = 0;
    } while (
        !m_spin.compare_exchange_weak(expected, 1, std::memory_order_acquire));
  }

  void unlock() { m_spin.store(0, std::memory_order_release); }

 private:
  std::atomic<uint32_t> m_spin;
};

class ticket_lock {
 public:
  void lock() {
    const int kNopConst = 5;
    const auto ticket = next_ticket.fetch_add(1, std::memory_order_relaxed);
    for (int i = 0; i < kNopConst; ++i) {
      if (now_serving.load(std::memory_order_acquire) == ticket) {
        return;
      }
      __asm__ volatile("nop");
    }
    for (int i = 0; i < kNopConst; ++i) {
      if (now_serving.load(std::memory_order_acquire) == ticket) {
        return;
      }
      std::this_thread::yield();
    }
    size_t sleep_time = 100;  // microseconds
    while (now_serving.load(std::memory_order_acquire) != ticket) {
      sleep_time <<= 1;

      int32_t jitter = rand() % 20 - 10;
      std::this_thread::sleep_for(
          std::chrono::microseconds(sleep_time + jitter));
    }
  }

  void unlock() {
    const auto successor = now_serving.load(std::memory_order_relaxed) + 1;
    now_serving.store(successor, std::memory_order_release);
  }

 private:
  std::atomic_size_t now_serving{0};
  std::atomic_size_t next_ticket{0};
};

template <typename Spinlock>
std::vector<std::chrono::nanoseconds> get_time_data(int num_of_threads) {
  std::vector<std::chrono::nanoseconds> time_data;
  time_data.reserve(num_of_threads);

  Spinlock spin_lock;

  auto thread_func = [&spin_lock, &time_data]() {
    auto start = std::chrono::high_resolution_clock::now();

    spin_lock.lock();

    auto stop = std::chrono::high_resolution_clock::now();

    spin_lock.unlock();

    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    time_data.push_back(duration);
  };

  std::vector<std::thread> threads;
  threads.reserve(num_of_threads);

  for (int i = 0; i < num_of_threads; ++i) {
    threads.emplace_back(thread_func);
  }

  for (int i = 0; i < num_of_threads; ++i) {
    threads[i].join();
  }

  return time_data;
}

int main() {
  std::cout << "TAS:\n";
  for (int i = 1; i < 9; ++i) {
    const auto time_data = get_time_data<spin_lock_TAS>(i);

    for (const auto& item : time_data) {
      std::cout << item.count() << ' ';
    }
    std::cout << '\n';
    sleep(2);
  }

  std::cout << "TTAS:\n";
  for (int i = 1; i < 9; ++i) {
    const auto time_data = get_time_data<spin_lock_TTAS>(i);

    for (const auto& item : time_data) {
      std::cout << item.count() << ' ';
    }
    std::cout << '\n';
    sleep(2);
  }

  std::cout << "Ticket lock:\n";
  for (int i = 1; i < 9; ++i) {
    const auto time_data = get_time_data<ticket_lock>(i);

    for (const auto& item : time_data) {
      std::cout << item.count() << ' ';
    }
    std::cout << '\n';
    sleep(2);
  }
  return 0;
}