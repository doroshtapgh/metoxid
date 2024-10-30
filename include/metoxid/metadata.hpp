#pragma once
#include <filesystem>
#include <unordered_map>
#include <exiv2/exiv2.hpp>

class Metadata {
public:
    Metadata(const std::filesystem::path& file);

    std::string GetComment() { return this->comment_; }
private:
    std::unique_ptr<Exiv2::Image> image_;
    
    std::string comment_;
    Exiv2::ExifData exif_data_;
    Exiv2::DataBuf icc_profile_;
    Exiv2::IptcData iptc_data_;
    Exiv2::XmpData xmp_data_;
    std::string xmp_packet_;
};

