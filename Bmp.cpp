#include<cmath>
#include<iostream>
#include<functional>
#include<stdint.h>
#include<fstream>
#include<vector>
#include<string>
using namespace std;
class BMP {
#pragma pack(push, 2)
    typedef struct BMPFileHeader {
        uint16_t fileType;
        uint32_t fileSize;
        uint16_t fileReserved1;
        uint16_t fileReserved2;
        uint32_t fileOffset;
        void test() {
            cout << "fileType:" << fileType << endl;
            cout << "fileSize:" << fileSize << endl;
            cout << "fileOffset:" << fileOffset << endl;
        }
    }BMPFileHeader;
#pragma pack(pop)
    typedef struct BMPInfoHeader {
        uint32_t infoSize;
        int32_t imageWidth;
        int32_t imageHeight;
        uint16_t imagePlanes;
        uint16_t imageBitCount;
        uint32_t imageIsCompression;
        uint32_t imageSize;
        int32_t xPelsPerMeter;
        int32_t yPelsPerMeter;
        uint32_t colorUsedNum;
        uint32_t colorImportantNum;
        void test() {
            cout << "infoSize:" << infoSize << endl;
            cout << "imageWidth:" << imageWidth << endl;
            cout << "imageHeight:" << imageHeight << endl;
            cout << "imagePlanes:" << imagePlanes << endl;
            cout << "imageBitCount:" << imageBitCount << endl;
            cout << "imageIsCompression:" << imageIsCompression << endl;
            cout << "imageSize:" << imageSize << endl;
            cout << "xPerlsPerMeter:" << xPelsPerMeter << endl;
            cout << "yPerlsPerMeter:" << yPelsPerMeter << endl;
            cout << "colorUsedNum:" << colorUsedNum << endl;
            cout << "colorImportantNum:" << colorImportantNum << endl;
        }
    }BMPInfoHeader;
    typedef struct RGBColor {
        uint8_t rgbBlue, rgbGreen, rgbRed, rgbReserved;
        void setColor(int b, int g, int r) {
            rgbBlue = b, rgbGreen = g, rgbRed = r;
        }
    }RGBColor;
    typedef struct FilterTemplate {
        vector<vector<int>> templat;
        int denominator;
    }FilterTemplate;
private:
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    vector<RGBColor> palette;
    vector<uint8_t> data;
    fstream fp;
    int rowStride, realStride;
    void setStride() {
        rowStride = infoHeader.imageWidth * infoHeader.imageBitCount / 8, realStride = (rowStride + 3) / 4 * 4;
    }
    template<typename tp> void read(tp& target) { fp.read((char*)&target, sizeof(target)); }
    template<typename tp> void readVector(vector<tp>& vec) { for (auto i = 0;i < vec.size();i++) { read(vec[i]); } }
    template<typename tp, typename... Args> void read(tp& target, Args&...args) { read(target);read(args...); }
    template<typename tp> void write(tp& target) { fp.write((char*)&target, sizeof(target)); }
    template<typename tp> void writeVector(vector<tp>& vec) { for (auto i = 0;i < vec.size();i++) { write(vec[i]); } }
    template<typename tp, typename... Args> void write(tp& target, Args&...args) { write(target), write(args...); }
    /*
    * change the size of the stride and data of the bmp
    * @param height --- the wanted height of the bmp
    * @param width --- the wanted width of the bmp
    */
    void changeSize(int height, int width) {
        infoHeader.imageHeight = height;
        infoHeader.imageWidth = width;
        setStride();
        data.resize(realStride * infoHeader.imageHeight);
        fileHeader.fileSize = data.size() > 0 ? fileHeader.fileOffset + data.size() * sizeof(*data.begin()) : fileHeader.fileOffset;
    }
    /*
    * generate palette
    */
    void generatePalette() {
        for (int i = 0;i < palette.size();i++) { palette[i].setColor(i, i, i); }
    }
public:
    BMP() {}
    BMP(string filename) { readFromFile(filename); }
    void readFromFile(string filename) {
        fp.open(filename.c_str(), ios::in | ios::binary);
        read(fileHeader, infoHeader);
        // if there is palette
        if (infoHeader.imageBitCount < 24) {
            palette.resize(pow(2, infoHeader.imageBitCount));
            readVector(palette);
        }
        fp.seekg(fileHeader.fileOffset, fp.beg);
        setStride();
        data.resize(realStride * infoHeader.imageHeight);
        readVector(data);
        fp.close();
    }
    void writeToFile(string filename) {
        fp.open(filename.c_str(), ios::out | ios::binary);
        write(fileHeader, infoHeader), writeVector(palette), writeVector(data);
        fp.close();
    }
    BMP Copy() {
        BMP bmp;
        bmp.fileHeader = fileHeader;
        bmp.infoHeader = infoHeader;
        bmp.palette.assign(palette.begin(), palette.end());
        bmp.data.assign(data.begin(), data.end());
        bmp.realStride = realStride, bmp.rowStride = rowStride;
        return bmp;
    }
    int getImageBitCount() { return infoHeader.imageBitCount; }
    void bmpConverter24To8(function<uint8_t(uint8_t, uint8_t, uint8_t)> fn = [](uint8_t b, uint8_t g, uint8_t r)->uint8_t {return (b * 0.114 + g * 0.299 + r * 0.587);}) {
        // generate palette
        generatePalette();
        // change bitCount
        infoHeader.imageBitCount = 8;
        // change stride 
        int stride_tmp = realStride;
        setStride();
        vector<uint8_t> vec(infoHeader.imageHeight * realStride);
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                int t = i * stride_tmp + j * 3;
                vec[i * realStride + j] = fn(data[t], data[t + 1], data[t + 2]);
            }
        }
        swap(data, vec);
        fileHeader.fileOffset += palette.size() * sizeof(*palette.begin());
        fileHeader.fileSize = data.size() > 0 ? fileHeader.fileOffset + data.size() * sizeof(*data.begin()) : fileHeader.fileOffset;
    }
    void bmpReverseColorWithPalette() {
        if (!palette.size()) { cerr << "Cannot do the operation 'reverse color of pictures with palette' with pictures without palette." << endl;return; }
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                data[i * realStride + j] = 255 - data[i * realStride + j];
            }
        }
    }
    void bmpRGBSplit(char cho) {
        if (palette.size()) { cerr << "Cannot do the operation 'split RGB color of pictures without palette' with pictures without palette." << endl;return; }
        if (cho == 'B' || cho == 'b') {
            auto fn = [](uint8_t b, uint8_t g, uint8_t r)->uint8_t {return b;};
            bmpConverter24To8(fn);
        } else if (cho == 'G' || cho == 'g') {
            auto fn = [](uint8_t b, uint8_t g, uint8_t r)->uint8_t {return g;};
            bmpConverter24To8(fn);
        } else {
            auto fn = [](uint8_t b, uint8_t g, uint8_t r)->uint8_t {return r;};
            bmpConverter24To8(fn);
        }
    }
    vector<int> bmpCountGrayHist() {
        if (infoHeader.imageBitCount != 8) { cerr << "Cannot do the operation 'count gray hist' with pictures whose colorBitCount is not 8." << endl; }
        vector<int> ans(256);
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                int tmp = data[i * realStride + j];
                ans[tmp]++;
            }
        }
        return ans;
    }
    void bmpHistogramEqualizationProcessing() {
        auto ans = bmpCountGrayHist();
        for (int i = 1;i < 256;i++) ans[i] += ans[i - 1];
        auto fn = [&ans](int s)->uint8_t {return ans[s] * 255 / ans[255];};
        vector<uint8_t> vec(256);
        for (int i = 0;i < 256;i++) vec[i] = fn(i);
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                data[i * realStride + j] = vec[data[i * realStride + j]];
            }
        }
    }
    void bmpDrawGreyHist(string filePath = "") {
        auto hist = this->bmpCountGrayHist();
        auto bmp = this->Copy();
        // count max of the hist
        int mx = 0;
        for (auto k : hist)mx = max(mx, k);
        // resize bmp
        this->changeSize(256, 256);
        for (int i = 0;i < 256;i++) {
            for (int j = 0;j < 256;j++) {
                data[i * realStride + j] = hist[255 - j] * 255 / mx < i ? 255 : 0;
            }
        }
        if (filePath != "") this->writeToFile(filePath);
        // // change the picture back
        swap(bmp, *this);
    }
    uint8_t getDataValue(int x, int y) {
        auto standardalize = [](int mi, int mx, int tar) {tar = tar > mi ? tar : mi;tar = tar < mx ? tar : mx;return tar;};
        x = standardalize(0, infoHeader.imageHeight, x);
        y = standardalize(0, infoHeader.imageWidth, y);
        return data[x * realStride + y];
    }
    void setDataValue(int x, int y, uint8_t val) {
        data[x * realStride + y] = val;
    }
    static FilterTemplate generateFilterTemplate(int x, int y, vector<int> val, int deno) {
        FilterTemplate ans;
        ans.templat.resize(x);
        int now = 0;
        for (int i = 0;i < x;i++) {
            ans.templat[i].resize(y);
            for (int j = 0;j < y;j++) {
                ans.templat[i][j] = val[now++];
            }
        }
        ans.denominator = deno;
        return ans;
    }
    void filter(FilterTemplate templat) {
        auto bmp = this->Copy();
        int size_x = templat.templat.size(), size_y = templat.templat[0].size();
        int dx = (size_x + 1) / 2, dy = (size_y + 1) / 2;
        for (int k = 0;k < infoHeader.imageHeight;k++) {
            for (int m = 0;m < infoHeader.imageWidth;m++) {
                int ans = 0;
                for (int i = 0;i < size_x;i++) {
                    for (int j = 0;j < size_y;j++) {
                        int x = k - dx + i, y = m - dy + j;
                        ans += templat.templat[i][j] * bmp.getDataValue(x, y);
                    }
                }
                ans /= templat.denominator;
                ans = ans <= 255 ? ans : 255;
                ans = ans >= 0 ? ans : 0;
                setDataValue(k, m, ans);
            }
        }
    }
    void filterStatistic(function<int(vector<int>)> fn, int size_x, int size_y) {
        auto bmp = this->Copy();
        int dx = (size_x + 1) / 2, dy = (size_y + 1) / 2;
        for (int k = 0;k < infoHeader.imageHeight;k++) {
            for (int m = 0;m < infoHeader.imageWidth;m++) {
                vector<int> tmp;
                for (int i = 0;i < size_x;i++) {
                    for (int j = 0;j < size_y;j++) {
                        int x = k - dx + i, y = m - dy + j;
                        tmp.push_back(bmp.getDataValue(x, y));
                    }
                }
                setDataValue(k, m, fn(tmp));
            }
        }
    }
};

void homework1() {
    // homework1
    BMP bmp("./resources/P1/rgb.bmp");
    BMP bmp1 = bmp.Copy();
    BMP bmp2 = bmp.Copy();
    BMP bmp3 = bmp.Copy();
    // task1
    bmp.bmpConverter24To8();
    bmp.writeToFile("./resources/P1/rgb1.bmp");
    // task2
    bmp.bmpReverseColorWithPalette();
    bmp.writeToFile("./resources/P1/rgb2.bmp");
    // task3
    bmp1.bmpRGBSplit('R');
    bmp1.writeToFile("./resources/P1/rgb3.bmp");
    bmp2.bmpRGBSplit('G');
    bmp2.writeToFile("./resources/P1/rgb4.bmp");
    bmp3.bmpRGBSplit('B');
    bmp3.writeToFile("./resources/P1/rgb5.bmp");

}
void homework2() {
    BMP bmp("./resources/P2/dim.bmp");
    bmp.bmpDrawGreyHist("./resources/P2/dim_hist.bmp");
    bmp.bmpHistogramEqualizationProcessing();
    bmp.writeToFile("./resources/P2/dim1.bmp");
    bmp.bmpDrawGreyHist("./resources/P2/dim1_hist.bmp");
}
void homework3() {
    // BMP bmp("./resources/P3/lena.bmp");
    // auto templat = BMP().generateFilterTemplate(5, 5, vector<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 25);
    // bmp.filter(templat);
    // bmp.writeToFile("./resources/P3/lena_res.bmp");
    BMP bmp1("./resources/P3/lena1.bmp");
    auto fn = [](vector<int> vec) {
        sort(vec.begin(), vec.end());
        return vec[vec.size()/2];
    };
    bmp1.filterStatistic(fn, 3, 3);
    bmp1.writeToFile("./resources/P3/lena1_res.bmp");
}


int main() {
    // homework1();
    // homework2();
    homework3();
    return 0;
}