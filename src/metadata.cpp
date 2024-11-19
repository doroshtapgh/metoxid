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
    this->exif_data_ = image_->exifData();
    this->icc_profile_ = image_->iccProfile();
    this->iptc_data_ = image_->iptcData();
    this->xmp_data_ = image_->xmpData();
    this->xmp_packet_ = image_->xmpPacket();

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
            auto field = std::make_pair(key, value);
            fields.insert(field);
        }

        Category category("Exif", fields);
    }
}

