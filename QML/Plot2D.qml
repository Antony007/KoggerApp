import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.1

import WaterFall 1.0

WaterFall {
    id: plot

    PinchArea {
        id: pinch2D
        anchors.fill: parent

        enabled: true
        onPinchUpdated: {
            plot.verZoomEvent((pinch.previousScale - pinch.scale)*500.0)
            plot.horScrollEvent(-(pinch.previousCenter.x - pinch.center.x))
            plot.verScrollEvent(pinch.previousCenter.y - pinch.center.y)
        }

        onPinchStarted: {
            mousearea.enabled = false
            plot.setMousePosition(-1, -1)
        }

        onPinchFinished: {
            mousearea.enabled = true
            plot.plotMousePosition(-1, -1)
        }

        MouseArea {
            id: mousearea

//            propagateComposedEvents: true

            enabled: true
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onWheel: {
                if (wheel.modifiers & Qt.ControlModifier) {
                    plot.verZoomEvent(-wheel.angleDelta.y)
                } else if (wheel.modifiers & Qt.ShiftModifier) {
                    plot.verScrollEvent(-wheel.angleDelta.y)
                } else {
                    plot.horScrollEvent(wheel.angleDelta.y)
                }
            }

            onClicked: {
                plot.focus = true
                if (mouse.button === Qt.RightButton) {
                    menuBlock.position(mouse.x, mouse.y)

                }
            }

            onReleased: {
                if (mouse.button === Qt.LeftButton) {
                    plot.plotMousePosition(-1, -1)
                }
            }

            onPressed: {
                if (mouse.button === Qt.LeftButton) {
                    menuBlock.visible = false
                }

                if (mouse.button === Qt.LeftButton) {
                    plot.plotMousePosition(mouse.x, mouse.y)
                }
            }

            onPositionChanged: {
                if(mousearea.pressedButtons & Qt.LeftButton) {
                    plot.plotMousePosition(mouse.x, mouse.y)
                }
            }

        }
    }

    onHeightChanged: {
        if(menuBlock.visible) {
            menuBlock.position(menuBlock.x, menuBlock.y)
        }
    }

    onWidthChanged: {
        if(menuBlock.visible) {
            menuBlock.position(menuBlock.x, menuBlock.y)
        }
    }

    MenuFrame {
        Layout.alignment: Qt.AlignHCenter
        //visible: visible2dButton.checked
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 4
        margins: 0

        // isDraggable: true
        isOpacityControlled: true

        ColumnLayout {

            CText {
                Layout.fillWidth: true
                Layout.topMargin: 0
                Layout.preferredWidth: theme.controlHeight*1.2
                // visible: chartEnable.checked // TODO
                horizontalAlignment: Text.AlignHCenter
                text: echogramLevelsSlider.stopValue
                small: true
            }

            ChartLevel {
                // opacity: 0.8
                Layout.fillWidth: true
                Layout.preferredWidth: theme.controlHeight*1.2
                id: echogramLevelsSlider
                // visible: chartEnable.checked // TODO
                Layout.alignment: Qt.AlignHCenter

                onStartValueChanged: {
                    plot.plotEchogramSetLevels(startValue, stopValue);
                }

                onStopValueChanged: {
                    plot.plotEchogramSetLevels(startValue, stopValue);
                }

                Component.onCompleted: {
                    plot.plotEchogramSetLevels(startValue, stopValue);
                }

                Settings {
                    property alias echogramLevelsStart: echogramLevelsSlider.startValue
                    property alias echogramLevelsStop: echogramLevelsSlider.stopValue
                }
            }

            CText {
                Layout.fillWidth: true
                Layout.preferredWidth: theme.controlHeight*1.2
                Layout.bottomMargin: 0
                // visible: chartEnable.checked // TODO
                horizontalAlignment: Text.AlignHCenter

                text: echogramLevelsSlider.startValue
                small: true
            }
        }
    }

    RowLayout {
        id: menuBlock
        Layout.alignment: Qt.AlignHCenter
        spacing: 1
        visible: false
        Layout.margins: 0

        function position(mx, my) {
            var oy = plot.height - (my + implicitHeight)
            if(oy < 0) {
                my = my + oy
            }

            if(my < 0) {
                my = 0
            }

            var ox = plot.width - (mx - implicitWidth)
            if(ox < 0) {
                mx = mx + ox
            }

            x = mx
            y = my
            visible = true
//            backgrn.focus = true
        }

        ButtonGroup { id: pencilbuttonGroup }

        CheckButton {
            Layout.fillWidth: true
            icon.source: "./icons/direction-arrows.svg"
            checked: plot.toolMode == 1
            backColor: theme.controlBackColor

            onCheckedChanged: {
                if (checked) {
                    plot.plotMouseTool(1)
                }
            }

            ButtonGroup.group: pencilbuttonGroup
        }

        CheckButton {
            Layout.fillWidth: true
            icon.source: "./icons/arrow-bar-to-down.svg"
            checked: plot.toolMode == 2
            backColor: theme.controlBackColor

            onCheckedChanged: {
                if (checked) {
                    plot.plotMouseTool(2)
                }
            }

            ButtonGroup.group: pencilbuttonGroup
        }

        CheckButton {
            Layout.fillWidth: true
            icon.source: "./icons/pencil.svg"
            checked: plot.toolMode == 3
            backColor: theme.controlBackColor

            onCheckedChanged: {
                if (checked) {
                    plot.plotMouseTool(3)
                }
            }

            ButtonGroup.group: pencilbuttonGroup
        }

        CheckButton {
            Layout.fillWidth: true
            icon.source: "./icons/arrow-bar-to-up.svg"
            checked: plot.toolMode == 4
            backColor: theme.controlBackColor

            onCheckedChanged: {
                if (checked) {
                    plot.plotMouseTool(4)
                }
            }

            ButtonGroup.group: pencilbuttonGroup
        }

        CheckButton {
            Layout.fillWidth: true
            icon.source: "./icons/eraser.svg"
            checked: plot.toolMode == 5
            backColor: theme.controlBackColor

            onCheckedChanged: {
                if (checked) {
                    plot.plotMouseTool(5)
                }
            }

            ButtonGroup.group: pencilbuttonGroup
        }
    }
}
