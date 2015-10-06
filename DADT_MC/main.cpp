/*
 * main.cpp
 *
 *  Created on: Sep 18, 2015
 *      Author: thomas
 */

#include "MIChain.h"
#include "AChain.h"
#include "Classifier.h"

int main() {

	MIChain* mc = new MIChain("/Users/thomas/Documents/workspace/DADT_MC/FridayRun/PAN11FridayRun.h5", 0);
	mc->iterate(1);
	mc->save_sample();
//	mc->iterate(1000);
//	mc->save_sample();
//	for (int i=0; i<8; i++) {
//		mc->iterate(100);
//		mc->save_sample();
//	}

//	Classifier cl("/Users/thomas/Documents/workspace/DADT_MC/FridayRun/PAN11FridayRun.h5");
//	cl.attribution_testset(0,1000);


//	cl.model_chain->load_sample(0, 1800);
//	cl.model_chain->calc_distributions();
//
////	cl.model_chain->phi_tw_D[0]->print_topN(10);

//	Classifier cl("/Users/thomas/Documents/workspace/DADT_MC/FridayRun/PAN11FridayRun.h5");
//	cl.model_chain->c_at_AT[52]->print();
//	for (int i=0; i<72; i++) {
//		std::cout << "Author " << i << " has sum c_at_AT = " << cl.model_chain->c_at_AT[i]->sum() << std::endl;
//	}
//
//	std::cout << "DONE" << std::endl;

//	for (int i=0; i<cl.model_chain->A; i++) {
//		std::cout << "Author " << i << " sum of theta " << cl.model_chain->theta_at_A[i]->sum() << std::endl;
//	}
//
//	int N = 100;
//	int* words = new int[100];
//	for (int i=0; i<N; i++) words[i] = 2*i;
//
//	AChain* ac = new AChain(N,words,cl.model_chain);
//	std::cout << ac->get_most_probable_author(100,100) << std::endl;

}
