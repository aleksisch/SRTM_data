#include <iostream>
#include <fstream>
#include <iomanip>

const int GRID_SIZE = 6000;

long double getNumberInGrid(long double val);

template <typename T>
struct quadruple {
    quadruple(T x1, T x2, T x3, T x4) :x1(x1), x2(x2), x3(x3), x4(x4) {}
    T x1;
    T x2;
    T x3;
    T x4;
};

class Coordinates {
    long double latitude;
    long double longitude;
public:
    Coordinates(char **pString) {
        latitude = std::stold(pString[1]);
        longitude = std::stold(pString[2]);
    }

    std::pair<int, int> getFileNumber() {
        std::pair<int, int> res;
        res.first = int((longitude + 180) / 5) + 1;
        res.second = int((60 - latitude) / 5) + 1;
        return res;
    }

    std::pair<long double , long double> getRowCol() {
        long double row = GRID_SIZE - getNumberInGrid(latitude);
        long double column = getNumberInGrid(longitude);
        return std::make_pair(row, column);
    }

    std::ifstream OpenFile() {
        auto tmp = getFileNumber();
        std::ostringstream s1, s2;
        s1 << std::setw(2) << std::setfill('0') << std::to_string(tmp.first);
        s2 << std::setw(2) << std::setfill('0') << std::to_string(tmp.second);
        std::string filename = "./data/srtm_" + s1.str() + "_" + s2.str() + ".tif";
        std::ifstream res;
        res.open(filename, std::ifstream::in | std::ios_base::binary);
        if (!res) {
            std::cout << "32768";
            throw "no data";
        }
        return res;
    }
};

template <typename T>
long double BilinearInterpolation(quadruple<T> h, long double row, long double col) {
    //   x1  r2       x2
    //       X
    //   x3  r1       x4
    long double r1 = ((GRID_SIZE - col) * h.x3 + col * h.x4) / GRID_SIZE;
    long double r2 = ((GRID_SIZE - col) * h.x1 + col * h.x2) / GRID_SIZE;
    long double res = ((GRID_SIZE - row) * r1 + row * r2) / GRID_SIZE;
    return res;
}


uint16_t readInt(std::ifstream& in, long pos) {
    uint16_t res = 1;
    in.seekg(pos);
    in.readsome(reinterpret_cast<char *>(&res), sizeof(int16_t));
    return res;
}

long double getHeight(char **argv) {
    Coordinates coord(argv);
    auto fstream = coord.OpenFile();
    const int bytes_offset = 3021 * 16 + 6;     //offset in SRTM geotiff file
    auto row_col = coord.getRowCol();
    int row = int(row_col.first);
    int column = int(row_col.second);
    int cur_pos = bytes_offset + (row * GRID_SIZE + column) * 2;
    quadruple<int> heights(readInt(fstream, cur_pos),
                           readInt(fstream, cur_pos + 1 * 2),
                           readInt(fstream, cur_pos + GRID_SIZE * 2),
                           readInt(fstream, cur_pos + (GRID_SIZE + 1) * 2));
    long double height = BilinearInterpolation(heights, row_col.first, row_col.second);
    return height;
}

long double getNumberInGrid(long double val) {
    long double res = (val - int(val / 5) * 5) / 5 * GRID_SIZE;
    if (res < 0)
        res += GRID_SIZE;
    return res;    //We have GRID_SIZE*GRID_SIZE grid in every 5*5 degrees.
}

int main(int argc, char** argv) {
    if (argc !=  3) {
        std::cout << "Enter coordinates in command line arguments" << std::endl;
        return -1;
    }
    try {
    	std::cout << getHeight(argv);
    } catch (...) {
    	return -1;
    }
    return 0;
}