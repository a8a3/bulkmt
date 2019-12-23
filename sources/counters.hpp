#pragma once

#include <ostream>

#include "command.hpp"

// ------------------------------------------------------------------
class worker_counters {
    friend std::ostream& operator << (std::ostream&, const worker_counters&);
    friend std::ostream& operator << (std::ostream&, const class main_counters&);

    size_t blocks_{0};
    size_t commands_{0};

    const std::string thread_name_;
public:
    explicit worker_counters(const std::string& thread_name) : thread_name_(thread_name) {}

    void count(const command_ptr& cmd) {
        ++blocks_;
        commands_ += cmd->sub_commands_.size();
    }
};

// ------------------------------------------------------------------
std::ostream& operator << (std::ostream& o, const worker_counters& wc) {
    o << wc.thread_name_ << ", blocks: " << wc.blocks_ << ", commands: " << wc.commands_ << '\n';
    return o;
}

// ------------------------------------------------------------------
class main_counters {
    friend std::ostream& operator << (std::ostream& o, const main_counters& mc);

    worker_counters wc_;
    size_t lines_count_{0};

public:
    explicit main_counters(const std::string& thread_name) : wc_(thread_name) {}

    void count(const command_ptr& cmd) {
        wc_.count(cmd);
    }

    void count_line() {
        ++lines_count_;
    }
};

// ------------------------------------------------------------------
std::ostream& operator << (std::ostream& o, const main_counters& mc) {
    o << mc.wc_.thread_name_
      << ", lines: " 
      << mc.lines_count_ << ", commands: " 
                         << mc.wc_.commands_ << ", blocks: " 
                                             << mc.wc_.blocks_ << '\n';
    return o;
}