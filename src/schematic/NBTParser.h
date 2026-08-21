#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace cloud9 {

enum class NbtTagType : std::uint8_t {
    End = 0,
    Byte = 1,
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    Double = 6,
    ByteArray = 7,
    String = 8,
    List = 9,
    Compound = 10,
    IntArray = 11,
    LongArray = 12,
};

struct NbtTag;
using NbtTagPtr = std::shared_ptr<NbtTag>;
using NbtList = std::vector<NbtTagPtr>;
using NbtCompound = std::map<std::string, NbtTagPtr>;
using NbtByteArray = std::vector<std::int8_t>;
using NbtIntArray = std::vector<std::int32_t>;
using NbtLongArray = std::vector<std::int64_t>;
using NbtValue = std::variant<std::monostate, std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                              float, double, NbtByteArray, std::string, NbtList, NbtCompound,
                              NbtIntArray, NbtLongArray>;

struct NbtTag {
    NbtTagType type{NbtTagType::End};
    std::string name;
    NbtValue value;

    [[nodiscard]] const NbtTag* child(const std::string& key) const;
    [[nodiscard]] NbtTag* child(const std::string& key);
    [[nodiscard]] const NbtCompound* compound() const noexcept;
    [[nodiscard]] const NbtList* list() const noexcept;
};

struct NbtParseResult {
    NbtTagPtr root;
    std::string error;

    [[nodiscard]] explicit operator bool() const noexcept { return root != nullptr && error.empty(); }
};

class NBTParser {
public:
    struct Limits {
        std::size_t maxBytes{256U * 1024U * 1024U};
        std::size_t maxNodes{4U * 1024U * 1024U};
        int maxDepth{512};
    };

    NBTParser() = default;
    explicit NBTParser(Limits limits) : limits_(limits) {}

    [[nodiscard]] NbtParseResult parse(std::span<const std::uint8_t> bytes, bool littleEndian = false) const;
    [[nodiscard]] NbtParseResult parseMaybeGzip(std::span<const std::uint8_t> bytes, bool littleEndian = false) const;

private:
    Limits limits_;
};

} // namespace cloud9
