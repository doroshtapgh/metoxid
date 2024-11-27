#include <metoxid/metadata.hpp>
#include <metoxid/utils.hpp>
#if defined(METOXID_LINUX) || defined(METOXID_MACOS)
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif
#include <exception>
#include <memory>

Metadata::Metadata(const std::filesystem::path& file) {
    try {
        this->image_ = Exiv2::ImageFactory::open(file);
        image_->readMetadata();
    } catch (Exiv2::Error& err) {
        fatalError("Failed to read file metadata, please check if the selected file is a media file: %s", err.what());
    } catch (const std::exception& e) {
        fatalError("%s", e.what());
    }

    this->comment_ = image_->comment();
    this->xmp_packet_ = image_->xmpPacket();
    this->exif_data_ = image_->exifData();
    this->iptc_data_ = image_->iptcData();
    this->xmp_data_ = image_->xmpData();

    if (!this->comment_.empty()) {
        Category category("Comment", {
            { "Comment", this->comment_ }
        });

        this->metadata_.push_back(category);
    }

    if (!this->exif_data_.empty()) {
        std::unordered_map<std::string, MetadataValue> fields;

        for (const auto& exif_entry : this->exif_data_) {
            std::string key = exif_entry.key();
            MetadataValue value = exif_entry.value();
            fields.insert({key, value});
        }

        Category category("Exif", fields);
        this->metadata_.push_back(category);
    }

    if (!this->iptc_data_.empty()) {
        std::unordered_map<std::string, MetadataValue> fields;

        for (const auto& iptc_entry : this->iptc_data_) {
            std::string key = iptc_entry.key();
            MetadataValue value = iptc_entry.value();
            fields.insert({key, value});
        }

        Category category("IPTC", fields);
        this->metadata_.push_back(category);
    }

    if (!this->xmp_data_.empty()) {
        std::unordered_map<std::string, MetadataValue> fields;

        for (const auto& xmp_entry : this->xmp_data_) {
            std::string key = xmp_entry.key();
            MetadataValue value = xmp_entry.value();
            fields.insert({key, value});
        }

        Category category("XMP Data", fields);
        this->metadata_.push_back(category);
    }

    if (!this->xmp_packet_.empty()) {
        Category category("XMP Packet", {
            { "XMP Packet", this->xmp_packet_ }
        });

        this->metadata_.push_back(category);
    }
}

void Metadata::Save() {
    for (catagory : this->metadata_){
        if (catagory.name == "Comment"){
            for (field : catagory.fields){
                this->comment_[Exiv2::CommentKey(field.first)] = field.second;
            }
        }
        else if (catagory.name == "Exif"){
            for (field : catagory.fields){
                this->exif_data_[Exiv2::ExifKey(field.first)] = field.second;
            }
        }
        else if (catagory.name == "IPTC"){
            for (field : catagory.fields){
                this->iptc_data_[Exiv2::IptcKey(field.first)] = field.second;
            }
        }
        else if (catagory.name == "XMP Data"){
            for (field : catagory.fields){
                this->xmp_data_[Exiv2::XmpKey(field.first)] = field.second;
            }
        }
        else if (catagory.name == "XMP Packet"){
            for (field : catagory.fields){
                this->xmp_packet_[Exiv2::XmpKey(field.first)] = field.second;
            }
        }
    }

    this->image_->setComment(this->comment_);
    this->image_->setXmpPacket(this->xmp_packet_);
    this->image_->setExifData(this->exif_data_);
    this->image_->setXmpData(this->xmp_data_);
    this->image_->setIptcData(this->iptc_data_);
    this->image_->writeMetadata();
}
