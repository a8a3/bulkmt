#pragma once

#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <vector>
#include <algorithm>

#define TRACE(_MSG_) std::cout << (_MSG_) << std::endl;

static constexpr auto block_start = "{";
static constexpr auto block_end   = "}";

// ------------------------------------------------------------------
class reader_observer {
public:
   virtual void notify(const std::string& str) = 0;
   virtual ~reader_observer() = default;
};
using reader_observer_sptr = std::shared_ptr<reader_observer>;
using reader_observer_wptr = std::weak_ptr<reader_observer>;

// ------------------------------------------------------------------
class command {
public:   
   virtual bool is_full() const = 0;
   virtual void add_subcommand(const std::string& subcommand) = 0;
   virtual ~command() = default;

   using sub_commands = std::vector<std::string>;
   const sub_commands& get_sub_commands() const {
      return sub_commands_;
   }
protected:
   sub_commands sub_commands_;
};
using command_ptr = std::unique_ptr<command>;

// ------------------------------------------------------------------
class fixed_size_command  : public command {
   const size_t command_size_;

public:
   fixed_size_command(size_t command_size) : command_size_(command_size) {
      sub_commands_.reserve(command_size_);
   }

   bool is_full() const override {
      return sub_commands_.size() == command_size_;
   };

   void add_subcommand(const std::string& subcommand) override {
      if (is_full()) {
         return;
      }
      sub_commands_.push_back(subcommand);
   }
};

// ------------------------------------------------------------------
class free_size_command : public command {
   size_t open_brackets_count_{1};

public:   
   free_size_command() = default;

   bool is_full() const override {
      return open_brackets_count_ == 0;
   };

   void add_subcommand(const std::string& subcommand) override {
      if (is_full()) {
         return;
      }

      if (subcommand == block_start) {
         ++open_brackets_count_;
      } else if (subcommand == block_end) {
         --open_brackets_count_;
      } else {
         sub_commands_.push_back(subcommand);
      }
   }
};

// ------------------------------------------------------------------
class bulk_commands : public reader_observer {
   command_ptr current_command_;
   const size_t bulk_size_;

public:
   bulk_commands(size_t bulk_size) : bulk_size_(bulk_size) {}
  ~bulk_commands() = default;

   command_ptr create_command(const std::string& token) {
      if (token == block_start) {
         return std::make_unique<free_size_command>();
      }
      auto fixed_size_cmd = std::make_unique<fixed_size_command>(bulk_size_);
      fixed_size_cmd->add_subcommand(token);
      return fixed_size_cmd;
   }

   void out_command(const command::sub_commands& sub_cmds) const {
      std::for_each(sub_cmds.begin(), sub_cmds.end(), 
      [&sub_cmds, i=size_t{0}] (const auto& token) mutable {
         std::cout << token;
         if (i < sub_cmds.size()) {
            std::cout << " ";
         }
         ++i;
      });
      std::cout << std::endl;
   }

   // reader_observer impl
   void notify(const std::string& str) override {
      if (current_command_) {
         current_command_->add_subcommand(str);

         if(current_command_->is_full()) {
            out_command(current_command_->get_sub_commands());
            current_command_.release();
         } 

      } else {
         current_command_ = create_command(str);
      }
   }
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
     TRACE("add new subscriber");
     observers_.push_back(observer);
  }

  void unsubscribe(const reader_observer_sptr& observer) {
     TRACE("try to unsubscribe subscriber...");
     const auto sz = observers_.size();
     observers_.remove_if([&observer](const auto& stored_observer) {
        return !observer.owner_before(stored_observer) &&
               !stored_observer.owner_before(observer);
     });
     if (sz > observers_.size()) {
        TRACE("done");
     } else {
        TRACE("failed");
     }
  }

private:
   std::list<reader_observer_wptr> observers_;
};