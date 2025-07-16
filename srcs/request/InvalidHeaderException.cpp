#include "InvalidHeaderException.hpp"

InvalidHeaderException::InvalidHeaderException(const std::string& message)
    : std::runtime_error("Invalid header: " + message) {}