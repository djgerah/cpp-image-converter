#include <bmp_image.h>
#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <iostream>
#include <string_view>

using namespace std;

enum Format 
{
    PPM,
    JPEG,
    BMP,
    UNKNOWN
};

class ImageFormatInterface 
{
    public:

        virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
        virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

namespace FormatInterfaces 
{   
    class PPM : public ImageFormatInterface
    {
        public:

            bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override
            {
                return img_lib::SavePPM(file, image);
            }

            img_lib::Image LoadImage(const img_lib::Path& file) const override
            {
                return img_lib::LoadPPM(file);
            }
    };

    class JPEG : public ImageFormatInterface
    {
        public:
        
            bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override
            {
                return img_lib::SaveJPEG(file, image);
            }

            img_lib::Image LoadImage(const img_lib::Path& file) const override
            {
                return img_lib::LoadJPEG(file);
            }
    };

    class BMP : public ImageFormatInterface 
    {
        public:

            bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override 
            {
                return img_lib::SaveBMP(file, image);
            }

            img_lib::Image LoadImage(const img_lib::Path& file) const override 
            {
                return img_lib::LoadBMP(file);
            }
    };
} // end of namespace FormatInterfaces

Format GetFormatByExtension(const img_lib::Path& input_file) 
{
    const string ext = input_file.extension().string();

    if (ext == ".ppm"sv) 
    {
        return Format::PPM;
    }

    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".bmp"sv) 
    {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}

const ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) 
{
    Format format = GetFormatByExtension(path);
    static const FormatInterfaces::PPM ppm_interface;
    static const FormatInterfaces::JPEG jpeg_interface;
    static const FormatInterfaces::BMP bmp_interface;

    switch (format) 
    {
        case Format::PPM:
            return &ppm_interface;

       case Format::JPEG:
            return &jpeg_interface;

        case Format::BMP:
            return &bmp_interface;

        default:
            return nullptr;
    }
}

int main(int argc, const char** argv) 
{
    if (argc != 3) 
    {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    const ImageFormatInterface* input_img = GetFormatInterface(in_path);
    if (!input_img) 
    {
        cerr << "Unknown format of the input file"sv << endl;
        return 2;
    }

    const ImageFormatInterface* output_img = GetFormatInterface(out_path);
    if (!output_img) 
    {
        cerr << "Unknown format of the output file"sv << endl;
        return 3;
    }

    img_lib::Image image = input_img->LoadImage(in_path);
    if (!image) 
    {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!output_img->SaveImage(out_path, image)) 
    {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;

    return 0;
}