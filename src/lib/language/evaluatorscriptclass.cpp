/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "evaluatorscriptclass.h"
#include "builtinvalue.h"
#include "evaluationdata.h"
#include "evaluator.h"
#include "item.h"
#include "filecontext.h"
#include <tools/qbsassert.h>

#include <QScriptEngine>
#include <QScriptString>
#include <QScriptValue>
#include <QDebug>

namespace qbs {
namespace Internal {

class SVConverter : ValueHandler
{
    EvaluatorScriptClass *const scriptClass;
    QScriptEngine *const engine;
    QScriptContext *const scriptContext;
    const QScriptValue *object;
    const ValuePtr &valuePtr;
    char pushedScopesCount;

public:
    const QScriptString *propertyName;
    const EvaluationData *data;
    QScriptValue *result;

    SVConverter(EvaluatorScriptClass *esc, const QScriptValue *obj, const ValuePtr &v)
        : scriptClass(esc)
        , engine(esc->engine())
        , scriptContext(esc->engine()->currentContext())
        , object(obj)
        , valuePtr(v)
        , pushedScopesCount(0)
    {
    }

    void start()
    {
        valuePtr->apply(this);
    }

private:
    void setupConvenienceProperty(const QString &conveniencePropertyName, QScriptValue *extraScope,
                                  const QScriptValue &scriptValue)
    {
        if (!extraScope->isObject())
            *extraScope = scriptClass->engine()->newObject();
        if (!scriptValue.isValid() || scriptValue.isUndefined()) {
            // If there's no such value, use an empty array to have the convenience property
            // still available.
            extraScope->setProperty(conveniencePropertyName, engine->newArray());
        } else {
            extraScope->setProperty(conveniencePropertyName, scriptValue);
        }
    }

    void pushScope(const QScriptValue &scope)
    {
        if (scope.isObject()) {
            scriptContext->pushScope(scope);
            ++pushedScopesCount;
        }
    }

    void popScopes()
    {
        for (; pushedScopesCount-- > 0;)
            scriptContext->popScope();
    }

    void handle(JSSourceValue *value)
    {
        ItemConstPtr conditionScopeItem;
        QScriptValue conditionScope;
        ItemPtr outerItem = data->item->outerItem();
        for (int i = 0; i < value->alternatives().count(); ++i) {
            const JSSourceValue::Alternative *alternative = 0;
            alternative = &value->alternatives().at(i);
            if (conditionScopeItem != alternative->conditionScopeItem) {
                conditionScopeItem = alternative->conditionScopeItem;
                conditionScope = data->evaluator->scriptValue(alternative->conditionScopeItem);
                QBS_ASSERT(conditionScope.isObject(), return);
            }
            engine->currentContext()->pushScope(conditionScope);
            const QScriptValue cr = engine->evaluate(alternative->condition);
            engine->currentContext()->popScope();
            if (cr.isError()) {
                *result = cr;
                return;
            }
            if (cr.toBool()) {
                // condition is true, let's use the value of this alternative
                if (alternative->value->sourceUsesOuter()) {
                    // Clone value but without alternatives.
                    JSSourceValuePtr outerValue = JSSourceValue::create();
                    outerValue->setFile(value->file());
                    outerValue->setSourceCode(value->sourceCode());
                    outerValue->setBaseValue(value->baseValue());
                    outerValue->setLocation(value->location());
                    outerItem = Item::create();
                    outerItem->setProperty(propertyName->toString(), outerValue);
                }
                value = alternative->value.data();
                break;
            }
        }

        QScriptValue extraScope;
        if (value->sourceUsesBase()) {
            QScriptValue baseValue;
            if (value->baseValue()) {
                SVConverter converter(scriptClass, object, value->baseValue());
                converter.propertyName = propertyName;
                converter.data = data;
                converter.result = &baseValue;
                converter.start();
            }
            setupConvenienceProperty(QLatin1String("base"), &extraScope, baseValue);
        }
        if (value->sourceUsesOuter())
            setupConvenienceProperty(QLatin1String("outer"), &extraScope,
                                     data->evaluator->property(outerItem, *propertyName));

        pushScope(data->evaluator->fileScope(value->file()));
        scriptContext->pushScope(*object);
        pushScope(extraScope);
        *result = engine->evaluate(value->sourceCode());
        popScopes();
        scriptContext->popScope();
    }

    void handle(ItemValue *value)
    {
        const ItemPtr &item = value->item();
        if (!item)
            qDebug() << "SVConverter got null item" << propertyName->toString();
        *result = data->evaluator->scriptValue(item);
        if (!result->isValid())
            qDebug() << "SVConverter returned invalid script value.";
    }

    void handle(VariantValue *variantValue)
    {
        *result = scriptClass->engine()->toScriptValue(variantValue->value());
    }

    void handle(BuiltinValue *builtinValue)
    {
        *result = scriptClass->scriptValueForBuiltin(builtinValue->builtin());
    }
};

bool debugProperties = false;
//bool debugProperties = true;
//#if 0
//const bool debugProperties = true;
//#else
//const bool debugProperties = false;
//#endif

enum QueryPropertyType
{
    QPTDefault,
    QPTParentProperty
};

EvaluatorScriptClass::EvaluatorScriptClass(QScriptEngine *scriptEngine, const Logger &logger)
    : QScriptClass(scriptEngine)
    , m_logger(logger)
{
    m_getenvBuiltin = scriptEngine->newFunction(js_getenv, 1);
    m_getHostOSBuiltin = scriptEngine->newFunction(js_getHostOS, 1);
}

QScriptClass::QueryFlags EvaluatorScriptClass::queryProperty(const QScriptValue &object,
                                                             const QScriptString &name,
                                                             QScriptClass::QueryFlags flags,
                                                             uint *id)
{
    Q_UNUSED(flags);

    // We assume that it's safe to save the result of the query in a member of the scriptclass.
    // It must be cleared in the property method before doing any further lookup.
    QBS_ASSERT(m_queryResult.isNull(), return QueryFlags());

    if (debugProperties)
        m_logger.qbsTrace() << "[SC] queryProperty " << object.objectId() << " " << name;

    EvaluationData *const data = EvaluationData::get(object);
    const QString nameString = name.toString();
    if (nameString == QLatin1String("parent")) {
        *id = QPTParentProperty;
        m_queryResult.data = data;
        return QScriptClass::HandlesReadAccess;
    }

    *id = QPTDefault;
    if (!data) {
        if (debugProperties)
            m_logger.qbsTrace() << "[SC] queryProperty: no data attached";
        return QScriptClass::QueryFlags();
    }

    return queryItemProperty(data, nameString);
}

QScriptClass::QueryFlags EvaluatorScriptClass::queryItemProperty(const EvaluationData *data,
                                                                 const QString &name,
                                                                 bool ignoreParent)
{
    for (const Item *item = data->item; item; item = item->prototype().data()) {
        m_queryResult.value = item->properties().value(name);
        if (!m_queryResult.value.isNull()) {
            m_queryResult.data = data;
            return HandlesReadAccess;
        }
    }

    if (!data->item->scope().isNull()) {
        if (debugProperties)
            m_logger.qbsTrace() << "[SC] queryProperty: query scope";
        EvaluationData scopedata = *data;
        scopedata.item = data->item->scope().data();
        const QueryFlags qf = queryItemProperty(&scopedata, name, true);
        if (qf.testFlag(HandlesReadAccess)) {
            m_queryResult.data = data;
            return qf;
        }
    }

    if (!ignoreParent && !data->item->parent().isNull()) {
        if (debugProperties)
            m_logger.qbsTrace() << "[SC] queryProperty: query parent";
        EvaluationData parentdata = *data;
        parentdata.item = data->item->parent().data();
        const QueryFlags qf = queryItemProperty(&parentdata, name, true);
        if (qf.testFlag(HandlesReadAccess)) {
            m_queryResult.data = data;
            return qf;
        }
    }

    if (debugProperties)
        m_logger.qbsTrace() << "[SC] queryProperty: no such property";
    return QScriptClass::QueryFlags();
}

QString EvaluatorScriptClass::resultToString(const QScriptValue &scriptValue)
{
    return (scriptValue.isObject()
        ? QLatin1String("[Object: ")
            + QString::number(scriptValue.objectId(), 16) + QLatin1Char(']')
        : scriptValue.toVariant().toString());
}

ItemPtr EvaluatorScriptClass::findItemInScope(const Item *item, const QString &typeName)
{
    for (ItemPtr it = item->scope(); it; it = it->scope()) {
        if (it->typeName() == typeName)
            return it;
    }
    return ItemPtr();
}

ItemPtr EvaluatorScriptClass::findParentOfType(const Item *item, const QString &typeName)
{
    for (ItemPtr it = item->parent(); it; it = it->parent()) {
        if (it->typeName() == typeName)
            return it;
    }
    return ItemPtr();
}

QScriptValue EvaluatorScriptClass::property(const QScriptValue &object, const QScriptString &name,
                                            uint id)
{
    const EvaluationData *data = m_queryResult.data;
    m_queryResult.data = 0;
    QBS_ASSERT(data, return QScriptValue());

    const QueryPropertyType qpt = static_cast<QueryPropertyType>(id);
    if (qpt == QPTParentProperty) {
        return data->item->parent().isNull() ?
                    engine()->undefinedValue() : data->evaluator->scriptValue(data->item->parent());
    }

    ValuePtr value;
    m_queryResult.value.swap(value);
    QBS_ASSERT(value, return QScriptValue());
    QBS_ASSERT(m_queryResult.isNull(), return QScriptValue());

    if (debugProperties)
        m_logger.qbsTrace() << "[SC] property " << name;

    QScriptValue result = data->valueCache.value(name);
    if (result.isValid()) {
        if (debugProperties)
            m_logger.qbsTrace() << "[SC] cache hit " << name << ": " << resultToString(result);
        return result;
    }

    SVConverter converter(this, &object, value);
    converter.propertyName = &name;
    converter.data = data;
    converter.result = &result;
    converter.start();

    if (debugProperties)
        m_logger.qbsTrace() << "[SC] cache miss " << name << ": " << resultToString(result);
    data->valueCache.insert(name, result);
    return result;
}

QScriptValue EvaluatorScriptClass::scriptValueForBuiltin(BuiltinValue::Builtin builtin) const
{
    switch (builtin) {
    case BuiltinValue::GetEnvFunction:
        return m_getenvBuiltin;
    case BuiltinValue::GetHostOSFunction:
        return m_getHostOSBuiltin;
    }
    QBS_ASSERT(!"unhandled builtin", ;);
    return QScriptValue();
}

QScriptValue EvaluatorScriptClass::js_getenv(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 1) {
        return context->throwError(QScriptContext::SyntaxError,
                                   QLatin1String("getenv expects 1 argument"));
    }
    const QByteArray name = context->argument(0).toString().toLocal8Bit();
    const QByteArray value = qgetenv(name);
    return value.isNull() ? engine->undefinedValue() : QString::fromLocal8Bit(value);
}

QScriptValue EvaluatorScriptClass::js_getHostOS(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    QString hostSystem;

#if defined(Q_OS_WIN)
    hostSystem = "windows";
#elif defined(Q_OS_MAC)
    hostSystem = "mac";
#elif defined(Q_OS_LINUX)
    hostSystem = "linux";
#elif defined(Q_OS_UNIX)
    hostSystem = "unix";
#else
#   error unknown host platform
#endif
    return engine->toScriptValue(hostSystem);
}

} // namespace Internal
} // namespace qbs