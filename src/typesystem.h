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

#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QDebug>

class MetaJavaType;
class QTextStream;

class EnumTypeEntry;
class FlagsTypeEntry;

extern QString strings_Object;
extern QString strings_String;
extern QString strings_Thread;
extern QString strings_char;
extern QString strings_java_lang;
extern QString strings_jchar;
extern QString strings_jobject;

struct Include
{
    enum IncludeType {
        IncludePath,
        LocalPath,
        JavaImport
    };

    Include() : type(IncludePath) { }
    Include(IncludeType t, const QString &nam) : type(t), name(nam) { };

    bool isValid() { return !name.isEmpty(); }

    IncludeType type;
    QString name;

    QString toString() const;
};
typedef QList<Include> IncludeList;

typedef QMap<int, QString> ArgumentMap;

class TemplateInstance;

namespace TypeSystem {
    enum Language {
        NoLanguage          = 0x0000,
        JavaCode            = 0x0001,
        NativeCode          = 0x0002,
        ShellCode           = 0x0004,
        ShellDeclaration    = 0x0008,
        PackageInitializer  = 0x0010,

        // masks
        All                 = JavaCode | NativeCode | ShellCode | ShellDeclaration | PackageInitializer,
        JavaAndNativeCode   = JavaCode | NativeCode
    };

    enum Ownership {
        InvalidOwnership,
        DefaultOwnership,
        JavaOwnership,
        CppOwnership
    };
};

struct ReferenceCount
{
    ReferenceCount() : threadSafe(false), access(Public) { }
    enum Action { // 0x01 - 0xff
        Invalid     = 0x00,
        Add         = 0x01,
        AddAll      = 0x02,
        Remove      = 0x04,
        Set         = 0x08,
        Ignore      = 0x10,

        ActionsMask = 0xff,

        Padding     = 0xffffffff
    };

    enum Flag { // 0x100 - 0xf00
        ThreadSafe      = 0x100,
        Static          = 0x200,
        DeclareVariable = 0x400,

        FlagsMask       = 0xf00
    };

    enum Access { // 0x1000 - 0xf000
        Private     = 0x1000,
        Protected   = 0x2000,
        Friendly    = 0x3000,
        Public      = 0x4000,

        AccessMask  = 0xf000
    };

    Action action;
    QString variableName;
    QString conditional;
    QString declareVariable;

    uint threadSafe : 1;

    uint access;
};

class CodeSnipFragment{
    private:
        const QString m_code;
        TemplateInstance *m_instance;

    public:
        CodeSnipFragment(const QString &code)
    : m_code(code),
        m_instance(0)
        {}

        CodeSnipFragment(TemplateInstance *instance)
    : m_instance(instance)
        {}

        QString code() const;
};

class CodeSnipAbstract{
    public:
        QString code() const;

        void addCode(const QString &code){
            codeList.append(new CodeSnipFragment(code));
        }

        void addTemplateInstance(TemplateInstance *ti){
            codeList.append(new CodeSnipFragment(ti));
        }

    protected:
        QList<CodeSnipFragment*> codeList;
};

class CustomFunction : public CodeSnipAbstract
{
    public:
        CustomFunction(const QString &n = QString()) : name(n) { }

        QString name;
        QString param_name;
};

class TemplateEntry : public CodeSnipAbstract
{
    public:
        TemplateEntry(const QString &name)
    : m_name(name)
        {};

        QString name() const{
            return m_name;
        };

    private:
        QString m_name;
};
typedef QHash<QString, TemplateEntry *> TemplateEntryHash;

class TemplateInstance
{
    public:
        TemplateInstance(const QString &name)
           : m_name(name)
        {}

        void addReplaceRule(const QString &name, const QString &value){
            replaceRules[name]=value;
        }

        QString expandCode() const;

        QString name() const {
            return m_name;
        }

    private:
        const QString m_name;
        QHash<QString, QString> replaceRules;
};


class CodeSnip : public CodeSnipAbstract
{
    public:
        enum Position {
            Beginning,
            End
        };

        CodeSnip() : language(TypeSystem::JavaCode) { }
        CodeSnip(TypeSystem::Language lang) : language(lang) { }

        // Very simple, easy to make code ugly if you try
        QString formattedCode(const QString &_defaultIndent);

        TypeSystem::Language language;
        Position position;
        ArgumentMap argumentMap;
};
typedef QList<CodeSnip> CodeSnipList;

struct ArgumentModification
{
    ArgumentModification(int idx) : removed_default_expression(false), removed(false), no_null_pointers(false), index(idx)
    {}

    // Should the default expression be removed?
    uint removed_default_expression : 1;
    uint removed : 1;
    uint no_null_pointers : 1;

    // The index of this argument
    int index;

    // Reference count flags for this argument
    QList<ReferenceCount> referenceCounts;

    // The text given for the new type of the argument
    QString modified_type;

    // The code to be used to construct a return value when no_null_pointers is true and
    // the returned value is null. If no_null_pointers is true and this string is
    // empty, then the base class implementation will be used (or a default construction
    // if there is no implementation)
    QString null_pointer_default_value;

    // The text of the new default expression of the argument
    QString replaced_default_expression;

    // The new definition of ownership for a specific argument
    QHash<TypeSystem::Language, TypeSystem::Ownership> ownerships;

    // Different conversion rules
    CodeSnipList conversion_rules;
};

struct Modification {
    enum Modifiers {
        Private =               0x0001,
        Protected =             0x0002,
        Public =                0x0003,
        Friendly =              0x0004,
        AccessModifierMask =    0x000f,

        Readable =              0x0010,
        Writable =              0x0020,

        CodeInjection =         0x0100,
        Rename =                0x0200,
        ReplaceExpression =     0x0800
    };

    Modification() : modifiers(0) { }

    bool isAccessModifier() const { return modifiers & AccessModifierMask; }
    Modifiers accessModifier() const { return Modifiers(modifiers & AccessModifierMask); }
    bool isPrivate() const { return accessModifier() == Private; }
    bool isProtected() const { return accessModifier() == Protected; }
    bool isPublic() const { return accessModifier() == Public; }
    bool isFriendly() const { return accessModifier() == Friendly; }
    QString accessModifierString() const;

    void setRenamedTo(const QString &name) { renamedToName = name; }
    QString renamedTo() const { return renamedToName; }
    bool isRenameModifier() const { return modifiers & Rename; }

    uint modifiers;
    QString renamedToName;
};

struct FunctionModification: public Modification
{
    FunctionModification() : removal(TypeSystem::NoLanguage) { }

    bool isCodeInjection() const { return modifiers & CodeInjection; }
    bool isRemoveModifier() const { return removal != TypeSystem::NoLanguage; }

    QString signature;
    CodeSnipList snips;
    TypeSystem::Language removal;

    QList<ArgumentModification> argument_mods;
};
typedef QList<FunctionModification> FunctionModificationList;

struct FieldModification: public Modification
{
    bool isReadable() const { return modifiers & Readable; }
    bool isWritable() const { return modifiers & Writable; }

    QString name;
};
typedef QList<FieldModification> FieldModificationList;

struct ExpensePolicy {
    ExpensePolicy() : limit(-1) { }
    int limit;
    QString cost;
    bool isValid() const { return limit >= 0; }
};

class InterfaceTypeEntry;
class ObjectTypeEntry;

class TypeEntry
{
public:
    enum Type {
        PrimitiveType,
        VoidType,
        FlagsType,
        EnumType,
        TemplateArgumentType,
        ThreadType,
        BasicValueType,
        StringType,
        ContainerType,
        InterfaceType,
        ObjectType,
        NamespaceType,
        VariantType,
        CharType,
        ArrayType,
        TypeSystemType,
        CustomType,
    };

    enum CodeGeneration {
        GenerateJava            = 0x0001,
        GenerateCpp             = 0x0002,
        GenerateForSubclass     = 0x0004,

        GenerateNothing         = 0,
        GenerateAll             = 0xffff
    };

    TypeEntry(const QString &name, Type t)
        : m_name(name),
          m_type(t),
          m_code_generation(GenerateAll),
          m_preferred_conversion(true)
    {
    };

    virtual ~TypeEntry() { }

    Type type() const { return m_type; }
    bool isPrimitive() const { return m_type == PrimitiveType; }
    bool isEnum() const { return m_type == EnumType; }
    bool isFlags() const { return m_type == FlagsType; }
    bool isInterface() const { return m_type == InterfaceType; }
    bool isObject() const { return m_type == ObjectType; }
    bool isString() const { return m_type == StringType; }
    bool isChar() const { return m_type == CharType; }
    bool isNamespace() const { return m_type == NamespaceType; }
    bool isContainer() const { return m_type == ContainerType; }
    bool isVariant() const { return m_type == VariantType; }
    bool isArray() const { return m_type == ArrayType; }
    bool isTemplateArgument() const { return m_type == TemplateArgumentType; }
    bool isVoid() const { return m_type == VoidType; }
    bool isThread() const { return m_type == ThreadType; }
    bool isCustom() const { return m_type == CustomType; }
    bool isBasicValue() const { return m_type == BasicValueType; }
    bool isTypeSystem() const { return m_type == TypeSystemType; }

    virtual bool preferredConversion() const { return m_preferred_conversion; }
    virtual void setPreferredConversion(bool b) { m_preferred_conversion = b; }

    // The type's name in C++, fully qualified
    QString name() const { return m_name; }

    uint codeGeneration() const { return m_code_generation; }
    void setCodeGeneration(uint cg) { m_code_generation = cg; }

    virtual QString qualifiedCppName() const { return m_name; }

    // Its type's name in JNI
    virtual QString jniName() const { return m_name; }

    // The type's name in Java
    virtual QString javaName() const { return m_name; }

    // The type to lookup when converting to Java
    virtual QString lookupName() const { return javaName(); }

    // The package
    virtual QString javaPackage() const { return QString(); }

    virtual QString qualifiedJavaName() const {
        QString pkg = javaPackage();
        if (pkg.isEmpty()) return javaName();
        return pkg + '.' + javaName();
    }

    virtual InterfaceTypeEntry *designatedInterface() const { return 0; }

    void setCustomConstructor(const CustomFunction &func) { m_customConstructor = func; }
    CustomFunction customConstructor() const { return m_customConstructor; }

    void setCustomDestructor(const CustomFunction &func) { m_customDestructor = func; }
    CustomFunction customDestructor() const { return m_customDestructor; }

    virtual bool isValue() const { return false; }
    virtual bool isComplex() const { return false; }

    virtual bool isNativeIdBased() const { return false; }

private:
    QString m_name;
    Type m_type;
    uint m_code_generation;
    CustomFunction m_customConstructor;
    CustomFunction m_customDestructor;
    bool m_preferred_conversion;
};
typedef QHash<QString, TypeEntry *> TypeEntryHash;


class TypeSystemTypeEntry : public TypeEntry
{
public:
    TypeSystemTypeEntry(const QString &name)
        : TypeEntry(name, TypeSystemType)
    {
    };

    QList<CodeSnip> snips;
};


class ThreadTypeEntry : public TypeEntry
{
public:
    ThreadTypeEntry() : TypeEntry("QThread", ThreadType) { setCodeGeneration(GenerateNothing); }

    QString jniName() const { return strings_jobject; }
    QString javaName() const { return strings_Thread; }
    QString javaPackage() const { return strings_java_lang; }
};

class VoidTypeEntry : public TypeEntry
{
public:
    VoidTypeEntry() : TypeEntry("void", VoidType) { }
};

class TemplateArgumentEntry : public TypeEntry
{
public:
    TemplateArgumentEntry(const QString &name)
        : TypeEntry(name, TemplateArgumentType), m_ordinal(0)
    {
    }

    int ordinal() const { return m_ordinal; }
    void setOrdinal(int o) { m_ordinal = o; }

private:
    int m_ordinal;
};


class ArrayTypeEntry : public TypeEntry
{
public:
    ArrayTypeEntry(const TypeEntry *nested_type) : TypeEntry("Array", ArrayType), m_nested_type(nested_type)
    {
        Q_ASSERT(m_nested_type);
    }

    void setNestedTypeEntry(TypeEntry *nested) { m_nested_type = nested; }
    const TypeEntry *nestedTypeEntry() const { return m_nested_type; }

    QString javaName() const { return m_nested_type->javaName() + "[]"; }
    QString jniName() const
    {
        if (m_nested_type->isPrimitive())
            return m_nested_type->jniName() + "Array";
        else
            return "jobjectArray";
    }

private:
    const TypeEntry *m_nested_type;
};


class PrimitiveTypeEntry : public TypeEntry
{
public:
    PrimitiveTypeEntry(const QString &name)
        : TypeEntry(name, PrimitiveType), m_preferred_conversion(true)
    {
    }

    QString javaName() const { return m_java_name; }
    void setJavaName(const QString &javaName) { m_java_name  = javaName; }

    QString jniName() const { return m_jni_name; }
    void setJniName(const QString &jniName) { m_jni_name = jniName; }

    QString javaObjectFullName() const { return javaObjectPackage() + "." + javaObjectName(); }
    QString javaObjectName() const;
    QString javaObjectPackage() const { return strings_java_lang; }

    virtual bool preferredConversion() const { return m_preferred_conversion; }
    virtual void setPreferredConversion(bool b) { m_preferred_conversion = b; }

private:
    QString m_java_name;
    QString m_jni_name;
    bool m_preferred_conversion;
};




struct EnumValueRedirection
{
    EnumValueRedirection(const QString &rej, const QString &us)
        : rejected(rej),
          used(us)
    {
    }
    QString rejected;
    QString used;
};

class EnumTypeEntry : public TypeEntry
{
public:
    EnumTypeEntry(const QString &nspace, const QString &enumName)
        : TypeEntry(nspace + QLatin1String("::") + enumName, EnumType),
          m_flags(0),
          m_extensible(false)
    {
        m_qualifier = nspace;
        m_java_name = enumName;
    }

    QString javaPackage() const { return m_package_name; }
    void setJavaPackage(const QString &package) { m_package_name = package; }

    QString javaName() const { return m_java_name; }
    QString javaQualifier() const { return m_qualifier; }
    QString qualifiedJavaName() const {
        QString pkg = javaPackage();
        if (pkg.isEmpty()) return javaQualifier() + '.' + javaName();
        return pkg + '.' + javaQualifier() + '.' + javaName();
    }

    QString jniName() const;

    QString qualifier() const { return m_qualifier; }

    virtual bool preferredConversion() const { return false; }

    bool isBoundsChecked() const { return m_lower_bound.isEmpty() && m_upper_bound.isEmpty(); }

    QString upperBound() const { return m_upper_bound; }
    void setUpperBound(const QString &bound) { m_upper_bound = bound; }

    QString lowerBound() const { return m_lower_bound; }
    void setLowerBound(const QString &bound) { m_lower_bound = bound; }

    void setFlags(FlagsTypeEntry *flags) { m_flags = flags; }
    FlagsTypeEntry *flags() const { return m_flags; }

    bool isExtensible() const { return m_extensible; }
    void setExtensible(bool is) { m_extensible = is; }

    bool isEnumValueRejected(const QString &name) { return m_rejected_enums.contains(name); }
    void addEnumValueRejection(const QString &name) { m_rejected_enums << name; }

    void addEnumValueRedirection(const QString &rejected, const QString &usedValue);
    QString enumValueRedirection(const QString &value) const;

    bool forceInteger() const { return m_force_integer; }
    void setForceInteger(bool force) { m_force_integer = force; }

private:
    QString m_package_name;
    QString m_qualifier;
    QString m_java_name;

    QString m_lower_bound;
    QString m_upper_bound;

    QStringList m_rejected_enums;
    QList<EnumValueRedirection> m_enum_redirections;

    FlagsTypeEntry *m_flags;

    bool m_extensible;
    bool m_force_integer;
};

class FlagsTypeEntry : public TypeEntry
{
public:
    FlagsTypeEntry(const QString &name) : TypeEntry(name, FlagsType), m_enum(0)
    {
    }

    QString qualifiedJavaName() const;
    QString javaName() const { return m_java_name; }
    QString jniName() const;
    virtual bool preferredConversion() const { return false; }

    QString originalName() const { return m_original_name; }
    void setOriginalName(const QString &s) { m_original_name = s; }

    QString flagsName() const { return m_java_name; }
    void setFlagsName(const QString &name) { m_java_name = name; }

    bool forceInteger() const { return m_enum->forceInteger(); }

    EnumTypeEntry *originator() const { return m_enum; }
    void setOriginator(EnumTypeEntry *e) { m_enum = e; }

    QString javaPackage() const { return m_enum->javaPackage(); }

private:
    QString m_original_name;
    QString m_java_name;
    EnumTypeEntry *m_enum;
};


class ComplexTypeEntry : public TypeEntry
{
public:
    ComplexTypeEntry(const QString &name, Type t)
        : TypeEntry(QString(name).replace("::", "_"), t),
          m_qualified_cpp_name(name),
          m_qobject(false),
          m_polymorphic_base(false)
    {
        Include inc;
        inc.name = "QVariant";
        inc.type = Include::IncludePath;

        addExtraInclude(inc);
    }

    bool isComplex() const { return true; }

    IncludeList extraIncludes() const { return m_extra_includes; }
    void setExtraIncludes(const IncludeList &includes) { m_extra_includes = includes; }
    void addExtraInclude(const Include &include)
    {
        if (!m_includes_used.value(include.name, false)) {
            m_extra_includes << include;
            m_includes_used[include.name] = true;
        }
    }

    ComplexTypeEntry *copy() const
    {
        ComplexTypeEntry *centry = new ComplexTypeEntry(name(), type());
        centry->setInclude(include());
        centry->setExtraIncludes(extraIncludes());
        centry->setFunctionModifications(functionModifications());
        centry->setFieldModifications(fieldModifications());
        centry->setQObject(isQObject());
        centry->setDefaultSuperclass(defaultSuperclass());
        centry->setCodeSnips(codeSnips());
        centry->setJavaPackage(javaPackage());

        return centry;
    }

    void setLookupName(const QString &name)
    {
        m_lookup_name = name;
    }

    virtual QString lookupName() const
    {
        return m_lookup_name.isEmpty() ? javaName() : m_lookup_name;
    }

    QString jniName() const { return strings_jobject; }


    Include include() const { return m_include; }
    void setInclude(const Include &inc) { m_include = inc; }

    CodeSnipList codeSnips() const { return m_code_snips; }
    void setCodeSnips(const CodeSnipList &codeSnips) { m_code_snips = codeSnips; }
    void addCodeSnip(const CodeSnip &codeSnip) { m_code_snips << codeSnip; }

    FunctionModificationList functionModifications() const { return m_function_mods; }
    void setFunctionModifications(const FunctionModificationList &functionModifications) {
        m_function_mods = functionModifications;
    }
    void addFunctionModification(const FunctionModification &functionModification) {
        m_function_mods << functionModification;
    }
    FunctionModificationList functionModifications(const QString &signature) const;

    FieldModification fieldModification(const QString &name) const;
    void setFieldModifications(const FieldModificationList &mods) { m_field_mods = mods; }
    FieldModificationList fieldModifications() const { return m_field_mods; }

    QString javaPackage() const { return m_package; }
    void setJavaPackage(const QString &package) { m_package = package; }

    bool isQObject() const { return m_qobject; }
    void setQObject(bool qobject) { m_qobject = qobject; }

    QString defaultSuperclass() const { return m_default_superclass; }
    void setDefaultSuperclass(const QString &sc) { m_default_superclass = sc; }

    virtual QString qualifiedCppName() const { return m_qualified_cpp_name; }


    void setIsPolymorphicBase(bool on)
    {
        m_polymorphic_base = on;
    }
    bool isPolymorphicBase() const { return m_polymorphic_base; }

    void setPolymorphicIdValue(const QString &value)
    {
        m_polymorphic_id_value = value;
    }
    QString polymorphicIdValue() const { return m_polymorphic_id_value; }

    void setExpensePolicy(const ExpensePolicy &policy) { m_expense_policy = policy; }
    const ExpensePolicy &expensePolicy() const { return m_expense_policy; }

    QString javaName() const { return m_java_name.isEmpty()
                                   ? TypeEntry::javaName()
                                   : m_java_name;
    }
    void setJavaName(const QString &name) { m_java_name = name; }

private:
    IncludeList m_extra_includes;
    Include m_include;
    QHash<QString, bool> m_includes_used;
    FunctionModificationList m_function_mods;
    FieldModificationList m_field_mods;
    CodeSnipList m_code_snips;
    QString m_package;
    QString m_default_superclass;
    QString m_qualified_cpp_name;
    QString m_java_name;
    uint m_qobject : 1;
    bool m_polymorphic_base;
    QString m_polymorphic_id_value;
    QString m_lookup_name;
    ExpensePolicy m_expense_policy;
};


class ContainerTypeEntry : public ComplexTypeEntry
{
public:
    enum Type {
        NoContainer,
        ListContainer,
        StringListContainer,
        LinkedListContainer,
        VectorContainer,
        StackContainer,
        QueueContainer,
        SetContainer,
        MapContainer,
        MultiMapContainer,
        HashContainer,
        MultiHashContainer,
        PairContainer,
    };

    ContainerTypeEntry(const QString &name, Type type)
        : ComplexTypeEntry(name, ContainerType)
    {
        m_type = type;
        setCodeGeneration(GenerateForSubclass);
    }

    Type type() const { return m_type; }
    QString javaName() const;
    QString javaPackage() const;
    QString qualifiedCppName() const;

private:
    Type m_type;
};


class NamespaceTypeEntry : public ComplexTypeEntry
{
public:
    NamespaceTypeEntry(const QString &name) : ComplexTypeEntry(name, NamespaceType) { }
};


class ValueTypeEntry : public ComplexTypeEntry
{
public:
    ValueTypeEntry(const QString &name) : ComplexTypeEntry(name, BasicValueType) { }

    bool isValue() const { return true; }

    virtual bool isNativeIdBased() const { return true; }

protected:
    ValueTypeEntry(const QString &name, Type t) : ComplexTypeEntry(name, t) { }
};


class StringTypeEntry : public ValueTypeEntry
{
public:
    StringTypeEntry(const QString &name)
        : ValueTypeEntry(name, StringType)
    {
        setCodeGeneration(GenerateNothing);
    }

    QString jniName() const { return strings_jobject; }
    QString javaName() const { return strings_String; }
    QString javaPackage() const { return strings_java_lang; }

    virtual bool isNativeIdBased() const { return false; }
};

class CharTypeEntry : public ValueTypeEntry
{
public:
    CharTypeEntry(const QString &name) : ValueTypeEntry(name, CharType)
    {
        setCodeGeneration(GenerateNothing);
    }

    QString jniName() const { return strings_jchar; }
    QString javaName() const { return strings_char; }
    QString javaPackage() const { return QString(); }

    virtual bool isNativeIdBased() const { return false; }
};

class VariantTypeEntry: public ValueTypeEntry
{
public:
    VariantTypeEntry(const QString &name) : ValueTypeEntry(name, VariantType) { }

    QString jniName() const { return strings_jobject; }
    QString javaName() const { return strings_Object; }
    QString javaPackage() const { return strings_java_lang; }

    virtual bool isNativeIdBased() const { return false; }
};


class InterfaceTypeEntry : public ComplexTypeEntry
{
public:
    InterfaceTypeEntry(const QString &name)
        : ComplexTypeEntry(name, InterfaceType)
    {
    }

    static QString interfaceName(const QString &name) {
        return name + "Interface";
    }

    ObjectTypeEntry *origin() const { return m_origin; }
    void setOrigin(ObjectTypeEntry *origin) { m_origin = origin; }

    virtual bool isNativeIdBased() const { return true; }
    virtual QString qualifiedCppName() const {
        return name().left(name().length() - interfaceName("").length());
    }

private:
    ObjectTypeEntry *m_origin;
};


class ObjectTypeEntry : public ComplexTypeEntry
{
public:
    ObjectTypeEntry(const QString &name)
        : ComplexTypeEntry(name, ObjectType), m_interface(0)
    {
    }

    InterfaceTypeEntry *designatedInterface() const { return m_interface; }
    void setDesignatedInterface(InterfaceTypeEntry *entry) { m_interface = entry; }

    virtual bool isNativeIdBased() const { return true; }

private:
    InterfaceTypeEntry *m_interface;
};

class CustomTypeEntry : public ComplexTypeEntry
{
public:
    CustomTypeEntry(const QString &name) : ComplexTypeEntry(name, CustomType) { }

    virtual void generateCppJavaToQt(QTextStream &s,
                                     const MetaJavaType *java_type,
                                     const QString &env_name,
                                     const QString &qt_name,
                                     const QString &java_name) const = 0;

    virtual void generateCppQtToJava(QTextStream &s,
                                     const MetaJavaType *java_type,
                                     const QString &env_name,
                                     const QString &qt_name,
                                     const QString &java_name) const = 0;
};

struct TypeRejection
{
    QString class_name;
    QString function_name;
    QString field_name;
    QString enum_name;
};

class TypeDatabase
{
public:
    TypeDatabase();

    static TypeDatabase *instance();

    QList<Include> extraIncludes(const QString &className);

    inline PrimitiveTypeEntry *findPrimitiveType(const QString &name);
    inline ComplexTypeEntry *findComplexType(const QString &name);
    inline ObjectTypeEntry *findObjectType(const QString &name);
    inline NamespaceTypeEntry *findNamespaceType(const QString &name);
    ContainerTypeEntry *findContainerType(const QString &name);

    TypeEntry *findType(const QString &name) { return m_entries.value(name); }
    TypeEntryHash entries() { return m_entries; }

    PrimitiveTypeEntry *findJavaPrimitiveType(const QString &java_name);

    void addRejection(const QString &class_name, const QString &function_name,
                      const QString &field_name, const QString &enum_name);
    bool isClassRejected(const QString &class_name);
    bool isFunctionRejected(const QString &class_name, const QString &function_name);
    bool isFieldRejected(const QString &class_name, const QString &field_name);
    bool isEnumRejected(const QString &class_name, const QString &enum_name);

    void addType(TypeEntry *e) { m_entries[e->qualifiedCppName()] = e; }

    TypeEntryHash flagsEntries() const { return m_flags_entries; }
    FlagsTypeEntry *findFlagsType(const QString &name) const;
    void addFlagsType(FlagsTypeEntry *fte) { m_flags_entries[fte->originalName()] = fte; }

    TemplateEntry *findTemplate(const QString &name) { return m_templates[name]; }
    void addTemplate(TemplateEntry *t) { m_templates[t->name()] = t; }

    void setSuppressWarnings(bool on) { m_suppressWarnings = on; }
    void addSuppressedWarning(const QString &s)
    {
        m_suppressedWarnings.append(s);
    }

    bool isSuppressedWarning(const QString &s)
    {
        if (!m_suppressWarnings)
            return false;

        foreach (const QString &_warning, m_suppressedWarnings) {
            QString warning(QString(_warning).replace("\\*", "&place_holder_for_asterisk;"));

            QStringList segs = warning.split("*", QString::SkipEmptyParts);
            if (segs.size() == 0)
                continue ;

            int i = 0;
            int pos = s.indexOf(QString(segs.at(i++)).replace("&place_holder_for_asterisk;", "*"));
            //qDebug() << "s == " << s << ", warning == " << segs;
            while (pos != -1) {
                if (i == segs.size())
                    return true;
                pos = s.indexOf(QString(segs.at(i++)).replace("&place_holder_for_asterisk;", "*"), pos);
            }
        }

        return false;
    }

    void setRebuildClasses(const QStringList &cls) { m_rebuild_classes = cls; }

    QString filename() const { return "typesystem.txt"; }

    bool parseFile(const QString &filename, bool generate = true);

private:
    bool m_suppressWarnings;
    TypeEntryHash m_entries;
    TypeEntryHash m_flags_entries;
    TemplateEntryHash m_templates;
    QStringList m_suppressedWarnings;

    QList<TypeRejection> m_rejections;
    QStringList m_rebuild_classes;
};

inline PrimitiveTypeEntry *TypeDatabase::findPrimitiveType(const QString &name)
{
    TypeEntry *entry = findType(name);
    if (entry != 0 && entry->isPrimitive())
        return static_cast<PrimitiveTypeEntry *>(entry);
    else
        return 0;
}

inline ComplexTypeEntry *TypeDatabase::findComplexType(const QString &name)
{
    TypeEntry *entry = findType(name);
    if (entry != 0 && entry->isComplex())
        return static_cast<ComplexTypeEntry *>(entry);
    else
        return 0;
}

inline ObjectTypeEntry *TypeDatabase::findObjectType(const QString &name)
{
    TypeEntry *entry = findType(name);
    if (entry != 0 && entry->isObject())
        return static_cast<ObjectTypeEntry *>(entry);
    else
        return 0;
}

inline NamespaceTypeEntry *TypeDatabase::findNamespaceType(const QString &name)
{
    TypeEntry *entry = findType(name);
    if (entry != 0 && entry->isNamespace())
        return static_cast<NamespaceTypeEntry *>(entry);
    else
        return 0;
}

QString fixCppTypeName(const QString &name);

#endif // TYPESYSTEM_H
