#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

#include "command.hpp"

// ------------------------------------------------------------------
class printer {
public:
    virtual void print(const command_ptr& cmd) const = 0;
    virtual ~printer() = default;
};
using printer_sptr = std::shared_ptr<printer>;
using pringer_wptr = std::weak_ptr<printer>;

// ------------------------------------------------------------------
class console_printer : public printer {
public:
    console_printer() = default;

    void print(const command_ptr& cmd) const override 
    {
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
    }    
};

// ------------------------------------------------------------------
class file_printer : public printer {
public:
    file_printer() = default;
 
    void print (const command_ptr& cmd) const override 
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
