// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <cstdint>

export module ACL.Main:Types;

export namespace ACL {
    using i8 = std::int8_t;

    using u8 = std::uint8_t;

    constexpr i8 i8min = INT8_MIN;

    constexpr i8 i8max = INT8_MAX;

    constexpr u8 u8max = UINT8_MAX;

    using i16 = std::int16_t;

    using u16 = std::uint16_t;

    constexpr i16 i16min = INT16_MIN;

    constexpr i16 i16max = INT16_MAX;

    constexpr u16 u16max = UINT16_MAX;

    using i32 = std::int32_t;

    using u32 = std::uint32_t;

    constexpr i32 i32min = INT32_MIN;

    constexpr i32 i32max = INT32_MAX;

    constexpr u32 u32max = UINT32_MAX;

    using i64 = std::int64_t;

    using u64 = std::uint64_t;

    constexpr i64 i64min = INT64_MIN;

    constexpr i64 i64max = INT64_MAX;

    constexpr u64 u64max = UINT64_MAX;

    using f32 = float;

    using f64 = double;

    using size = u64;
}
