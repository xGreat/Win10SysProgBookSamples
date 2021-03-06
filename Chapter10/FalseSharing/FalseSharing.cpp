#include "stdafx.h"

using namespace std;

struct ThreadData {
	long long start, end;
	const int* data;
	long long* counters;
};

long long CountEvenNumbers1(const int* data, long long size, int nthreads) {
	auto counters_buffer = make_unique<long long[]>(nthreads);
	auto counters = counters_buffer.get();
	auto tdata = make_unique<ThreadData[]>(nthreads);

	long long chunk = size / nthreads;
	vector<wil::unique_handle> threads;
	vector<HANDLE> handles;

	for (int i = 0; i < nthreads; i++) {
		long long start = i * chunk;
		long long end = i == nthreads - 1 ? size : ((long long)i + 1) * chunk;
		auto& d = tdata[i];
		d.start = start;
		d.end = end;
		d.counters = counters + i;
		d.data = data;

		wil::unique_handle hThread(::CreateThread(nullptr, 0, [](auto param) -> DWORD {
			auto d = (ThreadData*)param;
			auto start = d->start, end = d->end;
			auto counters = d->counters;
			auto data = d->data;

			for (; start < end; ++start)
				if (data[start] % 2 == 0)
					++(*counters);
			return 0;
			}, tdata.get() + i, 0, nullptr));

		handles.push_back(hThread.get());
		threads.push_back(move(hThread));
	}

	::WaitForMultipleObjects(nthreads, handles.data(), TRUE, INFINITE);

	long long sum = 0;
	for (int i = 0; i < nthreads; i++)
		sum += counters[i];
	return sum;
}

long long CountEvenNumbers2(const int* data, long long size, int nthreads) {
	auto counters_buffer = make_unique<long long[]>(nthreads);
	auto counters = counters_buffer.get();
	auto tdata = make_unique<ThreadData[]>(nthreads);

	long long chunk = size / nthreads;
	vector<wil::unique_handle> threads;
	vector<HANDLE> handles;

	for (int i = 0; i < nthreads; i++) {
		long long start = i * chunk;
		long long end = i == nthreads - 1 ? size : ((long long)i + 1) * chunk;
		auto& d = tdata[i];
		d.start = start;
		d.end = end;
		d.counters = counters + i;
		d.data = data;

		wil::unique_handle hThread(::CreateThread(nullptr, 0, [](auto param) -> DWORD {
			auto d = (ThreadData*)param;
			auto start = d->start, end = d->end;
			auto data = d->data;
			size_t count = 0;

			for (; start < end; ++start)
				if (data[start] % 2 == 0)
					count++;
			*(d->counters) = count;
			return 0;
			}, tdata.get() + i, 0, nullptr));

		handles.push_back(hThread.get());
		threads.push_back(move(hThread));
	}

	::WaitForMultipleObjects(nthreads, handles.data(), TRUE, INFINITE);

	long long sum = 0;
	for (int i = 0; i < nthreads; i++)
		sum += counters[i];
	return sum;
}

int main() {
	const long long size = (1LL << 32) - 1;	// just a large number
	cout << "Initializing data..." << endl;

	auto data = make_unique<int[]>(size);
	for (long long i = 0; i < size; i++)
		data[i] = (unsigned)i + 1;

	auto processors = min(8, ::GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));

	cout << "Option 1" << endl;
	for (WORD i = 1; i <= processors; ++i) {
		auto start = ::GetTickCount64();
		auto count = CountEvenNumbers1(data.get(), size, i);
		auto end = ::GetTickCount64();
		auto duration = end - start;
		cout << setw(2) << i << " threads " << "count: " << count << " time: " << duration << " msec" << endl;
	}

	cout << endl << "Option 2" << endl;
	for (WORD i = 1; i <= processors; ++i) {
		auto start = ::GetTickCount64();
		auto count = CountEvenNumbers2(data.get(), size, i);
		auto end = ::GetTickCount64();
		auto duration = end - start;
		cout << setw(2) << i << " threads " << "count: " << count << " time: " << duration << " msec" << endl;
	}
	return 0;
}

