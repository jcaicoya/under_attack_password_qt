#pragma once

#include <QList>
#include <QStringList>
#include <QWidget>

class QHBoxLayout;
class QPushButton;

class BottomNavBar : public QWidget
{
    Q_OBJECT

public:
    explicit BottomNavBar(QWidget* parent = nullptr);

    void setItems(const QStringList& labels);
    void setCurrentIndex(int index);
    int currentIndex() const { return m_currentIndex; }

signals:
    void currentIndexChanged(int index);

private:
    void rebuild();

    QStringList m_labels;
    QList<QPushButton*> m_buttons;
    QHBoxLayout* m_layout = nullptr;
    int m_currentIndex = -1;
};
