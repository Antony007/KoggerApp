#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QFont>
#include <QColor>

class Themes : public QObject
{
    Q_OBJECT
public:

    Themes() {
        setTheme();
        _isConsoleVisible = false;
    }

    Q_PROPERTY(QColor textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QColor textSolidColor READ textSolidColor NOTIFY changed)
    Q_PROPERTY(QFont textFont READ textFont NOTIFY changed)
    Q_PROPERTY(QFont textFontS READ textFontS NOTIFY changed)

    Q_PROPERTY(QColor menuBackColor READ menuBackColor NOTIFY changed)
    Q_PROPERTY(QColor controlBackColor READ controlBackColor NOTIFY changed)
    Q_PROPERTY(QColor controlBorderColor READ controlBorderColor NOTIFY changed)
    Q_PROPERTY(QColor controlSolidBackColor READ controlSolidBackColor NOTIFY changed)
    Q_PROPERTY(QColor controlSolidBorderColor READ controlSolidBorderColor NOTIFY changed)
    Q_PROPERTY(int controlHeight READ controlHeight NOTIFY changed)

    Q_PROPERTY(int themeID READ themeId WRITE setTheme NOTIFY changed)

    Q_PROPERTY(bool consoleVisible READ consoleVisible WRITE setConsoleVisible NOTIFY interfaceChanged)

    QColor textColor() { return *_textColor; }
    QColor textSolidColor() { return *_textSolidColor; }
    QFont textFont() { return *_textFont; }
    QFont textFontS() { return *_textFontS; }

    QColor menuBackColor() { return *_menuBackColor; }
    QColor controlBackColor() { return *_controlBackColor; }
    QColor controlBorderColor() { return *_controlBorderColor; }
    QColor controlSolidBackColor() { return *_controlSolidBackColor; }
    QColor controlSolidBorderColor() { return *_controlSolidBorderColor; }
    int controlHeight() { return _controlHeight; }

    void setTheme(int theme_id = 0) {
        _id = theme_id;
//        _textFont = new QFont("PT Sans Caption", 26);
//        _textFontS = new QFont("PT Sans Caption", 20);

        _textFont = new QFont("PT Sans Caption", 14);
        _textFontS = new QFont("PT Sans Caption", 12);

        if(theme_id == 0) {
            _textColor = new QColor(250, 250, 250);
            _textSolidColor = new QColor(250, 250, 250);
            _menuBackColor = new QColor(45, 45, 45, 240);
            _controlBackColor = new QColor(60, 60, 60);
            _controlBorderColor = new QColor(100, 100, 100);
            _controlSolidBackColor = new QColor(100, 100, 100);
            _controlSolidBorderColor = new QColor(150, 150, 150);
        } else if(theme_id == 1) {
            _textColor = new QColor(255, 255, 255);
            _textSolidColor = new QColor(0, 0, 0);
            _menuBackColor = new QColor(0, 0, 0, 255);
            _controlBackColor = new QColor(55, 55, 55);
            _controlBorderColor = new QColor(155, 155, 155);
            _controlSolidBackColor = new QColor(255, 255, 255);
            _controlSolidBorderColor = new QColor(0, 0, 0, 0);
        } else if(theme_id == 2) {
            _textColor = new QColor(25, 25, 25);
            _textSolidColor = new QColor(25, 25, 25);
            _menuBackColor = new QColor(240, 240, 240, 240);
            _controlBackColor = new QColor(250, 250, 250);
            _controlBorderColor = new QColor(100, 100, 100);
            _controlSolidBackColor = new QColor(255, 255, 255);
            _controlSolidBorderColor = new QColor(150, 150, 150);
        } else if(theme_id == 3) {
            _textColor = new QColor(0, 0, 0);
            _textSolidColor = new QColor(255, 255, 255);
            _menuBackColor = new QColor(250, 250, 250, 250);
            _controlBackColor = new QColor(255, 255, 255);
            _controlBorderColor = new QColor(100, 100, 100);
            _controlSolidBackColor = new QColor(0, 0, 0);
            _controlSolidBorderColor = new QColor(255, 255, 255);
        }

//        _controlHeight = 50; // android
        _controlHeight = 26;

        emit changed();
    }

    int themeId() { return _id; }

    void setConsoleVisible(bool vis) {
        _isConsoleVisible = vis;
        interfaceChanged();
    }

    bool consoleVisible() { return _isConsoleVisible; }

signals:
    void changed();

    void interfaceChanged();
protected:
    int _id = 0;

    QColor* _textColor;
    QColor* _textSolidColor;
    QFont* _textFont;
    QFont* _textFontS;

    QColor* _menuBackColor;
    QColor* _controlBackColor;
    QColor* _controlBorderColor;
    QColor* _controlSolidBackColor;
    QColor* _controlSolidBorderColor;
    int32_t _controlHeight;

    bool _isConsoleVisible;
};

#endif // THEME_H