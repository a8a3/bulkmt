#pragma once

#include <iostream>
#include <string>
#include <list>
#include <memory>

#define TRACE(_MSG_) std::cout << (_MSG_) << std::endl;

// ------------------------------------------------------------------
class reader_observer {
public:
   virtual void notify(const std::string& str) = 0;
};
using reader_observer_sptr = std::shared_ptr<reader_observer>;
using reader_observer_wptr = std::weak_ptr<reader_observer>;

// ------------------------------------------------------------------
class bulk_command : public reader_observer {
public:
   bulk_command() = default;
  ~bulk_command() = default;

  // i_reader_observer impl
  void notify(const std::string& str) override {
     std::cout << str << std::endl;
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