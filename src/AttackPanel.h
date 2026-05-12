#pragma once
#include <QFrame>
#include <QStringList>

class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;

class AttackPanel : public QFrame {
    Q_OBJECT
public:
    enum class Type { BruteForce, Dictionary, Oracle };

    explicit AttackPanel(Type type, QWidget* parent = nullptr);

    void setPassword(const QString& password);
    void activate();
    void reset();

    // Only meaningful for Oracle
    QString oracleWord() const;
    QString oracleDate() const;

signals:
    void foundClicked();
    void failedClicked();

private:
    void buildUi();
    void tick();
    QString nextAttempt();
    void applyStyle(const QString& borderColor, const QString& statusColor, const QString& status);

    Type         m_type;
    QString      m_password;
    bool         m_running = false;
    int          m_tickCount = 0;
    QStringList  m_baseAttempts;
    QTimer*      m_timer;

    QLabel*      m_statusLabel;
    QLabel*      m_attemptLabel;
    QLineEdit*   m_wordInput = nullptr;
    QLineEdit*   m_dateInput = nullptr;
    QPushButton* m_activateBtn;
    QPushButton* m_foundBtn;
    QPushButton* m_failedBtn;
};
