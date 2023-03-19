#include <iostream>
#include <memory>
#include <vector>

#include <chrono>
#include <cassert>
#include <random>

#include <thread>
#include <mutex>
#include <future> 
#include <atomic>
/* std::promise - used to set the value that a thread will return */
/* std::future  - used to retrieve the value from the thread */
/* std::atomic  - used to perform atomic operations on shared variables without explicit locking */

using namespace std;

#define u32 uint32_t
#define u64 uint64_t

u32 main(u32 argc, char* argv[]) {

	/* Configure the random generator */
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distr(1, 10);

	/* Create and fill the matrix with random numbers*/
	const u32 ROWS = 4092;
	const u32 COLS = 4092;

	u32* matrix = new u32[ROWS * COLS];
	for (u32 i = 0; i < ROWS; ++i) {
		for (u32 j = 0; j < COLS; ++j) {
			matrix[i * COLS + j] = distr(gen);
		}
	}

	/* Set the multithreaded parallel environment */
	mutex m;
	promise<u64> p;
	future<u64> f = p.get_future();
	atomic<u64> total_sum(0);

	auto lamb = [&](u32 start_row, u32 end_row) {
		
		u64 thread_sum = 0;
		for (u32 i = start_row; i < end_row; ++i) {

			u64 row_sum = 0;
			for (u32 j = 0; j < COLS; ++j)
				row_sum += matrix[i * COLS + j];	

			thread_sum += row_sum;
		}
		
		total_sum.fetch_add(thread_sum);
		cout << "Thread ID: " << this_thread::get_id() << ", summed: " << thread_sum << "\n";
	};
	
	/* Create and dispatch the worker threads */
	std::vector<std::thread> threads;
	const u32 num_threads = 12;
	const u32 rows_per_thread = ROWS / num_threads;
	for (u32 i = 0; i < 12; ++i) {

		u32 start_row = i * rows_per_thread;
		u32 end_row = (i + 1) * rows_per_thread;

		if (i == num_threads - 1) {
			end_row = ROWS;
		}

		threads.emplace_back(std::thread(lamb, start_row, end_row));
	}
	
	/* Wait for all threads to finish */
	u32 threads_joined = 0;
	while (threads_joined < num_threads) {
		for (auto& t : threads) {
			if (!t.joinable())
				continue;
			else {
				t.join();
				++threads_joined;
			}
		}
	}

	/* Get the value from the atomic variable via the promise and display it via the future */
	p.set_value(total_sum);
	std::cout << "Total matrix sum: " << f.get();

	return EXIT_SUCCESS;
}	

