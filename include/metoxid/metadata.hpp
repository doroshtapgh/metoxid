#pragma once
#include <filesystem>
#include <variant>
#include <boost/container/flat_map.hpp>
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
    // TODO: add a field similar to something like this
    // std::variant<std::string, std::unique_ptr<Exiv2::Value>> value;
};

class Metadata {
public:
    Metadata(const std::filesystem::path& file);

    boost::container::flat_map<std::string, MetadataField> GetDict() const {
        return this->metadata_;
    }

    void SetDict(const boost::container::flat_map<std::string, MetadataField>& dict) {
        this->metadata_ = dict;
    }
private:
    std::unique_ptr<Exiv2::Image> image_;
    
    std::string comment_;
    Exiv2::ExifData exif_data_;
    Exiv2::DataBuf icc_profile_;
    Exiv2::IptcData iptc_data_;
    Exiv2::XmpData xmp_data_;
    std::string xmp_packet_;

    boost::container::flat_map<std::string, MetadataField> metadata_;

    // og values are impractical for editing
    // dict is not
    // image <- og values only
    // og values -> dict(editing) -> og values -> image file(saving)

    // FormatType = { Comment, ExifDataField, IccProfileField, etc. }
    // FieldDecription { FieldType, FormatType };
    // Dict = Dictionary<FieldName, Pair<FieldDescription, FieldValue>>
    // Dict GetMetadata() { comment_ + exif_data_ + icc_proile_ + ... }
};

