// Exporter compatible with BMFont

#ifndef BMFONTEXPORTER_H
#define BMFONTEXPORTER_H

#include "../abstractexporter.h"

class BmFontExporter : public AbstractExporter
{
    Q_OBJECT
public:
    explicit BmFontExporter(QObject *parent = 0);

    virtual bool Export(QByteArray& out);
signals:

public slots:

};

#endif // BMFONTEXPORTER_H
