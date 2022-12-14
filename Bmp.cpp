#include<cmath>
#include<iostream>
#include<functional>
#include<stdint.h>
#include<fstream>
#include<vector>
#include<string>
#include<set>
#include<queue>
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
        cerr << pos << endl;
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
                if(y==other.y) return x<other.x;
                else return y<other.y;
            }
        }node;
        for(int i=0;i<data.size();i++) data[i] = 0;
        int dx[] = {0,0,1,-1};
        int dy[] = {1,-1,0,0};
        queue<node> q;
        set<node> vis;
        for(int x=0;x<infoHeader.imageHeight;x++) {
            for(int y=0;y<infoHeader.imageWidth;y++) {
                if(bmp.getDataValue(x,y)>230) {q.push({x,y}),vis.insert({x,y});}
            }
        }
        while(!q.empty()) {
            node now = q.front();
            setDataValue(now.x,now.y,255);
            q.pop();
            for(int i=0;i<4;i++) {
                int x1 = now.x+dx[i];
                int y1 = now.y+dy[i];
                if(x1<0||x1>=infoHeader.imageHeight||y1<0||y1>=infoHeader.imageWidth) continue;
                if(vis.count({x1,y1})) continue;
                if(abs(bmp.getDataValue(x1,y1)-bmp.getDataValue(now.x,now.y))<=11) {
                    q.push({x1,y1});
                    vis.insert({x1,y1});
                }
            }
        }
    }
    void bmpSeparate() {
        auto fn = [&](int x1,int y1,int x2,int y2) {
            int minv = getDataValue(x1,y1);
            int maxv = getDataValue(x1,y1);
            auto chmin = [](auto x,auto y){return x>y?y:x;};
            auto chmax = [](auto x,auto y){return x>y?x:y;};
            for(int i=x1;i<infoHeader.imageHeight;i++) {
                for(int j=y1;j<infoHeader.imageWidth;j++) {
                    minv = chmin(minv,getDataValue(i,j));
                    maxv = chmax(maxv,getDataValue(i,j));
                }
            }
            return maxv-minv<=25;
        };
        int x1 = 0,y1 = 0,x2 = infoHeader.imageHeight-1,y2 = infoHeader.imageWidth-1;
        typedef struct Node {
            int x1, y1,x2,y2;const bool operator<(Node other)const {
                if(x1!=other.x1) return x1<other.x1;
                if(y1!=other.y1) return y1<other.y1;
                if(x2!=other.x2) return x2<other.x2;
                if(y2!=other.y2) return y2<other.y2;
            }
        }node;
        queue<node> q;
        q.push({x1,y1,x2,y2});
        vector<node> vis;
        set<node> test;
        while(!q.empty()){
            vis.push_back(q.front());
            auto [x1,y1,x2,y2] = q.front();
            // cout<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
            q.pop();
            if(!fn(x1,y1,x2,y2)) {
                int x3 = (x1+x2)/2;
                int y3 = (y1+y2)/2;
                q.push({x1,y1,x3,y3});
                q.push({x3+1,y1,x2,y3});
                q.push({x1,y3+1,x3,y2});
                q.push({x3+1,y3+1,x2,y2});
            }
        }   
        for(auto &[x1,y1,x2,y2]:vis) {
            for(int i=x1;i<=x2;i++) setDataValue(i,y1,0),setDataValue(i,y2,0);
            for(int i=y1;i<=y2;i++) setDataValue(x1,i,0),setDataValue(x2,i,0);
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
    bmp.bmpDrawGreyHist("./resources/P2/dim_hist.bmp", bmp.bmpCountGrayHist());
    bmp.bmpHistogramEqualizationProcessing();
    bmp.writeToFile("./resources/P2/dim1.bmp");
    bmp.bmpDrawGreyHist("./resources/P2/dim1_hist.bmp", bmp.bmpCountGrayHist());
}
void homework3() {
    BMP bmp("./resources/P3/lena.bmp");
    auto templat = BMP().generateFilterTemplate(5, 5, vector<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 25);
    bmp.filter(templat);
    bmp.writeToFile("./resources/P3/lena_res.bmp");
    BMP bmp1("./resources/P3/lena1.bmp");
    auto fn = [](vector<int> vec) {
        sort(vec.begin(), vec.end());
        return vec[vec.size() / 2];
    };
    bmp1.bmpConverter24To8();
    bmp1.filterStatistic(fn, 3, 3);
    bmp1.writeToFile("./resources/P3/lena1_res.bmp");
}
void homework4() {
    int threshold = 175;
    /*
    prewitt:
        1   1   1   -1  0   1
        0   0   0   -1  0   1
        -1  -1  -1  -1  0   1
    */
    auto prewittOperator = [&threshold](auto getDataValue, int x, int y)->int {
        int ans1 = 0, ans2 = 0;
        for (int i = -1;i <= 1;i++) ans1 += getDataValue(x - 1, y + i);
        for (int i = -1;i <= 1;i++) ans1 += -getDataValue(x + 1, y + i);
        for (int i = -1;i <= 1;i++) ans2 += -getDataValue(x + i, y - 1);
        for (int i = -1;i <= 1;i++) ans2 += getDataValue(x + i, y + 1);
        return max(abs(ans1), abs(ans2)) > threshold ? 255 : 0;
    };
    BMP bmp("./resources/P4/lena.bmp");
    bmp.operateThroughOperator(prewittOperator);
    bmp.writeToFile("./resources/P4/prewitt.bmp");
    /*
    sobel:
        1   2   1   -1  0   1
        0   0   0   -2  0   2
        -1  -2  -1  -1  0   1
    */
    auto sobelOperator = [&threshold](auto getDataValue, int x, int y)->int {
        int ans1 = 0, ans2 = 0;
        for (int i = -1;i <= 1;i++) ans1 += getDataValue(x - 1, y + i) - getDataValue(x + 1, y + i);
        ans1 += getDataValue(x - 1, y) - getDataValue(x + 1, y);
        for (int i = -1;i <= 1;i++) ans2 += getDataValue(x + i, y + 1) - getDataValue(x + i, y - 1);
        ans2 += getDataValue(x, y + 1) - getDataValue(x, y - 1);
        return sqrt(ans1 * ans1 + ans2 * ans2) > threshold ? 255 : 0;
    };
    BMP bmp1("./resources/P4/lena.bmp");
    bmp1.operateThroughOperator(sobelOperator);
    bmp1.writeToFile("./resources/P4/sobel.bmp");
    /* log
        0 0  -1  0 0
        0 -1 -2 -1 0
        0 -2 16 -2 0
        0 -1 -2 -1 0
        0 0  -1  0 0
    */
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
    BMP bmp2("./resources/P4/lena.bmp");
    bmp2.operateThroughOperator(logOperator);
    bmp2.writeToFile("./resources/P4/log.bmp");
}
void homework5() {
    BMP bmp("./resources/P5/lena.bmp");
    bmp.bmpChangeSize(0.5, 0.5);
    bmp.writeToFile("./resources/P5/size1.bmp");
    BMP bmp1("./resources/P5/lena.bmp");
    bmp1.bmpChangeSize(2, 2);
    bmp1.writeToFile("./resources/P5/size2.bmp");
    BMP bmp2("./resources/P5/lena.bmp");
    bmp2.bmpMove(50, 50);
    bmp2.writeToFile("./resources/P5/move.bmp");
    BMP bmp3("./resources/P5/lena.bmp");
    bmp3.bmpMirror();
    bmp3.writeToFile("./resources/P5/mirror.bmp");
    BMP bmp4("./resources/P5/lena.bmp");
    bmp4.bmpRotate(50, 50, 70);
    bmp4.writeToFile("./resources/P5/rotate.bmp");
}
void homework6() {
    BMP bmp("./resources/P6/lena.bmp");
    bmp.bmpThreholdCut("./resources/P6/hist.bmp");
    bmp.writeToFile("./resources/P6/threholdCut.bmp");
}
void homework7() {
    // BMP bmp("./resources/P7/source.bmp");
    // bmp.bmpConverter24To8();
    // bmp.bmpSeedGrow();
    // bmp.writeToFile("./resources/P7/grow.bmp");
    BMP bmp1("./resources/P7/source.bmp");
    bmp1.bmpConverter24To8();
    bmp1.bmpSeparate();
    bmp1.writeToFile("./resources/P7/seprate.bmp");

}



signed main() {
    // homework1();
    // homework2();
    // homework3();
    // homework4();
    // homework5();
    // homework6();
    homework7();
    return 0;
}

