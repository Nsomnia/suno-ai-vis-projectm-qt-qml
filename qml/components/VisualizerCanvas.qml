import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    color: "#000000"
    
    // Placeholder gradient background until projectM renders
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1a1a2e" }
            GradientStop { position: 0.5; color: "#16213e" }
            GradientStop { position: 1.0; color: "#0f0f1a" }
        }
    }
    
    // Animated placeholder visualization
    Canvas {
        id: vizCanvas
        anchors.fill: parent
        
        property real time: 0
        property real bassLevel: 0.5
        property real midLevel: 0.5
        property real trebleLevel: 0.5
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            // Draw some placeholder visualization bars
            var barCount = 64;
            var barWidth = width / barCount;
            
            for (var i = 0; i < barCount; i++) {
                var x = i * barWidth;
                var normalizedPos = i / barCount;
                
                // Simulated frequency response
                var heightMultiplier = Math.sin(normalizedPos * Math.PI) * 
                                       (0.5 + 0.5 * Math.sin(time * 2 + i * 0.1));
                var barHeight = height * 0.6 * heightMultiplier;
                
                // Color based on frequency band
                var hue = (normalizedPos * 120 + time * 20) % 360;
                ctx.fillStyle = "hsla(" + hue + ", 70%, 50%, 0.8)";
                
                // Draw bar from center
                var centerY = height / 2;
                ctx.fillRect(x, centerY - barHeight / 2, barWidth - 2, barHeight);
            }
            
            // Draw circular element
            ctx.beginPath();
            ctx.arc(width / 2, height / 2, 100 + 50 * Math.sin(time), 0, 2 * Math.PI);
            ctx.strokeStyle = themeManager.accentColor;
            ctx.lineWidth = 2;
            ctx.stroke();
        }
        
        Timer {
            interval: 16 // ~60 fps
            running: true
            repeat: true
            onTriggered: {
                vizCanvas.time += 0.016;
                vizCanvas.requestPaint();
            }
        }
    }
    
    // Overlay info
    Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        spacing: 4
        
        Label {
            text: appController.visualizer ? 
                  ("Preset: " + (appController.visualizer.currentPreset || "None")) : 
                  "Initializing..."
            color: "white"
            font.pixelSize: 12
            opacity: 0.7
        }
        
        Label {
            text: "FPS: 60" // Would be actual FPS
            color: "white"
            font.pixelSize: 10
            opacity: 0.5
        }
    }
    
    // Fullscreen button
    RoundButton {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        
        icon.source: "qrc:/icons/maximize.svg"
        icon.color: "white"
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
    
    // Lyric display overlay
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
            
            text: "" // Would be bound to current lyric from timeline
            color: "white"
            font.pixelSize: 28
            font.weight: Font.Bold
            
            // Text shadow effect
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 2
                verticalOffset: 2
                radius: 8
                samples: 16
                color: "#80000000"
            }
        }
    }
    
    // Recording indicator
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
    
    // Mouse area for interactions
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
    
    // Context menu
    Menu {
        id: vizContextMenu
        
        MenuItem {
            text: "Next Preset"
            onTriggered: appController.visualizer.nextPreset()
        }
        MenuItem {
            text: "Previous Preset"
            onTriggered: appController.visualizer.previousPreset()
        }
        MenuItem {
            text: "Random Preset"
            onTriggered: appController.visualizer.randomPreset()
        }
        MenuSeparator {}
        MenuItem {
            text: "Lock Preset"
            checkable: true
            onTriggered: appController.visualizer.lockPreset(checked)
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
    
    // Drop shadow import for lyric effect
    component DropShadow: Item {
        property real horizontalOffset: 0
        property real verticalOffset: 0
        property real radius: 0
        property int samples: 0
        property color color: "black"
    }
}