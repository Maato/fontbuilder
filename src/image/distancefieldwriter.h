#ifndef DISTANCEFIELDWRITER_H
#define DISTANCEFIELDWRITER_H

#include "../abstractimagewriter.h"

class DistanceFieldWriter : public AbstractImageWriter
{
Q_OBJECT
public:
    DistanceFieldWriter(QString format, QObject *parent = 0);

    virtual bool Export(QFile& file);
    virtual QImage* reload(QFile& file);
private:
    QString m_format;
signals:

public slots:

};

#endif // DISTANCEFIELDWRITER_H
