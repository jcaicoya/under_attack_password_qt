#include "AttackPanel.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRandomGenerator>

static const QStringList kDictionary = {
    "123456","password","admin","qwerty","letmein","master","hello","monkey",
    "dragon","baseball","iloveyou","trustno1","sunshine","princess","welcome",
    "shadow","superman","michael","football","batman","jessica","ninja","abc123",
    "mustang","charlie","donald","samsung","pass","12345","123456789","1234567",
    "contraseña","clave","secreto","miperro","micasa","amor","corazon","familia",
    "admin123","password1","qwerty123","admin1234","pass1234","test","login",
};

AttackPanel::AttackPanel(Type type, QWidget* parent)
    : QFrame(parent), m_type(type)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &AttackPanel::tick);
    buildUi();
}

void AttackPanel::buildUi() {
    setMinimumWidth(220);
    setFrameShape(QFrame::StyledPanel);
    applyStyle("#293241", "#5F6B78", "ESPERANDO");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    // Title
    QString title;
    switch (m_type) {
        case Type::BruteForce: title = "FUERZA BRUTA"; break;
        case Type::Dictionary: title = "DICCIONARIO";  break;
        case Type::Oracle:     title = "ORÁCULO";      break;
    }
    auto* titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("color:#E0E0E0; font-family:'Courier New'; font-size:14px; font-weight:bold; letter-spacing:2px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(titleLabel);

    // Status
    m_statusLabel = new QLabel("ESPERANDO", this);
    m_statusLabel->setStyleSheet("color:#5F6B78; font-family:'Courier New'; font-size:11px; letter-spacing:1px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_statusLabel);

    // Oracle inputs
    if (m_type == Type::Oracle) {
        m_wordInput = new QLineEdit(this);
        m_wordInput->setPlaceholderText("Palabra clave");
        m_wordInput->setStyleSheet("background:#141B22; color:#E0E0E0; border:1px solid #293241; font-family:'Courier New'; padding:4px;");
        m_dateInput = new QLineEdit(this);
        m_dateInput->setPlaceholderText("Fecha / número");
        m_dateInput->setStyleSheet("background:#141B22; color:#E0E0E0; border:1px solid #293241; font-family:'Courier New'; padding:4px;");
        root->addWidget(m_wordInput);
        root->addWidget(m_dateInput);
    }

    // Attempt display
    m_attemptLabel = new QLabel("—", this);
    m_attemptLabel->setStyleSheet("color:#00FF55; font-family:'Courier New'; font-size:22px; font-weight:bold;");
    m_attemptLabel->setAlignment(Qt::AlignCenter);
    m_attemptLabel->setMinimumHeight(60);
    root->addWidget(m_attemptLabel, 1);

    // Activate button
    m_activateBtn = new QPushButton("ACTIVAR", this);
    m_activateBtn->setStyleSheet(
        "QPushButton{background:#1A2535; color:#00FF55; border:1px solid #00FF55; font-family:'Courier New'; font-size:11px; padding:5px;}"
        "QPushButton:hover{background:#00FF55; color:#000;}");
    root->addWidget(m_activateBtn);
    connect(m_activateBtn, &QPushButton::clicked, this, &AttackPanel::activate);

    // Found / Failed buttons
    auto* btnRow = new QHBoxLayout;
    m_foundBtn  = new QPushButton("ENCONTRADA ✓", this);
    m_failedBtn = new QPushButton("FALLIDA ✗", this);
    m_foundBtn->setEnabled(false);
    m_failedBtn->setEnabled(false);
    m_foundBtn->setStyleSheet(
        "QPushButton{background:#1A2535; color:#00FF55; border:1px solid #293241; font-family:'Courier New'; font-size:10px; padding:5px;}"
        "QPushButton:enabled:hover{background:#00FF55; color:#000;}"
        "QPushButton:disabled{color:#293241; border-color:#1A2535;}");
    m_failedBtn->setStyleSheet(
        "QPushButton{background:#1A2535; color:#FF4444; border:1px solid #293241; font-family:'Courier New'; font-size:10px; padding:5px;}"
        "QPushButton:enabled:hover{background:#FF4444; color:#000;}"
        "QPushButton:disabled{color:#293241; border-color:#1A2535;}");
    btnRow->addWidget(m_foundBtn);
    btnRow->addWidget(m_failedBtn);
    root->addLayout(btnRow);

    connect(m_foundBtn, &QPushButton::clicked, this, [this]() {
        m_timer->stop();
        m_running = false;
        m_attemptLabel->setText(m_password);
        m_attemptLabel->setStyleSheet("color:#00FF55; font-family:'Courier New'; font-size:22px; font-weight:bold;");
        applyStyle("#00FF55", "#00FF55", "ENCONTRADA ✓");
        m_activateBtn->setEnabled(false);
        m_foundBtn->setEnabled(false);
        m_failedBtn->setEnabled(false);
        emit foundClicked();
    });
    connect(m_failedBtn, &QPushButton::clicked, this, [this]() {
        m_timer->stop();
        m_running = false;
        m_attemptLabel->setText("—");
        applyStyle("#FF4444", "#FF4444", "FALLIDA ✗");
        m_activateBtn->setEnabled(false);
        m_foundBtn->setEnabled(false);
        m_failedBtn->setEnabled(false);
        emit failedClicked();
    });
}

void AttackPanel::setPassword(const QString& password) {
    m_password = password;
}

void AttackPanel::activate() {
    if (m_running) return;
    m_running   = true;
    m_tickCount = 0;

    // Build attempt list
    if (m_type == Type::BruteForce) {
        m_baseAttempts.clear();
        for (int i = 0; i < 200; ++i)
            m_baseAttempts << QString("%1").arg(QRandomGenerator::global()->bounded(10000), 4, 10, QChar('0'));
        for (int i = 0; i < 50; ++i)
            m_baseAttempts << QString("%1").arg(QRandomGenerator::global()->bounded(100000), 5, 10, QChar('0'));
    } else if (m_type == Type::Dictionary) {
        m_baseAttempts = kDictionary;
    } else {
        // Oracle: build from inputs
        const QString w = oracleWord().toLower().trimmed();
        const QString d = oracleDate().trimmed();
        m_baseAttempts.clear();
        if (!w.isEmpty() && !d.isEmpty()) {
            m_baseAttempts << w << d
                << w + d << d + w
                << w + d + "!" << w + "." + d
                << w + "_" + d << d + "_" + w
                << w + d + "1" << w + d + "#"
                << w.at(0).toUpper() + w.mid(1) + d
                << w + d + "123" << w.toUpper() + d;
        }
        if (m_baseAttempts.isEmpty()) m_baseAttempts << "???";
    }

    applyStyle("#FFAA00", "#FFAA00", "ANALIZANDO...");
    m_activateBtn->setEnabled(false);
    m_foundBtn->setEnabled(true);
    m_failedBtn->setEnabled(true);
    m_timer->start(90);
}

void AttackPanel::reset() {
    m_timer->stop();
    m_running = false;
    m_tickCount = 0;
    m_password.clear();
    m_attemptLabel->setText("—");
    m_attemptLabel->setStyleSheet("color:#00FF55; font-family:'Courier New'; font-size:22px; font-weight:bold;");
    applyStyle("#293241", "#5F6B78", "ESPERANDO");
    m_activateBtn->setEnabled(true);
    m_foundBtn->setEnabled(false);
    m_failedBtn->setEnabled(false);
    if (m_wordInput) m_wordInput->clear();
    if (m_dateInput) m_dateInput->clear();
}

QString AttackPanel::oracleWord() const {
    return m_wordInput ? m_wordInput->text() : QString{};
}
QString AttackPanel::oracleDate() const {
    return m_dateInput ? m_dateInput->text() : QString{};
}

void AttackPanel::tick() {
    ++m_tickCount;
    m_attemptLabel->setText(nextAttempt());
}

QString AttackPanel::nextAttempt() {
    if (m_baseAttempts.isEmpty()) return "...";
    const int idx = QRandomGenerator::global()->bounded(m_baseAttempts.size());
    return m_baseAttempts.at(idx);
}

void AttackPanel::applyStyle(const QString& borderColor, const QString& statusColor, const QString& status) {
    setStyleSheet(QString("AttackPanel { background:#0D1117; border:1px solid %1; border-radius:4px; }").arg(borderColor));
    if (m_statusLabel)
        m_statusLabel->setStyleSheet(QString("color:%1; font-family:'Courier New'; font-size:11px; letter-spacing:1px;").arg(statusColor));
    if (m_statusLabel)
        m_statusLabel->setText(status);
}
