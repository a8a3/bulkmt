#pragma once

#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <chrono>

#include "command.hpp"
#include "printer.hpp"

// ------------------------------------------------------------------
class reader_observer {
public:
   virtual void notify(const std::string& str) = 0;
   virtual ~reader_observer() = default;
};
using reader_observer_sptr = std::shared_ptr<reader_observer>;
using reader_observer_wptr = std::weak_ptr<reader_observer>;

// ------------------------------------------------------------------
class bulk_commands : public reader_observer {

public:   
   bulk_commands(size_t bulk_size) : bulk_size_(bulk_size) {}
  ~bulk_commands() {
      if (current_command_) {
         out_command(current_command_);
      }
   }

   command_ptr create_command(const std::string& token) {
      if (token == command::block_start) {
         return std::make_unique<free_size_command>();
      }
      auto fixed_size_cmd = std::make_unique<fixed_size_command>(bulk_size_);
      fixed_size_cmd->add_subcommand(token);
      return fixed_size_cmd;
   }

   void add_printer(const printer_sptr& prn) {
      printers_.push_back(prn);
   }

   void out_command(const command_ptr& cmd) const {
      for(const auto& printer: printers_) {
         if (!printer.expired()) {
            printer.lock()->print(cmd);
         }
      }
   }

   // reader_observer impl
   void notify(const std::string& str) override {
      if (current_command_) {
         current_command_->add_subcommand(str);

         if(current_command_->is_full()) {
            out_command(current_command_);
            current_command_.release();
         } 

      } else {
         current_command_ = create_command(str);
      }
   }

private:
   command_ptr current_command_;
   const size_t bulk_size_;

   using printers = std::list<pringer_wptr>; 
   printers printers_;
};

// ------------------------------------------------------------------
class reader {
public:
   reader() = default;
  ~reader() = default;

  void read(std::istream& s) const {
     std::string str;

     while (s.peek() != '\n' && std::getline(s, str)) {
        for (auto& observer: observers_) {
           if (!observer.expired()) {
              observer.lock()->notify(str);
           }
        }
     }
     s.ignore();
  }

  void subscribe(const reader_observer_sptr& observer) {
     observers_.push_back(observer);
  }

  void unsubscribe(const reader_observer_sptr& observer) {
     observers_.remove_if([&observer](const auto& stored_observer) {
        return !observer.owner_before(stored_observer) &&
               !stored_observer.owner_before(observer);
     });
  }

private:
   std::list<reader_observer_wptr> observers_;
};