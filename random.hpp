#ifndef random_hpp
#define random_hpp

/*! \file
    \brief Functions for returning various distributions of random numbers
*/

#include <boost/random.hpp>
#include <vector>
using std::vector;

//! Can be any of the generators described at http://www.boost.org/libs/random/random-generators.html
typedef boost::mt19937 OurGenerator;

//! Seed the random number generator, must be called before any of the other functions
void RandomSeed(OurGenerator::result_type seed);

//! Return uniformly distributed int's in [0..limit)
int RandomInt(int limit);

//! 80% of the time return uniformly distributed ints in [0..limit/5), otherwise return in [limit/5..limit)
int RandomInt8020(int limit);

//! Randomly choose between two consecutive integers floor(avg) and ceil(avg) so the average is avg
int RandomConsecutive(double avg);

//! Returns a random integer in [0..cf.size()) distributed by the cummulative probabilities given in cf
int RandomDiscrete(vector<double> const & cf);

#endif random_hpp
