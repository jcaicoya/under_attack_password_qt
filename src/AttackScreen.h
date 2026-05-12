#pragma once
#include <QWidget>

class QLabel;
class QPushButton;
class AttackPanel;

class AttackScreen : public QWidget {
    Q_OBJECT
public:
    explicit AttackScreen(QWidget* parent = nullptr);

    void onPasswordReceived(const QString& password);
    void onClientConnected();
    void onClientDisconnected();

signals:
    void verdictReady(bool cracked, const QString& password);

private:
    void buildUi();
    void resetAll();

    QString       m_password;
    QLabel*       m_connLabel;
    QLabel*       m_passwordLabel;
    QPushButton*  m_safeBtn;
    QPushButton*  m_resetBtn;
    AttackPanel*  m_brutePanel;
    AttackPanel*  m_dictPanel;
    AttackPanel*  m_oraclePanel;
};
