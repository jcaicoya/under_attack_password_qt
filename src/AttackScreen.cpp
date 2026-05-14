#include "AttackScreen.h"
#include "AttackPanel.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

AttackScreen::AttackScreen(QWidget* parent) : QWidget(parent) {
    buildUi();
}

void AttackScreen::buildUi() {
    setStyleSheet("background:#0A0F14;");
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(12);

    // ── Barra superior: conexión + contraseña recibida ────────
    auto* topBar = new QFrame(this);
    topBar->setStyleSheet("background:#0D1117; border:1px solid #293241; border-radius:4px;");
    auto* topRow = new QHBoxLayout(topBar);
    topRow->setContentsMargins(12, 8, 12, 8);

    m_connLabel = new QLabel("● SIN CONEXIÓN", topBar);
    m_connLabel->setStyleSheet("color:#FF4444; font-family:'Courier New'; font-size:12px;");

    auto* passLabelStatic = new QLabel("CONTRASEÑA:", topBar);
    passLabelStatic->setStyleSheet("color:#5F6B78; font-family:'Courier New'; font-size:12px;");

    m_passwordLabel = new QLabel("—", topBar);
    m_passwordLabel->setStyleSheet("color:#FFAA00; font-family:'Courier New'; font-size:18px; font-weight:bold; letter-spacing:2px;");

    m_safeBtn = new QPushButton("ENVIAR SEGURA ✓", topBar);
    m_safeBtn->setEnabled(false);
    m_safeBtn->setStyleSheet(
        "QPushButton{background:#1A2535; color:#00FF55; border:1px solid #293241; font-family:'Courier New'; font-size:11px; padding:6px 14px;}"
        "QPushButton:enabled{border-color:#00FF55;}"
        "QPushButton:enabled:hover{background:#00FF55; color:#000;}"
        "QPushButton:disabled{color:#293241;}");

    m_resetBtn = new QPushButton("RESET", topBar);
    m_resetBtn->setStyleSheet(
        "QPushButton{background:#1A2535; color:#5F6B78; border:1px solid #293241; font-family:'Courier New'; font-size:11px; padding:6px 14px;}"
        "QPushButton:hover{background:#293241; color:#E0E0E0;}");

    topRow->addWidget(m_connLabel);
    topRow->addStretch();
    topRow->addWidget(passLabelStatic);
    topRow->addWidget(m_passwordLabel);
    topRow->addSpacing(24);
    topRow->addWidget(m_safeBtn);
    topRow->addSpacing(8);
    topRow->addWidget(m_resetBtn);

    root->addWidget(topBar);

    // ── Tres paneles de ataque ────────────────────────────────
    auto* panelsRow = new QHBoxLayout;
    panelsRow->setSpacing(12);

    m_brutePanel  = new AttackPanel(AttackPanel::Type::BruteForce, this);
    m_dictPanel   = new AttackPanel(AttackPanel::Type::Dictionary,  this);
    m_oraclePanel = new AttackPanel(AttackPanel::Type::Oracle,      this);

    panelsRow->addWidget(m_brutePanel,  1);
    panelsRow->addWidget(m_dictPanel,   1);
    panelsRow->addWidget(m_oraclePanel, 1);

    root->addLayout(panelsRow, 1);

    // ── Conexiones ────────────────────────────────────────────
    connect(m_brutePanel, &AttackPanel::foundClicked, this, [this]() {
        m_dictPanel->setEnabled(false);
        m_oraclePanel->setEnabled(false);
        emit verdictReady(true, m_password);
    });
    connect(m_brutePanel, &AttackPanel::failedClicked, this, [this]() {
        m_dictPanel->activate();
    });

    connect(m_dictPanel, &AttackPanel::foundClicked, this, [this]() {
        m_oraclePanel->setEnabled(false);
        emit verdictReady(true, m_password);
    });
    connect(m_dictPanel, &AttackPanel::failedClicked, this, [this]() {
        m_oraclePanel->activate();
    });

    connect(m_oraclePanel, &AttackPanel::foundClicked, this, [this]() {
        emit verdictReady(true, m_password);
    });
    connect(m_oraclePanel, &AttackPanel::failedClicked, this, [this]() {
        m_safeBtn->setEnabled(true);
    });

    connect(m_safeBtn, &QPushButton::clicked, this, [this]() {
        m_safeBtn->setEnabled(false);
        emit verdictReady(false, {});
    });

    connect(m_resetBtn, &QPushButton::clicked, this, &AttackScreen::resetAll);
}

void AttackScreen::onPasswordReceived(const QString& password) {
    m_password = password;
    m_passwordLabel->setText(password);
    m_safeBtn->setEnabled(false);
    m_brutePanel->setEnabled(true);
    m_dictPanel->setEnabled(true);
    m_oraclePanel->setEnabled(true);
    m_brutePanel->setPassword(password);
    m_dictPanel->setPassword(password);
    m_oraclePanel->setPassword(password);
    m_brutePanel->activate();
}

void AttackScreen::onClientConnected(bool connected) {
    if (connected) {
        m_connLabel->setText("● CONECTADO");
        m_connLabel->setStyleSheet("color:#00FF55; font-family:'Courier New'; font-size:12px;");
    } else {
        m_connLabel->setText("● SIN CONEXIÓN");
        m_connLabel->setStyleSheet("color:#FF4444; font-family:'Courier New'; font-size:12px;");
        resetAll();
    }
}

void AttackScreen::resetAll() {
    m_password.clear();
    m_passwordLabel->setText("—");
    m_safeBtn->setEnabled(false);
    m_brutePanel->setEnabled(true);
    m_dictPanel->setEnabled(true);
    m_oraclePanel->setEnabled(true);
    m_brutePanel->reset();
    m_dictPanel->reset();
    m_oraclePanel->reset();
}
