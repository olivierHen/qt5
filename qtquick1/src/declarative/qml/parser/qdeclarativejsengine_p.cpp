/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "private/qdeclarativejsengine_p.h"

#include "private/qdeclarativejsglobal_p.h"
#include "private/qdeclarativejsnodepool_p.h"

#include <qnumeric.h>
#include <QHash>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {

uint qHash(const QDeclarativeJS::NameId &id)
{ return qHash(id.asString()); }

QString numberToString(double value)
{ return QString::number(value); }

int Ecma::RegExp::flagFromChar(const QChar &ch)
{
    static QHash<QChar, int> flagsHash;
    if (flagsHash.isEmpty()) {
        flagsHash[QLatin1Char('g')] = Global;
        flagsHash[QLatin1Char('i')] = IgnoreCase;
        flagsHash[QLatin1Char('m')] = Multiline;
    }
    QHash<QChar, int>::const_iterator it;
    it = flagsHash.constFind(ch);
    if (it == flagsHash.constEnd())
        return 0;
    return it.value();
}

QString Ecma::RegExp::flagsToString(int flags)
{
    QString result;
    if (flags & Global)
        result += QLatin1Char('g');
    if (flags & IgnoreCase)
        result += QLatin1Char('i');
    if (flags & Multiline)
        result += QLatin1Char('m');
    return result;
}

NodePool::NodePool(const QString &fileName, Engine *engine)
    : m_fileName(fileName), m_engine(engine)
{
    m_engine->setNodePool(this);
}

NodePool::~NodePool()
{
}

Code *NodePool::createCompiledCode(AST::Node *, CompilationUnit &)
{
    Q_ASSERT(0);
    return 0;
}

static int toDigit(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    else if ((c >= 'a') && (c <= 'z'))
        return 10 + c - 'a';
    else if ((c >= 'A') && (c <= 'Z'))
        return 10 + c - 'A';
    return -1;
}

double integerFromString(const char *buf, int size, int radix)
{
    if (size == 0)
        return qSNaN();

    double sign = 1.0;
    int i = 0;
    if (buf[0] == '+') {
        ++i;
    } else if (buf[0] == '-') {
        sign = -1.0;
        ++i;
    }

    if (((size-i) >= 2) && (buf[i] == '0')) {
        if (((buf[i+1] == 'x') || (buf[i+1] == 'X'))
            && (radix < 34)) {
            if ((radix != 0) && (radix != 16))
                return 0;
            radix = 16;
            i += 2;
        } else {
            if (radix == 0) {
                radix = 8;
                ++i;
            }
        }
    } else if (radix == 0) {
        radix = 10;
    }

    int j = i;
    for ( ; i < size; ++i) {
        int d = toDigit(buf[i]);
        if ((d == -1) || (d >= radix))
            break;
    }
    double result;
    if (j == i) {
        if (!qstrcmp(buf, "Infinity"))
            result = qInf();
        else
            result = qSNaN();
    } else {
        result = 0;
        double multiplier = 1;
        for (--i ; i >= j; --i, multiplier *= radix)
            result += toDigit(buf[i]) * multiplier;
    }
    result *= sign;
    return result;
}

double integerFromString(const QString &str, int radix)
{
    QByteArray ba = str.trimmed().toLatin1();
    return integerFromString(ba.constData(), ba.size(), radix);
}


Engine::Engine()
    : _lexer(0), _nodePool(0)
{ }

Engine::~Engine()
{ }

QSet<NameId> Engine::literals() const
{ return _literals; }

void Engine::addComment(int pos, int len, int line, int col)
{ if (len > 0) _comments.append(QDeclarativeJS::AST::SourceLocation(pos, len, line, col)); }

QList<QDeclarativeJS::AST::SourceLocation> Engine::comments() const
{ return _comments; }

NameId *Engine::intern(const QChar *u, int s)
{ return const_cast<NameId *>(&*_literals.insert(NameId(u, s))); }

QString Engine::toString(NameId *id)
{ return id->asString(); }

Lexer *Engine::lexer() const
{ return _lexer; }

void Engine::setLexer(Lexer *lexer)
{ _lexer = lexer; }

NodePool *Engine::nodePool() const
{ return _nodePool; }

void Engine::setNodePool(NodePool *nodePool)
{ _nodePool = nodePool; }



} // end of namespace QDeclarativeJS

QT_QML_END_NAMESPACE