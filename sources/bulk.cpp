#include <iostream>
#include "bulk_impl.hpp"

// ------------------------------------------------------------------
int main( int argc, char** argv) {

   std::ios::sync_with_stdio(false);

   if (argc != 2) {
      std::cout << "incorrect parameters count... \n"
                   "run as: bulk <N>\n"
                   "where N is bulk size\n";
      return 0;
   }

   const int bulk_size = std::stoi(argv[1]);
   std::cout << "bulk_size: " << bulk_size << std::endl;

   auto sp = std::make_shared<int>(100);

   auto sp_1(sp);
   auto sp_2(sp);

   auto pp = std::make_shared<int>(200);

   std::cout << std::boolalpha << sp.owner_before(sp_1) << std::endl  // false
                               << sp.owner_before(sp_2) << std::endl  // false
                               << sp.owner_before(pp)   << std::endl  // true
                               << pp.owner_before(sp)   << std::endl; // false

   auto own_the_same = [] (const auto& p1, const auto& p2) {
      return !p1.owner_before(p2) && !p2.owner_before(p1);
   };

   std::cout << std::boolalpha << own_the_same(sp, sp_1)   << std::endl  // true
                               << own_the_same(sp, sp_2)   << std::endl  // true
                               << own_the_same(sp_1, sp_2) << std::endl  // true
                               << own_the_same(pp, sp)     << std::endl; // false

   reader r;
   auto bulk = std::make_shared<bulk_command>();
   r.subscribe(bulk);
   r.read(std::cin);
   r.unsubscribe(bulk);
   r.read(std::cin);




   return 0;
}