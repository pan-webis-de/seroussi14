/*
 * Counter.h
 *
 *  Created on: Sep 17, 2015
 *      Author: thomas
 */

#ifndef COUNTER_H_
#define COUNTER_H_

#include <map>
#include <iostream>
#include "hdf5.h"
#include "hdf5_hl.h"

class Dynamic_Counter {
private:
	// Map to store entries and counts.
	std::map<int,int> entries;
	// Variable to cache sum.
	int count_sum;

public:
	Dynamic_Counter();
	virtual ~Dynamic_Counter();
	void inc(int ind);
	void dec(int ind);
	int at(int ind);
	int sum();
	void save(hid_t loc_id, std::string name);
	void load(hid_t loc_id, std::string name);
	void print();
};

class Static_Counter {
private:
	// Map to store entries and counts.
	int* entries;
	int length;
	// Variable to cache sum.
	int count_sum;

public:
	Static_Counter(int length);
	virtual ~Static_Counter();
	void inc(int ind);
	void dec(int ind);
	int at(int ind);
	int sum();
	void save(hid_t loc_id, std::string name);
	void load(hid_t loc_id, std::string name);
	void print();
};


#endif /* COUNTER_H_ */
