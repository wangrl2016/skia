//
// Created by wangrl on 2020/9/18.
//

#pragma once

namespace render {
    struct MoveOnly {
        MoveOnly(const MoveOnly&) = delete;

        MoveOnly& operator=(const MoveOnly&) = delete;

        MoveOnly(MoveOnly&&) noexcept = default;

        MoveOnly& operator=(MoveOnly&&) noexcept = default;

        MoveOnly() = default;
    };
}
