#include<cmath>
#include<iostream>
#include<functional>
#include<stdint.h>
#include<fstream>
#include<vector>
#include<string>
#include<set>
#include<queue>
#include<map>
#include<sstream>
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
    typedef struct Area {
        int x1, y1, x2, y2;const bool operator<(Area other)const {
            if (x1 != other.x1) return x1 < other.x1;
            if (y1 != other.y1) return y1 < other.y1;
            if (y2 != other.y2) return y2 < other.y2;
            return x2 < other.x2;
        }
    }area;
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
        palette.resize(pow(2, infoHeader.imageBitCount));
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
    int getImageHeight() { return infoHeader.imageHeight; }
    int getImageWidth() { return infoHeader.imageWidth; }
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
    void bmpDrawGreyHist(string filePath, vector<int> hist, int line = 0) {
        auto bmp = this->Copy();
        // count max of the hist
        int mx = 0;
        for (auto k : hist)mx = max(mx, k);
        // resize bmp
        mx *= 1.3;
        this->changeSize(256, 256);
        for (int i = 0;i < 256;i++) {
            for (int j = 0;j < 256;j++) {
                if (j != line) data[i * realStride + j] = hist[j] * 255 / mx < i ? 255 : 0;
                else setDataValue(i, j, 0);
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
        if (x >= infoHeader.imageHeight || y >= infoHeader.imageWidth) return;
        if (x < 0 || y < 0) return;
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
    void operateThroughOperator(function<uint8_t(function<uint8_t(int, int)>, int, int)> fn) {
        auto bmp = this->Copy();
        for (int k = 0;k < infoHeader.imageHeight;k++) {
            for (int m = 0;m < infoHeader.imageWidth;m++) {
                auto getDataValue = [&bmp](int x, int y)->int {return bmp.getDataValue(x, y);};
                setDataValue(k, m, fn(getDataValue, k, m));
            }
        }
    }
    int interpolationOnce(int x, int y, int x1, int x2, int y1, int y2) {
        int ans = (double)((x2 - x) * (y2 - y) * (int)getDataValue(x1, y1) + (x - x1) * (y2 - y) * (int)getDataValue(x2, y1)
            + (x2 - x) * (y - y1) * (int)getDataValue(x1, y2) + (x - x1) * (y - y1) * (int)getDataValue(x2, y2)) / (x2 - x1) / (y2 - y1);
        ans = ans > 255 ? 255 : ans;
        ans = ans < 0 ? 0 : ans;
        return ans;
    }
    void interpolation(vector<vector<bool> > vis, function<void(int, int, int&, int&, int&, int&)> fGet) {
        auto fn = [&](int x, int y) {
            int x1, x2, y1, y2;
            fGet(x, y, x1, y1, x2, y2);
            return interpolationOnce(x, y, x1, y1, x2, y2);
        };
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                if (vis[i][j]) continue;
                setDataValue(i, j, fn(i, j));
            }
        }
    }
    void bmpChangeSize(double fx, double fy) {
        BMP bmp = this->Copy();
        int h1 = infoHeader.imageHeight, w1 = infoHeader.imageWidth;
        changeSize(h1 * fx, w1 * fy);
        for (auto i = data.begin();i != data.end();i++) {
            *i = 0;
        }
        vector<vector<bool> > vis(infoHeader.imageHeight, vector<bool>(infoHeader.imageWidth));
        for (int i = 0;i < h1;i++) {
            for (int j = 0;j < w1;j++) {
                setDataValue(i * fx, j * fy, bmp.getDataValue(i, j));
                vis[i * fx][j * fy] = true;
            }
        }
        auto FGet = [&fx, &fy](int i, int j, int& x1, int& y1, int& x2, int& y2) {
            x1 = ((int)(i / fx)) * fx;
            x2 = ((int)(i / fx) - 1) * fx;
            if (x2 < 0) x2 = ((int)(i / fx) + 1) * fx;
            y1 = ((int)(j / fy)) * fy;
            y2 = ((int)(j / fy) - 1) * fy;
            if (y2 < 0) y2 = ((int)(j / fy) + 1) * fy;
            if (x1 > x2) swap(x1, x2);
            if (y1 > y2) swap(y1, y2);
        };
        interpolation(vis, FGet);
    }
    void bmpMove(int fx, int fy) {
        BMP bmp = this->Copy();
        for (int i = 0;i < data.size();i++) data[i] = 0;
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                setDataValue(i + fx, j + fy, bmp.getDataValue(i, j));
            }
        }
    }
    void bmpMirror() {
        BMP bmp = this->Copy();
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                setDataValue(i, j, bmp.getDataValue(infoHeader.imageHeight - i - 1, j));
            }
        }
    }
    void bmpRotate(int x0, int y0, double alpha) {
        alpha *= M_PI / 360;
        auto bmp = this->Copy();
        for (int i = 0;i < data.size();i++) data[i] = 0;
        vector<vector<bool>> vis(infoHeader.imageHeight, vector<bool>(infoHeader.imageWidth));
        auto fn = [&x0, &y0, &alpha](int x, int y)->pair<int, int> {
            int x1 = x - x0, y1 = y - y0;
            int x2 = x1 * cos(alpha) - y1 * sin(alpha);
            int y2 = x1 * sin(alpha) + y1 * cos(alpha);
            return { x2 + x0,y2 + y0 };
        };
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                auto [x, y] = fn(i, j);
                if (x < 0 || x >= infoHeader.imageHeight || y < 0 || y >= infoHeader.imageWidth) continue;
                setDataValue(x, y, bmp.getDataValue(i, j));
                vis[x][y] = 1;
            }
        }
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                if (vis[i][j]) continue;
                int left = i - 1, right = i + 1;
                while (left >= 0 && !vis[left][j]) left--;
                if (left < 0) continue;
                while (right < infoHeader.imageWidth && !vis[right][j]) right++;
                if (right >= infoHeader.imageWidth) continue;
                setDataValue(i, j, (double)(right - i) / (right - left) * getDataValue(right, j)
                    + (double)(i - left) / (right - left) * getDataValue(left, j));
            }
        }
    }
    void bmpThreholdCut(string filePath = "") {
        auto vec = bmpCountGrayHist();
        int ans = 0, pos = 0;
        int st = 0, en = 255;
        while (!vec[st]) st++;
        while (!vec[en]) en--;
        for (int i = st;i < en;i++) {
            double tmp1 = 0, tmp2 = 0;
            double ans1 = 0, ans2 = 0;
            for (int j = 0;j <= i;j++) {
                tmp1 += vec[j], ans1 += j * vec[j];
            }
            ans1 /= tmp1;
            for (int j = i + 1;j <= 255;j++) {
                tmp2 += vec[j], ans2 += j * vec[j];
            }
            ans2 /= tmp2;
            double p1 = tmp1 / (tmp1 + tmp2), p2 = tmp2 / (tmp1 + tmp2);
            double ans0 = p1 * p2 * (ans1 - ans2) * (ans1 - ans2);
            if (ans0 > ans) { pos = i, ans = ans0; }
        }
        // pos=120;
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                if (getDataValue(i, j) <= pos) setDataValue(i, j, 0);
                else setDataValue(i, j, 255);
            }
        }
        Copy().bmpDrawGreyHist(filePath, vec, pos);
    }
    void bmpSeedGrow() {
        BMP bmp = this->Copy();
        typedef struct Node {
            int x, y;const bool operator<(Node other)const {
                if (y == other.y) return x < other.x;
                else return y < other.y;
            }
        }node;
        for (int i = 0;i < data.size();i++) data[i] = 0;
        int dx[] = { 0,0,1,-1 };
        int dy[] = { 1,-1,0,0 };
        queue<node> q;
        set<node> vis;
        set<node> flagv;
        for (int x = 0;x < infoHeader.imageHeight;x++) {
            for (int y = 0;y < infoHeader.imageWidth;y++) {
                if (bmp.getDataValue(x, y) > 230) { q.push({ x,y }), vis.insert({ x,y }); flagv.insert({ x,y }); }
            }
        }
        while (!q.empty()) {
            node now = q.front();
            if (!flagv.count(now))
                setDataValue(now.x, now.y, 125);
            else setDataValue(now.x, now.y, 255);
            q.pop();
            for (int i = 0;i < 4;i++) {
                int x1 = now.x + dx[i];
                int y1 = now.y + dy[i];
                if (x1 < 0 || x1 >= infoHeader.imageHeight || y1 < 0 || y1 >= infoHeader.imageWidth) continue;
                if (vis.count({ x1,y1 })) continue;
                if (abs(bmp.getDataValue(x1, y1) - bmp.getDataValue(now.x, now.y)) <= 11) {
                    q.push({ x1,y1 });
                    vis.insert({ x1,y1 });
                }
            }
        }
    }
    void bmpSeparateDfs(int x1, int y1, int x2, int y2, map<area, pair<int, int>>& mp) {
        if (x2 < x1 || y2 < y1) return;
        if (x2 - x1 <= 4 || y2 - y1 <= 4) {
            mp[{x1, y1, x2, y2}] = { getDataValue(x1,y1),getDataValue(x1,y1) };
        } else {
            int x3 = (x1 + x2) / 2, y3 = (y1 + y2) / 2;
            bmpSeparateDfs(x1, y1, x3, y3, mp);
            bmpSeparateDfs(x3 + 1, y1, x2, y3, mp);
            bmpSeparateDfs(x1, y3 + 1, x3, y2, mp);
            bmpSeparateDfs(x3 + 1, y3 + 1, x2, y2, mp);
            auto [ri1, ra1] = mp[{x1, y1, x2, y2}];
            auto [ri2, ra2] = mp[{x3 + 1, y1, x2, y3}];
            auto [ri3, ra3] = mp[{x1, y3 + 1, x3, y2}];
            auto [ri4, ra4] = mp[{x3 + 1, y3 + 1, x2, y2}];
            int ri = min(min(ri1, ri2), min(ri3, ri4));
            int ra = max(max(ra1, ra2), max(ra3, ra4));
            mp[{x1, y1, x2, y2}] = { ri,ra };
        }
    }
    void bmpSeparate(int boundaryValue) {
        map<area, pair<int, int> > mp;
        bmpSeparateDfs(0, 0, infoHeader.imageHeight, infoHeader.imageWidth, mp);
        auto fn = [&mp](area s) {
            return mp[s].second - mp[s].first <= 200;
        };
        queue<area> q;
        q.push({ 0,0,infoHeader.imageHeight,infoHeader.imageWidth });
        while (!q.empty()) {
            area now = q.front();
            q.pop();
            if (now.x1 > now.x2 || now.y1 > now.y2) continue;
            for (int i = now.x1;i <= now.x2;i++) setDataValue(i, now.y1, 0), setDataValue(i, now.y2, boundaryValue);
            for (int i = now.y1;i <= now.y2;i++) setDataValue(now.x1, i, 0), setDataValue(now.x2, i, boundaryValue);
            if (fn(now)) continue;
            int x1 = now.x1, x2 = now.x2, y1 = now.y1, y2 = now.y2;
            int x3 = (x1 + x2) / 2, y3 = (y1 + y2) / 2;
            q.push({ x1,y1,x3,y3 });
            q.push({ x3 + 1,y1,x2,y3 });
            q.push({ x1,y3 + 1,x3,y2 });
            q.push({ x3 + 1,y3 + 1,x2,y2 });
        }
    }
    void bmpContourExtraction() {
        auto bmp = this->Copy();
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                if (getDataValue(i, j) < 127) {
                    setDataValue(i, j, 0);
                    bmp.setDataValue(i, j, 0);
                } else {
                    setDataValue(i, j, 255);
                    bmp.setDataValue(i, j, 255);
                }
            }
        }

        for (int i = 1;i < infoHeader.imageHeight - 1;i++) {
            for (int j = 1;j < infoHeader.imageWidth - 1;j++) {
                if (getDataValue(i, j) == 0) {
                    int count = 0;
                    for (int m = i - 1;m < i + 2;m++) {
                        for (int n = j - 1;n < j + 2;n++) {
                            if (getDataValue(m, n) == 0) {
                                count++;
                            }
                        }
                    }
                    if (count == 9) {
                        bmp.setDataValue(i, j, 255);
                    }
                }
            }
        }
        swap(*this,bmp);
    }
    void bmpZoneMark() {
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                if (getDataValue(i, j) < 127) {
                    setDataValue(i, j, 0);
                    setDataValue(i, j, 0);
                } else {
                    setDataValue(i, j, 255);
                    setDataValue(i, j, 255);
                }
            }
        }

        auto bmp = this->Copy();

        int srcValue = getDataValue(0, 0);
        vector<vector<int> > equalTable(infoHeader.imageHeight, vector<int>(infoHeader.imageWidth));
        for (int i = 0;i < infoHeader.imageHeight;i++) {//初始化
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                equalTable[i][j] = 0;
            }
        }

        int nextGroupNumber = 1;

        for (int i = 1; i < infoHeader.imageHeight - 1; i++) {
            for (int j = 1; j < infoHeader.imageWidth - 1; j++) {
                if (getDataValue(i, j) != srcValue) {
                    if (getDataValue(i, j - 1) == srcValue &&
                        getDataValue(i - 1, j - 1) == srcValue &&
                        getDataValue(i - 1, j) == srcValue &&
                        getDataValue(i - 1, j + 1) == srcValue
                        ) {
                        equalTable[i][j] = nextGroupNumber;
                        nextGroupNumber++;
                    } else if (getDataValue(i, j - 1) != srcValue) {
                        equalTable[i][j] = equalTable[i][j - 1];
                    } else if (getDataValue(i - 1, j - 1) != srcValue) {
                        equalTable[i][j] = equalTable[i - 1][j - 1];
                    } else if (getDataValue(i - 1, j) != srcValue) {
                        equalTable[i][j] = equalTable[i - 1][j];
                    } else if (getDataValue(i, j + 1) != srcValue) {
                        equalTable[i][j] = equalTable[i - 1][j + 1];
                    }

                }
            }
        }


        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                bmp.setDataValue(i, j, equalTable[i][j] * 60);
            }
        }
        swap(*this,bmp);
    }
    void bmpHough(int num) {
        BMP bmp = bmp.Copy();
        int sk = sqrt(infoHeader.imageHeight * infoHeader.imageHeight + infoHeader.imageWidth * infoHeader.imageWidth);
        vector<vector<int>> grey(sk, vector<int>(360));
        for (int i = 0;i < infoHeader.imageHeight;i++) {
            for (int j = 0;j < infoHeader.imageWidth;j++) {
                if (0 == getDataValue(i, j)) {
                    for (int m = 0;m < sk;m++) {
                        for (int n = 0;n < 360;n++) {
                            int m1 = j * cos(n) + i * sin(n);
                            if (abs(m - m1) < 1) grey[m][n]++;
                        }
                    }
                }
                setDataValue(i, j, 255);
            }
        }
        for (int m = 0;m < sk;m++) {
            for (int n = 0;n < 360;n++) {
                if (grey[m][n] >= num) {
                    for (int i = 0;i < infoHeader.imageHeight;i++) {
                        for (int j = 0;j < infoHeader.imageWidth;j++) {
                            int m1 = j * cos(n) + i * sin(n);
                            if (abs(m - m1) < 1) {
                                setDataValue(i, j, 0);
                            }
                        }
                    }
                }
            }
        }
    }
};
map<string, string> helpInfoMap;
map<string, function<void(vector<string>)>> commandMap;
map<string, int> commandParamNums;
set<string> commandStrings;
vector<string> commands;
void registerCommand(string commandString, string tip, function<void(vector<string>)> fn, int paramNum) {
    if (commandStrings.count(commandString)) {
        cerr << "You cannont register a command that already exists." << endl;
        return;
    }
    helpInfoMap[commandString] = tip;
    commandMap[commandString] = fn;
    commandParamNums[commandString] = paramNum;
    commandStrings.insert(commandString);
    commands.push_back(commandString);
}
void mainFrameShow() {
    cout << "Tips:help for more information" << endl;
    string str;
    while (getline(cin, str) && str != "exit") {
        if (str == "") continue;
        stringstream ss;
        ss << str;
        string command;
        ss >> command;
        vector<string> params;
        string tmp;
        while (ss >> tmp) {
            params.push_back(tmp);
        }
        if (!commandStrings.count(command)) {
            cout << "command:" << command << " not found.Try help for more infomation" << endl;
        } else {
            if (params.size() != commandParamNums[command]) {
                cout << "The command is used incorrectly" << endl;
            } else {
                try {
                    commandMap[command](params);
                } catch (exception e) {
                    cout << "The command is used incorrectly" << endl;
                }
            }
        }
    }
}
void init() {
    auto help = [](vector<string>) {
        for (auto command : commands) {
            cout << command << ": " << "\t" << helpInfoMap[command] << endl;
        }
    };
    registerCommand("help", "[]show help infomation", help, 0);
    auto convert24To8 = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpConverter24To8();
        bmp.writeToFile(params[1]);
    };
    registerCommand("convert24To8", "[source_file_path,target_path]: convert a 24-bit bmp to 8-bit", convert24To8, 2);
    auto reverseColor = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpReverseColorWithPalette();
        bmp.writeToFile(params[1]);
    };
    registerCommand("reverseColor", "[source_file_path,target_path]: reverse color of a pic", reverseColor, 2);
    auto rgbSplit = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpRGBSplit(params[2][0]);
        bmp.writeToFile(params[1]);
    };
    registerCommand("rgbSplit", "[source_file_path,target_path,'R'/'G'/'B']: split R/G/B color of a pic", rgbSplit, 3);
    auto drawGreyHist = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpDrawGreyHist(params[1], bmp.bmpCountGrayHist());
    };
    registerCommand("drawGreyHist", "[source_file_path,target_path]: draw the grey hist of a pic", drawGreyHist, 2);
    auto histogramEqualization = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpHistogramEqualizationProcessing();
        bmp.writeToFile(params[1]);
    };
    registerCommand("histogramEqualization", "[source_file_path,target_path]: process histogramEqualization on a pic", histogramEqualization, 2);
    auto averageFilter = [](vector<string> params) {
        BMP bmp(params[0]);
        auto templat = BMP().generateFilterTemplate(5, 5, vector<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 25);
        bmp.filter(templat);
        bmp.writeToFile(params[1]);
    };
    registerCommand("averageFilter", "[source_file_path,target_path]: process averageFiltering on a pic", averageFilter, 2);
    auto medianFilter = [](vector<string> params) {
        BMP bmp(params[0]);
        auto fn = [](vector<int> vec) {
            sort(vec.begin(), vec.end());
            return vec[vec.size() / 2];
        };
        if (bmp.getImageBitCount() == 24) bmp.bmpConverter24To8();
        bmp.filterStatistic(fn, 3, 3);
        bmp.writeToFile(params[1]);
    };
    registerCommand("medianFilter", "[source_file_path,target_path]: process medianFiltering on a pic", medianFilter, 2);
    auto prewittOperate = [](vector<string> params) {
        int threshold = 175;
        auto prewittOperator = [&threshold](auto getDataValue, int x, int y)->int {
            int ans1 = 0, ans2 = 0;
            for (int i = -1;i <= 1;i++) ans1 += getDataValue(x - 1, y + i);
            for (int i = -1;i <= 1;i++) ans1 += -getDataValue(x + 1, y + i);
            for (int i = -1;i <= 1;i++) ans2 += -getDataValue(x + i, y - 1);
            for (int i = -1;i <= 1;i++) ans2 += getDataValue(x + i, y + 1);
            return max(abs(ans1), abs(ans2)) > threshold ? 255 : 0;
        };
        BMP bmp(params[0]);
        bmp.operateThroughOperator(prewittOperator);
        bmp.writeToFile(params[1]);
    };
    registerCommand("prewittOperate", "[source_file_path,target_path]: process prewittOperating on a pic", prewittOperate, 2);
    auto sobelOperate = [](vector<string> params) {
        int threshold = 175;
        auto sobelOperator = [&threshold](auto getDataValue, int x, int y)->int {
            int ans1 = 0, ans2 = 0;
            for (int i = -1;i <= 1;i++) ans1 += getDataValue(x - 1, y + i) - getDataValue(x + 1, y + i);
            ans1 += getDataValue(x - 1, y) - getDataValue(x + 1, y);
            for (int i = -1;i <= 1;i++) ans2 += getDataValue(x + i, y + 1) - getDataValue(x + i, y - 1);
            ans2 += getDataValue(x, y + 1) - getDataValue(x, y - 1);
            return sqrt(ans1 * ans1 + ans2 * ans2) > threshold ? 255 : 0;
        };
        BMP bmp(params[0]);
        bmp.operateThroughOperator(sobelOperator);
        bmp.writeToFile(params[1]);
    };
    registerCommand("sobelOperate", "[source_file_path,target_path]: process sobelOperating on a pic", sobelOperate, 2);
    auto logOperate = [](vector<string> params) {
        int threshold = 175;
        auto logOperator = [&threshold](auto getDataValue, int x, int y)->int {
            int ans = 0;
            for (int i = -2;i <= 2;i++) {
                for (int j = -2;j <= 2;j++) {
                    int dis = abs(i) + abs(j);
                    if (dis == 2) ans -= getDataValue(i + x, j + y);
                    else if (dis == 1) ans -= 2 * getDataValue(i + x, j + y);
                    else if (dis == 0) ans += 16 * getDataValue(i + x, j + y);
                }
            }
            return abs(ans) > threshold ? 255 : 0;
        };
        BMP bmp(params[0]);
        bmp.operateThroughOperator(logOperator);
        bmp.writeToFile(params[1]);
    };
    registerCommand("logOperate", "[source_file_path,target_path]: process logOperating on a pic", logOperate, 2);
    auto changeSize = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpChangeSize(stoi(params[2]), stoi(params[3]));
        bmp.writeToFile(params[1]);
    };
    registerCommand("changeSize", "[source_file_path,target_path,fx,fy]: changeSize of a pic", changeSize, 4);
    auto move = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpMove(stoi(params[2]), stoi(params[3]));
        bmp.writeToFile(params[1]);
    };
    registerCommand("move", "[source_file_path,target_path,fx,fy]: process moving on a pic", move, 4);
    auto mirror = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpMirror();
        bmp.writeToFile(params[1]);
    };
    registerCommand("mirror", "[source_file_path,target_path]: generate a mirror of a pic", move, 2);
    auto rotate = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpRotate(stoi(params[2]), stoi(params[3]), stoi(params[4]));
        bmp.writeToFile(params[1]);
    };
    registerCommand("rotate", "[source_file_path,target_path, x,y,alpha]: generate a rotation of a pic", rotate, 5);
    auto threholdCut = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpThreholdCut(params[2]);
        bmp.writeToFile(params[1]);
    };
    registerCommand("threholdCut", "[source_file_path,target_path, hist_path]", threholdCut, 3);
    auto seedGrow = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpSeedGrow();
        bmp.writeToFile(params[1]);
    };
    registerCommand("seedGrow", "[source_file_path,target_path]: process seed growing on a pic", seedGrow, 2);
    auto separate = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpSeparate(255);
        bmp.writeToFile(params[1]);
    };
    registerCommand("separate", "[source_file_path,target_path]: process separation on a pic", separate, 2);
    auto hough = [](vector<string> params) {
        BMP bmp(params[0]);bmp.bmpHough(stoi(params[2]));
        bmp.writeToFile(params[1]);
    };
    registerCommand("hough", "[source_file_path,target_path,hough_threhold]: process hough on a pic", hough, 3);
    auto zoneMark = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpZoneMark();
        bmp.writeToFile(params[1]);
    };
    registerCommand("zoneMark","[source_file_path,target_path]:process zone marking on a pic",zoneMark,2);
    auto contourExtraction = [](vector<string> params) {
        BMP bmp(params[0]);
        bmp.bmpContourExtraction();
        bmp.writeToFile(params[1]);
    };
    registerCommand("contourExtraction","[source_file_path,target_path]:process contourExtraction on a pic",contourExtraction,2);
    auto all = [](vector<string> params) {
        auto getOutputPath = [](string filename)->string {return "./resources/results/" + filename + ".bmp";};
        auto getInputPath = [](string filename)->string {return "./resources/" + filename + ".bmp";};
        {
            BMP bmp(getInputPath("P1"));
            auto tmp = bmp.Copy();tmp.bmpRGBSplit('R'), tmp.writeToFile(getOutputPath("P1_3"));
            tmp = bmp.Copy();tmp.bmpRGBSplit('G'), tmp.writeToFile(getOutputPath("P1_4"));
            tmp = bmp.Copy();tmp.bmpRGBSplit('B'), tmp.writeToFile(getOutputPath("P1_5"));
            bmp.bmpConverter24To8(), bmp.writeToFile(getOutputPath("P1_1"));
            tmp = bmp.Copy();tmp.bmpReverseColorWithPalette(), tmp.writeToFile(getOutputPath("P1_2"));
        }
        {
            BMP bmp(getInputPath("P2"));
            bmp.bmpDrawGreyHist(getOutputPath("P2_1"),bmp.bmpCountGrayHist());
            bmp.bmpHistogramEqualizationProcessing();
            bmp.writeToFile(getOutputPath("P2_2"));
            bmp.bmpDrawGreyHist(getOutputPath("P2_3"),bmp.bmpCountGrayHist());
        }
        {
            BMP bmp(getInputPath("P3_1"));
            auto templat = BMP().generateFilterTemplate(5, 5, vector<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 25);
            bmp.filter(templat);
            bmp.writeToFile(getOutputPath("P3_1"));
            BMP bmp1(getInputPath("P3_2"));
            auto fn = [](vector<int> vec) {
                sort(vec.begin(), vec.end());
                return vec[vec.size() / 2];
            };
            bmp1.bmpConverter24To8();
            bmp1.filterStatistic(fn, 3, 3);
            bmp1.writeToFile(getOutputPath("P3_2"));
        }
        {
            BMP bmp1(getInputPath("P4")),bmp;
            bmp = bmp1.Copy();
            bmp.bmpChangeSize(0.5, 0.5),bmp.writeToFile(getOutputPath("P4_1"));
            bmp = bmp1.Copy();
            bmp.bmpChangeSize(2, 2),bmp.writeToFile(getOutputPath("P4_2"));
            bmp = bmp1.Copy();
            bmp.bmpMove(50,50),bmp.writeToFile(getOutputPath("P4_3"));
            bmp = bmp1.Copy();
            bmp.bmpMirror(),bmp.writeToFile(getOutputPath("P4_4"));
            bmp = bmp1.Copy();
            bmp.bmpRotate(50, 50, 70),bmp.writeToFile(getOutputPath("P4_5"));
        }
        {
            BMP bmp(getInputPath("P5"));
            bmp.bmpThreholdCut(getOutputPath("P5_2"));
            bmp.writeToFile(getOutputPath("P5_1"));
        }
        {
            BMP bmp(getInputPath("P6"));
            bmp.bmpConverter24To8(), bmp.bmpSeedGrow(), bmp.writeToFile(getOutputPath("P6_1"));
            BMP bmp1(getInputPath("P6"));
            bmp1.bmpConverter24To8(), bmp1.bmpSeparate(255), bmp1.writeToFile(getOutputPath("P6_2"));
        }
        {
            int threshold = 175;
            auto prewittOperator = [&threshold](auto getDataValue, int x, int y)->int {
                int ans1 = 0, ans2 = 0;
                for (int i = -1;i <= 1;i++) ans1 += getDataValue(x - 1, y + i);
                for (int i = -1;i <= 1;i++) ans1 += -getDataValue(x + 1, y + i);
                for (int i = -1;i <= 1;i++) ans2 += -getDataValue(x + i, y - 1);
                for (int i = -1;i <= 1;i++) ans2 += getDataValue(x + i, y + 1);
                return max(abs(ans1), abs(ans2)) > threshold ? 255 : 0;
            };
            auto sobelOperator = [&threshold](auto getDataValue, int x, int y)->int {
                int ans1 = 0, ans2 = 0;
                for (int i = -1;i <= 1;i++) ans1 += getDataValue(x - 1, y + i) - getDataValue(x + 1, y + i);
                ans1 += getDataValue(x - 1, y) - getDataValue(x + 1, y);
                for (int i = -1;i <= 1;i++) ans2 += getDataValue(x + i, y + 1) - getDataValue(x + i, y - 1);
                ans2 += getDataValue(x, y + 1) - getDataValue(x, y - 1);
                return sqrt(ans1 * ans1 + ans2 * ans2) > threshold ? 255 : 0;
            };
            auto logOperator = [&threshold](auto getDataValue, int x, int y)->int {
                int ans = 0;
                for (int i = -2;i <= 2;i++) {
                    for (int j = -2;j <= 2;j++) {
                        int dis = abs(i) + abs(j);
                        if (dis == 2) ans -= getDataValue(i + x, j + y);
                        else if (dis == 1) ans -= 2 * getDataValue(i + x, j + y);
                        else if (dis == 0) ans += 16 * getDataValue(i + x, j + y);
                    }
                }
                return abs(ans) > threshold ? 255 : 0;
            };
            BMP bmp(getInputPath("P7")),bmp1 = bmp.Copy(),bmp2 = bmp.Copy();
            bmp.operateThroughOperator(prewittOperator),bmp1.operateThroughOperator(sobelOperator),bmp2.operateThroughOperator(logOperator);
            bmp.writeToFile(getOutputPath("P7_1")),bmp1.writeToFile(getOutputPath("P7_2")),bmp2.writeToFile(getOutputPath("P7_3"));
        }
        {
            int threhold = 55;
            BMP bmp(getInputPath("P8"));
            bmp.bmpConverter24To8(), bmp.bmpHough(55),bmp.writeToFile(getOutputPath("P8"));
        }
        {
            BMP bmp(getInputPath("P9"));
            bmp.bmpConverter24To8(),bmp.bmpZoneMark(),bmp.writeToFile(getOutputPath("P9"));
        }
        {
            BMP bmp(getInputPath("P10"));
            bmp.bmpConverter24To8(),bmp.bmpContourExtraction(),bmp.writeToFile(getOutputPath("P10"));
        }
    };
    registerCommand("all", "[]execute all in default", all, 0);
}
signed main() {
    init(); 
    mainFrameShow();
    return 0;
}