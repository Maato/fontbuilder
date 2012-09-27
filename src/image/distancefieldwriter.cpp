#include "distancefieldwriter.h"
#include "layoutdata.h"
#include <QPainter>
#include <QtCore/qmath.h>
#include "../layoutconfig.h"
#include <QDebug>
#include <limits>

DistanceFieldWriter::DistanceFieldWriter(QString format,QObject *parent) :
    AbstractImageWriter(parent)
{
    setExtension("png");
    setReloadSupport(false);
    m_format = format;
}

class DistanceField {
    struct Distance { 
        short x, y;
        inline int GetDistSquared() const { int _x = x; int _y = y; return _x*_x + _y*_y; }
    };
    int mWidth;
    int mHeight;
    Distance * mInside;
    Distance * mOutside;
public:
    DistanceField(int width, int height) :
        mWidth(width+2),
        mHeight(height+2)
    {
        mInside = new Distance[mWidth * mHeight];
        mOutside = new Distance[mWidth * mHeight];
        
        const Distance close = { 0, 0 };
        const Distance far = { SHRT_MAX, SHRT_MAX };
        
        for(int y = 0; y < mHeight; y+=mHeight-1) {
            for(int x = 0; x < mWidth; x++) {
                mInside[x + y * mWidth] = close;
                mOutside[x + y * mWidth] = far;
            }
        }
        for(int x = 0; x < mWidth; x+= mWidth-1) {
            for(int y = 0; y < mHeight; y++) {
                mInside[x + y * mWidth] = close;
                mOutside[x + y * mWidth] = far;
            }
        }
    }
    ~DistanceField()
    {
        delete[] mInside;
        delete[] mOutside;
    }
    
    int GetDistance(int x, int y)
    {
        if(x < 0 || x >= mWidth-2 || y < 0 || y >= mHeight-2) 
            return 0;
        x+=1; y+=1;
        int in = mInside[x + y * mWidth].GetDistSquared();
        int out = mOutside[x + y * mWidth].GetDistSquared();
        return (int)roundf(sqrtf(in) - sqrtf(out));
    }
    
    void SetPixel(int x, int y, bool value)
    {
        if(x < 0 || x >= mWidth-2 || y < 0 || y >= mHeight-2)
            return;
        x+=1; y+=1;
        const Distance close = { 0, 0 };
        const Distance far = { SHRT_MAX, SHRT_MAX };
        if(value) {
            mInside[x + y * mWidth] = close;
            mOutside[x + y * mWidth] = far;
        } else {
            mInside[x + y * mWidth] = far;
            mOutside[x + y * mWidth] = close;
        }
    }
    
    void Generate() {
        Generate(mInside);
        Generate(mOutside);
    }

private:
    inline Distance GetOffset(Distance * data, int x, int y, int ox, int oy) {
        Distance other = data[(x+ox) + (y+oy) * mWidth];
        other.x += ox;
        other.y += oy;
        return other;
    }

    void Generate(Distance * data)
    {
        for (int y = 1; y < mHeight-1; y++) {
            for (int x = 1; x < mWidth-1; x++) {
                Distance p = data[x + y * mWidth];
                Distance up =      GetOffset(data, x, y,  0, -1);
                Distance left =    GetOffset(data, x, y, -1,  0);
                Distance upleft =  GetOffset(data, x, y, -1, -1);
                Distance upright = GetOffset(data, x, y,  1, -1);
                int dist = p.GetDistSquared();
                if(left.GetDistSquared() < dist)   { p = left;    dist = p.GetDistSquared(); }
                if(up.GetDistSquared() < dist)     { p = up;      dist = p.GetDistSquared(); }
                if(upleft.GetDistSquared() < dist) { p = upleft;  dist = p.GetDistSquared(); }
                if(upright.GetDistSquared() < dist){ p = upright; }
                data[x + y * mWidth] = p;
            }
    
            for (int x = mWidth - 2; x >= 1; x--) {
                Distance right = GetOffset(data, x, y, 1, 0);
                if(right.GetDistSquared() < data[x + y * mWidth].GetDistSquared())
                    data[x + y * mWidth] = right;
            }
        }
    
        for (int y = mHeight - 2; y >= 1; y--) {
            for (int x = mWidth - 2; x >= 1; x--) {
                Distance p = data[x + y * mWidth];
                Distance right =     GetOffset(data, x, y,  1, 0);
                Distance down =      GetOffset(data, x, y,  0, 1);
                Distance downleft =  GetOffset(data, x, y, -1, 1);
                Distance downright = GetOffset(data, x, y,  1, 1);
                int dist = p.GetDistSquared();
                if(right.GetDistSquared() < dist)     { p = right;     dist = p.GetDistSquared(); }
                if(down.GetDistSquared() < dist)      { p = down;      dist = p.GetDistSquared(); }
                if(downleft.GetDistSquared() < dist)  { p = downleft;  dist = p.GetDistSquared(); }
                if(downright.GetDistSquared() < dist) { p = downright; }
                data[x + y * mWidth] = p;
            }
            for (int x = 1; x < mWidth - 1; x++) {
                Distance left = GetOffset(data, x, y, -1, 0);
                if(left.GetDistSquared() < data[x + y * mWidth].GetDistSquared())
                    data[x + y * mWidth] = left;
            }
        }
    }
};


bool DistanceFieldWriter::Export(QFile& file) {
    QImage pixmap = buildImage();
    
    DistanceField * field = new DistanceField(pixmap.width(), pixmap.height());
    for(int y = 0; y < pixmap.height(); y++) {
        const QRgb * scanline = (QRgb*)pixmap.scanLine(y);
        for(int x = 0; x < pixmap.width(); x++) {
            field->SetPixel(x, y, qAlpha(scanline[x]) <= 0);
        }        
    }
    field->Generate();
    QImage largePixmap(pixmap.width(),pixmap.height(),QImage::Format_Indexed8);
    largePixmap.setColorCount(256);
    for(int i = 0; i < 256; i++) largePixmap.setColor(i, qRgb(i,i,i));
    for(int y = 0; y < pixmap.height(); y++) {
        for(int x = 0; x < pixmap.width(); x++) {
            int dist = field->GetDistance(x, y) + 128;
            if(dist < 0) dist = 0;
            if(dist > 255) dist = 255;
            largePixmap.setPixel(x, y, dist);
        }
    }
    delete field;
    
    /* Scale the large distance field down */
    QImage smallPixmap = largePixmap.scaled(pixmap.width() / 16, pixmap.height() / 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
    /* Convert the small pixmap to indexed format */
    QImage small8BitPixmap(smallPixmap.width(), smallPixmap.height(), QImage::Format_Indexed8);
    for(int i = 0; i < 256; i++) small8BitPixmap.setColor(i, qRgb(i,i,i));
    for(int y = 0; y < smallPixmap.height(); y++) {
        const QRgb * scanline = (QRgb*)smallPixmap.scanLine(y);
        for(int x = 0; x < smallPixmap.width(); x++) {
            small8BitPixmap.setPixel(x, y, qRed(scanline[x]));
        }
    }
    small8BitPixmap.save(&file, m_format.toUtf8().data());
    
    return true;
}

QImage* DistanceFieldWriter::reload(QFile& file) {
    QImage* img = new QImage();
    if (img->load(&file,m_format.toUtf8().data()))
        return img;
    delete img;
    return 0;
}
