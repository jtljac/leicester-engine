//
// Created by jacob on 05/11/22.
//

#pragma once
#include "BaseProcessor.h"

class CopyProcessor : public BaseProcessor {
public:
    void processFile(const std::string& src, const std::string& dest) override;

    std::string getConversionMessage(const std::string& src, const std::string& dest) override;
};
