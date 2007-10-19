/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of $PRODUCT$.
**
** $CPP_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GENERATOR_H
#define GENERATOR_H

#include "metajava.h"
#include "typesystem.h"

#include "codemodel.h"

#include <QObject>
#include <QFile>

class Generator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory);

public:
    enum Option {
        NoOption                 = 0x00000000,
        BoxedPrimitive           = 0x00000001,
        ExcludeConst             = 0x00000002,
        ExcludeReference         = 0x00000004,
        UseNativeIds             = 0x00000008,

        EnumAsInts               = 0x00000010,
        SkipName                 = 0x00000020,
        NoCasts                  = 0x00000040,
        SkipReturnType           = 0x00000080,
        OriginalName             = 0x00000100,
        ShowStatic               = 0x00000200,
        UnderscoreSpaces         = 0x00000400,
        ForceEnumCast            = 0x00000800,
        ArrayAsPointer           = 0x00001000,
        VirtualCall              = 0x00002000,
        SkipTemplateParameters   = 0x00004000,
        SkipAttributes           = 0x00008000,
        OriginalTypeDescription  = 0x00010000,
        SkipRemovedArguments     = 0x00020000,
        IncludeDefaultExpression = 0x00040000,
        NoReturnStatement        = 0x00080000,
        NoBlockedSlot            = 0x00100000,

        GlobalRefJObject         = 0x00100000,

        ForceValueType           = ExcludeReference | ExcludeConst
    };

    Generator();

    void setClasses(const AbstractMetaClassList &classes) { m_classes = classes; }
    AbstractMetaClassList classes() const { return m_classes; }

    QString outputDirectory() const { return m_out_dir; }
    void setOutputDirectory(const QString &outDir) { m_out_dir = outDir; }
    virtual void generate();
    void printClasses();

    int numGenerated() { return m_num_generated; }
    int numGeneratedAndWritten() { return m_num_generated_written; }

    virtual bool shouldGenerate(const AbstractMetaClass *) const { return true; }
    virtual QString subDirectoryForClass(const AbstractMetaClass *java_class) const;
    virtual QString fileNameForClass(const AbstractMetaClass *java_class) const;
    virtual void write(QTextStream &s, const AbstractMetaClass *java_class);

    bool hasDefaultConstructor(const AbstractMetaType *type);


protected:
    void verifyDirectoryFor(const QFile &file);

    AbstractMetaClassList m_classes;
    int m_num_generated;
    int m_num_generated_written;
    QString m_out_dir;
};

class Indentor {
public:
    Indentor():
        indent(0)
        {}
    int indent;
};

class Indentation {
public:
    Indentation(Indentor &indentor):
        indentor(indentor) 
        {
            indentor.indent++;
        }
    ~Indentation() 
        { 
            indentor.indent--;
        }

private:
    Indentor &indentor;
};

inline QTextStream &operator <<(QTextStream &s, const Indentor &indentor)
{
    for (int i=0; i<indentor.indent; ++i)
        s << "    ";
    return s;
}


#endif // GENERATOR_H
