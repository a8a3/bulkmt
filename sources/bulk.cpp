#include <iostream>
#include <fstream>

#include "bulk_impl.hpp"

using namespace std::chrono;

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

   const auto console_printer = [] (const command_ptr& cmd) {
      const auto sub_cmds = cmd->get_sub_commands();
      std::for_each(sub_cmds.cbegin(), sub_cmds.cend(), 
      [&sub_cmds, i=size_t{0}] (const auto& token) mutable {
         std::cout << token;
         if (i < sub_cmds.size()) {
            std::cout << " ";
         }
         ++i;
      });
      std::cout << std::endl;
   };

  const auto file_printer = [] (const command_ptr& cmd) {
      const auto sub_cmds = cmd->get_sub_commands();
      const auto cmd_creation_time = duration_cast<seconds>(cmd->get_creation_time_point().time_since_epoch()); 
      
      const auto file_name = "bulk" + std::to_string(cmd_creation_time.count()) + ".txt";
      std::ofstream file(file_name);

      std::for_each(sub_cmds.cbegin(), sub_cmds.cend(), 
      [&sub_cmds, &file, i=size_t{0}] (const auto& token) mutable {
         
         file << token;
         if (i < sub_cmds.size()) {
            file << " ";
         }
         ++i;
      });
      file << std::endl;
      file.close();
   };

   const auto bulk = std::make_shared<bulk_commands>(bulk_size);
   bulk->add_printer(console_printer);
   bulk->add_printer(file_printer);

   reader r;
   r.subscribe(bulk);
   r.read(std::cin);
   return 0;
}