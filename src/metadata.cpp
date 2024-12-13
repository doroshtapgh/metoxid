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

void Metadata::SetComment(const std::string& comment) {
    this->comment_ = comment;
}

void Metadata::SetXmpPacket(const std::string& xmp_packet) {
    this->xmp_packet_ = xmp_packet;
}

void Metadata::Save() {
    try{
        this->image_->setComment(this->comment_); //For some reason comment can not save FOR REAL
    }
    catch (const std::exception& e) {
        std::cout << "Failed to save comment: " << e.what() << std::endl;
    } 

    try{
        this->image_->setXmpPacket(this->xmp_packet_); //For some reason XMP can not save FOR REAL
    }
    catch (const std::exception& e) {
        std::cout << "Failed to save XMP packet: " << e.what() << std::endl;
    } 
    
    try{
        this->image_->setExifData(this->exif_data_); //For some reason EXIF can not save
    }
    catch (const std::exception& e){
        std::cout << "HI" << std::endl;
    }

    try{
    this->image_->setXmpData(this->xmp_data_); //For some reason XMP_Data can not save
    }
    catch (const std::exception& e) {
        std::cout << "hi" << std::endl;
    } 

    try{
    this->image_->setIptcData(this->iptc_data_); //For some reason IPTC can not save
    }
    catch (const std::exception& e) {
        std::cout << "hi" << std::endl;
    } 
    
    this->image_->writeMetadata(); //Finally writes metadata to the file
}