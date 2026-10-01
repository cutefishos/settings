// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTSMODEL_H
#define FONTSMODEL_H

#include <QThread>

class FontsModel : public QThread
{
    Q_OBJECT
    Q_PROPERTY(QStringList generalFonts READ generalFonts NOTIFY loadFinished)
    Q_PROPERTY(QStringList fixedFonts READ fixedFonts NOTIFY loadFinished)
    Q_PROPERTY(QString systemGeneralFont READ systemGeneralFont NOTIFY loadFinished)
    Q_PROPERTY(QString systemFixedFont READ systemFixedFont NOTIFY loadFinished)

public:
    explicit FontsModel(QObject *parent = nullptr);
    ~FontsModel() override;

    void run() override;

    QStringList generalFonts() const;
    QStringList fixedFonts() const;

    QString systemGeneralFont() const;
    QString systemFixedFont() const;

signals:
    void loadFinished();

private:
    QStringList m_generalFonts;
    QStringList m_fixedFonts;
    QString m_lanCode;
};

#endif // FONTSMODEL_H
