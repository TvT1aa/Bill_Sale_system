#include "thememanager.h"
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QStyleFactory>

void ThemeManager::forceLightTheme()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;

    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));

    // 设置高亮选中颜色（经典 Windows 蓝）
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::PlaceholderText, QColor(0, 0, 0, 160));
    // 3. 覆盖操作系统的意志，锁死这套调色板
    QApplication::setPalette(lightPalette);
}