/*! \file
    \brief Definitions of functions declared in random.hpp
*/

#include "random.hpp"

#include <boost/random.hpp>
#include <algorithm>
#include <assert.h>
using std::lower_bound;

typedef boost::uniform_01<OurGenerator, double> Our01;

static Our01 * r = 0;

int RandomInt(int limit)
{
  return (int)((*r)() * limit);
}

int RandomInt8020(int limit)
{
  int cut = limit / 5;
  if (RandomInt(5) == 0)
    return RandomInt(limit - cut) + cut;
  else
    return RandomInt(cut);
}

/*! If floor(avg) == ceil(avg) then that number will be returned 100% of the time. */
int RandomConsecutive(double avg)
{
  double fl = floor(avg);
  if ((*r)() < avg - fl)
    return (int)ceil(avg);
  else
    return (int)fl;
}

int RandomDiscrete(vector<double> const & cf)
{
  double d = (*r)();
  assert(d <= cf[cf.size() - 1]);
  return lower_bound(cf.begin(), cf.end(), d) - cf.begin();
}

void RandomSeed(OurGenerator::result_type seed)
{
  static OurGenerator generator(seed);
  static Our01 rnd(generator);
  r = &rnd;
}