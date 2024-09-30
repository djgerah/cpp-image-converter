#include "ppm_image.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib 
{
/*
    Программа должна поддерживать только один из шести возможных форматов PPM — P6. 
    Этот код указывается в начале файла. После него идут ширина и высота изображения, записанные обычными числами в текстовом виде. 
    Заканчивается заголовок максимальным значением цвета. Это число, которое в рассматриваемых примерах будет всегда равно 255. 
*/
    static const string_view PPM_SIG = "P6"sv;
    static const int PPM_MAX = 255;

    // реализуйте эту функцию самостоятельно
    bool SavePPM(const Path& file, const Image& image) 
    {
        ofstream ofs(file, ios::binary);
        // Элементы заголовка разделяются пробельными символами — пробелами или переносами строки. 
        // После заголовка записан символ перевода строки \n и начинаются данные изображения
        ofs << PPM_SIG << "\n" << image.GetWidth() << ' ' << image.GetHeight() << "\n" << PPM_MAX << "\n";

        const int w = image.GetWidth();
        const int h = image.GetHeight();
        vector<char> buff(w * 3);

        for (int y = 0; y < h; ++y) 
        {
            const Color* line = image.GetLine(y);
            
            for (int x = 0; x < w; ++x) 
            {
                // Пиксели кодируются подряд, каждый тремя байтами — значениями R, G и B. Прозрачность игнорируется
                buff[x * 3 + 0] = static_cast<char>(line[x].r);
                buff[x * 3 + 1] = static_cast<char>(line[x].g);
                buff[x * 3 + 2] = static_cast<char>(line[x].b);
            }
            ofs.write(buff.data(), w * 3);
        }

        return ofs.good();
    }

    Image LoadPPM(const Path& file) 
    {
        // открываем поток с флагом ios::binary
        // поскольку будем читать даные в двоичном формате
        ifstream ifs(file, ios::binary);

        // Функция LoadPPM возвращает пустое изображение, если его не удалось прочитать
        if (!ifs.is_open())
        {
            return Image();
        }

        std::string sign;
        int w, h, color_max;

        // читаем заголовок: он содержит формат, размеры изображения
        // и максимальное значение цвета
        ifs >> sign >> w >> h >> color_max;

        // мы поддерживаем изображения только формата P6
        // с максимальным значением цвета 255
        if (sign != PPM_SIG || color_max != PPM_MAX) 
        {
            return {};
        }

        // пропускаем один байт - это конец строки
        const char next = ifs.get();
        if (next != '\n') 
        {
            return {};
        }

        Image result(w, h, Color::Black());
        std::vector<char> buff(w * 3);

        for (int y = 0; y < h; ++y) 
        {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), w * 3);

            for (int x = 0; x < w; ++x) 
            {
                line[x].r = static_cast<byte>(buff[x * 3 + 0]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].b = static_cast<byte>(buff[x * 3 + 2]);
            }
        }

        return result;
    }
}  // end of namespace img_lib