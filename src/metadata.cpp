#include <metoxid.hpp>
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
        fatalError("exiv2 error occured: %s", err.what());
    } catch (const std::exception& e) {
        fatalError("%s", e.what());
    }

    this->comment_ = image_->comment();
    this->exif_data_ = image_->exifData();
    this->icc_profile_ = image_->iccProfile();
    this->iptc_data_ = image_->iptcData();
    this->xmp_data_ = image_->xmpData();
    this->xmp_packet_ = image_->xmpPacket();
}

