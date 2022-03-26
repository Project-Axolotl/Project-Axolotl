// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

std::vector<std::string> CrackString(
    const std::string& _cipherText,
    const uint8_t n
) {
    uint32_t length = _cipherText.size();
    uint32_t crackSize = length / n;
    uint32_t leftOver = length % n;

    std::vector<std::string> result;

    result.resize(n);
    
    for (uint32_t i = 0; i < leftOver; i++) {
        result[i].resize(crackSize + 1);
    }

    for (uint32_t i = leftOver; i < n; i++) {
        result[i].resize(crackSize);
    }

    for (uint32_t i = 0; i < crackSize; i++) {
        for (uint32_t j = 0; j < n; j++) {
            result[j][i] = _cipherText[i * n + j];
        }
    }

    for (uint32_t j = 0; j < leftOver; j++) {
        result[j][crackSize] = _cipherText[crackSize * n + j];
    }

    return result;
}

std::string AssembleString(
    const std::vector<std::string>& _cracks
) {
    uint8_t n = _cracks.size();
    uint32_t length = 0;
    for (uint32_t i = 0; i < n; i++) {
        length += _cracks[i].size();
    }

    std::string result;
    result.resize(length);

    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < _cracks[i].size(); j++) {
            result[j * n + i] = _cracks[i][j];
        }
    }

    return result;
}