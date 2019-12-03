#include <iostream>

#include "bulk_impl.hpp"
#include "printer.hpp"

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

   const auto bulk = std::make_shared<bulk_commands>(bulk_size);

   const auto c_printer = std::make_shared<console_printer>();
   const auto f_printer = std::make_shared<file_printer>();
   bulk->add_printer(c_printer);
   bulk->add_printer(f_printer);

   reader r;
   r.subscribe(bulk);
   r.read(std::cin);
   return 0;
}