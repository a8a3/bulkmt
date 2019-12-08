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

// ------------------------------------------------------------------
class printer {
public:
    virtual void print(const command_ptr& cmd) = 0;
    virtual void stop() {}
    virtual ~printer() = default;
};
using printer_sptr = std::shared_ptr<printer>;
using pringer_wptr = std::weak_ptr<printer>;

// ------------------------------------------------------------------
class console_printer : public printer {
    std::queue<command_ptr> cmd_queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
    std::thread worker_;

    void do_print() {

        const auto print_cmd = [] (const auto& cmd) {
            const auto sub_cmds = cmd->get_sub_commands();
            std::for_each(sub_cmds.cbegin(), sub_cmds.cend(), 
            [&sub_cmds, i=size_t{0}] (const auto& token) mutable {
                std::cout << token;
                if (i < sub_cmds.size()) {
                    std::cout << " ";
                    ++i;
                }
            });
            std::cout << std::endl;
        };

        while (!stop_) {
            // std::cout << "start wait\n";

            std::unique_lock<std::mutex> u_lock(mutex_);
            cv_.wait(u_lock, [this](){return !cmd_queue_.empty() || stop_;});

            // std::cout << "wake up\n";

            if (stop_) {
                // std::cout << "got stop signal\n";
                break; // while
            }

            // std::cout << "commands in queue: " << cmd_queue_.size() << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            const auto cmd = cmd_queue_.front();
            cmd_queue_.pop();
            u_lock.unlock();
            print_cmd(cmd);
        } // while

        std::lock_guard<std::mutex> lock(mutex_);
        while (!cmd_queue_.empty()) {

            std::cout << "queue is not empty yet\n";
            
            const auto cmd = cmd_queue_.front();
            cmd_queue_.pop();
            print_cmd(cmd);
        }
        
        // std::cout << "work thread " << std::this_thread::get_id() << " is stopped\n";
    }
public:
    console_printer() {
        // std::cout << "CTOR\n";
        worker_ = std::thread(&console_printer::do_print, this);
    }

    ~console_printer() override {
        // std::cout << "DTOR\n";

        stop_ = true;      // stop worker thread and notify it again
        cv_.notify_one();

        if (worker_.joinable()) {
            worker_.join();
        };
    }

    void print(const command_ptr& cmd) override 
    {   
        std::unique_lock<std::mutex> u_lock(mutex_);
        cmd_queue_.push(cmd);
        u_lock.unlock();
        cv_.notify_one();

    //     std::cout << "notify worker\n";
    }    
};

// ------------------------------------------------------------------
class file_printer : public printer {
public:
    file_printer() = default;
 
    void print (const command_ptr& cmd) override 
    {
        const auto sub_cmds = cmd->get_sub_commands();
        const auto cmd_creation_time = std::chrono::duration_cast<std::chrono::seconds>(cmd->get_creation_time_point().time_since_epoch()); 

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
    }
};
