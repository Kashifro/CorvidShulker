#pragma once
#include <vector>
#include <string>

class CompoundTag {
public:
    bool contains(const std::string& name) const {
        // For now, we just simulate Items existing
        // In a real hook, you would read the actual structure
        return name == "Items";
    }
};

