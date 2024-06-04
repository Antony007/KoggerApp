import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

TextField {
    property bool small: false

    id: control
    implicitHeight: theme.controlHeight
    height: theme.controlHeight
    padding: 0
    rightPadding: 10
    leftPadding: 10

    font: small ? theme.textFontS : theme.textFont
    color: theme.textColor

    background:  Rectangle {
        id: backRect
        radius: 1
        color: control.down ? theme.controlSolidBackColor : theme.controlBackColor
        border.color: control.down ? theme.controlSolidBorderColor : theme.controlBorderColor
        border.width: 0
    }
}
