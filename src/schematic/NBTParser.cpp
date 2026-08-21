#include "schematic/NBTParser.h"

#include <array>
#include <bit>
#include <cstring>
#include <limits>
#include <stdexcept>

#ifdef CLOUD9_HAS_ZLIB
#include <zlib.h>
#endif

namespace cloud9 {
namespace {

class Reader {
public:
    Reader(std::span<const std::uint8_t> bytes, bool littleEndian, const NBTParser::Limits& limits)
        : bytes_(bytes), littleEndian_(littleEndian), limits_(limits) {}

    NbtTagPtr readRoot() {
        const auto type = static_cast<NbtTagType>(readByte());
        if (type == NbtTagType::End) fail("root tag cannot be TAG_End");
        const std::string name = readString();
        return readPayload(type, name, 0);
    }

private:
    [[noreturn]] void fail(const char* message) const { throw std::runtime_error(message); }
    void require(std::size_t count) {
        if (count > bytes_.size() - position_) fail("NBT data is truncated");
    }

    std::uint8_t readByte() {
        require(1);
        return bytes_[position_++];
    }

    std::uint16_t readU16() {
        require(2);
        const std::uint16_t first = bytes_[position_++];
        const std::uint16_t second = bytes_[position_++];
        return littleEndian_ ? static_cast<std::uint16_t>(first | (second << 8U))
                             : static_cast<std::uint16_t>((first << 8U) | second);
    }

    std::uint32_t readU32() {
        require(4);
        std::uint32_t result = 0;
        if (littleEndian_) {
            for (int index = 0; index < 4; ++index) result |= static_cast<std::uint32_t>(bytes_[position_++]) << (index * 8);
        } else {
            for (int index = 0; index < 4; ++index) result = (result << 8U) | bytes_[position_++];
        }
        return result;
    }

    std::uint64_t readU64() {
        require(8);
        std::uint64_t result = 0;
        if (littleEndian_) {
            for (int index = 7; index >= 0; --index) result |= static_cast<std::uint64_t>(bytes_[position_++]) << (index * 8);
        } else {
            for (int index = 0; index < 8; ++index) result = (result << 8U) | bytes_[position_++];
        }
        return result;
    }

    std::int16_t readI16() { return std::bit_cast<std::int16_t>(readU16()); }
    std::int32_t readI32() { return std::bit_cast<std::int32_t>(readU32()); }
    std::int64_t readI64() { return std::bit_cast<std::int64_t>(readU64()); }
    float readFloat() { return std::bit_cast<float>(readU32()); }
    double readDouble() { return std::bit_cast<double>(readU64()); }

    std::string readString() {
        const std::uint16_t length = readU16();
        require(length);
        std::string result(reinterpret_cast<const char*>(bytes_.data() + position_), length);
        position_ += length;
        return result;
    }

    std::size_t readLength(const char* typeName) {
        const std::int32_t signedLength = readI32();
        if (signedLength < 0) fail(typeName);
        const std::size_t length = static_cast<std::size_t>(signedLength);
        if (length > limits_.maxBytes || length > limits_.maxNodes) fail(typeName);
        return length;
    }

    NbtTagPtr make(NbtTagType type, std::string name, NbtValue value) {
        if (++nodes_ > limits_.maxNodes) fail("NBT node limit exceeded");
        return std::make_shared<NbtTag>(NbtTag{type, std::move(name), std::move(value)});
    }

    NbtTagPtr readPayload(NbtTagType type, std::string name, int depth) {
        if (depth > limits_.maxDepth) fail("NBT nesting limit exceeded");
        switch (type) {
        case NbtTagType::Byte: return make(type, std::move(name), static_cast<std::int8_t>(readByte()));
        case NbtTagType::Short: return make(type, std::move(name), readI16());
        case NbtTagType::Int: return make(type, std::move(name), readI32());
        case NbtTagType::Long: return make(type, std::move(name), readI64());
        case NbtTagType::Float: return make(type, std::move(name), readFloat());
        case NbtTagType::Double: return make(type, std::move(name), readDouble());
        case NbtTagType::String: return make(type, std::move(name), readString());
        case NbtTagType::ByteArray: {
            const std::size_t length = readLength("invalid NBT byte array length");
            require(length);
            NbtByteArray values;
            values.reserve(length);
            for (std::size_t index = 0; index < length; ++index) values.push_back(static_cast<std::int8_t>(readByte()));
            return make(type, std::move(name), std::move(values));
        }
        case NbtTagType::IntArray: {
            const std::size_t length = readLength("invalid NBT int array length");
            if (length > (bytes_.size() - position_) / 4U) fail("NBT int array is truncated");
            NbtIntArray values;
            values.reserve(length);
            for (std::size_t index = 0; index < length; ++index) values.push_back(readI32());
            return make(type, std::move(name), std::move(values));
        }
        case NbtTagType::LongArray: {
            const std::size_t length = readLength("invalid NBT long array length");
            if (length > (bytes_.size() - position_) / 8U) fail("NBT long array is truncated");
            NbtLongArray values;
            values.reserve(length);
            for (std::size_t index = 0; index < length; ++index) values.push_back(readI64());
            return make(type, std::move(name), std::move(values));
        }
        case NbtTagType::List: {
            const auto childType = static_cast<NbtTagType>(readByte());
            if (childType == NbtTagType::End) {
                const std::int32_t count = readI32();
                if (count != 0) fail("TAG_List with TAG_End must be empty");
                return make(type, std::move(name), NbtList{});
            }
            const std::size_t length = readLength("invalid NBT list length");
            NbtList values;
            values.reserve(length);
            for (std::size_t index = 0; index < length; ++index) {
                values.push_back(readPayload(childType, {}, depth + 1));
            }
            return make(type, std::move(name), std::move(values));
        }
        case NbtTagType::Compound: {
            NbtCompound values;
            while (true) {
                const auto childType = static_cast<NbtTagType>(readByte());
                if (childType == NbtTagType::End) break;
                if (childType > NbtTagType::LongArray) fail("invalid NBT tag type");
                std::string childName = readString();
                NbtTagPtr child = readPayload(childType, childName, depth + 1);
                values[std::move(childName)] = std::move(child);
            }
            return make(type, std::move(name), std::move(values));
        }
        case NbtTagType::End:
            break;
        }
        fail("invalid NBT tag type");
    }

    std::span<const std::uint8_t> bytes_;
    std::size_t position_{0};
    bool littleEndian_{false};
    const NBTParser::Limits& limits_;
    std::size_t nodes_{0};
};

#ifdef CLOUD9_HAS_ZLIB
std::vector<std::uint8_t> inflateGzip(std::span<const std::uint8_t> input) {
    if (input.size() > std::numeric_limits<uInt>::max()) throw std::runtime_error("gzip input is too large");
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());
    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) throw std::runtime_error("could not initialize gzip decoder");

    std::vector<std::uint8_t> output;
    std::array<std::uint8_t, 64U * 1024U> buffer{};
    int status = Z_OK;
    while (status == Z_OK) {
        stream.next_out = buffer.data();
        stream.avail_out = static_cast<uInt>(buffer.size());
        status = inflate(&stream, Z_NO_FLUSH);
        const std::size_t produced = buffer.size() - stream.avail_out;
        output.insert(output.end(), buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(produced));
        if (output.size() > 256U * 1024U * 1024U) {
            inflateEnd(&stream);
            throw std::runtime_error("decompressed NBT exceeds safety limit");
        }
    }
    inflateEnd(&stream);
    if (status != Z_STREAM_END) throw std::runtime_error("invalid gzip NBT stream");
    return output;
}
#endif

} // namespace

const NbtTag* NbtTag::child(const std::string& key) const {
    const auto* values = compound();
    if (values == nullptr) return nullptr;
    const auto it = values->find(key);
    return it == values->end() ? nullptr : it->second.get();
}

NbtTag* NbtTag::child(const std::string& key) {
    auto* values = std::get_if<NbtCompound>(&value);
    if (values == nullptr) return nullptr;
    const auto it = values->find(key);
    return it == values->end() ? nullptr : it->second.get();
}

const NbtCompound* NbtTag::compound() const noexcept { return std::get_if<NbtCompound>(&value); }
const NbtList* NbtTag::list() const noexcept { return std::get_if<NbtList>(&value); }

NbtParseResult NBTParser::parse(std::span<const std::uint8_t> bytes, bool littleEndian) const {
    if (bytes.empty()) return {nullptr, "NBT input is empty"};
    if (bytes.size() > limits_.maxBytes) return {nullptr, "NBT input exceeds safety limit"};
    try {
        return {Reader(bytes, littleEndian, limits_).readRoot(), {}};
    } catch (const std::exception& exception) {
        return {nullptr, exception.what()};
    }
}

NbtParseResult NBTParser::parseMaybeGzip(std::span<const std::uint8_t> bytes, bool littleEndian) const {
    if (bytes.size() >= 2U && bytes[0] == 0x1FU && bytes[1] == 0x8BU) {
#ifdef CLOUD9_HAS_ZLIB
        try {
            const std::vector<std::uint8_t> decompressed = inflateGzip(bytes);
            return parse(decompressed, littleEndian);
        } catch (const std::exception& exception) {
            return {nullptr, exception.what()};
        }
#else
        return {nullptr, "gzip NBT requires zlib; rebuild with CLOUD9_ENABLE_ZLIB and zlib installed"};
#endif
    }
    return parse(bytes, littleEndian);
}

} // namespace cloud9
