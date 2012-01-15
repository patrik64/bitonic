#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Actor.h"

//information about sorting that will be passed as a message to an actor(s)
struct BitonicSortArgs {
	size_t m_startIndex;	//start index of container to sort
	size_t m_size;			//number of elements in container to sort
	bool m_directionUp;		//sort direction, true == up, false == down
	bool m_hybridSort;		//true == bitonic only, false == bitonic + std::sort
	int m_level;			//height of a binary three, i.e. 2^(m_level+1) - 1 == number of actors(threads) to create
};

//the actor class that will perform the sorting
//the act method will receive a message with instructions about sorting
//and depending on the value of the m_level it will either create two new actors
//and dispatch the work to them, or it will perform the sorting itself
class BitonicSortActor : public Actor {
public:
	void act() {
		while(true) { //loop
			//msg will contain the message from the queue
			boost::any msg;
			
			//the actor base class uses the message queue that is 
			//unlocked via conditional variable as soon
			//as there is something in it
			wait_and_dequeue(msg); //react

			try	{
				//message recieved and stored into boost::any variable msg
				if(msg.empty())
					continue;

				//now we have to investigate what is the message using its underlying type
				if(msg.type() == typeid(BitonicSortArgs)) {
					//the message contains the sorting instructions, performing a cast
					//and sorting the result into the res variable
					BitonicSortArgs res = boost::any_cast<BitonicSortArgs>(msg);
		
					if(res.m_size > 1) {
						//if the m_level is greater than 0 let this actor create two new ones
						//and let those two new actors perform the sort
						if(res.m_level > 0)	{
							size_t half = res.m_size/2;
							size_t half1 = half;
							size_t half2 = half;
							
							//in case there is an odd number of elements to sort
							if (res.m_size % 2 == 1)
								half1++;

							BitonicSortActor actor1;
							BitonicSortArgs s_args1;
							s_args1.m_startIndex = res.m_startIndex;
							s_args1.m_size = half1;
							s_args1.m_directionUp = res.m_directionUp;
							s_args1.m_hybridSort = res.m_hybridSort;
							s_args1.m_level = res.m_level - 1;
							
							actor1.m_cont.assign(m_cont.begin(), m_cont.begin() + half1);

							actor1.start();
							actor1.send(s_args1);
							actor1.send(std::string("EXIT"));

							BitonicSortActor actor2;
							BitonicSortArgs s_args2;
							s_args2.m_startIndex = res.m_startIndex;
							s_args2.m_size = half2;
							s_args2.m_directionUp = !res.m_directionUp;
							s_args2.m_hybridSort = res.m_hybridSort;
							s_args2.m_level = res.m_level - 1;
							
							actor2.m_cont.assign(m_cont.begin() + half1, m_cont.end());

							actor2.start();
							actor2.send(s_args2);
							actor2.send(std::string("EXIT"));

							actor1.end();
							actor2.end();

							m_cont.clear();

							//pick up the processed containers and join them
							m_cont.assign(actor2.m_cont.begin(), actor2.m_cont.end());
							m_cont.insert(m_cont.end(), actor1.m_cont.begin(), actor1.m_cont.end());
		
							//continue with bitonic merging
							bitonic_merge(0, res.m_size, res.m_directionUp, m_cont);
						} else {
							//m_level is zero, now either perform a hybrid sort using std::sort
							//or use the bitonic sort within this actor
							if(res.m_hybridSort) {
								if(res.m_directionUp)
									std::sort(m_cont.begin(), m_cont.end());
								else
									std::sort(m_cont.rbegin(), m_cont.rend());
							} else {
								bitonic_sort(res.m_startIndex, res.m_size, res.m_directionUp, m_cont);
							}
						}
					}
				}

				//if the message is of type std::string, investigate the value and if it is "EXIT"
				//exit the act method so that the thread can be joined and actor can be destroyed
				if(msg.type() == typeid(std::string)) {
					std::string res = boost::any_cast<std::string>(msg);
					if(res == std::string("EXIT")) {
						exit();
						return;
					}
				}
			}
			catch(const boost::bad_any_cast &) {
				assert("bad cast");	
			}
		}
	}

	//container with values to sort
	std::vector<int> m_cont;

private:

	//implementation of the bitonic sort
	void bitonic_compare(size_t i, size_t j, bool dir, std::vector<int> &vec) {
		if(dir == (vec[i] > vec[j]))
			std::swap(vec[i], vec[j]);
	}

	void bitonic_merge(size_t lo, size_t n, bool dir, std::vector<int> &vec) {
		if (n > 1) {
			int m = pp2(n);
			for (size_t i = lo; i < lo + n - m; i++)
				bitonic_compare(i, i+m, dir, vec);
			bitonic_merge(lo, m, dir, vec);
			bitonic_merge(lo+m, n-m, dir, vec);
		}
	}

	void bitonic_sort(size_t lo, size_t n, bool dir, std::vector<int> &vec)	{
		if(n > 1) {
			size_t m = n/2;
			bitonic_sort(lo, m, !dir, vec);
			bitonic_sort(lo+m, n-m, dir, vec);
			bitonic_merge(lo, n, dir, vec);
		}
	}

	//power of 2 that is less than n
	int pp2(size_t n) {
		size_t k = 1;
		while(k < n)
			k = k << 1;
		return k >> 1;
	}
};

//n == number of elements to sort
//level == height of a binary three, i.e. 2^(level+1) - 1 == number of actors(threads) to create
//hybrid == if true use bitonic sort with std::sort, if false use only bitonic sort 
void bitonic_sort_using_actors(int n, int level, bool hybrid = false) {
	std::cout << "*******************************************************************" << std::endl;
	
	if(hybrid)
		std::cout << "hybrid bitonic sort/std::sort using actors" << std::endl;
	else
		std::cout << "bitonic sort using actors" << std::endl;

	std::cout << "number of elements to sort: " << n << std::endl;
	std::cout << "level: " << level << std::endl;

	//create a new actor that will run the sorting operation
	BitonicSortActor actor;

	std::cout << std::endl << "filling the container and shuffling ..." << std::endl;
	
	//fill the container with decending values
	for (int i = 0; i < n; i++)	{
		actor.m_cont.push_back(n - i);
	}

	//shuffle the values in the container
	std::random_shuffle(actor.m_cont.begin(), actor.m_cont.end());

	//printout of unsorted values
	std::cout << std::endl << "input (before sort): first 5 elements" << std::endl;
	for (size_t i = 0; i < 5; i++) {
 		std::cout << actor.m_cont[i] << std::endl;
	}
	std::cout << std::endl << "input (before sort): last 5 elements" << std::endl;
	for (size_t i = 0; i < 5; i++) {
		std::cout << actor.m_cont[actor.m_cont.size() + i - 5] << std::endl;
	}

	//fill the sort instruction object that will be sent to the new actor
	BitonicSortArgs s_args;
	s_args.m_startIndex = 0;
	s_args.m_size = actor.m_cont.size();
	s_args.m_directionUp = true;
	s_args.m_hybridSort = hybrid;
	s_args.m_level = level;

	//start measuring the time
	auto start_time = boost::get_system_time();

	std::cout << std::endl << "sorting ..." << std::endl;

	//starting the actor (a new thread will be created now)
	actor.start();
	//sending the sort instrucions and performing the sort
	actor.send(s_args);
	//sending a string to actor that will deactivate the actor
	actor.send(std::string("EXIT"));

	//actor is not acting anymore, join the thread
	actor.end();

	auto end_time = boost::get_system_time();

	//printout of sorted values
	std::cout << std::endl << "sorted output: first 5 elements" << std::endl;
	for (size_t i = 0; i < 5; i++) {
 		std::cout << actor.m_cont[i] << std::endl;
	}
	std::cout << std::endl << "sorted output: last 5 elements" << std::endl;
	for (size_t i = 0; i < 5; i++) {
		std::cout << actor.m_cont[actor.m_cont.size() + i - 5] << std::endl;
	}

	std::cout << std::endl << "number of sorted elements: " << actor.m_cont.size() << std::endl;
	
	auto time_all = end_time - start_time;

	std::cout << "===================================================================" << std::endl;
	std::cout << "Time --> " << time_all.total_milliseconds() << " ms" << std::endl;
	std::cout << "===================================================================" << std::endl;
}

int main( int argc, char* argv[] ) {
	int n = 100000;

	//bitonic_sort_using_actors(n, 3); // using 15 actors, bitonic sort
	//bitonic_sort_using_actors(n, 2); // using 7 actors, bitonic sort
	//bitonic_sort_using_actors(n, 1); // using 3 actors, bitonic sort
	bitonic_sort_using_actors(n, 0); // using only 1 actor, bitonic sort
 
	//bitonic_sort_using_actors(n, 3, true); // using 15 actors, bitonic + std::sort

	return 0;
}
