import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import suno.visualizer 1.0

Rectangle {
    id: root

    color: "#000000"

    property var visualizer: appController ? appController.visualizer : null
    property var audioDecoder: appController ? appController.audioDecoder : null
    property bool projectMReady: visualizer && visualizer.initialized

    ProjectMRenderer {
        id: projectMRenderer
        anchors.fill: parent
        visible: projectMReady

        Connections {
            target: audioDecoder
            function onAudioDataReady(data, samples, channels) {
                if (projectMRenderer.initialized) {
                    projectMRenderer.addPCMData(data, samples, channels > 1)
                }
            }
        }
    }

    Rectangle {
        id: placeholderViz
        anchors.fill: parent
        visible: !projectMReady
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1a1a2e" }
            GradientStop { position: 0.5; color: "#16213e" }
            GradientStop { position: 1.0; color: "#0f0f1a" }
        }

        Canvas {
            id: vizCanvas
            anchors.fill: parent

            property real time: 0

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                var barCount = 64
                var barWidth = width / barCount

                for (var i = 0; i < barCount; i++) {
                    var x = i * barWidth
                    var normalizedPos = i / barCount

                    var heightMultiplier = Math.sin(normalizedPos * Math.PI) *
                        (0.5 + 0.5 * Math.sin(time * 2 + i * 0.1))
                    var barHeight = height * 0.6 * heightMultiplier

                    var hue = (normalizedPos * 120 + time * 20) % 360
                    ctx.fillStyle = "hsla(" + hue + ", 70%, 50%, 0.8)"

                    var centerY = height / 2
                    ctx.fillRect(x, centerY - barHeight / 2, barWidth - 2, barHeight)
                }

                ctx.beginPath()
                ctx.arc(width / 2, height / 2, 100 + 50 * Math.sin(time), 0, 2 * Math.PI)
                ctx.strokeStyle = themeManager.accentColor
                ctx.lineWidth = 2
                ctx.stroke()
            }

            Timer {
                interval: 16
                running: !projectMReady
                repeat: true
                onTriggered: {
                    vizCanvas.time += 0.016
                    vizCanvas.requestPaint()
                }
            }
        }

        Label {
            anchors.centerIn: parent
            text: "projectM Initializing..."
            color: "white"
            font.pixelSize: 18
            opacity: 0.6
        }
    }

    Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        spacing: 4

        Label {
            text: visualizer ?
                ("Preset: " + (visualizer.currentPreset || "None")) :
                "Initializing..."
            color: "white"
            font.pixelSize: 12
            opacity: 0.7
        }

        Label {
            text: "FPS: " + (visualizer ? visualizer.fps : 60)
            color: "white"
            font.pixelSize: 10
            opacity: 0.5
        }

        Label {
            text: visualizer ?
                ("Presets: " + visualizer.presetCount) :
                "Loading presets..."
            color: "white"
            font.pixelSize: 10
            opacity: 0.5
        }
    }

    RoundButton {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16

        icon.source: "qrc:/SunoVisualizer/resources/icons/maximize.svg"
        icon.color: themeManager.textColor
        flat: true
        opacity: 0.7

        onClicked: {
            if (mainWindow.visibility === Window.FullScreen) {
                mainWindow.showNormal()
            } else {
                mainWindow.showFullScreen()
            }
        }

        ToolTip.visible: hovered
        ToolTip.text: "Toggle Fullscreen (F11)"
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 40

        width: lyricLabel.width + 40
        height: lyricLabel.height + 20
        radius: 8
        color: Qt.rgba(0, 0, 0, 0.6)
        visible: lyricLabel.text.length > 0

        Label {
            id: lyricLabel
            anchors.centerIn: parent

            text: ""
            color: "white"
            font.pixelSize: 28
            font.weight: Font.Bold
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 16

        width: recLabel.width + 24
        height: recLabel.height + 12
        radius: 4
        color: "#CC000000"
        visible: appController.isRecording

        RowLayout {
            id: recLabel
            anchors.centerIn: parent
            spacing: 8

            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "#FF4444"

                SequentialAnimation on opacity {
                    running: appController.isRecording
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 500 }
                    NumberAnimation { to: 1.0; duration: 500 }
                }
            }

            Label {
                text: "REC"
                color: "#FF4444"
                font.pixelSize: 12
                font.weight: Font.Bold
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onDoubleClicked: {
            if (mainWindow.visibility === Window.FullScreen) {
                mainWindow.showNormal()
            } else {
                mainWindow.showFullScreen()
            }
        }

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                vizContextMenu.popup()
            }
        }
    }

    Menu {
        id: vizContextMenu

        MenuItem {
            text: "Next Preset"
            onTriggered: visualizer ? visualizer.nextPreset() : null
        }
        MenuItem {
            text: "Previous Preset"
            onTriggered: visualizer ? visualizer.previousPreset() : null
        }
        MenuItem {
            text: "Random Preset"
            onTriggered: visualizer ? visualizer.randomPreset() : null
        }
        MenuSeparator {}
        MenuItem {
            text: "Lock Preset"
            checkable: true
            onTriggered: visualizer ? visualizer.lockPreset(checked) : null
        }
        MenuSeparator {}
        MenuItem {
            text: "Fullscreen"
            onTriggered: {
                if (mainWindow.visibility === Window.FullScreen) {
                    mainWindow.showNormal()
                } else {
                    mainWindow.showFullScreen()
                }
            }
        }
    }
}
