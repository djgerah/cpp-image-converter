#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

namespace img_lib 
{
    struct Size 
    {
        int width;
        int height;
    };

/*
    Каждая компонента представлена числом типа std::byte, принимающим целые значения от 0 до 255. 
    Ноль соответствует минимальной яркости соответствующей компоненты или полной прозрачности, 
    255 — максимальной яркости или полной непрозрачности
*/
    struct Color 
    {
        static Color Black() 
        {
            return { std::byte{0}, std::byte{0}, std::byte{0}, std::byte{255} };
        }

        std::byte r;
        std::byte g;
        std::byte b;
        std::byte a;
    };

    class Image 
    {
        public:
            // создаёт пустое изображение
            Image() = default;

            // создаёт изображение заданного размера, заполняя его заданным цветом
            Image(int w, int h, Color fill);

            // геттеры для отдельного пикселя изображения
            Color GetPixel(int x, int y) const 
            {
                return const_cast<Image*>(this)->GetPixel(x, y);
            }
            
            Color& GetPixel(int x, int y) 
            {
                assert(x < GetWidth() && y < GetHeight() && x >= 0 && y >= 0);
                return GetLine(y)[x];
            }

            // геттер для заданной строки изображения
            Color* GetLine(int y);
            const Color* GetLine(int y) const;

            int GetWidth() const;
            int GetHeight() const;

            // шаг задаёт смещение соседних строк изображения
            // он обычно совпадает с шириной, но может быть больше неё
            int GetStep() const;

            // будем считать изображение корректным, если
            // его площадь положительна
            explicit operator bool() const 
            {
                return GetWidth() > 0 && GetHeight() > 0;
            }

            bool operator!() const 
            {
                return !operator bool();
            }

        private:
        
            int width_ = 0; // ширина изображения в пикселях
            int height_ = 0; // высота изображения в пикселях
            int step_; // отступ, то есть количество пикселей в одной строке

            std::vector<Color> pixels_; // все пиксели изображения хранятся в одном векторе
    };
}  // end of namespace img_lib