#ifndef PCERL_STATISTICS_H
#define PCERL_STATISTICS_H

#include <vector>
#include <map>
#include <mutex>

namespace SimulationErrorLocator {

struct StatisticsOutput {
	float min, max;
	float mean, dev;
	float sum;
};

class Statistics {
	std::vector<float> mContainer;

public:
	Statistics();
	~Statistics();

	void insert(float value);
	void clear();

	StatisticsOutput evaluate();

	void printContents();

	std::vector<float> valueList(){ return mContainer;}
};

}

#endif
