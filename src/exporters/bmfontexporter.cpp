#include "bmfontexporter.h"
#include "../fontconfig.h"
#include "../layoutconfig.h"
#include <QStringBuilder>

BmFontExporter::BmFontExporter(QObject *parent) :
    AbstractExporter(parent)
{
    setExtension("fnt");
}

bool BmFontExporter::Export(QByteArray& out) {
    QString res = QString("info ");
    {
        res += QString("face=\"")   + fontConfig()->family()                        + QString("\" ");
        res += QString("size=")     + QString().number(fontConfig()->size())        + QString(" ");
        res += QString("bold=")     + QString().number(fontConfig()->bold())        + QString(" ");
        res += QString("italic=")   + QString().number(fontConfig()->italic())      + QString(" ");
        res += QString("charset=")  + QString("\"\"")                               + QString(" "); 
        res += QString("unicode=")  + QString().number(1)                           + QString(" ");
        res += QString("stretchH=") + QString().number(100)                         + QString(" ");
        res += QString("smooth=")   + QString().number(fontConfig()->antialiased()) + QString(" ");
        res += QString("aa=")       + QString().number(1)                           + QString(" ");
        res += QString("padding=");
        {
            res += QString().number(layoutConfig()->offsetLeft())   + QString(",");
            res += QString().number(layoutConfig()->offsetTop())    + QString(",");
            res += QString().number(layoutConfig()->offsetRight())  + QString(",");
            res += QString().number(layoutConfig()->offsetBottom()) + QString(" ");
        }
        res += QString("spacing=")  + QString().number(0) + QString(" ");
        res += QString("outline=")  + QString().number(0) + QString(" ");
        res += QString("\n");
    }

    res += QString("common ");
    {
        res += QString("lineHeight=") + QString().number(metrics().height) + QString(" ");
        res += QString("base=")       + QString().number(0)                + QString(" ");
        res += QString("scaleW=")     + QString().number(texWidth())       + QString(" ");
        res += QString("scaleH=")     + QString().number(texHeight())      + QString(" ");
        res += QString("pages=")      + QString().number(1)                + QString(" ");
        res += QString("packed=")     + QString().number(0)                + QString(" ");
        res += QString("alphaChnl=")  + QString().number(0)                + QString(" ");
        res += QString("redChnl=")    + QString().number(0)                + QString(" ");
        res += QString("greenChnl=")  + QString().number(0)                + QString(" ");
        res += QString("blueChnl=")   + QString().number(0)                + QString(" ");
        res += QString("\n");
    }
    res += QString("page id=0 file=\"") + texFilename()                      + QString("\"\n");
    res += QString("chars count=")      + QString().number(symbols().size()) + QString("\n");
    
	QString kernings = QString("");
    int kernNumber = 0;
    foreach (const Symbol& c , symbols()) {
        QString charDef = QString("char ");
        charDef += QString("id=")       + QString().number(c.id)      + QString(" ");
        charDef += QString("x=")        + QString().number(c.placeX)  + QString(" ");
        charDef += QString("y=")        + QString().number(c.placeY)  + QString(" ");
        charDef += QString("width=")    + QString().number(c.placeW)  + QString(" ");
        charDef += QString("height=")   + QString().number(c.placeH)  + QString(" ");
        charDef += QString("xoffset=")  + QString().number(c.offsetX) + QString(" ");
        charDef += QString("yoffset=")  + QString().number(-c.offsetY) + QString(" ");
        charDef += QString("xadvance=") + QString().number(c.advance) + QString(" ");
        charDef += QString("page=")     + QString().number(0)         + QString(" ");
        charDef += QString("chnl=")     + QString().number(15);
        res += charDef + QString("\n");
        
        typedef QMap<ushort,int>::ConstIterator Kerning;
        for(Kerning k = c.kerning.begin(); k != c.kerning.end(); k++) {
            kernings += QString("kerning first=") + QString().number(c.id)      + QString(" ");
            kernings += QString("second=")        + QString().number(k.key())   + QString(" ");
            kernings += QString("amount=")        + QString().number(k.value()) + QString("\n");
            kernNumber++;
        }
    }

    res += QString("kernings count=") + QString().number(kernNumber) + QString("\n");
    res += kernings;

    out = res.toUtf8();
    return true;
}

AbstractExporter* BmFontExporterFactoryFunc (QObject* parent) {
    return new BmFontExporter(parent);
}
