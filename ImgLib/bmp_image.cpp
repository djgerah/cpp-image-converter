#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <iostream>

using namespace std;

namespace img_lib 
{
/*
    Важно, что между полями отсутствует padding. Таким образом, размер первой части (Заголовок файла) — 14 байт, 
    размер второй части (Информационный заголовок) — 40 байт.
*/
    static const int BITMAP_FILE_HEADER = 14;
    static const int BITMAP_INFO_HEADER = 40;
    
    static int GetBMPStride(int width);

    // Заголовок Bitmap File Header
    PACKED_STRUCT_BEGIN BitmapFileHeader
    {  
        BitmapFileHeader(int width, int height) 
        {
            bfSize = bfOffBits + GetBMPStride(width) * height;
        }

        // Подпись — 2 байта. Символы BM
        char bfType[2] = { 'B', 'M' };
        // Суммарный размер заголовка и данных — 4 байта, беззнаковое целое. Размер данных определяется как отступ, умноженный на высоту изображения
        uint32_t bfSize = 0;
        // Зарезервированное пространство — 4 байта, заполненные нулями
        uint32_t bfReserved = 0;
        // Отступ данных от начала файла — 4 байта, беззнаковое целое. Он равен размеру двух частей заголовка
        uint32_t bfOffBits = BITMAP_FILE_HEADER + BITMAP_INFO_HEADER;
    }
    PACKED_STRUCT_END

    // Заголовок Bitmap Info Header
    PACKED_STRUCT_BEGIN BitmapInfoHeader
    {
        BitmapInfoHeader(int width, int height)
            : biWidth(width)
            , biHeight(height)
            {
                biSizeImage = GetBMPStride(width) * height;
            }

        // Размер заголовка — 4 байта, беззнаковое целое. Учитывается только размер второй части заголовка
        uint32_t biSize = BITMAP_INFO_HEADER;
        // Ширина изображения в пикселях — 4 байта, знаковое целое
        uint32_t biWidth = 0;
        // Высота изображения в пикселях — 4 байта, знаковое целое
        uint32_t biHeight = 0;
        // Количество плоскостей — 2 байта, беззнаковое целое. В нашем случае всегда 1 — одна RGB плоскость
        uint16_t biPlanes = 1;
        // Количество бит на пиксель — 2 байта, беззнаковое целое. В нашем случае всегда 24
        uint16_t biBitCount = 24;
        // Тип сжатия — 4 байта, беззнаковое целое. В нашем случае всегда 0 — отсутствие сжатия
        uint32_t biCompression = 0;
        // Количество байт в данных — 4 байта, беззнаковое целое. Произведение отступа на высоту
        uint32_t biSizeImage = 0;
        // Горизонтальное разрешение, пикселей на метр — 4 байта, знаковое целое. Нужно записать 11811, что примерно соответствует 300 DPI
        int32_t biXPelsPerMeter = 11811;
        // Вертикальное разрешение, пикселей на метр — 4 байта, знаковое целое. Нужно записать 11811, что примерно соответствует 300 DPI
        int32_t biYPelsPerMeter = 11811;
        // Количество использованных цветов — 4 байта, знаковое целое. Нужно записать 0 — значение не определено
        int32_t biClrUsed = 0;
        // Количество значимых цветов — 4 байта, знаковое целое. Нужно записать 0x1000000
        int32_t biClrImportant = 0x1000000;
    }
    PACKED_STRUCT_END

    // Функция вычисления отступа по ширине
    static int GetBMPStride(int width) 
    {
        return 4 * ((width * 3 + 3) / 4);
    }

    // напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image) 
    {
        const int w = image.GetWidth();
        const int h = image.GetHeight();

        BitmapFileHeader bitmap_file_header(w, h);
        BitmapInfoHeader bitmap_info_header(w, h);

        ofstream out(file, ios::binary);
        out.write(reinterpret_cast<const char*>(&bitmap_file_header), BITMAP_FILE_HEADER);
        out.write(reinterpret_cast<const char*>(&bitmap_info_header), BITMAP_INFO_HEADER);

        const int row_stride = GetBMPStride(w);
        std::vector<char> buff(row_stride);

        for (int y = h - 1; y >= 0; --y) 
        {
            for (int x = 0; x < w; ++x) 
            {
                const Color component = image.GetPixel(x, y);

                buff[x * 3 + 0] = static_cast<char>(component.b);
                buff[x * 3 + 1] = static_cast<char>(component.g);
                buff[x * 3 + 2] = static_cast<char>(component.r);
            }
            out.write(reinterpret_cast<const char*>(buff.data()), row_stride);
        }

        return out.good();
    }

    // напишите эту функцию
    Image LoadBMP(const Path& file) 
    {
        // открываем поток с флагом ios::binary
        // поскольку будем читать даные в двоичном формате
        ifstream ifs(file, ios::binary);

        // Функция LoadBMP возвращает пустое изображение, если его не удалось прочитать
        if (!ifs.is_open()) 
        {
            return Image();
        }

        int w, h;
        ifs.ignore(18);
        ifs.read(reinterpret_cast<char*>(&w), sizeof(w));
        ifs.read(reinterpret_cast<char*>(&h), sizeof(h));
        ifs.ignore(28);

        BitmapFileHeader bitmap_file_header(w, h);
        BitmapInfoHeader bitmap_info_header(w, h);
        
        // Функция LoadBMP возвращает пустое изображение, если файл не является BMP
        if (bitmap_file_header.bfType[0] != 'B' || bitmap_file_header.bfType[1] != 'M') 
        {
            return Image();
        }

        // Так же функция LoadBMP возвращает пустое изображение, если заголовок некорректный
        if (bitmap_info_header.biSize != BITMAP_INFO_HEADER || bitmap_info_header.biBitCount != 24)
        {
            return Image();
        }

        int row_stride = GetBMPStride(w);
        Image result(w, h, Color::Black());
        std::vector<char> buff(row_stride);

        for (int y = h - 1; y >= 0; --y) 
        {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), row_stride);

            for (int x = 0; x < w; ++x) 
            {
                line[x].b = static_cast<byte>(buff[x * 3 + 0]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            }
        }

        return result;
    }
}  // end of namespace img_lib