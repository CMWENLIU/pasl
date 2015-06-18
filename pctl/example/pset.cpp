/*!
 * \file max.cpp
 * \brief Benchmarking script for parallel sorting algorithms
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "pasl.hpp"
#include "io.hpp"
#include "pset.hpp"
#include "pmap.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    
    void ex() {
      {
        pchunkedseq<int> xs = { 3, 1, 333, 2, 0, 123, 9, 8, 3 };
        xs.keep_if([&] (int x) {
          return x%2 == 0;
        });
        std::cout << "xs = " << xs << std::endl;
        return;
      }
      {
        pmap<int, bool> map;
        bool& b = map[3];
        map[123] = true;
        map[-1] = false;
        std::cout << "mp = " << map << std::endl;
        std::cout << "b = " << b << std::endl;
        return;
      }
      
      {
        pset<int> s5 = { 3, 0, 1, 100, 303, -1, 555, 3, 3, 3 };
        std::cout << "s = " << s5 << std::endl;
        pchunkedseq<int> xs = { 3, 0, 1, 100, 303, -1, 555, 3, 3, 3 };
        pset<int> s6(xs.cbegin(), xs.cend());
        std::cout << "s = " << s6 << std::endl;
        return;
      }
      
      {
        
        pset<int> x1 = { 0, 134, 774 };
        pset<int> x2 = { 224, 470, 546 };
        x1.intersect(x2);
        std::cout << "x1 = " << x1 << std::endl;
        return;
      }
      
      pset<int> s;
      s.insert(45);
      s.insert(3);
      s.insert(1);
      s.insert(3);

      s.erase(45);
      
      s.insert(45);
      s.insert(78);
      s.erase(1);
      
      pset<int> s2 = { 4, 45, 100, 303 };
      s.merge(s2);
      
      std::cout << "s = " << s << std::endl;
      
      pset<int> s3 = { 4, 100 };
      s.intersect(s3);
      
      std::cout << "s = " << s << std::endl;
      
      pset<int> s4 = { 4, 45, 100, 303 };
      pset<int> s5 = { 0, 1, 100, 303, 555 };

      s4.diff(s5);
      
      std::cout << s4 << std::endl;
      
    }
    
  }
}

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pasl::sched::launch(argc, argv, [&] (bool sequential) {
    pasl::pctl::ex();
  });
  return 0;
}

/***********************************************************************/
