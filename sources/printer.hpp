#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>

#include "command.hpp"
#include "counters.hpp"

struct work_thread {
   std::thread t_;
   std::string name_;
};

// ------------------------------------------------------------------
class printer {
protected:
    std::queue<command_ptr> cmd_queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    virtual void print(const command_ptr& cmd) = 0;
    virtual void stop() {}
    virtual ~printer() = default;
};
using printer_sptr = std::shared_ptr<printer>;
using printer_wptr = std::weak_ptr<printer>;

// ------------------------------------------------------------------
class console_printer : public printer {
    work_thread worker_{{}, "console_printer"};

    void do_print(const std::string& thread_name) {
        worker_counters wc(thread_name);
        
        while (!stop_) {
            std::unique_lock<std::mutex> u_lock(mutex_);
            cv_.wait(u_lock, [this](){return !cmd_queue_.empty() || stop_;});

            if (stop_) {
                break; // while
            }

            const auto cmd = cmd_queue_.front();
            cmd_queue_.pop();
            u_lock.unlock();
            std::cout << *cmd;

            wc.count(cmd);

        } // while

        std::lock_guard<std::mutex> lock(mutex_);
        while (!cmd_queue_.empty()) {
            std::cout << "queue is not empty yet\n";
            
            const auto cmd = cmd_queue_.front();
            cmd_queue_.pop();
            std::cout << *cmd;

            wc.count(cmd);
        }
        std::cout << wc;
    }
public:
    console_printer() {
        worker_.t_ = std::thread(&console_printer::do_print, this, worker_.name_);
    }

    ~console_printer() override {
        stop_ = true;      // stop worker thread and notify it again
        cv_.notify_one();

        if (worker_.t_.joinable()) {
            worker_.t_.join();
        };
    }

    void print(const command_ptr& cmd) override 
    {   
        std::unique_lock<std::mutex> u_lock(mutex_);
        cmd_queue_.push(cmd);
        u_lock.unlock();
        cv_.notify_one();
    }
};

// ------------------------------------------------------------------
class file_printer : public printer {
    std::array<work_thread, 2> workers_{work_thread{{}, "work_thread_1"},
                                        work_thread{{}, "work_thread_2"}};


   void write_to_file(const std::string& thread_name, const command_ptr& cmd) const {

// TODO
      std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

      const auto cmd_creation_time = std::chrono::duration_cast<std::chrono::microseconds>(cmd->get_creation_time_point().time_since_epoch());
      const auto file_name = "bulk_" + thread_name + "_"
                           + std::to_string(cmd_creation_time.count()) + "_"
                           + ".txt";
      std::ofstream file(file_name, std::ios::app);
      file << *cmd;
      file.close();
    };

    void do_print (const std::string& thread_name) {
        worker_counters wc(thread_name);

        while (!stop_) {
           std::unique_lock<std::mutex> u_lock(mutex_);
           cv_.wait(u_lock, [this](){return !cmd_queue_.empty() || stop_;});

           if (stop_) {
              break; // while
           }

           const auto cmd = cmd_queue_.front();
           cmd_queue_.pop();
           u_lock.unlock();

           write_to_file(thread_name, cmd);
           wc.count(cmd);
        } // while

        while (true) {
           std::unique_lock<std::mutex> lock(mutex_);

           if (cmd_queue_.empty()) {
              break;  // while
           }
           const auto cmd = cmd_queue_.front();
           cmd_queue_.pop();
           lock.unlock();

           write_to_file(thread_name, cmd);
           wc.count(cmd);
        } // while

        std::lock_guard<std::mutex> guard(mutex_);
        std::cout << wc;
    }

public:
    file_printer() {
        for (auto& worker: workers_) {
            worker.t_ = std::thread(&file_printer::do_print, this, worker.name_);
        }
    }

   ~file_printer() override {
      stop_ = true;
      cv_.notify_all();

      std::for_each(workers_.begin(), workers_.end(), [](auto& worker) {
         if (worker.t_.joinable()) {
            worker.t_.join();
         };
      });
   }

   void print(const command_ptr& cmd) override
   {
       std::unique_lock<std::mutex> u_lock(mutex_);
       cmd_queue_.push(cmd);
       u_lock.unlock();
       cv_.notify_one();
   }
};
