/*
 * PerfTimer.cpp:
 *  Provides timing information on a per-method basis
 */

#include "PerfTimer.h"


void MethodPerfProfile::begin_timer() {
	timer.start();
}

void MethodPerfProfile::end_timer() {
	total_calls++;
	total_microseconds += timer.get_microseconds();
}

void PerfTimer::begin(const char* method) {
	perf_map[method].begin_timer();
}

void PerfTimer::end(const char* method) {
	perf_map[method].end_timer();
}

void PerfTimer::print_results() {
	printf("**** START PERFORMANCE STATS ****\n");
	MethodPerfProfileMap::iterator it = perf_map.begin();
	for (; it != perf_map.end(); ++it) {
		MethodPerfProfile& mpp = it->second;
		float total = mpp.total_microseconds / 1000.0f;
		float avg = total / mpp.total_calls;
		printf("%s:\n\tAVG %.4fms\tTOTAL\t %.4fms\tCALLS\t %d\n", it->first,
				avg, total, mpp.total_calls);
	}
	printf("**** END PERFORMANCE STATS ****\n");
}

double PerfTimer::average_time(const char* method) {
	MethodPerfProfile& mpp = perf_map[method];
	float total = mpp.total_microseconds / 1000.0f;
	return total / mpp.total_calls;
}

void PerfTimer::clear() {
	perf_map.clear();
}


static PerfTimer __global_timer;

void perf_timer_begin(const char* funcname) {
	__global_timer.begin(funcname);
}

void perf_timer_clear() {
	__global_timer.clear();
}

void perf_timer_end(const char* funcname) {
	__global_timer.end(funcname);
}

double perf_timer_average_time(const char* funcname) {
	return __global_timer.average_time(funcname);
}

void perf_print_results() {
	__global_timer.print_results();
}


