#pragma once
#include <filesystem>
#include <variant>
#include <unordered_map>
#include <exiv2/exiv2.hpp>

enum FormatType {
    Comment,
    Exif,
    Icc,
    Iptc,
    XmpData,
    XmpPacket
};

struct MetadataField {
    FormatType format;
    std::variant<std::string, std::shared_ptr<Exiv2::Value>> value;
};

struct Category {
    std::string name;
    FormatType type;
    bool expanded;
    std::unordered_map<std::string, MetadataField> fields;

    Category(const std::string& name, FormatType type, const std::unordered_map<std::string, MetadataField>& fields) {
        this->name = name;
        this->type = type;
        this->expanded = false;
        this->fields = fields;
    }
};

class Metadata {
public:
    Metadata(const std::filesystem::path& file);

    std::vector<Category> GetDict() const {
        return this->metadata_;
    }

    void SetDict(const std::vector<Category>& dict) {
        this->metadata_ = dict;
    }
private:
    Exiv2::Image::AutoPtr image_;
    
    std::string comment_;
    Exiv2::ExifData exif_data_;
    Exiv2::DataBuf* icc_profile_;
    Exiv2::IptcData iptc_data_;
    Exiv2::XmpData xmp_data_;
    std::string xmp_packet_;

    std::vector<Category> metadata_;

    // og values are impractical for editing
    // dict is not
    // image <- og values only
    // og values -> dict(editing) -> og values -> image file(saving)

    // FormatType = { Comment, ExifDataField, IccProfileField, etc. }
    // FieldDecription { FieldType, FormatType };
    // Dict = Dictionary<FieldName, Pair<FieldDescription, FieldValue>>
    // Dict GetMetadata() { comment_ + exif_data_ + icc_proile_ + ... }
};

